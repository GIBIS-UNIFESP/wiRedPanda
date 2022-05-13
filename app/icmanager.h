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
    explicit ICManager(MainWindow *mainWindow = nullptr, QObject *parent = nullptr);
    ~ICManager() override;
    static ICManager *instance();
    static void setGlobalInstance(ICManager *icManager);

    ICPrototype *getPrototype(const QString &fname);
    bool loadIC(IC *ic, QString fname);
    void clear();
    bool loadFile(QString &fname);
    void openIC(const QString &fname);
    void wakeUp();

signals:
    void updatedIC();

private:
    static ICManager *globalICManager;

    void loadFile(QString &fname, const QString &parentFile);
    void setReloadFile(const QString &fileName);

    MainWindow *m_mainWindow;
    QFileSystemWatcher m_fileWatcher;
    QList<QString> requiresReload;
    QMap<QString, ICPrototype *> m_ics;
};

