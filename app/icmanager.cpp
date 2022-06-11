// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icmanager.h"

#include "common.h"
#include "globalproperties.h"
#include "ic.h"
#include "icprototype.h"
#include "mainwindow.h"

ICManager::ICManager(QObject *parent)
    : QObject(parent)
{
    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &ICManager::setReloadFile);
}

bool ICManager::loadIC(IC *ic, const QString &fileName)
{
    loadFile(fileName);
    ic->loadFile(fileName);
    return true;
}

void ICManager::loadFile(const QString &fileName)
{
    qCDebug(three) << tr("Loading file:") << fileName;
    QFileInfo fileInfo;
    fileInfo.setFile(QFileInfo(GlobalProperties::currentFile).absolutePath(), QFileInfo(fileName).fileName());
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        throw Pandaception(fileInfo.absoluteFilePath() + tr(" not found."));
    }
    instance().m_fileWatcher.addPath(fileInfo.absoluteFilePath());
    if (instance().m_ics.contains(fileInfo.baseName())) {
        qCDebug(three) << tr("IC already inserted:") << fileInfo.baseName();
        return;
    }
    qCDebug(three) << tr("Inserting IC:") << fileInfo.baseName();
    auto *prototype = new ICPrototype(fileInfo.absoluteFilePath());
    prototype->reload();
    instance().m_ics.insert(fileInfo.baseName(), prototype);
}

ICPrototype *ICManager::prototype(const QString &fileName)
{
    QFileInfo fileInfo(fileName);
    if (!instance().m_ics.contains(fileInfo.baseName())) {
        return nullptr;
    }
    return instance().m_ics[fileInfo.baseName()];
}

void ICManager::setReloadFile(const QString &fileName)
{
    qCDebug(zero) << tr("Detected change in IC:") << fileName;
    QString baseName = QFileInfo(fileName).baseName();
    m_fileWatcher.addPath(fileName);
    if (!m_requiresReload.contains(baseName)) {
        m_requiresReload.push_back(baseName);
        wakeUp();
    }
}

void ICManager::wakeUp()
{
    const auto requiresReload = instance().m_requiresReload;
    for (const QString &baseName : requiresReload) {
        if (instance().m_ics.contains(baseName)) {
            instance().m_ics.value(baseName)->reload();
        }
    }
    instance().m_requiresReload.clear();
    emit instance().updatedIC();
}
