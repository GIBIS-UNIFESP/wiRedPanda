// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/ClipboardManager.h"

#include <memory>

#include <QApplication>
#include <QClipboard>
#include <QDrag>
#include <QIODevice>
#include <QLoggingCategory>
#include <QMimeData>
#include <QPainter>

#include "App/Core/Common.h"
#include "App/Core/MimeTypes.h"
#include "App/Core/SentryHelpers.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Commands.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/Wiring/Connection.h"

ClipboardManager::ClipboardManager(Scene *scene)
    : m_scene(scene)
{
}

void ClipboardManager::copy()
{
    if (m_scene->selectedElements().empty()) {
        QApplication::clipboard()->clear();
        return;
    }

    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    serializeItems(m_scene->selectedItems(), stream);

    auto mimeData = std::make_unique<QMimeData>();
    mimeData->setData(MimeType::Clipboard, itemData);

    // Include only blobs used by the selected elements
    auto *registry = m_scene->icRegistry();
    QMap<QString, QByteArray> usedBlobs;
    for (auto *elm : m_scene->selectedElements()) {
        if (elm->isEmbedded() && !elm->blobName().isEmpty()) {
            const QString &name = elm->blobName();
            if (!usedBlobs.contains(name) && registry->hasBlob(name)) {
                usedBlobs[name] = registry->blob(name);
            }
        }
    }
    if (!usedBlobs.isEmpty()) {
        // Unversioned copy for backward compatibility with older app versions.
        QByteArray regBytes;
        QDataStream regStream(&regBytes, QIODevice::WriteOnly);
        regStream << usedBlobs;
        mimeData->setData(MimeType::BlobRegistry, regBytes);

        // Versioned copy (Qt_5_12) for newer app versions — preferred on paste.
        QByteArray regBytesV2;
        QDataStream regStreamV2(&regBytesV2, QIODevice::WriteOnly);
        regStreamV2.setVersion(QDataStream::Qt_5_12);
        regStreamV2 << usedBlobs;
        mimeData->setData(MimeType::BlobRegistryV2, regBytesV2);
    }

    QApplication::clipboard()->setMimeData(mimeData.release());
}

void ClipboardManager::cut()
{
    if (m_scene->selectedElements().isEmpty()) {
        QApplication::clipboard()->clear();
        return;
    }

    // Collect used blobs before deleting elements
    auto *registry = m_scene->icRegistry();
    QMap<QString, QByteArray> usedBlobs;
    for (auto *elm : m_scene->selectedElements()) {
        if (elm->isEmbedded() && !elm->blobName().isEmpty()) {
            const QString &name = elm->blobName();
            if (!usedBlobs.contains(name) && registry->hasBlob(name)) {
                usedBlobs[name] = registry->blob(name);
            }
        }
    }

    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    serializeAndDelete(m_scene->selectedItems(), stream);

    auto mimeData = std::make_unique<QMimeData>();
    mimeData->setData(MimeType::Clipboard, itemData);

    if (!usedBlobs.isEmpty()) {
        // Unversioned copy for backward compatibility with older app versions.
        QByteArray regBytes;
        QDataStream regStream(&regBytes, QIODevice::WriteOnly);
        regStream << usedBlobs;
        mimeData->setData(MimeType::BlobRegistry, regBytes);

        // Versioned copy (Qt_5_12) for newer app versions — preferred on paste.
        QByteArray regBytesV2;
        QDataStream regStreamV2(&regBytesV2, QIODevice::WriteOnly);
        regStreamV2.setVersion(QDataStream::Qt_5_12);
        regStreamV2 << usedBlobs;
        mimeData->setData(MimeType::BlobRegistryV2, regBytesV2);
    }

    QApplication::clipboard()->setMimeData(mimeData.release());
}

void ClipboardManager::paste()
{
    const auto *mimeData = QApplication::clipboard()->mimeData();

    if (!mimeData) {
        return;
    }

    // Import blob registry from clipboard so cross-tab paste of embedded ICs works.
    // Prefer BlobRegistryV2 (explicit Qt_5_12 version); fall back to the legacy
    // unversioned BlobRegistry produced by older app versions.
    QMap<QString, QByteArray> clipboardBlobs;
    if (mimeData->hasFormat(MimeType::BlobRegistryV2)) {
        QByteArray regBytes = mimeData->data(MimeType::BlobRegistryV2);
        QDataStream regStream(&regBytes, QIODevice::ReadOnly);
        regStream.setVersion(QDataStream::Qt_5_12);
        try { clipboardBlobs = Serialization::readBoundedBlobMap(regStream); } catch (...) {}
    } else if (mimeData->hasFormat(MimeType::BlobRegistry)) {
        QByteArray regBytes = mimeData->data(MimeType::BlobRegistry);
        QDataStream regStream(&regBytes, QIODevice::ReadOnly);
        try { clipboardBlobs = Serialization::readBoundedBlobMap(regStream); } catch (...) {}
    }
    QByteArray itemData;

    if (mimeData->hasFormat(MimeType::ClipboardLegacy)) {
        itemData = mimeData->data(MimeType::ClipboardLegacy);
    }

    if (mimeData->hasFormat(MimeType::Clipboard)) {
        itemData = mimeData->data(MimeType::Clipboard);
    }

    // Register any new embedded-IC blobs and add the pasted items as a single undo step —
    // registering the blobs untracked would leave them orphaned in the registry forever after
    // an undo, since AddItemsCommand only knows about scene items, not the blob registry.
    const bool needsMacro = !clipboardBlobs.isEmpty() && !itemData.isEmpty();
    if (needsMacro) {
        m_scene->undoStack()->beginMacro(tr("Paste"));
    }

    if (!clipboardBlobs.isEmpty()) {
        auto *registry = m_scene->icRegistry();
        for (auto it = clipboardBlobs.cbegin(); it != clipboardBlobs.cend(); ++it) {
            if (!registry->hasBlob(it.key())) {
                m_scene->receiveCommand(new RegisterBlobCommand(it.key(), it.value(), m_scene));
            }
        }
    }

    if (!itemData.isEmpty()) {
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        QVersionNumber version = Serialization::readPandaHeader(stream);
        deserializeAndAdd(stream, version);
    }

    if (needsMacro) {
        m_scene->undoStack()->endMacro();
    }
}

