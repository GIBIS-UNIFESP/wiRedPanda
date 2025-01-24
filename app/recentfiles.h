// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QFileSystemWatcher>
#include <QObject>

/**
 * Class to hold the list of recent open files
 */
class RecentFiles : public QObject
{
    Q_OBJECT

public:
    explicit RecentFiles(QObject *parent = nullptr);

    QStringList recentFiles();
    void addRecentFile(const QString &filePath);

signals:
    void recentFilesUpdated();

private:
    void saveRecentFiles();

    QStringList m_files;
    QFileSystemWatcher m_fileWatcher;
};
