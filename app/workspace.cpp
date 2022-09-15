// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspace.h"

#include "common.h"
#include "globalproperties.h"
#include "serializationfunctions.h"
#include "settings.h"
#include "simulationblocker.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSaveFile>
#include <QTemporaryFile>

WorkSpace::WorkSpace(QWidget *parent)
    : QWidget(parent)
{
    m_view.setCacheMode(QGraphicsView::CacheBackground);
    m_view.setScene(&m_scene);
    m_scene.setView(&m_view);
    m_scene.setSceneRect(m_view.rect());
    setLayout(new QHBoxLayout);
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

    qCDebug(zero) << tr("FileName: ") << fileName_;
    qCDebug(zero) << tr("Getting autosave settings info.");
    QStringList autosaves = Settings::value("autosaveFile").toStringList();
    qCDebug(zero) << tr("All auto save file names before save: ") << autosaves;

    QString autosaveFileName;
    qCDebug(zero) << tr("Checking if it is an autosave file or a new project, and ask for a fileName.");

    if (fileName_.isEmpty() || autosaves.contains(fileName_)) {
        qCDebug(zero) << tr("Should open window.");
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

    GlobalProperties::currentFile = fileName_;
    m_fileInfo = QFileInfo(fileName_);

    QSaveFile saveFile(fileName_);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        throw Pandaception(tr("Error opening file: ") + saveFile.errorString());
    }

    m_scene.setSceneRect(m_scene.itemsBoundingRect());

    QDataStream stream(&saveFile);
    stream.setVersion(QDataStream::Qt_5_12);
    save(stream);

    if (!saveFile.commit()) {
        throw Pandaception(tr("Could not save file: ") + saveFile.errorString());
    }

    m_scene.undoStack()->setClean();

    if (!autosaveFileName.isEmpty()) {
        qCDebug(zero) << tr("Remove from autosave list recovered file that has been saved.");
        autosaves.removeAll(autosaveFileName);
        Settings::setValue("autosaveFile", autosaves);
        qCDebug(zero) << tr("All auto save file names after removing recovered: ") << autosaves;
    }

    if (m_autosaveFile.exists()) {
        qCDebug(zero) << tr("Remove autosave from settings and delete it.");
        autosaves.removeAll(m_autosaveFile.fileName());
        Settings::setValue("autosaveFile", autosaves);
        m_autosaveFile.remove();
        qCDebug(zero) << tr("All auto save file names after removing autosave: ") << autosaves;
    }

    emit fileChanged(m_fileInfo);
}

void WorkSpace::save(QDataStream &stream)
{
    SerializationFunctions::saveHeader(stream, m_dolphinFileName, m_scene.sceneRect());
    SerializationFunctions::serialize(m_scene.items(Qt::SortOrder(-1)), stream);
}

void WorkSpace::load(const QString &fileName)
{
    QFile file(fileName);

    if (!file.exists()) {
        qCDebug(zero) << tr("This file does not exist: ") << fileName;
        throw Pandaception(tr("This file does not exist: ") + fileName);
    }

    GlobalProperties::currentFile = fileName;
    m_fileInfo = QFileInfo(fileName);

    qCDebug(zero) << tr("File exists.");

    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(zero) << tr("Could not open file: ") << file.errorString();
        throw Pandaception(tr("Could not open file: ") + file.errorString());
    }

    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_12);
    load(stream);

    emit fileChanged(m_fileInfo);
}

