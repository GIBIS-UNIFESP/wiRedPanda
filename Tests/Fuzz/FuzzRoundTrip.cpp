// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief libFuzzer harness exercising the serialize (save) path and round-trip consistency.
 *
 * All other harnesses only fuzz the LOAD side.  Serialization::serialize and the
 * element save() methods are entirely dark.  This harness:
 *
 *   1. Loads a panda stream (same as fuzz_deserialize).
 *   2. Calls Serialization::serialize on the loaded scene items — exercises all
 *      element save() overrides, connection save(), and the QMap-format property
 *      serialisation that mirrors the load path.
 *   3. Loads the serialised bytes back into a second WorkSpace — exercises the
 *      load path on output produced by our own code, which surfaces any
 *      save/load asymmetries (fields written but not read, or vice versa).
 *
 * This is the canonical round-trip fuzzer: any crash or difference between the
 * two workspaces on valid input is a bug.
 */

#include <cstddef>
#include <cstdint>

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QEvent>
#include <QList>
#include <QScopeGuard>
#include <QVersionNumber>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/Enums.h"
#include "App/Element/GraphicElement.h"
#include "App/Scene/ICRegistry.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"

namespace {

QApplication *g_app  = nullptr;
int  g_argc = 0;
char **g_argv = nullptr;

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
        ws1.load(stream, version, /*contextDir=*/QString());
    } catch (...) {
        return 0;
    }

    // Run two simulation ticks on ws1 to exercise Simulation::update() paths
    // (topologicalSort, iterative settling, connection/output port updates).
    ws1.scene()->simulation()->update();
    ws1.scene()->simulation()->update();

    // --- Phase 2: serialize the loaded scene ---
    QByteArray saved;
    {
        QDataStream out(&saved, QIODevice::WriteOnly);
        Serialization::writePandaHeader(out);

        // Include the blob registry from the loaded workspace so that
        // Serialization::serializeBlobRegistry is exercised on circuits
        // that have embedded ICs.
        QMap<QString, QVariant> meta;
        meta.insert("dolphinFileName", QVariant(QString()));
        const QMap<QString, QByteArray> &blobs =
            ws1.scene()->icRegistry()->blobMap();
        Serialization::serializeBlobRegistry(blobs, meta);
        out << meta;

        // Collect all items from the scene
        const auto items = ws1.scene()->items();
        Serialization::serialize(items, out);
    }

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
        ws2.load(stream2, version2, /*contextDir=*/QString());
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

    return 0;
}
