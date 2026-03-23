// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/ClipboardManager.h"

#include <QApplication>
#include <QClipboard>
#include <QDrag>
#include <QMimeData>
#include <QPainter>

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/Serialization.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Commands.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/SimulationBlocker.h"

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

    auto *mimeData = new QMimeData();
    mimeData->setData("application/x-wiredpanda-clipboard", itemData);

    QApplication::clipboard()->setMimeData(mimeData);
}

void ClipboardManager::cut()
{
    if (m_scene->selectedElements().isEmpty()) {
        QApplication::clipboard()->clear();
        return;
    }

    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    serializeAndDelete(m_scene->selectedItems(), stream);

    auto *mimeData = new QMimeData();
    mimeData->setData("application/x-wiredpanda-clipboard", itemData);

    QApplication::clipboard()->setMimeData(mimeData);
}

void ClipboardManager::paste()
{
    const auto *mimeData = QApplication::clipboard()->mimeData();

    QByteArray itemData;

    if (mimeData->hasFormat("wpanda/copydata")) {
        itemData = mimeData->data("wpanda/copydata");
    }

    if (mimeData->hasFormat("application/x-wiredpanda-clipboard")) {
        itemData = mimeData->data("application/x-wiredpanda-clipboard");
    }

    if (!itemData.isEmpty()) {
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        QVersionNumber version = Serialization::readPandaHeader(stream);
        deserializeAndAdd(stream, version);
    }
}

void ClipboardManager::cloneDrag(const QPointF &mousePos)
{
    auto *view = m_scene->view();
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
        if (((item->type() == GraphicElement::Type) || (item->type() == QNEConnection::Type)) && !item->isSelected()) {
            item->hide();
        }
    }

    QRectF rect;

    for (auto *element : selectedElements) {
        rect = rect.united(element->sceneBoundingRect());
    }

    // 8px padding avoids clipping port handles at the bounding-rect edges
    rect = rect.adjusted(-8, -8, 8, 8);

    auto mappedSize = view->transform().mapRect(rect).size().toSize();
    QImage image(mappedSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    // Opacity 0 makes the ghost transparent; the drag cursor shape still appears
    painter.setOpacity(0.0);
    QRectF target = image.rect();
    QRectF source = rect;
    m_scene->render(&painter, target, source);

    // Restore hidden items before the drag begins so the scene looks normal
    for (auto *item : items) {
        if (((item->type() == GraphicElement::Type) || (item->type() == QNEConnection::Type)) && !item->isSelected()) {
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

    auto *mimeData = new QMimeData();
    mimeData->setData("application/x-wiredpanda-cloneDrag", itemData);

    auto *drag = new QDrag(m_scene);
    drag->setMimeData(mimeData);
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

    QMap<quint64, QNEPort *> portMap;
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

