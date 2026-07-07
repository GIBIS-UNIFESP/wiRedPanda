// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief libFuzzer harness for Serialization::copyPandaFile() and readPreamble().
 *
 * All other harnesses exercise the WorkSpace::load() path which skips the
 * preamble-only reader (readPreamble) and the file copy helper (copyPandaFile).
 * These functions are called only during "Save As" operations:
 *
 *   - readPreamble(): reads the full panda preamble (header + dolphin + rect +
 *     metadata) WITHOUT deserialising scene elements.  The metadata QMap is
 *     parsed via readBoundedMetadata, same as load, but the code path through
 *     copyPandaFile is distinct.
 *
 *   - copyPandaFile(): recursively copies a .panda file and its file-backed IC
 *     dependencies (also copying the file itself and the peek-based
 *     plausibility pre-check, both formerly duplicated in the now-removed
 *     FileUtils::copyPandaDeps()).  The recursion guard (QSet<QString>) and
 *     the "fileBackedICs" metadata key parsing are completely dark without an
 *     explicit caller.
 *
 * Strategy:
 *   1. Write fuzz bytes to a QTemporaryFile — this is the "source" panda.
 *   2. Call Serialization::copyPandaFile(src, dest) — exercises readPreamble,
 *      the cycle guard, and the IC-dependency walk.
 *   3. Build a synthetic outer panda whose metadata contains a "fileBackedICs"
 *      entry pointing back to the first temp file — exercises the recursive
 *      copy + cycle detection path.
 */

#include <cstddef>
#include <cstdint>

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QEvent>
#include <QFileInfo>
#include <QIODevice>
#include <QList>
#include <QMap>
#include <QScopeGuard>
#include <QString>
#include <QStringList>
#include <QTemporaryFile>
#include <QVariant>
#include <QVersionNumber>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/IO/Serialization.h"

namespace {

QApplication *g_app  = nullptr;
int  g_argc = 0;
char **g_argv = nullptr;

// Build a minimal v5.1 panda whose metadata has a "fileBackedICs" list
// containing one entry — the basename of icFile.  This causes copyPandaFile
// to recurse into icFile, exercising cycle detection.
QByteArray buildOuterWithFileBackedIC(const QString &icBasename)
{
    QByteArray buf;
    QDataStream s(&buf, QIODevice::WriteOnly);
    s.setVersion(QDataStream::Qt_5_12);

    s.writeRawData("WPCF", 4);
    s << QList<int>{5, 1};

    QMap<QString, QVariant> meta;
    meta.insert("dolphinFileName", QVariant(QString()));
    meta.insert("fileBackedICs",   QVariant(QStringList{icBasename}));
    s << meta;

    // Empty element + connection streams
    s << quint32(0) << quint32(0);

    return buf;
}

} // namespace

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

    // --- Phase 1: write fuzz bytes as a "panda" source file ---
    QTemporaryFile srcFile;
    srcFile.setAutoRemove(true);
    if (!srcFile.open()) return 0;
    srcFile.write(reinterpret_cast<const char *>(data), static_cast<qint64>(size));
    srcFile.flush();
    srcFile.close();

    const QFileInfo srcInfo(srcFile.fileName());
    const QString dir = srcInfo.absolutePath();
    const QString basename = srcInfo.fileName();

    // Phase 1a: direct copy — exercises readPreamble on fuzz-controlled input
    {
        QTemporaryFile destFile;
        destFile.setAutoRemove(true);
        if (destFile.open()) {
            const QFileInfo destInfo(destFile.fileName());
            destFile.close(); // copyPandaFile opens it via QFile::copy
            // Remove pre-existing dest so QFile::copy can succeed
            QFile::remove(destInfo.absoluteFilePath());
            try {
                Serialization::copyPandaFile(srcInfo, destInfo);
            } catch (...) {}
        }
    }

    // --- Phase 2: build an outer panda that references the fuzz file as an IC ---
    // This exercises the recursive copyPandaFile path and the cycle guard.
    const QByteArray outer = buildOuterWithFileBackedIC(basename);

    QTemporaryFile outerFile;
    outerFile.setAutoRemove(true);
    if (!outerFile.open()) return 0;
    outerFile.write(outer);
    outerFile.flush();
    outerFile.close();

    // Place dest in the same directory so the IC lookup resolves correctly
    QTemporaryFile outerDest;
    outerDest.setAutoRemove(true);
    if (outerDest.open()) {
        const QFileInfo outerSrcInfo(outerFile.fileName());
        const QFileInfo outerDestInfo(outerDest.fileName());
        outerDest.close();
        QFile::remove(outerDestInfo.absoluteFilePath());
        try {
            Serialization::copyPandaFile(outerSrcInfo, outerDestInfo);
        } catch (...) {}
    }

    return 0;
}
