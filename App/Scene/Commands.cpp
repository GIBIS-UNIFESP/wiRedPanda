// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/Commands.h"

#include <algorithm>
#include <cmath>

#include <QCoreApplication>
#include <QIODevice>

#include "App/Core/Common.h"
#include "App/Core/Constants.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

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
        if (item->type() == Connection::Type) {
            if (!connections.contains(item)) {
                connections.append(item);
            }
        }
    }

    // Store the ids of all elements listed in items.
    storeIds(elements + connections, ids);
    // Store all elements linked to each connection that will not be deleted.
    storeOtherIds(connections, ids, otherIds);
    return elements + connections;
}

const QList<QGraphicsItem *> findItems(Scene *scene, const QList<int> &ids)
{
    QList<QGraphicsItem *> items;
    items.reserve(ids.size());

    for (const int id : ids) {
        if (auto *item = dynamic_cast<QGraphicsItem *>(scene->itemById(id))) {
            items.append(item);
        }
    }

    if (items.size() != ids.size()) {
        throw PANDACEPTION_WITH_CONTEXT("commands", "One or more items was not found on the scene.");
    }

    return items;
}

const QList<GraphicElement *> findElements(Scene *scene, const QList<int> &ids)
{
    QList<GraphicElement *> items;
    items.reserve(ids.size());

    for (const int id : ids) {
        if (auto *item = dynamic_cast<GraphicElement *>(scene->itemById(id))) {
            items.append(item);
        }
    }

    if (items.size() != ids.size()) {
        throw PANDACEPTION_WITH_CONTEXT("commands", "One or more elements was not found on the scene.");
    }

    return items;
}

Connection *findConn(Scene *scene, const int id)
{
    return dynamic_cast<Connection *>(scene->itemById(id));
}

GraphicElement *findElm(Scene *scene, const int id)
{
    return dynamic_cast<GraphicElement *>(scene->itemById(id));
}

void saveItems(Scene *scene, QByteArray &itemData, const QList<QGraphicsItem *> &items, const QList<int> &otherIds)
{
    itemData.clear();
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);

    const auto others = CommandUtils::findElements(scene, otherIds);

    for (auto *elm : others) {
        elm->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    }

    Serialization::serialize(items, stream, {.purpose = SerializationPurpose::InMemorySnapshot});
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

    QHash<quint64, Port *> portMap;
    auto context = scene->deserializationContext(portMap, version, SerializationPurpose::InMemorySnapshot);

    for (auto *elm : CommandUtils::findElements(scene, otherIds)) {
        elm->load(stream, context);
    }

    /* Assuming that all connections are stored after the elements, we will deserialize the elements first.
     * We will store one additional information: The element IDs! */
    const auto items = Serialization::deserialize(stream, context);

    if (items.size() != ids.size()) {
        throw PANDACEPTION_WITH_CONTEXT("commands", "One or more elements were not found on scene. Expected %1, found %2.", static_cast<int>(ids.size()), static_cast<int>(items.size()));
    }

    // Re-assign the original IDs so undo/redo chains that store IDs remain valid
    for (int i = 0; i < items.size(); ++i) {
        if (auto *itemId = dynamic_cast<ItemWithId *>(items.at(i))) {
            scene->updateItemId(itemId, ids.at(i));
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

void drainPortConnections(GraphicElement *elm, int fromPort, int toPort,
                          bool isInput, QDataStream &stream, Scene *scene)
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
            scene->removeItem(conn);
            delete conn;
        }
    }
}

}  // namespace CommandUtils

