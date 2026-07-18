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
#include "App/Scene/ConnectionManager.h"
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

/// Packs a small-integer index into the quint64 id space SpatialIndex uses, tagged by kind in
/// the top bits so elements/wires/ports never collide -- cheap and sufficient while this
/// canvas only ever holds a handful of hardcoded demo objects; a real id scheme (element/port
/// database ids) belongs to the SceneItemRegistry port in a later Phase 1 iteration.
constexpr quint64 kElementTag = 0ULL << 62;
constexpr quint64 kWireTag = 1ULL << 62;
constexpr quint64 kPortTag = 2ULL << 62;
quint64 elementId(int index) { return kElementTag | quint64(index); }
quint64 wireId(int index) { return kWireTag | quint64(index); }
quint64 portId(int index) { return kPortTag | quint64(index); }

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

/// Local undo command for the drag gesture -- see CanvasItem's doc comment for why this
/// isn't the real MoveCommand (ElementsCommand::elements() resolves through a concrete
/// Scene*'s itemById(), which doesn't exist here; that's Phase 3 scope). Mirrors
/// MoveCommand::redo()/undo()'s exact shape otherwise: elements are already at their new
/// positions when this is pushed (this canvas moves them live during the drag, same as
/// QGraphicsView's built-in ItemIsMovable handling does for the real MoveCommand), so
/// redo() re-applying m_newPositions on push is idempotent, not a second move.
class CanvasMoveCommand : public QUndoCommand
{
public:
    CanvasMoveCommand(QVector<GraphicElement *> elements, QVector<QPointF> oldPositions, QVector<QPointF> newPositions)
        : m_elements(std::move(elements))
        , m_oldPositions(std::move(oldPositions))
        , m_newPositions(std::move(newPositions))
    {
    }

    void redo() override
    {
        for (int i = 0; i < m_elements.size(); ++i) {
            m_elements.at(i)->setPos(m_newPositions.at(i));
        }
    }

    void undo() override
    {
        for (int i = 0; i < m_elements.size(); ++i) {
            m_elements.at(i)->setPos(m_oldPositions.at(i));
        }
    }

private:
    QVector<GraphicElement *> m_elements;
    QVector<QPointF> m_oldPositions;
    QVector<QPointF> m_newPositions;
};

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

    // Undo/redo (via m_undoStack.undo()/redo(), not yet wired to a keyboard shortcut) moves
    // elements directly through CanvasMoveCommand::undo()/redo() -- resync the index and
    // repaint after every push/undo/redo rather than duplicating that call at each call site.
    connect(&m_undoStack, &QUndoStack::indexChanged, this, [this](int) {
        rebuildSpatialIndex();
        update();
    });
}

