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
    void clear();
    bool loadIC(IC *ic, QString fname);
    void loadFile(QString &fname);
    void openIC(const QString &fname);
    ICPrototype *getPrototype(const QString &fname);
    static ICManager *instance();
    static void setGlobalInstance(ICManager *icManager);
    void wakeUp();
signals:
    void updatedIC();

private slots:
    void setReloadFile(const QString &fileName);

private:
    void loadFile(QString &fname, const QString &parentFile);

    static ICManager *globalICManager;

    QMap<QString, ICPrototype *> m_ics;
    MainWindow *m_mainWindow;
    QFileSystemWatcher m_fileWatcher;
    QList<QString> requiresReload;
};

