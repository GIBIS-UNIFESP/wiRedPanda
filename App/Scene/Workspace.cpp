// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/Workspace.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSaveFile>
#include <QScrollBar>
#include <QStandardPaths>
#include <QTemporaryFile>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/Settings.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Nodes/QNEPort.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/Versions.h"

WorkSpace::WorkSpace(QWidget *parent)
    : QWidget(parent)
{
    m_view.setCacheMode(QGraphicsView::CacheBackground);
    m_view.setScene(&m_scene);
    // Back-pointer lets the scene query the view (e.g., for zoom level in drawBackground)
    m_scene.setView(&m_view);
    m_scene.setSceneRect(m_view.rect());
    setLayout(new QHBoxLayout());
    layout()->addWidget(&m_view);

    // Adjust the scene rect after every zoom so that all items remain reachable
    // via panning, even when zoomed in very close
    connect(&m_view, &GraphicsView::zoomChanged, &m_scene, &Scene::resizeScene);

    // Trigger autosave on every change; autosave() checks the undo stack cleanliness
    // before actually writing to disk
    connect(&m_scene, &Scene::circuitHasChanged, this, &WorkSpace::autosave);

    setAutosaveFileName();

    // Ensure the global element-ID counter starts at least at the workspace's last known ID
    // so that a new WorkSpace in the same process session doesn't reuse existing IDs
    ElementFactory::setLastId(m_lastId);
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
    QStringList autosaves = Settings::autosaveFiles();
    qCDebug(zero) << "All auto save file names before save: " << autosaves;

    QString autosaveFileName;
    qCDebug(zero) << "Checking if it is an autosave file or a new project, and ask for a fileName.";

    // If saving to an autosave path or no path at all, prompt the user for a real filename
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

    setCurrentFile(fileName_);

    // QSaveFile writes to a temp file and commits atomically, preventing data loss
    // if the process is interrupted during a write
    QSaveFile saveFile(fileName_);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        throw PANDACEPTION("Error opening file: %1", saveFile.errorString());
    }

    // Set scene rect with same logic as resizeScene() to avoid viewport jump on element selection.
    // Tighten to item bounds with margins and visible viewport to ensure consistent scrollbar behavior.
    auto bounds = m_scene.itemsBoundingRect();
    auto tightRect = bounds;
    const auto viewList = m_scene.views();
    if (!viewList.isEmpty()) {
        auto *view = viewList.first();
        constexpr qreal margin = 100.0;
        const auto visibleScene = view->mapToScene(view->viewport()->rect()).boundingRect()
                                     .adjusted(-margin, -margin, margin, margin);
        tightRect = tightRect.united(visibleScene);

        const int hVal = view->horizontalScrollBar()->value();
        const int vVal = view->verticalScrollBar()->value();
        m_scene.setSceneRect(tightRect);
        view->horizontalScrollBar()->setValue(hVal);
        view->verticalScrollBar()->setValue(vVal);
    } else {
        m_scene.setSceneRect(tightRect);
    }

    QDataStream stream(&saveFile);
    Serialization::writePandaHeader(stream);
    save(stream);

    if (!saveFile.commit()) {
        throw PANDACEPTION("Could not save file: %1", saveFile.errorString());
    }

    // Mark the undo stack as clean so the title bar no longer shows unsaved-change indicator
    m_scene.undoStack()->setClean();

    // Clean up autosave records associated with this project now that it has a real file
    if (!autosaveFileName.isEmpty()) {
        qCDebug(zero) << "Remove from autosave list recovered file that has been saved.";
        autosaves.removeAll(autosaveFileName);
        Settings::setAutosaveFiles(autosaves);
        qCDebug(zero) << "All auto save file names after removing recovered: " << autosaves;
    }

    if (m_autosaveFile.exists()) {
        qCDebug(zero) << "Remove autosave from settings and delete it.";
        autosaves.removeAll(m_autosaveFile.fileName());
        Settings::setAutosaveFiles(autosaves);
        m_autosaveFile.remove();
        qCDebug(zero) << "All auto save file names after removing autosave: " << autosaves;
    }

    emit fileChanged(m_fileInfo);
}

void WorkSpace::save(QDataStream &stream)
{
    // The dolphin filename is the path of the beWavedDolphin waveform file that is
    // "associated" with this circuit; it is serialized here so the waveform tool can
    // be opened automatically when the circuit is loaded.  Empty string means none.
    stream << m_dolphinFileName;
    stream << m_scene.sceneRect();
    Serialization::serialize(m_scene.items(), stream);
}

void WorkSpace::load(const QString &fileName)
{
    QFile file(fileName);

    if (!file.exists()) {
        qCDebug(zero) << "This file does not exist: " << fileName;
        throw PANDACEPTION("This file does not exist: %1", fileName);
    }

    setCurrentFile(fileName);

    qCDebug(zero) << "File exists.";

    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(zero) << "Could not open file: " << file.errorString();
        throw PANDACEPTION("Could not open file: %1", file.errorString());
    }

    QDataStream stream(&file);
    QVersionNumber version = Serialization::readPandaHeader(stream);
    load(stream, version, QFileInfo(fileName).absolutePath());

    emit fileChanged(m_fileInfo);
}

