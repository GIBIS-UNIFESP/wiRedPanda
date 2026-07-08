// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Single, shared implementation of how an element's external-file reference
 * (an appearance image, a custom audio file, a file-based IC sub-circuit) gets
 * resolved on load and represented on save. Every class that owns such a reference
 * routes through these functions instead of reimplementing the same path arithmetic --
 * three independent copies of it had already silently drifted apart before this was
 * extracted (see the "avoid this bug class" thread this was written for).
 */

#pragma once

#include <optional>

#include <QDir>
#include <QFileInfo>
#include <QString>

#include "App/IO/SerializationContext.h"

namespace ExternalFilePath {

/// Resolves a stored non-resource path against \a contextDir for loading: tries the
/// path as given (joined with \a contextDir if relative) first; if that doesn't name
/// a plain file, falls back to a bare-filename lookup in the same directory (handles
/// a foreign-OS or stale absolute path saved on a different machine/app version, e.g.
/// a Windows "C:\...\file.ext" opened on Linux). Returns \a storedPath unchanged if
/// it's empty or \a contextDir is empty -- nothing to resolve. Purpose-independent:
/// pure path arithmetic, safe to call regardless of SerializationPurpose.
inline QString resolve(const QString &storedPath, const QString &contextDir)
{
    if (storedPath.isEmpty() || contextDir.isEmpty()) {
        return storedPath;
    }

    const QDir dir(contextDir);
    const QString asGiven = dir.filePath(storedPath);
    if (QFileInfo(asGiven).isFile()) {
        return asGiven;
    }

    return dir.filePath(QFileInfo(QString(storedPath).replace('\\', '/')).fileName());
}

/// Returns how \a path should be written for \a purpose. A resource path (":/...")
/// is never touched. Otherwise: SerializationPurpose::PortableFile strips it to its
/// bare filename (the file is expected to live alongside the saved file -- see
/// WorkSpace::save()'s copy step); SerializationPurpose::InMemorySnapshot writes it
/// verbatim, losslessly.
inline QString forWriting(const QString &path, SerializationPurpose purpose)
{
    if (path.startsWith(":/") || purpose == SerializationPurpose::InMemorySnapshot) {
        return path;
    }
    return QFileInfo(path).fileName();
}

/// Returns the value that should replace an element's current external-file field
/// when restoring \a storedValue, or std::nullopt to mean "leave the current value
/// untouched". For SerializationPurpose::InMemorySnapshot, always returns
/// \a storedValue verbatim (lossless round-trip, no cross-version staleness is
/// possible). For SerializationPurpose::PortableFile, a resource-path \a storedValue
/// returns std::nullopt (a stale saved resource identifier from an older app version
/// must not clobber the element's current compiled-in default); anything else is
/// resolved against \a contextDir via resolve() above.
inline std::optional<QString> forReading(const QString &storedValue, const QString &contextDir, SerializationPurpose purpose)
{
    if (purpose == SerializationPurpose::InMemorySnapshot) {
        return storedValue;
    }

    if (storedValue.startsWith(":/")) {
        return std::nullopt;
    }

    return resolve(storedValue, contextDir);
}

} // namespace ExternalFilePath
