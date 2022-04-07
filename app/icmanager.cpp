// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icmanager.h"

#include "common.h"
#include "globalproperties.h"
#include "ic.h"
#include "icnotfoundexception.h"
#include "icprototype.h"
#include "mainwindow.h"
#include "settings.h"

#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>

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

    if (globalICManager == this) {
        globalICManager = nullptr;
    }
}

void ICManager::clear()
{
    qCDebug(three) << "Clear ICManager.";
    QMap<QString, ICPrototype *> ics_aux = m_ics;
    m_ics.clear();
    qDeleteAll(ics_aux);
    if (!m_fileWatcher.files().empty()) {
        m_fileWatcher.removePaths(m_fileWatcher.files());
    }
    qCDebug(three) << "Finished clearing ICManager.";
}

bool ICManager::loadIC(IC *ic, QString fname)
{
    // qCDebug(zero) << "Loading IC file ......." << fname;
    loadFile(fname);
    ic->loadFile(fname);
    return true;
}

void ICManager::loadFile(QString &fname)
{
    qCDebug(three) << "Loading file:" << fname;
    QFileInfo finfo;
    // qCDebug(zero) << "Current main window dir:" << QFileInfo(GlobalProperties::currentFile).absolutePath();
    finfo.setFile(QFileInfo(GlobalProperties::currentFile).absolutePath(), QFileInfo(fname).fileName());
    // qCDebug(zero) << "IC file:" << finfo.absoluteFilePath();
    if (!finfo.exists() || !finfo.isFile()) {
        throw std::runtime_error(fname.toStdString() + tr(" not found.").toStdString());
    }
    m_fileWatcher.addPath(finfo.absoluteFilePath());
    if (m_ics.contains(finfo.baseName())) {
        qCDebug(three) << "IC already inserted:" << finfo.baseName();
    } else {
        qCDebug(three) << "Inserting IC:" << finfo.baseName();
        // qCDebug(zero) << "Inserting IC:" << finfo.absoluteFilePath();
        auto *prototype = new ICPrototype(finfo.absoluteFilePath());
        prototype->reload();
        // qCDebug(zero) << "Really Inserting IC:" << finfo.baseName();
        m_ics.insert(finfo.baseName(), prototype);
    }
}

void ICManager::openIC(const QString &fname)
{
    qCDebug(zero) << "Opening IC file:" << fname;
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
    qCDebug(zero) << "Detected change in IC:" << fileName;
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
                QMessageBox::critical(m_mainWindow, tr("Error!"), tr("Error reloading changed IC file. Perhaps it was deleted."));
            }
        }
    }
    requiresReload.clear();
    emit updatedIC();
}
