// Copyright (c) 2012, STANISLAW ADASZEWSKI
// SPDX-License-Identifier: BSD-3-Clause
//
// Portions Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Wiring/Connection.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPolygonF>
#include <QStyleOptionGraphicsItem>

#include "App/Core/Application.h"
#include "App/Core/ThemeManager.h"
#include "App/Scene/Scene.h"
#include "App/Wiring/ConnectionSerializer.h"
#include "App/Wiring/Port.h"

Connection::Connection(QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
    setFlag(QGraphicsItem::ItemIsSelectable);
    // Deliberately NOT DeviceCoordinateCache (the default NoCache stays): a wire is a thin
    // stroke crossing a huge, mostly-empty bounding box, so a device-cache tile costs a
    // bbox-sized pixmap clear + blend on every status colour change -- vastly more than just
    // re-stroking the path -- and multi-MB tiles for long wires thrash the global QPixmapCache.
    // Draw wires behind elements so port dots and element bodies always render on top
    setZValue(-1);

    // m_status starts at Status::Unknown; updateTheme() applies its pen via
    // applyStatusPen(). The wire colour is updated once both ports are attached.
    updateTheme();
}

Connection::~Connection()
{
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

    setPath(path);
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

    // Feed idle detection for adaptive wire antialiasing (a stream of status changes IS the
    // simulation repaint storm, and it stops exactly when the simulation does) -- meaningless,
    // always-wasted work for a Connection never added to a QGraphicsScene (CanvasItem's case).
    // scene() is a cheap O(1) cached-pointer read, kept unconditional; the qobject_cast is
    // skipped entirely when there's no scene at all. See applyStatusPen()'s own guard and
    // project memory project_quick_hotspot_fixes_2_landed.md for the profile that found this.
    if (scene()) {
        if (auto *scene_ = qobject_cast<Scene *>(scene())) {
            scene_->noteWireActivity();
        }
    }

    applyStatusPen();

    // Propagate to the destination port so its fill colour also reflects the signal state
    if (endPort()) {
        endPort()->setStatus(status);
    }
}

void Connection::applyStatusPen()
{
    // m_statusPen only feeds paint() (QGraphicsView-based rendering) and, on a width change,
    // the item's real pen (for shape()'s hit-testing stroke) -- both meaningless, always-wasted
    // work for a Connection never added to a QGraphicsScene: CanvasItem never calls paint() on
    // these (its wire rendering reads startPort()->status() directly) and hit-tests wires
    // through its own SpatialIndex-built stroke, never QGraphicsItem::shape(). scene() is a
    // cheap O(1) cached-pointer read; everything past it is real, measurable, always-wasted
    // cost otherwise -- see project memory project_quick_hotspot_fixes_2_landed.md.
    if (!scene()) {
        return;
    }

    // Error wires are drawn thicker (5 px) to draw attention to the problem;
    // other wires are thinner (3 px) to reduce visual clutter during simulation.
    // Unknown (undriven) wires use a distinct gray from Error (red).
    switch (m_status) {
    case Status::Unknown:  m_statusPen = QPen(m_unknownColor,  3); break;
    case Status::Inactive: m_statusPen = QPen(m_inactiveColor, 3); break;
    case Status::Active:   m_statusPen = QPen(m_activeColor,   3); break;
    case Status::Error:    m_statusPen = QPen(m_errorColor,    5); break;
    }

    // paint() draws with m_statusPen, not the item's own pen() -- boundingRect() is a fixed
    // margin independent of pen width (see its own comment), so calling the real setPen()
    // here would pay for a BSP-tree re-index (prepareGeometryChange()) on every status colour
    // change for no actual geometry benefit. The one exception is width: shape()'s default
    // hit-testing *does* stroke the path using the item's real pen width, so the real setPen()
    // still runs whenever the width actually changes (Error <-> non-Error) to keep an
    // Error-status wire's (wider) click target accurate.
    if (!qFuzzyCompare(m_statusPen.widthF(), pen().widthF())) {
        setPen(m_statusPen);
        // The real pen's width sets shape()'s stroke tolerance -- rebuild the cached shape.
        m_shapeDirty = true;
    } else {
        update();
    }
}

