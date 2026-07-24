// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/Workspace.h"

#include <algorithm>

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
#include "App/Exercise/ExerciseOverlay.h"
#include "App/IO/FileUtils.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Scene/Commands.h"
#include "App/Scene/ICRegistry.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/UI/MinimapWidget.h"
#include "App/Versions.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

namespace {

/// Returns true for QFileDevice errors that map to "target is locked / not
/// writable" — the conditions where re-prompting the user for a different
/// path lets them recover instead of staring at a modal error. QSaveFile in
/// particular reports WriteError (not PermissionsError) when an existing
/// file is read-only.
bool isReadOnlyFailure(QFileDevice::FileError error)
{
    return error == QFileDevice::PermissionsError
        || error == QFileDevice::OpenError
        || error == QFileDevice::WriteError;
}

} // namespace

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

    // Minimap overview: small widget overlayed on the workspace so it remains
    // static even when the view's zoom changes. Positioning is computed
    // relative to the view geometry in resizeEvent().
    m_minimap = new MinimapWidget(&m_scene, &m_view, this);
    m_minimap->setObjectName("minimap");
    m_minimap->raise();
    connect(m_minimap, &MinimapWidget::geometryChangeFinished, this, &WorkSpace::onMinimapGeometryChangeFinished);

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

    /// Refresh the parent's tab title (which shows "*" while dirty) whenever
    /// the undo stack's clean state flips — needed for inline-IC saves that
    /// push UpdateBlobCommand to a non-current parent workspace.
    connect(m_scene.undoStack(), &QUndoStack::cleanChanged, this, [this](bool /*clean*/) {
        emit fileChanged(m_fileInfo);
    });

    setAutosaveFileName();

    m_scene.setLastId(m_lastId);
}

WorkSpace::~WorkSpace()
{
    /// Block signals for the rest of this object's lifetime — including member
    /// destruction below, where m_scene's QUndoStack emits cleanChanged on
    /// teardown. Without this, the cleanChanged → fileChanged chain would
    /// reach a parent MainWindow whose destructor has already begun.
    /// (RAII QSignalBlocker won't do: it releases when this body returns,
    /// before m_scene is destroyed.)
    blockSignals(true);

    /// m_minimap is a QObject child of this WorkSpace, so Qt would normally
    /// destroy it automatically -- but only as part of ~QObject(), which runs
    /// after all of WorkSpace's own members (including m_view and m_scene)
    /// have already been destructed. m_minimap holds a raw pointer to m_view
    /// and has an event filter installed on m_view's viewport, so destroying
    /// m_view first would leave that filter dereferencing a dangling
    /// GraphicsView mid-teardown. Delete it explicitly, here, before any
    /// member destruction begins.
    delete m_minimap;
    m_minimap = nullptr;

    /// Mirror the pre-debounce QTemporaryFile semantics: a clean Workspace
    /// destruction discards its autosave so it isn't recovered next launch.
    /// (On a real crash the destructor doesn't run, so the recovery file remains.)
    m_autosaveDebounceTimer.stop();
    if (!m_autosaveFileName.isEmpty()) {
        QStringList autosaves = Settings::autosaveFiles();
        autosaves.removeAll(m_autosaveFileName);
        Settings::setAutosaveFiles(autosaves);
        QFile::remove(m_autosaveFileName);
        m_autosaveFileName.clear();
    }
}

void WorkSpace::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    // Guard with isVisible(): MainWindow restores its geometry (including queuing a maximized
    // state) before this tab is ever created, so this widget's very first resize fires while
    // it's still not genuinely on screen -- carrying the pre-maximize "normal" size, not the
    // final one. Consuming applyMinimapGeometry()'s one-time restore against that stale size
    // would lose the persisted position for the rest of the session (subsequent resizes only
    // re-clamp, they don't re-read Settings). Skipping it here lets the real, later resize --
    // once the window manager actually applies the maximized geometry -- do the restore
    // instead. showEvent() below covers windows that never resize again after becoming visible.
    if (m_minimap && isVisible())
        applyMinimapGeometry();

    if (m_exerciseOverlay && m_exerciseOverlay->isVisible())
        m_exerciseOverlay->repositionToParent();
}

void WorkSpace::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // Backstop for windows that never resize again after becoming visible (e.g. a
    // non-maximized launch, where restoreGeometry() already applied the final size before
    // show()) -- resizeEvent() above would otherwise never get a chance to restore at all.
    // Deferred briefly so a genuine maximize resize (handled above) wins the race when it's
    // fast enough; applyMinimapGeometry() only restores once (m_minimapPositioned), so a
    // redundant call here just re-clamps the already-correct geometry.
    if (m_minimap)
        QTimer::singleShot(100, this, [this] { applyMinimapGeometry(); });
}