CanvasItem::~CanvasItem()
{
    m_simulation.reset(); // stop the timer before the elements it references are destroyed

    // An in-progress wire never made it into m_connections; tear it down the same way
    // (before the elements/ports it may still be attached to one end of).
    cancelEditedWire();

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

    // Deliberately left unwired: exercises the wire-creation-by-dragging gesture (drag from
    // switchC's output port to led2's input port) against something real, since the rest of
    // the demo circuit above is fully pre-wired for the simulation-propagation demo.
    auto *switchC = new InputSwitch();
    switchC->setPos(40, 300);
    auto *led2 = new Led();
    led2->setPos(400, 300);

    m_elements = { switchA, switchB, andGate, led, switchC, led2 };

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
    m_portsById.clear();

    int portIndex = 0;
    for (int i = 0; i < m_elements.size(); ++i) {
        GraphicElement *element = m_elements.at(i);
        const quint64 id = elementId(i);
        const QRectF worldRect = element->boundingRect().translated(element->pos());
        m_index.insertBox(id, worldRect);
        m_elementsById.insert(id, element);

        // Ports are indexed on top of (not instead of) their owning element -- a click that
        // lands on a port's small glyph should hit the port, not just the element body, so
        // ports must be inserted after their element to win SpatialIndex::queryPoint()'s
        // insertion-order "last wins" priority (see its doc comment).
        for (auto *port : element->allPorts()) {
            const quint64 pid = portId(portIndex++);
            const QRectF portRect = port->boundingRect().translated(port->scenePos());
            m_index.insertBox(pid, portRect);
            m_portsById.insert(pid, port);
        }
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

void CanvasItem::startWireFromOutput(OutputPort *startPort)
{
    auto *connection = new Connection();
    connection->setStartPort(startPort);
    m_editedConnection = connection;
    m_editedWireFreeEnd = startPort->scenePos();
}

void CanvasItem::startWireFromInput(InputPort *endPort)
{
    auto *connection = new Connection();
    connection->setEndPort(endPort);
    m_editedConnection = connection;
    m_editedWireFreeEnd = endPort->scenePos();
}

void CanvasItem::updateEditedWireEnd(const QPointF &pos)
{
    if (!m_editedConnection) {
        return;
    }
    m_editedWireFreeEnd = pos;
    // Keep Connection's own start/end-pos state consistent even though this canvas doesn't
    // paint through Connection::paint() -- see this class's doc comment on m_editedWireFreeEnd.
    if (m_editedConnection->startPort()) {
        m_editedConnection->setEndPos(pos);
    } else if (m_editedConnection->endPort()) {
        m_editedConnection->setStartPos(pos);
    }
}

void CanvasItem::tryCompleteWire(const QPointF &pos)
{
    if (!m_editedConnection) {
        return;
    }

    // Same lookup priority SpatialIndex::queryPoint() already guarantees (last = topmost);
    // scanning in reverse finds the topmost PORT specifically, mirroring Scene::portAt()'s
    // dedicated port-only lookup rather than the general element/wire/port itemAt() query.
    const auto hits = m_index.queryPoint(pos);
    Port *targetPort = nullptr;
    for (auto it = hits.crbegin(); it != hits.crend(); ++it) {
        if (auto *port = m_portsById.value(*it, nullptr)) {
            targetPort = port;
            break;
        }
    }

    // Mirrors ConnectionManager::tryComplete's exact three-tier logic: nothing under the
    // cursor at all leaves the wire in-progress (no cancel -- a release over empty space
    // just means the wire keeps following the mouse until an explicit cancel elsewhere);
    // only a genuinely incompatible/rejected port cancels it.
    if (!targetPort) {
        return;
    }

    OutputPort *startPort = nullptr;
    InputPort *endPort = nullptr;
    if (m_editedConnection->startPort()) {
        startPort = m_editedConnection->startPort();
        endPort = dynamic_cast<InputPort *>(targetPort);
    } else if (m_editedConnection->endPort()) {
        startPort = dynamic_cast<OutputPort *>(targetPort);
        endPort = m_editedConnection->endPort();
    }

    if (!startPort || !endPort) {
        return;
    }

    if (!ConnectionManager::isConnectionAllowed(startPort, endPort)) {
        cancelEditedWire();
        return;
    }

    m_editedConnection->setStartPort(startPort);
    m_editedConnection->setEndPort(endPort);
    m_connections.append(m_editedConnection);
    m_editedConnection = nullptr;
    // The new wire changes both the simulation graph and the spatial index.
    m_simulation->restart();
    rebuildSpatialIndex();
}

void CanvasItem::cancelEditedWire()
{
    delete m_editedConnection;
    m_editedConnection = nullptr;
}

void CanvasItem::detachWire(InputPort *endPort)
{
    const auto connections = endPort->connections();
    if (connections.isEmpty()) {
        return;
    }
    auto *connection = connections.last();
    auto *startPort = connection->startPort();
    if (!startPort) {
        return;
    }

    m_connections.removeAll(connection);
    delete connection;
    m_simulation->restart();

    startWireFromOutput(startPort);
}

void CanvasItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    const auto hits = m_index.queryPoint(event->position());

    // Port hit: wire-creation workflow, mirrors SceneInteraction::mousePress's
    // "if (item->type() == Port::Type) { ... }" branch -- handled first, and returns, so a
    // port press never also starts an element drag or a rubber-band underneath it. Scans in
    // reverse for the topmost PORT specifically, same reasoning as tryCompleteWire().
    Port *hitPort = nullptr;
    for (auto it = hits.crbegin(); it != hits.crend(); ++it) {
        if (auto *port = m_portsById.value(*it, nullptr)) {
            hitPort = port;
            break;
        }
    }
    if (hitPort) {
        if (m_editedConnection) {
            // An in-progress wire exists; try to complete it at this port -- mirrors
            // "if (hasEditedConnection()) { tryComplete(pos); return true; }".
            tryCompleteWire(event->position());
        } else if (auto *outputPort = dynamic_cast<OutputPort *>(hitPort)) {
            startWireFromOutput(outputPort);
        } else if (auto *inputPort = dynamic_cast<InputPort *>(hitPort)) {
            // Empty input port: begin a new wire; occupied port: detach the existing wire.
            if (inputPort->connections().isEmpty()) {
                startWireFromInput(inputPort);
            } else {
                detachWire(inputPort);
            }
        }
        update();
        return;
    }

    // Not a port press: any wire still being dragged is cancelled here, mirroring
    // SceneInteraction::mousePress's unconditional "m_scene->connectionManager()->cancel()"
    // once the Port::Type branch above didn't already return.
    cancelEditedWire();

    if (hits.isEmpty()) {
        // Empty space: mirrors SceneInteraction::mousePress's "if (!item && LeftButton)
        // startSelectionRect()".
        startSelectionRect(event->position());
        update();
        return;
    }

    const quint64 topHit = hits.last(); // insertion-order priority, see SpatialIndex's doc comment
    auto *element = m_elementsById.value(topHit, nullptr);
    if (!element) {
        // A wire, not an element or port -- wire-splitting isn't ported yet (see this
        // class's doc comment), so a wire click is deliberately a no-op past the cancel above.
        update();
        return;
    }

    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        // Ctrl+click toggles individual membership in the selection, mirroring
        // SceneInteraction::mousePress's "item->setSelected(!item->isSelected())".
        if (m_selectedIds.contains(topHit)) {
            m_selectedIds.remove(topHit);
        } else {
            m_selectedIds.insert(topHit);
        }
    }

    toggleIfInput(element);

    // Drag snapshot: the clicked element plus the rest of the current selection, mirroring
    // SceneInteraction::mousePress's "include the clicked element even if not yet selected,
    // so a single-click drag of an unselected element works immediately".
    QSet<quint64> dragIds = m_selectedIds;
    dragIds.insert(topHit);

    m_dragElements.clear();
    m_dragStartPositions.clear();
    for (const quint64 id : std::as_const(dragIds)) {
        if (auto *dragElement = m_elementsById.value(id, nullptr)) {
            m_dragElements.append(dragElement);
            m_dragStartPositions.append(dragElement->pos());
        }
    }
    m_dragAnchor = event->position();
    m_draggingElement = true;

    update();
}

