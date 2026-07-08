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
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Commands.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"

SceneDropHandler::SceneDropHandler(Scene *scene)
    : m_scene(scene)
{
}

bool SceneDropHandler::isSupportedDropFormat(const QMimeData *mimeData)
{
    const auto &formats = mimeData->formats();
    return formats.contains(MimeType::DragDropLegacy)
           || formats.contains(MimeType::CloneDragLegacy)
           || formats.contains(MimeType::DragDrop)
           || formats.contains(MimeType::CloneDrag);
}

void SceneDropHandler::handleNewElementDrop(QGraphicsSceneDragDropEvent *event)
{
    // Both MIME types carry the same payload; the newer format has a namespaced key
    QByteArray itemData;

    if (event->mimeData()->hasFormat(MimeType::DragDropLegacy)) {
        itemData = event->mimeData()->data(MimeType::DragDropLegacy);
    }

    if (event->mimeData()->hasFormat(MimeType::DragDrop)) {
        itemData = event->mimeData()->data(MimeType::DragDrop);
    }

    QDataStream stream(&itemData, QIODevice::ReadOnly);
    Serialization::readPandaHeader(stream);

    const auto payload = readDragDropPayload(stream);

    // Subtract the drag offset so the element lands under the original grab point
    QPointF pos = event->scenePos() - payload.offset;
    qCDebug(zero) << payload.type << " at position: " << pos.x() << ", " << pos.y() << ", label: " << payload.icFileName;

    std::unique_ptr<GraphicElement> element(ElementFactory::buildElement(payload.type));
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
    raw->setPos(pos);
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
        if (item->type() == GraphicElement::Type) {
            item->setPos((item->pos() + offset));
            item->setSelected(true);
        }
    }

    m_scene->resizeScene();
}

void SceneDropHandler::addFromMimeData(QMimeData *mimeData)
{
    QByteArray itemData = mimeData->hasFormat(MimeType::DragDrop)
        ? mimeData->data(MimeType::DragDrop)
        : mimeData->data(MimeType::DragDropLegacy);
    QDataStream stream(&itemData, QIODevice::ReadOnly);
    Serialization::readPandaHeader(stream);

    const auto payload = readDragDropPayload(stream);

    std::unique_ptr<GraphicElement> element(ElementFactory::buildElement(payload.type));
    qCDebug(zero) << "Valid element.";

    // RAII for mimeData too — the original deleteLater at the bottom is
    // unreachable if loadFromDrop throws, so the QMimeData would leak alongside
    // the half-built element.
    auto mimeGuard = qScopeGuard([mimeData]() { mimeData->deleteLater(); });

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
}
