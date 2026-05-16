// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief libFuzzer harness targeting the pre-4.1 flat binary load path.
 *
 * Every existing harness generates v5.1 or v4.7 panda files and routes through
 * GraphicElement::loadNewFormat (the QMap-based path introduced in 4.1).
 * The original flat binary format (GraphicElement::loadOldFormat, versions 1.x
 * through 4.0.x) is entirely dark in all other harnesses.
 *
 * loadOldFormat reads a fixed sequence of fields directly from the stream:
 *   pos (QPointF) | rotation (qreal) | label (QString, >=1.2) |
 *   port sizes ×4 (quint64, >=1.3) | trigger (QKeySequence, >=1.9) |
 *   unused priority (quint64, >=4.0.1) | input ports | output ports |
 *   appearance names (>=2.7)
 *
 * Element-type overrides (e.g. Clock, InputSwitch, Buzzer) read additional
 * fields in the old path (bare ints, lock state, frequency, etc.) that are
 * only exercised here.
 *
 * This harness uses FuzzedDataProvider to generate structurally valid flat-
 * binary panda streams at a randomly selected old version, then feeds them
 * to WorkSpace::load to exercise loadOldFormat and all its version branches.
 */

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QEvent>
#include <QKeySequence>
#include <QPointF>
#include <QScopeGuard>
#include <QString>
#include <QVersionNumber>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/Enums.h"
#include "App/IO/Serialization.h"
#include "App/IO/VersionInfo.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/Versions.h"

