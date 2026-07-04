// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/ConnectionManager.h"

#include <utility>

#include <QGraphicsView>

#include "App/Core/SentryHelpers.h"
#include "App/Element/GraphicElement.h"
#include "App/Scene/Commands.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/PortHoverLabel.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

namespace {
/// Which side of its element \a port faces, so its label chip can be biased away from the
/// body regardless of rotation/mirroring (ports aren't only ever on the left/right edge).
PortHoverLabel::Side sideFor(Port *port)
{
    auto *element = port->graphicElement();
    if (!element) {
        return PortHoverLabel::Side::Right;
    }

    const QPointF delta = port->scenePos() - element->sceneBoundingRect().center();
    if (qAbs(delta.x()) >= qAbs(delta.y())) {
        return (delta.x() < 0) ? PortHoverLabel::Side::Left : PortHoverLabel::Side::Right;
    }
    return (delta.y() < 0) ? PortHoverLabel::Side::Top : PortHoverLabel::Side::Bottom;
}

/// Encodes \a port's position within \a element as a single index (inputs first, then
/// outputs), or -1 if \a port doesn't belong to \a element. Mirrors decodePort() below.
int encodePortIndex(GraphicElement *element, Port *port)
{
    for (int i = 0; i < (element->inputSize() + element->outputSize()); ++i) {
        if (i < element->inputSize()) {
            if (port == element->inputPort(i)) {
                return i;
            }
        } else if (port == element->outputPort(i - element->inputSize())) {
            return i;
        }
    }
    return -1;
}

/// Resolves an (element ID, port index) pair back to a Port, or nullptr if the element
/// no longer exists or the index is out of range — e.g. the element (and its port) was
/// deleted since the pair was captured. Mirrors encodePortIndex() above.
Port *decodePort(Scene *scene, const int elmId, const int portNumber)
{
    auto *element = dynamic_cast<GraphicElement *>(scene->itemById(elmId));
    if (!element) {
        return nullptr;
    }
    if (portNumber < element->inputSize()) {
        return element->inputPort(portNumber);
    }
    if ((portNumber - element->inputSize()) < element->outputSize()) {
        return element->outputPort(portNumber - element->inputSize());
    }
    return nullptr;
}
} // namespace

ConnectionManager::ConnectionManager(Scene *scene)
    : m_scene(scene)
{
}

// --- Wire creation workflow ---

void ConnectionManager::startFromOutput(OutputPort *startPort)
{
    sentryBreadcrumb("ui", QStringLiteral("Wire started"));
    auto *connection = new Connection();
    connection->setStartPort(startPort);
    connection->setEndPos(m_scene->mousePos());

    m_scene->addItem(connection);
    setEditedConnection(connection);
    connection->updatePath();
}

void ConnectionManager::startFromInput(InputPort *endPort)
{
    auto *connection = new Connection();
    connection->setEndPort(endPort);
    connection->setStartPos(m_scene->mousePos());

    m_scene->addItem(connection);
    setEditedConnection(connection);
    connection->updatePath();
}

void ConnectionManager::tryComplete(const QPointF &scenePos)
{
    auto *connection = editedConnection();
    auto *port = qgraphicsitem_cast<Port *>(m_scene->itemAt(scenePos));

    if (!port || !connection) {
        return;
    }

    /* The mouse is released over a Port. */
    OutputPort *startPort = nullptr;
    InputPort *endPort = nullptr;

    // A wire being dragged from an output needs the drop target to be an input, and vice versa.
    // The dynamic_cast returns nullptr if the port type doesn't match, which is caught below.
    if (connection->startPort() != nullptr) {
        startPort = connection->startPort();
        endPort = dynamic_cast<InputPort *>(port);
    } else if (connection->endPort() != nullptr) {
        startPort = dynamic_cast<OutputPort *>(port);
        endPort = connection->endPort();
    }

    if (!startPort || !endPort) {
        return;
    }

    /* Verifying if the connection is valid. */
    if (isConnectionAllowed(startPort, endPort)) {
        connection->setStartPort(startPort);
        connection->setEndPort(endPort);
        sentryBreadcrumb("ui", QStringLiteral("Wire connected"));
        m_scene->receiveCommand(new AddItemsCommand({connection}, m_scene));
        setEditedConnection(nullptr);
    } else {
        deleteEditedConnection();
    }
}