void WorkSpace::setMinimapVisible(bool visible)
{
    if (!m_minimap) return;
    m_minimap->setVisible(visible);
}

void WorkSpace::applyMinimapGeometry()
{
    if (!m_minimap)
        return;

    const int margin = 12;
    const QRect viewGeom = m_view.geometry();

    if (!m_minimapPositioned) {
        m_minimapPositioned = true;

        const QRect restored = Settings::minimapGeometry();
        if (restored.isValid()) {
            // Clamp size before position: a geometry persisted from a larger window/monitor
            // could otherwise still overflow, or push the position clamp negative.
            const int maxWidth = qMax(m_minimap->minimumWidth(), viewGeom.width() - 2 * margin);
            const int maxHeight = qMax(m_minimap->minimumHeight(), viewGeom.height() - 2 * margin);
            const int width = qBound(m_minimap->minimumWidth(), restored.width(), maxWidth);
            const int height = qBound(m_minimap->minimumHeight(), restored.height(), maxHeight);
            const int x = qBound(margin, restored.x(), viewGeom.width() - width - margin);
            const int y = qBound(margin, restored.y(), viewGeom.height() - height - margin);
            m_minimap->setGeometry(x, y, width, height);
            return;
        }

        // No persisted geometry (first launch, or never moved/resized): default to the
        // widget's own default size, anchored bottom-right.
        const int x = viewGeom.right() - m_minimap->width() - margin;
        const int y = viewGeom.bottom() - m_minimap->height() - margin;
        m_minimap->move(qMax(x, margin), qMax(y, margin));
        return;
    }

    // Subsequent resizes: re-clamp the minimap's own current geometry into the new bounds.
    // Deliberately does not re-read Settings -- that's only the persisted copy, refreshed on
    // user-driven moves/resizes (onMinimapGeometryChangeFinished()); re-reading it here on
    // every window resize would stomp legitimate in-session geometry with a stale value.
    const QRect current = m_minimap->geometry();
    const int maxWidth = qMax(m_minimap->minimumWidth(), viewGeom.width() - 2 * margin);
    const int maxHeight = qMax(m_minimap->minimumHeight(), viewGeom.height() - 2 * margin);
    const int width = qBound(m_minimap->minimumWidth(), current.width(), maxWidth);
    const int height = qBound(m_minimap->minimumHeight(), current.height(), maxHeight);
    const int x = qBound(margin, current.x(), viewGeom.width() - width - margin);
    const int y = qBound(margin, current.y(), viewGeom.height() - height - margin);
    m_minimap->setGeometry(x, y, width, height);
}

void WorkSpace::onMinimapGeometryChangeFinished(const QRect &geometry)
{
    Settings::setMinimapGeometry(geometry);
}

Scene *WorkSpace::scene()
{
    return &m_scene;
}

const Scene *WorkSpace::scene() const
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
    return !m_loadedVersion.isNull() && m_loadedVersion > FormatRev::current;
}

QFileInfo WorkSpace::fileInfo() const
{
    return m_fileInfo;
}

