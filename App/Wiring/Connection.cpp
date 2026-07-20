// Copyright (c) 2012, STANISLAW ADASZEWSKI
// SPDX-License-Identifier: BSD-3-Clause
//
// Portions Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Wiring/Connection.h"

#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPolygonF>

#include "App/Core/Application.h"
#include "App/Core/ThemeManager.h"
#include "App/Wiring/ConnectionSerializer.h"
#include "App/Wiring/Port.h"

Connection::Connection()
{
    // m_status starts at Status::Unknown; updateTheme() applies its pen via
    // applyStatusPen(). The wire colour is updated once both ports are attached.
    updateTheme();
}

Connection::~Connection()
{
    if (m_destroyedCallback) {
        m_destroyedCallback(this);
    }

    if (m_startPort) {
        m_startPort->detachConnection(this);
    }

    if (m_endPort) {
        m_endPort->detachConnection(this);
    }
}

void Connection::setStartPos(const QPointF point)
{
    m_startPos = point;
}

void Connection::setEndPos(const QPointF point)
{
    m_endPos = point;
}

void Connection::changePortAttachment(Port *oldPort, Port *newPort)
{
    // Detach from the previous port before attaching to the new one to avoid
    // dangling entries in the old port's connection list (e.g. during IC hot-reload)
    if (oldPort && (oldPort != newPort)) {
        oldPort->detachConnection(this);
    }
    if (newPort) {
        newPort->attachConnection(this);
    }
}

void Connection::setStartPort(OutputPort *port)
{
    auto *oldPort = m_startPort;
    m_startPort = port;
    changePortAttachment(oldPort, port);
    if (port) {
        setStartPos(port->scenePos());
        // Inherit the source port's signal status so the wire colour is correct immediately
        setStatus(port->status());
    }
}

void Connection::setEndPort(InputPort *port)
{
    auto *oldPort = m_endPort;
    m_endPort = port;
    changePortAttachment(oldPort, port);
    if (port) {
        setEndPos(port->scenePos());
        // Push the current wire status into the destination port so it displays correctly
        // even before the next simulation step
        port->setStatus(status());
    }
}

void Connection::updatePosFromPorts()
{
    const QPointF newStartPos = m_startPort ? m_startPort->scenePos() : m_startPos;
    const QPointF newEndPos = m_endPort ? m_endPort->scenePos() : m_endPos;

    // Skip the Bézier rebuild -- and the QPainterPath comparison setPath() does internally
    // to decide whether a repaint is even needed -- when neither endpoint actually moved.
    // Some callers (bulk commands, deserialization) refresh connections without knowing in
    // advance which ones were actually affected; QPointF equality is exact and cheap, so
    // this turns those into a no-op instead of a full path rebuild + Qt's own comparison.
    if (newStartPos == m_startPos && newEndPos == m_endPos) {
        return;
    }

    m_startPos = newStartPos;
    m_endPos = newEndPos;

    updatePath();
}

void Connection::updatePath()
{
    // Skip expensive Bézier path construction when there is no visible rendering.
    // Tests and fuzz harnesses never paint, so building and comparing
    // QPainterPaths is pure waste there. This must NOT key off interactiveMode:
    // both MCP modes run non-interactively yet still render (the --mcp-gui
    // window, and export_image painting the scene off-screen in headless --mcp).
    if (!Application::renderingEnabled) {
        return;
    }

    QPainterPath path;

    path.moveTo(m_startPos);

    qreal dx = m_endPos.x() - m_startPos.x();
    qreal dy = m_endPos.y() - m_startPos.y();

    // Cubic Bézier control points chosen so the wire leaves/arrives roughly horizontally
    // (small Y fraction) and curves gently in the middle (0.25/0.75 X fraction).
    // This gives an S-curve appearance typical of schematic wire routing tools.
    QPointF ctr1(m_startPos.x() + dx * 0.25, m_startPos.y() + dy * 0.1);
    QPointF ctr2(m_startPos.x() + dx * 0.75, m_startPos.y() + dy * 0.9);

    path.cubicTo(ctr1, ctr2, m_endPos);

    m_path = path;
    m_shapeDirty = true;
}

double Connection::angle()
{
    Port *port1 = m_startPort;
    Port *port2 = m_endPort;

    if (port1 && port2) {
        // Normalise: port1 = output, port2 = input regardless of which end was set first.
        // QLineF::angle() measures from positive X axis counter-clockwise, so the
        // returned angle always describes the direction from output to input.
        if (port2->isOutput()) {
            std::swap(port1, port2);
        }

        return QLineF(port1->scenePos(), port2->scenePos()).angle();
    }

    return 0.0;
}

