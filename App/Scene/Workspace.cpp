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
#include "App/Core/SentryHelpers.h"
#include "App/Core/Settings.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/Element/ICRegistry.h"
#include "App/IO/FileUtils.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Commands.h"
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

    m_scene.setLastId(m_lastId);
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
    sentryBreadcrumb("file", QStringLiteral("Save: %1").arg(fileName));

    if (m_isInlineIC) {
        if (!m_parentWorkspace) {
            qCWarning(zero) << "Inline IC tab: parent workspace was closed. Save is a no-op.";
            return;
        }

        // Inline-IC tabs serialize to a blob and emit a signal instead of writing to disk.
        const QString savedContextDir = m_scene.contextDir();
        auto restoreCtx = qScopeGuard([&] { m_scene.setContextDir(savedContextDir); });
        m_scene.setContextDir(m_parentContextDir);

        // Embed any file-backed ICs so the blob is self-contained
        for (auto *elm : m_scene.elements()) {
            if (elm->elementType() == ElementType::IC && !elm->isEmbedded()) {
                auto *ic = static_cast<IC *>(elm);
                const QString icFile = ic->file();
                const QString baseName = QFileInfo(icFile).baseName();
                if (!m_scene.icRegistry()->hasBlob(baseName)) {
                    QFileInfo fi(QDir(m_parentContextDir), icFile);
                    if (fi.exists()) {
                        QFile f(fi.absoluteFilePath());
                        if (f.open(QIODevice::ReadOnly)) {
                            m_scene.icRegistry()->registerBlob(baseName, f.readAll());
                        }
                    }
                }
                ic->setBlobName(baseName);
            }
        }

        // Serialize as a full .panda file (header + metadata + elements)
        QByteArray blob;
        QDataStream stream(&blob, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);

        QMap<QString, QVariant> metadata;
        Serialization::serializeBlobRegistry(m_scene.icRegistry()->blobMap(), metadata);
        stream << metadata;

        Serialization::serialize(m_scene.items(), stream);

        m_scene.undoStack()->setClean();
        emit icBlobSaved(m_parentICElementId, blob);
        return;
    }

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

    // Copy external file dependencies (skins, audio, IC sub-circuits, waveform)
    // to the new directory before updating contextDir, so save() can store bare filenames.
    const QString oldContextDir = m_scene.contextDir();
    const QString newContextDir = QFileInfo(fileName_).absolutePath();
    if (!oldContextDir.isEmpty() && oldContextDir != newContextDir) {
        for (auto *elm : m_scene.elements()) {
            for (const QString &file : elm->externalFiles()) {
                FileUtils::copyToDir(file, newContextDir);
                if (file.endsWith(".panda")) {
                    FileUtils::copyPandaDeps(file, oldContextDir, newContextDir);
                }
            }
        }

        // Copy the associated BeWavedDolphin waveform file if present
        if (!m_dolphinFileName.isEmpty()) {
            const QString resolved = QDir(oldContextDir).absoluteFilePath(m_dolphinFileName);
            FileUtils::copyToDir(resolved, newContextDir);
        }
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
    // Metadata section: all file-level fields stored as key-value pairs.
    QMap<QString, QVariant> metadata;

    if (!m_dolphinFileName.isEmpty()) {
        metadata["dolphinFileName"] = m_dolphinFileName;
    }

    // Extract port metadata from Input/Output elements in the scene
    {
        int inputCount = 0, outputCount = 0;
        QStringList inputLabels, outputLabels;
        for (auto *elm : m_scene.elements()) {
            if (elm->elementGroup() == ElementGroup::Input) {
                for (int i = 0; i < elm->outputSize(); ++i) {
                    ++inputCount;
                    QString lb = elm->label().isEmpty() ? ElementFactory::translatedName(elm->elementType()) : elm->label();
                    if (elm->outputSize() > 1 && !elm->outputPort(i)->name().isEmpty()) {
                        lb += " " + elm->outputPort(i)->name();
                    }
                    inputLabels.append(lb);
                }
            } else if (elm->elementGroup() == ElementGroup::Output) {
                for (int i = 0; i < elm->inputSize(); ++i) {
                    ++outputCount;
                    QString lb = elm->label().isEmpty() ? ElementFactory::translatedName(elm->elementType()) : elm->label();
                    if (elm->inputSize() > 1 && !elm->inputPort(i)->name().isEmpty()) {
                        lb += " " + elm->inputPort(i)->name();
                    }
                    outputLabels.append(lb);
                }
            }
        }
        if (inputCount > 0 || outputCount > 0) {
            metadata["inputCount"] = inputCount;
            metadata["outputCount"] = outputCount;
            metadata["inputLabels"] = inputLabels;
            metadata["outputLabels"] = outputLabels;
        }
    }

    Serialization::serializeBlobRegistry(m_scene.icRegistry()->blobMap(), metadata);

    // Collect unique file-backed IC filenames for copyFiles (Save As).
    QStringList fileBackedICs;
    for (auto *elm : m_scene.elements()) {
        if (elm->elementType() == ElementType::IC && !elm->isEmbedded()) {
            const QString icFile = static_cast<IC *>(elm)->file();
            if (!icFile.isEmpty() && !fileBackedICs.contains(QFileInfo(icFile).fileName())) {
                fileBackedICs.append(QFileInfo(icFile).fileName());
            }
        }
    }
    if (!fileBackedICs.isEmpty()) {
        metadata["fileBackedICs"] = fileBackedICs;
    }

    stream << metadata;

    Serialization::serialize(m_scene.items(), stream);
}

