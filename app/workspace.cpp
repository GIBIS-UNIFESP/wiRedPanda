// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspace.h"

#include "common.h"
#include "globalproperties.h"
#include "serialization.h"
#include "settings.h"
#include "simulationblocker.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTemporaryFile>

WorkSpace::WorkSpace(QWidget *parent)
    : QWidget(parent)
{
    m_view.setCacheMode(QGraphicsView::CacheBackground);
    m_view.setScene(&m_scene);
    m_scene.setView(&m_view);
    m_scene.setSceneRect(m_view.rect());
    setLayout(new QHBoxLayout());
    layout()->addWidget(&m_view);

    connect(&m_scene, &Scene::circuitHasChanged, this, &WorkSpace::autosave);

    setAutosaveFileName();
}

Scene *WorkSpace::scene()
{
    return &m_scene;
}

GraphicsView *WorkSpace::view()
{
    return &m_view;
}

Simulation *WorkSpace::simulation()
{
    return m_scene.simulation();
}

QFileInfo WorkSpace::fileInfo()
{
    return m_fileInfo;
}

void WorkSpace::save(const QString &fileName)
{
    QString fileName_ = fileName.isEmpty() ? m_fileInfo.absoluteFilePath() : fileName;

    qCDebug(zero) << "FileName: " << fileName_;
    qCDebug(zero) << "Getting autosave settings info.";
    QStringList autosaves = Settings::value("autosaveFile").toStringList();
    qCDebug(zero) << "All auto save file names before save: " << autosaves;

    QString autosaveFileName;
    qCDebug(zero) << "Checking if it is an autosave file or a new project, and ask for a fileName.";

    if (fileName_.isEmpty() || autosaves.contains(fileName_)) {
        qCDebug(zero) << "Should open window.";
        autosaveFileName = fileName_;

        if (m_fileInfo.fileName().isEmpty()) {
            const QString path = fileName.isEmpty() ? m_fileInfo.absolutePath() : QFileInfo(fileName).absolutePath();
            fileName_ = QFileDialog::getSaveFileName(this, tr("Save File"), path, tr("Panda files (*.panda)"));
        }
    }

    if (fileName_.isEmpty()) {
        return;
    }

    if (!fileName_.endsWith(".panda")) {
        fileName_.append(".panda");
    }

    GlobalProperties::currentDir = QFileInfo(fileName_).absolutePath();
    m_fileInfo = QFileInfo(fileName_);

    QSaveFile saveFile(fileName_);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        throw Pandaception(tr("Error opening file: ") + saveFile.errorString());
    }

    m_scene.setSceneRect(m_scene.itemsBoundingRect());

    QDataStream stream(&saveFile);
    Serialization::writePandaHeader(stream);
    save(stream);

    if (!saveFile.commit()) {
        throw Pandaception(tr("Could not save file: ") + saveFile.errorString());
    }

    m_scene.undoStack()->setClean();

    if (!autosaveFileName.isEmpty()) {
        qCDebug(zero) << "Remove from autosave list recovered file that has been saved.";
        autosaves.removeAll(autosaveFileName);
        Settings::setValue("autosaveFile", autosaves);
        qCDebug(zero) << "All auto save file names after removing recovered: " << autosaves;
    }

    if (m_autosaveFile.exists()) {
        qCDebug(zero) << "Remove autosave from settings and delete it.";
        autosaves.removeAll(m_autosaveFile.fileName());
        Settings::setValue("autosaveFile", autosaves);
        m_autosaveFile.remove();
        qCDebug(zero) << "All auto save file names after removing autosave: " << autosaves;
    }

    emit fileChanged(m_fileInfo);
}

void WorkSpace::save(QDataStream &stream)
{
    stream << m_dolphinFileName;
    stream << m_scene.sceneRect();
    Serialization::serialize(m_scene.items(), stream);
}

void WorkSpace::load(const QString &fileName)
{
    QFile file(fileName);

    if (!file.exists()) {
        qCDebug(zero) << "This file does not exist: " << fileName;
        throw Pandaception(tr("This file does not exist: ") + fileName);
    }

    GlobalProperties::currentDir = QFileInfo(fileName).absolutePath();
    m_fileInfo = QFileInfo(fileName);

    qCDebug(zero) << "File exists.";

    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(zero) << "Could not open file: " << file.errorString();
        throw Pandaception(tr("Could not open file: ") + file.errorString());
    }

    QDataStream stream(&file);
    QVersionNumber version = Serialization::readPandaHeader(stream);
    load(stream, version);

    emit fileChanged(m_fileInfo);
}

