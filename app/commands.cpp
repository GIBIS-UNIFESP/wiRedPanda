// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commands.h"

#include "common.h"
#include "elementfactory.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "qneconnection.h"
#include "qneport.h"
#include "scene.h"
#include "serializationfunctions.h"
#include "simulation.h"
#include "simulationblocker.h"

#include <QDrag>
#include <QGraphicsItem>
#include <QIODevice>
#include <cmath>
#include <stdexcept>

void storeIds(const QList<QGraphicsItem *> &items, QList<int> &ids)
{
    ids.reserve(items.size());
    for (auto *item : items) {
        if (auto *itemId = dynamic_cast<ItemWithId *>(item)) {
            ids.append(itemId->id());
        }
    }
}

void storeOtherIds(const QList<QGraphicsItem *> &connections, const QList<int> &ids, QList<int> &otherIds)
{
    for (auto *item : connections) {
        if (auto *conn = qgraphicsitem_cast<QNEConnection *>(item); item->type() == QNEConnection::Type && conn) {
            if (auto *p1 = conn->start(); p1 && p1->graphicElement() && !ids.contains(p1->graphicElement()->id())) {
                otherIds.append(p1->graphicElement()->id());
            }

            if (auto *p2 = conn->end(); p2 && p2->graphicElement() && !ids.contains(p2->graphicElement()->id())) {
                otherIds.append(p2->graphicElement()->id());
            }
        }
    }
}

QList<QGraphicsItem *> loadList(const QList<QGraphicsItem *> &items, QList<int> &ids, QList<int> &otherIds)
{
    QList<QGraphicsItem *> elements;
    /* Stores selected graphicElements */
    for (auto *item : items) {
        if (item->type() == GraphicElement::Type) {
            if (!elements.contains(item)) {
                elements.append(item);
            }
        }
    }

    QList<QGraphicsItem *> connections;
    /* Stores all the wires linked to these elements */
    for (auto *item : elements) {
        if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item)) {
            for (auto *port : elm->inputs()) {
                for (auto *conn : port->connections()) {
                    if (!connections.contains(conn)) {
                        connections.append(conn);
                    }
                }
            }

            for (auto *port : elm->outputs()) {
                for (auto *conn : port->connections()) {
                    if (!connections.contains(conn)) {
                        connections.append(conn);
                    }
                }
            }
        }
    }

    /* Stores the other wires selected */
    for (auto *item : items) {
        if (item->type() == QNEConnection::Type) {
            if (!connections.contains(item)) {
                connections.append(item);
            }
        }
    }

    /* Stores the ids of all elements listed in items; */
    storeIds(elements + connections, ids);
    /* Stores all the elements linked to each connection that will not be deleted. */
    storeOtherIds(connections, ids, otherIds);
    return elements + connections;
}

QList<QGraphicsItem *> findItems(const QList<int> &ids)
{
    QList<QGraphicsItem *> items;
    items.reserve(ids.size());

    for (const int id : ids) {
        if (auto *item = dynamic_cast<QGraphicsItem *>(ElementFactory::itemById(id))) {
            items.append(item);
        }
    }

    if (items.size() != ids.size()) {
        throw Pandaception(QObject::tr("One or more items was not found on the scene."));
    }

    return items;
}

QList<GraphicElement *> findElements(const QList<int> &ids)
{
    QList<GraphicElement *> items;
    items.reserve(ids.size());

    for (const int id : ids) {
        if (auto *item = dynamic_cast<GraphicElement *>(ElementFactory::itemById(id))) {
            items.append(item);
        }
    }

    if (items.size() != ids.size()) {
        throw Pandaception(QObject::tr("One or more elements was not found on the scene."));
    }

    return items;
}

QNEConnection *findConn(const int id)
{
    return dynamic_cast<QNEConnection *>(ElementFactory::itemById(id));
}

GraphicElement *findElm(const int id)
{
    return dynamic_cast<GraphicElement *>(ElementFactory::itemById(id));
}

void saveItems(QByteArray &itemData, const QList<QGraphicsItem *> &items, const QList<int> &otherIds)
{
    itemData.clear();
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_12);
    const auto others = findElements(otherIds);

    for (auto *elm : others) {
        elm->save(stream);
    }

    SerializationFunctions::serialize(items, stream);
}

