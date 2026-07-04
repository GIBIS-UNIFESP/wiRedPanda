// Copyright (c) 2012, STANISLAW ADASZEWSKI
// SPDX-License-Identifier: BSD-3-Clause
//
// Portions Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Wiring/Port.h"

#include <QPen>

#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElement.h"
#include "App/Wiring/Connection.h"

Port::Port(QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
    // ItemSendsScenePositionChanges triggers itemChange(ItemScenePositionHasChanged)
    // which keeps connected wires redrawn when the parent element moves
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

QPainterPath Port::shape() const
{
    // Hit-area: a small square centred on the port's origin (±kRadius), independent
    // of the painted glyph — the direction shapes (circle/triangle) would otherwise
    // shrink the grab target.
    QPainterPath path;
    path.addRect(QRectF(QPointF(-kRadius, -kRadius), QPointF(kRadius, kRadius)));
    return path;
}

const QList<Connection *> &Port::connections() const
{
    return m_connections;
}

void Port::attachConnection(Connection *conn)
{
    if (!conn) {
        return;
    }

    // Guard against duplicate entries; Connection::setStartPort/setEndPort call connect()
    // and may be called more than once during IC rewiring
    if (!m_connections.contains(conn)) {
        m_connections.append(conn);
    }

    updateConnections();
}

void Port::detachConnection(Connection *conn)
{
    m_connections.removeAll(conn);

    // Null out the port reference on the connection so it knows it is detached;
    // this prevents the connection from calling disconnect() again in its destructor
    if (conn->startPort() == this) {
        conn->setStartPort(nullptr);
    }

    if (conn->endPort() == this) {
        conn->setEndPort(nullptr);
    }

    updateConnections();
}

bool Port::isConnected(Port *otherPort)
{
    return std::any_of(m_connections.cbegin(), m_connections.cend(),
                       [&](auto *conn) { return (conn->startPort() == otherPort) || (conn->endPort() == otherPort); });
}

void Port::updateConnections()
{
    // Redraw all wires whose geometry depends on this port's scene position
    for (auto *conn : std::as_const(m_connections)) {
        conn->updatePosFromPorts();
    }

    // A port that violates its validity constraints (e.g. required but unconnected,
    // or multi-driver) must show Error so the wiring problem is clearly visible
    if (!isValid()) {
        setStatus(Status::Error);
        return;
    }

    // An unconnected optional input reverts to its default (design-time) signal value
    // rather than staying at whatever status it last had
    if (m_connections.empty() && isInput()) {
        setStatus(defaultValue());
        return;
    }

    // Re-sync from the connection so the port colour is correct after recovering
    // from a temporarily invalid state (e.g. a second wire was added then removed).
    // The simulation won't re-push an unchanged signal, so we pull it here.
    if (isInput() && m_connections.size() == 1) {
        setStatus(m_connections.constFirst()->status());
    }
}

QVariant Port::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemScenePositionHasChanged) {
        updateConnections();
    }

    return QGraphicsPathItem::itemChange(change, value);
}

int Port::index() const
{
    return m_index;
}

void Port::setIndex(const int index)
{
    m_index = index;
}

int Port::globalIndex() const
{
    if (isOutput() && m_graphicElement) {
        return m_index + m_graphicElement->inputSize();
    }
    return m_index;
}

quint64 Port::makeSerialId(quint64 elementBase, int globalIndex)
{
    return (elementBase << 16) | (static_cast<quint64>(globalIndex) & 0xFFFF);
}

QString Port::name() const
{
    return m_name;
}

void Port::setName(const QString &name)
{
    m_name = name;
    setToolTip(name);
}

void Port::setDefaultStatus(const Status defaultStatus)
{
    m_defaultStatus = defaultStatus;
    setStatus(defaultStatus);
}

QBrush Port::currentBrush() const
{
    return m_currentBrush;
}

void Port::setCurrentBrush(const QBrush &currentBrush)
{
    m_currentBrush = currentBrush;

    // Qt::yellow is used by hoverEnter() as a transient highlight; don't overwrite it
    // with the status colour while the user is hovering over the port
    if (brush().color() != Qt::yellow) {
        setBrush(currentBrush);
    }
}

bool Port::isRequired() const
{
    return m_required;
}

void Port::setRequired(const bool required)
{
    m_required = required;

    // Requiredness feeds isValid(): re-derive the displayed status so a port
    // marked optional recovers from Error and a newly required one shows it
    updateConnections();
}

void Port::setGraphicElement(GraphicElement *graphicElement)
{
    m_graphicElement = graphicElement;
}

void Port::hoverLeave()
{
    setBrush(currentBrush());
}

void Port::hoverEnter()
{
    setBrush(QBrush(ThemeManager::attributes().m_portHoverPort));
}

