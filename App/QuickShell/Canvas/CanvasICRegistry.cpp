// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Canvas/CanvasICRegistry.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QSaveFile>
#include <QUndoStack>

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/Wiring/Port.h"

CanvasICRegistry::CanvasICRegistry(CanvasItem *canvas)
    : m_canvas(canvas)
{
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
    m_blobs[name] = data;
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
