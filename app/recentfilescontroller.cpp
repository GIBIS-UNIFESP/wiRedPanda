// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recentfilescontroller.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QSettings>

#include "common.h"

// TODO: quotes bug
void RecentFilesController::addFile(const QString &fname)
{
    COMMENT("Setting recent file to : \"" << fname.toStdString() << "\"", 0);
    if (!QFile(fname).exists()) {
        return;
    }
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    if (!settings.contains(m_attrName)) {
        //! TODO: disabling this check for now, since it creates problems while loading .panda files as integrated circuits
        if (m_attrName == "recentICs") {
            settings.setValue("recentICs", "");
        } else {
            COMMENT("Early return because the settings do not contain attrName " << m_attrName.toStdString(), 0);
            return;
        }
    }
    QStringList files = settings.value(m_attrName).toStringList();

    files.removeAll(fname);

    files.prepend(fname);
    for (int i = 0; i < files.size();) {
        QFileInfo fileInfo(files.at(i));
        if (!fileInfo.exists()) {
            files.removeAt(i);
        } else {
            ++i;
        }
    }
    while (files.size() > MaxRecentFiles) {
        files.removeLast();
    }
    settings.setValue(m_attrName, files);

    emit recentFilesUpdated();
}

QStringList RecentFilesController::getFiles()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    if (!settings.contains(m_attrName)) {
        return QStringList();
    }
    QStringList files = settings.value(m_attrName).toStringList();
    for (int i = 0; i < files.size();) {
        QFileInfo fileInfo(files.at(i));
        if (!fileInfo.exists()) {
            files.removeAt(i);
        } else {
            ++i;
        }
    }
    while (files.size() > MaxRecentFiles) {
        files.removeLast();
    }
    settings.setValue(m_attrName, files);
    return files;
}