void Port::updateTheme()
{
    const auto &theme = ThemeManager::attributes();

    switch (m_status) {
    case Status::Unknown: {
        setPen(theme.m_portUnknownPen);
        setCurrentBrush(theme.m_portUnknownBrush);
        break;
    }
    case Status::Inactive: {
        setPen(theme.m_portInactivePen);
        setCurrentBrush(theme.m_portInactiveBrush);
        break;
    }
    case Status::Active: {
        setPen(theme.m_portActivePen);
        setCurrentBrush(theme.m_portActiveBrush);
        break;
    }
    case Status::Error: {
        setPen(theme.m_portErrorPen);
        setCurrentBrush(theme.m_portErrorBrush);
        break;
    }
    }
}

void Port::drainConnections(bool isInput)
{
    while (!m_connections.isEmpty()) {
        auto *conn = m_connections.constLast();
        m_connections.removeAll(conn);
        if (isInput) {
            conn->setEndPort(nullptr);
        } else {
            conn->setStartPort(nullptr);
        }
        // No Scene::removeItem call needed: ItemWithId self-unregisters from its
        // SceneItemRegistry in its own destructor, on any destruction path -- including
        // this one, where Qt's ~QGraphicsItem cascade dispatches to the non-virtual
        // QGraphicsScene::removeItem and would otherwise skip our override, leaving a
        // stale itemById entry pointing at freed memory (the WIREDPANDA-HC family).
        delete conn;
    }
}

InputPort::InputPort(QGraphicsItem *parent)
    : Port(parent)
{
    // Circle: neutral connection point — the signal terminates here
    QPainterPath path;
    path.addEllipse(QRectF(-kRadius, -kRadius, 2 * kRadius, 2 * kRadius));
    setPath(path);

    // A fresh port is unconnected and required by default, so the validity rule
    // enforced by setStatus()/updateConnections() applies from birth: show Error
    // until a wire arrives or setRequired(false) relaxes the port. Without this,
    // elements positioned before entering the scene (file load, scripted
    // creation) never get a scene-position change to trigger the rule.
    m_status = InputPort::isValid() ? m_status : Status::Error;

    // Style directly: setStatus() would early-return here because m_status
    // already holds its final construction-time value
    updateTheme();
}

InputPort::~InputPort()
{
    // An input port owns (and must clean up) all connections that terminate here.
    // Manually remove from the list before deleting to prevent the connection destructor
    // from calling disconnect() back into a partially destroyed port.
    drainConnections(true);
}

void InputPort::setStatus(const Status status)
{
    if (status == m_status) {
        return;
    }

    // If the port is invalid due to multiple drivers (bus conflict), emit Error so the
    // user sees a clear red signal instead of a silent gray Unknown.
    // Required-but-unconnected ports also become Error to make missing connections visible.
    m_status = InputPort::isValid() ? status : Status::Error;

    updateTheme();
}

bool InputPort::isInput() const
{
    return true;
}

bool InputPort::isOutput() const
{
    return false;
}

bool InputPort::isValid() const
{
    // Valid states: unconnected and optional (default value is safe to use), OR
    // exactly one connection (multi-driver wiring is not allowed in this simulation model)
    return m_connections.isEmpty() ? !isRequired() : (m_connections.size() == 1);
}

OutputPort::OutputPort(QGraphicsItem *parent)
    : Port(parent)
{
    // Right-pointing triangle: tip toward the wire, indicating signal flows outward
    QPainterPath path;
    path.moveTo(-kRadius, -kRadius);
    path.lineTo(+kRadius, 0);
    path.lineTo(-kRadius, +kRadius);
    path.closeSubpath();
    setPath(path);

    // Apply the initial pen/brush directly: setStatus() would early-return here
    // because m_status already holds the Unknown default
    updateTheme();
}

OutputPort::~OutputPort()
{
    // Mirror of InputPort destructor: output port also owns the connections that originate
    // here and must break the back-reference before deletion to avoid re-entrant disconnect()
    drainConnections(false);
}

void OutputPort::setStatus(const Status status)
{
    if (status == m_status) {
        return;
    }

    m_status = status;
    updateTheme();

    // Fan-out: broadcast the new signal status to every wire leaving this output port;
    // each wire in turn propagates it to the input port at its far end
    for (auto *conn : connections()) {
        conn->setStatus(status);
    }
}

bool OutputPort::isInput() const
{
    return false;
}

bool OutputPort::isOutput() const
{
    return true;
}

bool OutputPort::isValid() const
{
    // Output ports have unrestricted fan-out and no connectivity constraints;
    // their status is the simulation engine's responsibility, not a validity concern
    return true;
}
