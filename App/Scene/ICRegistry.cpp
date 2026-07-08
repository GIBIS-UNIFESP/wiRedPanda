// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/ICRegistry.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QFileInfo>
#include <QSaveFile>
#include <QThread>

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/Versions.h"

ICRegistry::ICRegistry(Scene *scene)
    : QObject(scene)
    , m_scene(scene)
{
    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged,
            this, &ICRegistry::onFileChanged, Qt::QueuedConnection);
}

const QByteArray &ICRegistry::cachedFileBytes(const QString &filePath)
{
    Q_ASSERT(QCoreApplication::instance()->thread() == QThread::currentThread());

    if (!m_fileCache.contains(filePath)) {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            m_fileCache[filePath] = file.readAll();
        } else {
            // Do NOT insert into m_fileCache on failure: QMap::operator[] would
            // default-construct and permanently cache an empty entry, silently
            // masking the failure on every subsequent lookup (including if the
            // file becomes readable later).
            qCWarning(zero) << "ICRegistry: cannot open IC file:" << filePath;
            static const QByteArray empty;
            return empty;
        }
    }
    return m_fileCache[filePath];
}

void ICRegistry::invalidate(const QString &filePath)
{
    m_fileCache.remove(filePath);
}

void ICRegistry::watchFile(const QString &filePath)
{
    if (!m_fileWatcher.files().contains(filePath)) {
        m_fileWatcher.addPath(filePath);
    }
}

QList<GraphicElement *> ICRegistry::findICsByFile(const QString &fileName) const
{
    const QFileInfo target(fileName);
    QList<GraphicElement *> result;
    for (auto *elm : m_scene->elements()) {
        if (elm->elementType() == ElementType::IC) {
            auto *ic = static_cast<IC *>(elm);
            if (QFileInfo(ic->file()) == target) {
                result.append(elm);
            }
        }
    }
    return result;
}

void ICRegistry::onFileChanged(const QString &filePath)
{
    qCDebug(zero) << "IC file changed:" << filePath;

    // Invalidate the cached definition so it's rebuilt on next access
    invalidate(filePath);

    // Re-add the watch (some OS remove it after a file change event)
    if (!m_fileWatcher.files().contains(filePath) && QFileInfo::exists(filePath)) {
        m_fileWatcher.addPath(filePath);
    }

    // Reload all IC instances referencing this file
    const auto targets = findICsByFile(filePath);
    if (targets.isEmpty()) {
        emit definitionChanged(filePath);
        return;
    }

    // Capture pre-reload state so the undo command can restore both the
    // ICs' element data and the scene wires that touch their ports.
    // Without this the wires get cascade-deleted by setInputSize/setOutputSize
    // inside loadFile and Cluster D throws on the next undo lookup.
    const auto connections = UpdateBlobCommand::captureConnections(targets);
    const QByteArray oldData = captureSnapshot(targets);

    try {
        reloadTargetsAtomically(targets, oldData, [&](IC *ic) { ic->loadFile(filePath); });
    } catch (...) {
        m_scene->setCircuitUpdateRequired();
        emit definitionChanged(filePath);
        throw;
    }
    m_scene->setCircuitUpdateRequired();

    auto *cmd = new UpdateBlobCommand(targets, oldData, connections, m_scene);
    m_scene->undoStack()->push(cmd);

    emit definitionChanged(filePath);
}

// --- Embedded IC blob storage ---

bool ICRegistry::hasBlob(const QString &name) const
{
    return m_blobs.contains(name);
}

QByteArray ICRegistry::blob(const QString &name) const
{
    return m_blobs.value(name);
}

void ICRegistry::setBlob(const QString &name, const QByteArray &data)
{
    m_blobs[name] = data;
}

void ICRegistry::registerBlob(const QString &name, const QByteArray &data)
{
    QSet<QString> visited;
    QMap<QString, QByteArray> workingBlobs;
    workingBlobs[name] = data;
    makeBlobSelfContained(name, visited, workingBlobs);
    m_blobs[name] = workingBlobs[name];
}

void ICRegistry::removeBlob(const QString &name)
{
    m_blobs.remove(name);
}

void ICRegistry::renameBlob(const QString &oldName, const QString &newName)
{
    // Defense in depth: the primary guard is at the caller (ElementEditor rejects a colliding
    // rename before ever constructing a command), but no-op here too rather than silently
    // overwriting an unrelated blob's bytes, so any future/other caller can't corrupt one IC's
    // data by renaming a different one onto it.
    if (!m_blobs.contains(oldName) || oldName == newName || m_blobs.contains(newName)) {
        return;
    }

    m_blobs[newName] = m_blobs.take(oldName);

    // Update all IC instances on the scene referencing the old name
    for (auto *elm : m_scene->elements()) {
        if (elm->isEmbedded() && elm->blobName() == oldName) {
            auto *ic = static_cast<IC *>(elm);
            ic->setBlobName(newName);
        }
    }

    // Update embedded IC references inside other blobs' metadata so that parent
    // blobs that contain the renamed blob as a nested dependency stay consistent.
    for (auto it = m_blobs.begin(); it != m_blobs.end(); ++it) {
        renameBlobReference(it.value(), oldName, newName);
    }
}

