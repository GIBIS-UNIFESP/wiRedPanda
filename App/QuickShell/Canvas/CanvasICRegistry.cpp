// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Canvas/CanvasICRegistry.h"

#include <QDataStream>
#include <QDir>
#include <QFileInfo>
#include <QIODevice>
#include <QSaveFile>
#include <QUndoStack>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/Wiring/Port.h"

CanvasICRegistry::CanvasICRegistry(CanvasItem *canvas)
    : QObject(canvas)
    , m_canvas(canvas)
{
    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &CanvasICRegistry::onFileChanged, Qt::QueuedConnection);
}

bool CanvasICRegistry::hasBlob(const QString &name) const
{
    return m_blobs.contains(name);
}

QByteArray CanvasICRegistry::blob(const QString &name) const
{
    return m_blobs.value(name);
}

void CanvasICRegistry::setBlob(const QString &name, const QByteArray &data)
{
    m_blobs[name] = data;
}

void CanvasICRegistry::registerBlob(const QString &name, const QByteArray &data)
{
    QSet<QString> visited;
    QMap<QString, QByteArray> workingBlobs;
    workingBlobs[name] = data;
    makeBlobSelfContained(name, visited, workingBlobs);
    m_blobs[name] = workingBlobs[name];
}

void CanvasICRegistry::removeBlob(const QString &name)
{
    m_blobs.remove(name);
}

void CanvasICRegistry::renameBlob(const QString &oldName, const QString &newName)
{
    if (!m_blobs.contains(oldName) || oldName == newName || m_blobs.contains(newName)) {
        return;
    }

    m_blobs[newName] = m_blobs.take(oldName);

    for (auto *elm : m_canvas->elements()) {
        if (elm->isEmbedded() && elm->blobName() == oldName) {
            static_cast<IC *>(elm)->setBlobName(newName);
        }
    }

    // Update embedded IC references inside other blobs' metadata so that parent blobs that
    // contain the renamed blob as a nested dependency stay consistent. Mirrors
    // ICRegistry::renameBlob()'s identical loop.
    for (auto it = m_blobs.begin(); it != m_blobs.end(); ++it) {
        renameBlobReference(it.value(), oldName, newName);
    }

    emit blobRenamed(oldName, newName);
}

QList<GraphicElement *> CanvasICRegistry::findICsByBlobName(const QString &blobName) const
{
    QList<GraphicElement *> result;
    for (auto *elm : m_canvas->elements()) {
        if (elm->isEmbedded() && elm->blobName() == blobName) {
            result.append(elm);
        }
    }
    return result;
}

bool CanvasICRegistry::initEmbeddedIC(IC *ic, const QString &blobName)
{
    if (!hasBlob(blobName)) {
        return false;
    }
    ic->setBlobName(blobName);
    ic->loadFromBlob(blob(blobName), m_canvas->contextDir());
    if (ic->label().isEmpty()) {
        ic->setLabel(blobName.toUpper());
    }
    return true;
}

QString CanvasICRegistry::uniqueBlobName(const QString &baseName) const
{
    if (!hasBlob(baseName)) {
        return baseName;
    }
    for (int i = 2;; ++i) {
        const QString candidate = baseName + QStringLiteral("_") + QString::number(i);
        if (!hasBlob(candidate)) {
            return candidate;
        }
    }
}

IC *CanvasICRegistry::createEmbeddedIC(const QString &blobName, const QByteArray &fileBytes, const QString &contextDir)
{
    auto *ic = new IC();
    ic->setBlobName(blobName);

    m_canvas->undoStack()->beginMacro(QStringLiteral("Add embedded IC"));
    m_canvas->receiveCommand(new CanvasRegisterBlobCommand(blobName, fileBytes, m_canvas));
    ic->loadFromBlob(fileBytes, contextDir);
    m_canvas->receiveCommand(new CanvasAddItemsCommand({ic}, m_canvas));
    m_canvas->undoStack()->endMacro();

    return ic;
}

QByteArray CanvasICRegistry::captureSnapshot(const QList<GraphicElement *> &targets)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    for (auto *elm : targets) {
        elm->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    }
    return data;
}

void CanvasICRegistry::rollbackElements(const QList<GraphicElement *> &elements, const QByteArray &snapshot, CanvasItem *canvas)
{
    QByteArray data(snapshot);
    QDataStream stream(&data, QIODevice::ReadOnly);
    const auto version = Serialization::readPandaHeader(stream);
    QHash<quint64, Port *> portMap;
    auto context = canvas->deserializationContext(portMap, version, SerializationPurpose::InMemorySnapshot);
    for (auto *elm : elements) {
        elm->load(stream, context);
    }
}

