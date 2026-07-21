// Copyright (c) 2012, STANISLAW ADASZEWSKI
// SPDX-License-Identifier: BSD-3-Clause
//
// Portions Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Wiring/Port.h"

#include <QTransform>

#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElement.h"
#include "App/Wiring/Connection.h"

Port::Port() = default;

Port::~Port() = default;

QTransform Port::orientationTransform() const
{
    if (!m_graphicElement || m_graphicElement->rotatesGraphic()) {
        // Rotatable elements apply no per-port transform -- their own rotation, applied
        // uniformly via elementTransform(), moves every port (and the body) together.
        return {};
    }

    // Non-rotatable elements keep their pixmap fixed and instead transform each port about
    // the pixmap centre individually, so the port moves to the rotated/mirrored side while
    // the graphic stays upright. Oracle-validated (qtquick-rewrite plan, Phase 8a) against the
    // original QGraphicsItem::setTransform()-based ElementOrientation::orientPort().
    const QPointF origin = m_graphicElement->pixmapCenter() - m_pos;
    QTransform t;
    t.translate(origin.x(), origin.y());
    t.rotate(m_graphicElement->rotation());
    t.scale(m_graphicElement->isFlippedX() ? -1 : 1, m_graphicElement->isFlippedY() ? -1 : 1);
    t.translate(-origin.x(), -origin.y());
    return t;
}

QPointF Port::scenePos() const
{
    // Mid-teardown (see m_draining's own comment): elementTransform()/pixmapCenter() may read
    // GraphicElement::m_appearance, already destroyed by the time a port is draining (it's
    // declared after m_ports, so it destructs first). The exact position doesn't matter here --
    // nothing downstream of a draining port's geometry is ever observed.
    if (!m_graphicElement || m_draining) {
        return m_pos;
    }

    if (m_graphicElement->rotatesGraphic()) {
        return m_graphicElement->elementTransform().map(m_pos);
    }

    // The port's own pos() is added *outside* the rotate/flip-about-origin transform, not
    // mapped through it directly -- getting this backwards produces positions that are close
    // but wrong (oracle-validated, see orientationTransform()'s doc comment).
    return m_graphicElement->pos() + m_pos + orientationTransform().map(QPointF(0, 0));
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

    // Only re-derive status/validity here, not a full updateConnections() -- attaching one
    // more wire doesn't move this port, so every already-attached sibling connection's cached
    // position is still correct; conn's own position is already set by Connection::
    // setStartPort()/setEndPort() (the only callers of attachConnection()) right after this
    // call returns. Calling the full updateConnections() here was previously re-running
    // Connection::updatePosFromPorts() -> Port::scenePos() for *every* connection already on
    // this port on *every single attach* -- for a high fan-out net (e.g. one shared clock
    // output driving thousands of inputs), that made deserializing a large circuit file
    // effectively O(fan-out^2) instead of O(fan-out). See project memory
    // project_quick_minimap_optimization_landed.md's follow-up investigation for the
    // measurement that found this (clocked_8000.panda's ~20s load time).
    revalidateStatus();
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

    // Same reasoning as attachConnection(): removing a wire doesn't move this port, so the
    // remaining connections' cached positions are still correct -- only status/validity needs
    // re-deriving.
    revalidateStatus();
}

bool Port::isConnected(Port *otherPort)
{
    return std::any_of(m_connections.cbegin(), m_connections.cend(),
                       [&](auto *conn) { return (conn->startPort() == otherPort) || (conn->endPort() == otherPort); });
}

void Port::updateConnections()
{
    // Mid-teardown (see m_draining's own comment): none of this work is ever observed,
    // and recomputing geometry here can read a partially-destroyed owner element.
    if (m_draining) {
        return;
    }

    // Redraw all wires whose geometry depends on this port's scene position -- only needed
    // when this port itself may have moved (rotate/flip, via ElementOrientation::orientPort()),
    // unlike attachConnection()/detachConnection() which call revalidateStatus() directly.
    for (auto *conn : std::as_const(m_connections)) {
        conn->updatePosFromPorts();
    }

    revalidateStatus();
}

void Port::revalidateStatus()
{
    // Mid-teardown (see m_draining's own comment): none of this work is ever observed, and
    // isValid()/defaultValue() can read a partially-destroyed owner element.
    if (m_draining) {
        return;
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
    // marked optional recovers from Error and a newly required one shows it -- this doesn't
    // move the port, so only status/validity needs re-deriving, not connection positions.
    revalidateStatus();
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
}

void Port::drainConnections(bool isInput)
{
    m_draining = true;
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

InputPort::InputPort()
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

OutputPort::OutputPort()
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
