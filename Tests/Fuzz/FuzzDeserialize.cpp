// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief libFuzzer harness for the .panda deserializer.
 *
 * Drives the same code path as File > Open: header read, embedded-IC blob
 * registry parsing, the element/connection stream loop in
 * Serialization::deserialize(), and the Scene::addItem() side effects that
 * follow.  An input loads beside the repository's circuits, so the IC files
 * it names resolve as they do for a file opened from its folder.  Run under
 * ASan + UBSan to catch the H2/HC family of bugs on inputs no human would
 * hand-write.  See Tests/Fuzz/README.md for build and run instructions.
 *
 * Custom mutator (LLVMFuzzerCustomMutator): the structure-aware .panda
 * mutator in PandaMutator.h, which keeps the corpus parseable, the current
 * compressed format included.
 */

#include <cstddef>
#include <cstdint>

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QEvent>
#include <QScopeGuard>
#include <QString>
#include <QTemporaryDir>
#include <QVersionNumber>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "Tests/Fuzz/FuzzCircuitDir.h"
#include "Tests/Fuzz/PandaMutator.h"

namespace {

QApplication *g_app = nullptr;
int  g_fuzzer_argc = 0;
char **g_fuzzer_argv = nullptr;
// The context directory: the repository's circuits (Tests/Fuzz/FuzzCircuitDir.h), so the IC files an input names load
QTemporaryDir *g_dir = nullptr;

} // namespace

// libFuzzer calls this to mutate one corpus entry (Tests/Fuzz/PandaMutator.h)
extern "C" size_t LLVMFuzzerCustomMutator(
    uint8_t *Data, size_t Size, size_t MaxSize, unsigned int Seed)
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

    g_fuzzer_argc = *argc;
    g_fuzzer_argv = *argv;
    g_app = new QApplication(g_fuzzer_argc, g_fuzzer_argv);

    g_dir = new QTemporaryDir();
    FuzzCircuitDir::fill(g_dir->path(), "fuzz_deserialize");

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

    const QByteArray buffer = QByteArray::fromRawData(reinterpret_cast<const char *>(data),
                                                      static_cast<int>(size));
    QDataStream stream(buffer);

    QVersionNumber version;
    try {
        version = Serialization::readPandaHeader(stream);
    } catch (...) {
        return 0;
    }

    WorkSpace workspace;
    try {
        workspace.load(stream, version, g_dir->path());
    } catch (...) {}

    workspace.scene()->simulation()->update();

    return 0;
}
