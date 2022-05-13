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
    explicit RecentFilesController(const QString &_attrName, QObject *parent = nullptr, bool saveSetting = true);

    static constexpr int MaxRecentFiles = 10;

    QStringList getRecentFiles();
    void addRecentFile(const QString &fname);

signals:
    void recentFilesUpdated();

private:
    void saveRecentFiles();

    QString m_attrName;
    QStringList m_files;
    bool m_saveSetting;
};

