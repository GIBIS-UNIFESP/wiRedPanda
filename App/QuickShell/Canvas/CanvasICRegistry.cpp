// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Canvas/CanvasICRegistry.h"

#include <QUndoStack>

#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"

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
    // contextDir empty: this canvas has no notion of "the directory of the loaded .panda
    // file" yet -- see CanvasItem::deserializationContext()'s identical note. Fine for
    // self-contained blobs; a blob with an inlined file-backed dependency (see this class's
    // doc comment on registerBlob()) would need a real contextDir to resolve it, same
    // narrower-than-production gap noted there.
    ic->loadFromBlob(blob(blobName), QString());
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