QList<GraphicElement *> CanvasICRegistry::findICsByFile(const QString &fileName) const
{
    const QFileInfo target(fileName);
    QList<GraphicElement *> result;
    for (auto *elm : m_canvas->elements()) {
        if (elm->elementType() == ElementType::IC) {
            auto *ic = static_cast<IC *>(elm);
            if (QFileInfo(ic->file()) == target) {
                result.append(elm);
            }
        }
    }
    return result;
}

void CanvasICRegistry::watchFile(const QString &filePath)
{
    if (!m_fileWatcher.files().contains(filePath)) {
        m_fileWatcher.addPath(filePath);
    }
}

void CanvasICRegistry::reloadTargetsAtomically(const QList<GraphicElement *> &targets, const QByteArray &oldData,
                                                const std::function<void(IC *)> &mutate)
{
    QList<GraphicElement *> updated;
    SimulationBlocker blocker(m_canvas->simulation());
    try {
        for (auto *elm : targets) {
            mutate(static_cast<IC *>(elm));
            updated.append(elm);
        }
    } catch (...) {
        rollbackElements(updated, oldData, m_canvas);
        throw;
    }
}

int CanvasICRegistry::embedICsByFile(const QString &fileName, const QByteArray &fileBytes, const QString &blobName)
{
    const auto targets = findICsByFile(fileName);
    if (targets.isEmpty()) {
        return 0;
    }

    const auto connections = CanvasUpdateBlobCommand::captureConnections(targets);
    const QByteArray oldData = captureSnapshot(targets);

    registerBlob(blobName, fileBytes);

    try {
        reloadTargetsAtomically(targets, oldData, [&](IC *ic) {
            ic->setBlobName(blobName);
            ic->loadFromBlob(m_blobs[blobName], m_canvas->contextDir());
        });
    } catch (...) {
        removeBlob(blobName);
        throw;
    }

    auto *cmd = new CanvasUpdateBlobCommand(targets, oldData, connections, m_canvas);
    // Newly registered above (registerBlob() at a name these targets weren't already using),
    // not replacing prior content -- leave m_oldBlob explicitly empty so undo() removes it
    // rather than restoring bytes that never existed. Mirrors ICRegistry::embedICsByFile().
    cmd->setOldBlob(QByteArray());
    m_canvas->receiveCommand(cmd);
    return static_cast<int>(targets.size());
}

int CanvasICRegistry::extractToFile(const QString &blobName, const QString &filePath)
{
    QSaveFile saveFile(filePath);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        throw PANDACEPTION_WITH_CONTEXT("CanvasICRegistry", "Could not open file: %1", saveFile.errorString());
    }
    saveFile.write(blob(blobName));
    if (!saveFile.commit()) {
        throw PANDACEPTION_WITH_CONTEXT("CanvasICRegistry", "Could not save file: %1", saveFile.errorString());
    }

    const auto targets = findICsByBlobName(blobName);
    if (targets.isEmpty()) {
        return 0;
    }

    const auto connections = CanvasUpdateBlobCommand::captureConnections(targets);
    const QByteArray oldData = captureSnapshot(targets);
    const QByteArray oldBlob = blob(blobName);

    const QString fileDir = QFileInfo(filePath).absolutePath();
    reloadTargetsAtomically(targets, oldData, [&](IC *ic) { ic->loadFile(filePath, fileDir); });

    removeBlob(blobName);

    auto *cmd = new CanvasUpdateBlobCommand(targets, oldData, connections, m_canvas);
    cmd->setOldBlob(oldBlob);
    // Elements are now file-backed (blobName() reads empty post-loadFile()), so the
    // constructor's auto-derived m_blobName would be empty too -- override it explicitly to
    // the original blob name so redo()/undo() can still manage the registry entry.
    cmd->setBlobName(blobName);
    m_canvas->receiveCommand(cmd);
    return static_cast<int>(targets.size());
}

void CanvasICRegistry::renameBlobReference(QByteArray &blobData, const QString &oldName, const QString &newName)
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

    // Rename the key in the embedded IC map.
    embeddedICs[newName] = embeddedICs.take(oldName);

    // Re-serialize the blob with updated metadata, preserving the elements/connections tail
    // byte-for-byte. Read that tail from the already-decompressed preamble.remainingPayload,
    // not readStream's device -- readPreamble() fully consumed the live device to do that
    // decompression, so the device has nothing left to offer here.
    const QByteArray elements = preamble.remainingPayload;
    auto metadata = preamble.metadata;
    Serialization::serializeBlobRegistry(embeddedICs, metadata);

    // Metadata and the elements tail must be compressed together as one payload (see
    // Serialization::writePayload()), not written as two independent raw writes --
    // readPreamble() on the other end decompresses the whole thing in one shot.
    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);
    payloadStream << metadata;
    payloadStream.writeRawData(elements.constData(), static_cast<int>(elements.size()));

    QByteArray newBlob;
    QDataStream writeStream(&newBlob, QIODevice::WriteOnly);
    Serialization::writePandaHeader(writeStream);
    Serialization::writePayload(writeStream, payload);

    blobData = newBlob;
}

