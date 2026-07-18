// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickWorkSpace.h"

#include <algorithm>

#include <QDir>
#include <QQmlEngine>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUuid>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/SentryHelpers.h"
#include "App/Core/Settings.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/IO/FileUtils.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasICRegistry.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/DialogProvider.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/Versions.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

namespace {

/// Mirrors Workspace.cpp's own isReadOnlyFailure(): QFileDevice errors that mean "target is
/// locked/not writable" rather than a genuine I/O error, where re-prompting for a different
/// path lets the user recover instead of hitting a dead-end modal.
bool isReadOnlyFailure(QFileDevice::FileError error)
{
    return error == QFileDevice::PermissionsError
        || error == QFileDevice::OpenError
        || error == QFileDevice::WriteError;
}

/// Adds \a item to \a canvas via whichever of CanvasItem::addItem()'s two overloads matches
/// its real type. Mirrors WorkSpace::load()'s/loadFromBlob()'s `m_scene.addItem(item)` loop
/// body, split across CanvasItem's two separately-typed overloads (see CanvasItem.h's doc
/// comment on addItem()). Unlike CanvasCommandUtils::addItems(), does not select the item --
/// a top-level file load shouldn't leave everything selected the way a paste/duplicate does.
void addLoadedItem(CanvasItem *canvas, QGraphicsItem *item, int &lastId)
{
    if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item)) {
        canvas->addItem(elm);
        lastId = (std::max)(lastId, elm->id());
    } else if (auto *conn = qgraphicsitem_cast<Connection *>(item)) {
        canvas->addItem(conn);
    }
}

} // namespace

QuickWorkSpace::QuickWorkSpace(QObject *parent)
    : QObject(parent)
    , m_canvas(std::make_unique<CanvasItem>(nullptr, /*buildDemo=*/false))
{
    // m_canvas's QQuickItem parent is nullptr above (QuickWorkSpace is a QObject, not a
    // QQuickItem, so it can't be one -- Main.qml reparents the canvas into the visible tree
    // itself, via canvas() below). Qt/QML's default ownership rule for a QObject with no
    // parent at the moment it's first exposed to JS (via canvas(), Q_INVOKABLE) is
    // JavaScriptOwnership: the JS garbage collector decides when to delete it, independent of
    // this unique_ptr -- and it will, silently, the first time nothing in the QML/JS graph
    // still references the returned value, calling ~CanvasItem() out from under this
    // unique_ptr's later use. Found via a real, reproducible SIGSEGV (Simulation::start() on
    // a null `this`) once the app ran long enough under a real QQmlEngine/event loop for the
    // GC to actually fire -- every earlier diagnostic in this rewrite ran synchronously before
    // app.exec() started, never letting this happen. CppOwnership makes this unique_ptr the
    // sole, correct owner, matching every other owned-but-QML-exposed object in this rewrite.
    QQmlEngine::setObjectOwnership(m_canvas.get(), QQmlEngine::CppOwnership);

    // Coalesce bursts of changes into one autosave write, mirroring WorkSpace's own debounce.
    // WorkSpace restarts the timer off Scene::circuitHasChanged (an autosave-specific signal
    // coalesced separately, with no CanvasItem equivalent yet); QUndoStack::indexChanged is the
    // right substitute here -- it fires on every push/undo/redo, unlike cleanChanged below,
    // which only fires on the dirty/clean *boundary* and would under-fire the restart on
    // successive edits made while already dirty.
    m_autosaveDebounceTimer.setSingleShot(true);
    m_autosaveDebounceTimer.setInterval(500);
    connect(&m_autosaveDebounceTimer, &QTimer::timeout, this, &QuickWorkSpace::autosave);
    connect(m_canvas->undoStack(), &QUndoStack::indexChanged, &m_autosaveDebounceTimer, qOverload<>(&QTimer::start));

    // Refreshes the tab title (shows "*" while dirty) whenever the undo stack's clean state
    // flips -- needed for inline-IC saves that push CanvasUpdateBlobCommand to a non-current
    // parent workspace. Mirrors WorkSpace's identical connection.
    connect(m_canvas->undoStack(), &QUndoStack::cleanChanged, this, [this](bool /*clean*/) {
        emit fileChanged(m_fileInfo);
        emit titleChanged();
    });

    setAutosaveFileName();
}

