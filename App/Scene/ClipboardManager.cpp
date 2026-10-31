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
#include <QSizeF>

#include "App/Core/Common.h"
#include "App/Core/Constants.h"
#include "App/Core/MimeTypes.h"
#include "App/Core/SentryHelpers.h"
#include "App/Element/Element.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
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

    // Everything is read before anything is recorded, so a clipboard that does not read throws with the scene, its
    // registry and its undo stack as they were. The new blobs are made self-contained as registering them would, and
    // the items read against the registry they will join.
    auto *registry = m_scene->icRegistry();
    QMap<QString, QByteArray> newBlobs;
    for (auto it = clipboardBlobs.cbegin(); it != clipboardBlobs.cend(); ++it) {
        if (!registry->hasBlob(it.key())) {
            newBlobs[it.key()] = registry->selfContainedBlob(it.key(), it.value());
        }
    }

    QList<QGraphicsItem *> items;
    QPointF center;
    if (!itemData.isEmpty()) {
        QMap<QString, QByteArray> blobs = registry->blobMap();
        blobs.insert(newBlobs);
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        const QVersionNumber version = Serialization::readPandaHeader(stream);
        items = readItems(stream, version, blobs, center);
    }

    // Register any new embedded-IC blobs and add the pasted items as a single undo step —
    // registering the blobs untracked would leave them orphaned in the registry forever after
    // an undo, since AddItemsCommand only knows about scene items, not the blob registry.
    const bool needsMacro = !clipboardBlobs.isEmpty() && !itemData.isEmpty();
    if (needsMacro) {
        m_scene->undoStack()->beginMacro(tr("Paste"));
    }

    for (auto it = newBlobs.cbegin(); it != newBlobs.cend(); ++it) {
        m_scene->receiveCommand(new RegisterBlobCommand(it.key(), it.value(), m_scene));
    }

    if (!itemData.isEmpty()) {
        addItems(items, center);
    }

    if (needsMacro) {
        m_scene->undoStack()->endMacro();
    }
    m_scene->reportDuplicateTxLabels(items);
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
    QSizeF mappedSize = viewTransform.mapRect(sourceRect).size();

    // Cap the ghost image at a sane maximum dimension. sourceRect derives from selected
    // elements' scene positions (a crafted/corrupted file, or a large paste offset, can place
    // one at an extreme-but-finite coordinate — the same gap CircuitExporter::renderToImage
    // hit), so mappedSize would otherwise be sized proportionally to that distance. Scale down
    // to fit instead of failing outright; the ghost is a transient visual aid, not saved output.
    // Scaled before it is rounded to whole pixels, since an extent past int's range has no int size.
    if (mappedSize.width() > kMaxDragImageDimension || mappedSize.height() > kMaxDragImageDimension) {
        mappedSize.scale(kMaxDragImageDimension, kMaxDragImageDimension, Qt::KeepAspectRatio);
    }

    QImage image(mappedSize.toSize(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    scene->render(&painter, QRectF(QPointF(), image.size()), sourceRect);
    painter.end();

    return image;
}

DragPreview::Picture ClipboardManager::selectionDragPicture(const QPointF &mousePos)
{
    const auto selectedElements = m_scene->selectedElements();

    if (selectedElements.isEmpty()) {
        return {};
    }

    // Temporarily hide non-selected items so the rendered image shows only the selection
    const auto items = m_scene->items();

    for (auto *item : items) {
        if (((item->type() == Element::Type) || (item->type() == Connection::Type)) && !item->isSelected()) {
            item->hide();
        }
    }

    QRectF rect;

    for (auto *element : selectedElements) {
        rect = rect.united(element->sceneOutlineRect());
    }

    // 8px padding avoids clipping port handles at the bounding-rect edges
    rect = rect.adjusted(-8, -8, 8, 8);

    // Rendered at the view's zoom and the screen's pixel ratio, so the picture is as sharp as the scene
    auto *view = m_scene->view();
    const qreal ratio = view ? view->devicePixelRatioF() : 1.0;
    const QTransform transform = (view ? view->transform() : QTransform()) * QTransform::fromScale(ratio, ratio);
    QPixmap pixmap = QPixmap::fromImage(buildDragImage(m_scene, transform, rect));

    for (auto *item : items) {
        if (((item->type() == Element::Type) || (item->type() == Connection::Type)) && !item->isSelected()) {
            item->show();
        }
    }

    // Drawn at the selection's size in the scene, however many pixels the image holds
    pixmap.setDevicePixelRatio(pixmap.width() / rect.width());
    // The drop moves each element by the cursor's travel since the press, snapped: for elements on the grid, the
    // selection's top-left moves to the snapped travel from where it is
    return {pixmap, mousePos, rect.topLeft()};
}

void ClipboardManager::cloneDrag(const QPointF &mousePos)
{
    sentryBreadcrumb("ui", QStringLiteral("Clone drag started"));
    qCDebug(zero) << "Ctrl + Drag action triggered.";

    if (m_scene->selectedElements().isEmpty()) {
        return;
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
    // The scene draws the copies where the drop will put them (see DragPreview); QDrag is given no picture.
    const DragPreview::Scope preview(selectionDragPicture(mousePos));
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
        if (item->type() == Element::Type) {
            center += item->pos();
            ++itemsQuantity;
        }
    }

    stream << center / static_cast<qreal>(itemsQuantity);
    Serialization::serialize(items, stream, {.purpose = SerializationPurpose::InMemorySnapshot});
}

void ClipboardManager::serializeAndDelete(const QList<QGraphicsItem *> &items, QDataStream &stream)
{
    serializeItems(items, stream);
    m_scene->deleteAction();
}

QList<QGraphicsItem *> ClipboardManager::readItems(QDataStream &stream, const QVersionNumber &version,
                                                  QMap<QString, QByteArray> &blobRegistry, QPointF &center)
{
    stream >> center;

    QHash<quint64, Port *> portMap;
    auto context = m_scene->deserializationContext(portMap, version, SerializationPurpose::InMemorySnapshot);
    context.blobRegistry = &blobRegistry;
    return Serialization::deserialize(stream, context);
}

void ClipboardManager::addItems(const QList<QGraphicsItem *> &items, const QPointF &center,
                                std::optional<QPointF> fixedOffset)
{
    m_scene->clearSelection();

    // Paste: shift elements so their centroid lands at the cursor, then nudge 32 px
    // diagonally so repeated pastes don't completely overlap. Duplicate: shift by exactly
    // fixedOffset from the originals so the copies sit a grid step down-right, in place.
    const QPointF offset = fixedOffset ? *fixedOffset
                                       : (m_scene->mousePos() - center - QPointF(32.0, 32.0));

    m_scene->receiveCommand(new AddItemsCommand(items, m_scene));

    for (auto *item : items) {
        if (item->type() == Element::Type) {
            item->setPos(Element::snapToGrid(item->pos() + offset));
        }
    }

    m_scene->resizeScene();
}

void ClipboardManager::duplicate()
{
    if (m_scene->selectedElements().empty()) {
        return;
    }

    // Serialize the selection to a private buffer so the system clipboard is left untouched.
    QByteArray itemData;
    QDataStream writeStream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(writeStream);
    serializeItems(m_scene->selectedItems(), writeStream);

    // Embedded-IC blobs referenced by the copies already live in this scene's registry
    // (same tab), so — unlike paste — no blob re-registration is needed.
    QDataStream readStream(&itemData, QIODevice::ReadOnly);
    const QVersionNumber version = Serialization::readPandaHeader(readStream);
    QPointF center;
    const auto added = readItems(readStream, version, m_scene->icRegistry()->blobMapRef(), center);
    addItems(added, center, QPointF(Constants::gridSize, Constants::gridSize));
    m_scene->reportDuplicateTxLabels(added);

    // Make the copies the active selection so they can be moved right away.
    for (auto *item : added) {
        item->setSelected(true);
    }
}
