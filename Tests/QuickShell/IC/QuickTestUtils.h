// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Quick-native, Scene-free replacements for the handful of Tests/Common/TestUtils.h
 * free functions the CPU/Level IC test suite uses (inputStatus/readMultiBitOutput/
 * setMultiBitInput/clockCycle/cpuComponentsDir). TestUtils.h itself pulls in
 * App/Scene/Scene.h/Workspace.h, which don't build -- shared here once instead of
 * re-declaring a local anonymous-namespace copy in every one of this suite's ~85 files.
 */

#pragma once

#include <stdexcept>

#if !defined(Q_OS_WIN)
#include <csignal>
#include <sys/resource.h>
#endif

#include <QImage>
#include <QMap>
#include <QPixmap>
#include <QRandomGenerator>
#include <QSaveFile>
#include <QSet>
#include <QString>
#include <QVariant>
#include <QVector>
#include <QVersionNumber>

#include "App/Core/Application.h"
#include "App/Core/ItemWithId.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"

// Mirrors Tests/Common/TestUtils.h's identical shim: QVERIFY_THROWS(type, expr) didn't exist
// before Qt 6.7 (only the differently-ordered QVERIFY_EXCEPTION_THROWN(expr, type)), and this
// devcontainer's own Qt 6.9.3 doesn't define a plain QVERIFY_THROWS at all -- only
// QVERIFY_THROWS_EXCEPTION.
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
#  define QVERIFY_THROWS(exType, ...) QVERIFY_THROWS_EXCEPTION(exType, __VA_ARGS__)
#else
#  define QVERIFY_THROWS(exType, ...) QVERIFY_EXCEPTION_THROWN(__VA_ARGS__, exType)
#endif