void Connection::save(QDataStream &stream) const
{
    ConnectionSerializer::save(*this, stream);
}

void Connection::load(QDataStream &stream, SerializationContext &context)
{
    ConnectionSerializer::load(*this, stream, context);
}

Port *Connection::otherPort(const Port *port) const
{
    if (port == m_startPort) {
        return m_endPort;
    }

    return m_startPort;
}

Status Connection::status() const
{
    return m_status;
}

void Connection::setStatus(const Status status)
{
    if (status == m_status) {
        return;
    }

    m_status = status;
    applyStatusPen();

    // Propagate to the destination port so its fill colour also reflects the signal state
    if (endPort()) {
        endPort()->setStatus(status);
    }
}

void Connection::applyStatusPen()
{
    // Error wires are drawn thicker (5 px) to draw attention to the problem;
    // other wires are thinner (3 px) to reduce visual clutter during simulation.
    // Unknown (undriven) wires use a distinct gray from Error (red).
    QPen pen;
    switch (m_status) {
    case Status::Unknown:  pen = QPen(m_unknownColor,  3); break;
    case Status::Inactive: pen = QPen(m_inactiveColor, 3); break;
    case Status::Active:   pen = QPen(m_activeColor,   3); break;
    case Status::Error:    pen = QPen(m_errorColor,    5); break;
    }

    // shape()'s hit-testing stroke tolerance tracks the pen width -- only invalidate the
    // cached shape when the width actually changes (Error <-> non-Error), not on every status
    // colour change that shares the same width.
    if (!qFuzzyCompare(pen.widthF(), m_statusPen.widthF())) {
        m_shapeDirty = true;
    }
    m_statusPen = pen;
}

void Connection::updateTheme()
{
    const auto &theme = ThemeManager::attributes();
    m_unknownColor = theme.m_connectionUnknown;
    m_inactiveColor = theme.m_connectionInactive;
    m_activeColor = theme.m_connectionActive;
    m_errorColor = theme.m_connectionError;
    m_selectedColor = theme.m_connectionSelected;

    // Re-derive the pen from the refreshed palette; without this the wire keeps
    // the previous theme's colour until its status next changes
    applyStatusPen();
}

void Connection::paint(QPainter *painter)
{
    // Highlight is drawn as a wider blue halo beneath the normal wire, so users can
    // easily see which wires belong to a selected element
    if (m_highLight) {
        painter->save();
        painter->setPen(QPen(Qt::blue, 10));
        painter->drawPath(m_path);
        painter->restore();
    }

    // When the wire itself is selected (clicked), switch to the selection colour;
    // otherwise use the status-driven pen set by setStatus() via applyStatusPen()
    painter->setPen(isSelected() ? QPen(m_selectedColor, 5) : m_statusPen);
    painter->drawPath(m_path);
}

bool Connection::highLight()
{
    return m_highLight;
}

void Connection::setHighLight(const bool highLight)
{
    m_highLight = highLight;
}

QRectF Connection::boundingRect() const
{
    // Expand beyond the path's tight bounding box by 10 px on all sides to ensure
    // the thick selection outline and highlight halo are fully covered during repaints
    return m_path.boundingRect().adjusted(-10, -10, 10, 10);
}

QPainterPath Connection::shape() const
{
    if (m_shapeDirty) {
        // Same stroke semantics as Qt's default (qt_graphicsItem_shapeFromPath): the pen's
        // width sets the click tolerance along the wire -- which is why m_statusPen's width
        // (see applyStatusPen()) is what this reads, not a fixed constant.
        QPainterPathStroker stroker;
        stroker.setWidth(qMax(m_statusPen.widthF(), 0.00000001));
        stroker.setCapStyle(m_statusPen.capStyle());
        stroker.setJoinStyle(m_statusPen.joinStyle());
        stroker.setMiterLimit(m_statusPen.miterLimit());

        // toSubpathPolygons() keeps open subpaths open -- toFillPolygon() would close the
        // polyline back to its start, adding a phantom straight segment to the hit area.
        QPainterPath flattened;
        for (const QPolygonF &polygon : m_path.toSubpathPolygons()) {
            flattened.addPolygon(polygon);
        }

        m_cachedShape = stroker.createStroke(flattened);
        m_shapeDirty = false;
    }

    return m_cachedShape;
}