WorkSpace::SaveOutcome WorkSpace::save(const QString &fileName)
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
                          .arg(m_loadedVersion.toString(), AppVersion::current.toString(), FormatRev::current.toString());
            QMessageBox::warning(this, tr("Cannot save."), message);
        }
        return SaveOutcome::Saved;
    }

    if (m_isInlineIC) {
        if (!m_parentWorkspace) {
            qCWarning(zero) << "Inline IC tab: parent workspace was closed. Save is a no-op.";
            return SaveOutcome::Saved;
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
                    QFile f(fi.absoluteFilePath());
                    // setBlobName() below marks the IC embedded unconditionally; if the
                    // dependency can't actually be read, embedding it anyway would leave the
                    // blob metadata pointing at a name never registered in the IC registry —
                    // and, since isEmbedded() would then always be true, this block would never
                    // retry on any later save either. Fail loudly now instead of producing a
                    // blob that throws "not found" only when someone tries to reload it.
                    if (!fi.exists() || !f.open(QIODevice::ReadOnly)) {
                        throw PANDACEPTION("Cannot save: sub-circuit \"%1\" could not be read to embed it.", icFile);
                    }
                    m_scene.icRegistry()->registerBlob(baseName, f.readAll());
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

        // Serialize as a full .panda file (header + compressed metadata + elements)
        QMap<QString, QVariant> metadata;
        Serialization::serializeBlobRegistry(m_scene.icRegistry()->blobMap(), metadata);

        QByteArray payload;
        QDataStream payloadStream(&payload, QIODevice::WriteOnly);
        payloadStream.setVersion(QDataStream::Qt_5_12);
        payloadStream << metadata;
        Serialization::serialize(m_scene.items(), payloadStream, {.purpose = SerializationPurpose::PortableFile});

        QByteArray blob;
        QDataStream stream(&blob, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        Serialization::writePayload(stream, payload);

        m_scene.undoStack()->setClean();
        emit icBlobSaved(m_parentICElementId, blob);
        return SaveOutcome::Saved;
    }

    Q_ASSERT_X(!fileName.isEmpty() && fileName.endsWith(".panda"),
               "WorkSpace::save", "caller must resolve a non-empty, .panda-suffixed path first");
    const QString &fileName_ = fileName;

    qCDebug(zero) << "FileName: " << fileName_;

    // Copy external file dependencies (appearances, audio, IC sub-circuits, waveform)
    // to the new directory before updating contextDir, so save() can store bare filenames.
    // Always run this, even on a brand-new project's first save or a re-save to the same
    // directory: copyToDir()/copyPandaFile() are no-ops for missing/self/already-present
    // files, so this is a cheap "ensure every dependency is present" pass every time --
    // including when a new dependency was added since the last save to an already-saved project.
    const QString oldContextDir = m_scene.contextDir();
    const QString newContextDir = QFileInfo(fileName_).absolutePath();
    for (auto *elm : m_scene.elements()) {
        for (const QString &file : elm->externalFiles()) {
            if (file.endsWith(".panda")) {
                // copyPandaFile copies the file itself and recursively copies any
                // fileBackedICs it references (resolved against the source file's own
                // directory, not oldContextDir -- a dependency freshly added from an
                // arbitrary external location never lived in oldContextDir), in one pass.
                const QFileInfo srcInfo(file);
                Serialization::copyPandaFile(srcInfo, QFileInfo(newContextDir + "/" + srcInfo.fileName()));
            } else {
                FileUtils::copyToDir(file, newContextDir);
            }
        }
    }

    // Copy the associated BeWavedDolphin waveform file if present
    if (!m_dolphinFileName.isEmpty()) {
        const QString resolved = QDir(oldContextDir).absoluteFilePath(m_dolphinFileName);
        FileUtils::copyToDir(resolved, newContextDir);
    }

    // QSaveFile writes to a temp file and commits atomically, preventing data loss
    // if the process is interrupted during a write
    QSaveFile saveFile(fileName_);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        // OneDrive lock, ZIP-extracted folder, network drive, write-protected attribute.
        // Reported only in interactive mode, where WorkspaceManager can re-prompt for a
        // writable location; non-interactive callers (CLI batch, MCP, tests) have no one
        // to show a dialog to, so this throws there exactly as any other I/O error would.
        if (Application::interactiveMode && isReadOnlyFailure(saveFile.error())) {
            return SaveOutcome::ReadOnlyTarget;
        }
        throw PANDACEPTION("Error opening file: %1", saveFile.errorString());
    }

    // Re-tighten the scene rect (avoids a viewport jump on element selection). Must go
    // through resizeScene() rather than a local computation: producing even a slightly
    // different rect than resizeScene()'s (quantized) one would ping-pong the scene rect
    // between two values across edit/autosave cycles, and every change makes Qt's BSP
    // index re-insert all items. No drag can be in progress during a save, so this takes
    // the same tighten branch the interactive callers use.
    m_scene.resizeScene();

    QDataStream stream(&saveFile);
    Serialization::writePandaHeader(stream);
    save(stream);

    if (!saveFile.commit()) {
        if (Application::interactiveMode && isReadOnlyFailure(saveFile.error())) { // LCOV_EXCL_LINE — same class as ICRegistry::extractToFile()'s/DolphinFile::save()'s existing exclusions: open() above already rejects the deterministic failure modes, so a commit()-only failure (e.g. losing rename permission mid-write) needs a second OS user or root.
            return SaveOutcome::ReadOnlyTarget; // LCOV_EXCL_LINE — see above.
        }
        throw PANDACEPTION("Could not save file: %1", saveFile.errorString()); // LCOV_EXCL_LINE — see above.
    }

    // Only adopt the new file/context identity once it's actually on disk — setting it earlier
    // left the workspace believing its current file was a path that was never written whenever
    // the open/commit above failed (interactive ReadOnlyTarget with the retry cancelled, or a
    // throw in non-interactive/MCP callers), with nothing to roll it back afterward.
    setCurrentFile(fileName_);

    // Mark the undo stack as clean so the title bar no longer shows unsaved-change indicator
    m_scene.undoStack()->setClean();

    // Clean up this workspace's own tracked autosave file now that it has a real save.
    if (!m_autosaveFileName.isEmpty() && QFile::exists(m_autosaveFileName)) {
        qCDebug(zero) << "Remove autosave from settings and delete it.";
        QStringList autosaves = Settings::autosaveFiles();
        autosaves.removeAll(m_autosaveFileName);
        Settings::setAutosaveFiles(autosaves);
        QFile::remove(m_autosaveFileName);
        m_autosaveFileName.clear();
        qCDebug(zero) << "All auto save file names after removing autosave: " << autosaves;
    }

    emit fileChanged(m_fileInfo);
    return SaveOutcome::Saved;
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

    // Metadata + elements + connections are serialized into an in-memory buffer
    // first, then compressed as one unit into the real stream -- see
    // Serialization::writePayload().
    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);

    payloadStream << metadata;
    Serialization::serialize(m_scene.items(), payloadStream, {.purpose = SerializationPurpose::PortableFile});

    Serialization::writePayload(stream, payload);
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

    bool needsMigration = (version < FormatRev::current) && Application::migrationEnabled;
    if (needsMigration) {
        createVersionedBackup(fileName, version);
    }

    load(stream, version, QFileInfo(fileName).absolutePath());
    file.close();

    if (needsMigration) {
        // save() now requires an already-.panda-suffixed path; fileName came from an
        // existing, successfully-opened file, so in practice it already has one, but
        // keep the same defensive fallback save() itself used to apply internally.
        QString migratedFileName = fileName;
        if (!migratedFileName.endsWith(".panda")) {
            migratedFileName.append(".panda");
        }
        // Re-save in new format. This is an automatic background step, not a user-
        // initiated save, so a read-only target (unlike an explicit Save command) isn't
        // worth an interactive re-prompt here -- just leave the on-disk file in its
        // original format and note why the migration didn't stick.
        if (save(migratedFileName) == SaveOutcome::ReadOnlyTarget) {
            qCWarning(zero) << "Could not migrate" << migratedFileName << "to the current format: target is read-only.";
        }
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
        if (version > FormatRev::current) {
            const QString fmtVersion = FormatRev::current.toString();
            const QString fileVersion = version.toString();
            const QString message = tr("This file was saved with a newer file format (version %1).\n"
                         "Your version supports file format %2.\n\n"
                         "The file will be opened but saving is blocked.\n"
                         "Please update wiRedPanda to edit and save this file.")
                          .arg(fileVersion, fmtVersion);
            QMessageBox::warning(this, tr("Newer version file."), message);
        } else if (version < FormatRev::current) {
            const QString backupFileName = m_fileInfo.completeBaseName() + ".v" + version.toString() + "." + m_fileInfo.suffix();
            const QString message = tr("This file is in an older format (version %1) and will be automatically upgraded to the current format (version %2).\n"
                         "A backup of the original file has been created with name: %3")
                         .arg(version.toString(), FormatRev::current.toString(), backupFileName);
            QMessageBox::information(this, tr("File upgraded."), message);
        }
    }

    // Everything past the header may be zlib-compressed (Rev100+); buffer and
    // decompress it once, then read the rest of the file from that in-memory
    // stream instead of the live device (which readPayload() has already fully
    // consumed to do the decompression).
    QByteArray payload = Serialization::readPayload(stream, version);
    QDataStream payloadStream(&payload, QIODevice::ReadOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);

    // V4.6+ stores all file-level fields in the metadata map.
    // Older versions wrote dolphinFileName and sceneRect positionally before the map.
    QMap<QString, QVariant> metadata;
    if (VersionInfo::hasUnifiedMetadata(version)) {
        metadata = Serialization::readBoundedMetadata(payloadStream);
        m_dolphinFileName = metadata.value("dolphinFileName").toString();
    } else {
        m_dolphinFileName = Serialization::loadDolphinFileName(payloadStream, version);
        Serialization::loadRect(payloadStream, version);
        if (VersionInfo::hasMetadata(version)) {
            metadata = Serialization::readBoundedMetadata(payloadStream);
        }
    }
    qCDebug(zero) << "Dolphin name: " << m_dolphinFileName;

    QMap<QString, QByteArray> blobRegistry = Serialization::deserializeBlobRegistry(metadata, version);

    // Populate the scene's IC registry with embedded IC blobs
    for (auto it = blobRegistry.cbegin(); it != blobRegistry.cend(); ++it) {
        m_scene.icRegistry()->setBlob(it.key(), it.value());
    }

    QHash<quint64, Port *> portMap;
    if (!contextDir.isEmpty()) {
        m_scene.setContextDir(contextDir);
    }
    auto context = m_scene.deserializationContext(portMap, version, SerializationPurpose::PortableFile);
    context.contextDir = contextDir;
    const auto items = Serialization::deserialize(payloadStream, context);
    qCDebug(zero) << "Finished loading items.";

    for (auto *item : items) {
        m_scene.addItem(item);

        // Track the highest element ID seen so that newly created elements
        // will receive IDs that don't collide with those just loaded
        if (auto *ge = qgraphicsitem_cast<GraphicElement *>(item)) {
            m_lastId = (std::max)(m_lastId, ge->id());
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

QString WorkSpace::dolphinFileName() const
{
    return m_dolphinFileName;
}

void WorkSpace::setAutosaveFileName()
{
    // Eagerly ensure the global autosaves directory exists; the actual filename
    // is computed lazily inside autosave() when the workspace first turns dirty.
    QDir autosavePath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/autosaves");
    if (!autosavePath.exists()) {
        autosavePath.mkpath(autosavePath.absolutePath()); // LCOV_EXCL_LINE — this is called from every WorkSpace constructor, so the global autosaves directory is already created by whichever WorkSpace-based test runs first in the process; a Meyer's-singleton-style one-time-only branch (pattern 12) that can't be isolated without disrupting shared state other tests depend on.
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
        path.mkpath(path.absolutePath()); // LCOV_EXCL_LINE — QFileInfo::isWritable() on a not-yet-created directory returns false, so the "project dir" branch above only ever selects a directory that already exists; the AppData fallback is the same already-created-by-setAutosaveFileName() path as the pattern-12 exclusion above. Not reachable without disrupting shared process-wide state.
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
        throw PANDACEPTION("Could not commit autosave file: %1", autosaveFile.errorString()); // LCOV_EXCL_LINE — same class as save()'s own commit-failure exclusion above: open() already rejects the deterministic failure modes, so a commit()-only failure needs a second OS user or root.
    }

    autosaves.append(m_autosaveFileName);
    Settings::setAutosaveFiles(autosaves);

    qCDebug(three) << "All auto save file names after adding autosave: " << autosaves;

    emit fileChanged(m_fileInfo);
}

void WorkSpace::setAutosaveFile()
{
    // Crash-recovery flow: a recovered autosave is being treated as if it
    // came from the original .panda file, so the next autosave write should
    // overwrite the project file directly rather than spawning a new
    // hidden temp file next to it. Surprising semantically but intentional.
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
    auto preamble = Serialization::readPreamble(stream);

    const auto blobRegistry = Serialization::deserializeBlobRegistry(preamble.metadata, preamble.version);
    for (auto it = blobRegistry.cbegin(); it != blobRegistry.cend(); ++it) {
        m_scene.icRegistry()->setBlob(it.key(), it.value());
    }

    QHash<quint64, Port *> portMap;
    auto context = m_scene.deserializationContext(portMap, preamble.version, SerializationPurpose::PortableFile);
    context.contextDir = parentContextDir;
    QDataStream elementsStream(&preamble.remainingPayload, QIODevice::ReadOnly);
    elementsStream.setVersion(QDataStream::Qt_5_12);
    const auto items = Serialization::deserialize(elementsStream, context);

    for (auto *item : items) {
        m_scene.addItem(item);

        if (auto *ge = qgraphicsitem_cast<GraphicElement *>(item)) {
            m_lastId = (std::max)(m_lastId, ge->id());
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
    if (targets.isEmpty()) { // LCOV_EXCL_LINE — elm itself (just confirmed isEmbedded() with this exact blobName, and already resolved as a live scene element above) satisfies findICsByBlobName()'s own identical isEmbedded()+blobName-match scan, so targets always contains at least elm.
        return; // LCOV_EXCL_LINE — see above.
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

void WorkSpace::setExerciseOverlay(ExerciseOverlay *overlay)
{
    m_exerciseOverlay = overlay;
}