void Connection::updateTheme()
{
    // The color-member refresh below is cheap (five QColor copies) and load-bearing: it's
    // called once unconditionally from the constructor -- necessarily before this item could
    // ever be scene-attached -- and applyStatusPen() reads these cached members rather than
    // the theme directly, so skipping this here would leave them permanently uninitialized for
    // any item, Widgets or not. Only the expensive, purely-paint-facing tail (the real pen
    // rebuild, the repaint request) is skipped when sceneless; applyStatusPen() already
    // no-ops itself in that case (see its own doc comment), so it's safe to call unconditionally.
    const auto &theme = ThemeManager::attributes();
    m_unknownColor = theme.m_connectionUnknown;
    m_inactiveColor = theme.m_connectionInactive;
    m_activeColor = theme.m_connectionActive;
    m_errorColor = theme.m_connectionError;
    m_selectedColor = theme.m_connectionSelected;

    // Re-derive the pen from the refreshed palette; without this the wire keeps
    // the previous theme's colour until its status next changes
    applyStatusPen();

    if (scene()) {
        update();
    }
}

void Connection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    // Adaptive quality (see Scene's wire-antialiasing accessors): while measured paint
    // passes are too slow for interactivity, drop this wire's antialiasing -- never force
    // it on, so a global choice like Fast Mode still wins. The painter state is
    // saved/restored around each item's paint by QGraphicsView (the DontSavePainterState
    // optimization flag is not set), so the hint can't leak into other items.
    if (auto *scene_ = qobject_cast<Scene *>(scene()); scene_ && !scene_->wireAntialiasingEnabled()) {
        painter->setRenderHint(QPainter::Antialiasing, false);
    }

    // Highlight is drawn as a wider blue halo beneath the normal wire, so users can
    // easily see which wires belong to a selected element
    if (m_highLight) {
        painter->save();
        painter->setPen(QPen(Qt::blue, 10));
        painter->drawPath(path());
        painter->restore();
    }

    // When the wire itself is selected (clicked), switch to the selection colour;
    // otherwise use the status-driven pen set by setStatus() via applyStatusPen()
    painter->setPen(isSelected() ? QPen(m_selectedColor, 5) : m_statusPen);
    painter->drawPath(path());
}

QVariant Connection::itemChange(GraphicsItemChange change, const QVariant &value)
{
    // When the wire is selected/deselected, visually highlight both endpoint ports
    // so the user can see which element pins are connected by this wire
    if (change == ItemSelectedChange) {
        if (value.toBool()) {
            if (startPort()) startPort()->hoverEnter();
            if (endPort()) endPort()->hoverEnter();
        } else {
            if (startPort()) startPort()->hoverLeave();
            if (endPort()) endPort()->hoverLeave();
        }
    }

    return QGraphicsPathItem::itemChange(change, value);
}

bool Connection::highLight()
{
    return m_highLight;
}

void Connection::setHighLight(const bool highLight)
{
    m_highLight = highLight;
    update();
}

QRectF Connection::boundingRect() const
{
    // Expand beyond the path's tight bounding box by 10 px on all sides to ensure
    // the thick selection outline and highlight halo are fully covered during repaints
    return path().boundingRect().adjusted(-10, -10, 10, 10);
}

QPainterPath Connection::shape() const
{
    if (m_shapeDirty) {
        // Same stroke semantics as Qt's default (qt_graphicsItem_shapeFromPath): the real
        // pen's width sets the click tolerance along the wire -- which is why the real
        // setPen() still runs on Error<->normal width transitions (see applyStatusPen()).
        // Stroking a flattened (polygonal) copy instead of the raw Bézier keeps later
        // intersects()/contains() tests on line segments; the flattening error is
        // sub-pixel, far inside the stroke tolerance.
        QPainterPathStroker stroker;
        stroker.setWidth(qMax(pen().widthF(), 0.00000001));
        stroker.setCapStyle(pen().capStyle());
        stroker.setJoinStyle(pen().joinStyle());
        stroker.setMiterLimit(pen().miterLimit());

        // toSubpathPolygons() keeps open subpaths open -- toFillPolygon() would close the
        // polyline back to its start, adding a phantom straight segment to the hit area.
        QPainterPath flattened;
        for (const QPolygonF &polygon : path().toSubpathPolygons()) {
            flattened.addPolygon(polygon);
        }

        m_cachedShape = stroker.createStroke(flattened);
        m_shapeDirty = false;
    }

    return m_cachedShape;
}

bool Connection::sceneEvent(QEvent *event)
{
    // Swallow Ctrl+click so the scene can use Ctrl+click for multi-selection without
    // the wire consuming the event and blocking the rubber-band/deselect behaviour
    if (auto mouseEvent = dynamic_cast<QGraphicsSceneMouseEvent *>(event)) {
        if (mouseEvent->modifiers().testFlag(Qt::ControlModifier)) {
            return true;
        }
    }

    return QGraphicsPathItem::sceneEvent(event);
}
