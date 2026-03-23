// Copyright (c) 2012, STANISLAW ADASZEWSKI
// SPDX-License-Identifier: BSD-3-Clause
//
// Portions Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Nodes/QNEPort.h"

#include <QCursor>
#include <QDebug>
#include <QFontMetrics>
#include <QGraphicsScene>
#include <QPen>

#include "App/Core/Enums.h"
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
    QPainterPath path;

    // Port hit-area / shape: a small square centred on the port's origin (±m_radius).
    // m_radius = 5 px gives a 10×10 px clickable area which is large enough to hit
    // reliably without obscuring nearby elements.  The same value is used for the
    // visual diamond size so appearance and hit-testing stay in sync.
    path.addPolygon(QRectF(QPointF(-m_radius, -m_radius), QPointF(m_radius, m_radius)));
    setPath(path);
}

LogicElement *QNEPort::logic() const
{
    return graphicElement() ? graphicElement()->logic() : nullptr;
}

const QList<QNEConnection *> &QNEPort::connections() const
{
    return m_connections;
}

void QNEPort::connect(QNEConnection *conn)
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

void QNEPort::disconnect(QNEConnection *conn)
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

    // A port that violates its validity constraints (e.g. required but unconnected) must
    // show the invalid status regardless of any incoming signal
    if (!isValid()) {
        setStatus(Status::Invalid);
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

Status QNEPort::defaultValue() const
{
    return m_defaultStatus;
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
    // Required ports default to Invalid when unconnected so the element (and its downstream
    // chain) immediately shows an error rather than silently using a zero/low default
    setDefaultStatus(required ? Status::Invalid : Status::Inactive);
}

void QNEPort::setSerialId(quint64 serialId)
{
    m_serialId = serialId;
}

quint64 QNEPort::serialId() const
{
    return m_serialId;
}

Status QNEPort::status() const
{
    return m_status;
}

GraphicElement *QNEPort::graphicElement() const
{
    return m_graphicElement;
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

QNEInputPort::QNEInputPort(QGraphicsItem *parent)
    : QNEPort(parent)
{
    // Place the label to the left of the port dot so it doesn't overlap the element body.
    // m_margin = 2 px gap between the port dot edge and the label text.
    m_label->setPos(-m_radius - m_margin - m_label->boundingRect().width(),
                    -m_label->boundingRect().height() / 2);

    QNEInputPort::setStatus(defaultValue());
}

QNEInputPort::~QNEInputPort()
{
    // An input port owns (and must clean up) all connections that terminate here.
    // Manually remove from the list before deleting to prevent the connection destructor
    // from calling disconnect() back into a partially destroyed port.
    while (!m_connections.isEmpty()) {
        auto *conn = m_connections.constLast();
        m_connections.removeAll(conn);
        conn->setEndPort(nullptr);
        delete conn;
    }
}

void QNEInputPort::setStatus(const Status status)
{
    if (status == m_status) {
        return;
    }

    // If the port itself is invalid (e.g. required but unconnected), clamp to Invalid
    // regardless of whatever signal status is being pushed in from a connected wire
    m_status = QNEInputPort::isValid() ? status : Status::Invalid;

    const auto theme = ThemeManager::attributes();

    switch (m_status) {
    case Status::Invalid: {
        setPen(theme.m_portInvalidPen);
        setCurrentBrush(theme.m_portInvalidBrush);
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

    default:
        // Handle unexpected status values gracefully - fallback to Invalid
        setPen(theme.m_portInvalidPen);
        setCurrentBrush(theme.m_portInvalidBrush);
        break;
    }
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
}

QNEOutputPort::QNEOutputPort(QGraphicsItem *parent)
    : QNEPort(parent)
{
    // Place the label to the right of the port dot, opposite to input ports,
    // so labels read outward from the element body on both sides
    m_label->setPos(m_radius + m_margin,
                    -m_label->boundingRect().height() / 2);

    QNEOutputPort::updateTheme();
}

QNEOutputPort::~QNEOutputPort()
{
    // Mirror of QNEInputPort destructor: output port also owns the connections that originate
    // here and must break the back-reference before deletion to avoid re-entrant disconnect()
    while (!m_connections.isEmpty()) {
        auto *conn = m_connections.constLast();
        m_connections.removeAll(conn);
        conn->setStartPort(nullptr);
        delete conn;
    }
}

void QNEOutputPort::setStatus(const Status status)
{
    if (status == m_status) {
        return;
    }

    m_status = status;

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
    // An output port is valid as long as its driving logic element is valid;
    // it has no connectivity constraints (fan-out is unrestricted)
    return (m_status != Status::Invalid);
}

void QNEOutputPort::updateTheme()
{
    const auto theme = ThemeManager::attributes();
    setPen(theme.m_portOutputPen);
    setCurrentBrush(theme.m_portOutputBrush);
}