void CanvasItem::mouseMoveEvent(QMouseEvent *event)
{
    if (m_draggingElement) {
        const QPointF delta = event->position() - m_dragAnchor;
        for (int i = 0; i < m_dragElements.size(); ++i) {
            m_dragElements.at(i)->setPos(m_dragStartPositions.at(i) + delta);
        }
        // Positions changed -- the index must reflect them for hit-testing/wire endpoints
        // (and for the wire batch node, which reads live scenePos() every paint anyway, but
        // the *index* itself is only ever rebuilt explicitly, not derived lazily).
        rebuildSpatialIndex();
        update();
        return;
    }

    // In-progress wire routing: mirrors SceneInteraction::mouseMove's
    // "if (hasEditedConnection()) { updateEditedEnd(pos); return true; }".
    if (m_editedConnection) {
        updateEditedWireEnd(event->position());
        update();
        return;
    }

    if (m_markingSelectionBox) {
        updateSelectionRect(event->position());
        update();
    }
}

void CanvasItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    if (m_draggingElement) {
        bool moved = false;
        QVector<QPointF> newPositions;
        newPositions.reserve(m_dragElements.size());
        for (int i = 0; i < m_dragElements.size(); ++i) {
            const QPointF newPos = m_dragElements.at(i)->pos();
            newPositions.append(newPos);
            if (newPos != m_dragStartPositions.at(i)) {
                moved = true;
            }
        }

        // Only push an undo entry if something actually moved -- mirrors
        // SceneInteraction::mouseRelease's "avoids polluting the undo stack with no-op moves
        // (click without drag)". indexChanged() (connected in the constructor) resyncs the
        // spatial index and repaints; this branch doesn't need to do either itself.
        if (moved) {
            m_undoStack.push(new CanvasMoveCommand(m_dragElements, m_dragStartPositions, newPositions));
        }

        m_draggingElement = false;
        m_dragElements.clear();
        m_dragStartPositions.clear();
    }

    if (m_markingSelectionBox) {
        finishSelectionRect();
    }

    // Complete an in-progress wire on release once no button is held any more -- mirrors
    // SceneInteraction::mouseRelease's "hasEditedConnection() && (event->buttons() ==
    // Qt::NoButton) -> tryComplete(pos)", the drag-to-connect gesture (press output -> drag
    // -> release on input). event->buttons() (plural: still-held buttons) rather than
    // event->button() (the button that triggered this release), matching the real check.
    if (m_editedConnection && event->buttons() == Qt::NoButton) {
        tryCompleteWire(event->position());
    }

    update();
}

