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

QNEPort::QNEPort(QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
    // ItemSendsScenePositionChanges triggers itemChange(ItemScenePositionHasChanged)
    // which keeps connected wires redrawn when the parent element moves
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

QPainterPath QNEPort::shape() const
{
    // Hit-area: a small square centred on the port's origin (±m_radius), independent
    // of the painted glyph.  m_radius = 5 px gives a 10×10 px clickable area which is
    // large enough to hit reliably without obscuring nearby elements; the direction
    // shapes (circle/triangle) would otherwise shrink the grab target.
    QPainterPath path;
    path.addRect(QRectF(QPointF(-m_radius, -m_radius), QPointF(m_radius, m_radius)));
    return path;
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
        return;
    }

    // Re-sync from the connection so the port colour is correct after recovering
    // from a temporarily invalid state (e.g. a second wire was added then removed).
    // The simulation won't re-push an unchanged signal, so we pull it here.
    if (isInput() && m_connections.size() == 1) {
        setStatus(m_connections.constFirst()->status());
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

    // Requiredness feeds isValid(): re-derive the displayed status so a port
    // marked optional recovers from Error and a newly required one shows it
    updateConnections();
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
}

void QNEPort::hoverEnter()
{
    setBrush(QBrush(ThemeManager::attributes().m_portHoverPort));
}

void QNEPort::updateTheme()
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
        // No Scene::removeItem call needed: ItemWithId self-unregisters from its
        // SceneItemRegistry in its own destructor, on any destruction path -- including
        // this one, where Qt's ~QGraphicsItem cascade dispatches to the non-virtual
        // QGraphicsScene::removeItem and would otherwise skip our override, leaving a
        // stale itemById entry pointing at freed memory (the WIREDPANDA-HC family).
        delete conn;
    }
}

QNEInputPort::QNEInputPort(QGraphicsItem *parent)
    : QNEPort(parent)
{
    // Circle: neutral connection point — the signal terminates here
    QPainterPath path;
    path.addEllipse(QRectF(-m_radius, -m_radius, 2 * m_radius, 2 * m_radius));
    setPath(path);

    // A fresh port is unconnected and required by default, so the validity rule
    // enforced by setStatus()/updateConnections() applies from birth: show Error
    // until a wire arrives or setRequired(false) relaxes the port. Without this,
    // elements positioned before entering the scene (file load, scripted
    // creation) never get a scene-position change to trigger the rule.
    m_status = QNEInputPort::isValid() ? m_status : Status::Error;

    // Style directly: setStatus() would early-return here because m_status
    // already holds its final construction-time value
    updateTheme();
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

    // Apply the initial pen/brush directly: setStatus() would early-return here
    // because m_status already holds the Unknown default
    updateTheme();
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
    // their status is the simulation engine's responsibility, not a validity concern
    return true;
}
