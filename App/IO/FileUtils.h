// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QString>

#include "App/Core/Common.h"

namespace FileUtils {

/// Copies the file at \a srcPath into \a destDir if it does not already exist there.
/// No-op for empty paths, resource paths (":/..."), or files already in \a destDir.
inline void copyToDir(const QString &srcPath, const QString &destDir)
{
    if (srcPath.isEmpty() || srcPath.startsWith(":/") || destDir.isEmpty()) {
        return;
    }

    const QFileInfo srcInfo(srcPath);
    if (!srcInfo.exists()) {
        return;
    }

    if (srcInfo.absoluteDir() == QDir(destDir)) {
        return;
    }

    const QString dest = destDir + "/" + srcInfo.fileName();
    if (!QFile::exists(dest)) {
        QFile srcFile(srcPath);
        if (!srcFile.copy(dest)) {
            throw PANDACEPTION_WITH_CONTEXT("FileUtils", "Could not copy %1 to %2: %3",
                                            srcPath, dest, srcFile.errorString());
        }
    }
}

/// Commits \a file when every write reached it, and returns whether it did. Qt defers a write's failure (a full disk, a
/// quota) to the flush, and Qt 6.7's commit() replaces the target with the truncated file even then; so a failed flush
/// or a recorded error leaves the file uncommitted, and its destructor discards it. errorString() says why.
inline bool commitWhenWritten(QSaveFile &file)
{
    if (!file.flush() || (file.error() != QFileDevice::NoError)) {
        return false;
    }
    return file.commit();
}

/// Returns true if \a a and \a b are the same file on disk or hold identical bytes. Used to
/// tell a harmless "same file, same name" re-add from a genuine name collision (a different
/// file that happens to share a name), which would otherwise be silently mis-bound.
inline bool filesHaveSameContent(const QFileInfo &a, const QFileInfo &b)
{
    const QString canonicalA = a.canonicalFilePath();
    if (!canonicalA.isEmpty() && canonicalA == b.canonicalFilePath()) {
        return true; // literally the same file on disk
    }
    if (a.size() != b.size()) {
        return false;
    }
    QFile fileA(a.absoluteFilePath());
    QFile fileB(b.absoluteFilePath());
    if (!fileA.open(QIODevice::ReadOnly) || !fileB.open(QIODevice::ReadOnly)) {
        return false; // can't confirm identical → treat as different
    }
    return fileA.readAll() == fileB.readAll();
}

} // namespace FileUtils
