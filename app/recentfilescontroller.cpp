#include "recentfilescontroller.h"
#include "common.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QSettings>

// TODO: quotes bug
void RecentFilesController::addFile(const QString &fname)
{
    COMMENT("Setting recent file to : \"" << fname.toStdString() << "\"", 0);
    if (!QFile(fname).exists()) {
        return;
    }
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    if (!settings.contains(attrName)) {
        //! TODO: disabling this check for now, since it creates problems while loading .panda files as integrated circuits
        if (attrName == "recentICs") {
            settings.setValue("recentICs", "");
        } else {
            COMMENT("Early return because the settings do not contain attrName " << attrName.toStdString(), 0);
            return;
        }
    }
    QStringList files = settings.value(attrName).toStringList();

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
    settings.setValue(attrName, files);

    emit recentFilesUpdated();
}

QStringList RecentFilesController::getFiles()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    if (!settings.contains(attrName)) {
        return (QStringList());
    }
    QStringList files = settings.value(attrName).toStringList();
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
    settings.setValue(attrName, files);
    return (files);
}
