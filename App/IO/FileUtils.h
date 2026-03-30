// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>

#include "App/IO/Serialization.h"
#include "App/IO/VersionInfo.h"

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
        QFile::copy(srcPath, dest);
    }
}

/// Recursively copies .panda IC dependencies from \a srcDir to \a destDir
/// by reading the fileBackedICs metadata key from the given \a pandaPath.
inline void copyPandaDeps(const QString &pandaPath, const QString &srcDir, const QString &destDir)
{
    QFile file(pandaPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QDataStream stream(&file);
    const auto preamble = Serialization::readPreamble(stream);
    if (!VersionInfo::hasMetadata(preamble.version)) {
        return;
    }

    const QStringList icFiles = preamble.metadata.value("fileBackedICs").toStringList();
    for (const QString &icFile : icFiles) {
        const QFileInfo icSrc(QDir(srcDir), icFile);
        const QFileInfo icDest(QDir(destDir), icFile);

        if (icSrc.exists() && !QFile::exists(icDest.absoluteFilePath())) {
            QFile::copy(icSrc.absoluteFilePath(), icDest.absoluteFilePath());
        }

        if (icSrc.exists()) {
            copyPandaDeps(icSrc.absoluteFilePath(), srcDir, destDir);
        }
    }
}

} // namespace FileUtils