void ConnectionManager::cancel()
{
    sentryBreadcrumb("ui", QStringLiteral("Wire cancelled"));
    if (editedConnection()) {
        deleteEditedConnection();
    }
}

void ConnectionManager::detach(InputPort *endPort)
{
    sentryBreadcrumb("ui", QStringLiteral("Wire detached"));
    const auto connections = endPort->connections();
    if (connections.isEmpty()) {
        return;
    }
    // Take the last connection — an input port normally has at most one, but
    // .last() is the safe choice if the model ever allows multiple
    auto *connection = connections.last();

    if (auto *startPort = connection->startPort()) {
        // Delete the existing wire, then immediately start a new in-progress wire
        // anchored to the same output port, so the user can re-route it
        m_scene->receiveCommand(new DeleteItemsCommand({connection}, m_scene));
        startFromOutput(startPort);
    }
}

// --- In-progress wire state ---

bool ConnectionManager::hasEditedConnection() const
{
    return editedConnection() != nullptr;
}

Connection *ConnectionManager::editedConnection() const
{
    return dynamic_cast<Connection *>(m_scene->itemById(m_editedConnectionId));
}

void ConnectionManager::updateEditedEnd(const QPointF &scenePos)
{
    auto *connection = editedConnection();
    if (!connection) {
        return;
    }

    if (connection->startPort()) {
        // Wire anchored at start: free end follows the mouse
        connection->setEndPos(scenePos);
        connection->updatePath();
        return;
    }

    if (connection->endPort()) {
        // Wire anchored at end (dragged from input): free start follows the mouse
        connection->setStartPos(scenePos);
        connection->updatePath();
        return;
    }

    // Connection lost both ports (e.g., element deleted mid-drag) — clean up
    deleteEditedConnection();
}

// --- Hover feedback ---

void ConnectionManager::updateHover(const QPointF &scenePos)
{
    auto *port = qgraphicsitem_cast<Port *>(m_scene->itemAt(scenePos));
    auto *hoverPort_ = hoverPort();

    // Only tear down and rebuild the hover state when the port under the cursor
    // actually changes. Rebuilding every mouse-move would recreate the peer label
    // chips each frame and make them flicker while the cursor sits on one port.
    if (hoverPort_ != port) {
        if (hoverPort_) {
            releaseHoverPort();
        }
        if (port) {
            setHoverPort(port);
        }
    }

    if (port) {
        auto *editedConn = editedConnection();
        if (editedConn && editedConn->startPort() && (editedConn->startPort()->isOutput() == port->isOutput())) {
            auto *view = m_scene->view();
            view->viewport()->setCursor(Qt::ForbiddenCursor);
        }
    }
}

void ConnectionManager::clearHover()
{
    releaseHoverPort();
}

// --- Validation ---

bool ConnectionManager::isConnectionAllowed(OutputPort *startPort, InputPort *endPort)
{
    if (!startPort || !endPort) {
        return false;
    }
    if (startPort->graphicElement() == endPort->graphicElement()) {
        return false;  // self-loop
    }
    if (startPort->isConnected(endPort)) {
        return false;  // duplicate
    }
    // Rx nodes receive their signal over the air; a physical wire on the input
    // port would be silently overridden by the simulation (tunnel convention).
    if (auto *elm = endPort->graphicElement(); elm && elm->wirelessMode() == WirelessMode::Rx) {
        return false;
    }
    // Tx nodes are dead-end transmitters; their output port drives the wireless
    // channel only — no physical wire should bypass the channel (tunnel convention).
    if (auto *elm = startPort->graphicElement(); elm && elm->wirelessMode() == WirelessMode::Tx) {
        return false;
    }
    return true;
}

// --- Private helpers ---

void ConnectionManager::setEditedConnection(Connection *connection)
{
    if (connection) {
        connection->setFocus();
        m_editedConnectionId = connection->id();
    } else {
        m_editedConnectionId = 0;
    }
}