ElementsCommand::ElementsCommand(const QList<GraphicElement *> &elements, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
{
    m_ids.reserve(elements.size());
    for (auto *elm : elements) {
        m_ids.append(elm->id());
    }
}

QList<GraphicElement *> ElementsCommand::elements() const
{
    return CommandUtils::findElements(m_scene, m_ids);
}

AddItemsCommand::AddItemsCommand(const QList<QGraphicsItem *> &items, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
{
    // Simulation must be paused while items are added to avoid a partial-topology update
    SimulationBlocker blocker(m_scene->simulation());
    // Add items to scene first so they receive positive scene-local IDs before
    // loadList captures them in storeIds. Items already in scene are skipped.
    CommandUtils::addItems(m_scene, items);
    // Collect canonical list (elements + attached wires) and store their IDs.
    // The second addItems call handles any wires discovered via port traversal.
    const auto items_ = CommandUtils::loadList(items, m_ids, m_otherIds);
    CommandUtils::addItems(m_scene, items_);
    setText(tr("Add %1 elements").arg(items_.size()));
}

void AddItemsCommand::undo()
{
    qCDebug(zero) << text();
    SimulationBlocker blocker(m_scene->simulation());
    const auto items = CommandUtils::findItems(m_scene, m_ids);
    CommandUtils::saveItems(m_scene, m_itemData, items, m_otherIds);
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
    const auto items = CommandUtils::findItems(m_scene, m_ids);
    CommandUtils::saveItems(m_scene, m_itemData, items, m_otherIds);
    CommandUtils::deleteItems(m_scene, items);
    m_scene->setCircuitUpdateRequired();
}

RotateCommand::RotateCommand(const QList<GraphicElement *> &items, const int angle, Scene *scene, QUndoCommand *parent)
    : ElementsCommand(items, scene, parent)
    , m_angle(angle)
{
    setText(tr("Rotate %1 degrees").arg(m_angle));
    m_positions.reserve(items.size());

    for (auto *item : items) {
        m_positions.append(item->pos());
        item->setPos(item->pos());

    }
}

void RotateCommand::undo()
{
    const auto elements = this->elements();

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
    const auto elements = this->elements();
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
    : ElementsCommand(list, scene, parent)
    , m_oldPositions(oldPositions)
{
    m_newPositions.reserve(list.size());

    for (auto *elm : list) {
        m_newPositions.append(elm->pos());
    }

    setText(tr("Move elements"));
}

void MoveCommand::undo()
{
    const auto elements = this->elements();

    for (int i = 0; i < elements.size(); ++i) {
        elements.at(i)->setPos(m_oldPositions.at(i));
    }

    m_scene->setAutosaveRequired();
}

void MoveCommand::redo()
{
    const auto elements = this->elements();

    for (int i = 0; i < elements.size(); ++i) {
        elements.at(i)->setPos(m_newPositions.at(i));
    }

    m_scene->setAutosaveRequired();
}

UpdateCommand::UpdateCommand(const QList<GraphicElement *> &elements, const QByteArray &oldData, Scene *scene, QUndoCommand *parent)
    : ElementsCommand(elements, scene, parent)
    // oldData must be captured by the caller *before* applying the change, so that
    // undo() can restore the previous state; the constructor cannot capture it here
    // because the change has already been applied by the time construction runs.
    , m_oldData(oldData)
{
    QDataStream stream(&m_newData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);

    // Snapshot the current (post-change) state as the "new" data used by redo()
    for (auto *elm : elements) {
        elm->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    }

    // Precompute once whether old→new changes wireless routing (a mode, or the label of a
    // Tx/Rx node — the label IS the channel). The live elements currently hold the NEW
    // state, so briefly round-trip through oldData to observe the OLD one. This cannot be
    // detected inside redo()/undo() around loadData(): the first redo()'s load is a no-op
    // (the caller mutated the elements before pushing), so old and new would compare equal
    // exactly when the escalation matters most. Only selections containing a
    // wireless-capable element pay for the round-trip.
    const auto newState = snapshotWirelessState();
    if (!newState.isEmpty()) {
        SimulationBlocker blocker(m_scene->simulation());
        loadData(m_oldData);
        m_wirelessTopologyChange = wirelessStateDiffers(snapshotWirelessState(), newState);
        loadData(m_newData);
    }

    setText(tr("Update %1 elements").arg(elements.size()));
}

void UpdateCommand::undo()
{
    qCDebug(zero) << text();
    // elm->load() can free and replace an IC's internal graph; a simulation
    // tick firing between that free and setPropertyUpdateRequired() (via
    // Application::notify + QMessageBox nested event loop) would fault.
    SimulationBlocker blocker(m_scene->simulation());
    loadData(m_oldData);
    refreshRuntimeState();
}

void UpdateCommand::redo()
{
    qCDebug(zero) << text();
    SimulationBlocker blocker(m_scene->simulation());
    loadData(m_newData);
    refreshRuntimeState();
}

QVector<UpdateCommand::WirelessState> UpdateCommand::snapshotWirelessState() const
{
    QVector<WirelessState> state;
    const auto elements = this->elements();
    for (auto *elm : elements) {
        if (elm && elm->hasWirelessMode()) {
            state.append({elm, elm->wirelessMode(), elm->label()});
        }
    }
    return state;
}

bool UpdateCommand::wirelessStateDiffers(const QVector<WirelessState> &before,
                                         const QVector<WirelessState> &after)
{
    // The wireless channel graph is built only inside Simulation::initialize()
    // (buildTxMap()/connectWirelessElements() read labels and modes there and nowhere
    // else), so a change here re-routes nothing until a full rebuild. A label change on
    // a node whose mode is None on both sides is decorative and keeps the fast path.
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

void UpdateCommand::refreshRuntimeState()
{
    // A wireless mode/label edit IS a topology change (the label is the Tx→Rx channel),
    // and the running simulation resolves channels only during initialize() — without a
    // full rebuild the old routing silently stays live.
    if (m_wirelessTopologyChange) {
        m_scene->setCircuitUpdateRequired();
        return;
    }

    // Everything else this command carries is property-only (label, color, ...) — never
    // topology — so it must not force a full Simulation::initialize(), which would rebuild
    // every Clock's phase from scratch and disrupt a running simulation over an unrelated
    // edit. setPropertyUpdateRequired() refreshes visuals/dirty-state only.
    m_scene->setPropertyUpdateRequired();
}

void UpdateCommand::loadData(QByteArray &itemData)
{
    const auto elements = this->elements();

    if (elements.isEmpty()) {
        return;
    }

    QDataStream stream(&itemData, QIODevice::ReadOnly);
    QVersionNumber version = Serialization::readPandaHeader(stream);

    QHash<quint64, Port *> portMap;
    auto context = m_scene->deserializationContext(portMap, version, SerializationPurpose::InMemorySnapshot);

    for (auto *elm : elements) {
        elm->load(stream, context);
        elm->setSelected(true);
    }
}

SplitCommand::SplitCommand(Connection *conn, QPointF mousePos, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
{
    auto *node = ElementFactory::buildElement(ElementType::Node);

    /* Align node to Grid */
    // Subtract pixmapCenter so the node's visual center lands on the mouse click,
    // then snap to the nearest grid intersection
    m_nodePos = mousePos - node->pixmapCenter();
    const int gridSize = Constants::gridSize;
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

    // Reserve a stable ID for the second wire segment (conn2) by briefly registering
    // it in the scene so it receives a real scene-local ID, then removing it.
    // redo() will recreate conn2 using updateItemId() to restore this same ID.
    auto *conn2 = new Connection();
    m_scene->addItem(conn2);
    m_c2Id = conn2->id();
    m_scene->removeItem(conn2);
    delete conn2;

    // Reserve a stable ID for the node the same way.
    m_scene->addItem(node);
    m_nodeId = node->id();
    m_scene->removeItem(node);
    delete node;

    setText(tr("Wire split"));
}

void SplitCommand::redo()
{
    qCDebug(zero) << text();
    // Block the 1 ms timer while connections and the split node are
    // deleted/rewired: a tick observing the scene mid-mutation could read a
    // momentarily detached port or a not-yet-fully-wired element.
    SimulationBlocker blocker(m_scene->simulation());
    auto *conn1 = CommandUtils::findConn(m_scene, m_c1Id);
    auto *elm1 = CommandUtils::findElm(m_scene, m_elm1Id);
    auto *elm2 = CommandUtils::findElm(m_scene, m_elm2Id);

    // Throw before any heap allocation so the throw-path can't leak.
    if (!conn1 || !elm1 || !elm2) {
        throw PANDACEPTION("Error trying to redo %1", text());
    }

    auto *conn2 = CommandUtils::findConn(m_scene, m_c2Id);
    auto *node = CommandUtils::findElm(m_scene, m_nodeId);

    // After undo(), conn2 and node were deleted; recreate them with the same
    // stable IDs so subsequent redo() calls find them correctly via findConn/findElm
    if (!conn2) {
        conn2 = new Connection();
        m_scene->updateItemId(conn2, m_c2Id);
    }

    if (!node) {
        node = ElementFactory::buildElement(ElementType::Node);
        m_scene->updateItemId(node, m_nodeId);
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
    // delete conn2 / delete node below must not race with the simulation tick.
    SimulationBlocker blocker(m_scene->simulation());
    auto *conn1 = CommandUtils::findConn(m_scene, m_c1Id);
    auto *conn2 = CommandUtils::findConn(m_scene, m_c2Id);
    auto *node = CommandUtils::findElm(m_scene, m_nodeId);
    auto *elm1 = CommandUtils::findElm(m_scene, m_elm1Id);
    auto *elm2 = CommandUtils::findElm(m_scene, m_elm2Id);

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
    : ElementsCommand(elements, scene, parent)
    , m_newType(type)
{
    m_types.reserve(elements.size());

    for (auto *oldElm : elements) {
        // Store both the ID and the original type so undo() can rebuild the old element
        // with the correct type and then re-assign the same ID via updateItemId()
        m_types.append(oldElm->elementType());
    }

    setText(tr("Morph %1 elements to %2").arg(elements.size()).arg(elements.constFirst()->objectName()));
}

void MorphCommand::restoreDeletedConnections(const QList<DeletedConnectionInfo> &deleted)
{
    // By this point transferConnections has already placed the restored elements in the
    // scene under their original IDs, so itemById() resolves correctly.
    for (const auto &info : deleted) {
        auto *morphedElm = dynamic_cast<GraphicElement *>(m_scene->itemById(info.morphedElementId));
        auto *otherElm   = dynamic_cast<GraphicElement *>(m_scene->itemById(info.otherElementId));
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
        // Restore the original ID so any undo command that stored this connection's
        // ID (e.g. DeleteItemsCommand) can still find it via scene->itemById().
        m_scene->updateItemId(conn, info.connectionId);
        m_scene->addItem(conn);
    }
}

void MorphCommand::undo()
{
    // transferConnections() deletes the current elements; a sim tick on
    // the torn state between delete and setCircuitUpdateRequired() faults.
    SimulationBlocker blocker(m_scene->simulation());

    auto newElms = elements();
    decltype(newElms) oldElms;
    oldElms.reserve(m_ids.size());

    for (int i = 0; i < m_ids.size(); ++i) {
        oldElms << ElementFactory::buildElement(m_types.at(i));
    }

    m_deletedConnectionsOnUndo.clear();
    transferConnections(newElms, oldElms, &m_deletedConnectionsOnUndo);

    // Restore connections that were deleted when the last redo() morphed to a smaller element.
    restoreDeletedConnections(m_deletedConnections);

    m_scene->setCircuitUpdateRequired();
}

void MorphCommand::redo()
{
    SimulationBlocker blocker(m_scene->simulation());

    auto oldElms = elements();
    decltype(oldElms) newElms;
    newElms.reserve(m_ids.size());

    for (int i = 0; i < m_ids.size(); ++i) {
        newElms << ElementFactory::buildElement(m_newType);
    }

    m_deletedConnections.clear();
    transferConnections(oldElms, newElms, &m_deletedConnections);

    // Restore connections that were deleted when the last undo() reverted to a smaller
    // original type — symmetric with undo()'s restoration above, so a connection added
    // while morphed and later dropped by undo() isn't lost permanently.
    restoreDeletedConnections(m_deletedConnectionsOnUndo);

    m_scene->setCircuitUpdateRequired();
}

void MorphCommand::transferConnections(const QList<GraphicElement *> &from, const QList<GraphicElement *> &to,
                                       QList<DeletedConnectionInfo> *deleted)
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

        // Mirror state is a base property of every element (not type-specific), so it is
        // always carried over — otherwise morphing a flipped element silently un-flips it.
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

        // --- Migrate existing wires to the new element's ports ---
        // The while loop drains the connection list; setEndPort/setStartPort calls
        // internally detach from oldElm and attach to newElm, so the list shrinks.
        // When the new element has fewer ports (e.g. Display14→Display7), connections
        // on the removed ports are deleted to avoid leaving dangling wires in the scene.
        transferPortConnections(oldElm, newElm, true, deleted);
        transferPortConnections(oldElm, newElm, false, deleted);

        // Reuse the old element's ID on the new element so that any external
        // references (e.g. undo commands) remain valid after the morph
        const int oldId = oldElm->id();
        m_scene->removeItem(oldElm);
        delete oldElm;

        m_scene->updateItemId(newElm, oldId);
        m_scene->addItem(newElm);
        newElm->updatePortsProperties();
    }
}

void MorphCommand::transferPortConnections(GraphicElement *oldElm, GraphicElement *newElm,
                                           bool isInput, QList<DeletedConnectionInfo> *deleted)
{
    const int portCount = isInput ? oldElm->inputSize() : oldElm->outputSize();
    for (int port = 0; port < portCount; ++port) {
        Port *oldPort = isInput ? static_cast<Port *>(oldElm->inputPort(port))
                                   : static_cast<Port *>(oldElm->outputPort(port));
        while (!oldPort->connections().isEmpty()) {
            auto *conn = oldPort->connections().constFirst();
            if (!conn) { break; }
            const bool ownsSide = isInput ? (conn->endPort() == oldElm->inputPort(port))
                                          : (conn->startPort() == oldElm->outputPort(port));
            if (!ownsSide) { break; }
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
                // Port no longer exists on the morphed element — record before deleting
                Port *otherPort = isInput ? static_cast<Port *>(conn->startPort())
                                             : static_cast<Port *>(conn->endPort());
                if (deleted && otherPort && otherPort->graphicElement()) {
                    deleted->append({conn->id(), oldElm->id(), port, isInput, otherPort->graphicElement()->id(), otherPort->index()});
                }
                conn->setStartPort(nullptr);
                conn->setEndPort(nullptr);
                m_scene->removeItem(conn);
                delete conn;
            }
        }
    }
}

FlipCommand::FlipCommand(const QList<GraphicElement *> &items, const int axis, Scene *scene, QUndoCommand *parent)
    : ElementsCommand(items, scene, parent)
    , m_axis(axis)
{
    if (items.isEmpty()) {
        return;
    }

    setText(tr("Flip %1 elements in axis %2").arg(items.size()).arg(axis));
    m_positions.reserve(items.size());

    // Compute the bounding box of all selected elements so redo() can mirror
    // each position about the selection's own axis rather than the scene origin
    double xmin = items.constFirst()->pos().rx();
    double ymin = items.constFirst()->pos().ry();
    double xmax = xmin;
    double ymax = ymin;

    for (auto *item : items) {
        m_positions.append(item->pos());
        xmin = (std::min)(xmin, item->pos().rx());
        ymin = (std::min)(ymin, item->pos().ry());
        xmax = (std::max)(xmax, item->pos().rx());
        ymax = (std::max)(ymax, item->pos().ry());
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
    for (auto *elm : elements()) {
        auto pos = elm->pos();

        // axis == 0: mirror across the vertical axis (flip horizontally)
        // axis == 1: mirror across the horizontal axis (flip vertically)
        // The formula reflects the coordinate: newX = xmin + (xmax - oldX)
        (m_axis == 0) ? pos.setX(m_minPos.rx() + (m_maxPos.rx() - pos.rx()))
                      : pos.setY(m_minPos.ry() + (m_maxPos.ry() - pos.ry()));

        elm->setPos(pos);

        // Toggle the element's mirror flag for the appropriate axis. Rotatable elements get a
        // true single-axis reflection of the whole item (scale(-1) about the pixmap centre);
        // non-rotatable input/output elements mirror only their ports, keeping the graphic
        // upright. Toggling is an involution, so undo == redo.
        (m_axis == 0) ? elm->setFlippedX(!elm->isFlippedX())
                      : elm->setFlippedY(!elm->isFlippedY());
    }

    m_scene->setAutosaveRequired();
}

ChangePortSizeCommand::ChangePortSizeCommand(const QList<GraphicElement *> &elements, const int newPortSize, Scene *scene, const bool isInput, QUndoCommand *parent)
    : ElementsCommand(elements, scene, parent)
    , m_newPortSize(newPortSize)
    , m_isInput(isInput)
{
    setText(isInput ? tr("Change input size to %1").arg(newPortSize)
                    : tr("Change output size to %1").arg(newPortSize));
}

void ChangePortSizeCommand::redo()
{
    // drainPortConnections() deletes Connections for removed ports; a
    // simulation tick between that delete and setCircuitUpdateRequired()
    // could still read a freed port/connection via a live element's
    // now-stale port list before the topology is rebuilt.
    SimulationBlocker blocker(m_scene->simulation());

    const auto elements = this->elements();

    QList<GraphicElement *> serializationOrder;
    serializationOrder.reserve(elements.size());
    m_oldData.clear();

    QDataStream stream(&m_oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);

    for (auto *elm : elements) {
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

    for (auto *elm : elements) {
        CommandUtils::drainPortConnections(elm, m_newPortSize, m_isInput ? elm->inputSize() : elm->outputSize(), m_isInput, stream, m_scene);
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

    m_scene->setCircuitUpdateRequired();
}

void ChangePortSizeCommand::undo()
{
    SimulationBlocker blocker(m_scene->simulation());

    const auto elements = this->elements();
    const auto serializationOrder = CommandUtils::findElements(m_scene, m_order);

    QDataStream stream(&m_oldData, QIODevice::ReadOnly);
    QVersionNumber version = Serialization::readPandaHeader(stream);

    QHash<quint64, Port *> portMap;
    auto context = m_scene->deserializationContext(portMap, version, SerializationPurpose::InMemorySnapshot);

    for (auto *elm : serializationOrder) {
        elm->load(stream, context);
    }

    for (auto *elm : elements) {
        int connCount; stream >> connCount;
        for (int i = 0; i < connCount; ++i) {
            int connId; stream >> connId;
            auto *conn = new Connection();
            conn->load(stream, context);
            m_scene->updateItemId(conn, connId);
            m_scene->addItem(conn);
        }

        elm->setSelected(true);
    }

    m_scene->setCircuitUpdateRequired();
}

ToggleTruthTableOutputCommand::ToggleTruthTableOutputCommand(GraphicElement *element, int pos, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_pos(pos)
{
    m_id = element->id();
    m_scene = scene;
    setText(tr("Toggle TruthTable Output at position: %1").arg(m_pos));
}

void ToggleTruthTableOutputCommand::redo()
{
    qCDebug(zero) << text();

    auto *truthtable = qobject_cast<TruthTable *>(CommandUtils::findElm(m_scene, m_id));

    if (!truthtable) throw PANDACEPTION("Could not find truthtable element!");

    // The key holds exactly 2048 bits (256 rows × 8 outputs); toggleBit on
    // any other position is an out-of-bounds write. This command is the model
    // boundary shared by the UI and the MCP server, and undo() == redo(), so
    // the bound is enforced here regardless of caller.
    if (m_pos < 0 || m_pos >= truthtable->key().size()) {
        throw PANDACEPTION("TruthTable toggle position out of range: %1", QString::number(m_pos));
    }

    truthtable->key().toggleBit(m_pos);

    m_scene->setCircuitUpdateRequired();
    emit m_scene->truthTableElementChanged(truthtable);
}

void ToggleTruthTableOutputCommand::undo()
{
    // toggleBit is self-inverse: undo == redo
    redo();
}

// --- RouteWiresCommand ---

RouteWiresCommand::RouteWiresCommand(const QVector<Entry> &entries, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_entries(entries)
    , m_scene(scene)
{
    setText(tr("Organize wires"));
}

void RouteWiresCommand::redo()
{
    apply(true);
}

void RouteWiresCommand::undo()
{
    apply(false);
}

void RouteWiresCommand::apply(bool useNew)
{
    for (const auto &entry : m_entries) {
        auto *conn = CommandUtils::findConn(m_scene, entry.connectionId);
        if (!conn) {
            continue;
        }

        if (useNew) {
            conn->setWireMode(WireMode::Orthogonal);
            conn->setWaypoints(entry.newWaypoints);
        } else {
            conn->setWireMode(entry.oldMode);
            conn->setWaypoints(entry.oldWaypoints);
        }

        conn->updatePosFromPorts();
    }

    m_scene->setAutosaveRequired();
}

// --- UpdateBlobCommand ---

QList<UpdateBlobCommand::ConnectionInfo> UpdateBlobCommand::captureConnections(const QList<GraphicElement *> &targets)
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

// --- RegisterBlobCommand ---

RegisterBlobCommand::RegisterBlobCommand(const QString &blobName, const QByteArray &data, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_blobName(blobName)
    , m_data(data)
    , m_scene(scene)
{
    setText(tr("Register blob \"%1\"").arg(blobName));
}

void RegisterBlobCommand::redo()
{
    m_scene->icRegistry()->registerBlob(m_blobName, m_data);
}

void RegisterBlobCommand::undo()
{
    m_scene->icRegistry()->removeBlob(m_blobName);
}

// --- RemoveBlobCommand ---

RemoveBlobCommand::RemoveBlobCommand(const QString &blobName, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_blobName(blobName)
    // Snapshot the blob bytes at construction so undo can restore them even
    // if the registry has been mutated in the meantime.
    , m_data(scene->icRegistry()->blob(blobName))
    , m_scene(scene)
{
    setText(tr("Remove blob \"%1\"").arg(blobName));
}

void RemoveBlobCommand::redo()
{
    m_scene->icRegistry()->removeBlob(m_blobName);
}

void RemoveBlobCommand::undo()
{
    m_scene->icRegistry()->setBlob(m_blobName, m_data);
}

// --- RenameBlobCommand ---

RenameBlobCommand::RenameBlobCommand(const QString &oldName, const QString &newName, Scene *scene, QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_oldName(oldName)
    , m_newName(newName)
    , m_scene(scene)
{
    setText(tr("Rename IC \"%1\" to \"%2\"").arg(oldName, newName));
}

void RenameBlobCommand::redo()
{
    m_scene->icRegistry()->renameBlob(m_oldName, m_newName);
}

void RenameBlobCommand::undo()
{
    m_scene->icRegistry()->renameBlob(m_newName, m_oldName);
}

// --- UpdateBlobCommand ---

UpdateBlobCommand::UpdateBlobCommand(const QList<GraphicElement *> &elements, const QByteArray &oldData,
                                     const QList<ConnectionInfo> &connections, Scene *scene, QUndoCommand *parent)
    : ElementsCommand(elements, scene, parent)
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
        m_newBlob = m_scene->icRegistry()->blob(m_blobName);
    }

    setText(tr("Update %1 IC blobs").arg(elements.size()));
}

void UpdateBlobCommand::redo()
{
    qCDebug(zero) << text();
    SimulationBlocker blocker(m_scene->simulation());
    auto *reg = m_scene->icRegistry();

    if (!m_blobName.isEmpty()) {
        if (m_newBlob.isEmpty()) {
            reg->removeBlob(m_blobName);
        } else {
            reg->setBlob(m_blobName, m_newBlob);
        }
    }
    loadData(m_newData);
    reconnectConnections();
    m_scene->setCircuitUpdateRequired();
}

void UpdateBlobCommand::undo()
{
    qCDebug(zero) << text();
    SimulationBlocker blocker(m_scene->simulation());
    auto *reg = m_scene->icRegistry();

    if (!m_blobName.isEmpty()) {
        if (m_oldBlob.isEmpty()) {
            reg->removeBlob(m_blobName);
        } else {
            reg->setBlob(m_blobName, m_oldBlob);
        }
    }
    loadData(m_oldData);
    reconnectConnections();
    m_scene->setCircuitUpdateRequired();
}

void UpdateBlobCommand::loadData(QByteArray &itemData)
{
    const auto elements = this->elements();
    if (elements.isEmpty()) {
        return;
    }

    QDataStream stream(&itemData, QIODevice::ReadOnly);
    QVersionNumber version = Serialization::readPandaHeader(stream);

    QHash<quint64, Port *> portMap;
    auto context = m_scene->deserializationContext(portMap, version, SerializationPurpose::InMemorySnapshot);

    for (auto *elm : elements) {
        elm->load(stream, context);
        elm->setSelected(true);
    }
}

void UpdateBlobCommand::reconnectConnections()
{
    for (const auto &ci : std::as_const(m_connections)) {
        auto *elm = dynamic_cast<GraphicElement *>(m_scene->itemById(ci.elementId));
        auto *otherElm = dynamic_cast<GraphicElement *>(m_scene->itemById(ci.otherElementId));
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
            // Port shrunk: the Connection that occupied this slot was
            // cascade-deleted by Qt when the IC's port was destroyed. Its
            // registry entry is already gone -- ItemWithId self-unregisters
            // from the registry in its own destructor, on any destruction
            // path. Undo restores the IC's port count and recreates the
            // connection with the original ID below.
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
        m_scene->updateItemId(conn, ci.connectionId);
        m_scene->addItem(conn);
    }
}
