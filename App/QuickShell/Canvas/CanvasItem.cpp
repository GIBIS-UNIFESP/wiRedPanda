// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Canvas/CanvasItem.h"

#include <QColor>
#include <QHoverEvent>
#include <QMouseEvent>
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGVertexColorMaterial>

#include "App/Core/Enums.h"
#include "App/Core/SimulationHost.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

namespace {

/// Serves a fixed, in-memory element list to Simulation -- the same narrow seam Scene itself
/// binds through (see App/Core/SimulationHost.h), just with no QGraphicsScene behind it.
class ListSimulationHost : public SimulationHost
{
public:
    explicit ListSimulationHost(const QVector<GraphicElement *> &elements)
        : m_elements(elements)
    {
    }

    QList<QGraphicsItem *> simulationItems() const override
    {
        QList<QGraphicsItem *> items;
        items.reserve(m_elements.size());
        for (auto *element : m_elements) {
            items.append(element);
        }
        return items;
    }

    void setMuted(bool) override
    {
        // No audio-producing elements in this Phase 1 demo circuit.
    }

private:
    QVector<GraphicElement *> m_elements;
};

/// Packs a small-integer element index into the quint64 id space SpatialIndex uses. Wires get
/// odd ids, elements even ids -- cheap and sufficient while this canvas only ever holds a
/// handful of hardcoded demo objects; a real id scheme (element/port database ids) belongs to
/// the SceneItemRegistry port in a later Phase 1 iteration.
quint64 elementId(int index) { return quint64(index) * 2; }
quint64 wireId(int index) { return quint64(index) * 2 + 1; }

/// Maps a Status to a display color for the flat-quad/line rendering this prototype uses in
/// place of real per-element SVG appearance (that porting work is Phase 2's job -- see the
/// Phase 0 audit finding on ElementAppearance in the plan's Context section).
QColor colorForStatus(const Status status)
{
    switch (status) {
    case Status::Active:   return QColor(76, 175, 80);   // green
    case Status::Inactive: return QColor(120, 120, 120); // gray
    case Status::Error:    return QColor(220, 53, 69);   // red
    case Status::Unknown:  return QColor(255, 152, 0);   // orange
    }
    return QColor(120, 120, 120);
}

/// The status this prototype renders an element's own color from: its first output when it
/// has one (switches, the AND gate), otherwise its first input (Led, a pure sink with no
/// outputs of its own).
Status representativeStatus(const GraphicElement *element)
{
    if (element->outputSize() > 0) {
        return element->outputValue(0);
    }
    if (element->inputSize() > 0) {
        return element->inputPort(0)->status();
    }
    return Status::Unknown;
}

void appendQuad(QSGGeometry::ColoredPoint2D *vertices, int &cursor, const QRectF &rect, const QColor &color)
{
    const auto r = uchar(color.red());
    const auto g = uchar(color.green());
    const auto b = uchar(color.blue());
    const auto a = uchar(color.alpha());
    const float left = float(rect.left());
    const float top = float(rect.top());
    const float right = float(rect.right());
    const float bottom = float(rect.bottom());

    // Two triangles: (left,top)-(right,top)-(left,bottom), (right,top)-(right,bottom)-(left,bottom).
    vertices[cursor++].set(left, top, r, g, b, a);
    vertices[cursor++].set(right, top, r, g, b, a);
    vertices[cursor++].set(left, bottom, r, g, b, a);
    vertices[cursor++].set(right, top, r, g, b, a);
    vertices[cursor++].set(right, bottom, r, g, b, a);
    vertices[cursor++].set(left, bottom, r, g, b, a);
}

} // namespace

CanvasItem::CanvasItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents, true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);

    buildDemoCircuit();
    rebuildSpatialIndex();

    // Simulation drives real state changes on its own 1ms timer, independent of this item's
    // render loop -- this periodic update() is what actually gets those changes on screen.
    connect(&m_refreshTimer, &QTimer::timeout, this, [this] { update(); });
    m_refreshTimer.start(16); // matches Simulation's own ~60fps visual-throttle cadence
}

CanvasItem::~CanvasItem()
{
    m_simulation.reset(); // stop the timer before the elements it references are destroyed

    // Connections must be torn down before the elements they reference: Port's destructor only
    // detaches (nulls out) any connection still pointing at it, it never deletes the Connection
    // itself (see InputPort::~InputPort()/drainConnections()) -- deleting Connections first runs
    // their own destructor's clean detachConnection() calls while the ports are still alive.
    qDeleteAll(m_connections);
    qDeleteAll(m_elements);
}

void CanvasItem::buildDemoCircuit()
{
    auto *switchA = new InputSwitch();
    switchA->setPos(40, 40);
    auto *switchB = new InputSwitch();
    switchB->setPos(40, 160);
    auto *andGate = new And();
    andGate->setPos(220, 100);
    auto *led = new Led();
    led->setPos(400, 100);

    m_elements = { switchA, switchB, andGate, led };

    auto *connA = new Connection();
    connA->setStartPort(switchA->outputPort(0));
    connA->setEndPort(andGate->inputPort(0));

    auto *connB = new Connection();
    connB->setStartPort(switchB->outputPort(0));
    connB->setEndPort(andGate->inputPort(1));

    auto *connOut = new Connection();
    connOut->setStartPort(andGate->outputPort(0));
    connOut->setEndPort(led->inputPort(0));

    m_connections = { connA, connB, connOut };

    m_host = std::make_unique<ListSimulationHost>(m_elements);
    m_simulation = std::make_unique<Simulation>(m_host.get());
    m_simulation->initialize();
    m_simulation->start();
}