void WorkSpace::load(QDataStream &stream, QVersionNumber version)
{
    qCDebug(zero) << "Loading file.";
    SimulationBlocker simulationBlocker(m_scene.simulation());
    qCDebug(zero) << "Stopped simulation.";
    qCDebug(zero) << "Version: " << version;

    if (GlobalProperties::verbose) {
        if (version > GlobalProperties::version) {
            const QString progVersion = GlobalProperties::version.toString();
            const QString fileVersion = version.toString();
            const QString message = tr("Warning! Your wiRedPanda (Version: %1) is possibly out of date.\n"
                         "The file you are opening was saved with version (%2).\n"
                         "Please check for updates if the file does not load correctly.")
                          .arg(progVersion, fileVersion);
            QMessageBox::warning(this, tr("Newer version file."), message);
        } else if (version < VERSION("4.0")) {
            const QString message = tr("Warning! This is an old version wiRedPanda project file (version < 4.0). "
                         "To open it correctly, save all the ICs and skins in the main project directory.");
            QMessageBox::warning(this, tr("Old version file."), message);
        }
    }

    m_dolphinFileName = Serialization::loadDolphinFileName(stream, version);
    qCDebug(zero) << "Dolphin name: " << m_dolphinFileName;

    Serialization::loadRect(stream, version);
    const auto items = Serialization::deserialize(stream, {}, version);
    qCDebug(zero) << "Finished loading items.";

    for (auto *item : items) {
        m_scene.addItem(item);
    }

    m_scene.setSceneRect(m_scene.itemsBoundingRect());

    qCDebug(zero) << "Finished loading file.";
}

void WorkSpace::setDolphinFileName(const QString &fileName)
{
    m_dolphinFileName = fileName;
}

QString WorkSpace::dolphinFileName()
{
    return m_dolphinFileName;
}

void WorkSpace::setAutosaveFileName()
{
    qCDebug(zero) << "Defining autosave path.";
    QDir autosavePath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/autosaves");

    if (!autosavePath.exists()) {
        autosavePath.mkpath(autosavePath.absolutePath());
    }

    qCDebug(zero) << "Autosavepath: " << autosavePath.absolutePath();
    m_autosaveFile.setFileTemplate(autosavePath.absoluteFilePath(".XXXXXX.panda"));
    qCDebug(zero) << "Setting current file to random file.";
}

void WorkSpace::autosave()
{
    qCDebug(two) << "Starting autosave.";
    QStringList autosaves = Settings::value("autosaveFile").toStringList();
    qCDebug(three) << "All auto save file names before autosaving: " << autosaves;

    qCDebug(zero) << "Checking if autosave file exists and if it contains current project file. If so, remove autosave file from it.";

    if (!m_autosaveFile.fileName().isEmpty() && autosaves.contains(m_autosaveFile.fileName())) {
        qCDebug(three) << "Removing current autosave file name.";
        autosaves.removeAll(m_autosaveFile.fileName());
        Settings::setValue("autosaveFile", autosaves);
    }

    qCDebug(zero) << "All auto save file names after possibly removing autosave: " << autosaves;
    qCDebug(zero) << "If autosave exists and undo stack is clean, remove it.";
    auto *undoStack = m_scene.undoStack();
    qCDebug(zero) << "Undo stack element: " << undoStack->index() << " of " << undoStack->count();

    if (undoStack->isClean()) {
        qCDebug(three) << "Undo stack is clean.";
        m_autosaveFile.remove();
        emit fileChanged(m_fileInfo);

        return;
    }

    qCDebug(three) << "Undo is !clean. Must set autosave file.";

    QDir path;

    if (m_fileInfo.fileName().isEmpty()) {
        qCDebug(three) << "Default value not set yet.";
        path.setPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/autosaves");

        if (!path.exists()) {
            path.mkpath(path.absolutePath());
        }

        qCDebug(three) << "Autosavepath: " << path.absolutePath();
        m_autosaveFile.setFileTemplate(path.absoluteFilePath(".XXXXXX.panda"));
        qCDebug(three) << "Setting current file to random file.";
    } else {
        qCDebug(three) << "Autosave path set to the current file's directory, if there is one.";
        path.setPath(m_fileInfo.absolutePath());
        qCDebug(three) << "Autosavepath: " << path.absolutePath();
        m_autosaveFile.setFileTemplate(path.absoluteFilePath("." + m_fileInfo.baseName() + ".XXXXXX.panda"));
        qCDebug(three) << "Setting current file to: " << m_fileInfo.absoluteFilePath();
    }

    if (!m_autosaveFile.open()) {
        throw Pandaception(tr("Error opening autosave file: ") + m_autosaveFile.errorString());
    }

    QString autosaveFileName = m_autosaveFile.fileName();
    GlobalProperties::currentDir = path.absolutePath();

    qCDebug(three) << "Writing to autosave file.";
    QDataStream stream(&m_autosaveFile);
    Serialization::writePandaHeader(stream);
    save(stream);
    m_autosaveFile.close();

    autosaves.append(autosaveFileName);
    Settings::setValue("autosaveFile", autosaves);

    qCDebug(three) << "All auto save file names after adding autosave: " << autosaves;

    emit fileChanged(m_fileInfo);
}

void WorkSpace::setAutosaveFile()
{
    m_autosaveFile.setFileName(m_fileInfo.filePath());
}
