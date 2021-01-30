#include "icmanager.h"
#include "filehelper.h"
#include "ic.h"
#include "icnotfoundexception.h"
#include "icprototype.h"
#include "mainwindow.h"
#include "qfileinfo.h"

#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>

ICManager *ICManager::globalICManager = nullptr;

ICManager::ICManager(MainWindow *mainWindow, QObject *parent)
    : QObject(parent)
    , mainWindow(mainWindow)
{
    if (globalICManager == nullptr) {
        globalICManager = this;
    }
    connect(&fileWatcher, &QFileSystemWatcher::fileChanged, this, &ICManager::reloadFile);
}

ICManager::~ICManager()
{
    clear();
    // fprintf(stderr, "Removing IC manager");
    if (globalICManager == this) {
        globalICManager = nullptr;
    }
}

bool ICManager::tryLoadFile(QString &fname, QString parentFile)
{
    try {
        loadFile(fname, parentFile);
    } catch (ICNotFoundException &err) {
        COMMENT("ICNotFoundException thrown: " << err.what(), 0);
        int ret = QMessageBox::warning(mainWindow, tr("Error"), QString::fromStdString(err.what()), QMessageBox::Ok, QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel) {
            return (false);
        } else {
            fname = mainWindow->getOpenICFile();
            if (fname.isEmpty()) {
                return (false);
            } else {
                return (tryLoadFile(fname, parentFile));
            }
        }
    }
    return (true);
}

void ICManager::loadFile(QString &fname, QString parentFile)
{
    QFileInfo finfo = FileHelper::findICFile(fname, parentFile);
    fname = finfo.filePath();
    Q_ASSERT(finfo.exists() && finfo.isFile());
    fileWatcher.addPath(finfo.absoluteFilePath());
    if (ics.contains(finfo.baseName())) {
        COMMENT("IC already inserted: " << finfo.baseName().toStdString(), 0);
    } else {
        COMMENT("Inserting IC: " << finfo.baseName().toStdString(), 0);
        ICPrototype *prototype = new ICPrototype(finfo.absoluteFilePath());
        prototype->reload();
        ics.insert(finfo.baseName(), prototype);
    }
}

void ICManager::clear()
{
    COMMENT("Clear ICManager", 0);
    QMap<QString, ICPrototype *> ics_aux = ics;
    ics.clear();
    for (auto it : ics_aux) {
        delete it;
    }
    if (fileWatcher.files().size() > 0) {
        fileWatcher.removePaths(fileWatcher.files());
    }
    COMMENT("Finished clearing ICManager.", 0);
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
    if (mainWindow) {
        mainWindow->updateRecentICs();
    }
}

bool ICManager::loadIC(IC *ic, QString fname, QString parentFile)
{
    if (tryLoadFile(fname, parentFile)) {
        ic->loadFile(fname);
    }
    updateRecentICs(fname);
    return (true);
}

ICPrototype *ICManager::getPrototype(QString fname)
{
    Q_ASSERT(!fname.isEmpty());
    QFileInfo finfo(fname);
    if (!ics.contains(finfo.baseName())) {
        return (nullptr);
    }
    return (ics[finfo.baseName()]);
}

bool ICManager::updatePrototypeFilePathName(QString sourceName, QString targetName)
{
    COMMENT("Updating IC name from " << sourceName.toStdString() << " to " << targetName.toStdString(), 0);
    Q_ASSERT(!sourceName.isEmpty());
    Q_ASSERT(!targetName.isEmpty());
    QFileInfo finfo(sourceName);
    if (!ics.contains(finfo.baseName())) {
        return (false);
    }
    COMMENT("Updating prototype IC name.", 0);
    auto proto = ics[finfo.baseName()];
    proto->fileName(targetName);
    COMMENT("Updating fileWatcher. Removing " << sourceName.toStdString(), 0);
    if (fileWatcher.removePath(sourceName)) {
        COMMENT("Adding " << targetName.toStdString() << " to fileWatcher.", 0);
        fileWatcher.addPath(targetName);
    } else {
        COMMENT("Warning. FileWatcher did not exist. Probably already changed by other IC instance update.", 0);
    }
    return (true);
}

ICManager *ICManager::instance()
{
    return (globalICManager);
}

void ICManager::reloadFile(QString fileName)
{
    COMMENT("Change in IC " << fileName.toStdString() << " detected.", 0);
    QString bname = QFileInfo(fileName).baseName();
    fileWatcher.addPath(fileName);
    if (warnAboutFileChange(bname)) {
        if (ics.contains(bname)) {
            try {
                ics[bname]->reload();
            } catch (std::runtime_error &e) {
                QMessageBox::warning(mainWindow, "Error", tr("Error reloading IC: ") + e.what(), QMessageBox::Ok, QMessageBox::NoButton);
            }
        }
    }
    emit updatedIC();
}

// Maybe this funcion should never be called and the main project should reload the IC every time it changes.
bool ICManager::warnAboutFileChange(const QString &fileName)
{
    COMMENT("File " << fileName.toStdString() << " has changed!", 0);
    QMessageBox msgBox;
    if (mainWindow) {
        msgBox.setParent(mainWindow);
    }
    msgBox.setLocale(QLocale::Portuguese);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setText(tr("The file %1 changed, do you want to reload?").arg(fileName));
    msgBox.setWindowModality(Qt::ApplicationModal);
    msgBox.setDefaultButton(QMessageBox::Yes);
    return (msgBox.exec() == QMessageBox::Yes);
}
