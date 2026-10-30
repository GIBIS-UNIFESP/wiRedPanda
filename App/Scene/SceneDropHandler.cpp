// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/SceneDropHandler.h"

#include <memory>

#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QtGlobal>

#include "App/Core/Common.h"
#include "App/Core/DragDropPayload.h"
#include "App/Core/Enums.h"
#include "App/Core/MimeTypes.h"
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Commands.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"

SceneDropHandler::SceneDropHandler(Scene *scene)
    : m_scene(scene)
{
}

bool SceneDropHandler::isSupportedDropFormat(const QMimeData *mimeData)
{
    // A new element's payload is small, so one that does not read is refused while it is dragged; a clone's items are
    // read when dropped
    const auto &formats = mimeData->formats();
    return formats.contains(MimeType::CloneDragLegacy)
           || formats.contains(MimeType::CloneDrag)
           || readDragDropPayload(mimeData).has_value();
}

void SceneDropHandler::handleNewElementDrop(QGraphicsSceneDragDropEvent *event)
{
    // Both MIME types carry the same payload; one from another program that does not read is refused
    const auto maybePayload = readDragDropPayload(event->mimeData());
    if (!maybePayload) {
        return;
    }
    const auto &payload = *maybePayload;

    // Subtract the drag offset so the element lands under the original grab point
    QPointF pos = event->scenePos() - payload.offset;
    qCDebug(zero) << payload.type << " at position: " << pos.x() << ", " << pos.y() << ", label: " << payload.icFileName;

    std::unique_ptr<Element> element(ElementFactory::buildElement(payload.type));
    qCDebug(zero) << "Valid element.";

    if (payload.isEmbedded && payload.type == ElementType::IC) {
        if (!m_scene->icRegistry()->initEmbeddedIC(static_cast<IC *>(element.get()), payload.blobName)) {
            return;
        }
    } else {
        // loadFromDrop can throw on malformed files; unique_ptr keeps the
        // freshly-allocated element from leaking when it does.
        element->loadFromDrop(payload.icFileName, m_scene->contextDir());
    }

    qCDebug(zero) << "Adding the element to the scene.";
    auto *raw = element.release();
    m_scene->receiveCommand(new AddItemsCommand({raw}, m_scene));

    qCDebug(zero) << "Cleaning the selection.";
    m_scene->clearSelection();

    qCDebug(zero) << "Setting created element as selected.";
    raw->setSelected(true);

    qCDebug(zero) << "Adjusting the position of the element.";
    raw->setPos(Element::snapToGrid(pos));
    m_scene->resizeScene();
}

void SceneDropHandler::handleCloneDrag(QGraphicsSceneDragDropEvent *event)
{
    QByteArray itemData;

    if (event->mimeData()->hasFormat(MimeType::CloneDragLegacy)) {
        itemData = event->mimeData()->data(MimeType::CloneDragLegacy);
    }

    if (event->mimeData()->hasFormat(MimeType::CloneDrag)) {
        itemData = event->mimeData()->data(MimeType::CloneDrag);
    }

    QDataStream stream(&itemData, QIODevice::ReadOnly);
    QVersionNumber version = Serialization::readPandaHeader(stream);

    // offset = mouse position at drag-start; recompute drop offset from current position
    QPointF offset; stream >> offset;
    QPointF ctr;    stream >> ctr;
    offset = event->scenePos() - offset;

    QHash<quint64, Port *> portMap;
    auto context = m_scene->deserializationContext(portMap, version, SerializationPurpose::InMemorySnapshot);
    const auto itemList = Serialization::deserialize(stream, context);

    m_scene->receiveCommand(new AddItemsCommand(itemList, m_scene));
    m_scene->clearSelection();

    for (auto *item : itemList) {
        if (item->type() == Element::Type) {
            item->setPos(Element::snapToGrid(item->pos() + offset));
            item->setSelected(true);
        }
    }

    m_scene->resizeScene();
    m_scene->reportDuplicateTxLabels(itemList);
}

void SceneDropHandler::addFromMimeData(QMimeData *mimeData, std::optional<QPointF> scenePos)
{
    // Owned from the start, so a payload that does not read does not leak it either
    auto mimeGuard = qScopeGuard([mimeData]() { mimeData->deleteLater(); });

    QByteArray itemData = mimeData->hasFormat(MimeType::DragDrop)
        ? mimeData->data(MimeType::DragDrop)
        : mimeData->data(MimeType::DragDropLegacy);
    QDataStream stream(&itemData, QIODevice::ReadOnly);
    Serialization::readPandaHeader(stream);

    const auto payload = readDragDropPayload(stream);

    std::unique_ptr<Element> element(ElementFactory::buildElement(payload.type));
    qCDebug(zero) << "Valid element.";

    if (payload.isEmbedded && payload.type == ElementType::IC) {
        if (!m_scene->icRegistry()->initEmbeddedIC(static_cast<IC *>(element.get()), payload.blobName)) {
            return;
        }
    } else {
        element->loadFromDrop(payload.icFileName, m_scene->contextDir());
    }

    qCDebug(zero) << "Adding the element to the scene.";
    auto *raw = element.release();
    m_scene->receiveCommand(new AddItemsCommand({raw}, m_scene));

    qCDebug(zero) << "Cleaning the selection.";
    m_scene->clearSelection();

    qCDebug(zero) << "Setting created element as selected.";
    raw->setSelected(true);

    // Add-without-drag (double-click / search): drop it where the caller asked (the view
    // centre) so it lands in sight instead of at the scene origin, snapped to the grid.
    if (scenePos) {
        raw->setPos(Element::snapToGrid(*scenePos));
    }

    m_scene->resizeScene();
}