void ConnectionManager::deleteEditedConnection()
{
    if (auto *connection = editedConnection()) {
        SimulationBlocker blocker(m_scene->simulation());
        m_scene->removeItem(connection);
        delete connection;
        m_scene->setCircuitUpdateRequired();
    }

    setEditedConnection(nullptr);
}

void ConnectionManager::setHoverPort(Port *port)
{
    if (!port) {
        m_hoverPortElmId = 0;
        m_hoverPortNumber = 0;
        return;
    }

    port->hoverEnter();
    auto *hoverElm = port->graphicElement();

    // Store element ID + port index rather than raw pointers so the hover state
    // remains valid across undo/redo operations that may recreate the element
    if (hoverElm && m_scene->contains(hoverElm->id())) {
        m_hoverPortElmId = hoverElm->id();
        m_hoverPortNumber = encodePortIndex(hoverElm, port);
    }

    // Highlight the connected ports immediately so the user can trace where the hovered
    // port's wires land. Track exactly which peers (same element-ID + port-index scheme as
    // above — Port isn't a QObject, so a raw pointer here could dangle if the peer's
    // element is deleted mid-hover) so releaseHoverPort() can release the same set later
    // even if connectivity changes mid-hover. Labels are shown separately by
    // showHoverLabels(), timed to match the native tooltip via Scene::helpEvent.
    m_highlightedPeers.clear();
    for (auto *peer : connectedPeers(port)) {
        peer->hoverEnter();
        if (auto *peerElm = peer->graphicElement(); peerElm && m_scene->contains(peerElm->id())) {
            m_highlightedPeers.append(qMakePair(peerElm->id(), encodePortIndex(peerElm, peer)));
        }
    }
}

void ConnectionManager::showHoverLabels(Port *port)
{
    clearHoverLabels();

    if (!port) {
        return;
    }

    // Reveal the label of the hovered port itself and every connected peer in situ, so the
    // user can compare a wire's two endpoint labels without tracing it across the canvas.
    // Ports without a name (most basic gates) are skipped, matching the native tooltip this
    // replaces (which silently shows nothing for empty text). Each chip is anchored at its
    // port and biased away from the element body, accounting for rotation/mirroring.
    const auto addLabel = [this](Port *target) {
        if (!target || target->name().isEmpty()) {
            return;
        }
        auto *label = new PortHoverLabel(target->name(), sideFor(target));
        label->setPos(target->scenePos());
        m_scene->addItem(label);
        m_peerLabels.append(label);
    };

    addLabel(port);
    for (auto *peer : connectedPeers(port)) {
        addLabel(peer);
    }
}

void ConnectionManager::releaseHoverPort()
{
    clearHoverLabels();

    const auto peerRefs = std::exchange(m_highlightedPeers, {});
    for (const auto &ref : peerRefs) {
        if (auto *peer = decodePort(m_scene, ref.first, ref.second)) {
            peer->hoverLeave();
        }
    }

    if (auto *hoverPort_ = hoverPort()) {
        hoverPort_->hoverLeave();
        setHoverPort(nullptr);
        auto *view = m_scene->view();
        view->viewport()->unsetCursor();
    }
}

QList<Port *> ConnectionManager::connectedPeers(Port *port)
{
    QList<Port *> peers;
    if (!port) {
        return peers;
    }

    const auto &connections = port->connections();
    peers.reserve(connections.size());
    for (auto *conn : connections) {
        Port *peer = (conn->startPort() == port) ? static_cast<Port *>(conn->endPort())
                                                    : static_cast<Port *>(conn->startPort());
        if (peer) {
            peers.append(peer);
        }
    }

    return peers;
}

void ConnectionManager::clearHoverLabels()
{
    for (const auto &label : std::as_const(m_peerLabels)) {
        if (!label.isNull()) {
            m_scene->removeItem(label);
            delete label.data();
        }
    }

    m_peerLabels.clear();
}

Port *ConnectionManager::hoverPort()
{
    Port *hoverPort = decodePort(m_scene, m_hoverPortElmId, m_hoverPortNumber);

    if (!hoverPort) {
        setHoverPort(nullptr);
    }

    return hoverPort;
}
