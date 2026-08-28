// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Per-platform resolution of install-relative content directories.
 */

#pragma once

#include <QString>
#include <QStringList>

/**
 * \class InstallRelativePaths
 * \brief Static utility resolving where a named content folder (e.g. "Examples",
 * "Exercises", "Tours") lives relative to the running application's install location.
 *
 * \details Shared by MainWindow::setupExamplesMenu() and ExerciseTourResources so the
 * per-platform candidate list (Windows/dev, macOS app bundle, Linux AppImage, Linux
 * native FHS install, WASM, CWD dev fallback) exists in exactly one place.
 */
class InstallRelativePaths
{
public:
    InstallRelativePaths() = delete;

    /// Builds the ordered list of candidate directories for \a category, relative to
    /// QCoreApplication::applicationDirPath(): Windows/dev builds, macOS app bundle
    /// Resources, Linux AppImage (via $APPDIR), Linux native FHS install
    /// (share/wiredpanda/\<category\> next to a bin/ install), WASM's virtual
    /// filesystem, and finally a bare-category CWD fallback for development.
    static QStringList candidates(const QString &category);

    /// True if \a directory is one of the candidates() entries for \a category, compared by
    /// canonical path so ".."-relative and CWD-relative candidates match correctly. Used to
    /// recognise bundled/installed content directories that must not be written into even when
    /// the filesystem would permit it (a dev checkout's Examples/ is writable, an installed
    /// one usually isn't). Read-only — creates nothing; a directory that doesn't exist is
    /// never a candidate.
    static bool isCandidate(const QString &category, const QString &directory);

    /// Returns the first candidate() entry that already exists on disk, or "" if none do.
    /// Read-only — creates nothing.
    static QString resolve(const QString &category);
};
