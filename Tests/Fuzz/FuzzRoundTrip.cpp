// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief libFuzzer harness exercising the serialize (save) path and round-trip consistency.
 *
 * All other harnesses only fuzz the LOAD side.  Serialization::serialize and the
 * element save() methods are entirely dark.  This harness:
 *
 *   1. Loads a panda stream (same as fuzz_deserialize), beside the
 *      repository's circuits so the IC files it names resolve; every load
 *      below takes the same directory.
 *   2. Saves it through WorkSpace::save(), as File > Save writes it — every
 *      element and connection save(), the blob registry, the pin metadata and
 *      the file-backed IC list.
 *   3. Loads the saved bytes back into a second WorkSpace, whose elements must
 *      keep their count, types and port counts.
 *   4. Saves that, loads it into a third and saves again: the application's own
 *      file must save to the same bytes, so a field written but not read back,
 *      or read into another, traps.  The first save is not compared, since a
 *      field read from an older format can change form once written in the
 *      current one.
 */

#include <cstddef>
#include <cstdint>
#include <memory>

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QEvent>
#include <QList>
#include <QScopeGuard>
#include <QTemporaryDir>
#include <QVersionNumber>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/Enums.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/Serialization.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "Tests/Fuzz/FuzzCircuitDir.h"
#include "Tests/Fuzz/PandaMutator.h"

namespace {

QApplication *g_app  = nullptr;
int  g_argc = 0;
char **g_argv = nullptr;
// The context directory: the repository's circuits (Tests/Fuzz/FuzzCircuitDir.h), so the IC files an input names load
// Held until exit, which removes it
std::unique_ptr<QTemporaryDir> g_dir;

// The circuit is loaded three times and saved three times; one loading more elements than this, nested ICs' included,
// would take a unit past about 10 s under ASan and is only loaded
constexpr qsizetype kMaxCircuitElements = 1500;

} // namespace

// The input is a whole .panda file: the shared structure-aware mutator works on a current-format file's decompressed
// body (Tests/Fuzz/PandaMutator.h)
extern "C" size_t LLVMFuzzerCustomMutator(uint8_t *Data, size_t Size, size_t MaxSize, unsigned int Seed)
{
    return PandaMutator::mutate(Data, Size, MaxSize, Seed);
}

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_IM_MODULES", "none");

    Comment::setVerbosity(-1);
    Application::interactiveMode = false;
    Application::renderingEnabled = false;
    Application::migrationEnabled = false;

    g_argc = *argc;
    g_argv = *argv;
    g_app  = new QApplication(g_argc, g_argv);

    g_dir = std::make_unique<QTemporaryDir>();
    FuzzCircuitDir::fill(g_dir->path(), "fuzz_round_trip");

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

    // --- Phase 1: load the fuzz input ---
    const QByteArray buffer = QByteArray::fromRawData(
        reinterpret_cast<const char *>(data), static_cast<int>(size));
    QDataStream stream(buffer);

    QVersionNumber version;
    try {
        version = Serialization::readPandaHeader(stream);
    } catch (...) {
        return 0;
    }

    WorkSpace ws1;
    try {
        ws1.load(stream, version, g_dir->path());
    } catch (...) {
        return 0;
    }
    if (FuzzCircuitDir::loadedElementCount(ws1.scene()->elements()) > kMaxCircuitElements) {
        return 0;
    }

    // Run two simulation ticks on ws1 to exercise Simulation::update() paths
    // (topologicalSort, iterative settling, connection/output port updates).
    ws1.scene()->simulation()->update();
    ws1.scene()->simulation()->update();

    // --- Phase 2: save the loaded scene, as File > Save writes it ---
    const auto save = [](WorkSpace &ws) {
        QByteArray bytes;
        QDataStream out(&bytes, QIODevice::WriteOnly);
        Serialization::writePandaHeader(out);
        ws.save(out);
        return bytes;
    };
    // No file in the corpus names a waveform; half the circuits are given one, as beWavedDolphin's save does
    if (ws1.dolphinFileName().isEmpty() && ((size % 2) == 1)) {
        ws1.setDolphinFileName("wave.dolphin");
    }
    const QByteArray saved = save(ws1);

    // --- Phase 3: reload the serialised output ---
    // Any crash here is a save/load asymmetry — the serializer wrote something
    // the deserializer can't handle.
    QDataStream stream2(saved);
    QVersionNumber version2;
    try {
        version2 = Serialization::readPandaHeader(stream2);
    } catch (...) {
        return 0;
    }

    WorkSpace ws2;
    try {
        ws2.load(stream2, version2, g_dir->path());
    } catch (...) {
        return 0;
    }

    // Verify element count AND element types are preserved across the round-trip.
    // Any asymmetry — count mismatch or type substitution — is a serialization bug.
    // Connections are excluded (element-only check) because dangling connections
    // are intentionally dropped during clean-up.
    const auto elems1 = ws1.scene()->elements();
    const auto elems2 = ws2.scene()->elements();

    if (elems1.isEmpty()) return 0;

    if (elems2.size() != elems1.size()) {
        __builtin_trap(); // element count changed across round-trip
    }

    for (int i = 0; i < elems1.size(); ++i) {
        if (elems2[i]->elementType() != elems1[i]->elementType()) {
            __builtin_trap(); // element type changed across round-trip
        }
        // Port count must be preserved (serializer must write the correct port list).
        // IC elements are excluded: their port count is rebuilt from the blob.
        if (elems1[i]->elementType() != ElementType::IC) {
            if (elems2[i]->inputSize() != elems1[i]->inputSize() ||
                elems2[i]->outputSize() != elems1[i]->outputSize()) {
                __builtin_trap(); // port count changed across round-trip
            }
        }
    }

    // --- Phase 4: a file the application wrote reads back and saves to the same bytes ---
    // A field the save writes and the load drops, or reads into another, changes the next save. The first save is
    // not compared with the second: a field read from an older format can take another form once written in the
    // current one (an old file's empty pin labels come back null), and only then is it the application's own.
    const QByteArray saved2 = save(ws2);
    QDataStream stream3(saved2);
    WorkSpace ws3;
    try {
        const QVersionNumber version3 = Serialization::readPandaHeader(stream3);
        ws3.load(stream3, version3, g_dir->path());
    } catch (...) {
        __builtin_trap(); // the application's own save does not load
    }
    if (save(ws3) != saved2) {
        __builtin_trap(); // the application's own save changed across a load
    }

    return 0;
}
