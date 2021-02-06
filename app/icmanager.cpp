// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "icmanager.h"

#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QDebug>

#include "filehelper.h"
#include "ic.h"
#include "icnotfoundexception.h"
#include "icprototype.h"
#include "mainwindow.h"

ICManager *ICManager::globalICManager = nullptr;

ICManager::ICManager(MainWindow *mainWindow, QObject *parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
{
    if (globalICManager == nullptr) {
        globalICManager = this;
    }
    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &ICManager::reloadFile);
}

ICManager::~ICManager()
{
    clear();
    // fprintf(stderr, "Removing IC manager");
    if (globalICManager == this) {
        globalICManager = nullptr;
    }
}

bool ICManager::tryLoadFile(QString &fname, const QString& parentFile)
{
    try {
        loadFile(fname, parentFile);
    } catch (ICNotFoundException &err) {
        qDebug() << "ICNotFoundException thrown: " << err.what();
        int ret = QMessageBox::warning(m_mainWindow, tr("Error"), QString::fromStdString(err.what()), QMessageBox::Ok, QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel) {
            return false;
        }
        fname = m_mainWindow->getOpenICFile();
        if (fname.isEmpty()) {
            return false;
        }
        return tryLoadFile(fname, parentFile);
    }
    return true;
}

void ICManager::loadFile(QString &fname, const QString& parentFile)
{
    QFileInfo finfo = FileHelper::findICFile(fname, parentFile);
    fname = finfo.filePath();
    Q_ASSERT(finfo.exists() && finfo.isFile());

    m_fileWatcher.addPath(finfo.absoluteFilePath());
    if (m_ics.contains(finfo.baseName())) {
        qDebug() << "IC already inserted: " << QString::fromStdString(finfo.baseName().toStdString());

    } else {
        qDebug() << "Inserting IC: " << QString::fromStdString(finfo.baseName().toStdString());
        auto *prototype = new ICPrototype(finfo.absoluteFilePath());
        prototype->reload();
        m_ics.insert(finfo.baseName(), prototype);
    }
}

void ICManager::clear()
{
    qDebug() << "Clear ICManager";
    QMap<QString, ICPrototype *> ics_aux = m_ics;
    m_ics.clear();
    for (auto it : ics_aux) {
        delete it;
    }
    if (m_fileWatcher.files().size() > 0) {
        m_fileWatcher.removePaths(m_fileWatcher.files());
    }
    qDebug() << "Finished clearing ICManager.";
}

void ICManager::updateRecentICs(const QString &fname)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(), QApplication::applicationName());
    QStringList files;
    if (settings.contains("recentICs")) {
        files = settings.value("recentICs").toStringList();
        files.removeAll(fname);
    }
    files.prepend(fname);
    settings.setValue("recentICs", files);
    if (m_mainWindow) {
        m_mainWindow->updateRecentICs();
    }
}

bool ICManager::loadIC(IC *ic, QString fname, QString parentFile)
{
    if (tryLoadFile(fname, parentFile)) {
        ic->loadFile(fname);
    }
    updateRecentICs(fname);
    return true;
}

ICPrototype *ICManager::getPrototype(const QString& fname)
{
    Q_ASSERT(!fname.isEmpty());
    QFileInfo finfo(fname);
    if (!m_ics.contains(finfo.baseName())) {
        return nullptr;
    }
    return m_ics[finfo.baseName()];
}

bool ICManager::updatePrototypeFilePathName(const QString& sourceName, const QString& targetName)
{
    qDebug() << "Updating IC name from " << QString::fromStdString(sourceName.toStdString()) << " to " << QString::fromStdString(targetName.toStdString());
    Q_ASSERT(!sourceName.isEmpty());
    Q_ASSERT(!targetName.isEmpty());
    QFileInfo finfo(sourceName);
    if (!m_ics.contains(finfo.baseName())) {
        return false;
    }

    qDebug() << "Updating prototype IC name.";
    auto proto = m_ics[finfo.baseName()];
    proto->fileName(targetName);
    qDebug() << "Updating fileWatcher. Removing " << QString::fromStdString(sourceName.toStdString());
    if (m_fileWatcher.removePath(sourceName)) {
        qDebug() << "Adding " << QString::fromStdString(targetName.toStdString()) << " to fileWatcher.";
        m_fileWatcher.addPath(targetName);
    } else {
        qDebug() << "Warning. FileWatcher did not exist. Probably already changed by other IC instance update.";
    }
    return true;
}

ICManager *ICManager::instance()
{
    return globalICManager;
}

void ICManager::reloadFile(const QString& fileName)
{
    qDebug() << "Change in IC " << QString::fromStdString(fileName.toStdString()) << " detected.";
    QString bname = QFileInfo(fileName).baseName();
    m_fileWatcher.addPath(fileName);
    if (warnAboutFileChange(bname)) {
        if (m_ics.contains(bname)) {
            try {
                m_ics[bname]->reload();
            } catch (std::runtime_error &e) {
                QMessageBox::warning(m_mainWindow, "Error", tr("Error reloading IC: ") + e.what(), QMessageBox::Ok, QMessageBox::NoButton);
            }
        }
    }
    emit updatedIC();
}

// Maybe this funcion should never be called and the main project should reload the IC every time it changes.
bool ICManager::warnAboutFileChange(const QString &fileName)
{
    qDebug() << "File " << QString::fromStdString(fileName.toStdString()) << " has changed!";
    QMessageBox msgBox;
    if (m_mainWindow) {
        msgBox.setParent(m_mainWindow);
    }
    msgBox.setLocale(QLocale::Portuguese);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setText(tr("The file %1 changed, do you want to reload?").arg(fileName));
    msgBox.setWindowModality(Qt::ApplicationModal);
    msgBox.setDefaultButton(QMessageBox::Yes);
    return msgBox.exec() == QMessageBox::Yes;
}
