// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QString>
#include <QStringList>

#include "App/Core/Common.h"
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
        QFile srcFile(srcPath);
        if (!srcFile.copy(dest)) {
            throw PANDACEPTION_WITH_CONTEXT("FileUtils", "Could not copy %1 to %2: %3",
                                            srcPath, dest, srcFile.errorString());
        }
    }
}

/// Recursively copies .panda IC dependencies from \a srcDir to \a destDir
/// by reading the fileBackedICs metadata key from the given \a pandaPath.
/// \a visited tracks already-processed canonical paths to break circular
/// fileBackedICs metadata (A→B→A); the root caller passes nothing.
inline void copyPandaDeps(const QString &pandaPath, const QString &srcDir, const QString &destDir,
                          QSet<QString> *visited = nullptr)
{
    QSet<QString> ownedVisited;
    if (!visited) {
        visited = &ownedVisited;
    }
    const QString canonical = QFileInfo(pandaPath).canonicalFilePath();
    if (canonical.isEmpty() || visited->contains(canonical)) {
        return;
    }
    visited->insert(canonical);

    QFile file(pandaPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    // Probe magic header (same strategy as I-008 / readPandaHeader): reject
    // non-panda content without touching the live stream so readPreamble's
    // status check remains reliable.
    const QByteArray magicBuf = file.peek(sizeof(quint32));
    if (magicBuf.size() < static_cast<int>(sizeof(quint32))) {
        return;
    }
    QDataStream magicProbe(magicBuf);
    magicProbe.setVersion(QDataStream::Qt_5_12);
    quint32 magic = 0;
    magicProbe >> magic;

    if (magic != Serialization::MAGIC_HEADER_CIRCUIT) {
        // May be a legacy "wiRedPanda X.Y" file: the leading 4 bytes encode the
        // byte-length of the QString. Accept only plausible lengths.
        static constexpr quint32 MaxLegacyHeaderBytes = 128;
        if (magic == 0xFFFFFFFFu || magic > MaxLegacyHeaderBytes) {
            return;
        }
        const int totalLen = static_cast<int>(sizeof(quint32) + magic);
        const QByteArray strBuf = file.peek(totalLen);
        if (strBuf.size() < totalLen) {
            return;
        }
        QDataStream strProbe(strBuf);
        strProbe.setVersion(QDataStream::Qt_5_12);
        QString appName;
        strProbe >> appName;
        if (!appName.startsWith("wiRedPanda", Qt::CaseInsensitive)) {
            return;
        }
    }

    QDataStream stream(&file);
    Serialization::Preamble preamble;
    try {
        preamble = Serialization::readPreamble(stream);
    } catch (...) {
        return; // Corrupt or non-panda file — nothing to recurse into
    }
    if (!VersionInfo::hasMetadata(preamble.version)) {
        return;
    }

    const QStringList icFiles = preamble.metadata.value("fileBackedICs").toStringList();
    for (const QString &icFile : icFiles) {
        const QFileInfo icSrc(QDir(srcDir), icFile);
        const QFileInfo icDest(QDir(destDir), icFile);

        if (icSrc.exists() && !QFile::exists(icDest.absoluteFilePath())) {
            QFile srcFile(icSrc.absoluteFilePath());
            if (!srcFile.copy(icDest.absoluteFilePath())) {
                throw PANDACEPTION_WITH_CONTEXT("FileUtils",
                                                "Could not copy %1 to %2: %3",
                                                icSrc.absoluteFilePath(), icDest.absoluteFilePath(),
                                                srcFile.errorString());
            }
        }

        if (icSrc.exists()) {
            copyPandaDeps(icSrc.absoluteFilePath(), srcDir, destDir, visited);
        }
    }
}

} // namespace FileUtils
