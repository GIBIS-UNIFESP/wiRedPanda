// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/IO/RecentFiles.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>

#include "App/Core/Common.h"
#include "App/Core/Settings.h"
#include "App/GlobalProperties.h"

RecentFiles::RecentFiles(QObject *parent)
    : QObject(parent)
{
    m_files = Settings::recentFiles();

    // Watch every already-known file so the menu updates live if a file is
    // deleted or renamed outside the application (e.g. by the OS or another app).
    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged, this, [this](const QString &filePath) {
        if (!QFile::exists(filePath)) {
            m_files.removeAll(filePath);
            emit recentFilesUpdated();
        }
    });
}

void RecentFiles::addRecentFile(const QString &filePath)
{
    qCDebug(three) << "Setting recent file to: " << filePath;

    if (!QFile(filePath).exists()) {
        return;
    }

    m_fileWatcher.addPath(filePath);

    // Move to front: remove any existing occurrence first so there are no
    // duplicates, then prepend so the latest file is always at index 0.
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
    // Walk with an index rather than an iterator so we can remove stale entries
    // in-place without invalidating the loop; the file-system watcher covers
    // deletions at runtime, but files may have disappeared while the app was
    // closed (between sessions).
    int i = 0;

    while (i < m_files.size()) {
        QFileInfo fileInfo(m_files.at(i));

        if (!fileInfo.exists()) {
            m_files.removeAt(i);
            continue;
        }

        ++i;
    }

    // Persist the cleaned list so stale paths don't accumulate across restarts.
    saveRecentFiles();

    return m_files;
}

void RecentFiles::saveRecentFiles()
{
    Settings::setRecentFiles(m_files);
}

