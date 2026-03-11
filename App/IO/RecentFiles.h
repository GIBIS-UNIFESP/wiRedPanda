// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QFileSystemWatcher>
#include <QObject>

/*!
 * @class RecentFiles
 * @brief Class to hold the list of recent open files
 */
class RecentFiles : public QObject
{
    Q_OBJECT

public:
    // --- Lifecycle ---

    explicit RecentFiles(QObject *parent = nullptr);

    // --- Access ---

    QStringList recentFiles();
    void addRecentFile(const QString &filePath);

signals:
    // --- Signals ---

    void recentFilesUpdated();

private:
    // --- Helpers ---

    void saveRecentFiles();

    // --- Members ---

    QStringList m_files;
    QFileSystemWatcher m_fileWatcher;
};
