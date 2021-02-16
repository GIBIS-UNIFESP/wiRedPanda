// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentfilescontroller.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>

#include "common.h"
#include "WPandaSettings.h"

RecentFilesController::RecentFilesController(QObject *parent, bool saveSetting)
    : QObject(parent)
    , m_saveSetting(saveSetting)
{
    if (m_saveSetting) {
        auto settings = WPandaSettings::self();
        m_files = settings->recentFileList();
    }
}

// TODO: quotes bug
void RecentFilesController::addRecentFile(const QString &fname)
{
    COMMENT("Setting recent file to : \"" << fname.toStdString() << "\"", 0);
    if (!QFile(fname).exists()) {
        return;
    }

    m_files.removeAll(fname);
    m_files.prepend(fname);
    if (m_files.size() > MaxRecentFiles) {
        m_files.erase(m_files.begin() + MaxRecentFiles, m_files.end());
    }

    emit recentFilesUpdated();

    saveRecentFiles();
}

QStringList RecentFilesController::getRecentFiles()
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
    if (m_saveSetting) {
        auto settings = WPandaSettings::self();
        settings->setRecentFileList(m_files);
    }
}
