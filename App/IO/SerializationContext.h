// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMap>
#include <QString>
#include <QVersionNumber>

class QNEPort;

/*!
 * @struct CopyOperation
 * @brief Carries the source/destination paths for IC file copy operations during
 *        deserialization (e.g. when pasting or duplicating a circuit that contains ICs).
 */
struct CopyOperation {
    QString srcPath;
    QString destPath;
    bool needed = false;
};

/*!
 * @struct SerializationContext
 * @brief Bundles all per-deserialization state so that load() overrides receive it
 *        through one explicit parameter instead of several globals.
 *
 * contextDir  — directory of the .panda file being loaded; used to resolve
 *               relative IC file paths and relative skin-file paths.
 * portMap     — accumulated port-pointer map built during deserialization.
 * version     — file-format version read from the stream header.
 * copyOperation — non-empty when the load is part of a copy/paste that needs
 *                 to duplicate IC files into the destination directory.
 * oldPtrToSerialId — maps old pointer IDs (from pre-Phase2b files) to new serial IDs
 *                    for backward compatibility.
 */
struct SerializationContext {
    QMap<quint64, QNEPort *> &portMap;
    QVersionNumber version;
    QString contextDir = {};
    CopyOperation copyOperation = {};
    QMap<quint64, quint64> oldPtrToSerialId = {};
    /// Sequential counter used as the element-local basis for port serialId fallback
    /// (V4.1.9–V4.3 files that lack the serialId key). Incremented once per element.
    int nextLocalId = 1;
};
