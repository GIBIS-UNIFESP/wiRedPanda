// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspace.h"

#include "common.h"
#include "globalproperties.h"
#include "serializationfunctions.h"
#include "settings.h"

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
    m_scene.setSceneRect(m_scene.sceneRect().united(m_view.rect()));
    setLayout(new QHBoxLayout);
    layout()->addWidget(&m_view);

    connect(scene(), &Scene::circuitAppearenceHasChanged, this, &WorkSpace::autosave);
    connect(scene(), &Scene::circuitHasChanged,           this, &WorkSpace::autosave);

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

SimulationController *WorkSpace::simulationController()
{
    return m_scene.simulationController();
}

QFileInfo WorkSpace::fileInfo()
{
    return m_fileInfo;
}

void WorkSpace::save(const QString &fileName)
{
    QString fileName_ = fileName.isEmpty() ? m_fileInfo.absoluteFilePath() : fileName;

    qCDebug(zero) << tr("fileName:") << fileName_;
    qCDebug(zero) << tr("Getting autosave settings info.");
    QStringList autosaves = Settings::value("autosaveFile").toStringList();
    qCDebug(zero) << tr("All auto save file names before save:") << autosaves;
    qCDebug(zero) << tr("Checking if it is an autosave file or a new project, and ask for a fileName.");
    QString autosaveFileName;

    if (fileName_.isEmpty() || (!autosaves.isEmpty() && autosaves.contains(fileName_))) {
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

    QSaveFile saveFile(fileName_);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        throw Pandaception(tr("Error opening file: ") + saveFile.errorString());
    }

    QDataStream stream(&saveFile);
    save(stream, m_dolphinFileName);

    if (!saveFile.commit()) {
        throw Pandaception(tr("Could not save file: ") + saveFile.errorString());
    }

    m_fileInfo = QFileInfo(fileName_);
    emit fileChanged(m_fileInfo.fileName());
    scene()->undoStack()->setClean();
    qCDebug(zero) << tr("Remove from autosave list recovered file that has been saved.");

    if (!autosaveFileName.isEmpty()) {
        autosaves.removeAll(autosaveFileName);
        Settings::setValue("autosaveFile", autosaves);
        qCDebug(zero) << tr("All auto save file names after removing recovered:") << autosaves;
    }

    qCDebug(zero) << tr("Remove autosave from settings and deleting it.");

    if (m_autosaveFile.exists()) {
        autosaves.removeAll(m_autosaveFile.fileName());
        Settings::setValue("autosaveFile", autosaves);
        m_autosaveFile.remove();
        qCDebug(zero) << tr("All auto save file names after removing autosave:") << autosaves;
    }
}

void WorkSpace::save(QDataStream &stream, const QString &dolphinFileName)
{
    SerializationFunctions::saveHeader(stream, dolphinFileName, m_scene.sceneRect());
    SerializationFunctions::serialize(m_scene.items(), stream);
}

void WorkSpace::load(const QString &fileName, const bool isPlaying)
{
    QFile file(fileName);

    if (!file.exists()) {
        qCDebug(zero) << tr("Error: This file does not exist:") << fileName;
        return;
    }

    GlobalProperties::currentFile = fileName;
    m_fileInfo = QFileInfo(fileName);

    qCDebug(zero) << tr("File exists");
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(zero) << tr("Could not open file:") << file.errorString();
        return;
    }

    QDataStream stream(&file);
    load(stream, isPlaying);
    qCDebug(zero) << tr("Closing file.");
    file.close();
}

