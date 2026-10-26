// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief libFuzzer harness for file-backed IC loading (IC::loadFromFile path).
 *
 * fuzz_ic_blob exercises IC::loadFromBlob (embedded blob path).
 * The file-backed path — IC::loadFromFile → IC::loadFileDirectly — is completely
 * separate and dark: it opens a QFile, reads a panda stream, handles cycle
 * detection, and calls IC::migrateFile on old-format sub-circuits.
 *
 * This harness writes the fuzz bytes to a file in a temporary directory that
 * also holds every .panda file of the repository's examples and test circuits
 * (read-only), so the IC files the input names resolve, then loads a workspace
 * panda stream whose IC element's `name` property points to the input's file.
 * When the IC element is loaded and the blob registry has no matching entry,
 * the loader falls through to IC::loadFromFile, which exercises the
 * file-backed path end-to-end; half the inputs load with migration on, as the
 * application does, half of those from a read-only file.  A circuit that loads
 * is reloaded through the registry's cache, simulated and saved.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QDirIterator>
#include <QEvent>
#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QIODevice>
#include <QMap>
#include <QPointF>
#include <QScopeGuard>
#include <QString>
#include <QTemporaryDir>
#include <QVariant>
#include <QVersionNumber>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "Tests/Fuzz/PandaMutator.h"

namespace {

QApplication *g_app  = nullptr;
int  g_argc = 0;
char **g_argv = nullptr;

// The context directory: every .panda file of the repository's examples and test circuits, under its base name as
// references resolve, read-only so a migration cannot rewrite them between inputs; the input is written beside them.
QTemporaryDir *g_dir = nullptr;

void copyDependencies(const QString &root)
{
    // Earlier directories win a base name: the examples and integration circuits are current, the backward
    // compatibility ones each version's copy
    for (const char *subdir : {"Examples", "Tests/Integration", "Tests/Fixtures", "Tests/BackwardCompatibility"}) {
        QDirIterator it(root + "/" + subdir, {"*.panda"}, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QFileInfo source(it.next());
            const QString target = g_dir->filePath(source.fileName());
            if (!QFileInfo::exists(target) && QFile::copy(source.absoluteFilePath(), target)) {
                QFile::setPermissions(target, QFileDevice::ReadOwner | QFileDevice::ReadUser);
            }
        }
    }
}

// Build an outer panda file referencing an IC by filename (not blob name).
// When loaded, the IC element will have no matching blob in the registry,
// causing it to fall through to IC::loadFromFile(contextDir).
QByteArray buildOuterPanda(const QString &icFileName)
{
    QByteArray buf;
    QDataStream s(&buf, QIODevice::WriteOnly);
    s.setVersion(QDataStream::Qt_5_12);

    // v5.1 header
    s.writeRawData("WPCF", 4);
    s << QList<int>{5, 1};

    // Metadata: empty (no embeddedICs → IC must load from file)
    QMap<QString, QVariant> meta;
    meta.insert("dolphinFileName", QVariant(QString()));
    s << meta;

    // One IC element whose file name references the temp file (no blob registry entry
    // → IC falls through to IC::loadFromFile which opens the temp file).
    //
    // Correct format (NO count header — Serialization::deserialize uses while(!atEnd)):
    //   int  GraphicElement::Type = 65539
    //   u64  ElementType::IC = 22
    //   [1] base map (pos, rotation, label)
    //   [2] port lists × 3 (each: quint32 count=0)
    //   [3] IC secondary map with "name" key → triggers file-backed load path
    s << static_cast<int>(65539);              // GraphicElement::Type discriminator
    s << static_cast<quint64>(22);             // ElementType::IC

    // [1] Base map (GraphicElement::loadNewFormat reads pos/rotation/label/etc.)
    QMap<QString, QVariant> baseProps;
    baseProps.insert("pos",      QVariant::fromValue(QPointF(100.0, 100.0)));
    baseProps.insert("rotation", QVariant(0.0));
    baseProps.insert("label",    QVariant(QString()));
    s << baseProps;

    // [2] Port lists (empty)
    s << quint32(0) << quint32(0) << quint32(0);

    // [3] IC secondary map — IC::load reads this after the base load()
    // "name" key with the file's basename triggers IC::loadFromFile(contextDir)
    QMap<QString, QVariant> icProps;
    icProps.insert("name", QVariant(icFileName));
    s << icProps;

    return buf;
}

} // namespace

// The raw path's input is the IC file itself, a whole .panda file: the shared structure-aware mutator works on a
// current-format file's decompressed body (Tests/Fuzz/PandaMutator.h)
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

    // The harness binary sits in the build directory, one level below the repository
    g_dir = new QTemporaryDir();
    copyDependencies(QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(".."));
    if (QDir(g_dir->path()).entryList({"*.panda"}, QDir::Files).isEmpty()) {
        std::fprintf(stderr, "fuzz_ic_file: no .panda file found to depend on beside %s\n",
                     qPrintable(QCoreApplication::applicationDirPath()));
        std::abort();
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

    if (size == 0) return 0;

    // The input is the IC's file, its references resolving to the dependencies beside it
    const QString inputPath = g_dir->filePath("fuzz_input.panda");
    QFile::setPermissions(inputPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser
                                         | QFileDevice::WriteUser);
    {
        QFile file(inputPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return 0;
        file.write(reinterpret_cast<const char *>(data), static_cast<qint64>(size));
    }

    // Half the inputs load as the application does, migrating an older file in place (a read-only dependency
    // refuses its migration, so the dependencies stay as copied). Half of those are read-only themselves, as on a
    // read-only share: the migration refuses to write, and the load unwinds with every item it parsed still held.
    Application::migrationEnabled = ((data[size - 1] & 1) != 0);
    auto restoreMigration = qScopeGuard([] { Application::migrationEnabled = false; });
    if ((data[size - 1] & 2) != 0) {
        QFile::setPermissions(inputPath, QFileDevice::ReadOwner | QFileDevice::ReadUser);
    }

    // Build and load the outer panda that references the input's file.
    const QByteArray outer = buildOuterPanda(QFileInfo(inputPath).fileName());
    QDataStream stream(outer);

    QVersionNumber version;
    try {
        version = Serialization::readPandaHeader(stream);
    } catch (...) {
        return 0;
    }

    WorkSpace ws;
    try {
        ws.load(stream, version, g_dir->path()); // contextDir → IC::loadFromFile uses it
    } catch (...) {
        return 0;
    }

    // Reloaded in its scene, as when its file changes: through the registry's cache of the file's bytes. Only a small
    // one: the reload is a second whole load, and a large circuit's two pass the round's timeout under ASan
    // (Tests/Fixtures/Perf/ic_heavy.panda, 27 KB, 47 s), spending the run in loading, which the first load covers.
    if (size <= 16384) {
        for (auto *elm : ws.scene()->elements()) {
            if (auto *ic = qobject_cast<IC *>(elm)) {
                try {
                    ic->loadFile(inputPath, g_dir->path());
                } catch (...) {}
            }
        }
    }

    // The loaded IC simulated and saved, as a circuit using it is
    ws.scene()->simulation()->update();
    ws.scene()->simulation()->update();
    QByteArray saved;
    QDataStream out(&saved, QIODevice::WriteOnly);
    Serialization::writePandaHeader(out);
    ws.save(out);

    return 0;
}