namespace {

QApplication *g_app  = nullptr;
int  g_argc = 0;
char **g_argv = nullptr;

// Old-format element types (excluding IC which needs file-backed blobs)
constexpr Enums::ElementType kOldFmtTypes[] = {
    Enums::ElementType::And,          Enums::ElementType::AudioBox,
    Enums::ElementType::Buzzer,       Enums::ElementType::Clock,
    Enums::ElementType::DFlipFlop,    Enums::ElementType::DLatch,
    Enums::ElementType::Demux,        Enums::ElementType::Display7,
    Enums::ElementType::Display14,    Enums::ElementType::Display16,
    Enums::ElementType::IC,           Enums::ElementType::InputButton,
    Enums::ElementType::InputGnd,     Enums::ElementType::InputRotary,
    Enums::ElementType::InputSwitch,  Enums::ElementType::InputVcc,
    Enums::ElementType::JKFlipFlop,
    Enums::ElementType::Led,          Enums::ElementType::Mux,
    Enums::ElementType::Nand,         Enums::ElementType::Node,
    Enums::ElementType::Nor,          Enums::ElementType::Not,
    Enums::ElementType::Or,           Enums::ElementType::SRFlipFlop,
    Enums::ElementType::SRLatch,      Enums::ElementType::TFlipFlop,
    Enums::ElementType::JKLatch,      Enums::ElementType::Xnor,
    Enums::ElementType::Xor,
};

struct VersionSpec { int maj, min, patch; };
// Known old versions: pick one that exercises different version branches
constexpr VersionSpec kOldVersions[] = {
    {1,1,0},   // hasClock
    {1,2,0},   // hasLabels
    {1,3,0},   // hasPortSizes
    {1,4,0},   // hasSceneRect boundary
    {1,6,0},   // hasDisplay7Color
    {1,7,0},   // hasDisplay7ExtColor
    {1,9,0},   // hasTrigger
    {2,0,0},   // gap: hasTrigger but !hasAudio
    {2,4,0},   // hasAudio
    {2,7,0},   // hasAppearanceNames
    {3,0,0},   // hasDolphinFilename
    {3,1,0},   // hasLockState
    {3,3,0},   // hasDolphinSentinelFix (sentinel "none" → "" fix)
    {4,0,0},   // just before QMap
    {4,0,1},   // hasUnusedPriority
};

// Write one element in the flat old-format binary layout.
// basePtr: unique base value for this element's port pointer IDs (for portMap registration).
void writeOldElement(QDataStream &s, FuzzedDataProvider &fdp,
                     Enums::ElementType etype, const QVersionNumber &ver,
                     quint64 basePtr = 0)
{
    namespace VI = VersionInfo;
    using ET = Enums::ElementType;

    // GraphicElement::Type discriminator: QGraphicsItem::UserType + 3 = 65539
    s << static_cast<int>(65539);

    // ElementType as quint64 — Serialization::deserialize reads via operator>>(ElementType&)
    s << etype;

    // pos (QPointF)
    s << QPointF(fdp.ConsumeFloatingPoint<double>(),
                 fdp.ConsumeFloatingPoint<double>());

    // rotation (qreal) — present in all versions (rotation added early)
    s << fdp.ConsumeFloatingPoint<double>();

    // label (QString, >=1.2)
    if (VI::hasLabels(ver))
        s << QString::fromStdString(fdp.ConsumeBytesAsString(
              fdp.ConsumeIntegralInRange<size_t>(0, 8)));

    // port sizes ×4 (quint64 min/max for in/out, >=1.3) — values are ignored by loader
    if (VI::hasPortSizes(ver)) {
        s << quint64(0) << quint64(fdp.ConsumeIntegralInRange<int>(0,4))
          << quint64(0) << quint64(fdp.ConsumeIntegralInRange<int>(0,4));
    }

    // trigger (QKeySequence, >=1.9)
    if (VI::hasTrigger(ver))
        s << QKeySequence(fdp.ConsumeIntegral<int>());

    // unused priority (quint64, >=4.0.1)
    if (VI::hasUnusedPriority(ver))
        s << quint64(0);

    // Old-format port entry: quint64 ptr + QString name + int flags.
    // loadInputPort/loadOutputPort always read these three fields.
    // ptr values are registered in context.portMap so connections can wire up.
    // Port counts match real element minInputSize/minOutputSize so element-specific
    // code paths (pin remapping, flipflop port ordering) actually execute.
    const auto writePortList = [&](int count, quint64 ptrBase) {
        s << quint64(count);
        for (int i = 0; i < count; ++i) {
            s << (ptrBase + static_cast<quint64>(i));
            s << QString(); // port name
            s << int(0);    // flags (unused)
        }
    };

    // minInputSize for each element type (from ElementConstraints):
    const int inCount =
        (etype == ET::Led  || etype == ET::Buzzer   || etype == ET::AudioBox ||
         etype == ET::Not  || etype == ET::Node) ? 1 :
        (etype == ET::And  || etype == ET::Or    || etype == ET::Nand ||
         etype == ET::Nor  || etype == ET::Xor   || etype == ET::Xnor ||
         etype == ET::Demux || etype == ET::DLatch || etype == ET::SRLatch) ? 2 :
        (etype == ET::Mux) ? 3 :
        (etype == ET::DFlipFlop || etype == ET::TFlipFlop) ? 4 :
        (etype == ET::JKFlipFlop || etype == ET::SRFlipFlop) ? 5 :
        (etype == ET::Display7) ? 8 :
        (etype == ET::Display14) ? 15 :
        (etype == ET::Display16) ? 17 : 0;
    writePortList(inCount, basePtr);

    // minOutputSize:
    const int outCount =
        (etype == ET::InputSwitch || etype == ET::InputButton ||
         etype == ET::InputGnd   || etype == ET::InputVcc ||
         etype == ET::Clock      || etype == ET::InputSwitch ||
         etype == ET::Mux        || etype == ET::Node) ? 1 :
        (etype == ET::And  || etype == ET::Or    || etype == ET::Nand ||
         etype == ET::Nor  || etype == ET::Xor   || etype == ET::Xnor ||
         etype == ET::Not) ? 1 :
        (etype == ET::InputRotary) ? 2 :
        (etype == ET::DFlipFlop || etype == ET::TFlipFlop || etype == ET::JKFlipFlop ||
         etype == ET::SRFlipFlop || etype == ET::DLatch   || etype == ET::SRLatch) ? 2 :
        (etype == ET::Demux) ? 2 : 0;
    writePortList(outCount, basePtr + 64);

    // appearance names (>=2.7): quint64 count + QString per entry
    // Write 1 entry to exercise loadPixmapAppearanceName (uses bounded string read).
    // IC elements drain appearance names silently; all others store them in
    // m_alternativeAppearances (size >= 1 for all non-IC elements).
    if (VI::hasAppearanceNames(ver)) {
        if (etype != ET::IC && fdp.ConsumeBool()) {
            s << quint64(1);
            s << QString(); // empty name = no override (keeps resource path)
        } else {
            s << quint64(0);
        }
    }

    // InputRotary: currentPort (int) written before lock state (>=3.1)
    if (etype == ET::InputRotary && !VI::hasQMapFormat(ver)) {
        s << fdp.ConsumeIntegral<int>(); // currentPort
    }

    // Element-type-specific fields in old format.
    // Formats exactly match what the element's loadOldFormat reads:
    if (etype == ET::Clock && !VI::hasQMapFormat(ver)) {
        // Clock::load reads: float freq + (if hasLockState) bool m_locked
        s << fdp.ConsumeFloatingPoint<float>();   // float, NOT double
        if (VI::hasLockState(ver))
            s << fdp.ConsumeBool();               // locked — inside clock section
    }
    // InputSwitch reads: bool isOn, then (if hasLockState) bool locked
    if (etype == ET::InputSwitch && !VI::hasQMapFormat(ver)) {
        s << fdp.ConsumeBool(); // isOn — must come BEFORE locked
        if (VI::hasLockState(ver))
            s << fdp.ConsumeBool(); // locked
    }
    // InputButton reads: (if hasLockState) bool locked
    if (etype == ET::InputButton && !VI::hasQMapFormat(ver) && VI::hasLockState(ver)) {
        s << fdp.ConsumeBool(); // locked
    }
    // InputRotary: currentPort already written above; locked comes after
    if (etype == ET::InputRotary && !VI::hasQMapFormat(ver) && VI::hasLockState(ver)) {
        s << fdp.ConsumeBool(); // locked
    }
    static constexpr const char *colors[] = {"White","Red","Green","Blue","Purple","Black"};
    // Display7/14/16: color is a bare QString for v3.1–4.0 (hasLockState && !hasQMapFormat)
    if ((etype == ET::Display7 || etype == ET::Display14 || etype == ET::Display16)
        && !VI::hasQMapFormat(ver) && VI::hasLockState(ver)) {
        s << QString(colors[fdp.ConsumeIntegralInRange<size_t>(0, 5)]);
    }
    // Led: color is a bare QString for v1.1–4.0 (hasClock && !hasQMapFormat)
    if (etype == ET::Led && !VI::hasQMapFormat(ver) && VI::hasClock(ver)) {
        s << QString(colors[fdp.ConsumeIntegralInRange<size_t>(0, 5)]);
    }
    // IC old format (v1.2–4.0): reads a bare QString file path if hasLabels
    // IC::load normalizes the path (strips directory, replaces backslashes),
    // then calls loadFile(path, contextDir) which fails gracefully when not found.
    // Use fuzz-controlled strings to exercise the path normalization code.
    if (etype == ET::IC && !VI::hasQMapFormat(ver) && VI::hasLabels(ver)) {
        const size_t nameLen = fdp.ConsumeIntegralInRange<size_t>(0, 32);
        s << QString::fromStdString(fdp.ConsumeBytesAsString(nameLen));
    }
    if ((etype == ET::Buzzer || etype == ET::AudioBox) && !VI::hasQMapFormat(ver)) {
        // v2.4–4.0 stored audio as a bare QString (note name or file path).
        // Buzzer::loadOldFormat reads: const QString note = readBoundedString(stream)
        // AudioBox::loadOldFormat reads: const QString audio = readBoundedString(stream)
        if (VI::hasAudio(ver)) {
            s << QString(); // empty string — valid (silence / no file)
        }
    }
}

// Build a complete pre-4.1 panda file using the flat binary element format.
QByteArray buildOldPanda(FuzzedDataProvider &fdp)
{
    QByteArray buf;
    QDataStream s(&buf, QIODevice::WriteOnly);
    s.setVersion(QDataStream::Qt_5_12);

    // Pick a version < 4.1 (old format)
    const VersionSpec &vs =
        kOldVersions[fdp.ConsumeIntegralInRange<size_t>(0, std::size(kOldVersions)-1)];
    const QVersionNumber ver(vs.maj, vs.min, vs.patch);

    // WPCF header
    s.writeRawData("WPCF", 4);
    s << QList<int>{vs.maj, vs.min};

    // Pre-4.6: dolphinFileName (QString) if hasDolphinFilename
    if (VersionInfo::hasDolphinFilename(ver))
        s << QString();

    // Pre-4.6: scene rect (QRectF = 4 doubles) if hasSceneRect and not unified
    if (VersionInfo::hasSceneRect(ver) && !VersionInfo::hasUnifiedMetadata(ver))
        s << 0.0 << 0.0 << 800.0 << 600.0;

    // Item stream — Serialization::deserialize() uses while(!stream.atEnd()),
    // NO preceding count field.  Each item starts with the type discriminator.
    // Each element gets a unique basePtr so its ports register in portMap with
    // distinct keys; connections can then reference those ptrs to wire up.
    QList<quint64> outPtrs, inPtrs; // collect port ptrs for connections
    const int elemCount = fdp.ConsumeIntegralInRange<int>(0, 4);
    for (int e = 0; e < elemCount; ++e) {
        const auto etype = kOldFmtTypes[
            fdp.ConsumeIntegralInRange<size_t>(0, std::size(kOldFmtTypes)-1)];
        const quint64 basePtr = static_cast<quint64>(e + 1) * 0x100;
        writeOldElement(s, fdp, etype, ver, basePtr);
        // Track output ptrs (base+64...) and input ptrs (base+0...) for connections
        outPtrs.append(basePtr + 64);
        inPtrs.append(basePtr);
    }

    // Connections: pre-4.7 format is two bare quint64 port IDs (no QMap).
    // Mix valid ptrs (exercises portMap.contains() → true path and actual wiring)
    // with random IDs (exercises the miss path).
    const int connCount = fdp.ConsumeIntegralInRange<int>(0, 3);
    for (int c = 0; c < connCount; ++c) {
        s << static_cast<int>(65538); // QNEConnection::Type = UserType + 2
        const bool useValid = fdp.ConsumeBool() && !outPtrs.isEmpty() && !inPtrs.isEmpty();
        if (useValid) {
            s << outPtrs[fdp.ConsumeIntegralInRange<size_t>(0, static_cast<size_t>(outPtrs.size()) - 1)];
            s << inPtrs[fdp.ConsumeIntegralInRange<size_t>(0, static_cast<size_t>(inPtrs.size()) - 1)];
        } else {
            s << fdp.ConsumeIntegral<quint64>();
            s << fdp.ConsumeIntegral<quint64>();
        }
    }

    return buf;
}

} // namespace

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_IM_MODULES", "none");

    Comment::setVerbosity(-1);
    Application::interactiveMode = false;
    Application::migrationEnabled = false;

    g_argc = *argc;
    g_argv = *argv;
    g_app  = new QApplication(g_argc, g_argv);

    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    // Drain Qt's deferred-delete queue on every exit path.  Audio elements
    // (Buzzer, AudioBox) queue DeferredDelete events for the audio backend's
    // internal QObject helpers when destroyed; without spinning the event
    // loop, those helpers stay alive and hold PipeWire stream FDs open
    // across iterations, exhausting the audio subsystem after ~1000 inputs.
    auto drainQtEvents = qScopeGuard([] {
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();
    });

    FuzzedDataProvider fdp(data, size);
    const QByteArray buf = buildOldPanda(fdp);
    QDataStream stream(buf);

    QVersionNumber version;
    try {
        version = Serialization::readPandaHeader(stream);
    } catch (...) {
        return 0;
    }

    WorkSpace workspace;
    try {
        // Pass a real contextDir so that relative appearance paths in loadPixmapAppearanceName
        // take the "contextDir + "/" + name" branch (line 577 of GraphicElementSerializer.cpp),
        // which is dark when contextDir is empty.  The actual files do not need to exist —
        // loadPixmapAppearanceName only stores the path string; it does not open the file.
        workspace.load(stream, version, QDir::tempPath());
    } catch (...) {}

    // Run simulation ticks.  Old-format files commonly contain Clock elements
    // (added in V1.1), so start() + update() exercises Clock::updateClock which
    // was dark in all other harnesses.
    auto *sim = workspace.scene()->simulation();
    sim->start();
    sim->update();
    sim->stop();
    sim->update();

    return 0;
}