void addItems(Scene *scene, const QList<QGraphicsItem *> &items)
{
    for (auto *item : items) {
        if (item->scene() != scene) {
            scene->addItem(item);
        }

        if (item->type() == GraphicElement::Type) {
            item->setSelected(true);
        }
    }
}

QList<QGraphicsItem *> loadItems(Scene *scene, QByteArray &itemData, const QList<int> &ids, QList<int> &otherIds)
{
    if (itemData.isEmpty()) {
        return {};
    }

    const auto otherElms = findElements(otherIds);
    QDataStream stream(&itemData, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_5_12);
    double version = GlobalProperties::version;
    QMap<quint64, QNEPort *> portMap;

    for (auto *elm : otherElms) {
        elm->load(stream, portMap, version);
    }

    /* Assuming that all connections are stored after the elements, we will deserialize the elements first.
     * We will store one additional information: The element IDs! */
    auto items = SerializationFunctions::deserialize(stream, version, portMap);

    if (items.size() != ids.size()) {
        throw Pandaception(QObject::tr("One or more elements were not found on scene. Expected %1, found %2.").arg(ids.size(), items.size()));
    }

    for (int i = 0; i < items.size(); ++i) {
        if (auto *itemId = dynamic_cast<ItemWithId *>(items.at(i))) {
            ElementFactory::updateItemId(itemId, ids.at(i));
        }
    }

    addItems(scene, items);
    return items;
}

void deleteItems(Scene *scene, const QList<QGraphicsItem *> &items)
{
    /* Delete items on reverse order */
    for (auto it = items.rbegin(); it != items.rend(); ++it) {
        scene->removeItem(*it);
        delete *it;
    }
}

