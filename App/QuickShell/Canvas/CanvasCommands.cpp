// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Canvas/CanvasCommands.h"

#include <algorithm>

#include <QDataStream>
#include <QIODevice>
#include <QTransform>

#include "App/Core/ItemWithId.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

CanvasElementsCommand::CanvasElementsCommand(const QList<GraphicElement *> &elements, CanvasItem *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_canvas(canvas)
{
    m_ids.reserve(elements.size());
    for (auto *elm : elements) {
        m_ids.append(elm->id());
    }
}

QList<GraphicElement *> CanvasElementsCommand::elements() const
{
    QList<GraphicElement *> result;
    result.reserve(m_ids.size());
    for (const int id : m_ids) {
        if (auto *elm = dynamic_cast<GraphicElement *>(m_canvas->itemById(id))) {
            result.append(elm);
        }
    }
    return result;
}

CanvasMoveCommand::CanvasMoveCommand(const QList<GraphicElement *> &elements, const QList<QPointF> &oldPositions,
                                     CanvasItem *canvas, QUndoCommand *parent)
    : CanvasElementsCommand(elements, canvas, parent)
    , m_oldPositions(oldPositions)
{
    m_newPositions.reserve(elements.size());
    for (auto *elm : elements) {
        m_newPositions.append(elm->pos());
    }
}

void CanvasMoveCommand::undo()
{
    const auto elems = elements();
    for (int i = 0; i < elems.size(); ++i) {
        elems.at(i)->setPos(m_oldPositions.at(i));
    }
}

void CanvasMoveCommand::redo()
{
    const auto elems = elements();
    for (int i = 0; i < elems.size(); ++i) {
        elems.at(i)->setPos(m_newPositions.at(i));
    }
}

CanvasRotateCommand::CanvasRotateCommand(const QList<GraphicElement *> &items, int angle, CanvasItem *canvas, QUndoCommand *parent)
    : CanvasElementsCommand(items, canvas, parent)
    , m_angle(angle)
{
    m_positions.reserve(items.size());
    for (auto *item : items) {
        m_positions.append(item->pos());
    }
}

void CanvasRotateCommand::undo()
{
    const auto elems = elements();
    for (int i = 0; i < elems.size(); ++i) {
        auto *elm = elems.at(i);
        elm->setRotation(elm->rotation() - m_angle);
        elm->setPos(m_positions.at(i));
        elm->update();
        elm->setSelected(true);
    }
}

void CanvasRotateCommand::redo()
{
    const auto elems = elements();
    double cx = 0;
    double cy = 0;
    int sz = 0;

    for (auto *elm : elems) {
        cx += elm->pos().x();
        cy += elm->pos().y();
        ++sz;
    }

    if (sz != 0) {
        cx /= sz;
        cy /= sz;
    }

    // Translate-rotate-translate-back maps each element position around the centroid.
    QTransform transform;
    transform.translate(cx, cy);
    transform.rotate(m_angle);
    transform.translate(-cx, -cy);

    for (auto *elm : elems) {
        elm->setPos(transform.map(elm->pos()));
        elm->setRotation(elm->rotation() + m_angle);
    }
}

CanvasFlipCommand::CanvasFlipCommand(const QList<GraphicElement *> &items, int axis, CanvasItem *canvas, QUndoCommand *parent)
    : CanvasElementsCommand(items, canvas, parent)
    , m_axis(axis)
{
    if (items.isEmpty()) {
        return;
    }

    m_positions.reserve(items.size());

    // Compute the bounding box of all selected elements so redo() can mirror each position
    // about the selection's own axis rather than the scene origin.
    double xmin = items.constFirst()->pos().x();
    double ymin = items.constFirst()->pos().y();
    double xmax = xmin;
    double ymax = ymin;

    for (auto *item : items) {
        m_positions.append(item->pos());
        xmin = (std::min)(xmin, item->pos().x());
        ymin = (std::min)(ymin, item->pos().y());
        xmax = (std::max)(xmax, item->pos().x());
        ymax = (std::max)(ymax, item->pos().y());
    }

    m_minPos = QPointF(xmin, ymin);
    m_maxPos = QPointF(xmax, ymax);
}

void CanvasFlipCommand::undo()
{
    // Flip is an involution: applying it twice returns to the original state.
    redo();
}

void CanvasFlipCommand::redo()
{
    for (auto *elm : elements()) {
        auto pos = elm->pos();

        // axis == 0: mirror across the vertical axis (flip horizontally)
        // axis == 1: mirror across the horizontal axis (flip vertically)
        (m_axis == 0) ? pos.setX(m_minPos.x() + (m_maxPos.x() - pos.x()))
                      : pos.setY(m_minPos.y() + (m_maxPos.y() - pos.y()));

        elm->setPos(pos);

        (m_axis == 0) ? elm->setFlippedX(!elm->isFlippedX())
                      : elm->setFlippedY(!elm->isFlippedY());
    }
}

namespace CanvasCommandUtils {

void storeIds(const QList<QGraphicsItem *> &items, QList<int> &ids)
{
    ids.reserve(ids.size() + items.size());
    for (auto *item : items) {
        if (auto *itemId = dynamic_cast<ItemWithId *>(item)) {
            ids.append(itemId->id());
        }
    }
}

void storeOtherIds(const QList<QGraphicsItem *> &connections, const QList<int> &ids, QList<int> &otherIds)
{
    // Track elements on the opposite end of connections that are NOT being operated on. Their
    // port state must be re-saved/re-loaded alongside the operation so undo restores it too.
    for (auto *item : connections) {
        if (auto *conn = qgraphicsitem_cast<Connection *>(item)) {
            if (auto *port1 = conn->startPort(); port1 && port1->graphicElement() && !ids.contains(port1->graphicElement()->id())) {
                otherIds.append(port1->graphicElement()->id());
            }
            if (auto *port2 = conn->endPort(); port2 && port2->graphicElement() && !ids.contains(port2->graphicElement()->id())) {
                otherIds.append(port2->graphicElement()->id());
            }
        }
    }
}

QList<QGraphicsItem *> loadList(const QList<QGraphicsItem *> &items, QList<int> &ids, QList<int> &otherIds)
{
    QList<QGraphicsItem *> elements;
    for (auto *item : items) {
        if (item->type() == GraphicElement::Type) {
            if (!elements.contains(item)) {
                elements.append(item);
            }
        }
    }

    // Always include the wires connected to the affected elements, even if the caller only
    // passed the element body -- keeps the topology consistent on undo/redo.
    QList<QGraphicsItem *> connections;
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

    for (auto *item : items) {
        if (item->type() == Connection::Type) {
            if (!connections.contains(item)) {
                connections.append(item);
            }
        }
    }

    storeIds(elements + connections, ids);
    storeOtherIds(connections, ids, otherIds);
    return elements + connections;
}

QList<QGraphicsItem *> findItems(CanvasItem *canvas, const QList<int> &ids)
{
    QList<QGraphicsItem *> items;
    items.reserve(ids.size());
    for (const int id : ids) {
        if (auto *item = dynamic_cast<QGraphicsItem *>(canvas->itemById(id))) {
            items.append(item);
        }
    }
    return items;
}

QList<GraphicElement *> findElements(CanvasItem *canvas, const QList<int> &ids)
{
    QList<GraphicElement *> items;
    items.reserve(ids.size());
    for (const int id : ids) {
        if (auto *item = dynamic_cast<GraphicElement *>(canvas->itemById(id))) {
            items.append(item);
        }
    }
    return items;
}

Connection *findConn(CanvasItem *canvas, int id)
{
    return dynamic_cast<Connection *>(canvas->itemById(id));
}

GraphicElement *findElm(CanvasItem *canvas, int id)
{
    return dynamic_cast<GraphicElement *>(canvas->itemById(id));
}

void saveItems(CanvasItem *canvas, QByteArray &itemData, const QList<QGraphicsItem *> &items, const QList<int> &otherIds)
{
    itemData.clear();
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);

    const auto others = findElements(canvas, otherIds);
    for (auto *elm : others) {
        elm->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    }

    Serialization::serialize(items, stream, {.purpose = SerializationPurpose::InMemorySnapshot});
}

void addItems(CanvasItem *canvas, const QList<QGraphicsItem *> &items)
{
    for (auto *item : items) {
        if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item)) {
            canvas->addItem(elm);
            elm->setSelected(true);
        } else if (auto *conn = qgraphicsitem_cast<Connection *>(item)) {
            canvas->addItem(conn);
        }
    }
}

