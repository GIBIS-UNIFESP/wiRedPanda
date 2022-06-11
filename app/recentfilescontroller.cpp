// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentfilescontroller.h"

#include "common.h"
#include "settings.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>

RecentFilesController::RecentFilesController(QObject *parent)
    : QObject(parent)
{
    if (Settings::contains("recentFileList")) {
        m_files = Settings::value("recentFileList").toStringList();
    }
}

// TODO: quotes bug
void RecentFilesController::addRecentFile(const QString &fileName)
{
    qCDebug(three) << tr("Setting recent file to:") << fileName;
    if (!QFile(fileName).exists()) {
        return;
    }
    m_files.removeAll(fileName);
    m_files.prepend(fileName);
    if (m_files.size() > MaxRecentFiles) {
        m_files.erase(m_files.begin() + MaxRecentFiles, m_files.end());
    }
    emit recentFilesUpdated();
    saveRecentFiles();
}

QStringList RecentFilesController::recentFiles()
{
    int i = 0;
    while (i < m_files.size()) {
        QFileInfo fileInfo(m_files.at(i));
        if (!fileInfo.exists()) {
            m_files.removeAt(i);
            continue;
        }
        ++i;
    }
    saveRecentFiles();

    return m_files;
}

void RecentFilesController::saveRecentFiles()
{
    Settings::setValue("recentFileList", m_files);
}
