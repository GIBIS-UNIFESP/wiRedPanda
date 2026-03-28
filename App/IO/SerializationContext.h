// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Deserialization context structs passed through load() call chains.
 */

#pragma once

#include <QMap>
#include <QString>
#include <QVersionNumber>

class QNEPort;

/**
 * \struct CopyOperation
 * \brief Carries the source/destination paths for IC file copy operations during
 *        deserialization (e.g. when pasting or duplicating a circuit that contains ICs).
 */
struct CopyOperation {
    QString srcPath;  ///< Source directory of the IC files.
    QString destPath; ///< Destination directory to copy IC files into.
    bool needed = false; ///< True when the deserialization requires IC file copying.
};

/**
 * \struct SerializationContext
 * \brief Bundles all per-deserialization state so that load() overrides receive it
 *        through one explicit parameter instead of several globals.
 *
 * \details
 * - \c contextDir — directory of the .panda file being loaded; used to resolve
 *   relative IC file paths and relative skin-file paths.
 * - \c portMap — accumulated port-pointer map built during deserialization.
 * - \c version — file-format version read from the stream header.
 * - \c copyOperation — non-empty when the load is part of a copy/paste that needs
 *   to duplicate IC files into the destination directory.
 * - \c oldPtrToSerialId — maps old pointer IDs (from pre-serialId files) to new
 *   serial IDs for backward compatibility.
 */
struct SerializationContext {
    QMap<quint64, QNEPort *> &portMap; ///< Accumulated port-pointer map built during deserialization.
    QVersionNumber version;            ///< File-format version read from the stream header.
    QString contextDir = {};           ///< Directory of the .panda file (for relative path resolution).
    CopyOperation copyOperation = {};  ///< IC file copy state (non-empty during paste/duplicate).
    QMap<quint64, quint64> oldPtrToSerialId = {}; ///< Legacy pointer-ID → serial-ID mapping.
    /// Sequential counter used as the element-local basis for port serialId fallback
    /// (V4.1.9–V4.3 files that lack the serialId key). Incremented once per element.
    int nextLocalId = 1;
    /// Blob registry for resolving embedded IC blobNames during deserialization.
    QMap<QString, QByteArray> *blobRegistry = nullptr;
};

