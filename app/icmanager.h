/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef ICMANAGER_H
#define ICMANAGER_H

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
    ICManager(MainWindow *mainWindow = nullptr, QObject *parent = nullptr);
    ~ICManager() override;
    void clear();
    bool loadIC(IC *ic, QString fname, const QString &parentFile = "");
    ICPrototype *getPrototype(const QString& fname);
    static ICManager *instance();
    static void setGlobalInstance(ICManager *icManager);
    bool updatePrototypeFilePathName(const QString& sourceName, const QString& targetName);
    void wakeUp();
signals:
    void updatedIC();
    void addRecentIcFile(const QString& fname);

private slots:
    void setReloadFile(const QString& bname);

private:
    bool tryLoadFile(QString &fname, const QString& parentFile);
    void loadFile(QString &fname, const QString& parentFile);

    static ICManager *globalICManager;

    QMap<QString, ICPrototype *> m_ics;
    MainWindow *m_mainWindow;
    QFileSystemWatcher m_fileWatcher;
    QList<QString> requiresReload;
};

#endif // ICMANAGER_H