QList<QGraphicsItem *> loadItems(CanvasItem *canvas, QByteArray &itemData, const QList<int> &ids, QList<int> &otherIds)
{
    if (itemData.isEmpty()) {
        return {};
    }

    QDataStream stream(&itemData, QIODevice::ReadOnly);
    QVersionNumber version = Serialization::readPandaHeader(stream);

    QHash<quint64, Port *> portMap;
    auto context = canvas->deserializationContext(portMap, version, SerializationPurpose::InMemorySnapshot);

    for (auto *elm : findElements(canvas, otherIds)) {
        elm->load(stream, context);
    }

    // Elements are serialized before connections; deserialize() reconstructs both in that order.
    const auto items = Serialization::deserialize(stream, context);

    // Re-assign the original ids so undo/redo chains that store ids elsewhere remain valid.
    for (int i = 0; i < items.size() && i < ids.size(); ++i) {
        if (auto *itemId = dynamic_cast<ItemWithId *>(items.at(i))) {
            canvas->updateItemId(itemId, ids.at(i));
        }
    }

    addItems(canvas, items);
    return items;
}

void deleteItems(CanvasItem *canvas, const QList<QGraphicsItem *> &items)
{
    // Delete in reverse order, same as Commands.cpp's CommandUtils::deleteItems().
    for (auto it = items.rbegin(); it != items.rend(); ++it) {
        if (auto *elm = qgraphicsitem_cast<GraphicElement *>(*it)) {
            canvas->removeItem(elm);
        } else if (auto *conn = qgraphicsitem_cast<Connection *>(*it)) {
            canvas->removeItem(conn);
        }
        delete *it;
    }
}

