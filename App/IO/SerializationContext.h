// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Deserialization/serialization context structs passed through load()/save() call chains.
 */

#pragma once

#include <QHash>
#include <QMap>
#include <QString>
#include <QVersionNumber>

class Port;

/**
 * \enum SerializationPurpose
 * \brief What a serialization operation is for -- shared by both load() and save(),
 *        so the same distinction is never expressed two different, independently
 *        maintained ways.
 *
 * \details Every external-file reference an element holds (an appearance image, a
 * custom audio file, a file-based IC sub-circuit) needs a different representation
 * depending on which of these an operation is:
 * - \c PortableFile — a real, portable \c .panda file on disk (or a blob embedded in
 *   one). Written paths are stripped to a bare filename, since the referenced file is
 *   expected to live alongside the saved file (see \c WorkSpace::save()'s copy step).
 *   Read paths are relative/bare and get resolved against \c contextDir; a stored
 *   resource (":/...") value is NOT trusted on load, since the compiled-in resource it
 *   names may have been renamed/moved by a newer app version -- the element's own
 *   current default is left alone instead.
 * - \c InMemorySnapshot — an ephemeral, in-process byte array that never leaves this
 *   run: undo/redo snapshots, clipboard copy/paste, IC-reload rollback. No
 *   cross-version or cross-machine staleness is possible, so every value -- resource
 *   or not -- round-trips losslessly, verbatim, both ways.
 *
 * There is deliberately no default value anywhere this enum is used: every
 * construction site must say which one it means, so a future caller that forgets
 * simply fails to compile rather than silently inheriting the wrong behavior.
 */
enum class SerializationPurpose {
    PortableFile,
    InMemorySnapshot,
};

/**
 * \struct SerializationContext
 * \brief Bundles all per-deserialization state so that load() overrides receive it
 *        through one explicit parameter instead of several globals.
 *
 * \details
 * - \c portMap — accumulated port-pointer map built during deserialization.
 * - \c version — file-format version read from the stream header.
 * - \c purpose — see SerializationPurpose; mandatory, no default.
 * - \c contextDir — directory of the .panda file being loaded; used to resolve
 *   relative IC file paths and relative appearance-file paths.
 * - \c oldPtrToSerialId — maps old pointer IDs (from pre-serialId files) to new
 *   serial IDs for backward compatibility.
 */
struct SerializationContext {
    QHash<quint64, Port *> &portMap; ///< Accumulated port-pointer map built during deserialization.
    QVersionNumber version;            ///< File-format version read from the stream header.
    SerializationPurpose purpose;       ///< What this deserialization is for; see SerializationPurpose.
    QString contextDir = {};           ///< Directory of the .panda file (for relative path resolution).
    QHash<quint64, quint64> oldPtrToSerialId = {}; ///< Legacy pointer-ID → serial-ID mapping.
    /// Sequential counter used as the element-local basis for port serialId fallback
    /// (V4.1.9–V4.3 files that lack the serialId key). Incremented once per element.
    int nextLocalId = 1;
    /// Blob registry for resolving embedded IC blobNames during deserialization.
    QMap<QString, QByteArray> *blobRegistry = nullptr;
};

/**
 * \struct SerializationOptions
 * \brief Options passed to GraphicElement::save() (and friends); the save-side
 *        counterpart of SerializationContext, kept separate since save() needs none
 *        of that struct's load-only fields (portMap, version, ...).
 */
struct SerializationOptions {
    SerializationPurpose purpose; ///< What this serialization is for; see SerializationPurpose. Mandatory, no default.
};
