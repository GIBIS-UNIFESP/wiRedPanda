// Copyright 2015 - 2022, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentfilescontroller.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QSettings>

#include "common.h"

RecentFilesController::RecentFilesController(const QString &_attrName, QObject *parent, bool saveSetting)
    : QObject(parent)
    , m_attrName(_attrName)
    , m_saveSetting(saveSetting)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    if (settings.contains(m_attrName)) {
        m_files = settings.value(m_attrName).toStringList();
    }
}

// TODO: quotes bug
void RecentFilesController::addRecentFile(const QString &fname)
{
    COMMENT("Setting recent file to : \"" << fname.toStdString() << "\"", 3);
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
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
        settings.setValue(m_attrName, m_files);
    }
}