void WorkSpace::load(QDataStream &stream)
{
    qCDebug(zero) << tr("Loading file.");
    SimulationBlocker simulationBlocker(m_scene.simulation());
    qCDebug(zero) << tr("Stopped simulation.");
    const double version = SerializationFunctions::loadVersion(stream);
    qCDebug(zero) << tr("Version: ") << version;

    if (GlobalProperties::verbose) {
        if (version > GlobalProperties::version) {
            QMessageBox::warning(this, tr("Newer version file."), tr("Warning! Your WiRedPanda is possibly out of date.\n The file you are opening was saved in a newer version.\n Please check for updates."));
        } else if (version < 4.0) {
            QMessageBox::warning(this, tr("Old version file."), tr("Warning! This is an old version WiRedPanda project file (version < 4.0). To open it correctly, save all the ICs and skins in the main project directory."));
        }
    }

    m_dolphinFileName = SerializationFunctions::loadDolphinFileName(stream, version);
    qCDebug(zero) << tr("Dolphin name: ") << m_dolphinFileName;

    SerializationFunctions::loadRect(stream, version);
    const auto items = SerializationFunctions::deserialize(stream, {}, version);
    qCDebug(zero) << tr("Finished loading items.");

    for (auto *item : items) {
        m_scene.addItem(item);
    }

    m_scene.setSceneRect(m_scene.itemsBoundingRect());

    qCDebug(zero) << tr("Finished loading file.");
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
    qCDebug(zero) << tr("Defining autosave path.");
    QDir autosavePath(QDir::currentPath() + "/autosaves");

    if (!autosavePath.exists()) {
        autosavePath.mkpath(autosavePath.absolutePath());
    }

    qCDebug(zero) << tr("Autosavepath: ") << autosavePath.absolutePath();
    m_autosaveFile.setFileTemplate(autosavePath.absoluteFilePath(".XXXXXX.panda"));
    qCDebug(zero) << tr("Setting current file to random file.");
}

void WorkSpace::autosave()
{
    qCDebug(two) << tr("Starting autosave.");
    QStringList autosaves = Settings::value("autosaveFile").toStringList();
    qCDebug(three) << tr("All auto save file names before autosaving: ") << autosaves;

    qCDebug(zero) << tr("Checking if autosave file exists and if it contains current project file. If so, remove autosave file from it.");

    if (!m_autosaveFile.fileName().isEmpty() && autosaves.contains(m_autosaveFile.fileName())) {
        qCDebug(three) << tr("Removing current autosave file name.");
        autosaves.removeAll(m_autosaveFile.fileName());
        Settings::setValue("autosaveFile", autosaves);
    }

    qCDebug(zero) << tr("All auto save file names after possibly removing autosave: ") << autosaves;
    qCDebug(zero) << tr("If autosave exists and undo stack is clean, remove it.");
    auto *undoStack = m_scene.undoStack();
    qCDebug(zero) << tr("Undo stack element: ") << undoStack->index() << tr(" of ") << undoStack->count();

    if (undoStack->isClean()) {
        qCDebug(three) << tr("Undo stack is clean.");
        m_autosaveFile.remove();
        emit fileChanged(m_fileInfo);

        return;
    }

    qCDebug(three) << tr("Undo is !clean. Must set autosave file.");

    if (m_fileInfo.fileName().isEmpty()) {
        qCDebug(three) << tr("Default value not set yet.");
        QDir path(QDir::currentPath() + "/autosaves");

        if (!path.exists()) {
            path.mkpath(path.absolutePath());
        }

        qCDebug(three) << tr("Autosavepath: ") << path.absolutePath();
        m_autosaveFile.setFileTemplate(path.absoluteFilePath(".XXXXXX.panda"));
        qCDebug(three) << tr("Setting current file to random file.");
    } else {
        qCDebug(three) << tr("Autosave path set to the current file's directory, if there is one.");
        QDir path(m_fileInfo.absolutePath());
        qCDebug(three) << tr("Autosavepath: ") << path.absolutePath();
        m_autosaveFile.setFileTemplate(path.absoluteFilePath("." + m_fileInfo.baseName() + ".XXXXXX.panda"));
        qCDebug(three) << tr("Setting current file to: ") << m_fileInfo.absoluteFilePath();
    }

    if (!m_autosaveFile.open()) {
        throw Pandaception(tr("Error opening autosave file: ") + m_autosaveFile.errorString());
    }

    QString autosaveFileName = m_autosaveFile.fileName();
    GlobalProperties::currentFile = autosaveFileName;

    qCDebug(three) << tr("Writing to autosave file.");
    QDataStream stream(&m_autosaveFile);
    stream.setVersion(QDataStream::Qt_5_12);
    save(stream);
    m_autosaveFile.close();

    autosaves.append(autosaveFileName);
    Settings::setValue("autosaveFile", autosaves);

    qCDebug(three) << tr("All auto save file names after adding autosave: ") << autosaves;

    emit fileChanged(m_fileInfo);
}

void WorkSpace::setIsAutosave()
{
    m_autosaveFile.setFileName(m_fileInfo.filePath());
}