namespace QuickTestUtils {

/// Returns \c true if \a elm's input \a port is Active. Mirrors TestUtils::inputStatus()
/// (its null-port qFatal() guard is dropped -- LCOV_EXCL_LINE'd there as unreachable in any
/// correctly-written test, so a test-author mistake here just crashes on the null deref
/// instead, same observable outcome).
inline bool inputStatus(GraphicElement *elm, int port = 0)
{
    return elm->inputPort(port)->status() == Status::Active;
}

/// Sets each of \a inputs' on/off state from \a value's bits (index 0 = LSB). Mirrors
/// TestUtils::setMultiBitInput().
inline void setMultiBitInput(const QVector<InputSwitch *> &inputs, int value)
{
    for (int i = 0; i < inputs.size(); ++i) {
        const bool bit = (value >> i) & 1;
        inputs[i]->setOn(bit);
    }
}

/// Reads \a elements' input \a port bits (index 0 = LSB) into one combined integer. Mirrors
/// TestUtils::readMultiBitOutput().
template<typename T>
inline int readMultiBitOutput(const QVector<T *> &elements, int port = 0)
{
    int result = 0;
    for (int i = 0; i < elements.size(); ++i) {
        if (inputStatus(static_cast<GraphicElement *>(elements[i]), port)) {
            result |= (1 << i);
        }
    }
    return result;
}

/// Runs a complete clock pulse (rising edge + falling edge) on \a clk, updating \a simulation
/// after each edge. Mirrors TestUtils::clockCycle().
inline void clockCycle(Simulation *simulation, InputSwitch *clk)
{
    clk->setOn(true);  // Rising edge (triggers latch on edge-triggered flip-flops)
    simulation->update();
    clk->setOff();     // Falling edge (return to low for next cycle)
    simulation->update();
}

/// Toggles \a clk's on/off state (single edge) and updates \a simulation. Mirrors
/// TestUtils::clockToggle() -- fine-grained clock control, as opposed to clockCycle()'s
/// full rising+falling pulse.
inline void clockToggle(Simulation *simulation, InputSwitch *clk)
{
    clk->setOn(!clk->isOn());
    simulation->update();
}

/// Initializes a source element (0 inputs, 1 output) for direct-logic testing. Mirrors
/// TestUtils::initSrc().
inline void initSrc(GraphicElement &elm)
{
    elm.initSimulationVectors(0, 1);
}

/// Directory holding the IC .panda fixtures the CPU/Level test suite loads. Mirrors
/// TestUtils::cpuComponentsDir() -- same path, computed the same way (CURRENTDIR is set via
/// target_compile_definitions() on test_wiredpanda, see CMakeLists.txt).
inline QString cpuComponentsDir()
{
#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string
    return QString(QUOTE(CURRENTDIR)) + "/Integration/IC/Components/";
#undef _QUOTE
#undef QUOTE
}

/// Directory holding the shipped example .panda circuits. Mirrors TestUtils::examplesDir()
/// -- same path, computed the same way.
inline QString examplesDir()
{
#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string
    return QString(QUOTE(CURRENTDIR)) + "/../Examples/";
#undef _QUOTE
#undef QUOTE
}

/// Directory holding the versioned backward-compatibility .panda fixtures. Mirrors
/// TestUtils::backwardCompatibilityDir() -- same path, computed the same way.
inline QString backwardCompatibilityDir()
{
#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string
    return QString(QUOTE(CURRENTDIR)) + "/../Tests/BackwardCompatibility/";
#undef _QUOTE
#undef QUOTE
}

/// Directory holding the committed golden SystemVerilog export fixtures. Mirrors
/// TestUtils::systemVerilogExpectedDir() -- same path, computed the same way.
inline QString systemVerilogExpectedDir()
{
#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string
    return QString(QUOTE(CURRENTDIR)) + "/Integration/SystemVerilog/";
#undef _QUOTE
#undef QUOTE
}

/// Directory holding the committed golden Arduino sketch export fixtures. Mirrors
/// TestUtils::arduinoExpectedDir() -- same path, computed the same way.
inline QString arduinoExpectedDir()
{
#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string
    return QString(QUOTE(CURRENTDIR)) + "/Integration/Arduino/";
#undef _QUOTE
#undef QUOTE
}

/// One step of an engine-differential test vector: the inputs driven and the outputs the
/// wiRedPanda engine produced (the oracle). Mirrors TestUtils::DiffStep.
struct DiffStep {
    QVector<int> inputBits;   ///< Value driven on each input switch (caller order).
    QVector<int> expectedOut; ///< Engine output on each LED (caller order) -- the reference.
};

/// Seeded-random differential stimulus driven through the engine oracle. Mirrors
/// TestUtils::generateDifferentialVectors() exactly -- see its own doc comment in
/// Tests/Common/TestUtils.h for the full rationale (reset preambles, settle-then-sample,
/// clock-edge-polarity sampling). Backend-agnostic: the returned steps feed either the
/// SystemVerilog (iverilog) or Arduino (simavr) testbench, which must reproduce
/// \a expectedOut for every step. Deterministic for a given \a seed (uses a local
/// QRandomGenerator, not the global one).
inline QVector<DiffStep> generateDifferentialVectors(
    const QVector<InputSwitch *> &switches,
    const QVector<Led *> &leds,
    Simulation *sim,
    const int clockIdx,
    const QVector<int> &asyncLowIdx,
    const QVector<int> &asyncHighIdx,
    const quint32 seed,
    const int numSteps)
{
    QVector<DiffStep> steps;
    QRandomGenerator rng(seed);
    const QSet<int> asyncSet(asyncLowIdx.cbegin(), asyncLowIdx.cend());
    const QSet<int> asyncHighSet(asyncHighIdx.cbegin(), asyncHighIdx.cend());

    auto captureInputs = [&]() {
        QVector<int> bits;
        bits.reserve(switches.size());
        for (auto *sw : switches) {
            bits.append(sw->isOn() ? 1 : 0);
        }
        return bits;
    };
    auto captureOutputs = [&]() {
        QVector<int> bits;
        bits.reserve(leds.size());
        for (auto *led : leds) {
            bits.append(inputStatus(led, 0) ? 1 : 0);
        }
        return bits;
    };
    auto record = [&]() {
        steps.append({captureInputs(), captureOutputs()});
    };
    // A setup step drives inputs but records no expected output, so the testbench
    // replays it without checking. Used for the reset preamble, which brings a
    // freshly seeded comb-loop latch to a defined state both the engine and the
    // export agree on before the checked stimulus begins.
    auto recordSetup = [&]() {
        steps.append({captureInputs(), {}});
    };
    // Settle the engine to a fixed point after each input change before sampling.
    // A SINGLE update is not enough for gated-clock multi-cycle designs: the
    // combinational path that feeds the flip-flops (cycle-counter -> decode ->
    // enable) needs several passes to converge, and a non-settled D would be
    // captured at the next clock edge, drifting the engine's state one cycle off
    // the exported (non-blocking) SystemVerilog. The committed engine semantics
    // are validated against iverilog only at the settled fixed point, so we
    // reproduce the same multi-pass settle the original stimulus used.
    constexpr int kSettleIterations = 20;
    auto settle = [&]() {
        for (int u = 0; u < kSettleIterations; ++u) {
            sim->update();
        }
    };

    // Initial (unrecorded) settle: clock and data LOW, active-low async controls
    // HIGH (inactive). This matches the testbench's t=0 init.
    for (int i = 0; i < switches.size(); ++i) {
        switches[i]->setOn(asyncSet.contains(i));
    }
    settle();

    // Reset preamble (SETUP, not checked): a cross-coupled flip-flop powers on
    // into a free-running bistable state the engine and a faithful gate-level
    // export resolve to *opposite* values. The export's seeded latch regs start
    // at the metastable (0,0) point, and a *single* async assert can't climb out
    // of it (and a clock pulse there only corrupts it). Asserting BOTH controls
    // at once injects the input events that drive the latch off (0,0); releasing
    // all but one then leaves a single async control asserted, which forces a
    // defined state the engine and export agree on. No clock pulse is needed --
    // async preset/clear set the latch directly. These steps are replayed by the
    // testbench but not checked, so checks begin from the settled state.
    if (!asyncLowIdx.isEmpty()) {
        for (const int idx : asyncLowIdx) {
            switches[idx]->setOn(false);              // assert all (active-low)
        }
        settle();
        recordSetup();
        for (int i = 1; i < asyncLowIdx.size(); ++i) {
            switches[asyncLowIdx[i]]->setOn(true);    // release all but the first
        }
        settle();
        recordSetup();
        switches[asyncLowIdx.first()]->setOn(true);   // release; state now held
        settle();
        recordSetup();
    }

    // Active-high reset/seed preamble (SETUP, not checked): a counter or register
    // exposes an active-HIGH Reset/Init that drives its embedded flip-flops to a
    // defined state (0 for Reset, the seed pattern for Init). Without it the
    // run starts from the flip-flops' ambiguous power-on value, which the engine
    // (settles q=1) and a faithful gate-level export (settles q=0) resolve
    // oppositely. Pulse it HIGH then release so both begin the checked stimulus
    // from the same defined state; it stays inactive (LOW) for the random body.
    if (!asyncHighIdx.isEmpty()) {
        for (const int idx : asyncHighIdx) {
            switches[idx]->setOn(true);               // assert reset/seed
        }
        settle();
        recordSetup();
        for (const int idx : asyncHighIdx) {
            switches[idx]->setOn(false);              // release; state now held
        }
        settle();
        recordSetup();
    }

    for (int s = 0; s < numSteps; ++s) {
        // Randomise non-clock, non-async inputs (stable across the clock pulse so
        // an edge-triggered element captures a defined value).
        for (int i = 0; i < switches.size(); ++i) {
            if (i == clockIdx || asyncSet.contains(i) || asyncHighSet.contains(i)) {
                continue;
            }
            switches[i]->setOn((rng.generate() & 1U) != 0U);
        }
        // Hold async controls inactive during synchronous operation: the preamble
        // already established a defined state, so the random body exercises the
        // clocked data path. (Asserting async mid-run only adds metastable corners
        // where iverilog's comb-loop evaluation and the engine's settle diverge on
        // undefined behaviour -- not export defects.)
        for (const int idx : asyncLowIdx) {
            switches[idx]->setOn(true);
        }
        // Hold active-high reset/seed inactive (LOW) during synchronous operation.
        for (const int idx : asyncHighIdx) {
            switches[idx]->setOn(false);
        }

        if (clockIdx >= 0) {
            // Change data ONLY while the clock is stable (currently LOW), then
            // pulse the clock. This keeps every clock edge -- rising AND falling --
            // free of a simultaneous data change, so an edge-triggered element
            // captures deterministically (no setup-time race between the engine's
            // and the export's evaluation order). Sampling at clock-HIGH also
            // makes the check edge-polarity sensitive.
            settle();
            record();                       // new data, clock LOW
            switches[clockIdx]->setOn(true);
            settle();
            record();                       // rising edge (data stable)
            switches[clockIdx]->setOn(false);
            settle();
            record();                       // falling edge (data stable)
        } else {
            settle();
            record();
        }
    }

    return steps;
}

#if !defined(Q_OS_WIN)
/// Mirrors TestUtils::ScopedTinyFsizeLimit exactly (see its own doc comment there for the
/// full rationale): caps RLIMIT_FSIZE to a few bytes for its lifetime, ignoring SIGXFSZ, so a
/// test can deterministically force a QSaveFile write to fail at commit() without a second OS
/// user, root, or a full disk. POSIX-only; RLIMIT_FSIZE has no Windows equivalent.
struct ScopedTinyFsizeLimit {
    struct rlimit previousLimit {};
    void (*previousHandler)(int) = nullptr;