AddItemsCommand::AddItemsCommand(const QList<QGraphicsItem *> &items, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
{
    SimulationBlocker blocker(m_scene->simulation());
    const auto items_ = loadList(items, m_ids, m_otherIds);
    addItems(m_scene, items_);
    setText(tr("Add %1 elements").arg(items_.size()));
}

void AddItemsCommand::undo()
{
    qCDebug(zero) << text();
    SimulationBlocker blocker(m_scene->simulation());
    const auto items = findItems(m_ids);
    saveItems(m_itemData, items, m_otherIds);
    deleteItems(m_scene, items);
    m_scene->setCircuitUpdateRequired();
}

void AddItemsCommand::redo()
{
    qCDebug(zero) << text();
    SimulationBlocker blocker(m_scene->simulation());
    loadItems(m_scene, m_itemData, m_ids, m_otherIds);
    m_scene->setCircuitUpdateRequired();
}

DeleteItemsCommand::DeleteItemsCommand(const QList<QGraphicsItem *> &items, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
{
    SimulationBlocker blocker(m_scene->simulation());
    auto items_ = loadList(items, m_ids, m_otherIds);
    setText(tr("Delete %1 elements").arg(items_.size()));
}

void DeleteItemsCommand::undo()
{
    qCDebug(zero) << text();
    SimulationBlocker blocker(m_scene->simulation());
    loadItems(m_scene, m_itemData, m_ids, m_otherIds);
    m_scene->setCircuitUpdateRequired();
}

void DeleteItemsCommand::redo()
{
    qCDebug(zero) << text();
    SimulationBlocker blocker(m_scene->simulation());
    auto items = findItems(m_ids);
    saveItems(m_itemData, items, m_otherIds);
    deleteItems(m_scene, items);
    m_scene->setCircuitUpdateRequired();
}

RotateCommand::RotateCommand(const QList<GraphicElement *> &items, const int angle, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_angle(angle)
    , m_scene(scene)
{
    setText(tr("Rotate %1 degrees").arg(m_angle));
    m_ids.reserve(items.size());
    m_positions.reserve(items.size());

    for (auto *item : items) {
        m_positions.append(item->pos());
        item->setPos(item->pos());
        m_ids.append(item->id());
    }
}

void RotateCommand::undo()
{
    qCDebug(zero) << text();
    const auto elements = findElements(m_ids);

    for (int i = 0; i < elements.size(); ++i) {
        auto *elm = elements.at(i);
        elm->setRotation(elm->rotation() - m_angle);
        elm->setPos(m_positions.at(i));
        elm->update();
        elm->setSelected(true);
    }

    m_scene->setAutosaveRequired();
}

void RotateCommand::redo()
{
    qCDebug(zero) << text();
    const auto elements = findElements(m_ids);
    double cx = 0;
    double cy = 0;
    int sz = 0;

    for (auto *elm : elements) {
        cx += elm->pos().x();
        cy += elm->pos().y();
        sz++;
    }

    if (sz != 0) {
        cx /= sz;
        cy /= sz;
    }

    QTransform transform;
    transform.translate(cx, cy);
    transform.rotate(m_angle);
    transform.translate(-cx, -cy);

    for (auto *elm : elements) {
        elm->setPos(transform.map(elm->pos()));
        elm->setRotation(elm->rotation() + m_angle);
    }

    m_scene->setAutosaveRequired();
}

MoveCommand::MoveCommand(const QList<GraphicElement *> &list, const QList<QPointF> &oldPositions, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_oldPositions(oldPositions)
    , m_scene(scene)
{
    m_newPositions.reserve(list.size());
    m_ids.reserve(list.size());

    for (auto *elm : list) {
        m_ids.append(elm->id());
        m_newPositions.append(elm->pos());
    }

    setText(tr("Move elements"));
}

void MoveCommand::undo()
{
    qCDebug(zero) << text();
    const auto elements = findElements(m_ids);

    for (int i = 0; i < elements.size(); ++i) {
        elements.at(i)->setPos(m_oldPositions.at(i));
    }

    m_scene->setAutosaveRequired();
}

void MoveCommand::redo()
{
    qCDebug(zero) << text();
    const auto elements = findElements(m_ids);

    for (int i = 0; i < elements.size(); ++i) {
        elements.at(i)->setPos(m_newPositions.at(i));
    }

    m_scene->setAutosaveRequired();
}

UpdateCommand::UpdateCommand(const QList<GraphicElement *> &elements, const QByteArray &oldData, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_oldData(oldData)
    , m_scene(scene)
{
    m_ids.reserve(elements.size());
    QDataStream stream(&m_newData, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_12);

    for (auto *elm : elements) {
        elm->save(stream);
        m_ids.append(elm->id());
    }

    setText(tr("Update %1 elements").arg(elements.size()));
}

void UpdateCommand::undo()
{
    qCDebug(zero) << text();
    loadData(m_oldData);
    m_scene->setCircuitUpdateRequired();
}

void UpdateCommand::redo()
{
    qCDebug(zero) << text();
    loadData(m_newData);
    m_scene->setCircuitUpdateRequired();
}

void UpdateCommand::loadData(QByteArray &itemData)
{
    const auto elements = findElements(m_ids);

    if (elements.isEmpty()) {
        return;
    }

    QDataStream stream(&itemData, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_5_12);
    QMap<quint64, QNEPort *> portMap;

    double version = GlobalProperties::version;

    for (auto *elm : elements) {
        elm->load(stream, portMap, version);
        elm->setSelected(true);
    }
}

SplitCommand::SplitCommand(QNEConnection *conn, QPointF mousePos, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
{
    auto *node = ElementFactory::buildElement(ElementType::Node);

    /* Align node to Grid */
    m_nodePos = mousePos - node->boundingRect().center();
    const int gridSize = GlobalProperties::gridSize;
    qreal xV = qRound(m_nodePos.x() / gridSize) * gridSize;
    qreal yV = qRound(m_nodePos.y() / gridSize) * gridSize;
    m_nodePos = QPointF(xV, yV);
    /* Rotate line according to angle between p1 and p2 */
    const int angle = static_cast<int>(conn->angle());
    m_nodeAngle = static_cast<int>(360 - 90 * (std::round(angle / 90.0)));

    /* Assigning class attributes */
    m_elm1Id = conn->start()->graphicElement()->id();
    m_elm2Id = conn->end()->graphicElement()->id();

    m_c1Id = conn->id();
    m_c2Id = ElementFactory::buildConnection()->id();

    m_nodeId = node->id();

    setText(tr("Wire split"));
}

void SplitCommand::redo()
{
    qCDebug(zero) << text();
    auto *conn1 = findConn(m_c1Id);
    auto *conn2 = findConn(m_c2Id);
    auto *node = findElm(m_nodeId);
    auto *elm1 = findElm(m_elm1Id);
    auto *elm2 = findElm(m_elm2Id);

    if (!conn2) {
        conn2 = ElementFactory::buildConnection();
        ElementFactory::updateItemId(conn2, m_c2Id);
    }

    if (!node) {
        node = ElementFactory::buildElement(ElementType::Node);
        ElementFactory::updateItemId(node, m_nodeId);
    }

    if (!conn1 || !conn2 || !elm1 || !elm2 || !node) {
        throw Pandaception(tr("Error trying to redo ") + text());
    }

    node->setPos(m_nodePos);
    node->setRotation(m_nodeAngle);

    auto *endPort = conn1->end();
    conn2->setStart(node->outputPort());
    conn2->setEnd(endPort);
    conn1->setEnd(node->inputPort());

    m_scene->addItem(node);
    m_scene->addItem(conn2);

    conn1->updatePosFromPorts();
    conn2->updatePosFromPorts();

    m_scene->setCircuitUpdateRequired();
}

void SplitCommand::undo()
{
    qCDebug(zero) << text();
    auto *conn1 = findConn(m_c1Id);
    auto *conn2 = findConn(m_c2Id);
    auto *node = findElm(m_nodeId);
    auto *elm1 = findElm(m_elm1Id);
    auto *elm2 = findElm(m_elm2Id);

    if (!conn1 || !conn2 || !elm1 || !elm2 || !node) {
        throw Pandaception(tr("Error trying to undo ") + text());
    }

    conn1->setEnd(conn2->end());

    conn1->updatePosFromPorts();

    m_scene->removeItem(conn2);
    m_scene->removeItem(node);

    delete conn2;
    delete node;
    m_scene->setCircuitUpdateRequired();
}

MorphCommand::MorphCommand(const QList<GraphicElement *> &elements, ElementType type, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_newType(type)
    , m_scene(scene)
{
    m_ids.reserve(elements.size());
    m_types.reserve(elements.size());

    for (auto *oldElm : elements) {
        m_ids.append(oldElm->id());
        m_types.append(oldElm->elementType());
    }

    setText(tr("Morph %1 elements to %2").arg(elements.size()).arg(elements.constFirst()->objectName()));
}

void MorphCommand::undo()
{
    qCDebug(zero) << text();
    QList<GraphicElement *> newElms = findElements(m_ids);
    QList<GraphicElement *> oldElms;
    oldElms.reserve(m_ids.size());

    for (int i = 0; i < m_ids.size(); ++i) {
        oldElms << ElementFactory::buildElement(m_types.at(i));
    }

    transferConnections(newElms, oldElms);
    m_scene->setCircuitUpdateRequired();
}

void MorphCommand::redo()
{
    qCDebug(zero) << text();
    QList<GraphicElement *> oldElms = findElements(m_ids);
    QList<GraphicElement *> newElms;
    newElms.reserve(m_ids.size());

    for (int i = 0; i < m_ids.size(); ++i) {
        newElms << ElementFactory::buildElement(m_newType);
    }

    transferConnections(oldElms, newElms);
    m_scene->setCircuitUpdateRequired();
}

void MorphCommand::transferConnections(QList<GraphicElement *> from, QList<GraphicElement *> to)
{
    for (int elm = 0; elm < from.size(); ++elm) {
        auto *oldElm = from.at(elm);
        auto *newElm = to.at(elm);

        newElm->setInputSize(oldElm->inputSize());
        newElm->setPos(oldElm->pos());

        if (oldElm->elementType() == ElementType::Not && newElm->elementType() == ElementType::Node) {
            newElm->moveBy(16, 16);
        }

        if (oldElm->elementType() == ElementType::Node && newElm->elementType() == ElementType::Not) {
            newElm->moveBy(-16, -16);
        }

        if (newElm->isRotatable() && oldElm->isRotatable()) {
            newElm->setRotation(oldElm->rotation());
        }

        if (newElm->hasLabel() && oldElm->hasLabel() && (oldElm->elementType() != ElementType::Buzzer)) {
            newElm->setLabel(oldElm->label());
        }

        if (newElm->hasColors() && oldElm->hasColors()) {
            newElm->setColor(oldElm->color());
        }

        if (newElm->hasFrequency() && oldElm->hasFrequency()) {
            newElm->setFrequency(oldElm->frequency());
        }

        if (newElm->hasTrigger() && oldElm->hasTrigger()) {
            newElm->setTrigger(oldElm->trigger());
        }

        for (int port = 0; port < oldElm->inputSize(); ++port) {
            while (!oldElm->inputPort(port)->connections().isEmpty()) {
                if (auto *conn = oldElm->inputPort(port)->connections().constFirst();
                        conn && conn->end() == oldElm->inputPort(port)) {
                    conn->setEnd(newElm->inputPort(port));
                }
            }
        }

        for (int port = 0; port < oldElm->outputSize(); ++port) {
            while (!oldElm->outputPort(port)->connections().isEmpty()) {
                if (auto *conn = oldElm->outputPort(port)->connections().constFirst();
                        conn && conn->start() == oldElm->outputPort(port)) {
                    conn->setStart(newElm->outputPort(port));
                }
            }
        }

        const int oldId = oldElm->id();
        m_scene->removeItem(oldElm);
        delete oldElm;

        ElementFactory::updateItemId(newElm, oldId);
        m_scene->addItem(newElm);
        newElm->updatePortsProperties();
    }
}

// FIXME: verticalFlip is rotating on the horizontal axis too
FlipCommand::FlipCommand(const QList<GraphicElement *> &items, const int axis, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
    , m_axis(axis)
{
    if (items.isEmpty()) {
        return;
    }

    setText(tr("Flip %1 elements in axis %2").arg(items.size(), axis));
    m_ids.reserve(items.size());
    m_positions.reserve(items.size());
    double xmin = items.constFirst()->pos().rx();
    double ymin = items.constFirst()->pos().ry();
    double xmax = xmin;
    double ymax = ymin;

    for (auto *item : items) {
        m_positions.append(item->pos());
        m_ids.append(item->id());
        xmin = qMin(xmin, item->pos().rx());
        ymin = qMin(ymin, item->pos().ry());
        xmax = qMax(xmax, item->pos().rx());
        ymax = qMax(ymax, item->pos().ry());
    }

    m_minPos = QPointF(xmin, ymin);
    m_maxPos = QPointF(xmax, ymax);
}

void FlipCommand::undo()
{
    qCDebug(zero) << text();
    redo();
}

void FlipCommand::redo()
{
    qCDebug(zero) << text();
    const auto list = findElements(m_ids);

    for (auto *elm : list) {
        auto pos = elm->pos();

        (m_axis == 0) ? pos.setX(m_minPos.rx() + (m_maxPos.rx() - pos.rx()))
                      : pos.setY(m_minPos.ry() + (m_maxPos.ry() - pos.ry()));

        elm->setPos(pos);

        if (elm->isRotatable()) {
            elm->setRotation(elm->rotation() + 180);
        }
    }

    m_scene->setAutosaveRequired();
}

ChangeInputSizeCommand::ChangeInputSizeCommand(const QList<GraphicElement *> &elements, const int newInputSize, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
    , m_newInputSize(newInputSize)
{
    m_ids.reserve(elements.size());

    for (auto *elm : elements) {
        m_ids.append(elm->id());
    }

    setText(tr("Change input size to %1").arg(newInputSize));
}

void ChangeInputSizeCommand::redo()
{
    qCDebug(zero) << text();
    const auto m_elements = findElements(m_ids);

    QList<GraphicElement *> serializationOrder;
    serializationOrder.reserve(m_elements.size());
    m_oldData.clear();
    QDataStream stream(&m_oldData, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_12);

    for (auto *elm : m_elements) {
        elm->save(stream);
        serializationOrder.append(elm);

        for (int port = m_newInputSize; port < elm->inputSize(); ++port) {
            for (auto *conn : elm->inputPort(port)->connections()) {
                auto *otherPort = conn->otherPort(elm->inputPort(port));
                otherPort->graphicElement()->save(stream);
                serializationOrder.append(otherPort->graphicElement());
            }
        }
    }

    for (auto *elm : m_elements) {
        for (int port = m_newInputSize; port < elm->inputSize(); ++port) {
            while (!elm->inputPort(port)->connections().isEmpty()) {
                auto *conn = elm->inputPort(port)->connections().constFirst();
                conn->save(stream);
                m_scene->removeItem(conn);
                auto *otherPort = conn->otherPort(elm->inputPort(port));
                elm->inputPort(port)->disconnect(conn);
                otherPort->disconnect(conn);
            }
        }

        elm->setInputSize(m_newInputSize);
    }

    m_order.clear();

    for (auto *elm : serializationOrder) {
        m_order.append(elm->id());
    }

    m_scene->setCircuitUpdateRequired();
}

void ChangeInputSizeCommand::undo()
{
    qCDebug(zero) << text();
    const auto m_elements = findElements(m_ids);
    const auto serializationOrder = findElements(m_order);

    QDataStream stream(&m_oldData, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_5_12);
    double version = GlobalProperties::version;
    QMap<quint64, QNEPort *> portMap;

    for (auto *elm : serializationOrder) {
        elm->load(stream, portMap, version);
    }

    for (auto *elm : m_elements) {
        for (int in = m_newInputSize; in < elm->inputSize(); ++in) {
            auto *conn = ElementFactory::buildConnection();
            conn->load(stream, portMap);
            m_scene->addItem(conn);
        }

        elm->setSelected(true);
    }

    m_scene->setCircuitUpdateRequired();
}

ChangeOutputSizeCommand::ChangeOutputSizeCommand(const QList<GraphicElement *> &elements, const int newOutputSize, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
    , m_newOutputSize(newOutputSize)
{
    m_ids.reserve(elements.size());

    for (auto *elm : elements) {
        m_ids.append(elm->id());
    }

    setText(tr("Change input size to %1").arg(newOutputSize));
}

void ChangeOutputSizeCommand::redo()
{
    qCDebug(zero) << text();
    const auto m_elements = findElements(m_ids);

    QList<GraphicElement *> serializationOrder;
    m_oldData.clear();
    QDataStream stream(&m_oldData, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_12);
    serializationOrder.reserve(m_elements.size());

    for (auto *elm : m_elements) {
        elm->save(stream);
        serializationOrder.append(elm);

        for (int port = m_newOutputSize; port < elm->outputSize(); ++port) {
            for (auto *conn : elm->outputPort(port)->connections()) {
                auto *otherPort = conn->otherPort(elm->outputPort(port));
                otherPort->graphicElement()->save(stream);
                serializationOrder.append(otherPort->graphicElement());
            }
        }
    }

    for (auto *elm : m_elements) {
        for (int port = m_newOutputSize; port < elm->outputSize(); ++port) {
            while (!elm->outputPort(port)->connections().isEmpty()) {
                auto *conn = elm->outputPort(port)->connections().constFirst();
                conn->save(stream);
                m_scene->removeItem(conn);
                auto *otherPort = conn->otherPort(elm->outputPort(port));
                elm->outputPort(port)->disconnect(conn);
                otherPort->disconnect(conn);
            }
        }

        elm->setOutputSize(m_newOutputSize);
        elm->setSelected(true);
    }

    m_order.clear();

    for (auto *elm : serializationOrder) {
        m_order.append(elm->id());
    }

    m_scene->setCircuitUpdateRequired();
}

void ChangeOutputSizeCommand::undo()
{
    qCDebug(zero) << text();
    const auto elements = findElements(m_ids);
    const auto serializationOrder = findElements(m_order);

    QDataStream stream(&m_oldData, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_5_12);
    double version = GlobalProperties::version;
    QMap<quint64, QNEPort *> portMap;

    for (auto *elm : serializationOrder) {
        elm->load(stream, portMap, version);
    }

    for (auto *elm : elements) {
        for (int out = m_newOutputSize; out < elm->outputSize(); ++out) {
            auto *conn = ElementFactory::buildConnection();
            conn->load(stream, portMap);
            m_scene->addItem(conn);
        }

        elm->setSelected(true);
    }

    m_scene->setCircuitUpdateRequired();
}
