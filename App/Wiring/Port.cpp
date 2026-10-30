// Copyright (c) 2012, STANISLAW ADASZEWSKI
// SPDX-License-Identifier: BSD-3-Clause
//
// Portions Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Wiring/Port.h"

#include <utility>

#include <QPainter>
#include <QPen>

#include "App/Core/ThemeManager.h"
#include "App/Element/Element.h"
#include "App/Wiring/Connection.h"

Port::Port(QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
    // ItemSendsGeometryChanges makes itemChange() hear of the port's own moves and transforms, which re-route its
    // wires; its element's moves reach the wires through Element::itemChange(). Not
    // ItemSendsScenePositionChanges: Qt walks every item with that flag on each move of any item with children, so
    // one per port made each element's move cost the whole scene's port count.
    setFlag(QGraphicsItem::ItemSendsGeometryChanges);
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
} // LCOV_EXCL_LINE -- compiler-generated cleanup for the returned QPainterPath; the body above is reached on every real hit-test (pattern 1)

QRectF Port::boundingRect() const
{
    // The default QGraphicsPathItem bound is pen-exact (zero slack beyond the stroke), which
    // gives DeviceCoordinateCache's device-pixel tile no room to antialias the edge -- at high
    // zoom the pen's edge gets a hard clip instead of a soft fade. 1 local unit of margin (the
    // port glyph is only ~10 units across) fixes that without perceptibly growing the glyph.
    return QGraphicsPathItem::boundingRect().adjusted(-1, -1, 1, 1);
}

void Port::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    // Mirrors QGraphicsPathItem's default paint exactly, except the pen comes from
    // m_currentPen (see updateTheme()) instead of the item's own, real pen() -- the brush
    // is unaffected, still the item's real (and cheap to update) brush().
    painter->setPen(m_currentPen);
    painter->setBrush(brush());
    painter->drawPath(path());
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
    if (!m_connectionSet.contains(conn)) {
        m_connectionSet.insert(conn);
        m_connections.append(conn);
    }

    revalidateStatus();
}

void Port::detachConnection(Connection *conn)
{
    // The list holds each wire once, so the search stops where it is found: at the front when wires go in the order
    // they came, as when a circuit's wires are deleted or an IC's boundary wires move to its proxy nodes.
    if (m_connectionSet.remove(conn)) {
        m_connections.removeOne(conn);
    }

    // Null out the port reference on the connection so it knows it is detached;
    // this prevents the connection from calling disconnect() again in its destructor
    if (conn->startPort() == this) {
        conn->setStartPort(nullptr);
    }

    if (conn->endPort() == this) {
        conn->setEndPort(nullptr);
    }

    revalidateStatus();
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

    revalidateStatus();
}

void Port::revalidateStatus()
{
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
    if ((change == ItemPositionHasChanged) || (change == ItemTransformHasChanged)) {
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
    if (isOutput() && m_element) {
        return m_index + m_element->inputSize();
    }
    return m_index;
}

quint64 Port::makeSerialId(quint64 elementBase, int globalIndex)
{
    return (elementBase << 16) | (static_cast<quint64>(globalIndex) & 0xFFFF);
}

QString Port::name() const
{
    return m_nameSource ? m_nameSource() : m_name;
}

QString Port::sourceName() const
{
    return m_sourceName;
}

void Port::setName(const QString &name)
{
    setName(name, name);
}

void Port::setName(const QString &name, const QString &sourceName)
{
    m_name = name;
    m_nameSource = nullptr;
    m_sourceName = sourceName;
}

void Port::setNameSource(std::function<QString()> name, const QString &sourceName)
{
    Q_ASSERT(name);
    Q_ASSERT(!sourceName.isEmpty());
    m_name.clear();
    m_nameSource = std::move(name);
    m_sourceName = sourceName;
}

void Port::setDefaultStatus(const Status defaultStatus)
{
    m_defaultStatus = defaultStatus;
    // A connected input stays on its wire: an element sets its ports' defaults each time it lays them out, and the
    // simulation pulls an input from its driver only while running. It takes the default when its wire goes
    // (updateConnections()).
    if (isInput() && !m_connections.isEmpty()) {
        return;
    }
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
    revalidateStatus();
}

void Port::setElement(Element *element)
{
    m_element = element;
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

    // m_currentPen (drawn by paint()) is set directly instead of going through the item's
    // own setPen() -- boundingRect() pads a pen-exact bound (see its own comment) but all
    // four status pens below share the same implicit default width (theme.m_port*Pen are
    // bare QColor, converted to QPen here), so the value never actually changes and the
    // real setPen() would only pay for an unneeded BSP-tree re-index (prepareGeometryChange()).
    // shape() already uses a fixed hit-area independent of pen width, so unlike Connection
    // there's no hit-testing reason to ever fall back to the item's real pen either.
    switch (m_status) {
    case Status::Unknown: {
        m_currentPen = theme.m_portUnknownPen;
        setCurrentBrush(theme.m_portUnknownBrush);
        break;
    }
    case Status::Inactive: {
        m_currentPen = theme.m_portInactivePen;
        setCurrentBrush(theme.m_portInactiveBrush);
        break;
    }
    case Status::Active: {
        m_currentPen = theme.m_portActivePen;
        setCurrentBrush(theme.m_portActiveBrush);
        break;
    }
    case Status::Error: {
        m_currentPen = theme.m_portErrorPen;
        setCurrentBrush(theme.m_portErrorBrush);
        break;
    }
    }

    update();
}

void Port::drainConnections(bool isInput)
{
    while (!m_connections.isEmpty()) {
        auto *conn = m_connections.constLast();
        m_connections.removeLast();
        m_connectionSet.remove(conn);
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
