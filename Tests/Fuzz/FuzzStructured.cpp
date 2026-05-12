// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Structure-aware libFuzzer harness for the .panda deserializer.
 *
 * FuzzDeserialize uses raw-byte mutation: the fuzzer edits arbitrary bytes and
 * hopes to accidentally produce valid-looking structure.  For deeply-nested
 * maps and QMap-of-QVariant property bags, random mutation rarely threads the
 * needle far enough to exercise semantic checks.
 *
 * This harness uses FuzzedDataProvider to split the fuzz input into typed
 * fields and synthesise a structurally valid (though semantically arbitrary)
 * .panda stream.  Guaranteed structure means the fuzzer spends its budget on
 * semantic content — property values, port counts, connection wiring — instead
 * of on recovering from broken magic bytes or truncated counts.
 *
 * Coverage target: every element type's load() branch, every VersionInfo
 * predicate branch, metadata map handling, and connection wiring.
 */

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include <QApplication>
#include <QBitArray>
#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QEvent>
#include <QIODevice>
#include <QKeySequence>
#include <QMap>
#include <QPointF>
#include <QScopeGuard>
#include <QString>
#include <QVariant>
#include <QVersionNumber>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/IO/VersionInfo.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationThrottleDisabler.h"
#include "App/Versions.h"