void ICRegistry::clearBlobs()
{
    m_blobs.clear();
}

QList<GraphicElement *> ICRegistry::findICsByBlobName(const QString &blobName) const
{
    QList<GraphicElement *> result;
    for (auto *elm : m_scene->elements()) {
        if (elm->isEmbedded() && elm->blobName() == blobName) {
            result.append(elm);
        }
    }
    return result;
}

bool ICRegistry::initEmbeddedIC(IC *ic, const QString &blobName)
{
    if (!hasBlob(blobName)) {
        return false;
    }
    ic->setBlobName(blobName);
    ic->loadFromBlob(blob(blobName), m_scene->contextDir());
    if (ic->label().isEmpty()) {
        ic->setLabel(blobName.toUpper());
    }
    return true;
}

QString ICRegistry::uniqueBlobName(const QString &baseName) const
{
    if (!hasBlob(baseName)) {
        return baseName;
    }
    for (int i = 2; ; ++i) {
        const QString candidate = baseName + "_" + QString::number(i);
        if (!hasBlob(candidate)) {
            return candidate;
        }
    }
}

IC *ICRegistry::createEmbeddedIC(const QString &blobName, const QByteArray &fileBytes, const QString &contextDir)
{
    auto *ic = new IC();
    ic->setBlobName(blobName);

    m_scene->undoStack()->beginMacro(QCoreApplication::tr("Add embedded IC"));
    m_scene->receiveCommand(new RegisterBlobCommand(blobName, fileBytes, m_scene));
    ic->loadFromBlob(fileBytes, contextDir);
    m_scene->receiveCommand(new AddItemsCommand({ic}, m_scene));
    m_scene->undoStack()->endMacro();

    return ic;
}

int ICRegistry::embedICsByFile(const QString &fileName, const QByteArray &fileBytes,
                               const QString &blobName)
{
    const auto targets = findICsByFile(fileName);
    if (targets.isEmpty()) {
        return 0;
    }

    const auto connections = UpdateBlobCommand::captureConnections(targets);
    const QByteArray oldData = captureSnapshot(targets);

    registerBlob(blobName, fileBytes);

    try {
        reloadTargetsAtomically(targets, oldData, [&](IC *ic) {
            ic->setBlobName(blobName);
            ic->loadFromBlob(m_blobs[blobName], m_scene->contextDir());
        });
    } catch (...) {
        removeBlob(blobName);
        throw;
    }

    auto *cmd = new UpdateBlobCommand(targets, oldData, connections, m_scene);
    // This blob is newly registered above (registerBlob() at a name these targets weren't
    // already using), not replacing prior content — leave m_oldBlob at its default-empty so
    // undo() removes it rather than restoring bytes that never existed. Explicit rather than
    // relying on the constructor default, unlike every other UpdateBlobCommand call site.
    cmd->setOldBlob(QByteArray());
    m_scene->undoStack()->push(cmd);
    return static_cast<int>(targets.size());
}

int ICRegistry::extractToFile(const QString &blobName, const QString &filePath)
{
    // Write blob to disk
    QSaveFile saveFile(filePath);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        throw PANDACEPTION("Could not open file: %1", saveFile.errorString());
    }
    saveFile.write(blob(blobName));
    if (!saveFile.commit()) {
        throw PANDACEPTION("Could not save file: %1", saveFile.errorString());
    }

    // Convert all embedded ICs with this blobName to file-backed
    const auto targets = findICsByBlobName(blobName);
    if (targets.isEmpty()) {
        return 0;
    }

    const auto connections = UpdateBlobCommand::captureConnections(targets);
    const QByteArray oldData = captureSnapshot(targets);
    const QByteArray oldBlob = blob(blobName);

    const QString fileDir = QFileInfo(filePath).absolutePath();
    reloadTargetsAtomically(targets, oldData, [&](IC *ic) { ic->loadFile(filePath, fileDir); });

    removeBlob(blobName);

    auto *cmd = new UpdateBlobCommand(targets, oldData, connections, m_scene);
    cmd->setOldBlob(oldBlob);
    cmd->setBlobName(blobName);
    m_scene->undoStack()->push(cmd);
    return static_cast<int>(targets.size());
}

void ICRegistry::rollbackElements(const QList<GraphicElement *> &elements, const QByteArray &snapshot,
                                   Scene *scene)
{
    QByteArray data(snapshot);
    QDataStream stream(&data, QIODevice::ReadOnly);
    const auto version = Serialization::readPandaHeader(stream);
    QHash<quint64, Port *> portMap;
    auto ctx = scene->deserializationContext(portMap, version, SerializationPurpose::InMemorySnapshot);
    for (auto *elm : elements) {
        elm->load(stream, ctx);
    }
}

