// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Recent-files list management with filesystem watching.
 */

#pragma once

#include <QFileSystemWatcher>
#include <QObject>

/**
 * \class RecentFiles
 * \brief Manages the list of recently opened .panda files.
 *
 * \details Files are persisted to QSettings and watched with QFileSystemWatcher
 * so that deleted files are automatically removed from the list.
 * A maximum of \c maxFiles entries is kept.
 */
class RecentFiles : public QObject
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    /// Constructs the recent-files manager with \a parent.
    explicit RecentFiles(QObject *parent = nullptr);

    // --- Access ---

    /**
     * \brief Returns the current list of recent file paths.
     * \return Paths in most-recently-used order.
     */
    QStringList recentFiles();

    /**
     * \brief Prepends \a filePath to the recent-files list and saves it.
     * \param filePath Absolute path of the file just opened or saved.
     */
    void addRecentFile(const QString &filePath);

signals:
    /// Emitted whenever the recent-files list changes (add or file deleted).
    void recentFilesUpdated();

private:
    // --- Helpers ---

    void saveRecentFiles();

    // --- Members ---

    QStringList m_files;
    QFileSystemWatcher m_fileWatcher;
};

