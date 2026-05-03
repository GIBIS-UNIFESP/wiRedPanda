// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/ConnectionManager.h"

#include <QGraphicsView>

#include "App/Core/SentryHelpers.h"
#include "App/Element/GraphicElement.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Commands.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationBlocker.h"

ConnectionManager::ConnectionManager(Scene *scene)
    : m_scene(scene)
{
}

// --- Wire creation workflow ---

void ConnectionManager::startFromOutput(QNEOutputPort *startPort)
{
    sentryBreadcrumb("ui", QStringLiteral("Wire started"));
    auto *connection = new QNEConnection();
    connection->setStartPort(startPort);
    connection->setEndPos(m_scene->mousePos());

    m_scene->addItem(connection);
    setEditedConnection(connection);
    connection->updatePath();
}

void ConnectionManager::startFromInput(QNEInputPort *endPort)
{
    auto *connection = new QNEConnection();
    connection->setEndPort(endPort);
    connection->setStartPos(m_scene->mousePos());

    m_scene->addItem(connection);
    setEditedConnection(connection);
    connection->updatePath();
}

void ConnectionManager::tryComplete(const QPointF &scenePos)
{
    auto *connection = editedConnection();
    auto *port = qgraphicsitem_cast<QNEPort *>(m_scene->itemAt(scenePos));

    if (!port || !connection) {
        return;
    }

    /* The mouse is released over a QNEPort. */
    QNEOutputPort *startPort = nullptr;
    QNEInputPort *endPort = nullptr;

    // A wire being dragged from an output needs the drop target to be an input, and vice versa.
    // The dynamic_cast returns nullptr if the port type doesn't match, which is caught below.
    if (connection->startPort() != nullptr) {
        startPort = connection->startPort();
        endPort = dynamic_cast<QNEInputPort *>(port);
    } else if (connection->endPort() != nullptr) {
        startPort = dynamic_cast<QNEOutputPort *>(port);
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

void ConnectionManager::detach(QNEInputPort *endPort)
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

QNEConnection *ConnectionManager::editedConnection() const
{
    return dynamic_cast<QNEConnection *>(m_scene->itemById(m_editedConnectionId));
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
    auto *port = qgraphicsitem_cast<QNEPort *>(m_scene->itemAt(scenePos));
    auto *hoverPort_ = hoverPort();

    if (hoverPort_ && (hoverPort_ != port)) {
        releaseHoverPort();
    }

    if (port) {
        auto *editedConn = editedConnection();
        releaseHoverPort();
        setHoverPort(port);

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

bool ConnectionManager::isConnectionAllowed(QNEOutputPort *startPort, QNEInputPort *endPort)
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

void ConnectionManager::setEditedConnection(QNEConnection *connection)
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
        // Pause the simulation timer for the duration of the delete: if a
        // re-initialize ever ran while this in-progress wire was on the
        // scene, it would now sit in Simulation::m_connections, and freeing
        // it without invalidating that vector would leave a dangling entry
        // for the next tick to dereference. Matches the H2 cluster fix.
        SimulationBlocker blocker(m_scene->simulation());
        m_scene->removeItem(connection);
        delete connection;
    }

    setEditedConnection(nullptr);
}

void ConnectionManager::setHoverPort(QNEPort *port)
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

        // Encode inputs first (indices 0..inputSize-1), then outputs (inputSize..total-1)
        // so a single integer uniquely identifies any port on an element
        for (int i = 0; i < (hoverElm->inputSize() + hoverElm->outputSize()); ++i) {
            if (i < hoverElm->inputSize()) {
                if (port == hoverElm->inputPort(i)) {
                    m_hoverPortNumber = i;
                }
            } else if (port == hoverElm->outputPort(i - hoverElm->inputSize())) {
                m_hoverPortNumber = i;
            }
        }
    }
}

void ConnectionManager::releaseHoverPort()
{
    if (auto *hoverPort_ = hoverPort()) {
        hoverPort_->hoverLeave();
        setHoverPort(nullptr);
        auto *view = m_scene->view();
        view->viewport()->unsetCursor();
    }
}

QNEPort *ConnectionManager::hoverPort()
{
    QNEPort *hoverPort = nullptr;

    if (auto *hoverElm = dynamic_cast<GraphicElement *>(m_scene->itemById(m_hoverPortElmId))) {
        if (m_hoverPortNumber < hoverElm->inputSize()) {
            hoverPort = hoverElm->inputPort(m_hoverPortNumber);
        } else if (((m_hoverPortNumber - hoverElm->inputSize()) < hoverElm->outputSize())) {
            hoverPort = hoverElm->outputPort(m_hoverPortNumber - hoverElm->inputSize());
        }
    }

    if (!hoverPort) {
        setHoverPort(nullptr);
    }

    return hoverPort;
}

