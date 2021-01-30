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
    QMap<QString, ICPrototype *> ics;
    MainWindow *mainWindow;

    QFileSystemWatcher fileWatcher;

public:
    ICManager(MainWindow *mainWindow = nullptr, QObject *parent = nullptr);
    ~ICManager() override;
    void clear();
    bool loadIC(IC *ic, QString fname, QString parentFile = "");
    ICPrototype *getPrototype(QString fname);
    static ICManager *instance();

    bool updatePrototypeFilePathName(QString sourceName, QString targetName);
signals:
    void updatedIC();

private slots:
    void reloadFile(QString bname);

private:
    bool tryLoadFile(QString &fname, QString parentFile);
    void loadFile(QString &fname, QString parentFile);
    bool warnAboutFileChange(const QString &fileName);

    static ICManager *globalICManager;
    void updateRecentICs(const QString &fname);
};

#endif // ICMANAGER_H
