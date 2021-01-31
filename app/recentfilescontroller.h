/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef RECENTFILESCONTROLLER_H
#define RECENTFILESCONTROLLER_H

#include <QObject>

class RecentFilesController : public QObject
{
    Q_OBJECT
    QString attrName;

public:
    static constexpr int MaxRecentFiles = 10;
    explicit RecentFilesController(const QString &_attrName, QObject *parent = nullptr)
        : QObject(parent)
        , attrName(_attrName)
    {
    }
    void addFile(const QString &fname);
    QStringList getFiles();

signals:
    void recentFilesUpdated();
};

#endif /* RECENTFILESCONTROLLER_H */