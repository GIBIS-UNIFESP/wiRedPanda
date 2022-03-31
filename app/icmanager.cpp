// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icmanager.h"

#include "common.h"
#include "globalproperties.h"
#include "ic.h"
#include "icnotfoundexception.h"
#include "icprototype.h"
<<<<<<< HEAD
#include "MainWindow.h"
=======
#include "mainwindow.h"
#include "settings.h"

#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
>>>>>>> master

ICManager *ICManager::globalICManager = nullptr;

ICManager::ICManager(MainWindow *mainWindow, QObject *parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
{
    if (globalICManager == nullptr) {
        globalICManager = this;
    }
    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &ICManager::setReloadFile);
}

ICManager::~ICManager()
{
    clear();
    Settings::setValue("recentICs", "");

    // fprintf(stderr, "Removing IC manager");
    if (globalICManager == this) {
        globalICManager = nullptr;
    }
}

void ICManager::clear()
{
    COMMENT("Clear ICManager", 3);
    QMap<QString, ICPrototype *> ics_aux = m_ics;
    m_ics.clear();
    qDeleteAll(ics_aux);
    if (!m_fileWatcher.files().empty()) {
        m_fileWatcher.removePaths(m_fileWatcher.files());
    }
    COMMENT("Finished clearing ICManager.", 3);
}

bool ICManager::loadIC(IC *ic, QString fname)
{
    // qDebug() << "Loading IC file ....... " << fname;
    loadFile(fname);
    ic->loadFile(fname);
    return true;
}

void ICManager::loadFile(QString &fname)
{
    COMMENT("Loading file " << fname.toStdString(), 3);
    QFileInfo finfo;
    // qDebug() << "Current main window dir: " << QFileInfo(GlobalProperties::currentFile).absolutePath();
    finfo.setFile(QFileInfo(GlobalProperties::currentFile).absolutePath(), QFileInfo(fname).fileName());
    // qDebug() << "IC file: " << finfo.absoluteFilePath();
    Q_ASSERT(finfo.exists() && finfo.isFile());
    m_fileWatcher.addPath(finfo.absoluteFilePath());
    if (m_ics.contains(finfo.baseName())) {
        COMMENT("IC already inserted: " << finfo.baseName().toStdString(), 3);
    } else {
        COMMENT("Inserting IC: " << finfo.baseName().toStdString(), 3);
        // qDebug() << "Inserting IC: " << finfo.absoluteFilePath();
        auto *prototype = new ICPrototype(finfo.absoluteFilePath());
        prototype->reload();
        // qDebug() << "Really Inserting IC: " << finfo.baseName();
        m_ics.insert(finfo.baseName(), prototype);
    }
}

void ICManager::openIC(const QString &fname)
{
    COMMENT("Opening IC file " << fname.toStdString(), 0);
    m_mainWindow->loadPandaFile(fname);
}

ICPrototype *ICManager::getPrototype(const QString &fname)
{
    Q_ASSERT(!fname.isEmpty());
    QFileInfo finfo(fname);
    if (!m_ics.contains(finfo.baseName())) {
        return nullptr;
    }
    return m_ics[finfo.baseName()];
}

ICManager *ICManager::instance()
{
    return globalICManager;
}

void ICManager::setGlobalInstance(ICManager *icManager)
{
    globalICManager = icManager;
}

void ICManager::setReloadFile(const QString &fileName)
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
    for (QString &bname : requiresReload) {
        if (m_ics.contains(bname)) {
            try {
                m_ics[bname]->reload();
            } catch (...) {
                QMessageBox::warning(m_mainWindow, tr("File error."), tr("Error reloading changed IC file. Perhaps it was deleted."));
            }
        }
    }
    requiresReload.clear();
    emit updatedIC();
}
