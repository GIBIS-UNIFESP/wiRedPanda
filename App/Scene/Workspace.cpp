// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/Workspace.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QSaveFile>
#include <QScrollBar>
#include <QStandardPaths>
#include <QUuid>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/SentryHelpers.h"
#include "App/Core/Settings.h"
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
#include "App/UI/FileDialogProvider.h"
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

    // Coalesce bursts of changes into one autosave write — a multi-element
    // paste, drag-rotate, or rapid typing in a label otherwise spams the
    // disk and widens the window for partial-write corruption.
    m_autosaveDebounceTimer.setSingleShot(true);
    m_autosaveDebounceTimer.setInterval(500);
    connect(&m_autosaveDebounceTimer, &QTimer::timeout, this, &WorkSpace::autosave);
    connect(&m_scene, &Scene::circuitHasChanged, &m_autosaveDebounceTimer, qOverload<>(&QTimer::start));

    setAutosaveFileName();

    m_scene.setLastId(m_lastId);
}

WorkSpace::~WorkSpace()
{
    flushPendingAutosave();
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

bool WorkSpace::isFromNewerVersion() const
{
    return !m_loadedVersion.isNull() && m_loadedVersion > FileVersion::current;
}

QFileInfo WorkSpace::fileInfo()
{
    return m_fileInfo;
}

void WorkSpace::save(const QString &fileName)
{
    sentryBreadcrumb("file", QStringLiteral("Save: %1").arg(fileName));

    // The user save supersedes any pending autosave; cancel it so the timer
    // doesn't fire after we've removed the autosave file and re-create it.
    m_autosaveDebounceTimer.stop();

    if (isFromNewerVersion()) {
        if (Application::interactiveMode) {
            const QString message = tr("This file was saved with a newer file format (version %1).\n"
                         "Your wiRedPanda version (%2) supports file format %3.\n\n"
                         "Please update wiRedPanda to save changes to this file.")
                          .arg(m_loadedVersion.toString(), AppVersion::current.toString(), FileVersion::current.toString());
            QMessageBox::warning(this, tr("Cannot save."), message);
        }
        return;
    }

    if (m_isInlineIC) {
        if (!m_parentWorkspace) {
            qCWarning(zero) << "Inline IC tab: parent workspace was closed. Save is a no-op.";
            return;
        }

        // Inline-IC tabs serialize to a blob and emit a signal instead of writing to disk.
        const QString contextDir = m_scene.contextDir();

        // Embed any file-backed ICs so the blob is self-contained
        for (auto *elm : m_scene.elements()) {
            if (elm->elementType() == ElementType::IC && !elm->isEmbedded()) {
                auto *ic = static_cast<IC *>(elm);
                const QString icFile = ic->file();
                const QString baseName = QFileInfo(icFile).baseName();
                if (!m_scene.icRegistry()->hasBlob(baseName)) {
                    QFileInfo fi(QDir(contextDir), icFile);
                    if (fi.exists()) {
                        QFile f(fi.absoluteFilePath());
                        if (f.open(QIODevice::ReadOnly)) {
                            m_scene.icRegistry()->registerBlob(baseName, f.readAll());
                        }
                    }
                }
                // Switch the IC to blob-backed for serialization; do NOT call
                // loadFromBlob. The IC already has its internal state loaded
                // from the same file we just registered as a blob, so a reload
                // would only destroy and rebuild ports — which cascade-deletes
                // every scene wire connected to the IC (silent data loss) and
                // races the simulation tick when play is running (H2-shape crash).
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
            fileName_ = FileDialogs::provider()->getSaveFileName(this, tr("Save File"), path, tr("Panda files (*.panda)")).fileName;
        }
    }

    if (fileName_.isEmpty()) {
        return;
    }

    if (!fileName_.endsWith(".panda")) {
        fileName_.append(".panda");
    }

    // Copy external file dependencies (appearances, audio, IC sub-circuits, waveform)
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
        if (Application::interactiveMode && saveFile.error() == QFileDevice::PermissionsError) {
            // OneDrive lock, ZIP-extracted folder, network drive, write-protected
            // attribute. Re-prompt for a writable location instead of throwing
            // the user into a stuck "Acesso negado" dialog with no way out.
            const QString newPath = FileDialogs::provider()->getSaveFileName(
                this, tr("Save File (original location is read-only)"),
                QFileInfo(fileName_).fileName(),
                tr("Panda files (*.panda)")).fileName;
            if (!newPath.isEmpty()) {
                save(newPath);
            }
            return;
        }
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
        if (Application::interactiveMode && saveFile.error() == QFileDevice::PermissionsError) {
            const QString newPath = FileDialogs::provider()->getSaveFileName(
                this, tr("Save File (original location is read-only)"),
                QFileInfo(fileName_).fileName(),
                tr("Panda files (*.panda)")).fileName;
            if (!newPath.isEmpty()) {
                save(newPath);
            }
            return;
        }
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

    if (!m_autosaveFileName.isEmpty() && QFile::exists(m_autosaveFileName)) {
        qCDebug(zero) << "Remove autosave from settings and delete it.";
        autosaves.removeAll(m_autosaveFileName);
        Settings::setAutosaveFiles(autosaves);
        QFile::remove(m_autosaveFileName);
        m_autosaveFileName.clear();
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

    // Extract port metadata from Input/Output elements in the scene,
    // sorted by Y/X position to match IC runtime port order.
    const auto portMeta = IC::buildPortMetadata(m_scene.elements());
    if (portMeta.inputCount > 0 || portMeta.outputCount > 0) {
        metadata["inputCount"] = portMeta.inputCount;
        metadata["outputCount"] = portMeta.outputCount;
        metadata["inputLabels"] = portMeta.inputLabels;
        metadata["outputLabels"] = portMeta.outputLabels;
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
    m_loadedVersion = version;

    bool needsMigration = (version < FileVersion::current) && Application::migrationEnabled;
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
        if (version > FileVersion::current) {
            const QString fmtVersion = FileVersion::current.toString();
            const QString fileVersion = version.toString();
            const QString message = tr("This file was saved with a newer file format (version %1).\n"
                         "Your version supports file format %2.\n\n"
                         "The file will be opened but saving is blocked.\n"
                         "Please update wiRedPanda to edit and save this file.")
                          .arg(fileVersion, fmtVersion);
            QMessageBox::warning(this, tr("Newer version file."), message);
        } else if (version < FileVersion::current) {
            const QString backupFileName = m_fileInfo.completeBaseName() + ".v" + version.toString() + "." + m_fileInfo.suffix();
            const QString message = tr("This file is in an older format (version %1) and will be automatically upgraded to the current format (version %2).\n"
                         "A backup of the original file has been created with name: %3")
                         .arg(version.toString(), FileVersion::current.toString(), backupFileName);
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
    // Eagerly ensure the global autosaves directory exists; the actual filename
    // is computed lazily inside autosave() when the workspace first turns dirty.
    QDir autosavePath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/autosaves");
    if (!autosavePath.exists()) {
        autosavePath.mkpath(autosavePath.absolutePath());
    }
    m_autosaveFileName.clear();
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

    if (isFromNewerVersion()) {
        return; // Autosaving a newer-version file would lose data
    }

    qCDebug(two) << "Starting autosave.";
    QStringList autosaves = Settings::autosaveFiles();
    qCDebug(three) << "All auto save file names before autosaving: " << autosaves;

    auto *undoStack = m_scene.undoStack();
    qCDebug(zero) << "Undo stack element: " << undoStack->index() << " of " << undoStack->count();

    // If the undo stack is clean the project has no unsaved changes, so there's
    // nothing to protect; delete any leftover autosave file and bail out.
    if (undoStack->isClean()) {
        qCDebug(three) << "Undo stack is clean.";
        if (!m_autosaveFileName.isEmpty()) {
            autosaves.removeAll(m_autosaveFileName);
            Settings::setAutosaveFiles(autosaves);
            QFile::remove(m_autosaveFileName);
            m_autosaveFileName.clear();
        }
        emit fileChanged(m_fileInfo);
        return;
    }

    qCDebug(three) << "Undo is !clean. Must set autosave file.";

    // Choose the directory to write into, mirroring the pre-existing policy:
    // unsaved projects go to the global autosaves dir, saved projects go next
    // to their .panda file unless that directory is read-only.
    QDir path;
    if (m_fileInfo.fileName().isEmpty()) {
        path.setPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/autosaves");
    } else {
        const QFileInfo dirInfo(m_fileInfo.absolutePath());
        if (dirInfo.isWritable()) {
            path.setPath(m_fileInfo.absolutePath());
        } else {
            path.setPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/autosaves");
        }
    }
    if (!path.exists()) {
        path.mkpath(path.absolutePath());
    }
    qCDebug(three) << "Autosavepath: " << path.absolutePath();

    // Reuse a stable filename across writes so QSaveFile can replace the same
    // target atomically. If the project's directory has changed (Save As to a
    // new path), drop the old autosave file in the previous location first.
    const QString prefix = m_fileInfo.fileName().isEmpty() ? QStringLiteral(".") : "." + m_fileInfo.baseName() + ".";
    if (!m_autosaveFileName.isEmpty() && QFileInfo(m_autosaveFileName).absolutePath() != path.absolutePath()) {
        autosaves.removeAll(m_autosaveFileName);
        QFile::remove(m_autosaveFileName);
        m_autosaveFileName.clear();
    }
    if (m_autosaveFileName.isEmpty()) {
        const QString tag = QUuid::createUuid().toString(QUuid::Id128);
        m_autosaveFileName = path.absoluteFilePath(prefix + tag + ".panda");
    }

    // Drop the previous registry entry before writing so we don't double-list it.
    if (autosaves.contains(m_autosaveFileName)) {
        autosaves.removeAll(m_autosaveFileName);
        Settings::setAutosaveFiles(autosaves);
    }

    qCDebug(three) << "Writing to autosave file.";
    // QSaveFile writes to a sibling temp file and atomically renames on commit,
    // truncating any prior contents. Both partial-write corruption (process
    // killed mid-write) and shrink-leftover-tail corruption (new circuit
    // shorter than the previous autosave) are eliminated.
    QSaveFile autosaveFile(m_autosaveFileName);
    if (!autosaveFile.open(QIODevice::WriteOnly)) {
        throw PANDACEPTION("Error opening autosave file: %1", autosaveFile.errorString());
    }

    QDataStream stream(&autosaveFile);
    Serialization::writePandaHeader(stream);
    save(stream);

    if (!autosaveFile.commit()) {
        throw PANDACEPTION("Could not commit autosave file: %1", autosaveFile.errorString());
    }

    autosaves.append(m_autosaveFileName);
    Settings::setAutosaveFiles(autosaves);

    qCDebug(three) << "All auto save file names after adding autosave: " << autosaves;

    emit fileChanged(m_fileInfo);
}

void WorkSpace::setAutosaveFile()
{
    m_autosaveFileName = m_fileInfo.filePath();
}

void WorkSpace::flushPendingAutosave()
{
    if (m_autosaveDebounceTimer.isActive()) {
        m_autosaveDebounceTimer.stop();
        autosave();
    }
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

    // Update the registry and reload all targets atomically: if any loadFromBlob
    // throws, roll back every element that was already updated so we don't push a
    // partial undo command (mirrors the pattern in ICRegistry::embedICsByFile).
    m_scene.icRegistry()->setBlob(targetBlobName, blob);
    QList<GraphicElement *> updated;
    try {
        for (auto *target : targets) {
            auto *ic = static_cast<IC *>(target);
            ic->loadFromBlob(blob, m_scene.contextDir());
            updated.append(target);
        }
    } catch (...) {
        ICRegistry::rollbackElements(updated, oldData, &m_scene);
        m_scene.icRegistry()->setBlob(targetBlobName, oldBlob);
        throw;
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

    const bool hasBlob = m_scene.icRegistry()->hasBlob(blobName);
    if (toDelete.isEmpty() && !hasBlob) {
        return;
    }

    // Pair the IC deletion with blob removal in a single macro so undo
    // restores both — eagerly removing the blob outside the command would
    // leave restored ICs pointing at a registry entry that no longer exists.
    m_scene.undoStack()->beginMacro(tr("Remove embedded IC \"%1\"").arg(blobName));
    if (!toDelete.isEmpty()) {
        m_scene.receiveCommand(new DeleteItemsCommand(toDelete, &m_scene));
    }
    if (hasBlob) {
        m_scene.receiveCommand(new RemoveBlobCommand(blobName, &m_scene));
    }
    m_scene.undoStack()->endMacro();
}

void WorkSpace::setCurrentFile(const QString &filePath)
{
    m_fileInfo = QFileInfo(filePath);
    m_scene.setContextDir(m_fileInfo.absolutePath());
}