    ScopedTinyFsizeLimit()
    {
        previousHandler = std::signal(SIGXFSZ, SIG_IGN);
        getrlimit(RLIMIT_FSIZE, &previousLimit);
        struct rlimit tiny = previousLimit;
        tiny.rlim_cur = 16;
        setrlimit(RLIMIT_FSIZE, &tiny);
    }

    ~ScopedTinyFsizeLimit()
    {
        setrlimit(RLIMIT_FSIZE, &previousLimit);
        std::signal(SIGXFSZ, previousHandler);
    }
};
#endif

/// Initializes \a elm's simulation vectors from its own current port counts. Mirrors
/// TestUtils::initElm() -- needed before calling updateLogic() directly on a standalone
/// (not builder-owned) element outside a full simulation.
inline void initElm(GraphicElement &elm)
{
    elm.initSimulationVectors(elm.inputSize(), elm.outputSize());
}

/// Returns true if \a pixmap has any non-transparent pixel. Mirrors TestUtils::pixmapHasInk()
/// -- used to assert that paint() actually drew something.
inline bool pixmapHasInk(const QPixmap &pixmap)
{
    const QImage image = pixmap.toImage();
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (qAlpha(image.pixel(x, y)) != 0) {
                return true;
            }
        }
    }
    return false;
}

