// Copyright (c) 2012, STANISLAW ADASZEWSKI
// SPDX-License-Identifier: BSD-3-Clause
//
// Portions Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Originally derived from Stanislaw Adaszewski's Qt Node Editor (qneblock); since
// re-authored for wiRedPanda. The BSD attribution above is retained as a license
// obligation for the derived design.

#include "App/Wiring/Port.h"

#include <QPen>

#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElement.h"
#include "App/Wiring/Connection.h"

Port::Port(QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
    // ItemSendsScenePositionChanges triggers itemChange(ItemScenePositionHasChanged),
    // which keeps connected wires redrawn when the parent element moves.
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    // The diamond hit-area is a small square centred on the port origin (±kRadius). The
    // same value drives the visual size so appearance and hit-testing stay in sync.
    QPainterPath path;
    path.addPolygon(QRectF(QPointF(-kRadius, -kRadius), QPointF(kRadius, kRadius)));
    setPath(path);
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

    // Guard against duplicate entries; Connection::setStartPort/setEndPort call attach()
    // and may run more than once during IC rewiring.
    if (!m_connections.contains(conn)) {
        m_connections.append(conn);
    }

    updateConnections();
}

void Port::detachConnection(Connection *conn)
{
    m_connections.removeAll(conn);

    // Null out the port reference on the connection so it knows it is detached; this
    // prevents the connection from calling detach() again in its destructor.
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
    // Redraw all wires whose geometry depends on this port's scene position.
    for (auto *conn : std::as_const(m_connections)) {
        conn->updatePosFromPorts();
    }

    // A port that violates its validity constraints (e.g. required but unconnected, or
    // multi-driver) must show Error so the wiring problem is clearly visible.
    if (!isValid()) {
        setStatus(Status::Error);
        return;
    }

    // An unconnected optional input reverts to its default (design-time) signal value
    // rather than staying at whatever status it last had.
    if (m_connections.empty() && isInput()) {
        setStatus(defaultValue());
        return;
    }

    // Re-sync from the connection so the port colour is correct after recovering from a
    // temporarily invalid state (e.g. a second wire was added then removed). The
    // simulation won't re-push an unchanged signal, so we pull it here.
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
    // with the status colour while the user is hovering over the port.
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
}

int Port::globalIndex() const
{
    // Output ports are numbered after all input ports so a single integer uniquely orders every
    // port on the element (inputs 0..inputSize-1, then outputs inputSize..total-1).
    if (isOutput() && m_graphicElement) {
        return m_graphicElement->inputSize() + m_index;
    }
    return m_index;
}

quint64 Port::makeSerialId(quint64 elementBase, int globalIndex)
{
    return (elementBase << 16) | (static_cast<quint64>(globalIndex) & 0xFFFF);
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
        // Just delete: the connection drops its own id-registry entry via ~ItemWithId
        // (which forgets it from the SceneItemRegistry), and ~QGraphicsItem removes it
        // from the scene. No Scene knowledge needed here — this structurally avoids the
        // stale itemById entry that Qt's non-virtual QGraphicsScene::removeItem cascade
        // left behind (WIREDPANDA-HC family).
        delete conn;
    }
}

// --- InputPort ---

InputPort::InputPort(QGraphicsItem *parent)
    : Port(parent)
{
    InputPort::setStatus(defaultValue());
}

InputPort::~InputPort()
{
    // An input port owns (and must clean up) all connections that terminate here. Remove
    // them from the list before deleting to prevent the connection destructor from
    // calling detach() back into a partially destroyed port.
    drainConnections(true);
}

void InputPort::applyStatusStyle()
{
    const auto theme = ThemeManager::attributes();

    switch (m_status) {
    case Status::Unknown:
        setPen(theme.m_portUnknownPen);
        setCurrentBrush(theme.m_portUnknownBrush);
        break;
    case Status::Inactive:
        setPen(theme.m_portInactivePen);
        setCurrentBrush(theme.m_portInactiveBrush);
        break;
    case Status::Active:
        setPen(theme.m_portActivePen);
        setCurrentBrush(theme.m_portActiveBrush);
        break;
    case Status::Error:
        setPen(theme.m_portErrorPen);
        setCurrentBrush(theme.m_portErrorBrush);
        break;
    }
}

void InputPort::setStatus(const Status status)
{
    if (status == m_status) {
        return;
    }

    // If the port is invalid due to multiple drivers (bus conflict), show Error so the
    // user sees a clear red signal instead of a silent gray Unknown. Required-but-
    // unconnected ports also become Error to make missing connections visible.
    m_status = InputPort::isValid() ? status : Status::Error;

    applyStatusStyle();
}

void InputPort::updateTheme()
{
    // Re-apply the current status' pen/brush from the (possibly changed) palette so input
    // ports recolour on a live theme switch instead of waiting for the next setStatus().
    applyStatusStyle();
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
    // Valid states: unconnected and optional (default value is safe to use), OR exactly
    // one connection (multi-driver wiring is not allowed in this simulation model).
    return m_connections.isEmpty() ? !isRequired() : (m_connections.size() == 1);
}

// --- OutputPort ---

OutputPort::OutputPort(QGraphicsItem *parent)
    : Port(parent)
{
    OutputPort::updateTheme();
}

OutputPort::~OutputPort()
{
    // Mirror of the InputPort destructor: an output port owns the connections that
    // originate here and must break the back-reference before deletion to avoid a
    // re-entrant detach().
    drainConnections(false);
}

void OutputPort::setStatus(const Status status)
{
    if (status == m_status) {
        return;
    }

    m_status = status;

    // Fan-out: broadcast the new signal status to every wire leaving this output port;
    // each wire in turn propagates it to the input port at its far end.
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
    // An output port is valid as long as its driving logic element is valid; it has no
    // connectivity constraints (fan-out is unrestricted).
    return (m_status != Status::Unknown && m_status != Status::Error);
}

void OutputPort::updateTheme()
{
    const auto theme = ThemeManager::attributes();
    setPen(theme.m_portOutputPen);
    setCurrentBrush(theme.m_portOutputBrush);
}