void CanvasICRegistry::makeBlobSelfContained(const QString &name, QSet<QString> &visited,
                                              QMap<QString, QByteArray> &blobs, int depth)
{
    if (depth >= kMaxBlobNestingDepth) {
        throw PANDACEPTION_WITH_CONTEXT("CanvasICRegistry",
            "Embedded IC dependency chain exceeds the maximum nesting depth (%1) while resolving '%2'",
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

    // Recurse into already-embedded blobs to ensure they are self-contained.
    for (auto it = embeddedICs.begin(); it != embeddedICs.end(); ++it) {
        const QString &depName = it.key();
        blobs[depName] = it.value();
        makeBlobSelfContained(depName, visited, blobs, depth + 1);
        it.value() = blobs[depName];
    }

    // Resolve file-backed IC dependencies from disk and embed them.
    if (metadata.contains("fileBackedICs")) {
        const QStringList files = metadata.value("fileBackedICs").toStringList();
        const QString contextDir = m_canvas->contextDir();

        for (const QString &fileName : files) {
            const QString baseName = QFileInfo(fileName).baseName();
            if (embeddedICs.contains(baseName)) {
                continue;
            }

            QFileInfo fi(QDir(contextDir), fileName);
            if (!fi.exists()) {
                qCWarning(zero) << "makeBlobSelfContained: dependency" << fileName << "not found for blob" << name << "— skipping.";
                continue;
            }

            QFile file(fi.absoluteFilePath());
            if (!file.open(QIODevice::ReadOnly)) {
                qCWarning(zero) << "makeBlobSelfContained: cannot open dependency" << fi.absoluteFilePath() << "for blob" << name << "— blob will be incomplete.";
                continue;
            }
            QByteArray fileBytes = file.readAll();
            file.close();

            // Recursively make the dep self-contained before embedding.
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

    // Re-serialize the blob with updated metadata, preserving the elements/connections tail
    // byte-for-byte. Read that tail from the already-decompressed preamble.remainingPayload,
    // not readStream's device -- readPreamble() fully consumed the live device to do that
    // decompression, so the device has nothing left to offer here.
    const QByteArray elements = preamble.remainingPayload;

    Serialization::serializeBlobRegistry(embeddedICs, metadata);

    // Metadata and the elements tail must be compressed together as one payload (see
    // Serialization::writePayload()), not written as two independent raw writes --
    // readPreamble() on the other end decompresses the whole thing in one shot.
    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);
    payloadStream << metadata;
    payloadStream.writeRawData(elements.constData(), static_cast<int>(elements.size()));

    QByteArray newBlob;
    QDataStream writeStream(&newBlob, QIODevice::WriteOnly);
    Serialization::writePandaHeader(writeStream);
    Serialization::writePayload(writeStream, payload);

    blobs[name] = newBlob;
}

void CanvasICRegistry::onFileChanged(const QString &filePath)
{
    // Guarded for the same reason ICRegistry::onFileChanged() is: this is a queued slot that
    // may need to rethrow past the inner catch below, and throwing across Qt's signal-slot
    // dispatch is undefined behaviour on macOS (Qt's own Exception Safety docs, QTBUG-15197) --
    // guardedSlot() keeps the catch frame inside the slot itself.
    Application::guardedSlot(this, [this, &filePath] {
        // Re-add the watch (some OS remove it after a file change event).
        if (!m_fileWatcher.files().contains(filePath) && QFileInfo::exists(filePath)) {
            m_fileWatcher.addPath(filePath);
        }

        const auto targets = findICsByFile(filePath);
        if (targets.isEmpty()) {
            emit definitionChanged(filePath);
            return;
        }

        // Capture pre-reload state so the undo command can restore both the ICs' element data
        // and the canvas wires that touch their ports. Without this the wires get
        // cascade-deleted by setInputSize/setOutputSize inside loadFile.
        const auto connections = CanvasUpdateBlobCommand::captureConnections(targets);
        const QByteArray oldData = captureSnapshot(targets);

        try {
            reloadTargetsAtomically(targets, oldData, [&](IC *ic) { ic->loadFile(filePath); });
        } catch (...) {
            m_canvas->restartSimulation();
            emit definitionChanged(filePath);
            throw;
        }
        m_canvas->restartSimulation();

        auto *cmd = new CanvasUpdateBlobCommand(targets, oldData, connections, m_canvas);
        m_canvas->receiveCommand(cmd);

        emit definitionChanged(filePath);
    });
}