void ICRegistry::reloadTargetsAtomically(const QList<GraphicElement *> &targets, const QByteArray &oldData,
                                          const std::function<void(IC *)> &mutate)
{
    // Stop the simulation for the entire loop, not just each individual mutation. Between
    // freeing one IC's old internal graph and rebuilding it, the scene's sorted vectors hold
    // dangling pointers; ticking on that state would fault — and that stale state persists
    // across the whole loop, since setCircuitUpdateRequired() only runs once, after every
    // target has been reloaded, not after each one.
    QList<GraphicElement *> updated;
    SimulationBlocker blocker(m_scene->simulation());
    try {
        for (auto *elm : targets) {
            mutate(static_cast<IC *>(elm));
            updated.append(elm);
        }
    } catch (...) {
        rollbackElements(updated, oldData, m_scene);
        throw;
    }
}

void ICRegistry::makeBlobSelfContained(const QString &name, QSet<QString> &visited,
                                       QMap<QString, QByteArray> &blobs, int depth)
{
    if (depth >= kMaxBlobNestingDepth) {
        throw PANDACEPTION("Embedded IC dependency chain exceeds the maximum nesting depth (%1) while resolving '%2'",
                           QString::number(kMaxBlobNestingDepth), name);
    }

    if (visited.contains(name)) {
        qCWarning(zero) << "Circular blob reference detected:" << name << "— skipping.";
        return;
    }
    visited.insert(name);

    QByteArray blobData(blobs[name]);
    QDataStream readStream(&blobData, QIODevice::ReadOnly);
    const auto preamble = Serialization::readPreamble(readStream);

    if (!VersionInfo::hasMetadata(preamble.version)) {
        return;
    }

    auto metadata = preamble.metadata;
    auto embeddedICs = Serialization::deserializeBlobRegistry(metadata, preamble.version);
    bool modified = false;

    // Recurse into already-embedded blobs to ensure they are self-contained
    for (auto it = embeddedICs.begin(); it != embeddedICs.end(); ++it) {
        const QString &depName = it.key();
        blobs[depName] = it.value();
        makeBlobSelfContained(depName, visited, blobs, depth + 1);
        it.value() = blobs[depName];
    }

    // Resolve file-backed IC dependencies from disk and embed them
    if (metadata.contains("fileBackedICs")) {
        const QStringList files = metadata.value("fileBackedICs").toStringList();
        const QString contextDir = m_scene->contextDir();

        for (const QString &fileName : files) {
            const QString baseName = QFileInfo(fileName).baseName();
            if (embeddedICs.contains(baseName)) {
                continue;
            }

            QFileInfo fi(QDir(contextDir), fileName);
            if (!fi.exists()) {
                qCWarning(zero) << "makeBlobSelfContained: dependency" << fileName
                                << "not found for blob" << name << "— skipping.";
                continue;
            }

            QFile file(fi.absoluteFilePath());
            if (!file.open(QIODevice::ReadOnly)) {
                qCWarning(zero) << "makeBlobSelfContained: cannot open dependency" << fi.absoluteFilePath()
                                << "for blob" << name << "— blob will be incomplete.";
                continue;
            }
            QByteArray fileBytes = file.readAll();
            file.close();

            // Recursively make the dep self-contained before embedding
            blobs[baseName] = fileBytes;
            makeBlobSelfContained(baseName, visited, blobs, depth + 1);
            embeddedICs[baseName] = blobs[baseName];
        }

        metadata.remove("fileBackedICs");
        modified = true;
    }

    if (!modified) {
        return;
    }

    // Re-serialize the blob with updated metadata
    const QByteArray elements = readStream.device()->readAll();

    Serialization::serializeBlobRegistry(embeddedICs, metadata);

    QByteArray newBlob;
    QDataStream writeStream(&newBlob, QIODevice::WriteOnly);
    Serialization::writePandaHeader(writeStream);
    writeStream << metadata;
    writeStream.writeRawData(elements.constData(), static_cast<int>(elements.size()));

    blobs[name] = newBlob;
}

void ICRegistry::renameBlobReference(QByteArray &blobData, const QString &oldName, const QString &newName)
{
    QDataStream readStream(&blobData, QIODevice::ReadOnly);
    const auto preamble = Serialization::readPreamble(readStream);

    if (!VersionInfo::hasMetadata(preamble.version)) {
        return;
    }

    auto embeddedICs = Serialization::deserializeBlobRegistry(preamble.metadata, preamble.version);
    if (!embeddedICs.contains(oldName)) {
        return;
    }

    // Rename the key in the embedded IC map
    embeddedICs[newName] = embeddedICs.take(oldName);

    // Re-serialize the blob with updated metadata
    const QByteArray elements = readStream.device()->readAll();
    auto metadata = preamble.metadata;
    Serialization::serializeBlobRegistry(embeddedICs, metadata);

    QByteArray newBlob;
    QDataStream writeStream(&newBlob, QIODevice::WriteOnly);
    Serialization::writePandaHeader(writeStream);
    writeStream << metadata;
    writeStream.writeRawData(elements.constData(), static_cast<int>(elements.size()));

    blobData = newBlob;
}

QByteArray ICRegistry::captureSnapshot(const QList<GraphicElement *> &targets)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    for (auto *elm : targets) {
        elm->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    }
    return data;
}