void CanvasItem::rebuildSpatialIndex()
{
    m_index.clear();
    m_elementsById.clear();

    for (int i = 0; i < m_elements.size(); ++i) {
        GraphicElement *element = m_elements.at(i);
        const quint64 id = elementId(i);
        const QRectF worldRect = element->boundingRect().translated(element->pos());
        m_index.insertBox(id, worldRect);
        m_elementsById.insert(id, element);
    }

    for (int i = 0; i < m_connections.size(); ++i) {
        Connection *connection = m_connections.at(i);
        QPainterPath path;
        path.moveTo(connection->startPort()->scenePos());
        path.lineTo(connection->endPort()->scenePos());
        QPainterPathStroker stroker;
        stroker.setWidth(6.0); // generous click target, matching Port::kRadius's spirit
        const QPainterPath stroke = stroker.createStroke(path);
        m_index.insertShape(wireId(i), stroke.boundingRect(), stroke);
    }
}

void CanvasItem::toggleIfInput(GraphicElement *element)
{
    if (auto *inputSwitch = qobject_cast<InputSwitch *>(element)) {
        inputSwitch->setOn(!inputSwitch->isOn(), 0);
    }
}

void CanvasItem::mousePressEvent(QMouseEvent *event)
{
    const auto hits = m_index.queryPoint(event->position());
    if (hits.isEmpty()) {
        return;
    }
    const quint64 topHit = hits.last(); // insertion-order priority, see SpatialIndex's doc comment
    if (auto *element = m_elementsById.value(topHit, nullptr)) {
        toggleIfInput(element);
        update();
    }
}

void CanvasItem::hoverMoveEvent(QHoverEvent *event)
{
    const auto hits = m_index.queryPoint(event->position());
    const quint64 newHovered = hits.isEmpty() ? 0 : hits.last();
    if (newHovered != m_hoveredId) {
        m_hoveredId = newHovered;
        update();
    }
}

void CanvasItem::hoverLeaveEvent(QHoverEvent *)
{
    if (m_hoveredId != 0) {
        m_hoveredId = 0;
        update();
    }
}

QSGNode *CanvasItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    // Gate/wire positions never change after buildDemoCircuit() in this prototype (no
    // drag-to-move yet -- that's SceneInteraction's job, Task 18's remaining scope), so the
    // index only needs rebuilding once; only per-vertex color depends on live simulation state
    // and hover, both refreshed here on every repaint.
    auto *root = oldNode ? oldNode : new QSGNode();
    if (!oldNode) {
        auto *wireNode = new QSGGeometryNode();
        auto *gateNode = new QSGGeometryNode();
        root->appendChildNode(wireNode); // wires paint first, gates on top
        root->appendChildNode(gateNode);
    }

    QSGNode *wireNode = root->firstChild();
    QSGNode *gateNode = wireNode->nextSibling();

    // --- Wires: one QSGGeometryNode, GL_LINES, colored by the driving port's live status ---
    {
        auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), int(m_connections.size()) * 2);
        geometry->setDrawingMode(QSGGeometry::DrawLines);
        geometry->setLineWidth(2.0f);
        QSGGeometry::ColoredPoint2D *vertices = geometry->vertexDataAsColoredPoint2D();
        int cursor = 0;
        for (auto *connection : m_connections) {
            const QColor color = colorForStatus(connection->startPort()->status());
            const QPointF start = connection->startPort()->scenePos();
            const QPointF end = connection->endPort()->scenePos();
            vertices[cursor++].set(float(start.x()), float(start.y()),
                                    uchar(color.red()), uchar(color.green()), uchar(color.blue()), uchar(color.alpha()));
            vertices[cursor++].set(float(end.x()), float(end.y()),
                                    uchar(color.red()), uchar(color.green()), uchar(color.blue()), uchar(color.alpha()));
        }
        static_cast<QSGGeometryNode *>(wireNode)->setGeometry(geometry);
        static_cast<QSGGeometryNode *>(wireNode)->setFlag(QSGNode::OwnsGeometry);
        if (!static_cast<QSGGeometryNode *>(wireNode)->material()) {
            auto *material = new QSGVertexColorMaterial();
            static_cast<QSGGeometryNode *>(wireNode)->setMaterial(material);
            static_cast<QSGGeometryNode *>(wireNode)->setFlag(QSGNode::OwnsMaterial);
        }
    }

    // --- Gates: one QSGGeometryNode, GL_TRIANGLES, colored by state + hover highlight ---
    {
        const int vertexCount = int(m_elements.size()) * 6;
        auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), vertexCount);
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        QSGGeometry::ColoredPoint2D *vertices = geometry->vertexDataAsColoredPoint2D();
        int cursor = 0;
        for (int i = 0; i < m_elements.size(); ++i) {
            GraphicElement *element = m_elements.at(i);
            QColor color = colorForStatus(representativeStatus(element));
            if (elementId(i) == m_hoveredId) {
                color = color.lighter(150); // hover highlight: brighten the batched fill in place
            }
            const QRectF worldRect = element->boundingRect().translated(element->pos());
            appendQuad(vertices, cursor, worldRect, color);
        }
        static_cast<QSGGeometryNode *>(gateNode)->setGeometry(geometry);
        static_cast<QSGGeometryNode *>(gateNode)->setFlag(QSGNode::OwnsGeometry);
        if (!static_cast<QSGGeometryNode *>(gateNode)->material()) {
            auto *material = new QSGVertexColorMaterial();
            static_cast<QSGGeometryNode *>(gateNode)->setMaterial(material);
            static_cast<QSGGeometryNode *>(gateNode)->setFlag(QSGNode::OwnsMaterial);
        }
    }

    return root;
}