void WorkSpace::load(const QString &fileName)
{
    sentryBreadcrumb("file", QStringLiteral("Load: %1").arg(fileName));

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

    bool needsMigration = (version < AppVersion::current) && Application::migrationEnabled;
    if (needsMigration) {
        createVersionedBackup(fileName, version);
    }

    load(stream, version, QFileInfo(fileName).absolutePath());
    file.close();

    if (needsMigration) {
        save(fileName);  // Re-save in new format
    }

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
        } else if (version < AppVersion::current) {
            const QString backupFileName = m_fileInfo.completeBaseName() + ".v" + version.toString() + "." + m_fileInfo.suffix();
            const QString message = tr("This file is in an older format (version %1) and will be automatically upgraded to the current format (version %2).\n"
                         "A backup of the original file has been created with name: %3")
                         .arg(version.toString(), AppVersion::current.toString(), backupFileName);
            QMessageBox::information(this, tr("File upgraded."), message);
        }
    }

    // V4.6+ stores all file-level fields in the metadata map.
    // Older versions wrote dolphinFileName and sceneRect positionally before the map.
    QMap<QString, QVariant> metadata;
    if (VersionInfo::hasUnifiedMetadata(version)) {
        stream >> metadata;
        m_dolphinFileName = metadata.value("dolphinFileName").toString();
    } else {
        m_dolphinFileName = Serialization::loadDolphinFileName(stream, version);
        Serialization::loadRect(stream, version);
        if (VersionInfo::hasMetadata(version)) {
            stream >> metadata;
        }
    }
    qCDebug(zero) << "Dolphin name: " << m_dolphinFileName;

    QMap<QString, QByteArray> blobRegistry = Serialization::deserializeBlobRegistry(metadata);

    // Populate the scene's IC registry with embedded IC blobs
    for (auto it = blobRegistry.cbegin(); it != blobRegistry.cend(); ++it) {
        m_scene.icRegistry()->setBlob(it.key(), it.value());
    }

    QMap<quint64, QNEPort *> portMap;
    if (!contextDir.isEmpty()) {
        m_scene.setContextDir(contextDir);
    }
    auto context = m_scene.deserializationContext(portMap, version);
    context.contextDir = contextDir;
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

    m_scene.setLastId(m_lastId);

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
    if (m_isInlineIC) {
        return; // Inline IC tabs don't autosave to disk
    }

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

void WorkSpace::createVersionedBackup(const QString &fileName, const QVersionNumber &version)
{
    Serialization::createVersionedBackup(fileName, version);
}

