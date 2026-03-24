// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ICRegistry.h"

#include <QDataStream>
#include <QDir>
#include <QFileInfo>
#include <QSaveFile>

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/IO/VersionInfo.h"
#include "App/Scene/Commands.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/Versions.h"

ICRegistry::ICRegistry(Scene *scene)
    : QObject(scene)
    , m_scene(scene)
{
    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged,
            this, &ICRegistry::onFileChanged);
}

bool ICRegistry::has(const QString &filePath) const
{
    return m_definitions.contains(filePath);
}

const ICDefinition *ICRegistry::definition(const QString &filePath, const QString &contextDir)
{
    // Cycle detection
    if (m_loadingFiles.contains(filePath)) {
        return nullptr;
    }

    if (!m_definitions.contains(filePath)) {
        m_loadingFiles.insert(filePath);
        m_definitions[filePath] = ICDefinition::fromFile(filePath, contextDir);
        m_loadingFiles.remove(filePath);
    }
    return &m_definitions[filePath];
}

void ICRegistry::invalidate(const QString &filePath)
{
    m_definitions.remove(filePath);
}

void ICRegistry::watchFile(const QString &filePath)
{
    if (!m_fileWatcher.files().contains(filePath)) {
        m_fileWatcher.addPath(filePath);
    }
}

QList<GraphicElement *> ICRegistry::findICsByFile(const QString &fileName) const
{
    QList<GraphicElement *> result;
    for (auto *elm : m_scene->elements()) {
        if (elm->elementType() == ElementType::IC) {
            auto *ic = static_cast<IC *>(elm);
            if (QFileInfo(ic->icFile()) == QFileInfo(fileName)) {
                result.append(elm);
            }
        }
    }
    return result;
}

bool ICRegistry::isLoading(const QString &canonicalPath) const
{
    return m_loadingFiles.contains(canonicalPath);
}

bool ICRegistry::beginLoading(const QString &canonicalPath)
{
    if (m_loadingFiles.contains(canonicalPath)) {
        return false;
    }
    m_loadingFiles.insert(canonicalPath);
    return true;
}

void ICRegistry::endLoading(const QString &canonicalPath)
{
    m_loadingFiles.remove(canonicalPath);
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
    for (auto *elm : targets) {
        auto *ic = static_cast<IC *>(elm);
        ic->loadFile(filePath);
    }

    if (!targets.isEmpty()) {
        m_scene->simulation()->restart();
    }

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
    setBlob(name, data);
    QSet<QString> visited;
    makeBlobSelfContained(name, visited);
}

void ICRegistry::removeBlob(const QString &name)
{
    m_blobs.remove(name);
}