void WorkSpace::load(QDataStream &stream, const QVersionNumber &version, const QString &contextDir)
{
    qCDebug(zero) << "Loading file.";
    // Block simulation updates while items are being added to avoid intermediate
    // partial-topology updates that could crash or produce incorrect output
    SimulationBlocker simulationBlocker(m_scene.simulation());
    qCDebug(zero) << "Stopped simulation.";
    qCDebug(zero) << "Version: " << version;

    if (Application::interactiveMode) {
        if (version > AppVersion::current) {
            const QString progVersion = AppVersion::current.toString();
            const QString fileVersion = version.toString();
            const QString message = tr("Warning! Your wiRedPanda (Version: %1) is possibly out of date.\n"
                         "The file you are opening was saved with version (%2).\n"
                         "Please check for updates if the file does not load correctly.")
                          .arg(progVersion, fileVersion);
            QMessageBox::warning(this, tr("Newer version file."), message);
        } else if (version < Versions::V_4_0) {
            const QString message = tr("Warning! This is an old version wiRedPanda project file (version < 4.0). "
                         "To open it correctly, save all the ICs and skins in the main project directory.");
            QMessageBox::warning(this, tr("Old version file."), message);
        }
    }

    m_dolphinFileName = Serialization::loadDolphinFileName(stream, version);
    qCDebug(zero) << "Dolphin name: " << m_dolphinFileName;

    // loadRect reads (and discards) the stored scene rect; the actual rect is
    // recomputed from items after they are added because items may have moved
    // relative to the stored rect (e.g., old files with a different viewport origin).
    Serialization::loadRect(stream, version);
    if (!contextDir.isEmpty()) {
        m_scene.setContextDir(contextDir);
    }
    QMap<quint64, QNEPort *> portMap;
    SerializationContext context{portMap, version, contextDir};
    const auto items = Serialization::deserialize(stream, context);
    qCDebug(zero) << "Finished loading items.";

    for (auto *item : items) {
        m_scene.addItem(item);

        // Track the highest element ID seen so that newly created elements
        // will receive IDs that don't collide with those just loaded
        if (auto *ge = qgraphicsitem_cast<GraphicElement *>(item)) {
            m_lastId = qMax(m_lastId, ge->id());
        }
    }

    ElementFactory::setLastId(m_lastId);

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
    // The leading dot makes the autosave file hidden on Unix; XXXXXX is replaced
    // by QTemporaryFile with a unique suffix to prevent collisions between workspaces
    m_autosaveFile.setFileTemplate(autosavePath.absoluteFilePath(".XXXXXX.panda"));
    qCDebug(zero) << "Setting current file to random file.";
}

int WorkSpace::lastId() const
{
    return m_lastId;
}

void WorkSpace::setLastId(int newLastId)
{
    m_lastId = newLastId;
}

void WorkSpace::autosave()
{
    qCDebug(two) << "Starting autosave.";
    QStringList autosaves = Settings::autosaveFiles();
    qCDebug(three) << "All auto save file names before autosaving: " << autosaves;

    qCDebug(zero) << "Checking if autosave file exists and if it contains current project file. If so, remove autosave file from it.";

    // Remove the stale autosave entry from the registry before creating the new one;
    // this prevents the registry from growing unboundedly on each autosave cycle
    if (!m_autosaveFile.fileName().isEmpty() && autosaves.contains(m_autosaveFile.fileName())) {
        qCDebug(three) << "Removing current autosave file name.";
        autosaves.removeAll(m_autosaveFile.fileName());
        Settings::setAutosaveFiles(autosaves);
    }

    qCDebug(zero) << "All auto save file names after possibly removing autosave: " << autosaves;
    qCDebug(zero) << "If autosave exists and undo stack is clean, remove it.";
    auto *undoStack = m_scene.undoStack();
    qCDebug(zero) << "Undo stack element: " << undoStack->index() << " of " << undoStack->count();

    // If the undo stack is clean the project has no unsaved changes, so there's
    // nothing to protect; delete any leftover autosave file and bail out
    if (undoStack->isClean()) {
        qCDebug(three) << "Undo stack is clean.";
        m_autosaveFile.remove();
        emit fileChanged(m_fileInfo);

        return;
    }

    qCDebug(three) << "Undo is !clean. Must set autosave file.";

    QDir path;

    if (m_fileInfo.fileName().isEmpty()) {
        // Unsaved new project: write autosave to the global autosave directory
        qCDebug(three) << "Default value not set yet.";
        path.setPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/autosaves");

        if (!path.exists()) {
            path.mkpath(path.absolutePath());
        }

        qCDebug(three) << "Autosavepath: " << path.absolutePath();
        m_autosaveFile.setFileTemplate(path.absoluteFilePath(".XXXXXX.panda"));
        qCDebug(three) << "Setting current file to random file.";
    } else {
        // Existing project: write autosave next to the real file so it's easy to find
        // in case of crash recovery
        qCDebug(three) << "Autosave path set to the current file's directory, if there is one.";
        path.setPath(m_fileInfo.absolutePath());
        qCDebug(three) << "Autosavepath: " << path.absolutePath();
        m_autosaveFile.setFileTemplate(path.absoluteFilePath("." + m_fileInfo.baseName() + ".XXXXXX.panda"));
        qCDebug(three) << "Setting current file to: " << m_fileInfo.absoluteFilePath();
    }

    if (!m_autosaveFile.open()) {
        throw PANDACEPTION("Error opening autosave file: %1", m_autosaveFile.errorString());
    }

    QString autosaveFileName = m_autosaveFile.fileName();

    qCDebug(three) << "Writing to autosave file.";
    QDataStream stream(&m_autosaveFile);
    Serialization::writePandaHeader(stream);
    save(stream);
    m_autosaveFile.close();

    autosaves.append(autosaveFileName);
    Settings::setAutosaveFiles(autosaves);

    qCDebug(three) << "All auto save file names after adding autosave: " << autosaves;

    emit fileChanged(m_fileInfo);
}

void WorkSpace::setAutosaveFile()
{
    m_autosaveFile.setFileName(m_fileInfo.filePath());
}

void WorkSpace::setCurrentFile(const QString &filePath)
{
    m_fileInfo = QFileInfo(filePath);
    m_scene.setContextDir(m_fileInfo.absolutePath());
}

