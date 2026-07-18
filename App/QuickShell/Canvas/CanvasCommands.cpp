// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Canvas/CanvasCommands.h"

#include <algorithm>
#include <cmath>

#include <QDataStream>
#include <QIODevice>
#include <QTransform>

#include "App/Core/Constants.h"
#include "App/Core/ItemWithId.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/QuickShell/Canvas/CanvasICRegistry.h"
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

void serializeItems(const QList<QGraphicsItem *> &items, QDataStream &stream)
{
    // Compute the centroid of all selected elements (not connections) so paste can place the
    // clipboard contents relative to the cursor position.
    QPointF center(0.0, 0.0);
    int itemsQuantity = 0;

    for (auto *item : items) {
        if (item->type() == GraphicElement::Type) {
            center += item->pos();
            ++itemsQuantity;
        }
    }

    stream << center / static_cast<qreal>(itemsQuantity);
    Serialization::serialize(items, stream, {.purpose = SerializationPurpose::InMemorySnapshot});
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

// --- CanvasRegisterBlobCommand ---

CanvasRegisterBlobCommand::CanvasRegisterBlobCommand(const QString &blobName, const QByteArray &data, CanvasItem *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_blobName(blobName)
    , m_data(data)
    , m_canvas(canvas)
{
}

void CanvasRegisterBlobCommand::redo()
{
    m_canvas->icRegistry()->registerBlob(m_blobName, m_data);
}

void CanvasRegisterBlobCommand::undo()
{
    m_canvas->icRegistry()->removeBlob(m_blobName);
}

// --- CanvasRemoveBlobCommand ---

CanvasRemoveBlobCommand::CanvasRemoveBlobCommand(const QString &blobName, CanvasItem *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_blobName(blobName)
    , m_data(canvas->icRegistry()->blob(blobName))
    , m_canvas(canvas)
{
}

void CanvasRemoveBlobCommand::redo()
{
    m_canvas->icRegistry()->removeBlob(m_blobName);
}

void CanvasRemoveBlobCommand::undo()
{
    m_canvas->icRegistry()->setBlob(m_blobName, m_data);
}

// --- CanvasRenameBlobCommand ---

CanvasRenameBlobCommand::CanvasRenameBlobCommand(const QString &oldName, const QString &newName, CanvasItem *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_oldName(oldName)
    , m_newName(newName)
    , m_canvas(canvas)
{
}

void CanvasRenameBlobCommand::redo()
{
    m_canvas->icRegistry()->renameBlob(m_oldName, m_newName);
}

void CanvasRenameBlobCommand::undo()
{
    m_canvas->icRegistry()->renameBlob(m_newName, m_oldName);
}

// --- CanvasUpdateBlobCommand ---

QList<CanvasUpdateBlobCommand::ConnectionInfo> CanvasUpdateBlobCommand::captureConnections(const QList<GraphicElement *> &targets)
{
    QList<ConnectionInfo> connections;
    for (auto *target : targets) {
        for (int i = 0; i < target->inputSize(); ++i) {
            for (auto *conn : target->inputPort(i)->connections()) {
                auto *otherPort = conn->startPort();
                if (otherPort && otherPort->graphicElement()) {
                    connections.append({conn->id(), target->id(), i, true,
                                        otherPort->graphicElement()->id(), otherPort->index()});
                }
            }
        }
        for (int i = 0; i < target->outputSize(); ++i) {
            for (auto *conn : target->outputPort(i)->connections()) {
                auto *otherPort = conn->endPort();
                if (otherPort && otherPort->graphicElement()) {
                    connections.append({conn->id(), target->id(), i, false,
                                        otherPort->graphicElement()->id(), otherPort->index()});
                }
            }
        }
    }
    return connections;
}

CanvasUpdateBlobCommand::CanvasUpdateBlobCommand(const QList<GraphicElement *> &elements, const QByteArray &oldData,
                                                 const QList<ConnectionInfo> &connections, CanvasItem *canvas, QUndoCommand *parent)
    : CanvasElementsCommand(elements, canvas, parent)
    , m_oldData(oldData)
    , m_connections(connections)
{
    QDataStream stream(&m_newData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);

    for (auto *elm : elements) {
        elm->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    }

    if (!elements.isEmpty()) {
        m_blobName = elements.first()->blobName();
        m_newBlob = m_canvas->icRegistry()->blob(m_blobName);
    }
}

void CanvasUpdateBlobCommand::redo()
{
    SimulationBlocker blocker(m_canvas->simulation());
    auto *reg = m_canvas->icRegistry();

    if (!m_blobName.isEmpty()) {
        if (m_newBlob.isEmpty()) {
            reg->removeBlob(m_blobName);
        } else {
            reg->setBlob(m_blobName, m_newBlob);
        }
    }
    loadData(m_newData);
    reconnectConnections();
    m_canvas->restartSimulation();
}

void CanvasUpdateBlobCommand::undo()
{
    SimulationBlocker blocker(m_canvas->simulation());
    auto *reg = m_canvas->icRegistry();

    if (!m_blobName.isEmpty()) {
        if (m_oldBlob.isEmpty()) {
            reg->removeBlob(m_blobName);
        } else {
            reg->setBlob(m_blobName, m_oldBlob);
        }
    }
    loadData(m_oldData);
    reconnectConnections();
    m_canvas->restartSimulation();
}

void CanvasUpdateBlobCommand::loadData(QByteArray &itemData)
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

void CanvasUpdateBlobCommand::reconnectConnections()
{
    for (const auto &ci : std::as_const(m_connections)) {
        auto *elm = dynamic_cast<GraphicElement *>(m_canvas->itemById(ci.elementId));
        auto *otherElm = dynamic_cast<GraphicElement *>(m_canvas->itemById(ci.otherElementId));
        if (!elm || !otherElm) {
            continue;
        }

        InputPort *inPort = nullptr;
        OutputPort *outPort = nullptr;

        if (ci.isInput) {
            inPort = (ci.portIndex >= 0 && ci.portIndex < elm->inputSize()) ? elm->inputPort(ci.portIndex) : nullptr;
            outPort = (ci.otherPortIndex >= 0 && ci.otherPortIndex < otherElm->outputSize()) ? otherElm->outputPort(ci.otherPortIndex) : nullptr;
        } else {
            outPort = (ci.portIndex >= 0 && ci.portIndex < elm->outputSize()) ? elm->outputPort(ci.portIndex) : nullptr;
            inPort = (ci.otherPortIndex >= 0 && ci.otherPortIndex < otherElm->inputSize()) ? otherElm->inputPort(ci.otherPortIndex) : nullptr;
        }

        if (!inPort || !outPort) {
            // Port shrunk: the Connection that occupied this slot was cascade-deleted when
            // the IC's port was destroyed. Undo restores the port count and recreates the
            // connection with the original id below.
            continue;
        }

        bool alreadyConnected = false;
        for (auto *conn : inPort->connections()) {
            if (conn->startPort() == outPort) {
                alreadyConnected = true;
                break;
            }
        }
        if (alreadyConnected) {
            continue;
        }

        auto *conn = new Connection();
        conn->setStartPort(outPort);
        conn->setEndPort(inPort);
        m_canvas->updateItemId(conn, ci.connectionId);
        m_canvas->addItem(conn);
    }
}

// --- CanvasSplitCommand ---

CanvasSplitCommand::CanvasSplitCommand(Connection *conn, QPointF mousePos, CanvasItem *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_canvas(canvas)
{
    auto *node = ElementFactory::buildElement(ElementType::Node);

    // Align node to grid: subtract pixmapCenter so the node's visual center lands on the
    // mouse click, then snap to the nearest grid intersection.
    m_nodePos = mousePos - node->pixmapCenter();
    const int gridSize = Constants::gridSize;
    const qreal xV = qRound(m_nodePos.x() / gridSize) * gridSize;
    const qreal yV = qRound(m_nodePos.y() / gridSize) * gridSize;
    m_nodePos = QPointF(xV, yV);

    // Quantize the wire's angle to the nearest cardinal direction so the node's arrow graphic
    // points along the wire.
    const int angle = static_cast<int>(conn->angle());
    m_nodeAngle = static_cast<int>(360 - 90 * (std::round(angle / 90.0)));

    auto *startPort = conn->startPort();
    auto *endPort = conn->endPort();
    auto *startElement = startPort ? startPort->graphicElement() : nullptr;
    auto *endElement = endPort ? endPort->graphicElement() : nullptr;
    Q_ASSERT(startElement && endElement); // caller (mouseDoubleClickEvent) already checked both ports exist

    m_elm1Id = startElement->id();
    m_elm2Id = endElement->id();
    m_c1Id = conn->id();

    // Reserve a stable id for the second wire segment by briefly registering it, then
    // removing it -- redo() recreates it under this same id via updateItemId().
    auto *conn2 = new Connection();
    m_canvas->addItem(conn2);
    m_c2Id = conn2->id();
    m_canvas->removeItem(conn2);
    delete conn2;

    m_canvas->addItem(node);
    m_nodeId = node->id();
    m_canvas->removeItem(node);
    delete node;
}

void CanvasSplitCommand::redo()
{
    SimulationBlocker blocker(m_canvas->simulation());
    auto *conn1 = CanvasCommandUtils::findConn(m_canvas, m_c1Id);
    auto *elm1 = CanvasCommandUtils::findElm(m_canvas, m_elm1Id);
    auto *elm2 = CanvasCommandUtils::findElm(m_canvas, m_elm2Id);
    if (!conn1 || !elm1 || !elm2) {
        return;
    }

    auto *conn2 = CanvasCommandUtils::findConn(m_canvas, m_c2Id);
    auto *node = CanvasCommandUtils::findElm(m_canvas, m_nodeId);

    if (!conn2) {
        conn2 = new Connection();
        m_canvas->updateItemId(conn2, m_c2Id);
    }
    if (!node) {
        node = ElementFactory::buildElement(ElementType::Node);
        m_canvas->updateItemId(node, m_nodeId);
    }

    node->setPos(m_nodePos);
    node->setRotation(m_nodeAngle);

    auto *endPort = conn1->endPort();
    if (!endPort) {
        return;
    }

    // Wire topology after split: elm1 -> conn1 -> node -> conn2 -> elm2.
    conn2->setStartPort(node->outputPort());
    conn2->setEndPort(endPort);
    conn1->setEndPort(node->inputPort());

    m_canvas->addItem(node);
    m_canvas->addItem(conn2);

    conn1->updatePosFromPorts();
    conn2->updatePosFromPorts();

    m_canvas->restartSimulation();
}

void CanvasSplitCommand::undo()
{
    SimulationBlocker blocker(m_canvas->simulation());
    auto *conn1 = CanvasCommandUtils::findConn(m_canvas, m_c1Id);
    auto *conn2 = CanvasCommandUtils::findConn(m_canvas, m_c2Id);
    auto *node = CanvasCommandUtils::findElm(m_canvas, m_nodeId);
    if (!conn1 || !conn2 || !node) {
        return;
    }

    // Restore the original direct wire: conn1 skips the node and connects straight to elm2.
    conn1->setEndPort(conn2->endPort());
    conn1->updatePosFromPorts();

    m_canvas->removeItem(conn2);
    m_canvas->removeItem(node);
    delete conn2;
    delete node;

    m_canvas->restartSimulation();
}

// --- CanvasMorphCommand ---

CanvasMorphCommand::CanvasMorphCommand(const QList<GraphicElement *> &elements, ElementType type, CanvasItem *canvas, QUndoCommand *parent)
    : CanvasElementsCommand(elements, canvas, parent)
    , m_newType(type)
{
    m_types.reserve(elements.size());
    for (auto *oldElm : elements) {
        m_types.append(oldElm->elementType());
    }
}

void CanvasMorphCommand::restoreDeletedConnections(const QList<DeletedConnectionInfo> &deleted)
{
    for (const auto &info : deleted) {
        auto *morphedElm = dynamic_cast<GraphicElement *>(m_canvas->itemById(info.morphedElementId));
        auto *otherElm = dynamic_cast<GraphicElement *>(m_canvas->itemById(info.otherElementId));
        if (!morphedElm || !otherElm) {
            continue;
        }

        auto *conn = new Connection();
        if (info.isInput) {
            conn->setStartPort(otherElm->outputPort(info.otherPortIndex));
            conn->setEndPort(morphedElm->inputPort(info.portIndex));
        } else {
            conn->setStartPort(morphedElm->outputPort(info.portIndex));
            conn->setEndPort(otherElm->inputPort(info.otherPortIndex));
        }
        m_canvas->updateItemId(conn, info.connectionId);
        m_canvas->addItem(conn);
    }
}

void CanvasMorphCommand::undo()
{
    SimulationBlocker blocker(m_canvas->simulation());

    auto newElms = elements();
    decltype(newElms) oldElms;
    oldElms.reserve(m_ids.size());
    for (int i = 0; i < m_ids.size(); ++i) {
        oldElms << ElementFactory::buildElement(m_types.at(i));
    }

    m_deletedConnectionsOnUndo.clear();
    transferConnections(newElms, oldElms, &m_deletedConnectionsOnUndo);
    restoreDeletedConnections(m_deletedConnections);

    m_canvas->restartSimulation();
}

void CanvasMorphCommand::redo()
{
    SimulationBlocker blocker(m_canvas->simulation());

    auto oldElms = elements();
    decltype(oldElms) newElms;
    newElms.reserve(m_ids.size());
    for (int i = 0; i < m_ids.size(); ++i) {
        newElms << ElementFactory::buildElement(m_newType);
    }

    m_deletedConnections.clear();
    transferConnections(oldElms, newElms, &m_deletedConnections);
    restoreDeletedConnections(m_deletedConnectionsOnUndo);

    m_canvas->restartSimulation();
}

void CanvasMorphCommand::transferConnections(const QList<GraphicElement *> &from, const QList<GraphicElement *> &to,
                                             QList<DeletedConnectionInfo> *deleted)
{
    for (int elm = 0; elm < from.size(); ++elm) {
        auto *oldElm = from.at(elm);
        auto *newElm = to.at(elm);

        newElm->setInputSize(oldElm->inputSize());
        newElm->setPos(oldElm->pos());

        // Not<->Node morphs need a 16px position adjustment: the two element types have
        // different pixmap sizes and their visual centers differ by that amount.
        if ((oldElm->elementType() == ElementType::Not) && (newElm->elementType() == ElementType::Node)) {
            newElm->moveBy(16, 16);
        }
        if ((oldElm->elementType() == ElementType::Node) && (newElm->elementType() == ElementType::Not)) {
            newElm->moveBy(-16, -16);
        }

        if (newElm->rotatesGraphic() && oldElm->rotatesGraphic()) {
            newElm->setRotation(oldElm->rotation());
        }
        if (newElm->hasLabel() && oldElm->hasLabel()) {
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

        // Mirror state is a base property of every element, always carried over.
        newElm->setFlippedX(oldElm->isFlippedX());
        newElm->setFlippedY(oldElm->isFlippedY());

        if (newElm->hasAudio() && oldElm->hasAudio()) {
            newElm->setAudio(oldElm->audio());
        }
        if (newElm->hasVolume() && oldElm->hasVolume()) {
            newElm->setVolume(oldElm->volume());
        }
        if (newElm->hasDelay() && oldElm->hasDelay()) {
            newElm->setDelay(oldElm->delay());
        }

        // Migrate existing wires to the new element's ports; connections on ports the new
        // element doesn't have are deleted (recorded in *deleted for undo/redo restoration).
        transferPortConnections(oldElm, newElm, true, deleted);
        transferPortConnections(oldElm, newElm, false, deleted);

        // Reuse the old element's id on the new element so external references stay valid.
        const int oldId = oldElm->id();
        m_canvas->removeItem(oldElm);
        delete oldElm;

        m_canvas->updateItemId(newElm, oldId);
        m_canvas->addItem(newElm);
        newElm->updatePortsProperties();
    }
}

void CanvasMorphCommand::transferPortConnections(GraphicElement *oldElm, GraphicElement *newElm,
                                                 bool isInput, QList<DeletedConnectionInfo> *deleted)
{
    const int portCount = isInput ? oldElm->inputSize() : oldElm->outputSize();
    for (int port = 0; port < portCount; ++port) {
        Port *oldPort = isInput ? static_cast<Port *>(oldElm->inputPort(port))
                                   : static_cast<Port *>(oldElm->outputPort(port));
        while (!oldPort->connections().isEmpty()) {
            auto *conn = oldPort->connections().constFirst();
            if (!conn) {
                break;
            }
            const bool ownsSide = isInput ? (conn->endPort() == oldElm->inputPort(port))
                                          : (conn->startPort() == oldElm->outputPort(port));
            if (!ownsSide) {
                break;
            }
            Port *newPort = isInput ? static_cast<Port *>(newElm->inputPort(port))
                                       : static_cast<Port *>(newElm->outputPort(port));
            if (newPort) {
                if (isInput) {
                    conn->setEndPort(static_cast<InputPort *>(newPort));
                } else {
                    conn->setStartPort(static_cast<OutputPort *>(newPort));
                }
                conn->setHighLight(false);
            } else {
                Port *otherPort = isInput ? static_cast<Port *>(conn->startPort())
                                             : static_cast<Port *>(conn->endPort());
                if (deleted && otherPort && otherPort->graphicElement()) {
                    deleted->append({conn->id(), oldElm->id(), port, isInput, otherPort->graphicElement()->id(), otherPort->index()});
                }
                conn->setStartPort(nullptr);
                conn->setEndPort(nullptr);
                m_canvas->removeItem(conn);
                delete conn;
            }
        }
    }
}

// --- CanvasToggleTruthTableOutputCommand ---

CanvasToggleTruthTableOutputCommand::CanvasToggleTruthTableOutputCommand(GraphicElement *element, int pos, CanvasItem *canvas, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_canvas(canvas)
    , m_id(element->id())
    , m_pos(pos)
{
}

void CanvasToggleTruthTableOutputCommand::redo()
{
    auto *truthtable = qobject_cast<TruthTable *>(CanvasCommandUtils::findElm(m_canvas, m_id));
    if (!truthtable) {
        return;
    }
    // The key holds exactly 2048 bits (256 rows x 8 outputs); toggleBit on any other position
    // is an out-of-bounds write.
    if (m_pos < 0 || m_pos >= truthtable->key().size()) {
        return;
    }
    truthtable->key().toggleBit(m_pos);
    m_canvas->restartSimulation();
}

void CanvasToggleTruthTableOutputCommand::undo()
{
    // toggleBit is self-inverse: undo == redo.
    redo();
}
