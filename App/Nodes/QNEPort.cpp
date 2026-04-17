// Copyright (c) 2012, STANISLAW ADASZEWSKI
// SPDX-License-Identifier: BSD-3-Clause
//
// Portions Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Nodes/QNEPort.h"

#include <QPen>

#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElement.h"
#include "App/Nodes/QNEConnection.h"

static const int s_inputPortMetatypeId = qRegisterMetaType<QNEInputPort>();
static const int s_outputPortMetatypeId = qRegisterMetaType<QNEOutputPort>();

QNEPort::QNEPort(QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
    // ItemSendsScenePositionChanges triggers itemChange(ItemScenePositionHasChanged)
    // which keeps connected wires redrawn when the parent element moves
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

const QList<QNEConnection *> &QNEPort::connections() const
{
    return m_connections;
}

void QNEPort::attachConnection(QNEConnection *conn)
{
    if (!conn) {
        return;
    }

    // Guard against duplicate entries; QNEConnection::setStartPort/setEndPort call connect()
    // and may be called more than once during IC rewiring
    if (!m_connections.contains(conn)) {
        m_connections.append(conn);
    }

    updateConnections();
}

void QNEPort::detachConnection(QNEConnection *conn)
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

bool QNEPort::isConnected(QNEPort *otherPort)
{
    return std::any_of(m_connections.cbegin(), m_connections.cend(),
                       [&](auto *conn) { return (conn->startPort() == otherPort) || (conn->endPort() == otherPort); });
}

void QNEPort::updateConnections()
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
    }
}

QVariant QNEPort::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemScenePositionHasChanged) {
        updateConnections();
    }

    return QGraphicsPathItem::itemChange(change, value);
}

int QNEPort::index() const
{
    return m_index;
}

void QNEPort::setIndex(const int index)
{
    m_index = index;
}

QString QNEPort::name() const
{
    return m_name;
}

void QNEPort::setName(const QString &name)
{
    m_name = name;
    setToolTip(name);
}

void QNEPort::setDefaultStatus(const Status defaultStatus)
{
    m_defaultStatus = defaultStatus;
    setStatus(defaultStatus);
}

QBrush QNEPort::currentBrush() const
{
    return m_currentBrush;
}

void QNEPort::setCurrentBrush(const QBrush &currentBrush)
{
    m_currentBrush = currentBrush;

    // Qt::yellow is used by hoverEnter() as a transient highlight; don't overwrite it
    // with the status colour while the user is hovering over the port
    if (brush().color() != Qt::yellow) {
        setBrush(currentBrush);
    }
}

bool QNEPort::isRequired() const
{
    return m_required;
}

void QNEPort::setRequired(const bool required)
{
    m_required = required;
}

void QNEPort::setSerialId(quint64 serialId)
{
    m_serialId = serialId;
}

quint64 QNEPort::serialId() const
{
    return m_serialId;
}

void QNEPort::setGraphicElement(GraphicElement *graphicElement)
{
    m_graphicElement = graphicElement;
}

void QNEPort::hoverLeave()
{
    setBrush(currentBrush());
    update();
}

void QNEPort::hoverEnter()
{
    setBrush(QBrush(ThemeManager::attributes().m_portHoverPort));
    update();
}

void QNEPort::drainConnections(bool isInput)
{
    while (!m_connections.isEmpty()) {
        auto *conn = m_connections.constLast();
        m_connections.removeAll(conn);
        if (isInput) {
            conn->setEndPort(nullptr);
        } else {
            conn->setStartPort(nullptr);
        }
        delete conn;
    }
}

QNEInputPort::QNEInputPort(QGraphicsItem *parent)
    : QNEPort(parent)
{
    // Circle: neutral connection point — universally understood as "signal comes in here"
    QPainterPath path;
    path.addEllipse(QRectF(-m_radius, -m_radius, 2 * m_radius, 2 * m_radius));
    setPath(path);

    QNEInputPort::updateTheme();
}

QNEInputPort::~QNEInputPort()
{
    // An input port owns (and must clean up) all connections that terminate here.
    // Manually remove from the list before deleting to prevent the connection destructor
    // from calling disconnect() back into a partially destroyed port.
    drainConnections(true);
}

void QNEInputPort::setStatus(const Status status)
{
    if (status == m_status) {
        return;
    }

    // If the port is invalid due to multiple drivers (bus conflict), emit Error so the
    // user sees a clear red signal instead of a silent gray Unknown.
    // Required-but-unconnected ports also become Error to make missing connections visible.
    m_status = QNEInputPort::isValid() ? status : Status::Error;

    updateTheme();
}

bool QNEInputPort::isInput() const
{
    return true;
}

bool QNEInputPort::isOutput() const
{
    return false;
}

bool QNEInputPort::isValid() const
{
    // Valid states: unconnected and optional (default value is safe to use), OR
    // exactly one connection (multi-driver wiring is not allowed in this simulation model)
    return m_connections.isEmpty() ? !isRequired() : (m_connections.size() == 1);
}

void QNEInputPort::updateTheme()
{
    const auto theme = ThemeManager::attributes();

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

    update();
}

QNEOutputPort::QNEOutputPort(QGraphicsItem *parent)
    : QNEPort(parent)
{
    // Right-pointing triangle: tip toward the wire, indicating signal flows outward
    QPainterPath path;
    path.moveTo(-m_radius, -m_radius);
    path.lineTo(+m_radius, 0);
    path.lineTo(-m_radius, +m_radius);
    path.closeSubpath();
    setPath(path);

    QNEOutputPort::updateTheme();
}

QNEOutputPort::~QNEOutputPort()
{
    // Mirror of QNEInputPort destructor: output port also owns the connections that originate
    // here and must break the back-reference before deletion to avoid re-entrant disconnect()
    drainConnections(false);
}

void QNEOutputPort::setStatus(const Status status)
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

bool QNEOutputPort::isInput() const
{
    return false;
}

bool QNEOutputPort::isOutput() const
{
    return true;
}

bool QNEOutputPort::isValid() const
{
    // Output ports have unrestricted fan-out and no connectivity constraints;
    // simulation status is set by the engine, not by connectivity logic
    return true;
}

void QNEOutputPort::updateTheme()
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

    update();
}

