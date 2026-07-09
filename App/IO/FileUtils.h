// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDir>
#include <QFile>
#include <QFileInfo>
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
