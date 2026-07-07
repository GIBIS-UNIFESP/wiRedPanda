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

} // namespace FileUtils