QuickWorkSpace::~QuickWorkSpace()
{
    // Mirror WorkSpace's clean-destruction semantics: a clean shutdown discards the autosave
    // so it isn't offered for recovery next launch (a real crash skips this destructor
    // entirely, so the recovery file correctly remains in that case).
    m_autosaveDebounceTimer.stop();
    if (!m_autosaveFileName.isEmpty()) {
        QStringList autosaves = Settings::autosaveFiles();
        autosaves.removeAll(m_autosaveFileName);
        Settings::setAutosaveFiles(autosaves);
        QFile::remove(m_autosaveFileName);
        m_autosaveFileName.clear();
    }
}

bool QuickWorkSpace::isFromNewerVersion() const
{
    return !m_loadedVersion.isNull() && m_loadedVersion > FormatRev::current;
}

QString QuickWorkSpace::title() const
{
    if (m_isInlineIC) {
        return "[" + m_inlineBlobName + "]";
    }
    if (m_fileInfo.exists()) {
        return m_fileInfo.fileName();
    }
    QString name = m_untitledTitle.isEmpty() ? tr("New Project") : m_untitledTitle;
    if (m_isRecovered) {
        name += tr(" (recovered)");
    }
    return name;
}

QuickWorkSpace::SaveOutcome QuickWorkSpace::save(const QString &fileName)
{
    sentryBreadcrumb("file", QStringLiteral("Save: %1").arg(fileName));

    m_autosaveDebounceTimer.stop();

    if (isFromNewerVersion()) {
        if (Application::interactiveMode) {
            const QString message = tr("This file was saved with a newer file format (version %1).\n"
                         "Your wiRedPanda version (%2) supports file format %3.\n\n"
                         "Please update wiRedPanda to save changes to this file.")
                          .arg(m_loadedVersion.toString(), AppVersion::current.toString(), FormatRev::current.toString());
            Dialogs::provider()->choice(tr("Cannot save."), message, {DialogButton::Ok}, DialogButton::Ok);
        }
        return SaveOutcome::Saved;
    }

    if (m_isInlineIC) {
        if (!m_parentWorkspace) {
            qCWarning(zero) << "Inline IC tab: parent workspace was closed. Save is a no-op.";
            return SaveOutcome::Saved;
        }

        const QString contextDir = m_canvas->contextDir();

        for (auto *elm : m_canvas->elements()) {
            if (elm->elementType() == ElementType::IC && !elm->isEmbedded()) {
                auto *ic = static_cast<IC *>(elm);
                const QString icFile = ic->file();
                const QString baseName = QFileInfo(icFile).baseName();
                if (!m_canvas->icRegistry()->hasBlob(baseName)) {
                    QFileInfo fi(QDir(contextDir), icFile);
                    QFile f(fi.absoluteFilePath());
                    if (!fi.exists() || !f.open(QIODevice::ReadOnly)) {
                        throw PANDACEPTION("Cannot save: sub-circuit \"%1\" could not be read to embed it.", icFile);
                    }
                    m_canvas->icRegistry()->registerBlob(baseName, f.readAll());
                }
                ic->setBlobName(baseName);
            }
        }

        QMap<QString, QVariant> metadata;
        Serialization::serializeBlobRegistry(m_canvas->icRegistry()->blobMapRef(), metadata);

        QByteArray payload;
        QDataStream payloadStream(&payload, QIODevice::WriteOnly);
        payloadStream.setVersion(QDataStream::Qt_5_12);
        payloadStream << metadata;
        QList<QGraphicsItem *> allItems;
        for (auto *elm : m_canvas->elements()) {
            allItems.append(elm);
        }
        for (auto *conn : m_canvas->connections()) {
            allItems.append(conn);
        }
        Serialization::serialize(allItems, payloadStream, {.purpose = SerializationPurpose::PortableFile});

        QByteArray blob;
        QDataStream stream(&blob, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        Serialization::writePayload(stream, payload);

        m_canvas->undoStack()->setClean();
        emit icBlobSaved(m_parentICElementId, blob);
        return SaveOutcome::Saved;
    }

    Q_ASSERT_X(!fileName.isEmpty() && fileName.endsWith(".panda"),
               "QuickWorkSpace::save", "caller must resolve a non-empty, .panda-suffixed path first");

    qCDebug(zero) << "FileName: " << fileName;

    const QString oldContextDir = m_canvas->contextDir();
    const QString newContextDir = QFileInfo(fileName).absolutePath();
    for (auto *elm : m_canvas->elements()) {
        for (const QString &file : elm->externalFiles()) {
            if (file.endsWith(".panda")) {
                const QFileInfo srcInfo(file);
                Serialization::copyPandaFile(srcInfo, QFileInfo(newContextDir + "/" + srcInfo.fileName()));
            } else {
                FileUtils::copyToDir(file, newContextDir);
            }
        }
    }

    if (!m_dolphinFileName.isEmpty()) {
        const QString resolved = QDir(oldContextDir).absoluteFilePath(m_dolphinFileName);
        FileUtils::copyToDir(resolved, newContextDir);
    }

    QSaveFile saveFile(fileName);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        if (Application::interactiveMode && isReadOnlyFailure(saveFile.error())) {
            return SaveOutcome::ReadOnlyTarget;
        }
        throw PANDACEPTION("Error opening file: %1", saveFile.errorString());
    }

    QDataStream stream(&saveFile);
    Serialization::writePandaHeader(stream);
    save(stream);

    if (!saveFile.commit()) {
        if (Application::interactiveMode && isReadOnlyFailure(saveFile.error())) {
            return SaveOutcome::ReadOnlyTarget;
        }
        throw PANDACEPTION("Could not save file: %1", saveFile.errorString());
    }

    setCurrentFile(fileName);
    m_canvas->undoStack()->setClean();

    if (!m_autosaveFileName.isEmpty() && QFile::exists(m_autosaveFileName)) {
        QStringList autosaves = Settings::autosaveFiles();
        autosaves.removeAll(m_autosaveFileName);
        Settings::setAutosaveFiles(autosaves);
        QFile::remove(m_autosaveFileName);
        m_autosaveFileName.clear();
    }

    emit fileChanged(m_fileInfo);
    emit titleChanged();
    return SaveOutcome::Saved;
}

