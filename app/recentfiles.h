// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

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
};
