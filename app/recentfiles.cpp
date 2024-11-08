// Copyright 2015 - 2024, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentfiles.h"

#include "common.h"
#include "globalproperties.h"
//#include "settings.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>

RecentFiles::RecentFiles(QObject *parent)
    : QObject(parent)
{
   /* if (Settings::contains("recentFileList")) {
        m_files = Settings::value("recentFileList").toStringList();
    }*/

    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged, this, [=](const QString &filePath) {
        if (!QFile::exists(filePath)) {
            m_files.removeAll(filePath);
            emit recentFilesUpdated();
        }
    });
}

void RecentFiles::addRecentFile(const QString &filePath)
{
    qCDebug(three) << tr("Setting recent file to: ") << filePath;

    if (!QFile(filePath).exists()) {
        return;
    }

    m_fileWatcher.addPath(filePath);

    m_files.removeAll(filePath);
    m_files.prepend(filePath);

    if (m_files.size() > GlobalProperties::maxRecentFiles) {
        m_files.erase(m_files.begin() + GlobalProperties::maxRecentFiles, m_files.end());
    }

    saveRecentFiles();

    emit recentFilesUpdated();
}

QStringList RecentFiles::recentFiles()
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

void RecentFiles::saveRecentFiles()
{
    //Settings::setValue("recentFileList", m_files);
}