void QuickWorkSpace::save(QDataStream &stream)
{
    QMap<QString, QVariant> metadata;

    if (!m_dolphinFileName.isEmpty()) {
        metadata["dolphinFileName"] = m_dolphinFileName;
    }

    const auto portMeta = IC::buildPortMetadata(m_canvas->elements());
    if (portMeta.inputCount > 0 || portMeta.outputCount > 0) {
        metadata["inputCount"] = portMeta.inputCount;
        metadata["outputCount"] = portMeta.outputCount;
        metadata["inputLabels"] = portMeta.inputLabels;
        metadata["outputLabels"] = portMeta.outputLabels;
    }

    Serialization::serializeBlobRegistry(m_canvas->icRegistry()->blobMapRef(), metadata);

    QStringList fileBackedICs;
    for (auto *elm : m_canvas->elements()) {
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

    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);

    payloadStream << metadata;
    QList<QGraphicsItem *> allItems;
    for (auto *elm : m_canvas->elements()) {
        allItems.append(elm);
    }
    for (auto *conn : m_canvas->connections()) {
        allItems.append(conn);
    }
    Serialization::serialize(allItems, payloadStream, {.purpose = SerializationPurpose::PortableFile});

    Serialization::writePayload(stream, payload);
}

void QuickWorkSpace::load(const QString &fileName)
{
    sentryBreadcrumb("file", QStringLiteral("Load: %1").arg(fileName));

    QFile file(fileName);

    if (!file.exists()) {
        throw PANDACEPTION("This file does not exist: %1", fileName);
    }

    setCurrentFile(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        throw PANDACEPTION("Could not open file: %1", file.errorString());
    }

    QDataStream stream(&file);
    QVersionNumber version = Serialization::readPandaHeader(stream);
    m_loadedVersion = version;

    bool needsMigration = (version < FormatRev::current) && Application::migrationEnabled;
    if (needsMigration) {
        Serialization::createVersionedBackup(fileName, version);
    }

    load(stream, version, QFileInfo(fileName).absolutePath());
    file.close();

    if (needsMigration) {
        QString migratedFileName = fileName;
        if (!migratedFileName.endsWith(".panda")) {
            migratedFileName.append(".panda");
        }
        if (save(migratedFileName) == SaveOutcome::ReadOnlyTarget) {
            qCWarning(zero) << "Could not migrate" << migratedFileName << "to the current format: target is read-only.";
        }
    }

    emit fileChanged(m_fileInfo);
    emit titleChanged();
}