/// Writes \a builder's elements, plus every Connection reachable from their ports, as the
/// metadata+payload section of the .panda format (everything AFTER the header, i.e. what
/// WorkSpace::save(QDataStream&) itself writes -- the caller writes the header separately via
/// Serialization::writePandaHeader(), same split WorkSpace::save(path)/autosave() use). Shared
/// core behind savePandaFile() (real file) and savePandaBytes() (in-memory) below. \a
/// dolphinFileName and \a blobRegistry, when non-empty, are embedded exactly as WorkSpace::save()
/// embeds m_dolphinFileName and Scene::icRegistry()->blobMap().
inline void writePandaPayload(QuickCircuitBuilder &builder, QDataStream &stream,
                               const QString &dolphinFileName = {},
                               const QMap<QString, QByteArray> &blobRegistry = {})
{
    const auto elements = builder.elements();

    QMap<QString, QVariant> metadata;
    if (!dolphinFileName.isEmpty()) {
        metadata["dolphinFileName"] = dolphinFileName;
    }
    const auto portMeta = IC::buildPortMetadata(elements);
    if (portMeta.inputCount > 0 || portMeta.outputCount > 0) {
        metadata["inputCount"] = portMeta.inputCount;
        metadata["outputCount"] = portMeta.outputCount;
        metadata["inputLabels"] = portMeta.inputLabels;
        metadata["outputLabels"] = portMeta.outputLabels;
    }
    Serialization::serializeBlobRegistry(blobRegistry, metadata);

    QList<ItemWithId *> items;
    QSet<Connection *> seenConnections;
    for (auto *elm : elements) {
        items.append(elm);
        for (auto *port : elm->inputs()) {
            for (auto *conn : port->connections()) {
                if (!seenConnections.contains(conn)) {
                    seenConnections.insert(conn);
                    items.append(conn);
                }
            }
        }
        for (auto *port : elm->outputs()) {
            for (auto *conn : port->connections()) {
                if (!seenConnections.contains(conn)) {
                    seenConnections.insert(conn);
                    items.append(conn);
                }
            }
        }
    }

    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);
    payloadStream << metadata;
    Serialization::serialize(items, payloadStream, {.purpose = SerializationPurpose::PortableFile});
    Serialization::writePayload(stream, payload);
}

/// Saves \a builder's elements, plus every Connection reachable from their ports, to a real
/// .panda file at \a filePath -- a Scene-free replacement for the WorkSpace::save(path) idiom
/// several IC tests use to build a real sub-circuit file, then load it back via IC::loadFile().
/// Mirrors WorkSpace::save()'s own core (writePandaHeader + port-metadata + compressed
/// element/connection payload) -- the Widgets-dialog/contextDir-copying/undo-stack/autosave
/// machinery around that core is irrelevant to what these tests actually exercise, so it's
/// intentionally left out rather than carried forward. \a blobRegistry, when non-empty, is
/// embedded exactly as WorkSpace::save() embeds Scene::icRegistry()->blobMap() -- for tests
/// that need a saved file to reference an embedded (blob-backed) IC without a real
/// Scene/ICRegistry to own it. Throws std::runtime_error on any I/O failure (open/commit).
inline void savePandaFile(QuickCircuitBuilder &builder, const QString &filePath,
                           const QMap<QString, QByteArray> &blobRegistry = {})
{
    QSaveFile saveFile(filePath);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        throw std::runtime_error(QString("Could not open %1 for writing: %2")
                                      .arg(filePath, saveFile.errorString()).toStdString());
    }

    QDataStream stream(&saveFile);
    Serialization::writePandaHeader(stream);
    writePandaPayload(builder, stream, {}, blobRegistry);

    if (!saveFile.commit()) {
        throw std::runtime_error(QString("Could not save %1: %2")
                                      .arg(filePath, saveFile.errorString()).toStdString());
    }
}