void drainPortConnections(GraphicElement *elm, int fromPort, int toPort,
                          bool isInput, QDataStream &stream, CanvasItem *canvas)
{
    int connCount = 0;
    for (int port = fromPort; port < toPort; ++port) {
        Port *p = isInput ? static_cast<Port *>(elm->inputPort(port)) : static_cast<Port *>(elm->outputPort(port));
        connCount += static_cast<int>(p->connections().size());
    }
    stream << connCount;

    for (int port = fromPort; port < toPort; ++port) {
        Port *p = isInput ? static_cast<Port *>(elm->inputPort(port)) : static_cast<Port *>(elm->outputPort(port));
        while (!p->connections().isEmpty()) {
            auto *conn = p->connections().constFirst();
            stream << conn->id();
            conn->save(stream);
            conn->setStartPort(nullptr);
            conn->setEndPort(nullptr);
            canvas->removeItem(conn);
            delete conn;
        }
    }
}

}  // namespace CanvasCommandUtils

CanvasAddItemsCommand::CanvasAddItemsCommand(const QList<QGraphicsItem *> &items, CanvasItem *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_canvas(canvas)
{
    // Simulation must be paused while items are added to avoid a partial-topology update.
    SimulationBlocker blocker(m_canvas->simulation());
    // Add items to the canvas first so they receive real ids before loadList captures them.
    CanvasCommandUtils::addItems(m_canvas, items);
    // Collect the canonical list (elements + attached wires) and store their ids. The second
    // addItems() call handles any wires discovered via port traversal.
    const auto items_ = CanvasCommandUtils::loadList(items, m_ids, m_otherIds);
    CanvasCommandUtils::addItems(m_canvas, items_);
}

void CanvasAddItemsCommand::undo()
{
    SimulationBlocker blocker(m_canvas->simulation());
    const auto items = CanvasCommandUtils::findItems(m_canvas, m_ids);
    CanvasCommandUtils::saveItems(m_canvas, m_itemData, items, m_otherIds);
    CanvasCommandUtils::deleteItems(m_canvas, items);
    m_canvas->restartSimulation();
}

void CanvasAddItemsCommand::redo()
{
    SimulationBlocker blocker(m_canvas->simulation());
    CanvasCommandUtils::loadItems(m_canvas, m_itemData, m_ids, m_otherIds);
    m_canvas->restartSimulation();
}

CanvasDeleteItemsCommand::CanvasDeleteItemsCommand(const QList<QGraphicsItem *> &items, CanvasItem *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_canvas(canvas)
{
    CanvasCommandUtils::loadList(items, m_ids, m_otherIds);
}

void CanvasDeleteItemsCommand::undo()
{
    SimulationBlocker blocker(m_canvas->simulation());
    CanvasCommandUtils::loadItems(m_canvas, m_itemData, m_ids, m_otherIds);
    m_canvas->restartSimulation();
}

void CanvasDeleteItemsCommand::redo()
{
    SimulationBlocker blocker(m_canvas->simulation());
    const auto items = CanvasCommandUtils::findItems(m_canvas, m_ids);
    CanvasCommandUtils::saveItems(m_canvas, m_itemData, items, m_otherIds);
    CanvasCommandUtils::deleteItems(m_canvas, items);
    m_canvas->restartSimulation();
}

CanvasUpdateCommand::CanvasUpdateCommand(const QList<GraphicElement *> &elements, const QByteArray &oldData,
                                         CanvasItem *canvas, QUndoCommand *parent)
    : CanvasElementsCommand(elements, canvas, parent)
    , m_oldData(oldData)
{
    QDataStream stream(&m_newData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);

    for (auto *elm : elements) {
        elm->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    }

    // Precompute once whether old->new changes wireless routing -- see Commands.h's
    // UpdateCommand for the full rationale (this cannot be detected inside redo()/undo()
    // around loadData(): the first redo()'s load is a no-op, since the caller already
    // mutated the elements before pushing).
    const auto newState = snapshotWirelessState();
    if (!newState.isEmpty()) {
        SimulationBlocker blocker(m_canvas->simulation());
        loadData(m_oldData);
        m_wirelessTopologyChange = wirelessStateDiffers(snapshotWirelessState(), newState);
        loadData(m_newData);
    }
}

void CanvasUpdateCommand::undo()
{
    SimulationBlocker blocker(m_canvas->simulation());
    loadData(m_oldData);
    refreshRuntimeState();
}

void CanvasUpdateCommand::redo()
{
    SimulationBlocker blocker(m_canvas->simulation());
    loadData(m_newData);
    refreshRuntimeState();
}

QVector<CanvasUpdateCommand::WirelessState> CanvasUpdateCommand::snapshotWirelessState() const
{
    QVector<WirelessState> state;
    for (auto *elm : elements()) {
        if (elm && elm->hasWirelessMode()) {
            state.append({elm, elm->wirelessMode(), elm->label()});
        }
    }
    return state;
}

bool CanvasUpdateCommand::wirelessStateDiffers(const QVector<WirelessState> &before, const QVector<WirelessState> &after)
{
    if (before.size() != after.size()) {
        return true; // defensive: the set of wireless-capable elements itself changed
    }
    for (int i = 0; i < before.size(); ++i) {
        const auto &b = before.at(i);
        const auto &a = after.at(i);
        if (b.element != a.element || b.mode != a.mode) {
            return true;
        }
        if (b.label != a.label && a.mode != WirelessMode::None) {
            return true;
        }
    }
    return false;
}

void CanvasUpdateCommand::refreshRuntimeState()
{
    // A wireless mode/label edit IS a topology change (the label is the Tx->Rx channel), and
    // the running simulation resolves channels only during initialize() -- without a full
    // rebuild the old routing silently stays live.
    if (m_wirelessTopologyChange) {
        m_canvas->restartSimulation();
        return;
    }

    // Everything else this command carries is property-only (label, color, ...), never
    // topology -- a full restartSimulation() would be correct but unnecessarily rebuilds the
    // simulation graph (every Clock's phase, etc.) for an edit that didn't change it. This
    // canvas has no cheaper "visual refresh only" path yet (Scene's setPropertyUpdateRequired()
    // equivalent), so a plain repaint is the closest available -- revisit if this proves
    // visibly wrong once a chrome property panel actually drives this command (Phase 4).
    m_canvas->update();
}

void CanvasUpdateCommand::loadData(QByteArray &itemData)
{
    const auto elems = elements();
    if (elems.isEmpty()) {
        return;
    }

    QDataStream stream(&itemData, QIODevice::ReadOnly);
    QVersionNumber version = Serialization::readPandaHeader(stream);

    QHash<quint64, Port *> portMap;
    auto context = m_canvas->deserializationContext(portMap, version, SerializationPurpose::InMemorySnapshot);

    for (auto *elm : elems) {
        elm->load(stream, context);
        elm->setSelected(true);
    }
}

CanvasChangePortSizeCommand::CanvasChangePortSizeCommand(const QList<GraphicElement *> &elements, int newPortSize,
                                                         CanvasItem *canvas, bool isInput, QUndoCommand *parent)
    : CanvasElementsCommand(elements, canvas, parent)
    , m_newPortSize(newPortSize)
    , m_isInput(isInput)
{
}

void CanvasChangePortSizeCommand::redo()
{
    SimulationBlocker blocker(m_canvas->simulation());

    const auto elems = elements();

    QList<GraphicElement *> serializationOrder;
    serializationOrder.reserve(elems.size());
    m_oldData.clear();

    QDataStream stream(&m_oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);

    for (auto *elm : elems) {
        elm->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
        serializationOrder.append(elm);

        const int oldSize = m_isInput ? elm->inputSize() : elm->outputSize();
        for (int port = m_newPortSize; port < oldSize; ++port) {
            Port *nport = m_isInput ? static_cast<Port *>(elm->inputPort(port)) : elm->outputPort(port);
            for (auto *conn : nport->connections()) {
                Port *otherPort = m_isInput ? static_cast<Port *>(conn->startPort()) : conn->endPort();
                otherPort->graphicElement()->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
                serializationOrder.append(otherPort->graphicElement());
            }
        }
    }

    for (auto *elm : elems) {
        CanvasCommandUtils::drainPortConnections(elm, m_newPortSize, m_isInput ? elm->inputSize() : elm->outputSize(),
                                                 m_isInput, stream, m_canvas);
        if (m_isInput) {
            elm->setInputSize(m_newPortSize);
        } else {
            elm->setOutputSize(m_newPortSize);
            elm->setSelected(true);
        }
    }

    m_order.clear();
    for (auto *elm : serializationOrder) {
        m_order.append(elm->id());
    }

    m_canvas->restartSimulation();
}

void CanvasChangePortSizeCommand::undo()
{
    SimulationBlocker blocker(m_canvas->simulation());

    const auto elems = elements();
    const auto serializationOrder = CanvasCommandUtils::findElements(m_canvas, m_order);

    QDataStream stream(&m_oldData, QIODevice::ReadOnly);
    QVersionNumber version = Serialization::readPandaHeader(stream);

    QHash<quint64, Port *> portMap;
    auto context = m_canvas->deserializationContext(portMap, version, SerializationPurpose::InMemorySnapshot);

    for (auto *elm : serializationOrder) {
        elm->load(stream, context);
    }

    for (auto *elm : elems) {
        int connCount;
        stream >> connCount;
        for (int i = 0; i < connCount; ++i) {
            int connId;
            stream >> connId;
            auto *conn = new Connection();
            conn->load(stream, context);
            m_canvas->updateItemId(conn, connId);
            m_canvas->addItem(conn);
        }
        elm->setSelected(true);
    }

    m_canvas->restartSimulation();
}