void WorkSpace::load(QDataStream &stream, const bool isPlaying)
{
    qCDebug(zero) << tr("Loading file.");
    m_scene.simulationController()->stop();
    qCDebug(zero) << tr("Stopped simulation.");
    const double version = SerializationFunctions::loadVersion(stream);
    if (version > GlobalProperties::version && GlobalProperties::verbose) {
        QMessageBox::warning(this, tr("Newer version file."), tr("Warning! Your WiRedPanda is possibly out of date.\n The file you are opening was saved in a newer version.\n Please check for updates."));
    } else if (version < 4.0 && GlobalProperties::verbose) {
        QMessageBox::warning(this, tr("Old version file."), tr("Warning! This is an old version WiRedPanda project file (version < 4.0). To open it correctly, save all the ICs and skins in the main project directory."));
    }
    qCDebug(zero) << tr("Version:") << version;
    m_dolphinFileName = SerializationFunctions::loadDolphinFileName(stream, version);
    qCDebug(zero) << tr("Dolphin name:") << m_dolphinFileName;
    QRectF rect(SerializationFunctions::loadRect(stream, version));
    qCDebug(zero) << tr("Header Ok. Version:") << version;
    auto items = SerializationFunctions::deserialize(stream, version);
    qCDebug(zero) << tr("Finished loading items.");
    for (auto *item : qAsConst(items)) {
        m_scene.addItem(item);
    }
    qCDebug(three) << tr("This code tries to centralize the elements in scene using the rectangle. But it is not working well.");
    // TODO: improve this
    m_scene.setSceneRect(m_scene.itemsBoundingRect());
    if (auto *view = m_scene.view()) {
        rect = rect.united(view->rect());
        rect.moveCenter(QPointF(0, 0));
        m_scene.setSceneRect(m_scene.sceneRect().united(rect));
        view->centerOn(m_scene.itemsBoundingRect().center());
    }
    m_scene.clearSelection();
    if (isPlaying) {
        m_scene.simulationController()->start();
    }
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

void WorkSpace::selectWorkspace()
{
    ICManager::wakeUp();
}

void WorkSpace::setAutosaveFileName()
{
    qCDebug(zero) << tr("Defining autosave path.");
    qCDebug(zero) << tr("Default file does not exist:") << m_fileInfo.absoluteFilePath();
    QDir autosavePath(QDir::temp());
    qCDebug(zero) << tr("Autosavepath:") << autosavePath.absolutePath();
    m_autosaveFile.setFileTemplate(autosavePath.absoluteFilePath(".XXXXXX.panda"));
    qCDebug(zero) << tr("Setting current file to random file in tmp.");
}

void WorkSpace::autosave()
{
    qCDebug(two) << tr("Starting autosave.");
    QStringList autosaves = Settings::value("autosaveFile").toStringList();
    qCDebug(zero) << tr("Cheking if autosavefile exists and if it contains current project file. If so, remove autosavefile from it.");
    qCDebug(three) << tr("All auto save file names before autosaving:") << autosaves;

    if (!m_autosaveFile.fileName().isEmpty() && autosaves.contains(m_autosaveFile.fileName())) {
        qCDebug(three) << tr("Removing current autosave file name.");
        autosaves.removeAll(m_autosaveFile.fileName());
        Settings::setValue("autosaveFile", autosaves);
    }

    qCDebug(zero) << tr("All auto save file names after possibly removing autosave:") << autosaves;
    qCDebug(zero) << tr("If autosave exists and undo stack is clean, remove it.");
    auto *undoStack = scene()->undoStack();
    qCDebug(zero) << tr("undostack element:") << undoStack->index() << tr("of") << undoStack->count();

    if (undoStack->isClean()) {
        emit fileChanged(m_fileInfo.exists() ? m_fileInfo.fileName() : tr("New Project"));
        qCDebug(three) << tr("Undo stack is clean.");

        if (m_autosaveFile.exists()) {
            m_autosaveFile.remove();
        }

        return;
    }

    qCDebug(three) << tr("Undo is !clean. Must set autosave file.");

    if (m_autosaveFile.exists()) {
        qCDebug(three) << tr("Autosave file already exists. Delete it to update.");
        m_autosaveFile.remove();
    }

    if (m_fileInfo.fileName().isEmpty()) {
        qCDebug(three) << tr("Default value not set yet.");
        QDir path(QDir::temp());
        qCDebug(three) << tr("Autosavepath:") << path.absolutePath();
        m_autosaveFile.setFileTemplate(path.absoluteFilePath(".XXXXXX.panda"));
        qCDebug(three) << tr("Setting current file to random file in tmp.");
    } else {
        qCDebug(three) << tr("Autosave path set to the current file's directory, if there is one.");
        QDir path(m_fileInfo.absolutePath());
        qCDebug(three) << tr("Autosavepath:") << path.absolutePath();
        m_autosaveFile.setFileTemplate(path.absoluteFilePath("." + m_fileInfo.baseName() + ".XXXXXX.panda"));
        qCDebug(three) << tr("Setting current file to:") << m_fileInfo.absoluteFilePath();
    }

    if (!m_autosaveFile.open()) {
        throw Pandaception(tr("Error opening autosave file: ") + m_autosaveFile.errorString());
    }

    qCDebug(three) << tr("Writing to autosave file.");
    QDataStream stream(&m_autosaveFile);
    QString autosaveFileName = m_autosaveFile.fileName();

    try {
        save(stream, dolphinFileName());
    } catch (std::runtime_error &e) {
        qCDebug(zero) << tr("Error autosaving project:") << e.what();
        m_autosaveFile.remove();
        throw;
    }

    m_autosaveFile.close();
    autosaves.append(autosaveFileName);
    Settings::setValue("autosaveFile", autosaves);
    qCDebug(three) << tr("All auto save file names after adding autosave:") << autosaves;

    emit fileChanged(m_fileInfo.exists() ? m_fileInfo.fileName() + "*" : tr("New Project*"));
}

void WorkSpace::setIsAutosave()
{
    if (m_autosaveFile.exists()) {
        m_autosaveFile.remove();
    }

    m_autosaveFile.setFileName(m_fileInfo.filePath());
}