bool ClipboardManager::canPaste(const QMimeData *mimeData)
{
    // Mirrors the formats paste() reads above — copy()/cut() write only the
    // current format, while older app versions wrote the legacy one.
    return mimeData
           && (mimeData->hasFormat(MimeType::Clipboard) || mimeData->hasFormat(MimeType::ClipboardLegacy));
}

QImage ClipboardManager::buildDragImage(Scene *scene, const QTransform &viewTransform, const QRectF &sourceRect)
{
    QSize mappedSize = viewTransform.mapRect(sourceRect).size().toSize();

    // Cap the ghost image at a sane maximum dimension. sourceRect derives from selected
    // elements' scene positions (a crafted/corrupted file, or a large paste offset, can place
    // one at an extreme-but-finite coordinate — the same gap CircuitExporter::renderToImage
    // hit), so mappedSize would otherwise be sized proportionally to that distance. Scale down
    // to fit instead of failing outright; the ghost is a transient visual aid, not saved output.
    if (mappedSize.width() > kMaxDragImageDimension || mappedSize.height() > kMaxDragImageDimension) {
        mappedSize.scale(kMaxDragImageDimension, kMaxDragImageDimension, Qt::KeepAspectRatio);
    }

    QImage image(mappedSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    // Opacity 0 makes the ghost transparent; the drag cursor shape still appears
    painter.setOpacity(0.0);
    scene->render(&painter, QRectF(QPointF(), mappedSize), sourceRect);
    painter.end();

    return image;
}

void ClipboardManager::cloneDrag(const QPointF &mousePos)
{
    auto *view = m_scene->view();
    sentryBreadcrumb("ui", QStringLiteral("Clone drag started"));
    qCDebug(zero) << "Ctrl + Drag action triggered.";
    const auto selectedElements = m_scene->selectedElements();

    if (selectedElements.isEmpty()) {
        return;
    }

    // --- Build drag pixmap ---
    // Temporarily hide non-selected items so the rendered image shows only
    // the selection, giving the drag ghost the correct visual appearance
    const auto items = m_scene->items();

    for (auto *item : items) {
        if (((item->type() == GraphicElement::Type) || (item->type() == Connection::Type)) && !item->isSelected()) {
            item->hide();
        }
    }

    QRectF rect;

    for (auto *element : selectedElements) {
        rect = rect.united(element->sceneBoundingRect());
    }

    // 8px padding avoids clipping port handles at the bounding-rect edges
    rect = rect.adjusted(-8, -8, 8, 8);

    const QImage image = buildDragImage(m_scene, view->transform(), rect);

    // Restore hidden items before the drag begins so the scene looks normal
    for (auto *item : items) {
        if (((item->type() == GraphicElement::Type) || (item->type() == Connection::Type)) && !item->isSelected()) {
            item->show();
        }
    }

    // --- Serialize selection for drop target ---
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    // Embed the mouse-press position so the drop handler can compute the correct offset
    stream << mousePos;
    serializeItems(m_scene->selectedItems(), stream);

    auto mimeData = std::make_unique<QMimeData>();
    mimeData->setData(MimeType::CloneDrag, itemData);

    auto *drag = new QDrag(m_scene);
    drag->setMimeData(mimeData.release());
    drag->setPixmap(QPixmap::fromImage(image));
    // Hot-spot aligns the drag image to the original element positions under the cursor
    QPointF offset = view->transform().map(mousePos - rect.topLeft());
    drag->setHotSpot(offset.toPoint());
    SimulationBlocker blocker(m_scene->simulation());
    drag->exec(Qt::CopyAction, Qt::CopyAction);
}

// --- Private helpers ---

void ClipboardManager::serializeItems(const QList<QGraphicsItem *> &items, QDataStream &stream)
{
    // Compute the centroid of all selected elements (not connections) so that
    // paste can place the clipboard contents relative to the cursor position
    QPointF center(0.0, 0.0);
    int itemsQuantity = 0;

    for (auto *item : items) {
        if (item->type() == GraphicElement::Type) {
            center += item->pos();
            ++itemsQuantity;
        }
    }

    stream << center / static_cast<qreal>(itemsQuantity);
    Serialization::serialize(items, stream);
}

void ClipboardManager::serializeAndDelete(const QList<QGraphicsItem *> &items, QDataStream &stream)
{
    serializeItems(items, stream);
    m_scene->deleteAction();
}

void ClipboardManager::deserializeAndAdd(QDataStream &stream, const QVersionNumber &version)
{
    m_scene->clearSelection();

    QPointF center; stream >> center;

    QHash<quint64, Port *> portMap;
    auto context = m_scene->deserializationContext(portMap, version);
    const auto itemList = Serialization::deserialize(stream, context);
    // Shift pasted elements so their centroid lands at the cursor position,
    // then nudge 32 px diagonally so repeated pastes are visually offset and
    // don't completely overlap the original selection.
    const QPointF offset = m_scene->mousePos() - center - QPointF(32.0, 32.0);

    m_scene->receiveCommand(new AddItemsCommand(itemList, m_scene));

    for (auto *item : itemList) {
        if (item->type() == GraphicElement::Type) {
            item->setPos((item->pos() + offset));
        }
    }

    m_scene->resizeScene();
}