/// In-memory sibling of savePandaFile(): returns a complete .panda byte stream (header +
/// metadata + compressed payload) instead of writing to disk -- a Scene-free replacement for
/// the WorkSpace-based "saveToMemory()" idiom several serialization round-trip tests use.
inline QByteArray savePandaBytes(QuickCircuitBuilder &builder, const QString &dolphinFileName = {},
                                  const QMap<QString, QByteArray> &blobRegistry = {})
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    writePandaPayload(builder, stream, dolphinFileName, blobRegistry);
    return data;
}

/// Result of loadPandaStream()/loadPandaBytes(): the file-level fields WorkSpace itself would
/// otherwise hold (m_dolphinFileName, the embedded-IC blob registry) -- everything else
/// (elements/connections) is added directly to the QuickCircuitBuilder passed in.
struct LoadedPandaFile {
    QString dolphinFileName;
    QMap<QString, QByteArray> blobRegistry;
};

/// Deserializes a .panda-format stream POSITIONED RIGHT AFTER THE HEADER (i.e. \a version is
/// already known, from Serialization::readPandaHeader()) into \a builder -- a Scene-free
/// replacement for the WorkSpace::load(QDataStream&, version, contextDir) idiom. Ownership of
/// every produced GraphicElement transfers to \a builder (addOwnedElement()); Connections need
/// no separate registration (see QuickCircuitBuilder's own class doc comment -- transitively
/// owned by their ports' owning elements). Mirrors WorkSpace::load()'s core (readPayload +
/// readBoundedMetadata/loadDolphinFileName/loadRect + deserializeBlobRegistry + deserialize) --
/// the interactive-mode version-mismatch dialogs, m_lastId bookkeeping, and real-ICRegistry blob
/// population are irrelevant to what these tests exercise (a plain QMap is just as good a blob
/// source for IC::loadFromBlob() as a real ICRegistry's), so intentionally left out.
inline LoadedPandaFile loadPandaStream(QuickCircuitBuilder &builder, QDataStream &stream,
                                        const QVersionNumber &version, const QString &contextDir = {})
{
    LoadedPandaFile result;

    QByteArray payload = Serialization::readPayload(stream, version);
    QDataStream payloadStream(&payload, QIODevice::ReadOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);

    QMap<QString, QVariant> metadata;
    if (VersionInfo::hasUnifiedMetadata(version)) {
        metadata = Serialization::readBoundedMetadata(payloadStream);
        result.dolphinFileName = metadata.value("dolphinFileName").toString();
    } else {
        result.dolphinFileName = Serialization::loadDolphinFileName(payloadStream, version);
        Serialization::loadRect(payloadStream, version);
        if (VersionInfo::hasMetadata(version)) {
            metadata = Serialization::readBoundedMetadata(payloadStream);
        }
    }

    result.blobRegistry = Serialization::deserializeBlobRegistry(metadata, version);

    QHash<quint64, Port *> portMap;
    SerializationContext context{.portMap = portMap, .version = version,
                                  .purpose = SerializationPurpose::PortableFile, .contextDir = contextDir};
    context.blobRegistry = &result.blobRegistry;
    const auto items = Serialization::deserialize(payloadStream, context);

    for (auto *item : items) {
        if (auto *elm = dynamic_cast<GraphicElement *>(item)) {
            builder.addOwnedElement(elm);
        }
    }

    return result;
}

/// In-memory sibling of loadPandaStream(): takes a complete .panda byte stream (as produced by
/// savePandaBytes(), or a real file's contents), reads its own header, and deserializes into
/// \a builder.
inline LoadedPandaFile loadPandaBytes(QuickCircuitBuilder &builder, const QByteArray &data,
                                       const QString &contextDir = {})
{
    QDataStream stream(data);
    const QVersionNumber version = Serialization::readPandaHeader(stream);
    return loadPandaStream(builder, stream, version, contextDir);
}

/// Same idea as TestUtils::ScopedInteractiveMode: Application::interactiveMode is forced
/// false globally for the test suite, so a test that toggles it must restore it even on a
/// failed assertion.
struct ScopedInteractiveMode {
    bool original = Application::interactiveMode;

    ~ScopedInteractiveMode()
    {
        Application::interactiveMode = original;
    }
};

} // namespace QuickTestUtils