void WorkSpace::loadFromBlob(const QByteArray &blob, WorkSpace *parent, int icElementId, const QString &parentContextDir)
{
    SimulationBlocker simulationBlocker(m_scene.simulation());

    if (!parentContextDir.isEmpty()) {
        m_scene.setContextDir(parentContextDir);
    }

    // Blob is a full .panda file
    QByteArray blobData(blob);
    QDataStream stream(&blobData, QIODevice::ReadOnly);
    const auto preamble = Serialization::readPreamble(stream);

    const auto blobRegistry = Serialization::deserializeBlobRegistry(preamble.metadata);
    for (auto it = blobRegistry.cbegin(); it != blobRegistry.cend(); ++it) {
        m_scene.icRegistry()->setBlob(it.key(), it.value());
    }

    QMap<quint64, QNEPort *> portMap;
    auto context = m_scene.deserializationContext(portMap, preamble.version);
    context.contextDir = parentContextDir;
    const auto items = Serialization::deserialize(stream, context);

    for (auto *item : items) {
        m_scene.addItem(item);

        if (auto *ge = qgraphicsitem_cast<GraphicElement *>(item)) {
            m_lastId = qMax(m_lastId, ge->id());
        }
    }

    m_scene.setLastId(m_lastId);
    m_scene.setSceneRect(m_scene.itemsBoundingRect());

    // Set inline-IC mode after successful deserialization
    m_isInlineIC = true;
    m_parentWorkspace = parent;
    m_parentICElementId = icElementId;
    m_parentContextDir = parentContextDir;

    // Derive blob name from the parent IC element
    if (parent) {
        if (auto *item = parent->scene()->itemById(icElementId)) {
            if (auto *elm = dynamic_cast<GraphicElement *>(item)) {
                m_inlineBlobName = elm->blobName();
            }
        }
    }
}

void WorkSpace::onChildICBlobSaved(int icElementId, const QByteArray &blob)
{
    auto *item = m_scene.itemById(icElementId);
    if (!item) {
        return; // Orphaned child tab — IC was deleted or undone
    }

    auto *elm = dynamic_cast<GraphicElement *>(item);
    if (!elm || !elm->isEmbedded()) {
        return;
    }

    const QString targetBlobName = elm->blobName();
    const auto targets = m_scene.icRegistry()->findICsByBlobName(targetBlobName);
    if (targets.isEmpty()) {
        return;
    }

    const auto connections = UpdateBlobCommand::captureConnections(targets);

    SimulationBlocker simulationBlocker(m_scene.simulation());

    const QByteArray oldData = ICRegistry::captureSnapshot(targets);
    QByteArray oldBlob = m_scene.icRegistry()->blob(targetBlobName);

    // Update the registry and reload all targets
    m_scene.icRegistry()->setBlob(targetBlobName, blob);
    for (auto *target : targets) {
        auto *ic = static_cast<IC *>(target);
        ic->loadFromBlob(blob, m_scene.contextDir());
    }

    auto *cmd = new UpdateBlobCommand(targets, oldData, connections, &m_scene);
    cmd->setOldBlob(oldBlob);
    m_scene.undoStack()->push(cmd);
}

void WorkSpace::removeEmbeddedIC(const QString &blobName)
{
    QList<QGraphicsItem *> toDelete;

    for (auto *item : m_scene.items()) {
        if (item->type() != GraphicElement::Type) {
            continue;
        }
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (elm && elm->isEmbedded() && elm->blobName() == blobName) {
            toDelete.append(item);
            // Also collect connections to this element
            for (int i = 0; i < elm->inputSize(); ++i) {
                for (auto *conn : elm->inputPort(i)->connections()) {
                    if (!toDelete.contains(conn)) {
                        toDelete.append(conn);
                    }
                }
            }
            for (int i = 0; i < elm->outputSize(); ++i) {
                for (auto *conn : elm->outputPort(i)->connections()) {
                    if (!toDelete.contains(conn)) {
                        toDelete.append(conn);
                    }
                }
            }
        }
    }

    if (!toDelete.isEmpty()) {
        m_scene.receiveCommand(new DeleteItemsCommand(toDelete, &m_scene));
    }

    m_scene.icRegistry()->removeBlob(blobName);
}

void WorkSpace::setCurrentFile(const QString &filePath)
{
    m_fileInfo = QFileInfo(filePath);
    m_scene.setContextDir(m_fileInfo.absolutePath());
}