void QuickWorkSpace::load(QDataStream &stream, const QVersionNumber &version, const QString &contextDir)
{
    SimulationBlocker simulationBlocker(m_canvas->simulation());

    if (Application::interactiveMode) {
        if (version > FormatRev::current) {
            const QString fmtVersion = FormatRev::current.toString();
            const QString fileVersion = version.toString();
            const QString message = tr("This file was saved with a newer file format (version %1).\n"
                         "Your version supports file format %2.\n\n"
                         "The file will be opened but saving is blocked.\n"
                         "Please update wiRedPanda to edit and save this file.")
                          .arg(fileVersion, fmtVersion);
            Dialogs::provider()->choice(tr("Newer version file."), message, {DialogButton::Ok}, DialogButton::Ok);
        } else if (version < FormatRev::current) {
            const QString backupFileName = m_fileInfo.completeBaseName() + ".v" + version.toString() + "." + m_fileInfo.suffix();
            const QString message = tr("This file is in an older format (version %1) and will be automatically upgraded to the current format (version %2).\n"
                         "A backup of the original file has been created with name: %3")
                         .arg(version.toString(), FormatRev::current.toString(), backupFileName);
            Dialogs::provider()->choice(tr("File upgraded."), message, {DialogButton::Ok}, DialogButton::Ok);
        }
    }

    QByteArray payload = Serialization::readPayload(stream, version);
    QDataStream payloadStream(&payload, QIODevice::ReadOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);

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

    QMap<QString, QByteArray> blobRegistry = Serialization::deserializeBlobRegistry(metadata, version);
    for (auto it = blobRegistry.cbegin(); it != blobRegistry.cend(); ++it) {
        m_canvas->icRegistry()->setBlob(it.key(), it.value());
    }

    QHash<quint64, Port *> portMap;
    if (!contextDir.isEmpty()) {
        m_canvas->setContextDir(contextDir);
    }
    auto context = m_canvas->deserializationContext(portMap, version, SerializationPurpose::PortableFile);
    context.contextDir = contextDir;
    const auto items = Serialization::deserialize(payloadStream, context);

    for (auto *item : items) {
        addLoadedItem(m_canvas.get(), item, m_lastId);
    }

    m_canvas->setLastId(m_lastId);
}

void QuickWorkSpace::setAutosaveFileName()
{
    QDir autosavePath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/autosaves");
    if (!autosavePath.exists()) {
        autosavePath.mkpath(autosavePath.absolutePath());
    }
    m_autosaveFileName.clear();
}

void QuickWorkSpace::autosave()
{
    if (m_isInlineIC) {
        return;
    }

    if (isFromNewerVersion()) {
        return;
    }

    QStringList autosaves = Settings::autosaveFiles();

    auto *undoStack = m_canvas->undoStack();

    if (undoStack->isClean()) {
        if (!m_autosaveFileName.isEmpty()) {
            autosaves.removeAll(m_autosaveFileName);
            Settings::setAutosaveFiles(autosaves);
            QFile::remove(m_autosaveFileName);
            m_autosaveFileName.clear();
        }
        emit fileChanged(m_fileInfo);
        emit titleChanged();
        return;
    }

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

    if (autosaves.contains(m_autosaveFileName)) {
        autosaves.removeAll(m_autosaveFileName);
        Settings::setAutosaveFiles(autosaves);
    }

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

    emit fileChanged(m_fileInfo);
    emit titleChanged();
}

void QuickWorkSpace::setAutosaveFile()
{
    m_autosaveFileName = m_fileInfo.filePath();
}

void QuickWorkSpace::flushPendingAutosave()
{
    if (m_autosaveDebounceTimer.isActive()) {
        m_autosaveDebounceTimer.stop();
        autosave();
    }
}