void CanvasItem::startSelectionRect(const QPointF &anchor)
{
    m_selectionAnchor = anchor;
    m_markingSelectionBox = true;
    m_selectionRect = QRectF(anchor, anchor);
    // A fresh rubber-band replaces the previous selection, matching the baseline (no
    // modifier-driven add/subtract) rubber-band behavior -- SceneInteraction's own
    // setSelectionArea() call has the same replace semantics by default.
    m_selectedIds.clear();
}

void CanvasItem::updateSelectionRect(const QPointF &current)
{
    m_selectionRect = QRectF(m_selectionAnchor, current).normalized();
    // Mirrors SceneInteraction::mouseMove's "m_scene->setSelectionArea(selectionBox)":
    // SpatialIndex::queryRect() is this canvas's equivalent intersects-shape query.
    const auto hits = m_index.queryRect(m_selectionRect);
    m_selectedIds.clear();
    for (const quint64 id : hits) {
        if (m_elementsById.contains(id)) { // exclude wire ids -- only elements are selectable
            m_selectedIds.insert(id);
        }
    }
}

void CanvasItem::finishSelectionRect()
{
    m_markingSelectionBox = false;
    m_selectionRect = QRectF();
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
    // Element/wire positions can change now (drag-to-move rebuilds m_index on every move, see
    // mouseMoveEvent()); this method itself only ever reads current live state on each repaint,
    // it never assumes positions are static.
    auto *root = oldNode ? oldNode : new QSGNode();
    if (!oldNode) {
        auto *wireNode = new QSGGeometryNode();
        auto *gateNode = new QSGGeometryNode();
        auto *overlayNode = new QSGGeometryNode(); // live rubber-band rect, paints on top of everything
        root->appendChildNode(wireNode); // wires paint first, gates on top
        root->appendChildNode(gateNode);
        root->appendChildNode(overlayNode);
    }

    QSGNode *wireNode = root->firstChild();
    QSGNode *gateNode = wireNode->nextSibling();
    QSGNode *overlayNode = gateNode->nextSibling();

    // --- Wires: one QSGGeometryNode, GL_LINES, colored by the driving port's live status.
    // The in-progress wire (if any) is appended last, from its anchored port to
    // m_editedWireFreeEnd, in a neutral color -- it has no driving Status yet since it isn't
    // committed to any element's simulation graph. ---
    {
        const int committedCount = int(m_connections.size());
        const int vertexCount = (committedCount + (m_editedConnection ? 1 : 0)) * 2;
        auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), vertexCount);
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
        if (m_editedConnection) {
            static const QColor kEditedWireColor(158, 158, 158, 220);
            const QPointF anchored = m_editedConnection->startPort()
                ? m_editedConnection->startPort()->scenePos()
                : m_editedConnection->endPort()->scenePos();
            vertices[cursor++].set(float(anchored.x()), float(anchored.y()),
                                    uchar(kEditedWireColor.red()), uchar(kEditedWireColor.green()),
                                    uchar(kEditedWireColor.blue()), uchar(kEditedWireColor.alpha()));
            vertices[cursor++].set(float(m_editedWireFreeEnd.x()), float(m_editedWireFreeEnd.y()),
                                    uchar(kEditedWireColor.red()), uchar(kEditedWireColor.green()),
                                    uchar(kEditedWireColor.blue()), uchar(kEditedWireColor.alpha()));
        }
        static_cast<QSGGeometryNode *>(wireNode)->setGeometry(geometry);
        static_cast<QSGGeometryNode *>(wireNode)->setFlag(QSGNode::OwnsGeometry);
        if (!static_cast<QSGGeometryNode *>(wireNode)->material()) {
            auto *material = new QSGVertexColorMaterial();
            static_cast<QSGGeometryNode *>(wireNode)->setMaterial(material);
            static_cast<QSGGeometryNode *>(wireNode)->setFlag(QSGNode::OwnsMaterial);
        }
    }

    // --- Gates: one QSGGeometryNode, GL_TRIANGLES. Each selected element gets an extra
    // padded underlay quad drawn just before its own (mirrors ElementAppearance::render()'s
    // real selection treatment: a rounded selection-brush rect drawn behind the body before
    // the body itself). Plain fill color reflects live simulation state; hover brightens it. ---
    {
        const int vertexCount = int(m_elements.size() + m_selectedIds.size()) * 6;
        auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), vertexCount);
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        QSGGeometry::ColoredPoint2D *vertices = geometry->vertexDataAsColoredPoint2D();
        int cursor = 0;
        for (int i = 0; i < m_elements.size(); ++i) {
            GraphicElement *element = m_elements.at(i);
            const quint64 id = elementId(i);
            const QRectF worldRect = element->boundingRect().translated(element->pos());

            if (m_selectedIds.contains(id)) {
                static const QColor kSelectionColor(33, 150, 243, 140); // translucent blue underlay
                const QRectF padded = worldRect.adjusted(-4, -4, 4, 4);
                appendQuad(vertices, cursor, padded, kSelectionColor);
            }

            QColor color = colorForStatus(representativeStatus(element));
            if (id == m_hoveredId) {
                color = color.lighter(150); // hover highlight: brighten the batched fill in place
            }
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

    // --- Rubber-band overlay: translucent quad over the live selection rect, only while
    // marking. Zero-vertex geometry when idle -- draws nothing, cheaper than removing/
    // re-adding the node every press/release. ---
    {
        const int vertexCount = m_markingSelectionBox ? 6 : 0;
        auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), vertexCount);
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        if (m_markingSelectionBox) {
            QSGGeometry::ColoredPoint2D *vertices = geometry->vertexDataAsColoredPoint2D();
            int cursor = 0;
            static const QColor kMarqueeColor(33, 150, 243, 70);
            appendQuad(vertices, cursor, m_selectionRect, kMarqueeColor);
        }
        static_cast<QSGGeometryNode *>(overlayNode)->setGeometry(geometry);
        static_cast<QSGGeometryNode *>(overlayNode)->setFlag(QSGNode::OwnsGeometry);
        if (!static_cast<QSGGeometryNode *>(overlayNode)->material()) {
            auto *material = new QSGVertexColorMaterial();
            static_cast<QSGGeometryNode *>(overlayNode)->setMaterial(material);
            static_cast<QSGGeometryNode *>(overlayNode)->setFlag(QSGNode::OwnsMaterial);
        }
    }

    return root;
}
