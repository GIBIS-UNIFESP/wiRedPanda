// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/Commands.h"

#include <cmath>

#include <QCoreApplication>
#include <QIODevice>

#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationBlocker.h"

namespace CommandUtils {

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
    // Track elements on the opposite end of connections that are NOT being deleted.
    // These elements must be re-saved/re-loaded during undo so their port state
    // (connected/disconnected) is correctly restored after the operation.
    for (auto *item : connections) {
        if (auto *conn = qgraphicsitem_cast<QNEConnection *>(item)) {
            if (auto *port1 = conn->startPort(); port1 && port1->graphicElement() && !ids.contains(port1->graphicElement()->id())) {
                otherIds.append(port1->graphicElement()->id());
            }

            if (auto *port2 = conn->endPort(); port2 && port2->graphicElement() && !ids.contains(port2->graphicElement()->id())) {
                otherIds.append(port2->graphicElement()->id());
            }
        }
    }
}

const QList<QGraphicsItem *> loadList(const QList<QGraphicsItem *> &items, QList<int> &ids, QList<int> &otherIds)
{
    // --- Collect elements in the operation ---
    QList<QGraphicsItem *> elements;
    /* Stores selected graphicElements */
    for (auto *item : items) {
        if (item->type() == GraphicElement::Type) {
            if (!elements.contains(item)) {
                elements.append(item);
            }
        }
    }

    // --- Collect all wires attached to those elements ---
    // Always include the wires connected to the affected elements, even if the
    // user only selected the element body; this keeps the topology consistent
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

const QList<QGraphicsItem *> findItems(const QList<int> &ids)
{
    QList<QGraphicsItem *> items;
    items.reserve(ids.size());

    for (const int id : ids) {
        if (auto *item = dynamic_cast<QGraphicsItem *>(ElementFactory::itemById(id))) {
            items.append(item);
        }
    }

    if (items.size() != ids.size()) {
        throw PANDACEPTION_WITH_CONTEXT("commands", "One or more items was not found on the scene.");
    }

    return items;
}

const QList<GraphicElement *> findElements(const QList<int> &ids)
{
    QList<GraphicElement *> items;
    items.reserve(ids.size());

    for (const int id : ids) {
        if (auto *item = dynamic_cast<GraphicElement *>(ElementFactory::itemById(id))) {
            items.append(item);
        }
    }

    if (items.size() != ids.size()) {
        throw PANDACEPTION_WITH_CONTEXT("commands", "One or more elements was not found on the scene.");
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
    Serialization::writePandaHeader(stream);

    const auto others = CommandUtils::findElements(otherIds);

    for (auto *elm : others) {
        elm->save(stream);
    }

    Serialization::serialize(items, stream);
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

const QList<QGraphicsItem *> loadItems(Scene *scene, QByteArray &itemData, const QList<int> &ids, QList<int> &otherIds)
{
    if (itemData.isEmpty()) {
        return {};
    }

    QDataStream stream(&itemData, QIODevice::ReadOnly);
    QVersionNumber version = Serialization::readPandaHeader(stream);

    QMap<quint64, QNEPort *> portMap;
    SerializationContext context{portMap, version, Serialization::contextDir};

    for (auto *elm : CommandUtils::findElements(otherIds)) {
        elm->load(stream, context);
    }

    /* Assuming that all connections are stored after the elements, we will deserialize the elements first.
     * We will store one additional information: The element IDs! */
    const auto items = Serialization::deserialize(stream, context);

    if (items.size() != ids.size()) {
        throw PANDACEPTION_WITH_CONTEXT("commands", "One or more elements were not found on scene. Expected %1, found %2.", ids.size(), items.size());
    }

    // Re-assign the original IDs so undo/redo chains that store IDs remain valid
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

}  // namespace CommandUtils

AddItemsCommand::AddItemsCommand(const QList<QGraphicsItem *> &items, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
{
    // Simulation must be paused while items are added to avoid a partial-topology update
    SimulationBlocker blocker(m_scene->simulation());
    // Note: QUndoStack::push() calls redo() immediately after construction, so
    // the constructor itself acts as the first redo — items are added here, not later.
    const auto items_ = CommandUtils::loadList(items, m_ids, m_otherIds);
    CommandUtils::addItems(m_scene, items_);
    setText(tr("Add %1 elements").arg(items_.size()));
}

void AddItemsCommand::undo()
{
    qCDebug(zero) << text();
    SimulationBlocker blocker(m_scene->simulation());
    const auto items = CommandUtils::findItems(m_ids);
    CommandUtils::saveItems(m_itemData, items, m_otherIds);
    CommandUtils::deleteItems(m_scene, items);
    m_scene->setCircuitUpdateRequired();
}

void AddItemsCommand::redo()
{
    qCDebug(zero) << text();
    SimulationBlocker blocker(m_scene->simulation());
    CommandUtils::loadItems(m_scene, m_itemData, m_ids, m_otherIds);
    m_scene->setCircuitUpdateRequired();
}

DeleteItemsCommand::DeleteItemsCommand(const QList<QGraphicsItem *> &items, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
{
    SimulationBlocker blocker(m_scene->simulation());
    // Unlike AddItemsCommand, the constructor only captures IDs — it does NOT delete yet.
    // Deletion happens in redo() so that QUndoStack::push() triggers the first delete,
    // keeping the constructor side-effect-free for safe exception handling.
    const auto items_ = CommandUtils::loadList(items, m_ids, m_otherIds);
    setText(tr("Delete %1 elements").arg(items_.size()));
}

void DeleteItemsCommand::undo()
{
    qCDebug(zero) << text();
    SimulationBlocker blocker(m_scene->simulation());
    CommandUtils::loadItems(m_scene, m_itemData, m_ids, m_otherIds);
    m_scene->setCircuitUpdateRequired();
}

void DeleteItemsCommand::redo()
{
    qCDebug(zero) << text();
    SimulationBlocker blocker(m_scene->simulation());
    const auto items = CommandUtils::findItems(m_ids);
    CommandUtils::saveItems(m_itemData, items, m_otherIds);
    CommandUtils::deleteItems(m_scene, items);
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
    const auto elements = CommandUtils::findElements(m_ids);

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
    const auto elements = CommandUtils::findElements(m_ids);
    double cx = 0;
    double cy = 0;
    int sz = 0;

    for (auto *elm : elements) {
        cx += elm->pos().x();
        cy += elm->pos().y();
        ++sz;
    }

    if (sz != 0) {
        cx /= sz;
        cy /= sz;
    }

    // --- Apply group rotation via 2D transform ---
    // Translate-rotate-translate-back maps each element position around the centroid
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
    const auto elements = CommandUtils::findElements(m_ids);

    for (int i = 0; i < elements.size(); ++i) {
        elements.at(i)->setPos(m_oldPositions.at(i));
    }

    m_scene->setAutosaveRequired();
}

void MoveCommand::redo()
{
    qCDebug(zero) << text();
    const auto elements = CommandUtils::findElements(m_ids);

    for (int i = 0; i < elements.size(); ++i) {
        elements.at(i)->setPos(m_newPositions.at(i));
    }

    m_scene->setAutosaveRequired();
}

UpdateCommand::UpdateCommand(const QList<GraphicElement *> &elements, const QByteArray &oldData, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    // oldData must be captured by the caller *before* applying the change, so that
    // undo() can restore the previous state; the constructor cannot capture it here
    // because the change has already been applied by the time construction runs.
    , m_oldData(oldData)
    , m_scene(scene)
{
    m_ids.reserve(elements.size());
    QDataStream stream(&m_newData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);

    // Snapshot the current (post-change) state as the "new" data used by redo()
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
    const auto elements = CommandUtils::findElements(m_ids);

    if (elements.isEmpty()) {
        return;
    }

    QDataStream stream(&itemData, QIODevice::ReadOnly);
    QVersionNumber version = Serialization::readPandaHeader(stream);

    QMap<quint64, QNEPort *> portMap;
    SerializationContext context{portMap, version, Serialization::contextDir};

    for (auto *elm : elements) {
        elm->load(stream, context);
        elm->setSelected(true);
    }
}

SplitCommand::SplitCommand(QNEConnection *conn, QPointF mousePos, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
{
    auto *node = ElementFactory::buildElement(ElementType::Node);

    /* Align node to Grid */
    // Subtract pixmapCenter so the node's visual center lands on the mouse click,
    // then snap to the nearest grid intersection
    m_nodePos = mousePos - node->pixmapCenter();
    const int gridSize = Scene::gridSize;
    qreal xV = qRound(m_nodePos.x() / gridSize) * gridSize;
    qreal yV = qRound(m_nodePos.y() / gridSize) * gridSize;
    m_nodePos = QPointF(xV, yV);

    /* Rotate line according to angle between p1 and p2 */
    // QLineF::angle() is counter-clockwise from the positive X axis (0–360°).
    // Dividing by 90 and rounding quantises to the nearest cardinal direction (0–3),
    // then (360 − 90*step) converts that back to a clockwise Qt rotation angle so
    // the node's arrow graphic points in the correct direction along the wire.
    const int angle = static_cast<int>(conn->angle());
    m_nodeAngle = static_cast<int>(360 - 90 * (std::round(angle / 90.0)));

    /* Assigning class attributes */
    auto *startPort = conn->startPort();
    auto *endPort = conn->endPort();
    if (!startPort || !endPort) {
        throw PANDACEPTION("Invalid connection ports in SplitCommand constructor");
    }

    auto *startElement = startPort->graphicElement();
    auto *endElement = endPort->graphicElement();
    if (!startElement || !endElement) {
        throw PANDACEPTION("Invalid graphic elements in SplitCommand constructor");
    }

    m_elm1Id = startElement->id();
    m_elm2Id = endElement->id();

    m_c1Id = conn->id();
    // Pre-allocate a second QNEConnection solely to claim a unique ID from ElementFactory
    // before the constructor returns.  The object is immediately orphaned (no scene or
    // parent), but its ID is registered and will be reused by redo() so that the split
    // wire's second segment always has the same stable ID across multiple undo/redo cycles.
    m_c2Id = (new QNEConnection())->id();

    m_nodeId = node->id();

    setText(tr("Wire split"));
}

void SplitCommand::redo()
{
    qCDebug(zero) << text();
    auto *conn1 = CommandUtils::findConn(m_c1Id);
    auto *conn2 = CommandUtils::findConn(m_c2Id);
    auto *node = CommandUtils::findElm(m_nodeId);
    auto *elm1 = CommandUtils::findElm(m_elm1Id);
    auto *elm2 = CommandUtils::findElm(m_elm2Id);

    // After undo(), conn2 and node were deleted; recreate them with the same
    // stable IDs so subsequent redo() calls find them correctly via findConn/findElm
    if (!conn2) {
        conn2 = new QNEConnection();
        ElementFactory::updateItemId(conn2, m_c2Id);
    }

    if (!node) {
        node = ElementFactory::buildElement(ElementType::Node);
        ElementFactory::updateItemId(node, m_nodeId);
    }

    if (!conn1 || !conn2 || !elm1 || !elm2 || !node) {
        throw PANDACEPTION("Error trying to redo %1", text());
    }

    node->setPos(m_nodePos);
    node->setRotation(m_nodeAngle);

    auto *endPort = conn1->endPort();
    if (!endPort) {
        throw PANDACEPTION("Error: endPort is null in SplitCommand::redo()");
    }

    // Wire topology after split: elm1 → conn1 → node → conn2 → elm2
    conn2->setStartPort(node->outputPort());
    conn2->setEndPort(endPort);
    conn1->setEndPort(node->inputPort());

    m_scene->addItem(node);
    m_scene->addItem(conn2);

    conn1->updatePosFromPorts();
    conn2->updatePosFromPorts();

    m_scene->setCircuitUpdateRequired();
}

void SplitCommand::undo()
{
    qCDebug(zero) << text();
    auto *conn1 = CommandUtils::findConn(m_c1Id);
    auto *conn2 = CommandUtils::findConn(m_c2Id);
    auto *node = CommandUtils::findElm(m_nodeId);
    auto *elm1 = CommandUtils::findElm(m_elm1Id);
    auto *elm2 = CommandUtils::findElm(m_elm2Id);

    if (!conn1 || !conn2 || !elm1 || !elm2 || !node) {
        throw PANDACEPTION("Error trying to undo %1", text());
    }

    // Restore the original direct wire: conn1 skips the node and connects straight to elm2
    conn1->setEndPort(conn2->endPort());

    conn1->updatePosFromPorts();

    // Remove the node and the second wire segment introduced by the split
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
        // Store both the ID and the original type so undo() can rebuild the old element
        // with the correct type and then re-assign the same ID via updateItemId()
        m_ids.append(oldElm->id());
        m_types.append(oldElm->elementType());
    }

    setText(tr("Morph %1 elements to %2").arg(elements.size()).arg(elements.constFirst()->objectName()));
}

void MorphCommand::undo()
{
    qCDebug(zero) << text();
    auto newElms = CommandUtils::findElements(m_ids);
    decltype(newElms) oldElms;
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
    auto oldElms = CommandUtils::findElements(m_ids);
    decltype(oldElms) newElms;
    newElms.reserve(m_ids.size());

    for (int i = 0; i < m_ids.size(); ++i) {
        newElms << ElementFactory::buildElement(m_newType);
    }

    transferConnections(oldElms, newElms);
    m_scene->setCircuitUpdateRequired();
}

void MorphCommand::transferConnections(const QList<GraphicElement *> &from, const QList<GraphicElement *> &to)
{
    for (int elm = 0; elm < from.size(); ++elm) {
        auto *oldElm = from.at(elm);
        auto *newElm = to.at(elm);

        newElm->setInputSize(oldElm->inputSize());
        newElm->setPos(oldElm->pos());

        // Not↔Node morphs need a 16px position adjustment because the two element
        // types have different pixmap sizes and their visual centers differ by that amount
        if ((oldElm->elementType() == ElementType::Not) && (newElm->elementType() == ElementType::Node)) {
            newElm->moveBy(16, 16);
        }

        if ((oldElm->elementType() == ElementType::Node) && (newElm->elementType() == ElementType::Not)) {
            newElm->moveBy(-16, -16);
        }

        // Copy over all compatible properties; each is guarded by capability checks
        // so that mismatched element types are silently skipped
        if (newElm->isRotatable() && oldElm->isRotatable()) {
            newElm->setRotation(oldElm->rotation());
        }

        // Buzzer label is intentionally not preserved because the next/prev audio
        // shortcut relies on a fresh default label to display the audio name
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

        // --- Migrate existing wires to the new element's ports ---
        // The while loop drains the connection list; setEndPort/setStartPort calls
        // internally detach from oldElm and attach to newElm, so the list shrinks
        for (int port = 0; port < oldElm->inputSize(); ++port) {
            while (!oldElm->inputPort(port)->connections().isEmpty()) {
                if (auto *conn = oldElm->inputPort(port)->connections().constFirst(); conn && (conn->endPort() == oldElm->inputPort(port))) {
                    conn->setEndPort(newElm->inputPort(port));
                }
            }
        }

        for (int port = 0; port < oldElm->outputSize(); ++port) {
            while (!oldElm->outputPort(port)->connections().isEmpty()) {
                if (auto *conn = oldElm->outputPort(port)->connections().constFirst(); conn && (conn->startPort() == oldElm->outputPort(port))) {
                    conn->setStartPort(newElm->outputPort(port));
                }
            }
        }

        // Reuse the old element's ID on the new element so that any external
        // references (e.g. undo commands) remain valid after the morph
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

    // Compute the bounding box of all selected elements so redo() can mirror
    // each position about the selection's own axis rather than the scene origin
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
    // Flip is an involution: applying it twice returns to the original state,
    // so undo is identical to redo (position formula and +180° rotation cancel out)
    redo();
}

void FlipCommand::redo()
{
    qCDebug(zero) << text();
    for (auto *elm : CommandUtils::findElements(m_ids)) {
        auto pos = elm->pos();

        // axis == 0: mirror across the vertical axis (flip horizontally)
        // axis == 1: mirror across the horizontal axis (flip vertically)
        // The formula reflects the coordinate: newX = xmin + (xmax - oldX)
        (m_axis == 0) ? pos.setX(m_minPos.rx() + (m_maxPos.rx() - pos.rx()))
                      : pos.setY(m_minPos.ry() + (m_maxPos.ry() - pos.ry()));

        elm->setPos(pos);

        // Adding 180° to the element's own rotation flips the pixmap direction
        // to match the mirrored position (works because flip is an involution)
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
    const auto m_elements = CommandUtils::findElements(m_ids);

    // --- Snapshot current state before shrinking ---
    // Save element state and the state of any elements on the other end of
    // connections that will be severed, so undo() can fully restore them
    QList<GraphicElement *> serializationOrder;
    serializationOrder.reserve(m_elements.size());
    m_oldData.clear();

    QDataStream stream(&m_oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);

    for (auto *elm : m_elements) {
        elm->save(stream);
        serializationOrder.append(elm);

        // Save the upstream elements whose wires will be cut (ports >= newInputSize)
        for (int port = m_newInputSize; port < elm->inputSize(); ++port) {
            for (auto *conn : elm->inputPort(port)->connections()) {
                auto *outputPort = conn->startPort();
                outputPort->graphicElement()->save(stream);
                serializationOrder.append(outputPort->graphicElement());
            }
        }
    }

    for (auto *elm : m_elements) {
        // Count connections first, write count before connection data
        int connCount = 0;
        for (int port = m_newInputSize; port < elm->inputSize(); ++port) {
            connCount += elm->inputPort(port)->connections().size();
        }
        stream << connCount;

        for (int port = m_newInputSize; port < elm->inputSize(); ++port) {
            while (!elm->inputPort(port)->connections().isEmpty()) {
                auto *conn = elm->inputPort(port)->connections().constFirst();
                conn->save(stream);
                m_scene->removeItem(conn);
                auto *outputPort = conn->startPort();
                elm->inputPort(port)->disconnect(conn);
                outputPort->disconnect(conn);
            }
        }

        elm->setInputSize(m_newInputSize);
    }

    // Record the serialization order (by ID) so undo() can reload in the same sequence
    m_order.clear();

    for (auto *elm : serializationOrder) {
        m_order.append(elm->id());
    }

    m_scene->setCircuitUpdateRequired();
}

void ChangeInputSizeCommand::undo()
{
    qCDebug(zero) << text();
    const auto m_elements = CommandUtils::findElements(m_ids);
    const auto serializationOrder = CommandUtils::findElements(m_order);

    QDataStream stream(&m_oldData, QIODevice::ReadOnly);
    QVersionNumber version = Serialization::readPandaHeader(stream);

    QMap<quint64, QNEPort *> portMap;
    SerializationContext context{portMap, version, Serialization::contextDir};

    // Restore element and upstream element state first (expands port count back),
    // then reconstruct the connection objects that were severed during redo()
    for (auto *elm : serializationOrder) {
        elm->load(stream, context);
    }

    for (auto *elm : m_elements) {
        int connCount; stream >> connCount;
        for (int i = 0; i < connCount; ++i) {
            auto *conn = new QNEConnection();
            conn->load(stream, context);
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

    setText(tr("Change output size to %1").arg(newOutputSize));
}

void ChangeOutputSizeCommand::redo()
{
    qCDebug(zero) << text();
    const auto m_elements = CommandUtils::findElements(m_ids);

    QList<GraphicElement *> serializationOrder;
    serializationOrder.reserve(m_elements.size());
    m_oldData.clear();

    QDataStream stream(&m_oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);

    for (auto *elm : m_elements) {
        elm->save(stream);
        serializationOrder.append(elm);

        for (int port = m_newOutputSize; port < elm->outputSize(); ++port) {
            for (auto *conn : elm->outputPort(port)->connections()) {
                auto *inputPort = conn->endPort();
                inputPort->graphicElement()->save(stream);
                serializationOrder.append(inputPort->graphicElement());
            }
        }
    }

    for (auto *elm : m_elements) {
        // Count connections first, write count before connection data
        int connCount = 0;
        for (int port = m_newOutputSize; port < elm->outputSize(); ++port) {
            connCount += elm->outputPort(port)->connections().size();
        }
        stream << connCount;

        for (int port = m_newOutputSize; port < elm->outputSize(); ++port) {
            while (!elm->outputPort(port)->connections().isEmpty()) {
                auto *conn = elm->outputPort(port)->connections().constFirst();
                conn->save(stream);
                m_scene->removeItem(conn);
                auto *inputPort = conn->endPort();
                elm->outputPort(port)->disconnect(conn);
                inputPort->disconnect(conn);
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
    const auto elements = CommandUtils::findElements(m_ids);
    const auto serializationOrder = CommandUtils::findElements(m_order);

    QDataStream stream(&m_oldData, QIODevice::ReadOnly);
    QVersionNumber version = Serialization::readPandaHeader(stream);

    QMap<quint64, QNEPort *> portMap;
    SerializationContext context{portMap, version, Serialization::contextDir};

    for (auto *elm : serializationOrder) {
        elm->load(stream, context);
    }

    for (auto *elm : elements) {
        int connCount; stream >> connCount;
        for (int i = 0; i < connCount; ++i) {
            auto *conn = new QNEConnection();
            conn->load(stream, context);
            m_scene->addItem(conn);
        }

        elm->setSelected(true);
    }

    m_scene->setCircuitUpdateRequired();
}

ToggleTruthTableOutputCommand::ToggleTruthTableOutputCommand(GraphicElement *element, int pos, Scene *scene, ElementEditor *elementeditor, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_pos(pos)
{
    m_id = element->id();
    m_scene = scene;
    m_elementeditor = elementeditor;
    setText(tr("Toggle TruthTable Output at position: %1").arg(m_pos));
}

void ToggleTruthTableOutputCommand::redo()
{
    qCDebug(zero) << text();

    auto *truthtable = qobject_cast<TruthTable *>(CommandUtils::findElm(m_id));

    if (!truthtable) throw PANDACEPTION("Could not find truthtable element!");

    // toggleBit is its own inverse: applying it a second time undoes it, so both
    // redo() and undo() call the identical operation — no separate state capture needed
    truthtable->key().toggleBit(m_pos);

    m_scene->setCircuitUpdateRequired();
    if (m_elementeditor) {
        m_elementeditor->truthTable();
    }
}

void ToggleTruthTableOutputCommand::undo()
{
    qCDebug(zero) << text();

    auto *truthtable = qobject_cast<TruthTable *>(CommandUtils::findElm(m_id));

    if (!truthtable) throw PANDACEPTION("Could not find truthtable element!");

    truthtable->key().toggleBit(m_pos);

    m_scene->setCircuitUpdateRequired();
    if (m_elementeditor) {
        m_elementeditor->truthTable();
    }
}
