// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icmanager.h"

#include <QApplication>
#include <qdebug.h>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>

#include "common.h"
#include "filehelper.h"
#include "ic.h"
#include "icnotfoundexception.h"
#include "icprototype.h"
#include "mainwindow.h"

ICManager *ICManager::globalICManager = nullptr;

ICManager::ICManager(MainWindow *mainWindow, QObject *parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , requiresReload()
{
    if (globalICManager == nullptr) {
        globalICManager = this;
    }
    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &ICManager::setReloadFile);
    if (m_mainWindow) {
        connect(this, &ICManager::addRecentIcFile, m_mainWindow, &MainWindow::addRecentIcFile);
    }
}

ICManager::~ICManager()
{
    clear();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    settings.setValue("recentICs", "");

    // fprintf(stderr, "Removing IC manager");
    if (globalICManager == this) {
        globalICManager = nullptr;
    }
}

bool ICManager::tryLoadFile(QString &fname, const QString& parentFile)
{
    try {
        COMMENT("Trying yo loading file " << fname.toStdString() << " with parentfile name: " << parentFile.toStdString(), 3);
        loadFile(fname, parentFile);
    } catch (ICNotFoundException &err) {
        COMMENT("ICNotFoundException thrown: " << err.what(), 0);
        int ret = QMessageBox::warning(m_mainWindow, tr("Error"), QString::fromStdString(err.what()), QMessageBox::Ok, QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel) {
            return false;
        }
        fname = m_mainWindow->getOpenICFile();
        if (fname.isEmpty()) {
            return false;
        }
        return tryLoadFile(fname, parentFile);
    }
    return true;
}

void ICManager::loadFile(QString &fname, const QString& parentFile)
{
    COMMENT("Loading file " << fname.toStdString() << " with parentfile name: " << parentFile.toStdString(), 3);
    QFileInfo finfo = FileHelper::findICFile(fname, parentFile);
    fname = finfo.filePath();
    Q_ASSERT(finfo.exists() && finfo.isFile());
    m_fileWatcher.addPath(finfo.absoluteFilePath());
    if (m_ics.contains(finfo.baseName())) {
        COMMENT("IC already inserted: " << finfo.baseName().toStdString(), 3);
    } else {
        COMMENT("Inserting IC: " << finfo.baseName().toStdString(), 3);
        auto *prototype = new ICPrototype(finfo.absoluteFilePath());
        prototype->reload();
        m_ics.insert(finfo.baseName(), prototype);
    }
}

void ICManager::clear()
{
    COMMENT("Clear ICManager", 3);
    QMap<QString, ICPrototype *> ics_aux = m_ics;
    m_ics.clear();
    qDeleteAll(ics_aux);
    if (m_fileWatcher.files().size() > 0) {
        m_fileWatcher.removePaths(m_fileWatcher.files());
    }
    COMMENT("Finished clearing ICManager.", 3);
}

bool ICManager::loadIC(IC *ic, QString fname, const QString &parentFile)
{
    COMMENT("Loading IC file " << fname.toStdString() << " with parentfile name: " << parentFile.toStdString(), 3);
    if (tryLoadFile(fname, parentFile)) {
        ic->loadFile(fname);
    }
    emit addRecentIcFile(fname);
    return true;
}

void ICManager::openIC(QString fname)
{
    COMMENT("Opening IC file " << fname.toStdString(), 0);
    m_mainWindow->loadPandaFile(fname);
}

ICPrototype *ICManager::getPrototype(const QString& fname)
{
    Q_ASSERT(!fname.isEmpty());
    QFileInfo finfo(fname);
    if (!m_ics.contains(finfo.baseName())) {
        return nullptr;
    }
    return m_ics[finfo.baseName()];
}

bool ICManager::updatePrototypeFilePathName(const QString& sourceName, const QString& targetName)
{
    COMMENT("Updating IC name from " << sourceName.toStdString() << " to " << targetName.toStdString(), 0);
    Q_ASSERT(!sourceName.isEmpty());
    Q_ASSERT(!targetName.isEmpty());
    QFileInfo finfo(sourceName);
    if (!m_ics.contains(finfo.baseName())) {
        return false;
    }
    COMMENT("Updating prototype IC name.", 0);
    auto proto = m_ics[finfo.baseName()];
    proto->fileName(targetName);
    COMMENT("Updating m_fileWatcher. Removing " << sourceName.toStdString(), 0);
    if (m_fileWatcher.removePath(sourceName)) {
        COMMENT("Adding " << targetName.toStdString() << " to m_fileWatcher.", 0);
        m_fileWatcher.addPath(targetName);
    } else {
        COMMENT("Warning. FileWatcher did not exist. Probably already changed by other IC instance update.", 0);
    }
    return true;
}

ICManager *ICManager::instance()
{
    return globalICManager;
}

void ICManager::setGlobalInstance(ICManager *icManager)
{
    globalICManager = icManager;
}

void ICManager::setReloadFile(const QString& fileName)
{
    COMMENT("Change in IC " << fileName.toStdString() << " detected.", 0);
    QString bname = QFileInfo(fileName).baseName();
    m_fileWatcher.addPath(fileName);
    if (!requiresReload.contains(bname)) {
        requiresReload.push_back(bname);
        if (globalICManager == this) {
            wakeUp();
        }
    }
}

void ICManager::wakeUp()
{
    for (QString &bname: requiresReload) {
        if (m_ics.contains(bname)) {
            try {
                m_ics[bname]->reload();
            }  catch (...) {
                QMessageBox::warning(m_mainWindow, tr("File error."), tr("Error reloading changed IC file. Perhaps it was deleted."));
            }
        }
    }
    requiresReload.clear();
    emit updatedIC();
}
