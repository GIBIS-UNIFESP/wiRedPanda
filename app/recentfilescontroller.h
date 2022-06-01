/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QObject>

/**
 * Class to hold the list of recent open files
 */
class RecentFilesController : public QObject
{
    Q_OBJECT

public:
    explicit RecentFilesController(QObject *parent = nullptr);

    static constexpr int MaxRecentFiles = 10;

    QStringList recentFiles();
    void addRecentFile(const QString &fileName);

signals:
    void recentFilesUpdated();

private:
    void saveRecentFiles();

    QStringList m_files;
};