namespace {

QApplication *g_app  = nullptr;
int  g_argc = 0;
char **g_argv = nullptr;

// IC blob built in LLVMFuzzerInitialize using real elements (InputSwitch + Led).
// Using ElementFactory guarantees correct format without hardcoding bytes.
// IC::loadFromBlob exercises processLoadedItems → loadBoundaryElement for both.
QByteArray g_icBlob;

// Nested IC blob: a panda containing one IC element that references g_icBlob.
// Used to exercise 2-level IC nesting and the recursion depth guard in
// IC::deserializeAndLoad (kMaxICNestingDepth limit).
QByteArray g_nestedIcBlob;

// All valid element types including IC (blob provided via g_icBlob, built in LLVMFuzzerInitialize)
constexpr Enums::ElementType kElementTypes[] = {
    Enums::ElementType::And,
    Enums::ElementType::IC,
    Enums::ElementType::AudioBox,
    Enums::ElementType::Buzzer,
    Enums::ElementType::Clock,
    Enums::ElementType::DFlipFlop,
    Enums::ElementType::DLatch,
    Enums::ElementType::Demux,
    Enums::ElementType::Display14,
    Enums::ElementType::Display16,
    Enums::ElementType::Display7,
    Enums::ElementType::InputButton,
    Enums::ElementType::InputGnd,
    Enums::ElementType::InputRotary,
    Enums::ElementType::InputSwitch,
    Enums::ElementType::InputVcc,
    Enums::ElementType::JKFlipFlop,
    Enums::ElementType::JKLatch,
    Enums::ElementType::Led,
    Enums::ElementType::Mux,
    Enums::ElementType::Nand,
    Enums::ElementType::Node,
    Enums::ElementType::Nor,
    Enums::ElementType::Not,
    Enums::ElementType::Or,
    Enums::ElementType::SRFlipFlop,
    Enums::ElementType::SRLatch,
    Enums::ElementType::TFlipFlop,
    Enums::ElementType::Text,
    Enums::ElementType::TruthTable,
    Enums::ElementType::Line,
    Enums::ElementType::Xnor,
    Enums::ElementType::Xor,
};

// Known-valid file format versions to pick from
constexpr std::pair<int,int> kVersions[] = {
    {4,1}, {4,2}, {4,3}, {4,4}, {4,5}, {4,6}, {4,7}, {5,0}, {5,1},
};

// Write a QString in Qt_5_12 wire format (quint32 byte-len + UTF-16-BE)
void writeStr(QDataStream &s, const QString &str) { s << str; }

// Returned by writeElementMap — serialIds registered in portMap for this element.
// Zero means the port list was written with count=0 (no ID registered).
struct PortIds {
    quint64 inputId  = 0;
    quint64 outputId = 0;
};

// Synthesise the two-map structure each element expects:
//
//   [1] Base map    — read by GraphicElement::loadNewFormat()
//                     keys: pos, rotation, label, trigger, flippedX, flippedY
//   [2] Port lists  — 0 or 1 port per list, each with a serialId so that
//                     context.portMap is populated and connections can wire up
//   [3] Secondary map — read by the element's own load() override
//                     keys vary by element type (see below)
//
// baseId: unique serial-ID base for this element (e.g. elemIndex * 4).
// Returns the serialIds actually written (0 = none written for that side).
PortIds writeElementMap(QDataStream &s, FuzzedDataProvider &fdp,
                        Enums::ElementType type, const QVersionNumber &ver,
                        quint64 baseId)
{
    using ET = Enums::ElementType;

    // --- [1] Base map (GraphicElement::loadNewFormat reads these) ---
    QMap<QString, QVariant> base;
    base.insert("pos", QVariant::fromValue(
        QPointF(fdp.ConsumeFloatingPoint<double>(),
                fdp.ConsumeFloatingPoint<double>())));
    base.insert("rotation", QVariant(fdp.ConsumeFloatingPoint<double>()));
    base.insert("label", QVariant(QString::fromStdString(
        fdp.ConsumeBytesAsString(fdp.ConsumeIntegralInRange<size_t>(0, 16)))));
    if (fdp.ConsumeBool()) {
        const int k1 = fdp.ConsumeIntegral<int>();
        const int k2 = fdp.ConsumeBool() ? fdp.ConsumeIntegral<int>() : 0;
        const int k3 = fdp.ConsumeBool() ? fdp.ConsumeIntegral<int>() : 0;
        const int k4 = fdp.ConsumeBool() ? fdp.ConsumeIntegral<int>() : 0;
        base.insert("trigger", QVariant::fromValue(QKeySequence(k1, k2, k3, k4)));
    }
    if (fdp.ConsumeBool()) base.insert("flippedX", QVariant(true));
    if (fdp.ConsumeBool()) base.insert("flippedY", QVariant(true));
    s << base;

    // --- [2] Port lists ---
    // Write 0 or 1 port for each list.  Each port entry is a minimal QMap
    // with a "serialId" key so the port is registered in context.portMap and
    // connections can wire up via portMap.contains(id) → setStartPort/setEndPort.
    //
    // Source-only elements (constant inputs, clocks) have no meaningful input;
    // sink-only elements (LEDs, displays, buzzers) have no meaningful output.
    // We skip those port lists to avoid spurious addPort() calls.
    PortIds ids;

    const bool canHaveInput = (type != ET::InputButton && type != ET::InputSwitch
        && type != ET::InputGnd && type != ET::InputVcc && type != ET::Clock
        && type != ET::Text && type != ET::Line);
    const bool canHaveOutput = (type != ET::Led && type != ET::Display7
        && type != ET::Display14 && type != ET::Display16
        && type != ET::Buzzer && type != ET::AudioBox
        && type != ET::Text && type != ET::Line);

    // Write 0–2 ports per list.  Writing >1 port exercises the port-loop body
    // and addPort() for elements whose actual port count is less than serialized.
    // First port gets a stable serialId so connections can reference it; extra
    // ports get unique IDs that won't collide.  Legacy "ptr" key is included
    // sometimes to exercise oldPtrToSerialId backward-compat mapping.
    const auto writePortList = [&](bool canHave, quint64 firstId) -> quint64 {
        if (!canHave) { s << quint32(0); return 0; }
        const int count = fdp.ConsumeIntegralInRange<int>(0, 2);
        s << static_cast<quint32>(count);
        quint64 registeredId = 0;
        for (int p = 0; p < count; ++p) {
            const quint64 pid = (p == 0) ? firstId : firstId + 200 + static_cast<quint64>(p);
            QMap<QString, QVariant> pm;
            pm.insert("serialId", QVariant::fromValue(pid));
            if (fdp.ConsumeBool())
                pm.insert("ptr", QVariant::fromValue(pid + 100));
            if (type == ET::IC)
                pm.insert("name", QVariant(QString("port%1").arg(p)));
            s << pm;
            if (p == 0) registeredId = pid;
        }
        return registeredId;
    };

    ids.inputId  = writePortList(canHaveInput,  baseId);
    ids.outputId = writePortList(canHaveOutput, baseId + 1); // canHaveOutput declared above

    // All elements have m_alternativeAppearances.size() >= 1, so count=1 is
    // safe for every type.  Writing a skinName entry exercises the appearance
    // loading path in GraphicElement::loadNewFormat (lines 338-342 of
    // GraphicElementSerializer.cpp) which was entirely dark before this.
    if (fdp.ConsumeBool()) {
        s << quint32(1);
        QMap<QString, QVariant> appEntry;
        appEntry.insert("skinName", QVariant(QString::fromStdString(
            fdp.ConsumeBytesAsString(fdp.ConsumeIntegralInRange<size_t>(0, 8)))));
        s << appEntry;
    } else {
        s << quint32(0);
    }

    // --- [3] Element-specific secondary map ---
    // Each element's load() calls readBoundedMetadata() once more after the
    // base load().  Missing this map causes the next type-discriminator bytes
    // to be consumed as a map count → stream desync → everything after throws.
    //
    // All kVersions are 4.1+, so hasQMapFormat is always true here.
    // Node's secondary map is additionally gated on hasWirelessMode (v4.4+).
    static constexpr const char *colors[] = {
        "White","Red","Green","Blue","Purple","Black",
    };
    QMap<QString, QVariant> extra;

    if (type == ET::Clock) {
        extra.insert("frequency", QVariant(fdp.ConsumeFloatingPoint<double>()));
        extra.insert("delay",     QVariant(fdp.ConsumeFloatingPoint<double>()));
        s << extra;
    } else if (type == ET::Buzzer) {
        extra.insert("frequency", QVariant(fdp.ConsumeFloatingPoint<double>()));
        extra.insert("volume",    QVariant(fdp.ConsumeFloatingPoint<double>()));
        s << extra;
    } else if (type == ET::AudioBox) {
        extra.insert("audiobox", QVariant(QString::fromStdString(
            fdp.ConsumeBytesAsString(fdp.ConsumeIntegralInRange<size_t>(0, 8)))));
        extra.insert("volume",   QVariant(fdp.ConsumeFloatingPoint<double>()));
        s << extra;
    } else if (type == ET::InputSwitch) {
        extra.insert("isOn",   QVariant(fdp.ConsumeBool()));
        extra.insert("locked", QVariant(fdp.ConsumeBool()));
        s << extra;
    } else if (type == ET::InputButton) {
        extra.insert("locked", QVariant(fdp.ConsumeBool()));
        s << extra;
    } else if (type == ET::InputRotary) {
        extra.insert("currentPort", QVariant(fdp.ConsumeIntegral<int>()));
        s << extra;
    } else if (type == ET::Led || type == ET::Display7 ||
               type == ET::Display14 || type == ET::Display16) {
        extra.insert("color", QVariant(QString(
            colors[fdp.ConsumeIntegralInRange<int>(0, 5)])));
        s << extra;
    } else if (type == ET::Node && VersionInfo::hasWirelessMode(ver)) {
        // value 3 is out-of-range and exercises the clamp-to-None path
        extra.insert("wirelessMode", QVariant(fdp.ConsumeIntegralInRange<int>(0, 3)));
        s << extra;
    } else if (type == ET::TruthTable && VersionInfo::hasTruthTableData(ver)) {
        const int nbits = fdp.ConsumeIntegralInRange<int>(0, 8);
        QBitArray key(nbits);
        for (int b = 0; b < nbits; ++b) key.setBit(b, fdp.ConsumeBool());
        extra.insert("key", QVariant(key));
        s << extra;
    } else if (type == ET::IC) {
        // IC::load checks "name" → "fileName" → "blobName" in order.
        // Rotate through all three keys; also alternate between the flat blob
        // and the nested blob to exercise 2-level IC loading and the depth guard.
        static const char *icKeys[] = {"blobName", "name", "fileName"};
        const char *key = icKeys[fdp.ConsumeIntegralInRange<size_t>(0, 2)];
        const bool useNested = fdp.ConsumeBool();
        const QString blobName = useNested ? "fuzz_ic_nested.panda" : "fuzz_ic.panda";
        extra.insert(QString::fromLatin1(key), QVariant(blobName));
        s << extra;
    }
    // Elements with no secondary map (And, Or, Not, flip-flops, Mux, Demux,
    // InputGnd, InputVcc, Text, Line, Node pre-v4.4): nothing more to write.
    return ids;
}

// Synthesise a complete .panda byte stream from fuzz input.
QByteArray buildPanda(FuzzedDataProvider &fdp)
{
    QByteArray buf;
    QDataStream s(&buf, QIODevice::WriteOnly);
    s.setVersion(QDataStream::Qt_5_12);

    // --- Header ---
    s.writeRawData("WPCF", 4);
    const auto &[verMaj, verMin] = kVersions[
        fdp.ConsumeIntegralInRange<size_t>(0, std::size(kVersions) - 1)];
    const QVersionNumber ver(verMaj, verMin);
    s << QList<int>{verMaj, verMin};

    // --- Workspace metadata ---
    // V4.6+: one unified QMap<QString,QVariant> (includes dolphinFileName).
    // V4.1–V4.5: positional dolphin QString + QRectF + optional metadata map.
    if (VersionInfo::hasUnifiedMetadata(ver)) {
        // Embed a minimal IC blob so IC elements can successfully loadFromBlob().
        // WorkSpace::load() calls deserializeBlobRegistry on this metadata and
        // populates ICRegistry before any element's load() runs.
        QByteArray regBytes;
        {
            QDataStream regStream(&regBytes, QIODevice::WriteOnly);
            regStream.setVersion(QDataStream::Qt_5_12);
            // Include both blobs: the flat IC and the nested IC.
            // Elements can reference either "fuzz_ic.panda" (1-level) or
            // "fuzz_ic_nested.panda" (2-level, exercises recursion guard).
            QMap<QString, QByteArray> blobMap;
            blobMap.insert("fuzz_ic.panda",        g_icBlob);
            blobMap.insert("fuzz_ic_nested.panda", g_nestedIcBlob);
            regStream << blobMap;
        }
        QMap<QString, QVariant> meta;
        meta.insert("dolphinFileName", QVariant(QString()));
        meta.insert("embeddedICs", QVariant(regBytes));
        // Add fileBackedICs as a QStringList ~50% of the time.
        // WorkSpace::load ignores this key but readBoundedMetadata still parses
        // it via readBoundedVariant → readBoundedStringList, exercising that path.
        if (fdp.ConsumeBool()) {
            const int n = fdp.ConsumeIntegralInRange<int>(0, 2);
            QStringList names;
            for (int i = 0; i < n; ++i) {
                names.append(QString::fromStdString(
                    fdp.ConsumeBytesAsString(fdp.ConsumeIntegralInRange<size_t>(0, 8))));
            }
            meta.insert("fileBackedICs", QVariant(names));
        }
        s << meta;
    } else {
        writeStr(s, QString()); // dolphin filename
        // hasSceneRect is true for all versions >= 1.4, so always write the rect
        s << 0.0 << 0.0 << 800.0 << 600.0; // QRectF (4 doubles)
        if (VersionInfo::hasMetadata(ver)) {
            // v4.5 has metadata but no unified format — add blob registry
            // so IC elements can load even in this version.
            // Pre-v5.1 files use unversioned (Qt default) blob registry bytes.
            QByteArray regBytes;
            {
                QDataStream regStream(&regBytes, QIODevice::WriteOnly);
                QMap<QString, QByteArray> blobMap;
                blobMap.insert("fuzz_ic.panda", g_icBlob);
                regStream << blobMap;
            }
            QMap<QString, QVariant> meta;
            meta.insert("embeddedICs", QVariant(regBytes));
            s << meta;
        }
    }

    // --- Item stream ---
    // Serialization::deserialize() reads items in a while(!stream.atEnd()) loop
    // with NO preceding count field.  Each item starts with an int type tag:
    //   GraphicElement::Type = QGraphicsItem::UserType + 3 = 65539
    //   QNEConnection::Type  = QGraphicsItem::UserType + 2 = 65538
    // After the element type tag, ElementType is written as quint64 via
    // operator<<(QDataStream&, const ElementType&).

    // Collect output and input serialIds so connections can reference real ports.
    // Using known IDs makes portMap.contains() return true → setStartPort/setEndPort
    // is reached, exercising the full connection-wiring path.
    QList<quint64> outputIds, inputIds;

    const int elemCount = fdp.ConsumeIntegralInRange<int>(0, 8);
    for (int e = 0; e < elemCount; ++e) {
        s << static_cast<int>(QGraphicsItem::UserType + 3); // GraphicElement::Type

        const auto etype = kElementTypes[
            fdp.ConsumeIntegralInRange<size_t>(0, std::size(kElementTypes) - 1)];
        s << etype; // ElementType::operator<< writes quint64

        // baseId: 4 IDs per element (2 used: input = base, output = base+1)
        const quint64 baseId = static_cast<quint64>(e) * 4 + 1; // start at 1, never 0
        const PortIds ids = writeElementMap(s, fdp, etype, ver, baseId);
        if (ids.outputId) outputIds.append(ids.outputId);
        if (ids.inputId)  inputIds.append(ids.inputId);
    }

    // Connections: mix of valid (known-good port IDs) and fuzz-controlled.
    // Valid connections exercise setStartPort/setEndPort and updatePosFromPorts.
    const int connCount = fdp.ConsumeIntegralInRange<int>(0, 4);
    for (int c = 0; c < connCount; ++c) {
        s << static_cast<int>(QGraphicsItem::UserType + 2); // QNEConnection::Type

        // Prefer valid port IDs when available; fall back to fuzz-controlled.
        const bool useValidIds = fdp.ConsumeBool()
                                 && !outputIds.isEmpty() && !inputIds.isEmpty();
        quint64 startId, endId;
        if (useValidIds) {
            startId = outputIds[fdp.ConsumeIntegralInRange<size_t>(
                0, static_cast<size_t>(outputIds.size()) - 1)];
            endId = inputIds[fdp.ConsumeIntegralInRange<size_t>(
                0, static_cast<size_t>(inputIds.size()) - 1)];
        } else {
            startId = fdp.ConsumeIntegral<quint64>();
            endId   = fdp.ConsumeIntegral<quint64>();
        }

        if (VersionInfo::hasConnectionQMap(ver)) {
            QMap<QString, QVariant> connMap;
            connMap.insert("startPortId", QVariant::fromValue(startId));
            connMap.insert("endPortId",   QVariant::fromValue(endId));
            s << connMap;
        } else {
            s << startId << endId;
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

    // Build a valid IC blob using actual elements so IC::loadBoundaryElement is
    // exercised when FuzzStructured generates IC elements.  ElementFactory works
    // after QApplication is constructed.
    //
    // The blob is a v5.1 panda containing InputSwitch (id=1, boundary input)
    // and Led (id=2, boundary output).  IC::processLoadedItems classifies them
    // as input/output boundaries and calls loadBoundaryElement for each.
    {
        auto *sw  = ElementFactory::buildElement(ElementType::InputSwitch);
        auto *led = ElementFactory::buildElement(ElementType::Led);
        sw->setId(1);
        led->setId(2);

        QList<QGraphicsItem *> items;
        items.append(sw);
        items.append(led);

        g_icBlob.clear();
        QDataStream s(&g_icBlob, QIODevice::WriteOnly);
        Serialization::writePandaHeader(s);

        QMap<QString, QVariant> meta;
        meta.insert("dolphinFileName", QVariant(QString()));
        s << meta;

        Serialization::serialize(items, s);

        qDeleteAll(items);
    }

    // Build g_nestedIcBlob: a panda that itself contains one IC element referencing
    // "fuzz_ic.panda" (which is g_icBlob).  This creates a 2-level nesting:
    //   outer panda → IC("fuzz_ic_nested.panda") → IC("fuzz_ic.panda") → InputSwitch+Led
    // Exercises IC::deserializeAndLoad recursion guard (depth stays at 2 < 16).
    {
        // Serialise g_icBlob into the nested blob's embeddedICs map
        QByteArray innerRegBytes;
        {
            QDataStream rs(&innerRegBytes, QIODevice::WriteOnly);
            rs.setVersion(QDataStream::Qt_5_12);
            QMap<QString, QByteArray> reg;
            reg.insert("fuzz_ic.panda", g_icBlob);
            rs << reg;
        }

        g_nestedIcBlob.clear();
        QDataStream ns(&g_nestedIcBlob, QIODevice::WriteOnly);
        Serialization::writePandaHeader(ns);

        QMap<QString, QVariant> meta;
        meta.insert("dolphinFileName", QVariant(QString()));
        meta.insert("embeddedICs",     QVariant(innerRegBytes));
        ns << meta;

        // One IC element in the nested blob referencing "fuzz_ic.panda" (= g_icBlob).
        // setBlobName() makes isEmbedded() return true → IC::save() writes the blob name.
        auto *ic = static_cast<IC *>(ElementFactory::buildElement(ElementType::IC));
        ic->setId(1);
        ic->setBlobName("fuzz_ic.panda");
        QList<QGraphicsItem *> icList;
        icList.append(ic);
        Serialization::serialize(icList, ns);
        qDeleteAll(icList);
    }

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

    // Reserve the last min(512, size/4) bytes as fuzz-controlled IC blob content.
    // When small, use the built-in g_icBlob (guaranteed valid); otherwise let the
    // fuzzer mutate the blob bytes to discover what makes a valid IC sub-circuit.
    // This teaches the fuzzer the IC blob format incrementally via corpus evolution.
    const size_t icBlobSize = (size >= 128) ? std::min<size_t>(512, size / 4) : 0;
    const size_t pandaSize  = size - icBlobSize;

    // Replace g_icBlob with fuzz bytes ~50% of the time when enough input is available
    QByteArray savedBlob;
    if (icBlobSize > 0 && (data[0] & 1)) {
        savedBlob = g_icBlob; // preserve for restore
        g_icBlob  = QByteArray(reinterpret_cast<const char *>(data + pandaSize),
                               static_cast<int>(icBlobSize));
    }

    FuzzedDataProvider fdp(data, pandaSize);
    const QByteArray buf  = buildPanda(fdp);

    // Restore built-in blob if we temporarily replaced it
    if (!savedBlob.isEmpty()) {
        g_icBlob = std::move(savedBlob);
    }

    QDataStream stream(buf);

    QVersionNumber version;
    try {
        version = Serialization::readPandaHeader(stream);
    } catch (...) {
        return 0;
    }

    WorkSpace workspace;
    try {
        workspace.load(stream, version, QString());
    } catch (...) {}

    // Run simulation ticks to exercise Simulation.cpp paths that are dark when
    // the timer never fires.  Application::interactiveMode=false ensures visual
    // update phases 3-4 also execute each tick.
    //
    // start() sets m_timer.isActive()=true so the clock-update branch (lines
    // 67–73) runs on the next update().  A manual update() call is used instead
    // of waiting for the real timer (which would require QEventLoop processing
    // and unpredictable tick counts).  stop() then halts the timer.
    auto *sim = workspace.scene()->simulation();
    sim->start();   // exercises Simulation::start(), makes m_timer.isActive() true
    sim->update();  // exercises clock-update branch + full simulation loop
    sim->stop();    // exercises Simulation::stop()
    sim->update();  // one more tick with timer stopped (exercises non-clock path)

    // Disable then re-enable visual throttle to cover SimulationThrottleDisabler
    // (which wraps setVisualThrottleEnabled) and the setVisualThrottleEnabled
    // function itself (Simulation.cpp lines 42-47) which was dark.
    {
        SimulationThrottleDisabler disabler(sim);
        // Constructor calls setVisualThrottleEnabled(false); destructor re-enables.
    }

    return 0;
}