void ICRegistry::renameBlob(const QString &oldName, const QString &newName)
{
    if (!m_blobs.contains(oldName) || oldName == newName) {
        return;
    }

    m_blobs[newName] = m_blobs.take(oldName);

    // Update all IC instances referencing the old name
    for (auto *elm : m_scene->elements()) {
        if (elm->isEmbeddedIC() && elm->blobName() == oldName) {
            auto *ic = static_cast<IC *>(elm);
            ic->setBlobName(newName);
        }
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
        if (elm->isEmbeddedIC() && elm->blobName() == blobName) {
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

    // Mutate all targets atomically: if any loadFromBlob throws, roll back
    // every element that was already updated so we don't push a partial undo command.
    QList<GraphicElement *> updated;
    try {
        for (auto *elm : targets) {
            auto *ic = static_cast<IC *>(elm);
            ic->setBlobName(blobName);
            ic->loadFromBlob(m_blobs[blobName], m_scene->contextDir());
            updated.append(elm);
        }
    } catch (...) {
        // Roll back already-updated elements from the oldData snapshot
        QDataStream rollback(&const_cast<QByteArray &>(oldData), QIODevice::ReadOnly);
        const auto version = Serialization::readPandaHeader(rollback);
        QMap<quint64, QNEPort *> portMap;
        auto ctx = m_scene->deserializationContext(portMap, version);
        for (auto *elm : std::as_const(updated)) {
            elm->load(rollback, ctx);
        }
        removeBlob(blobName);
        throw;
    }

    auto *cmd = new UpdateBlobCommand(targets, oldData, connections, m_scene);
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

    // Mutate all targets atomically: if any loadFile throws, roll back
    // every element that was already updated so we don't push a partial undo command.
    const QString fileDir = QFileInfo(filePath).absolutePath();
    QList<GraphicElement *> updated;
    try {
        for (auto *elm : targets) {
            auto *ic = static_cast<IC *>(elm);
            ic->loadFile(filePath, fileDir);
            updated.append(elm);
        }
    } catch (...) {
        QDataStream rollback(&const_cast<QByteArray &>(oldData), QIODevice::ReadOnly);
        const auto version = Serialization::readPandaHeader(rollback);
        QMap<quint64, QNEPort *> portMap;
        auto ctx = m_scene->deserializationContext(portMap, version);
        for (auto *elm : std::as_const(updated)) {
            elm->load(rollback, ctx);
        }
        throw;
    }

    removeBlob(blobName);

    auto *cmd = new UpdateBlobCommand(targets, oldData, connections, m_scene);
    cmd->setOldBlob(oldBlob);
    cmd->setBlobName(blobName);
    m_scene->undoStack()->push(cmd);
    return static_cast<int>(targets.size());
}

void ICRegistry::makeBlobSelfContained(const QString &name, QSet<QString> &visited)
{
    if (visited.contains(name)) {
        qCWarning(zero) << "Circular blob reference detected:" << name << "— skipping.";
        return;
    }
    visited.insert(name);

    QByteArray blobData(m_blobs[name]);
    QDataStream readStream(&blobData, QIODevice::ReadOnly);
    const auto preamble = Serialization::readPreamble(readStream);

    if (!VersionInfo::hasMetadata(preamble.version)) {
        return;
    }

    auto metadata = preamble.metadata;
    auto embeddedICs = Serialization::deserializeBlobRegistry(metadata);
    bool modified = false;

    // Recurse into already-embedded blobs to ensure they are self-contained
    for (auto it = embeddedICs.begin(); it != embeddedICs.end(); ++it) {
        // Temporarily store in m_blobs so makeBlobSelfContained can operate on it
        const QString &depName = it.key();
        m_blobs[depName] = it.value();
        makeBlobSelfContained(depName, visited);
        it.value() = m_blobs[depName];
        m_blobs.remove(depName);
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
                continue;
            }

            QFile file(fi.absoluteFilePath());
            if (!file.open(QIODevice::ReadOnly)) {
                continue;
            }
            QByteArray fileBytes = file.readAll();
            file.close();

            // Recursively make the dep self-contained before embedding
            m_blobs[baseName] = fileBytes;
            makeBlobSelfContained(baseName, visited);
            embeddedICs[baseName] = m_blobs[baseName];
            m_blobs.remove(baseName);
        }

        metadata.remove("fileBackedICs");
        modified = true;
    }

    if (!modified) {
        return;
    }

    // Re-serialize the blob with updated metadata
    const QByteArray elements = blobData.mid(readStream.device()->pos());

    Serialization::serializeBlobRegistry(embeddedICs, metadata);

    QByteArray newBlob;
    QDataStream writeStream(&newBlob, QIODevice::WriteOnly);
    Serialization::writePandaHeader(writeStream);
    writeStream << QString();   // dolphin
    writeStream << QRectF();    // rect
    writeStream << metadata;
    writeStream.writeRawData(elements.constData(), elements.size());

    m_blobs[name] = newBlob;
}

QByteArray ICRegistry::captureSnapshot(const QList<GraphicElement *> &targets)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    for (auto *elm : targets) {
        elm->save(stream);
    }
    return data;
}

