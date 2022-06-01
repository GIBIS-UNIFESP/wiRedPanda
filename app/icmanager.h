/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <QFileSystemWatcher>
#include <QMap>
#include <QObject>

class MainWindow;
class ICPrototype;
class IC;

class ICManager : public QObject
{
    Q_OBJECT

public:
    static ICManager &instance()
    {
        static ICManager instance;
        return instance;
    }

    static ICPrototype *prototype(const QString &fileName);
    static bool loadIC(IC *ic, const QString &fileName);
    static void loadFile(const QString &fileName);
    static void wakeUp();

signals:
    void openIC(const QString &fileName);
    void updatedIC();

private:
    explicit ICManager(QObject *parent = nullptr);

    void loadFile(QString &fileName, const QString &parentFile);
    void setReloadFile(const QString &fileName);

    QFileSystemWatcher m_fileWatcher;
    QList<QString> m_requiresReload;
    QMap<QString, ICPrototype *> m_ics;
};