void QuickWorkSpace::loadFromBlob(const QByteArray &blob, QuickWorkSpace *parent, int icElementId, const QString &parentContextDir)
{
    SimulationBlocker simulationBlocker(m_canvas->simulation());

    if (!parentContextDir.isEmpty()) {
        m_canvas->setContextDir(parentContextDir);
    }

    QByteArray blobData(blob);
    QDataStream stream(&blobData, QIODevice::ReadOnly);
    auto preamble = Serialization::readPreamble(stream);

    const auto blobRegistry = Serialization::deserializeBlobRegistry(preamble.metadata, preamble.version);
    for (auto it = blobRegistry.cbegin(); it != blobRegistry.cend(); ++it) {
        m_canvas->icRegistry()->setBlob(it.key(), it.value());
    }

    QHash<quint64, Port *> portMap;
    auto context = m_canvas->deserializationContext(portMap, preamble.version, SerializationPurpose::PortableFile);
    context.contextDir = parentContextDir;
    QDataStream elementsStream(&preamble.remainingPayload, QIODevice::ReadOnly);
    elementsStream.setVersion(QDataStream::Qt_5_12);
    const auto items = Serialization::deserialize(elementsStream, context);

    for (auto *item : items) {
        addLoadedItem(m_canvas.get(), item, m_lastId);
    }

    m_canvas->setLastId(m_lastId);

    m_isInlineIC = true;
    m_parentWorkspace = parent;
    m_parentICElementId = icElementId;

    if (parent) {
        if (auto *item = parent->canvas()->itemById(icElementId)) {
            if (auto *elm = dynamic_cast<GraphicElement *>(item)) {
                m_inlineBlobName = elm->blobName();
            }
        }
    }
}

void QuickWorkSpace::onChildICBlobSaved(int icElementId, const QByteArray &blob)
{
    auto *item = m_canvas->itemById(icElementId);
    if (!item) {
        return;
    }

    auto *elm = dynamic_cast<GraphicElement *>(item);
    if (!elm || !elm->isEmbedded()) {
        return;
    }

    const QString targetBlobName = elm->blobName();
    const auto targets = m_canvas->icRegistry()->findICsByBlobName(targetBlobName);
    if (targets.isEmpty()) {
        return;
    }

    const auto connections = CanvasUpdateBlobCommand::captureConnections(targets);

    SimulationBlocker simulationBlocker(m_canvas->simulation());

    const QByteArray oldData = CanvasICRegistry::captureSnapshot(targets);
    QByteArray oldBlob = m_canvas->icRegistry()->blob(targetBlobName);

    m_canvas->icRegistry()->setBlob(targetBlobName, blob);
    QList<GraphicElement *> updated;
    try {
        for (auto *target : targets) {
            auto *ic = static_cast<IC *>(target);
            ic->loadFromBlob(blob, m_canvas->contextDir());
            updated.append(target);
        }
    } catch (...) {
        CanvasICRegistry::rollbackElements(updated, oldData, m_canvas.get());
        m_canvas->icRegistry()->setBlob(targetBlobName, oldBlob);
        throw;
    }

    auto *cmd = new CanvasUpdateBlobCommand(targets, oldData, connections, m_canvas.get());
    cmd->setOldBlob(oldBlob);
    m_canvas->undoStack()->push(cmd);
}

void QuickWorkSpace::removeEmbeddedIC(const QString &blobName)
{
    QList<QGraphicsItem *> toDelete;

    for (auto *elm : m_canvas->elements()) {
        if (elm->isEmbedded() && elm->blobName() == blobName) {
            toDelete.append(elm);
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

    const bool hasBlob = m_canvas->icRegistry()->hasBlob(blobName);
    if (toDelete.isEmpty() && !hasBlob) {
        return;
    }

    m_canvas->undoStack()->beginMacro(tr("Remove embedded IC \"%1\"").arg(blobName));
    if (!toDelete.isEmpty()) {
        m_canvas->receiveCommand(new CanvasDeleteItemsCommand(toDelete, m_canvas.get()));
    }
    if (hasBlob) {
        m_canvas->receiveCommand(new CanvasRemoveBlobCommand(blobName, m_canvas.get()));
    }
    m_canvas->undoStack()->endMacro();
}

void QuickWorkSpace::setCurrentFile(const QString &filePath)
{
    m_fileInfo = QFileInfo(filePath);
    m_canvas->setContextDir(m_fileInfo.absolutePath());
}
