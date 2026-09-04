// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Canvas/CanvasItem.h"

#include <algorithm>
#include <cmath>

#include <QClipboard>
#include <QColor>
#include <QGuiApplication>
#include <QHoverEvent>
#include <QKeyEvent>
#include <QMatrix4x4>
#include <QMimeData>
#include <QMouseEvent>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPrinter>
#include <QQuickWindow>
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGTexture>
#include <QSGTextureMaterial>
#include <QSGTransformNode>
#include <QSGVertexColorMaterial>
#include <QVarLengthArray>
#include <QWheelEvent>

#include "App/Core/Common.h"
#include "App/Core/Constants.h"
#include "App/Core/Enums.h"
#include "App/Core/InstallRelativePaths.h"
#include "App/Core/ItemWithId.h"
#include "App/Core/MimeTypes.h"
#include "App/Core/SimulationHost.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/AudioOutputElement.h"
#include "App/Element/GraphicElements/Demux.h"
#include "App/Element/GraphicElements/Display14.h"
#include "App/Element/GraphicElements/Display16.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/Element/GraphicElements/InputButton.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Mux.h"
#include "App/Element/GraphicElements/Text.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/ConnectionValidity.h"
#include "App/Wiring/Port.h"

namespace {

/// Serves CanvasItem's live, in-memory element list to Simulation -- the same narrow seam
/// Scene itself binds through (see App/Core/SimulationHost.h), just with no QGraphicsScene
/// behind it. Binds to CanvasItem::m_elements BY REFERENCE, not a snapshot copy: the Add/
/// Delete commands grow and shrink that vector after construction (CanvasItem::addItem()/
/// removeItem() append/remove elements directly), and Simulation::restart() re-reads
/// simulationItems() on every call -- a copied snapshot would silently go stale the first
/// time an element was added or removed after startup. Safe for the reference to outlive
/// this host: m_elements is a CanvasItem member that outlives m_host in ~CanvasItem() (the
/// destructor stops the simulation timer before any member teardown begins, so nothing calls
/// back into this host after m_elements itself is torn down).
class ListSimulationHost : public SimulationHost
{
public:
    explicit ListSimulationHost(const QVector<GraphicElement *> &elements)
        : m_elements(elements)
    {
    }

    QList<ItemWithId *> simulationItems() const override
    {
        QList<ItemWithId *> items;
        items.reserve(m_elements.size());
        for (auto *element : m_elements) {
            items.append(element);
        }
        return items;
    }

    void setMuted(bool) override
    {
        // No audio-producing elements in this demo circuit.
    }

private:
    const QVector<GraphicElement *> &m_elements;
};

/// Packs an id into the quint64 id space SpatialIndex uses, tagged by kind in the top bits so
/// elements/wires/ports never collide. elementId()/wireId() take the real ItemWithId::id()
/// (assigned via CanvasItem::addItem(), see this class's doc comment on the id/registry layer)
/// -- ItemWithId::id() is always non-negative once assigned, same precondition an array index
/// had. portId() stays index-based, but the index itself is never positional: Port (not an
/// ItemWithId, see Port.h) has no real id of its own, so CanvasItem::spatialIdFor() assigns and
/// caches a real one per Port* the first time each is seen, the same "real assigned id" shape
/// m_itemRegistry gives elements/connections.
constexpr quint64 kElementTag = 0ULL << 62;
constexpr quint64 kWireTag = 1ULL << 62;
constexpr quint64 kPortTag = 2ULL << 62;
quint64 elementId(int id) { return kElementTag | quint64(id); }
quint64 wireId(int id) { return kWireTag | quint64(id); }
quint64 portId(int index) { return kPortTag | quint64(index); }
/// Strips a tagged id's top-2-bit kind tag, recovering the plain id/index passed to
/// elementId()/wireId()/portId(). Used by mouseDoubleClickEvent() to recover a wire's real
/// Connection::id() from a SpatialIndex hit.
int unwrapId(quint64 tagged) { return int(tagged & ~(kWireTag | kPortTag)); }

/// Builds the click-target stroke SpatialIndex indexes \a connection's wire under -- shared by
/// CanvasItem::rebuildSpatialIndex() and CanvasItem::updateSpatialIndexFor() so the stroke
/// width can't drift between the two paths.
QPainterPath strokeShapeFor(Connection *connection)
{
    // An in-progress wire (mid-drag, or freshly `new Connection()`'d with neither endpoint
    // attached yet) has a null startPort()/endPort() until both ends are wired up; both callers
    // (rebuildSpatialIndex()/updateSpatialIndexFor()) run unconditionally over every registered
    // connection, not just fully-wired ones, so this must guard against that case explicitly.
    if (!connection->startPort() || !connection->endPort()) {
        return {};
    }
    QPainterPath path;
    path.moveTo(connection->startPort()->scenePos());
    path.lineTo(connection->endPort()->scenePos());
    QPainterPathStroker stroker;
    stroker.setWidth(6.0); // generous click target, matching Port::kRadius's spirit
    return stroker.createStroke(path);
}

/// Maps a Status to a display color for the wire (still flat-line) rendering this canvas
/// uses -- wires have no appearance texture of their own to draw instead.
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

/// Returns a QSGGeometry with exactly \a vertexCount vertices, reusing \a node's existing
/// geometry object across frames (via QSGGeometry::allocate()) instead of `new`-ing a fresh one
/// on every repaint -- the common case is that a node's vertex count stays the same frame to
/// frame (only add/delete/paste/undo/redo/hover-toggle/rubber-band-toggle actually change one),
/// so the allocation itself doesn't need redoing just because the *contents* do. Only allocates
/// a genuinely new QSGGeometry on the node's first-ever build (\a node has none yet). Callers
/// still write fresh vertex data into the returned geometry and must call
/// node->markDirty(QSGNode::DirtyGeometry) themselves -- this only owns the allocation, not the
/// data or the dirty notification.
QSGGeometry *geometryFor(QSGGeometryNode *node, const QSGGeometry::AttributeSet &attributes, int vertexCount)
{
    if (QSGGeometry *existing = node->geometry()) {
        existing->allocate(vertexCount);
        return existing;
    }
    auto *geometry = new QSGGeometry(attributes, vertexCount);
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    return geometry;
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

/// Same two-triangle quad as appendQuad(), but textured (position + UV) instead of flat-colored
/// -- used for real per-element appearance drawn through TextureAtlas.
void appendTexturedQuad(QSGGeometry::TexturedPoint2D *vertices, int &cursor, const QRectF &rect, const QRectF &uv)
{
    const float left = float(rect.left());
    const float top = float(rect.top());
    const float right = float(rect.right());
    const float bottom = float(rect.bottom());
    const float u0 = float(uv.left());
    const float v0 = float(uv.top());
    const float u1 = float(uv.right());
    const float v1 = float(uv.bottom());

    vertices[cursor++].set(left, top, u0, v0);
    vertices[cursor++].set(right, top, u1, v0);
    vertices[cursor++].set(left, bottom, u0, v1);
    vertices[cursor++].set(right, top, u1, v0);
    vertices[cursor++].set(right, bottom, u1, v1);
    vertices[cursor++].set(left, bottom, u0, v1);
}

/// Segment count each wire is tessellated into for the Bézier curve below -- 1 straight
/// segment (2 vertices) becomes 16 segments (32 vertices) per wire.
constexpr int kWireSegments = 16;

// Zoom step ladder (factor 1.25 per step), identical range to GraphicsView.cpp's own
// kMaxViewZoomLevel/kMinViewZoomLevel -- deliberately asymmetric (elements stay useful when
// tiny, so far more zoom-out than in is allowed).
constexpr int kMaxZoomLevel = 7;  ///< 1.25^7 ~= 4.8x.
constexpr int kMinZoomLevel = -9; ///< 0.8^9 ~= 0.13x.

/// Above this many elements, renderMinimapImage() switches from each element's real
/// appearance+label (paintElementsInto()) to flat rects + straight wire lines
/// (paintElementsSimplifiedInto()) -- individual element detail is imperceptible at the
/// minimap's thumbnail scale regardless. The direct-paint cost scales roughly linearly with
/// element count (measured debug-build: ~25ms at 1000 elements, ~89ms at 5000, ~358ms at 20000),
/// with no severe super-linear blowup, so 2000 is a deliberately conservative threshold rather
/// than one pushed as high as the numbers would allow -- the caching this same path relies on is
/// what actually matters for the common case; this fallback only matters for the rarer
/// real-rebuild path on a very large circuit.
constexpr int kMinimapSimplifiedThreshold = 2000;

/// World-space padding scrollableWorldRect() adds on every side, so a scrollbar's travel range
/// never collapses exactly onto the content/visible area's own edges (dragging to either
/// extreme still leaves a little headroom, matching a generous QGraphicsView sceneRect()'s own
/// feel rather than a razor-tight fit).
constexpr qreal kScrollMargin = 200.0;

/// Evaluates the exact same cubic Bézier S-curve Connection::updatePath() builds
/// (App/Wiring/Connection.cpp) at kWireSegments+1 points from \a start to \a end, so the
/// interactive canvas's wire shape matches the real production curve instead of a single
/// straight line segment.
QVarLengthArray<QPointF, kWireSegments + 1> tessellateWire(const QPointF &start, const QPointF &end)
{
    const qreal dx = end.x() - start.x();
    const qreal dy = end.y() - start.y();
    const QPointF ctr1(start.x() + dx * 0.25, start.y() + dy * 0.1);
    const QPointF ctr2(start.x() + dx * 0.75, start.y() + dy * 0.9);

    QVarLengthArray<QPointF, kWireSegments + 1> points;
    for (int i = 0; i <= kWireSegments; ++i) {
        const qreal t = qreal(i) / kWireSegments;
        const qreal mt = 1.0 - t;
        // Cubic Bézier: B(t) = mt^3*P0 + 3*mt^2*t*P1 + 3*mt*t^2*P2 + t^3*P3
        const qreal x = mt * mt * mt * start.x() + 3 * mt * mt * t * ctr1.x() + 3 * mt * t * t * ctr2.x() + t * t * t * end.x();
        const qreal y = mt * mt * mt * start.y() + 3 * mt * mt * t * ctr1.y() + 3 * mt * t * t * ctr2.y() + t * t * t * end.y();
        points.append(QPointF(x, y));
    }
    return points;
}

/// Screen-space radius the port-hover highlight ring is drawn at -- no zoom to scale by yet
/// (this canvas has no pan/zoom transform), so a flat constant is enough for now.
constexpr qreal kPortHoverMarkerRadius = 8.0;

/// Every port wired to \a port via a Connection, in \a port's own connections() order. Mirrors
/// ConnectionManager::connectedPeers() (App/Scene/ConnectionManager.cpp, WIDGETS_ONLY_SOURCES
/// on this branch) -- reimplemented locally rather than called, same reasoning as
/// isConnectionAllowed()/connectionRejectionReason() needing the portable ConnectionValidity.h
/// free functions instead of ConnectionManager's own methods.
QVector<Port *> connectedPeers(Port *port)
{
    QVector<Port *> peers;
    for (auto *conn : port->connections()) {
        Port *other = (conn->startPort() == port)
            ? static_cast<Port *>(conn->endPort())
            : static_cast<Port *>(conn->startPort());
        if (other) {
            peers.append(other);
        }
    }
    return peers;
}

/// Which side of its owning element's body a port's name chip should be biased toward --
/// inputs read left-to-right into the element, outputs read out of it, matching this project's
/// own left-input/right-output port layout convention throughout every element family.
QString sideForPort(const Port *port)
{
    return port->isInput() ? QStringLiteral("left") : QStringLiteral("right");
}

/// Snapshots each element's current position, applies \a newPositions (already computed by
/// the caller), then pushes one CanvasMoveCommand for the whole batch -- shared by every
/// align/distribute method. Mirrors Scene.cpp's identically-named anonymous-namespace helper.
void moveElementsTo(CanvasItem *canvas, const QList<GraphicElement *> &elements, const QList<QPointF> &newPositions)
{
    QList<QPointF> oldPositions;
    oldPositions.reserve(elements.size());
    for (auto *elm : elements) {
        oldPositions.append(elm->pos());
    }

    for (int i = 0; i < elements.size(); ++i) {
        elements.at(i)->setPos(newPositions.at(i));
    }

    canvas->receiveCommand(new CanvasMoveCommand(elements, oldPositions, canvas));
}

} // namespace

CanvasItem::CanvasItem(QQuickItem *parent, bool buildDemo)
    : QQuickItem(parent)
{
    setFlag(QQuickItem::ItemHasContents, true);
    // RightButton is needed for the context-menu gesture (mousePressEvent()'s RightButton
    // branch); MiddleButton for the pan gesture (mirrors GraphicsView::mousePressEvent()'s
    // identical MiddleButton check).
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
    setAcceptHoverEvents(true);
    // Without this, panned/dragged content (built in world coordinates, unbounded by this
    // item's own screen rect) paints straight over whatever QML sits to this item's left --
    // the SplitView's palette/element-editor pane. GraphicsView gets this for free from its
    // viewport; Quick items don't clip their own paint node by default.
    setClip(true);

    if (buildDemo) {
        buildDemoCircuit();
    }

    // Host/simulation setup is real, always-needed infrastructure, not demo-only content --
    // unconditional regardless of buildDemo, using whatever m_elements buildDemoCircuit() (or
    // nothing, for a real document's initially-empty canvas) left behind.
    m_host = std::make_unique<ListSimulationHost>(m_elements);
    m_simulation = std::make_unique<Simulation>(m_host.get());
    m_simulation->initialize();
    m_simulation->start();

    rebuildSpatialIndex();

    // Simulation drives real state changes on its own 1ms timer, independent of this item's
    // render loop; visualStateChanged() fires only when a real, display-rate-throttled visual
    // flush actually happened (Simulation::update()'s own m_atFixedPoint/m_visualsDirty/
    // visualsDue gate) -- schedules exactly one repaint per real change instead of a blind
    // poll on every tick.
    connect(m_simulation.get(), &Simulation::visualStateChanged, this, [this] { update(); });

    // Mirrors Scene's own identical constructor connection: without it, a live Light/Dark/System
    // theme switch never refreshes this canvas's already-rendered elements/ports/connections.
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &CanvasItem::updateTheme);
}

void CanvasItem::setGatesVisible(bool visible)
{
    if (m_gatesVisible == visible) {
        return;
    }
    m_gatesVisible = visible;
    rebuildSpatialIndex(); // hidden gates must stop being hit-testable, not just stop drawing
    // isElementHidden()'s result changed for every affected element, but that's not reflected
    // in any per-element ElementRenderCache field (isRenderDirty() only tracks port *status*
    // changes) -- drop the whole render cache so every tile gets a fresh isElementHidden()
    // check on the next repaint, mirroring TextureAtlas::clear()'s own "drop everything on a
    // global invalidation" precedent used for the theme-refresh fix.
    m_elementRenderCache.clear();
    update();
}

void CanvasItem::setWiresVisible(bool visible)
{
    if (m_wiresVisible == visible) {
        return;
    }
    m_wiresVisible = visible;
    rebuildSpatialIndex();
    // See setGatesVisible()'s identical comment -- isPortHidden()'s result changed for every
    // port, invalidating the port glyphs baked into every element's cached tile.
    m_elementRenderCache.clear();
    update();
}

void CanvasItem::updateTheme()
{
    for (auto *element : m_elements) {
        element->updateTheme();
    }
    for (auto *conn : m_connections) {
        conn->updateTheme();
    }
    // Drops every cached appearance tile -- see this method's own header doc comment for why
    // that's required alongside the element/connection loops above.
    m_atlas.clear();
    update();
}

bool CanvasItem::isElementHidden(const GraphicElement *element) const
{
    if (!m_gatesVisible) {
        const auto group = element->elementGroup();
        // Only hide/show internal logic gates; Input, Output and Other elements (e.g.
        // labels, ICs) are always kept visible regardless of this toggle.
        if (group != ElementGroup::Input && group != ElementGroup::Output && group != ElementGroup::Other) {
            return true;
        }
    }
    if (!m_wiresVisible && element->elementType() == ElementType::Node) {
        // Node elements are purely wire-routing helpers with no logical function; hiding
        // wires should hide nodes too since they're meaningless without wires.
        return true;
    }
    return false;
}

bool CanvasItem::isPortHidden(const Port *port) const
{
    if (m_wiresVisible) {
        return false;
    }
    // For non-Node elements, hide only their port handles (the connectable dots), not the
    // element body itself, so the gate symbols remain visible. A Node's own ports are already
    // covered by isElementHidden() hiding the whole element.
    const auto *owner = port->graphicElement();
    return owner && owner->elementType() != ElementType::Node;
}

CanvasItem::~CanvasItem()
{
    // QUndoStack::~QUndoStack() calls clear() internally, which emits indexChanged()/
    // cleanChanged() -- harmless for this class's own internal state (nothing here connects
    // to those signals; every command calls restartSimulation()/rebuildSpatialIndex()
    // directly instead of relying on a signal), but an external owner (QuickWorkSpace)
    // connects to undoStack() from outside, so silence signals here too as a defensive
    // measure for whichever owner destroys this canvas while still holding such a
    // connection -- see QuickWorkSpace::~QuickWorkSpace() for the matching guard on that side.
    m_undoStack.blockSignals(true);

    delete m_gridDotTexture; // not owned by a QSGTextureMaterial -- see its own doc comment

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

void CanvasItem::addItem(GraphicElement *element)
{
    if (!element) {
        return;
    }
    // Membership half (mirrors QGraphicsScene::addItem() being the other half of Scene::
    // addItem()'s job): idempotent, so callers that already appended element to m_elements
    // directly (buildDemoCircuit()) and callers relying on addItem() to do it
    // (CanvasCommandUtils::addItems(), the Add/Delete command family) both work.
    // m_elementSet is the O(1) membership check backing this -- see its own doc comment.
    if (!m_elementSet.contains(element)) {
        m_elements.append(element);
        m_elementSet.insert(element);
    }
    // Id-registration half. Unassigned (-1): give it the next id. Pre-assigned
    // (updateItemId() restore path): preserve it and advance the registry's counter past it --
    // mirrors Scene::addItem() exactly.
    if (element->id() < 0) {
        element->setId(nextId());
    } else {
        m_itemRegistry.setLastId(element->id());
    }
    m_itemRegistry.registerItem(element);

    // File-backed-IC hot-reload registration, mirrors Scene::registerItem()'s identical hook.
    if (auto *ic = qobject_cast<IC *>(element); ic && !ic->file().isEmpty()) {
        m_icRegistry.watchFile(ic->file());
    }
}

void CanvasItem::addItem(Connection *connection)
{
    if (!connection) {
        return;
    }
    if (!m_connectionSet.contains(connection)) {
        m_connections.append(connection);
        m_connectionSet.insert(connection);
    }
    if (connection->id() < 0) {
        connection->setId(nextId());
    } else {
        m_itemRegistry.setLastId(connection->id());
    }
    m_itemRegistry.registerItem(connection);
}

void CanvasItem::removeItem(GraphicElement *element)
{
    if (element) {
        m_elements.removeAll(element);
        m_elementSet.remove(element);
        m_itemRegistry.unregisterItem(element);
        // Prunes the render-state memoization (see m_elementRenderCache's doc comment) so it
        // never grows unbounded across a long editing session -- always safe to call here since
        // removeItem() runs before any real C++ destruction of element.
        m_elementRenderCache.remove(element);
        // Same pruning for spatialIdFor()'s id cache -- otherwise a long add/delete/add churn
        // would grow m_portSpatialIds unbounded even though most of its entries name ports that
        // no longer exist.
        for (auto *port : element->inputs()) {
            m_portSpatialIds.remove(port);
            m_portScenePosCache.remove(port);
        }
        for (auto *port : element->outputs()) {
            m_portSpatialIds.remove(port);
            m_portScenePosCache.remove(port);
        }
    }
}

void CanvasItem::removeItem(Connection *connection)
{
    if (connection) {
        m_connections.removeAll(connection);
        m_connectionSet.remove(connection);
        m_itemRegistry.unregisterItem(connection);
    }
}

QList<GraphicElement *> CanvasItem::selectedElements() const
{
    QList<GraphicElement *> result;
    for (auto *element : m_elements) {
        if (element->isSelected()) {
            result.append(element);
        }
    }
    return result;
}

void CanvasItem::receiveCommand(QUndoCommand *cmd)
{
    m_undoStack.push(cmd);
    update();
}

void CanvasItem::restartSimulation()
{
    m_simulation->restart();
    rebuildSpatialIndex();
    update();
}

void CanvasItem::deleteSelected()
{
    QList<ItemWithId *> items;
    for (auto *elm : selectedElements()) {
        items.append(elm);
    }
    if (items.isEmpty()) {
        return;
    }

    // Clear selection before the command, mirroring Scene::deleteAction() -- and cancel an
    // in-progress drag if it targeted one of the elements being deleted (the WIREDPANDA-H9
    // mid-drag-delete race Scene's own m_dragSnapshot QPointers guard against; this canvas
    // doesn't use QPointers here, so guard explicitly instead).
    for (auto *elm : std::as_const(m_elements)) {
        elm->setSelected(false);
    }
    m_selectedIds.clear();
    for (auto *item : std::as_const(items)) {
        if (auto *elm = dynamic_cast<GraphicElement *>(item); elm && m_dragElements.contains(elm)) {
            m_draggingElement = false;
            m_dragElements.clear();
            m_dragStartPositions.clear();
            break;
        }
    }

    receiveCommand(new CanvasDeleteItemsCommand(items, this));
    emit selectionChanged();
}

SerializationContext CanvasItem::deserializationContext(QHash<quint64, Port *> &portMap,
                                                         const QVersionNumber &version,
                                                         SerializationPurpose purpose)
{
    SerializationContext context{.portMap = portMap, .version = version, .purpose = purpose, .contextDir = m_contextDir};
    context.blobRegistry = &m_icRegistry.blobMapRef();
    return context;
}

void CanvasItem::rotate(int angle)
{
    const auto elements_ = selectedElements();
    if (!elements_.isEmpty()) {
        receiveCommand(new CanvasRotateCommand(elements_, angle, this));
    }
}

void CanvasItem::rotateRight()
{
    rotate(90);
}

void CanvasItem::rotateLeft()
{
    rotate(-90);
}

void CanvasItem::flipHorizontally()
{
    const auto elements_ = selectedElements();
    if (!elements_.isEmpty()) {
        receiveCommand(new CanvasFlipCommand(elements_, 0, this));
    }
}

void CanvasItem::flipVertically()
{
    const auto elements_ = selectedElements();
    if (!elements_.isEmpty()) {
        receiveCommand(new CanvasFlipCommand(elements_, 1, this));
    }
}

void CanvasItem::alignLeft()
{
    const auto elements_ = selectedElements();
    if (elements_.size() < 2) {
        return;
    }

    qreal target = elements_.constFirst()->sceneBoundingRect().left();
    for (auto *elm : elements_) {
        target = std::min(target, elm->sceneBoundingRect().left());
    }

    QList<QPointF> newPositions;
    newPositions.reserve(elements_.size());
    for (auto *elm : elements_) {
        const qreal delta = target - elm->sceneBoundingRect().left();
        newPositions.append(elm->pos() + QPointF(delta, 0));
    }

    moveElementsTo(this, elements_, newPositions);
}

void CanvasItem::alignRight()
{
    const auto elements_ = selectedElements();
    if (elements_.size() < 2) {
        return;
    }

    qreal target = elements_.constFirst()->sceneBoundingRect().right();
    for (auto *elm : elements_) {
        target = std::max(target, elm->sceneBoundingRect().right());
    }

    QList<QPointF> newPositions;
    newPositions.reserve(elements_.size());
    for (auto *elm : elements_) {
        const qreal delta = target - elm->sceneBoundingRect().right();
        newPositions.append(elm->pos() + QPointF(delta, 0));
    }

    moveElementsTo(this, elements_, newPositions);
}

void CanvasItem::alignTop()
{
    const auto elements_ = selectedElements();
    if (elements_.size() < 2) {
        return;
    }

    qreal target = elements_.constFirst()->sceneBoundingRect().top();
    for (auto *elm : elements_) {
        target = std::min(target, elm->sceneBoundingRect().top());
    }

    QList<QPointF> newPositions;
    newPositions.reserve(elements_.size());
    for (auto *elm : elements_) {
        const qreal delta = target - elm->sceneBoundingRect().top();
        newPositions.append(elm->pos() + QPointF(0, delta));
    }

    moveElementsTo(this, elements_, newPositions);
}

void CanvasItem::alignBottom()
{
    const auto elements_ = selectedElements();
    if (elements_.size() < 2) {
        return;
    }

    qreal target = elements_.constFirst()->sceneBoundingRect().bottom();
    for (auto *elm : elements_) {
        target = std::max(target, elm->sceneBoundingRect().bottom());
    }

    QList<QPointF> newPositions;
    newPositions.reserve(elements_.size());
    for (auto *elm : elements_) {
        const qreal delta = target - elm->sceneBoundingRect().bottom();
        newPositions.append(elm->pos() + QPointF(0, delta));
    }

    moveElementsTo(this, elements_, newPositions);
}

void CanvasItem::alignHorizontalCenter()
{
    const auto elements_ = selectedElements();
    if (elements_.size() < 2) {
        return;
    }

    qreal sum = 0;
    for (auto *elm : elements_) {
        sum += elm->sceneBoundingRect().center().x();
    }
    const qreal target = sum / static_cast<qreal>(elements_.size());

    QList<QPointF> newPositions;
    newPositions.reserve(elements_.size());
    for (auto *elm : elements_) {
        const qreal delta = target - elm->sceneBoundingRect().center().x();
        newPositions.append(elm->pos() + QPointF(delta, 0));
    }

    moveElementsTo(this, elements_, newPositions);
}

void CanvasItem::alignVerticalCenter()
{
    const auto elements_ = selectedElements();
    if (elements_.size() < 2) {
        return;
    }

    qreal sum = 0;
    for (auto *elm : elements_) {
        sum += elm->sceneBoundingRect().center().y();
    }
    const qreal target = sum / static_cast<qreal>(elements_.size());

    QList<QPointF> newPositions;
    newPositions.reserve(elements_.size());
    for (auto *elm : elements_) {
        const qreal delta = target - elm->sceneBoundingRect().center().y();
        newPositions.append(elm->pos() + QPointF(0, delta));
    }

    moveElementsTo(this, elements_, newPositions);
}

void CanvasItem::distributeHorizontally()
{
    auto elements_ = selectedElements();
    if (elements_.size() < 3) {
        return;
    }

    std::sort(elements_.begin(), elements_.end(), [](GraphicElement *a, GraphicElement *b) {
        return a->sceneBoundingRect().left() < b->sceneBoundingRect().left();
    });

    const qreal spanStart = elements_.constFirst()->sceneBoundingRect().left();
    const qreal spanEnd = elements_.constLast()->sceneBoundingRect().right();

    qreal totalWidth = 0;
    for (auto *elm : elements_) {
        totalWidth += elm->sceneBoundingRect().width();
    }
    const qreal gap = (spanEnd - spanStart - totalWidth) / static_cast<qreal>(elements_.size() - 1);

    QList<QPointF> newPositions;
    newPositions.reserve(elements_.size());
    newPositions.append(elements_.constFirst()->pos()); // leftmost stays fixed as an anchor

    qreal cursor = elements_.constFirst()->sceneBoundingRect().right() + gap;
    for (int i = 1; i < elements_.size() - 1; ++i) {
        auto *elm = elements_.at(i);
        const qreal delta = cursor - elm->sceneBoundingRect().left();
        newPositions.append(elm->pos() + QPointF(delta, 0));
        cursor += elm->sceneBoundingRect().width() + gap;
    }

    newPositions.append(elements_.constLast()->pos()); // rightmost stays fixed as an anchor

    moveElementsTo(this, elements_, newPositions);
}

void CanvasItem::distributeVertically()
{
    auto elements_ = selectedElements();
    if (elements_.size() < 3) {
        return;
    }

    std::sort(elements_.begin(), elements_.end(), [](GraphicElement *a, GraphicElement *b) {
        return a->sceneBoundingRect().top() < b->sceneBoundingRect().top();
    });

    const qreal spanStart = elements_.constFirst()->sceneBoundingRect().top();
    const qreal spanEnd = elements_.constLast()->sceneBoundingRect().bottom();

    qreal totalHeight = 0;
    for (auto *elm : elements_) {
        totalHeight += elm->sceneBoundingRect().height();
    }
    const qreal gap = (spanEnd - spanStart - totalHeight) / static_cast<qreal>(elements_.size() - 1);

    QList<QPointF> newPositions;
    newPositions.reserve(elements_.size());
    newPositions.append(elements_.constFirst()->pos()); // topmost stays fixed as an anchor

    qreal cursor = elements_.constFirst()->sceneBoundingRect().bottom() + gap;
    for (int i = 1; i < elements_.size() - 1; ++i) {
        auto *elm = elements_.at(i);
        const qreal delta = cursor - elm->sceneBoundingRect().top();
        newPositions.append(elm->pos() + QPointF(0, delta));
        cursor += elm->sceneBoundingRect().height() + gap;
    }

    newPositions.append(elements_.constLast()->pos()); // bottommost stays fixed as an anchor

    moveElementsTo(this, elements_, newPositions);
}

void CanvasItem::nextElm()
{
    const auto selected = selectedElements();
    const bool needsMacro = selected.size() > 1;
    if (needsMacro) {
        m_undoStack.beginMacro(QStringLiteral("Morph elements"));
    }

    for (auto *element : selected) {
        const auto nextType = Enums::nextElmType(element->elementType());
        if (nextType == ElementType::Unknown) {
            continue; // no "next" in the cycle for this type
        }
        const int id = element->id();
        receiveCommand(new CanvasMorphCommand(QList<GraphicElement *>{element}, nextType, this));
        // CanvasMorphCommand replaces the element in-place under the same id -- the old
        // pointer is invalid after the command's redo(), re-resolve by id.
        if (auto *morphed = dynamic_cast<GraphicElement *>(itemById(id))) {
            morphed->setSelected(true);
        }
    }

    if (needsMacro) {
        m_undoStack.endMacro();
    }
    emit selectionChanged();
}

void CanvasItem::prevElm()
{
    const auto selected = selectedElements();
    const bool needsMacro = selected.size() > 1;
    if (needsMacro) {
        m_undoStack.beginMacro(QStringLiteral("Morph elements"));
    }

    for (auto *element : selected) {
        const auto prevType = Enums::prevElmType(element->elementType());
        if (prevType == ElementType::Unknown) {
            continue;
        }
        const int id = element->id();
        receiveCommand(new CanvasMorphCommand(QList<GraphicElement *>{element}, prevType, this));
        if (auto *morphed = dynamic_cast<GraphicElement *>(itemById(id))) {
            morphed->setSelected(true);
        }
    }

    if (needsMacro) {
        m_undoStack.endMacro();
    }
    emit selectionChanged();
}

void CanvasItem::applyPropertyWithUndo(GraphicElement *element, const std::function<void()> &mutate)
{
    QByteArray oldData;
    {
        QDataStream stream(&oldData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(stream);
        element->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});
    }
    mutate();
    receiveCommand(new CanvasUpdateCommand({element}, oldData, this));
}

void CanvasItem::adjustMainProperty(int dir)
{
    const auto selected = selectedElements();
    const bool needsMacro = selected.size() > 1;
    if (needsMacro) {
        m_undoStack.beginMacro(QStringLiteral("Cycle element properties"));
    }

    for (auto *element : selected) {
        switch (element->elementType()) {
        case ElementType::And:
        case ElementType::Or:
        case ElementType::Nand:
        case ElementType::Nor:
        case ElementType::Xor:
        case ElementType::Xnor:
        case ElementType::Led:
        case ElementType::TruthTable: {
            const int newSize = element->inputSize() + dir;
            if (newSize >= element->minInputSize() && newSize <= element->maxInputSize()) {
                receiveCommand(new CanvasChangePortSizeCommand({element}, newSize, this, true));
            }
            break;
        }
        case ElementType::InputRotary: {
            const int newSize = element->outputSize() + dir;
            if (newSize >= element->minOutputSize() && newSize <= element->maxOutputSize()) {
                receiveCommand(new CanvasChangePortSizeCommand({element}, newSize, this, false));
            }
            break;
        }
        case ElementType::Clock:
            if (element->hasFrequency()) {
                applyPropertyWithUndo(element, [element, dir] { element->setFrequency(element->frequency() + dir * 0.5); });
            }
            break;
        case ElementType::Buzzer:
            if (element->hasFrequency()) {
                applyPropertyWithUndo(element, [element, dir] {
                    element->setFrequency(std::clamp(element->frequency() + dir * 100.0, 20.0, 20000.0));
                });
            }
            break;
        case ElementType::Display16:
            if (dir < 0 && element->hasColors()) {
                applyPropertyWithUndo(element, [element] { element->setColor(element->previousColor()); });
            }
            break;
        case ElementType::Display14:
        case ElementType::Display7:
            if (element->hasColors()) {
                applyPropertyWithUndo(element, [element, dir] {
                    element->setColor(dir < 0 ? element->previousColor() : element->nextColor());
                });
            }
            break;
        case ElementType::AudioBox:
        case ElementType::DFlipFlop:
        case ElementType::DLatch:
        case ElementType::Demux:
        case ElementType::IC:
        case ElementType::InputButton:
        case ElementType::InputGnd:
        case ElementType::InputSwitch:
        case ElementType::InputVcc:
        case ElementType::JKFlipFlop:
        case ElementType::JKLatch:
        case ElementType::Line:
        case ElementType::Mux:
        case ElementType::Node:
        case ElementType::Not:
        case ElementType::SRFlipFlop:
        case ElementType::SRLatch:
        case ElementType::TFlipFlop:
        case ElementType::Text:
        case ElementType::Unknown:
            break;
        }

        // Toggling selection off and on forces a future property inspector to refresh.
        element->setSelected(false);
        element->setSelected(true);
    }

    if (needsMacro) {
        m_undoStack.endMacro();
    }
    emit selectionChanged();
}

void CanvasItem::prevMainPropShortcut() { adjustMainProperty(-1); }
void CanvasItem::nextMainPropShortcut() { adjustMainProperty(1); }

void CanvasItem::adjustSecondaryProperty(int dir)
{
    const auto selected = selectedElements();
    const bool needsMacro = selected.size() > 1;
    if (needsMacro) {
        m_undoStack.beginMacro(QStringLiteral("Cycle element properties"));
    }

    for (auto *element : selected) {
        switch (element->elementType()) {
        case ElementType::TruthTable: {
            const int newSize = element->outputSize() + dir;
            if (newSize >= element->minOutputSize() && newSize <= element->maxOutputSize()) {
                receiveCommand(new CanvasChangePortSizeCommand({element}, newSize, this, false));
            }
            break;
        }
        case ElementType::Led:
            if (element->hasColors()) {
                applyPropertyWithUndo(element, [element, dir] {
                    element->setColor(dir < 0 ? element->previousColor() : element->nextColor());
                });
            }
            break;
        case ElementType::And:
        case ElementType::AudioBox:
        case ElementType::Buzzer:
        case ElementType::Clock:
        case ElementType::DFlipFlop:
        case ElementType::DLatch:
        case ElementType::Demux:
        case ElementType::Display14:
        case ElementType::Display16:
        case ElementType::Display7:
        case ElementType::IC:
        case ElementType::InputButton:
        case ElementType::InputGnd:
        case ElementType::InputRotary:
        case ElementType::InputSwitch:
        case ElementType::InputVcc:
        case ElementType::JKFlipFlop:
        case ElementType::JKLatch:
        case ElementType::Line:
        case ElementType::Mux:
        case ElementType::Nand:
        case ElementType::Node:
        case ElementType::Nor:
        case ElementType::Not:
        case ElementType::Or:
        case ElementType::SRFlipFlop:
        case ElementType::SRLatch:
        case ElementType::TFlipFlop:
        case ElementType::Text:
        case ElementType::Unknown:
        case ElementType::Xnor:
        case ElementType::Xor:
            break;
        }

        element->setSelected(false);
        element->setSelected(true);
    }

    if (needsMacro) {
        m_undoStack.endMacro();
    }
    emit selectionChanged();
}

void CanvasItem::prevSecndPropShortcut() { adjustSecondaryProperty(-1); }
void CanvasItem::nextSecndPropShortcut() { adjustSecondaryProperty(1); }

namespace {

/// Bundles every blob referenced by an embedded IC in \a elements into \a mimeData, under both
/// MimeType::BlobRegistry (legacy, unversioned) and MimeType::BlobRegistryV2 (Qt_5_12-versioned,
/// preferred on paste) -- mirrors ClipboardManager::copy()/cut()'s identical dual-format
/// bundling exactly, so a clipboard round trip stays interoperable with the Widgets app during
/// the migration (either side's clipboard payload can be pasted into the other's).
void bundleUsedBlobs(CanvasItem *canvas, const QList<GraphicElement *> &elements, QMimeData *mimeData)
{
    QMap<QString, QByteArray> usedBlobs;
    for (auto *elm : elements) {
        if (elm->isEmbedded() && !elm->blobName().isEmpty()) {
            const QString &name = elm->blobName();
            if (!usedBlobs.contains(name) && canvas->icRegistry()->hasBlob(name)) {
                usedBlobs[name] = canvas->icRegistry()->blob(name);
            }
        }
    }
    if (usedBlobs.isEmpty()) {
        return;
    }

    QByteArray regBytes;
    QDataStream regStream(&regBytes, QIODevice::WriteOnly);
    regStream << usedBlobs;
    mimeData->setData(MimeType::BlobRegistry, regBytes);

    QByteArray regBytesV2;
    QDataStream regStreamV2(&regBytesV2, QIODevice::WriteOnly);
    regStreamV2.setVersion(QDataStream::Qt_5_12);
    regStreamV2 << usedBlobs;
    mimeData->setData(MimeType::BlobRegistryV2, regBytesV2);
}

} // namespace

void CanvasItem::copyAction()
{
    const auto selected = selectedElements();
    QList<ItemWithId *> items;
    for (auto *elm : selected) {
        items.append(elm);
    }
    if (items.isEmpty()) {
        QGuiApplication::clipboard()->clear();
        return;
    }

    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    CanvasCommandUtils::serializeItems(items, stream);

    auto *mimeData = new QMimeData();
    mimeData->setData(MimeType::Clipboard, itemData);
    bundleUsedBlobs(this, selected, mimeData);
    QGuiApplication::clipboard()->setMimeData(mimeData);
}

void CanvasItem::cutAction()
{
    const auto selected = selectedElements();
    QList<ItemWithId *> items;
    for (auto *elm : selected) {
        items.append(elm);
    }
    if (items.isEmpty()) {
        QGuiApplication::clipboard()->clear();
        return;
    }

    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    CanvasCommandUtils::serializeItems(items, stream);

    auto *mimeData = new QMimeData();
    mimeData->setData(MimeType::Clipboard, itemData);
    bundleUsedBlobs(this, selected, mimeData);

    deleteSelected();

    QGuiApplication::clipboard()->setMimeData(mimeData);
}

void CanvasItem::pasteAction()
{
    const auto *mimeData = QGuiApplication::clipboard()->mimeData();
    if (!mimeData) {
        return;
    }

    // Import blob registry from clipboard so cross-tab (or cross-window, cross-app-instance --
    // the OS clipboard doesn't care) paste of embedded ICs works, mirroring
    // ClipboardManager::paste()'s identical import. Prefer BlobRegistryV2; fall back to the
    // legacy unversioned BlobRegistry a Widgets-side copy on an older version might have set.
    QMap<QString, QByteArray> clipboardBlobs;
    if (mimeData->hasFormat(MimeType::BlobRegistryV2)) {
        QByteArray regBytes = mimeData->data(MimeType::BlobRegistryV2);
        QDataStream regStream(&regBytes, QIODevice::ReadOnly);
        regStream.setVersion(QDataStream::Qt_5_12);
        try { clipboardBlobs = Serialization::readBoundedBlobMap(regStream); } catch (...) {}
    } else if (mimeData->hasFormat(MimeType::BlobRegistry)) {
        QByteArray regBytes = mimeData->data(MimeType::BlobRegistry);
        QDataStream regStream(&regBytes, QIODevice::ReadOnly);
        try { clipboardBlobs = Serialization::readBoundedBlobMap(regStream); } catch (...) {}
    }

    QByteArray itemData;
    if (mimeData->hasFormat(MimeType::ClipboardLegacy)) {
        itemData = mimeData->data(MimeType::ClipboardLegacy);
    }
    if (mimeData->hasFormat(MimeType::Clipboard)) {
        itemData = mimeData->data(MimeType::Clipboard);
    }
    if (itemData.isEmpty() && clipboardBlobs.isEmpty()) {
        return;
    }

    // Register any new embedded-IC blobs and add the pasted items as a single undo step --
    // registering the blobs untracked would leave them orphaned in the registry forever after
    // an undo, since CanvasAddItemsCommand only knows about canvas items, not the blob registry.
    const bool needsMacro = !clipboardBlobs.isEmpty() && !itemData.isEmpty();
    if (needsMacro) {
        m_undoStack.beginMacro(tr("Paste"));
    }

    if (!clipboardBlobs.isEmpty()) {
        for (auto it = clipboardBlobs.cbegin(); it != clipboardBlobs.cend(); ++it) {
            if (!icRegistry()->hasBlob(it.key())) {
                receiveCommand(new CanvasRegisterBlobCommand(it.key(), it.value(), this));
            }
        }
    }

    if (!itemData.isEmpty()) {
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        QVersionNumber version = Serialization::readPandaHeader(stream);
        deserializeAndAdd(stream, version);
    }

    if (needsMacro) {
        m_undoStack.endMacro();
    }
}

void CanvasItem::duplicateAction()
{
    QList<ItemWithId *> items;
    for (auto *elm : selectedElements()) {
        items.append(elm);
    }
    if (items.isEmpty()) {
        return;
    }

    // Serialize to a private buffer so the system clipboard is left untouched.
    QByteArray itemData;
    QDataStream writeStream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(writeStream);
    CanvasCommandUtils::serializeItems(items, writeStream);

    QDataStream readStream(&itemData, QIODevice::ReadOnly);
    const QVersionNumber version = Serialization::readPandaHeader(readStream);
    const QPointF step(Constants::gridSize, Constants::gridSize);
    const auto added = deserializeAndAdd(readStream, version, step);

    for (auto *item : added) {
        if (auto *elm = dynamic_cast<GraphicElement *>(item)) {
            elm->setSelected(true);
        }
    }
    emit selectionChanged();
}

void CanvasItem::addElementFromPalette(ElementType type, const QString &icFileName, bool isEmbedded, const QPointF &screenPos)
{
    const QPointF pos = screenToWorld(screenPos);

    std::unique_ptr<GraphicElement> element(ElementFactory::buildElement(type));

    if (isEmbedded && type == ElementType::IC) {
        if (!icRegistry()->initEmbeddedIC(static_cast<IC *>(element.get()), icFileName)) {
            return;
        }
    } else {
        // A no-op for every type but IC (GraphicElement::loadFromDrop() is a bare no-op base
        // implementation) -- loadFromDrop can throw on a malformed IC file; the unique_ptr
        // above keeps the freshly-allocated element from leaking when it does.
        element->loadFromDrop(icFileName, contextDir());
    }

    for (auto *elm : std::as_const(m_elements)) {
        elm->setSelected(false);
    }
    m_selectedIds.clear();

    auto *raw = element.release();
    receiveCommand(new CanvasAddItemsCommand({raw}, this));
    raw->setSelected(true);
    raw->setPos(pos);
    rebuildSpatialIndex();
    emit selectionChanged();
}

void CanvasItem::addTourDemoInputSwitch()
{
    for (auto *elm : std::as_const(m_elements)) {
        elm->setSelected(false);
    }
    m_selectedIds.clear();

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    receiveCommand(new CanvasAddItemsCommand({sw}, this));
    sw->setSelected(true);
    sw->setPos(0, 0);
    rebuildSpatialIndex();
    emit selectionChanged();
}

void CanvasItem::addTourDemoWaveformCircuit()
{
    for (auto *elm : std::as_const(m_elements)) {
        elm->setSelected(false);
    }
    m_selectedIds.clear();

    auto *clock1 = ElementFactory::buildElement(ElementType::Clock);
    auto *clock2 = ElementFactory::buildElement(ElementType::Clock);
    auto *gate = ElementFactory::buildElement(ElementType::And);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    clock1->setPos(-160, -60);
    clock2->setPos(-160, 60);
    gate->setPos(0, 0);
    led->setPos(160, 0);

    auto *conn1 = new Connection();
    conn1->setStartPort(clock1->outputPort(0));
    conn1->setEndPort(gate->inputPort(0));
    auto *conn2 = new Connection();
    conn2->setStartPort(clock2->outputPort(0));
    conn2->setEndPort(gate->inputPort(1));
    auto *conn3 = new Connection();
    conn3->setStartPort(gate->outputPort(0));
    conn3->setEndPort(led->inputPort(0));

    // CanvasAddItemsCommand auto-discovers conn1..conn3 via port traversal (loadList(), same
    // mechanism as addElementFromPalette()'s own single-item add) and registers all 7 items with
    // the canvas's Simulation -- mirrors MainWindow::runTourDemoAction()'s identical single-call
    // AddItemsCommand shape, not three separate connection-only commands.
    receiveCommand(new CanvasAddItemsCommand({clock1, clock2, gate, led}, this));

    conn1->updatePath();
    conn2->updatePath();
    conn3->updatePath();

    rebuildSpatialIndex();
    emit selectionChanged();
}

QList<ItemWithId *> CanvasItem::deserializeAndAdd(QDataStream &stream, const QVersionNumber &version,
                                                  std::optional<QPointF> fixedOffset)
{
    for (auto *elm : std::as_const(m_elements)) {
        elm->setSelected(false);
    }
    m_selectedIds.clear();

    QPointF center;
    stream >> center;

    QHash<quint64, Port *> portMap;
    auto context = deserializationContext(portMap, version, SerializationPurpose::InMemorySnapshot);
    const auto itemList = Serialization::deserialize(stream, context);

    // Paste: shift elements so their centroid lands at the cursor, then nudge 32px diagonally
    // so repeated pastes don't completely overlap. Duplicate: shift by exactly fixedOffset
    // from the originals so the copies sit a grid step down-right, in place.
    const QPointF offset = fixedOffset ? *fixedOffset : (mousePos() - center - QPointF(32.0, 32.0));

    receiveCommand(new CanvasAddItemsCommand(itemList, this));

    for (auto *item : itemList) {
        if (auto *elm = dynamic_cast<GraphicElement *>(item)) {
            elm->setPos(elm->pos() + offset);
        }
    }
    rebuildSpatialIndex();
    emit selectionChanged();

    return itemList;
}

void CanvasItem::mute(bool mute)
{
    for (auto *element : std::as_const(m_elements)) {
        if (auto *audioElement = qobject_cast<AudioOutputElement *>(element)) {
            audioElement->mute(mute);
        }
    }
}

void CanvasItem::selectAll()
{
    for (auto *element : std::as_const(m_elements)) {
        element->setSelected(true);
        m_selectedIds.insert(elementId(element->id()));
    }
    emit selectionChanged();
}

void CanvasItem::selectOnly(GraphicElement *element)
{
    for (auto *elm : std::as_const(m_elements)) {
        elm->setSelected(elm == element);
    }
    m_selectedIds.clear();
    if (element) {
        m_selectedIds.insert(elementId(element->id()));
    }
    emit selectionChanged();
}

void CanvasItem::commitInlineLabelEdit(GraphicElement *element, const QString &newLabel)
{
    if (!element || newLabel == element->label()) {
        return;
    }

    QByteArray oldData;
    QDataStream stream(&oldData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    element->save(stream, {.purpose = SerializationPurpose::InMemorySnapshot});

    element->setLabel(newLabel);
    receiveCommand(new CanvasUpdateCommand({element}, oldData, this));
}

void CanvasItem::morphSelectionTo(ElementType type)
{
    const auto selected = selectedElements();
    if (selected.isEmpty()) {
        return;
    }
    receiveCommand(new CanvasMorphCommand(selected, type, this));
}

QImage CanvasItem::renderICPreviewImage(GraphicElement *ic) const
{
    auto *icElement = qobject_cast<IC *>(ic);
    if (!icElement) {
        return {};
    }

    const auto &internal = icElement->internalElements();
    // Mirrors ICPreviewPopup::MaxElementCount -- ICPreviewPopup itself is Widgets-only, not
    // includable here, so the same real limit is duplicated as a local constant.
    constexpr qsizetype kMaxElementCount = 500;
    if (internal.isEmpty() || internal.size() > kMaxElementCount) {
        return {};
    }

    QRectF bounds;
    for (auto *element : internal) {
        bounds |= element->boundingRect().translated(element->pos());
    }
    // Padding matches ICRenderer::generatePreviewPixmap()'s own -16..+16.
    bounds = bounds.adjusted(-16, -16, 16, 16);
    // Defense-in-depth, mirroring generatePreviewPixmap()'s identical guard: a non-finite
    // element position/rotation makes this NaN, and QSizeF::toSize() asserts on that.
    if (!std::isfinite(bounds.width()) || !std::isfinite(bounds.height())) {
        return {};
    }

    // Mirrors ICPreviewPopup::MaxWidth/MaxHeight.
    constexpr int kMaxWidth = 500;
    constexpr int kMaxHeight = 350;
    QSize targetSize = bounds.size().toSize();
    targetSize.scale(kMaxWidth, kMaxHeight, Qt::KeepAspectRatio);
    if (targetSize.isEmpty()) {
        return {};
    }

    QImage image(targetSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(42, 42, 42));

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    const qreal scale = std::min(static_cast<qreal>(targetSize.width()) / bounds.width(),
                                 static_cast<qreal>(targetSize.height()) / bounds.height());
    painter.scale(scale, scale);
    painter.translate(-bounds.topLeft());

    // Each element's paint() draws in its own local coordinate system; translating by pos()
    // alone (not boundingRect().topLeft()) lands it correctly here, mirroring how this class's
    // own gate-rendering block's tile-placement math resolves to the same "world position ==
    // local paint coordinate + pos()" relationship (see updatePaintNode()'s gate-rendering
    // block for the single-element, tile-packed version of the same technique).
    for (auto *element : internal) {
        painter.save();
        painter.translate(element->pos());
        element->paint(&painter);
        painter.restore();
    }

    return image;
}

QRectF CanvasItem::elementsBoundingRect() const
{
    QRectF bounds;
    for (auto *element : m_elements) {
        bounds |= element->boundingRect().translated(element->pos());
    }
    for (auto *connection : m_connections) {
        // Connection's path is built directly in canvas coordinates (Connection::updatePath()
        // moveTo()s m_startPos/m_endPos, real port scene positions) -- pos() is never set away
        // from its (0, 0) default, unlike GraphicElement, so boundingRect() alone is already
        // canvas-space; no translate needed here or in paintElementsInto() below.
        bounds |= connection->boundingRect();
    }
    return bounds;
}

void CanvasItem::applyContentFitTransform(QPainter *painter, const QRectF &target, const QRectF &source) const
{
    const qreal scale = std::min(target.width() / source.width(), target.height() / source.height());
    const QSizeF scaledSize = source.size() * scale;
    const QPointF offset = target.topLeft()
        + QPointF((target.width() - scaledSize.width()) / 2.0, (target.height() - scaledSize.height()) / 2.0);

    painter->translate(offset);
    painter->scale(scale, scale);
    painter->translate(-source.topLeft());
}

void CanvasItem::paintElementsInto(QPainter *painter, const QRectF &target, const QRectF &source) const
{
    if (target.isEmpty() || source.isEmpty()) {
        return;
    }

    painter->save();
    applyContentFitTransform(painter, target, source);

    // World position == local paint coordinate + pos(), same relationship
    // renderICPreviewImage()'s own comment documents.
    for (auto *element : m_elements) {
        painter->save();
        painter->translate(element->pos());
        element->paint(painter);
        painter->restore();
    }
    for (auto *connection : m_connections) {
        connection->paint(painter);
    }

    painter->restore();
}

void CanvasItem::paintElementsSimplifiedInto(QPainter *painter, const QRectF &target, const QRectF &source) const
{
    if (target.isEmpty() || source.isEmpty()) {
        return;
    }

    painter->save();
    applyContentFitTransform(painter, target, source);
    // Axis-aligned flat rects/lines don't need antialiasing at this scale, and it skips the
    // raster engine's antialiased span-fill path -- this call only, doesn't affect anything
    // else painted before/after it in its own save()/restore() block.
    painter->setRenderHint(QPainter::Antialiasing, false);

    const ThemeAttributes &theme = ThemeManager::attributes();
    for (auto *element : m_elements) {
        painter->fillRect(element->boundingRect().translated(element->pos()), theme.m_minimapElementBrush);
    }

    QPen wirePen(theme.m_connectionInactive);
    wirePen.setWidth(0);
    painter->setPen(wirePen);
    for (auto *connection : m_connections) {
        painter->drawLine(connection->startPort()->scenePos(), connection->endPort()->scenePos());
    }

    painter->restore();
}

QImage CanvasItem::renderExportImage(const QRectF &paddedRect) const
{
    // Mirrors CircuitExporter::renderScaledImage()'s exact cap: scale down only if paddedRect
    // exceeds kMaxImageDimension per side, 1:1 otherwise (see that function's own doc comment
    // for why -- unbounded element positions could otherwise size this proportionally to tens
    // of gigabytes).
    QSizeF targetSize = paddedRect.size();
    const qreal scale = std::min({1.0, kMaxImageDimension / targetSize.width(),
                                   kMaxImageDimension / targetSize.height()});
    if (scale < 1.0) {
        targetSize *= scale;
    }

    // Transparent background, unlike renderICPreviewImage()'s opaque dark fill -- this is an
    // export product, not a popup with its own chrome. Explicit ARGB32_Premultiplied +
    // fill(), not QPixmap: see CircuitExporter::renderScaledImage()'s identical comment on why
    // QPixmap(size) can silently flatten to opaque on some platforms.
    QImage image(targetSize.toSize(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    paintElementsInto(&painter, QRectF(QPointF(), targetSize), paddedRect);

    return image;
}

void CanvasItem::exportToImage(const QString &filePath) const
{
    const QImage image = renderExportImage(elementsBoundingRect().adjusted(-64, -64, 64, 64));
    if (!image.save(filePath)) {
        throw PANDACEPTION("Could not save image to %1.", filePath);
    }
}

void CanvasItem::exportToPdf(const QString &filePath) const
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    // Landscape fits most circuits better than portrait, matching CircuitExporter::renderToPdf().
    printer.setPageOrientation(QPageLayout::Orientation::Landscape);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);

    QPainter painter;
    if (!painter.begin(&printer)) {
        throw PANDACEPTION("Could not print this circuit to PDF.");
    }

    // painter.viewport() after begin() is the printer's full device rect in the painter's own
    // coordinate system -- exactly what QGraphicsScene::render(painter, QRectF(), source) uses
    // internally when its target argument is empty (CircuitExporter::renderToPdf()'s own call
    // shape), so this reproduces that behavior without needing QGraphicsScene::render() itself.
    paintElementsInto(&painter, QRectF(painter.viewport()), elementsBoundingRect().adjusted(-64, -64, 64, 64));
    painter.end();
}

void CanvasItem::clearSelection()
{
    for (auto *elm : std::as_const(m_elements)) {
        elm->setSelected(false);
    }
    m_selectedIds.clear();
    emit selectionChanged();
}

qreal CanvasItem::zoomScale() const
{
    // 1.25/0.8 are exact reciprocals, matching GraphicsView::zoomIn()/zoomOut()'s own comment:
    // a zoom in then out returns to exactly the original scale without floating-point drift.
    return std::pow(1.25, m_zoomLevel);
}

QPointF CanvasItem::screenToWorld(const QPointF &screenPt) const
{
    return screenPt / zoomScale() + m_panOffset;
}

QPointF CanvasItem::worldToScreen(const QPointF &worldPt) const
{
    return (worldPt - m_panOffset) * zoomScale();
}

bool CanvasItem::canZoomIn() const
{
    return m_zoomLevel < kMaxZoomLevel;
}

bool CanvasItem::canZoomOut() const
{
    return m_zoomLevel > kMinZoomLevel;
}

void CanvasItem::zoomIn(std::optional<QPointF> screenAnchor)
{
    if (!canZoomIn()) {
        return;
    }

    // Anchor defaults to this item's own center, mirroring GraphicsView::AnchorUnderMouse's
    // fallback behavior (the last-known mouse position if the wheel hasn't moved recently is
    // Qt's own internal concern; a menu-triggered zoom has no mouse position to anchor on at
    // all, so the visible center is the closest equivalent to "zoom into what's on screen").
    const QPointF anchor = screenAnchor.value_or(QPointF(width() / 2.0, height() / 2.0));
    const QPointF worldAnchor = screenToWorld(anchor);

    ++m_zoomLevel;

    // Re-solve panOffset so worldAnchor keeps mapping to the same screen point post-zoom --
    // mirrors GraphicsView::wheelEvent()'s centerOn(mapToScene(...)) correction, done directly
    // via the same worldToScreen()/screenToWorld() relationship instead of a separate
    // scroll-and-recenter step.
    m_panOffset = worldAnchor - anchor / zoomScale();

    update();
    emit zoomChanged();
}

void CanvasItem::zoomOut(std::optional<QPointF> screenAnchor)
{
    if (!canZoomOut()) {
        return;
    }

    const QPointF anchor = screenAnchor.value_or(QPointF(width() / 2.0, height() / 2.0));
    const QPointF worldAnchor = screenToWorld(anchor);

    --m_zoomLevel;

    m_panOffset = worldAnchor - anchor / zoomScale();

    update();
    emit zoomChanged();
}

void CanvasItem::resetZoom()
{
    m_zoomLevel = 0;
    m_panOffset = QPointF();
    update();
    emit zoomChanged();
}

void CanvasItem::zoomToFit()
{
    // Fit the current selection if there is one (zoom-to-selection); otherwise the whole
    // circuit -- mirrors GraphicsView::zoomToFit() exactly, including its own 16px padding.
    QRectF target;
    const auto selected = selectedElements();
    if (!selected.isEmpty()) {
        for (auto *element : selected) {
            target |= element->boundingRect().translated(element->pos());
        }
    } else {
        target = elementsBoundingRect();
    }

    if (!target.isValid() || target.isEmpty() || width() <= 0.0 || height() <= 0.0) {
        return;
    }

    target.adjust(-16.0, -16.0, 16.0, 16.0);

    // fitInView's ideal continuous scale, snapped DOWN to the nearest discrete zoom step so the
    // whole target still fits (floor never overshoots the viewport) -- mirrors
    // GraphicsView::zoomToFit()'s identical fitScale/std::floor(std::log(...)/std::log(1.25))
    // snapping, computed directly here since this canvas has no QGraphicsView::fitInView() to
    // call.
    const qreal fitScale = std::min(width() / target.width(), height() / target.height());
    const int level = std::clamp(static_cast<int>(std::floor(std::log(fitScale) / std::log(1.25))), kMinZoomLevel, kMaxZoomLevel);

    m_zoomLevel = level;
    // Center target in the viewport at the new scale: the world point at target's center must
    // map to the screen point at this item's own center.
    const QPointF screenCenter(width() / 2.0, height() / 2.0);
    m_panOffset = target.center() - screenCenter / zoomScale();

    update();
    emit zoomChanged();
}

void CanvasItem::centerOn(const QPointF &worldPoint)
{
    // The world point at this item's own center must map to worldPoint, at the current zoom
    // level -- same relationship zoomToFit() solves for target.center(), just without also
    // changing m_zoomLevel.
    const QPointF screenCenter(width() / 2.0, height() / 2.0);
    m_panOffset = worldPoint - screenCenter / zoomScale();
    update();
    emit zoomChanged();
}

QRectF CanvasItem::visibleWorldRect() const
{
    return QRectF(screenToWorld(QPointF(0.0, 0.0)), screenToWorld(QPointF(width(), height())));
}

QRectF CanvasItem::scrollableWorldRect() const
{
    // elementsBoundingRect() on an empty canvas is QRectF() -- a real, zero-size rect *at the
    // origin*, not a sentinel "nothing here" value -- so unioning it in unconditionally would
    // permanently anchor the origin inside the scrollable range even after panning far away
    // from it, growing the range asymmetrically depending on which side of the origin the view
    // is currently on. Skipping the union while there's nothing on the canvas keeps the range
    // exactly visibleWorldRect() (plus margin), i.e. it moves cleanly with the view instead.
    const QRectF elements = elementsBoundingRect();
    QRectF content = visibleWorldRect();
    if (!elements.isEmpty()) {
        content = content.united(elements);
    }
    return content.adjusted(-kScrollMargin, -kScrollMargin, kScrollMargin, kScrollMargin);
}

qreal CanvasItem::horizontalScrollPosition() const
{
    const QRectF content = scrollableWorldRect();
    return content.width() > 0.0 ? (m_panOffset.x() - content.left()) / content.width() : 0.0;
}

void CanvasItem::setHorizontalScrollPosition(qreal position)
{
    // visibleWorldRect().topLeft() is exactly m_panOffset (screenToWorld(0, 0) is m_panOffset
    // plus a zero screen-space term) -- setting m_panOffset.x() directly places the requested
    // world x at this item's own local x = 0, which is the ScrollBar-drag scroll-to gesture.
    const QRectF content = scrollableWorldRect();
    m_panOffset.setX(content.left() + position * content.width());
    update();
    emit zoomChanged();
}

qreal CanvasItem::horizontalScrollSize() const
{
    const QRectF content = scrollableWorldRect();
    return content.width() > 0.0 ? std::clamp(visibleWorldRect().width() / content.width(), 0.0, 1.0) : 1.0;
}

qreal CanvasItem::verticalScrollPosition() const
{
    const QRectF content = scrollableWorldRect();
    return content.height() > 0.0 ? (m_panOffset.y() - content.top()) / content.height() : 0.0;
}

void CanvasItem::setVerticalScrollPosition(qreal position)
{
    const QRectF content = scrollableWorldRect();
    m_panOffset.setY(content.top() + position * content.height());
    update();
    emit zoomChanged();
}

qreal CanvasItem::verticalScrollSize() const
{
    const QRectF content = scrollableWorldRect();
    return content.height() > 0.0 ? std::clamp(visibleWorldRect().height() / content.height(), 0.0, 1.0) : 1.0;
}

QRectF CanvasItem::minimapContentRect(qreal targetWidth, qreal targetHeight) const
{
    if (targetWidth <= 0.0 || targetHeight <= 0.0) {
        return {};
    }

    // Mirrors MinimapWidget::computeTransform()'s "never more zoomed-in than the view"
    // reasoning: elementsBoundingRect() alone would tightly crop a freshly-loaded or small
    // circuit; unioning with the live viewport guarantees the viewport-rect overlay this rect
    // feeds never needs to extend past the thumbnail's own bounds.
    const QRectF src = elementsBoundingRect().united(visibleWorldRect());
    if (!src.isValid() || src.isEmpty()) {
        return {};
    }

    QRectF grown = src;
    const qreal targetAspect = targetWidth / targetHeight;
    const qreal srcAspect = src.width() / src.height();
    if (srcAspect < targetAspect) {
        const qreal grow = (src.height() * targetAspect - src.width()) / 2.0;
        grown.adjust(-grow, 0.0, grow, 0.0);
    } else if (srcAspect > targetAspect) {
        const qreal grow = (src.width() / targetAspect - src.height()) / 2.0;
        grown.adjust(0.0, -grow, 0.0, grow);
    }
    return grown;
}

QImage CanvasItem::renderMinimapImage(qreal targetWidth, qreal targetHeight) const
{
    // Cache check: minimapContentRect() unions the circuit's own bounds with the *current*
    // viewport only to guarantee the viewport-rect overlay never has to extend past the
    // thumbnail's own edges (small/empty-circuit case) -- for any circuit big enough for this
    // render's cost to matter, the viewport is already inside the content bounds, so that union
    // is a no-op and the thumbnail genuinely doesn't need to change while panning/zooming within
    // it. Invalidated wholesale by rebuildSpatialIndex(), the same trigger m_portScenePosCache
    // already uses for an identical reason -- see that field's own doc comment.
    const QRectF visible = visibleWorldRect();
    const QSizeF targetSize(targetWidth, targetHeight);
    const bool cacheValid = !m_cachedMinimapContentRect.isEmpty()
        && m_cachedMinimapTargetSize == targetSize
        && m_cachedMinimapContentRect.contains(visible);
    if (cacheValid) {
        return m_cachedMinimapImage;
    }

    const QRectF content = minimapContentRect(targetWidth, targetHeight);
    if (content.isEmpty()) {
        m_cachedMinimapImage = QImage();
        m_cachedMinimapContentRect = QRectF();
        m_cachedMinimapTargetSize = targetSize;
        return m_cachedMinimapImage;
    }
    ++m_minimapRebuildCount;

    QImage image(QSize(qRound(targetWidth), qRound(targetHeight)), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    // content already matches the target aspect ratio exactly (grown above), so this fit is
    // always exact -- no letterboxing offset for the caller to separately track.
    if (m_elements.size() > kMinimapSimplifiedThreshold) {
        paintElementsSimplifiedInto(&painter, QRectF(0.0, 0.0, targetWidth, targetHeight), content);
    } else {
        paintElementsInto(&painter, QRectF(0.0, 0.0, targetWidth, targetHeight), content);
    }

    m_cachedMinimapImage = image;
    m_cachedMinimapContentRect = content;
    m_cachedMinimapTargetSize = targetSize;
    return m_cachedMinimapImage;
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

    // Unwired too: these three exercise the custom-paint families (vector trapezoid/DIP-rect
    // bodies drawn via drawBody()-equivalent methods, offscreen-rendered through the same
    // TextureAtlas as the icon-based elements above) rendering correctly through the atlas --
    // proving the polymorphic paint()-dispatch design generalizes, not just that it works for
    // GraphicElement::paint()'s free-inheritance majority.
    auto *mux = new Mux();
    mux->setPos(40, 460);
    auto *demux = new Demux();
    demux->setPos(220, 460);
    auto *truthTable = new TruthTable();
    truthTable->setPos(400, 460);

    // Also unwired: the segment-compositing family. Their active-segment overlays paint on
    // top of an unchanged base pixmap (see appearanceKeyFor()'s doc comment), so these three
    // also exercise the one extra cache-key dimension this family needs beyond every other
    // ported family so far.
    auto *display7 = new Display7();
    display7->setPos(40, 620);
    auto *display14 = new Display14();
    display14->setPos(220, 620);
    auto *display16 = new Display16();
    display16->setPos(460, 620);

    // InputButton: exercises the momentary press/release path (m_pressedInputButton, see this
    // class's header). InputRotary: click-to-advance-port, via activateOnPress(). Text: the
    // one non-rendering-strategy custom-paint case, a boundingRect() override that includes an
    // empty-state hint child item.
    auto *inputButton = new InputButton();
    inputButton->setPos(40, 720);
    auto *inputRotary = new InputRotary();
    inputRotary->setPos(220, 720);
    auto *text = new Text();
    text->setPos(400, 720);

    // IC: the hardest ported family (body paint + hover-preview signal chain + internal
    // simulation graph, see this class's doc comment). Loads a real example sub-circuit via
    // the already-portable InstallRelativePaths. A missing Examples/ directory in some other
    // run environment degrades to an unloaded (still paints fine, just empty) IC rather than
    // crashing.
    auto *ic = new IC();
    ic->setPos(640, 40);
    const QString examplesDir = InstallRelativePaths::resolve(QStringLiteral("Examples"));
    if (!examplesDir.isEmpty()) {
        try {
            ic->loadFile(QStringLiteral("jkflipflop.panda"), examplesDir);
        } catch (const std::exception &e) {
            qWarning("CanvasItem demo circuit: IC example failed to load (%s) -- rendering unloaded", e.what());
        }
    }

    // addItem() itself appends to m_elements now (see its doc comment) -- the bulk assignment
    // here just fixes the demo circuit's iteration order (id assignment order matches this
    // list, not construction order, which happens to already match); addItem()'s own
    // "already contains" check makes the subsequent per-element addItem() calls idempotent.
    m_elements = { switchA, switchB, andGate, led, switchC, led2,
                    mux, demux, truthTable, display7, display14, display16,
                    inputButton, inputRotary, text, ic };
    for (auto *element : std::as_const(m_elements)) {
        addItem(element);
    }

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
    for (auto *connection : std::as_const(m_connections)) {
        addItem(connection);
    }
    // Host/simulation setup is not here -- see the constructor, which owns it unconditionally
    // now (not just for the buildDemo=true path).
}

void CanvasItem::rebuildSpatialIndex()
{
    m_index.clear();
    m_elementsById.clear();
    m_portsById.clear();
    // Wholesale invalidation of the wire-rendering position cache -- this function runs at
    // every structural change (add/delete/paste/undo/redo/rotate/flip/morph), so this adds no
    // new O(n) event, it only removes stale entries at a point where the index is being
    // rebuilt anyway. Element drag no longer calls this function at all (see
    // updateSpatialIndexFor(), which invalidates only the moved element's own ports) -- see
    // m_portScenePosCache's doc comment.
    m_portScenePosCache.clear();
    // Also invalidates the minimap thumbnail cache (m_cachedMinimapImage/m_cachedMinimapContentRect)
    // for the identical reason as m_portScenePosCache above -- see that field's own doc comment.
    m_cachedMinimapContentRect = QRectF();

    for (auto *element : std::as_const(m_elements)) {
        // A hidden gate/Node isn't hit-testable either -- matches real QGraphicsItem::
        // setVisible(false) semantics (an invisible item receives no mouse events), which is
        // what this toggle replaces.
        if (isElementHidden(element)) {
            continue;
        }

        const quint64 id = elementId(element->id());
        m_index.insertBox(id, element->sceneBoundingRect());
        m_elementsById.insert(id, element);

        // Ports are indexed on top of (not instead of) their owning element -- a click that
        // lands on a port's small glyph should hit the port, not just the element body, so
        // ports must be inserted after their element to win SpatialIndex::queryPoint()'s
        // insertion-order "last wins" priority (see its doc comment).
        const auto indexPort = [&](Port *port) {
            if (isPortHidden(port)) {
                return;
            }
            const quint64 pid = spatialIdFor(port);
            const QRectF portRect = port->boundingRect().translated(port->scenePos());
            m_index.insertBox(pid, portRect);
            m_portsById.insert(pid, port);
        };
        for (auto *port : element->inputs())  { indexPort(port); }
        for (auto *port : element->outputs()) { indexPort(port); }
    }

    if (m_wiresVisible) {
        for (auto *connection : std::as_const(m_connections)) {
            const QPainterPath stroke = strokeShapeFor(connection);
            m_index.insertShape(wireId(connection->id()), stroke.boundingRect(), stroke);
        }
    }
}

quint64 CanvasItem::spatialIdFor(Port *port)
{
    const auto it = m_portSpatialIds.constFind(port);
    if (it != m_portSpatialIds.constEnd()) {
        return *it;
    }
    const quint64 pid = portId(int(m_nextPortSpatialId++));
    m_portSpatialIds.insert(port, pid);
    return pid;
}

void CanvasItem::updateSpatialIndexFor(GraphicElement *element)
{
    if (isElementHidden(element)) {
        return;
    }

    const quint64 id = elementId(element->id());
    // Matches rebuildSpatialIndex()'s own formula exactly (sceneBoundingRect(), not a naive
    // boundingRect().translated(pos())) -- the two must never disagree about a rotated/flipped
    // element's world box, or a drag on a pre-rotated element would leave its hit-test box
    // wrong (too small/misaligned) until some other event forces a full rebuild.
    m_index.insertBox(id, element->sceneBoundingRect());

    const auto indexPort = [&](Port *port) {
        if (isPortHidden(port)) {
            return;
        }
        const quint64 pid = spatialIdFor(port);
        const QRectF portRect = port->boundingRect().translated(port->scenePos());
        m_index.insertBox(pid, portRect);
        m_portsById.insert(pid, port); // idempotent on a cache hit, needed on a genuine first sight
        m_portScenePosCache.remove(port); // must not paint the wire from its pre-move position

        if (m_wiresVisible) {
            for (auto *connection : port->connections()) {
                const QPainterPath stroke = strokeShapeFor(connection);
                m_index.insertShape(wireId(connection->id()), stroke.boundingRect(), stroke);
            }
        }
    };
    for (auto *port : element->inputs())  { indexPort(port); }
    for (auto *port : element->outputs()) { indexPort(port); }
}

void CanvasItem::activateOnPress(GraphicElement *element)
{
    // Both InputSwitch::mousePressEvent() and InputRotary::mousePressEvent() gate their click
    // response on "!m_locked" (isLocked()) -- this dispatcher must carry that same guard, or a
    // locked switch/rotary could still be toggled/advanced by a plain click.
    if (auto *inputSwitch = qobject_cast<InputSwitch *>(element)) {
        if (!inputSwitch->isLocked()) {
            inputSwitch->setOn(!inputSwitch->isOn(), 0);
            m_simulation->wakeSoon();
        }
        return;
    }
    if (auto *inputRotary = qobject_cast<InputRotary *>(element)) {
        // Mirrors InputRotary::mousePressEvent's "setOn(true, (m_currentPort + 1) %
        // outputSize())" -- outputValue() is the public equivalent of the private
        // m_currentPort it reads (InputRotary::outputValue() returns "the index of the
        // currently active output port").
        if (!inputRotary->isLocked()) {
            inputRotary->setOn(true, (inputRotary->outputValue() + 1) % inputRotary->outputSize());
            m_simulation->wakeSoon();
        }
    }
    // InputButton isn't dispatched through here: its momentary on-while-held behavior needs
    // a release counterpart too, so it's handled directly in mousePressEvent()/
    // mouseReleaseEvent() via m_pressedInputButton instead of this press-only dispatcher.
}

QString CanvasItem::appearanceKeyFor(GraphicElement *element) const
{
    // appearanceCacheKey() alone already distinguishes different elements showing different
    // pixmaps (and correctly *shares* a cache entry between elements showing identical ones,
    // e.g. two unrotated And gates) since it tracks the live QPixmap's own identity -- but
    // that's not enough on its own once every element's tile also bakes in its own ports'
    // live-status glyphs (see the atlas-lookup call site): Display7/14/16's segment overlays
    // painting on top of an otherwise-unchanged base pixmap was the first case that needed
    // per-port status in the key, and now every element needs it for the same underlying
    // reason (its ports are part of the same cached tile).
    //
    // QString::number() below, not .arg(): .arg(qreal)/.arg(int) route through
    // QLocaleData::doubleToString()/applyIntegerFormatting() even with no explicit width/locale
    // -- a real, sustained cost once this cache-miss-only builder runs across many continuously-
    // clocking elements. Output is still just an opaque '|'-joined identity key, never
    // user-facing, so this changes nothing else about behavior.
    QString key = QString::number(element->appearanceCacheKey());
    key += QLatin1Char('|');
    key += QString::number(element->rotation());
    key += QLatin1Char('|');
    key += QString::number(element->isFlippedX() ? 1 : 0);
    key += QLatin1Char('|');
    key += QString::number(element->isFlippedY() ? 1 : 0);
    key += QLatin1Char('|');
    key += QString::number(element->isSelected() ? 1 : 0);

    // Every port's live status (inputs then outputs) -- a
    // status change must invalidate the cached tile since the port glyph baked into it
    // (see the atlas-lookup call site) would otherwise go stale. isPortHidden() is per-port
    // (not just the global m_wiresVisible flag) since a Node's ports are never individually
    // hidden -- appending it per port, not once for the whole element, keeps the key correct
    // for both cases without needing to special-case Node here too.
    const auto appendPortKey = [&](Port *port) {
        key += QLatin1Char('|');
        key += QString::number(int(port->status()));
        key += QLatin1Char('|');
        key += QString::number(isPortHidden(port) ? 1 : 0);
    };
    for (auto *port : element->inputs())  { appendPortKey(port); }
    for (auto *port : element->outputs()) { appendPortKey(port); }

    // The label (and, for Text, its empty-state hint -- whose own visibility is entirely
    // derived from whether this same label text is empty, so no separate key dimension is
    // needed for it) is baked into the same tile -- otherwise two elements with identical
    // appearance/rotation/flip/selection/ports but different label text would incorrectly
    // share one cached tile.
    key += QLatin1Char('|');
    key += element->labelItem()->text();

    return key;
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

    if (!isConnectionAllowed(startPort, endPort)) {
        cancelEditedWire();
        return;
    }

    m_editedConnection->setStartPort(startPort);
    m_editedConnection->setEndPort(endPort);
    addItem(m_editedConnection); // appends to m_connections and assigns a real id
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

    removeItem(connection); // removes from m_connections and unregisters its id
    delete connection;
    m_simulation->restart();

    startWireFromOutput(startPort);
}

void CanvasItem::mousePressEvent(QMouseEvent *event)
{
    // Middle-button drag pans the canvas -- mirrors GraphicsView::mousePressEvent(). Space-held
    // left-drag also pans (see m_spacePanHeld's doc comment for why this is a press-time branch
    // here rather than GraphicsView's own mouseMoveEvent-only "m_pan || m_space" check: a
    // deliberate, small cleanup, not a behavior change a user could observe -- either way,
    // space+drag pans the view and nothing else).
    if (event->button() == Qt::MiddleButton || (m_spacePanHeld && event->button() == Qt::LeftButton)) {
        m_panning = true;
        m_panAnchor = event->position();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    // Every other branch below reasons about positions in WORLD (scene) coordinates -- see
    // screenToWorld()/worldToScreen()'s doc comment on this class. m_lastMousePos backs
    // mousePos(), documented as world coordinates (used to place pasted elements relative to
    // where the elements themselves live, not relative to this item's own local pixels).
    const QPointF worldPos = screenToWorld(event->position());
    m_lastMousePos = worldPos;

    if (event->button() == Qt::RightButton) {
        // handleRightClick() takes the SCREEN position -- it needs both: world, for its own
        // hit-testing, and screen, to pass through to elementContextMenuRequested()/
        // emptyContextMenuRequested() unchanged (Main.qml positions the popup menu directly at
        // that canvasHost-local pixel coordinate).
        handleRightClick(event->position());
        return;
    }

    if (event->button() != Qt::LeftButton) {
        return;
    }

    const auto hits = m_index.queryPoint(worldPos);

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
            tryCompleteWire(worldPos);
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
        startSelectionRect(worldPos);
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
        // Ctrl+click(-and-drag) clones the current selection in place, then drags the copies --
        // mirrors Scene::eventFilter() (App/Scene/Scene.cpp), which intercepts Ctrl+Left-click
        // on a GraphicElement/Connection *before* SceneInteraction::mousePress() ever sees it:
        // "item->setSelected(true)" (adds to the existing selection without clearing it, so a
        // Ctrl-click on an already-multi-selected group clones the whole group) followed by
        // ClipboardManager::cloneDrag(). Production's own "ghost" image is deliberately fully
        // transparent (ClipboardManager::buildDragImage() renders at opacity 0.0 -- only the
        // OS's default drag cursor is visible during the drag itself, the clone only becomes
        // visible on drop) -- so driving this canvas's own existing drag-to-move machinery
        // directly on real, live duplicated elements gives the identical net effect (new
        // elements at original + dragDelta, selected, one undoable add) with strictly better
        // visual feedback, not a simplification that drops anything a user could observe. A
        // Ctrl-click with no subsequent movement still leaves a real, undoable duplicate behind
        // (via the add command alone), matching production's own zero-distance case.
        element->setSelected(true);
        m_selectedIds.insert(topHit);

        QList<ItemWithId *> toClone;
        for (auto *selected : selectedElements()) {
            toClone.append(selected);
        }

        QByteArray itemData;
        QDataStream writeStream(&itemData, QIODevice::WriteOnly);
        Serialization::writePandaHeader(writeStream);
        CanvasCommandUtils::serializeItems(toClone, writeStream);

        QDataStream readStream(&itemData, QIODevice::ReadOnly);
        const QVersionNumber version = Serialization::readPandaHeader(readStream);
        // Zero offset: the clone starts exactly overlapping the originals (under the cursor),
        // unlike duplicateAction()'s deliberate one-grid-step offset.
        const auto added = deserializeAndAdd(readStream, version, QPointF());

        m_dragElements.clear();
        m_dragStartPositions.clear();
        for (auto *item : added) {
            // added can include Connections (wires between two cloned elements, per
            // Serialization's usual selection-serialize semantics) alongside GraphicElements.
            if (auto *addedElement = dynamic_cast<GraphicElement *>(item)) {
                addedElement->setSelected(true);
                m_selectedIds.insert(elementId(addedElement->id()));
                m_dragElements.append(addedElement);
                m_dragStartPositions.append(addedElement->pos());
            }
        }
        m_dragAnchor = worldPos;
        m_draggingElement = true;

        emit selectionChanged();
        update();
        return;
    }

    if (event->modifiers().testFlag(Qt::ShiftModifier)) {
        // Shift+click toggles individual membership in the selection, mirroring
        // SceneInteraction::mousePress's "item->setSelected(!item->isSelected())" -- including
        // the real GraphicElement::setSelected() flag, so real paint()'s own selection-outline
        // drawing (ElementAppearance::render()'s selected-state branch) reflects it.
        // Checked/updated via element->isSelected() (the real state selectedElements() reads),
        // not m_selectedIds.contains() -- m_selectedIds is a write-only cache the mouse
        // gestures use for their own delta-tracking that several other call sites
        // (addElementFromPalette(), deserializeAndAdd(), nextElm()/prevElm()) leave stale by
        // calling setSelected(true) without inserting into it.
        // Shift, not Ctrl: production's Scene::eventFilter() remaps Shift+click to Ctrl+click
        // internally before falling through to this exact toggle logic -- Ctrl+click itself is
        // intercepted first, unconditionally, for clone-drag (above), so it never reaches here.
        if (element->isSelected()) {
            m_selectedIds.remove(topHit);
            element->setSelected(false);
        } else {
            m_selectedIds.insert(topHit);
            element->setSelected(true);
        }
        emit selectionChanged();
    } else if (!element->isSelected()) {
        // Plain click on an element outside the current selection. In production this is NOT
        // something SceneInteraction::mousePress() implements itself -- it returns false for a
        // plain element click, and Scene::mousePressEvent() falls through to
        // QGraphicsScene::mousePressEvent(), whose built-in default click-to-select handles it
        // for free. CanvasItem has no QGraphicsScene base to inherit that from, so it needs its
        // own explicit version here. Clicking an element that's already part of the current
        // selection leaves the selection alone (so a drag can move the whole group); only a
        // click outside it collapses the selection to just this element, mirroring
        // QGraphicsScene's own default behavior. Collapses via selectedElements() (real
        // isSelected() state), not m_selectedIds, for the same staleness reason as above.
        for (auto *other : selectedElements()) {
            other->setSelected(false);
        }
        m_selectedIds.clear();
        m_selectedIds.insert(topHit);
        element->setSelected(true);
        emit selectionChanged();
    }

    activateOnPress(element);
    if (auto *inputButton = qobject_cast<InputButton *>(element)) {
        // Momentary: on for as long as the button is held, matching real
        // InputButton::mousePressEvent()/mouseReleaseEvent(). See m_pressedInputButton's doc
        // comment for why release is tracked here rather than re-hit-tested on release.
        //
        // InputButton::mousePressEvent() gates its whole body on "!m_locked" (same class of
        // guard as activateOnPress()'s InputSwitch/InputRotary check above), so a locked button
        // must not be pressed here either -- otherwise it would be tracked for release despite
        // being locked.
        if (!inputButton->isLocked()) {
            inputButton->setOn();
            m_pressedInputButton = inputButton;
            m_simulation->wakeSoon();
        }
    }

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
    m_dragAnchor = worldPos;
    m_draggingElement = true;

    update();
}

void CanvasItem::handleRightClick(const QPointF &screenPos)
{
    // Hit-testing needs world coordinates; the emitted signals keep screenPos as-is (Main.qml
    // positions the popup menu directly at that canvasHost-local pixel coordinate).
    const QPointF worldPos = screenToWorld(screenPos);
    const auto hits = m_index.queryPoint(worldPos);
    if (hits.isEmpty()) {
        emit emptyContextMenuRequested(screenPos);
        return;
    }

    const quint64 topHit = hits.last();
    auto *element = m_elementsById.value(topHit, nullptr);
    if (!element) {
        // A port or wire hit -- and (this canvas has no wire/port selection concept) never
        // already-selected -- mirrors Scene::contextMenu()'s silent no-op for this exact case:
        // an unselected non-GraphicElement item shows no menu at all, not even the empty-canvas
        // one.
        return;
    }

    if (!element->isSelected()) {
        // Right-clicking an unselected element clears the old selection and selects only it.
        for (auto *other : selectedElements()) {
            other->setSelected(false);
        }
        element->setSelected(true);
        m_selectedIds.clear();
        m_selectedIds.insert(topHit);
        emit selectionChanged();
    }

    emit elementContextMenuRequested(element, screenPos);
}

void CanvasItem::mouseMoveEvent(QMouseEvent *event)
{
    if (m_panning) {
        // Mirrors GraphicsView::mouseMoveEvent()'s "m_pan || m_space" branch, adjusting
        // m_panOffset directly instead of scrollbar values -- screen-space delta divided by
        // the current zoom scale, since m_panOffset lives in world units.
        const QPointF screenDelta = event->position() - m_panAnchor;
        m_panOffset -= screenDelta / zoomScale();
        m_panAnchor = event->position();
        update();
        emit zoomChanged();
        return;
    }

    const QPointF worldPos = screenToWorld(event->position());
    m_lastMousePos = worldPos;

    if (m_draggingElement) {
        const QPointF delta = worldPos - m_dragAnchor;
        for (int i = 0; i < m_dragElements.size(); ++i) {
            m_dragElements.at(i)->setPos(m_dragStartPositions.at(i) + delta);
        }
        // Positions changed -- the index must reflect them for hit-testing/wire endpoints (and
        // for the wire batch node, which reads live scenePos() every paint anyway, but the
        // *index* itself is only ever rebuilt explicitly, not derived lazily). Every position
        // was set above before this loop starts, so each updateSpatialIndexFor() call already
        // sees every dragged element's final position regardless of iteration order -- a wire
        // between two co-dragged elements gets re-stroked once per endpoint, a harmless
        // idempotent overwrite, not a correctness issue. A full rebuildSpatialIndex() here would
        // redo this for every element/port/wire on the whole canvas on every mouse-move sample.
        for (auto *element : std::as_const(m_dragElements)) {
            updateSpatialIndexFor(element);
        }
        update();
        return;
    }

    // In-progress wire routing: mirrors SceneInteraction::mouseMove's
    // "if (hasEditedConnection()) { updateEditedEnd(pos); return true; }".
    if (m_editedConnection) {
        updateEditedWireEnd(worldPos);
        update();
        return;
    }

    if (m_markingSelectionBox) {
        updateSelectionRect(worldPos);
        update();
    }
}

void CanvasItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        m_panning = false;
        unsetCursor();
        event->accept();
        return;
    }

    if (event->button() != Qt::LeftButton) {
        return;
    }

    // A space-held pan-drag started on a LeftButton press (see mousePressEvent()) ends here
    // too, without falling through to the click/drag-release logic below -- mirrors
    // GraphicsView's own pan gesture ending cleanly regardless of which button drove it.
    if (m_panning) {
        m_panning = false;
        unsetCursor();
        event->accept();
        return;
    }

    if (m_pressedInputButton) {
        if (auto *inputButton = qobject_cast<InputButton *>(m_pressedInputButton); inputButton && !inputButton->isLocked()) {
            inputButton->setOff();
        }
        m_pressedInputButton = nullptr;
        m_simulation->wakeSoon();
        update();
    }

    if (m_draggingElement) {
        bool moved = false;
        for (int i = 0; i < m_dragElements.size(); ++i) {
            if (m_dragElements.at(i)->pos() != m_dragStartPositions.at(i)) {
                moved = true;
                break;
            }
        }

        // Only push an undo entry if something actually moved -- mirrors
        // SceneInteraction::mouseRelease's "avoids polluting the undo stack with no-op moves
        // (click without drag)". CanvasMoveCommand's constructor captures the new (already-
        // applied) positions itself, mirroring MoveCommand's exact "elements are already at
        // their new positions when pushed" shape.
        if (moved) {
            receiveCommand(new CanvasMoveCommand(m_dragElements, m_dragStartPositions, this));
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
        tryCompleteWire(screenToWorld(event->position()));
    }

    update();
}

void CanvasItem::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        return;
    }

    const QPointF worldPos = screenToWorld(event->position());
    const auto hits = m_index.queryPoint(worldPos);

    // Same port-priority scan as mousePressEvent(): a port at this point means there's no
    // wire-split to do here (mirrors Scene::itemAt()'s port-over-everything priority, which
    // SceneInteraction::mouseDoubleClick() relies on via its single itemAt() call).
    for (auto it = hits.crbegin(); it != hits.crend(); ++it) {
        if (m_portsById.contains(*it)) {
            return;
        }
    }

    if (hits.isEmpty()) {
        return;
    }

    const quint64 topHit = hits.last();
    if (auto *element = m_elementsById.value(topHit, nullptr)) {
        // An element is here, not a wire -- mirrors GraphicElement::mouseDoubleClickEvent()'s
        // "if (hasLabel()) emit inlineEditRequested(this)" (see this method's own class-level
        // doc comment for why that real method isn't called directly). IC is handled separately
        // below, mirroring IC::handleDoubleClick()'s own override instead of the labelable-
        // element path -- an IC opens its sub-circuit tab, it never gets inline label editing.
        if (auto *ic = qobject_cast<IC *>(element)) {
            // Mirrors IC::handleDoubleClick()'s exact pair: cancel any pending hover preview,
            // then request the sub-circuit open (icOpenRequested()'s own doc comment covers the
            // blobName/filePath resolution QuickWorkspaceManager performs on the other end).
            emit icPreviewCancelRequested(element);
            emit icOpenRequested(ic->id(), ic->blobName(), ic->file());
            return;
        }
        if (element->hasLabel()) {
            QRectF targetRect = element->labelSceneBoundingRect();
            if (targetRect.isEmpty()) {
                targetRect = element->sceneBoundingRect();
            }
            // World -> screen: Main.qml positions the inline TextField directly at this rect,
            // in canvasHost-local pixel coordinates (see inlineEditRequested()'s doc comment).
            const QRectF screenRect(worldToScreen(targetRect.topLeft()), targetRect.size() * zoomScale());
            emit inlineEditRequested(element, element->label(), screenRect);
        }
        return;
    }

    // Must be a wire id (SpatialIndex only ever holds element/port/wire ids here).
    const int connId = unwrapId(topHit);
    if (auto *connection = CanvasCommandUtils::findConn(this, connId); connection && connection->startPort() && connection->endPort()) {
        receiveCommand(new CanvasSplitCommand(connection, worldPos, this));
    }
}

void CanvasItem::startSelectionRect(const QPointF &anchor)
{
    m_selectionAnchor = anchor;
    m_markingSelectionBox = true;
    m_selectionRect = QRectF(anchor, anchor);
    // A fresh rubber-band (or a plain click on empty space, which starts and immediately ends
    // one) replaces the previous selection, matching the baseline (no modifier-driven
    // add/subtract) rubber-band behavior -- SceneInteraction's own setSelectionArea() call has
    // the same replace semantics by default. clearSelection() deselects via the real
    // isSelected() state across every element, not a m_selectedIds-driven loop -- m_selectedIds
    // is a write-only mouse-gesture cache (see this class's own mousePressEvent() doc comment)
    // that several call sites (addItems(), palette adds, morph/nextElm()/prevElm(), any
    // command's own auto-select) never populate, so relying on it here would leave elements
    // selected via any of those paths never deselected.
    clearSelection();
}

void CanvasItem::updateSelectionRect(const QPointF &current)
{
    m_selectionRect = QRectF(m_selectionAnchor, current).normalized();
    // Mirrors SceneInteraction::mouseMove's "m_scene->setSelectionArea(selectionBox)":
    // SpatialIndex::queryRect() is this canvas's equivalent intersects-shape query.
    const auto hits = m_index.queryRect(m_selectionRect);
    QSet<quint64> newSelection;
    for (const quint64 id : hits) {
        if (m_elementsById.contains(id)) { // exclude wire ids -- only elements are selectable
            newSelection.insert(id);
        }
    }

    // Sync the real GraphicElement::setSelected() flag for exactly the delta -- elements
    // leaving the rubber band vs. entering it -- rather than clearing everything and
    // resetting, so this stays correct even though this is called on every mouse-move
    // during a drag.
    bool changed = false;
    for (const quint64 id : std::as_const(m_selectedIds)) {
        if (!newSelection.contains(id)) {
            if (auto *element = m_elementsById.value(id, nullptr)) {
                element->setSelected(false);
            }
            changed = true;
        }
    }
    for (const quint64 id : std::as_const(newSelection)) {
        if (!m_selectedIds.contains(id)) {
            if (auto *element = m_elementsById.value(id, nullptr)) {
                element->setSelected(true);
            }
            changed = true;
        }
    }
    m_selectedIds = std::move(newSelection);
    if (changed) {
        emit selectionChanged();
    }
}

void CanvasItem::finishSelectionRect()
{
    m_markingSelectionBox = false;
    m_selectionRect = QRectF();
}

bool CanvasItem::isOverOwnPort(GraphicElement *owner, const QPointF &pos) const
{
    for (const quint64 id : m_index.queryPoint(pos)) {
        if (auto *port = m_portsById.value(id, nullptr); port && port->graphicElement() == owner) {
            return true;
        }
    }
    return false;
}

void CanvasItem::updatePortHover(const QPointF &pos)
{
    // Same topmost-hit lookup tryCompleteWire() uses: SpatialIndex::queryPoint() guarantees
    // last = topmost, so scanning in reverse finds the first (topmost) id that's actually a
    // port rather than an element or wire.
    const auto hits = m_index.queryPoint(pos);
    Port *portUnderCursor = nullptr;
    for (auto it = hits.crbegin(); it != hits.crend(); ++it) {
        if (auto *port = m_portsById.value(*it, nullptr)) {
            portUnderCursor = port;
            break;
        }
    }

    if (portUnderCursor == m_hoveredPort) {
        return;
    }

    m_hoveredPort = portUnderCursor;
    emit portHoverChanged(buildPortHoverChips(portUnderCursor));
}

void CanvasItem::clearPortHover()
{
    if (!m_hoveredPort) {
        return;
    }
    m_hoveredPort = nullptr;
    emit portHoverChanged({});
}

QVariantList CanvasItem::buildPortHoverChips(Port *hoverPort) const
{
    QVariantList chips;
    if (!hoverPort) {
        return chips;
    }

    const auto &theme = ThemeManager::attributes();
    const auto addChip = [&chips, &theme](Port *port) {
        if (!port) {
            return;
        }
        const QPointF pos = port->scenePos();
        QVariantMap chip;
        chip[QStringLiteral("screenX")] = pos.x();
        chip[QStringLiteral("screenY")] = pos.y();
        chip[QStringLiteral("radius")] = kPortHoverMarkerRadius;
        chip[QStringLiteral("side")] = sideForPort(port);
        chip[QStringLiteral("text")] = port->name();
        // Real theme colors, not generic Qt Quick palette roles -- see this method's own
        // doc comment for why. Mirrors PortHoverLabel's original color choices exactly
        // (App/Scene/PortHoverLabel.cpp, WIDGETS_ONLY_SOURCES on this branch).
        chip[QStringLiteral("ringColor")] = theme.m_portHoverPort;
        chip[QStringLiteral("labelBgColor")] = theme.m_portHoverLabelBg;
        chip[QStringLiteral("labelTextColor")] = theme.m_portHoverLabelText;
        chips.append(chip);
    };

    addChip(hoverPort);
    for (auto *peer : connectedPeers(hoverPort)) {
        addChip(peer);
    }

    return chips;
}

void CanvasItem::hoverMoveEvent(QHoverEvent *event)
{
    // event->globalPosition() (used below for the IC hover-preview popup) is OS-level global
    // screen coordinates -- entirely separate from this item's own local space, unaffected by
    // pan/zoom, so it's used unconverted exactly as before.
    const QPointF worldPos = screenToWorld(event->position());
    const auto hits = m_index.queryPoint(worldPos);
    const quint64 newHoveredId = hits.isEmpty() ? 0 : hits.last();

    if (newHoveredId != m_hoveredId) {
        // Leaving the old element first, entering the new one after, mirrors real Scene
        // behavior (and IC's own preview show/hide request pairing -- see this class's doc
        // comment on the hover-preview signal chain).
        if (auto *oldIc = qobject_cast<IC *>(m_elementsById.value(m_hoveredId, nullptr))) {
            oldIc->previewHideRequested();
            emit icPreviewHideRequested();
        }
        m_hoveredId = newHoveredId;
        if (auto *newIc = qobject_cast<IC *>(m_elementsById.value(newHoveredId, nullptr))) {
            if (isOverOwnPort(newIc, worldPos)) {
                newIc->previewHideRequested();
                emit icPreviewHideRequested();
            } else {
                const QPoint screenPos = event->globalPosition().toPoint();
                newIc->previewRequested(newIc, screenPos);
                emit icPreviewRequested(newIc, screenPos);
            }
        }
        update();
    } else if (auto *ic = qobject_cast<IC *>(m_elementsById.value(m_hoveredId, nullptr))) {
        // Same IC, cursor still moving within it -- keep the pending-preview position current.
        if (isOverOwnPort(ic, worldPos)) {
            ic->previewHideRequested();
            emit icPreviewHideRequested();
        } else {
            const QPoint screenPos = event->globalPosition().toPoint();
            ic->previewMoved(ic, screenPos);
            emit icPreviewMoved(ic, screenPos);
        }
    }

    updatePortHover(worldPos);
}

void CanvasItem::hoverLeaveEvent(QHoverEvent *)
{
    if (m_hoveredId != 0) {
        if (auto *ic = qobject_cast<IC *>(m_elementsById.value(m_hoveredId, nullptr))) {
            ic->previewHideRequested();
            emit icPreviewHideRequested();
        }
        m_hoveredId = 0;
        update();
    }

    clearPortHover();
}

bool CanvasItem::nudgeSelection(QKeyEvent *event)
{
    // Only plain / Shift+arrow -- leave Ctrl/Alt combinations to other handlers.
    if (event->modifiers().testFlag(Qt::ControlModifier) || event->modifiers().testFlag(Qt::AltModifier)) {
        return false;
    }

    int dx = 0;
    int dy = 0;
    switch (event->key()) {
    case Qt::Key_Left:  dx = -1; break;
    case Qt::Key_Right: dx =  1; break;
    case Qt::Key_Up:    dy = -1; break;
    case Qt::Key_Down:  dy =  1; break;
    default: return false;
    }

    const QList<GraphicElement *> selected = selectedElements();
    if (selected.isEmpty()) {
        return false; // nothing selected -- let the arrow key do whatever the base class does
    }

    // One grid cell by default; Shift jumps four cells for coarse positioning.
    const int step = event->modifiers().testFlag(Qt::ShiftModifier) ? Constants::gridSize * 4 : Constants::gridSize;
    const QPointF delta(dx * step, dy * step);

    QList<QPointF> oldPositions;
    oldPositions.reserve(selected.size());
    for (auto *elm : selected) {
        oldPositions.append(elm->pos());
    }
    for (auto *elm : selected) {
        elm->setPos(elm->pos() + delta);
    }

    receiveCommand(new CanvasMoveCommand(selected, oldPositions, this));
    event->accept();
    return true;
}

void CanvasItem::keyPressEvent(QKeyEvent *event)
{
    // Ignore auto-repeat: holding a trigger key must fire once, not oscillate an InputSwitch
    // (whose keyboard trigger toggles on every press) dozens of times a second -- mirrors
    // Scene::keyPressEvent()'s identical guard.
    if (event->isAutoRepeat()) {
        QQuickItem::keyPressEvent(event);
        return;
    }

    // Spacebar held = pan mode (same as middle-mouse drag), mirroring
    // GraphicsView::keyPressEvent(); checked by mousePressEvent()'s own m_spacePanHeld branch.
    // Returns here rather than production's fall-through (which still calls the base handler
    // afterwards) -- harmless, since Space matches none of this method's other shortcuts below.
    if (event->key() == Qt::Key_Space) {
        m_spacePanHeld = true;
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    // Arrow keys nudge the current selection by a grid step (Shift = a larger step) as one
    // undoable move; consumes the event only when it actually moves a selection.
    if (nudgeSelection(event)) {
        return;
    }

    // Rotate/flip: no chrome menu/QAction shortcut layer intercepts these before they reach
    // here the way the real app's MainWindowUI.cpp-bound QActions do, so they're matched
    // directly -- see this class's doc comment on rotateRight()/etc. for why flipVertically()
    // has no binding here (it has none in the real app either).
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        // Zoom: mirrors GraphicsView's own actions (MainWindowUI.cpp's setupUi() -- "Ctrl+="/
        // "Ctrl++" both zoom in since most users reach for the unshifted "=" key, "Ctrl+-" zooms
        // out, "Ctrl+0" resets, "Ctrl+Shift+F" fits). No chrome menu/QAction shortcut layer
        // intercepts these before they reach here, same as rotate/flip below.
        if (event->modifiers().testFlag(Qt::ShiftModifier) && event->key() == Qt::Key_F) {
            zoomToFit();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_Equal || event->key() == Qt::Key_Plus) {
            zoomIn();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_Minus) {
            zoomOut();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_0) {
            resetZoom();
            event->accept();
            return;
        }
        if (event->modifiers().testFlag(Qt::ShiftModifier) && event->key() == Qt::Key_R) {
            rotateLeft();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_R) {
            rotateRight();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_H) {
            flipHorizontally();
            event->accept();
            return;
        }
        // Clipboard/select-all: matches the real app's Ctrl+A/C/X/V/D shortcuts
        // (MainWindowUI.cpp's setupUi()).
        if (event->key() == Qt::Key_A) {
            selectAll();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_C) {
            copyAction();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_X) {
            cutAction();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_V) {
            pasteAction();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_D) {
            duplicateAction();
            event->accept();
            return;
        }
    }

    // Delete: matches the real app's unmodified "Del" shortcut.
    if (event->key() == Qt::Key_Delete) {
        deleteSelected();
        event->accept();
        return;
    }

    // Type/property cycling: matches SceneUiBinder.cpp's real unmodified QShortcut bindings
    // ("[" / "]" main property, "{" / "}" secondary property, "<" / ">" type-cycling).
    switch (event->key()) {
    case Qt::Key_BracketLeft:  prevMainPropShortcut(); event->accept(); return;
    case Qt::Key_BracketRight: nextMainPropShortcut(); event->accept(); return;
    case Qt::Key_BraceLeft:    prevSecndPropShortcut(); event->accept(); return;
    case Qt::Key_BraceRight:   nextSecndPropShortcut(); event->accept(); return;
    case Qt::Key_Less:         prevElm(); event->accept(); return;
    case Qt::Key_Greater:      nextElm(); event->accept(); return;
    default: break;
    }

    // Skip keyboard triggers while Ctrl is held, so the Ctrl-modified shortcuts above don't
    // also fire an element's trigger key -- mirrors Scene::keyPressEvent()'s identical guard.
    if (!event->modifiers().testFlag(Qt::ControlModifier)) {
        for (auto *element : std::as_const(m_elements)) {
            if (element->hasTrigger() && !element->trigger().isEmpty() && element->trigger().matches(event->key())) {
                if (auto *input = qobject_cast<GraphicElementInput *>(element); input && !input->isLocked()) {
                    input->setOn();
                    // Unlike mouse-driven triggers (mousePressEvent() calls update()
                    // unconditionally at the end of its element-click branch), this had no
                    // explicit repaint call of its own -- silently relying on the blind refresh
                    // timer CanvasItem no longer has (see Simulation::visualStateChanged()'s
                    // doc comment). Without this, a keyboard-triggered switch would never
                    // visibly update while the simulation is paused.
                    update();
                    // Downstream propagation through the rest of the circuit only happens
                    // inside Simulation::update() -- with the timer possibly fully stopped (no
                    // clocks), this input's own change needs an explicit prompt sweep, same
                    // reasoning as the mouse-driven triggers in activateOnPress()/
                    // mousePressEvent().
                    m_simulation->wakeSoon();
                }
            }
        }
    }

    QQuickItem::keyPressEvent(event);
}

void CanvasItem::keyReleaseEvent(QKeyEvent *event)
{
    // On X11 a held key emits release/press pairs; ignoring auto-repeat here avoids releasing
    // a momentary InputButton mid-hold -- mirrors Scene::keyReleaseEvent()'s identical guard.
    if (event->isAutoRepeat()) {
        QQuickItem::keyReleaseEvent(event);
        return;
    }

    if (event->key() == Qt::Key_Space) {
        m_spacePanHeld = false;
        unsetCursor();
        event->accept();
        return;
    }

    if (!event->modifiers().testFlag(Qt::ControlModifier)) {
        for (auto *element : std::as_const(m_elements)) {
            if (element->hasTrigger() && !element->trigger().isEmpty() && element->trigger().matches(event->key())) {
                // Only InputButton (momentary) is released on key-up; InputSwitch stays latched.
                if (auto *input = qobject_cast<GraphicElementInput *>(element);
                    input && !input->isLocked() && (element->elementType() == ElementType::InputButton)) {
                    input->setOff();
                    update(); // see the matching keyPressEvent() branch's doc comment
                    m_simulation->wakeSoon(); // see the matching keyPressEvent() branch's doc comment
                }
            }
        }
    }

    QQuickItem::keyReleaseEvent(event);
}

void CanvasItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    if (newGeometry.size() != oldGeometry.size()) {
        emit zoomChanged();
    }
}

void CanvasItem::wheelEvent(QWheelEvent *event)
{
    const int zoomDirection = event->angleDelta().y();
    if (zoomDirection > 0) {
        zoomIn(event->position());
    } else if (zoomDirection < 0) {
        zoomOut(event->position());
    }
    event->accept();
}

QSGNode *CanvasItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    // Element/wire positions can change now (drag-to-move rebuilds m_index on every move, see
    // mouseMoveEvent()); this method itself only ever reads current live state on each repaint,
    // it never assumes positions are static.
    auto *root = oldNode ? oldNode : new QSGNode();
    QSGGeometryNode *backgroundNode;
    QSGTransformNode *transformNode;
    if (!oldNode) {
        // Solid scene-background fill, painted first and outside transformNode -- it always
        // covers the whole item regardless of pan/zoom, mirroring Scene::drawBackground()'s own
        // QGraphicsScene::drawBackground(painter, rect) base-fill call before it draws grid dots.
        backgroundNode = new QSGGeometryNode();
        root->appendChildNode(backgroundNode);

        // One QSGTransformNode carries the pan/zoom mapping for every child below it, so the
        // grid/gate/wire/hover/overlay vertex-building code beneath stays entirely in WORLD
        // coordinates, unchanged by pan/zoom -- see screenToWorld()/worldToScreen()'s doc
        // comment on this class for the split this mirrors.
        transformNode = new QSGTransformNode();
        auto *gridNode = new QSGGeometryNode(); // dot grid, underneath everything else in world space
        auto *wireHaloNode = new QSGGeometryNode(); // selected-element wire highlight, underneath the wires themselves
        auto *wireNode = new QSGGeometryNode();
        auto *hoverNode = new QSGGeometryNode(); // hover highlight, underneath the gate it highlights
        auto *gateNode = new QSGGeometryNode();  // real per-element appearance, textured
        auto *overlayNode = new QSGGeometryNode(); // live rubber-band rect, paints on top of everything
        transformNode->appendChildNode(gridNode); // grid paints first, under everything
        transformNode->appendChildNode(wireHaloNode); // halo paints next, under the wires
        transformNode->appendChildNode(wireNode);
        transformNode->appendChildNode(hoverNode);
        transformNode->appendChildNode(gateNode);
        transformNode->appendChildNode(overlayNode);
        root->appendChildNode(transformNode);
    } else {
        backgroundNode = static_cast<QSGGeometryNode *>(root->firstChild());
        transformNode = static_cast<QSGTransformNode *>(backgroundNode->nextSibling());
    }

    // screenPt = (worldPt - panOffset) * zoomScale, exactly matching worldToScreen() -- QMatrix4x4
    // composes right-to-left (the last-called op applies to the input vector first), so
    // translate is built AFTER scale here even though it conceptually happens first.
    QMatrix4x4 matrix;
    const auto scale = float(zoomScale());
    matrix.scale(scale, scale, 1.0f);
    matrix.translate(float(-m_panOffset.x()), float(-m_panOffset.y()), 0.0f);
    transformNode->setMatrix(matrix);

    QSGNode *gridNode = transformNode->firstChild();
    QSGNode *wireHaloNode = gridNode->nextSibling();
    QSGNode *wireNode = wireHaloNode->nextSibling();
    QSGNode *hoverNode = wireNode->nextSibling();
    QSGNode *gateNode = hoverNode->nextSibling();
    QSGNode *overlayNode = gateNode->nextSibling();

    // --- Background: a single quad covering the whole item, filled with the theme's scene
    // background color. Mirrors QGraphicsScene::drawBackground()'s own implicit base fill
    // (Scene::drawBackground() calls it before drawing grid dots). Outside transformNode, so
    // it always covers the full viewport regardless of pan/zoom. ---
    {
        const ThemeAttributes &theme = ThemeManager::attributes();
        QSGGeometry *geometry = geometryFor(backgroundNode, QSGGeometry::defaultAttributes_ColoredPoint2D(), 6);
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        QSGGeometry::ColoredPoint2D *vertices = geometry->vertexDataAsColoredPoint2D();
        int cursor = 0;
        appendQuad(vertices, cursor, QRectF(0, 0, width(), height()), theme.m_sceneBgBrush);
        backgroundNode->markDirty(QSGNode::DirtyGeometry);
        if (!backgroundNode->material()) {
            auto *material = new QSGVertexColorMaterial();
            backgroundNode->setMaterial(material);
            backgroundNode->setFlag(QSGNode::OwnsMaterial);
        }
    }

    // --- Grid: a dot at every Constants::gridSize world-space intersection within the visible
    // viewport, mirroring Scene::drawBackground()'s own painter->drawPoints() loop. Drawn as a
    // handful of quads, each textured with a shared kMacroTileSize-square "macro tile" image
    // (many dots pre-rendered onto one transparent-background QImage), rather than one
    // flat-colored quad per dot, which could reach hundreds of thousands of vertices when
    // zoomed out over a large circuit.
    //
    // A single quad per macro tile using QSGTexture::Repeat wrap mode (tiling one small dot
    // image across the whole visible rect) is not used here: QSGTexture::Repeat / UV values
    // past 1.0 silently sample the (transparent) clamped edge instead of tiling on this QRhi
    // backend/driver. So this version never asks the GPU to tile anything: the macro tile is
    // pre-rendered with its own internal repetition (many dots baked into one image via a
    // QPainter loop) and every quad samples it with UV kept within [0,1].
    //
    // kMacroTileSize (a multiple of Constants::gridSize) bounds the quad count: at the most
    // extreme zoom-out this project allows, the visible world rect is on the order of a few
    // thousand world units per axis, which divided by kMacroTileSize is only a handful of tiles
    // per axis -- nowhere near a vertex-count concern (6 vertices/quad), while the macro tile
    // image itself stays small enough (kMacroTileSize px square) to build with a plain QPainter
    // loop cheaply.
    //
    // The macro tile is rebuilt only when the discrete zoom level or the dot's theme color
    // changes (panning, by far the common case, reuses the same texture untouched, only moving/
    // adding/removing whole quads): dotWorldSize (kDotScreenPixels/zoomScale()) keeps the dot at
    // a constant ~1.5px on screen regardless of zoom, the same way QPainter's default (cosmetic,
    // width-0) pen keeps Widgets' dots at a constant 1 device pixel -- baking that world size
    // into the tile at the current zoom level reproduces it exactly, just recomputed per zoom
    // step instead of per dot. Skipped below zoom 0.3 (dots would be sub-pixel/invisible),
    // matching Scene::drawBackground()'s own early return. ---
    {
        const ThemeAttributes &theme = ThemeManager::attributes();
        constexpr qreal kDotScreenPixels = 1.5;
        constexpr int kMacroTileSize = 512; // multiple of Constants::gridSize (32 dots/side)

        auto *gridGeomNode = static_cast<QSGGeometryNode *>(gridNode);
        int vertexCount = 0;
        qint64 leftTile = 0;
        qint64 topTile = 0;
        int cols = 0;
        int rows = 0;
        if (scale >= 0.3f) {
            const QRectF visible = visibleWorldRect();
            leftTile = static_cast<qint64>(std::floor(visible.left() / kMacroTileSize)) * kMacroTileSize;
            topTile = static_cast<qint64>(std::floor(visible.top() / kMacroTileSize)) * kMacroTileSize;
            const qint64 rightTile = static_cast<qint64>(std::ceil(visible.right() / kMacroTileSize)) * kMacroTileSize;
            const qint64 bottomTile = static_cast<qint64>(std::ceil(visible.bottom() / kMacroTileSize)) * kMacroTileSize;
            cols = int((rightTile - leftTile) / kMacroTileSize);
            rows = int((bottomTile - topTile) / kMacroTileSize);

            if (!m_gridDotTexture || m_gridDotTextureZoomLevel != m_zoomLevel || m_gridDotTextureColor != theme.m_sceneBgDots) {
                const qreal dotWorldSize = kDotScreenPixels / double(scale);
                QImage tile(kMacroTileSize, kMacroTileSize, QImage::Format_ARGB32_Premultiplied);
                tile.fill(Qt::transparent);
                QPainter tilePainter(&tile);
                tilePainter.setRenderHint(QPainter::Antialiasing);
                tilePainter.setPen(Qt::NoPen);
                tilePainter.setBrush(theme.m_sceneBgDots);
                for (int y = Constants::gridSize / 2; y < kMacroTileSize; y += Constants::gridSize) {
                    for (int x = Constants::gridSize / 2; x < kMacroTileSize; x += Constants::gridSize) {
                        tilePainter.drawEllipse(QPointF(x, y), dotWorldSize / 2.0, dotWorldSize / 2.0);
                    }
                }
                tilePainter.end();

                delete m_gridDotTexture;
                m_gridDotTexture = window()->createTextureFromImage(tile, QQuickWindow::TextureHasAlphaChannel);
                m_gridDotTexture->setFiltering(QSGTexture::Linear);
                m_gridDotTextureZoomLevel = m_zoomLevel;
                m_gridDotTextureColor = theme.m_sceneBgDots;
            }

            vertexCount = cols * rows * 6;
        }

        QSGGeometry *geometry = geometryFor(gridGeomNode, QSGGeometry::defaultAttributes_TexturedPoint2D(), vertexCount);
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        if (vertexCount > 0) {
            QSGGeometry::TexturedPoint2D *vertices = geometry->vertexDataAsTexturedPoint2D();
            int cursor = 0;
            static const QRectF kFullUv(0.0, 0.0, 1.0, 1.0);
            for (int row = 0; row < rows; ++row) {
                for (int col = 0; col < cols; ++col) {
                    const QRectF tileWorldRect(qreal(leftTile + qint64(col) * kMacroTileSize),
                                                qreal(topTile + qint64(row) * kMacroTileSize),
                                                qreal(kMacroTileSize), qreal(kMacroTileSize));
                    appendTexturedQuad(vertices, cursor, tileWorldRect, kFullUv);
                }
            }
        }
        gridGeomNode->markDirty(QSGNode::DirtyGeometry);

        auto *material = static_cast<QSGTextureMaterial *>(gridGeomNode->material());
        if (!material) {
            material = new QSGTextureMaterial();
            gridGeomNode->setMaterial(material);
            gridGeomNode->setFlag(QSGNode::OwnsMaterial);
        }
        if (m_gridDotTexture) {
            material->setTexture(m_gridDotTexture);
        }
        gridGeomNode->markDirty(QSGNode::DirtyMaterial);
    }

    // --- Wires: two QSGGeometryNodes, GL_LINES, both tessellated into the same cubic Bézier
    // S-curve Connection::updatePath() draws (see tessellateWire()) instead of a single straight
    // segment. wireNode carries the real wire, colored by the driving port's live status, or by
    // a selection color when the wire itself is selected (mirrors Connection::paint()'s own
    // isSelected() branch). wireHaloNode carries a wider underlay stroke, painted just before
    // wireNode so it sits underneath, for every wire attached to a currently-selected element
    // (mirrors Connection::paint()'s own highlight halo, normally driven by
    // GraphicElement::highlight()/Connection::setHighLight() via itemChange() -- dead for
    // CanvasItem since its elements are never scene-attached, so this reads live isSelected()
    // state fresh every frame instead, the same pattern the gate node already uses for its own
    // selection outline). The in-progress wire (if any) is appended last to wireNode, from its
    // anchored port to m_editedWireFreeEnd, in a neutral color -- it has no driving Status yet
    // since it isn't committed to any element's simulation graph, and is never haloed. ---
    {
        static const QColor kHaloColor(33, 150, 243, 130);
        const ThemeAttributes &theme = ThemeManager::attributes();

        // Memoizes Port::scenePos() -- see m_portScenePosCache's doc comment. Ports only move
        // when rebuildSpatialIndex() has already run, so a cache miss here means "not seen
        // since the cache was last cleared", populated on first use.
        const auto cachedScenePos = [this](Port *port) {
            auto it = m_portScenePosCache.constFind(port);
            if (it != m_portScenePosCache.cend()) {
                return *it;
            }
            const QPointF pos = port->scenePos();
            m_portScenePosCache.insert(port, pos);
            return pos;
        };

        struct WireGeometry {
            QVarLengthArray<QPointF, kWireSegments + 1> points;
            QColor color;
            bool highlighted;
        };
        QVector<WireGeometry> wires;
        int highlightedCount = 0;
        // Show Wires toggle off -- leaves `wires` empty, so nothing below draws; matches
        // VisibilityManager::showWires(false) hiding every Connection outright.
        if (m_wiresVisible) {
            wires.reserve(m_connections.size());
            for (auto *connection : m_connections) {
                const QPointF start = cachedScenePos(connection->startPort());
                const QPointF end = cachedScenePos(connection->endPort());
                // Mirrors Connection::paint()'s own isSelected() branch: selected color comes
                // from the real theme, but the per-status palette (colorForStatus()) stays a
                // fixed literal -- only the selected-wire case is themed.
                const QColor color = connection->isSelected()
                    ? theme.m_connectionSelected
                    : colorForStatus(connection->startPort()->status());
                auto *startElement = connection->startPort()->graphicElement();
                auto *endElement = connection->endPort()->graphicElement();
                const bool highlighted = (startElement && startElement->isSelected()) || (endElement && endElement->isSelected());
                if (highlighted) {
                    ++highlightedCount;
                }
                wires.append(WireGeometry{tessellateWire(start, end), color, highlighted});
            }
        }

        const int editedVertexCount = m_editedConnection ? kWireSegments * 2 : 0;
        QSGGeometry *geometry = geometryFor(static_cast<QSGGeometryNode *>(wireNode),
                                             QSGGeometry::defaultAttributes_ColoredPoint2D(),
                                             int(wires.size()) * kWireSegments * 2 + editedVertexCount);
        geometry->setDrawingMode(QSGGeometry::DrawLines);
        geometry->setLineWidth(2.0f);
        QSGGeometry::ColoredPoint2D *vertices = geometry->vertexDataAsColoredPoint2D();
        int cursor = 0;
        for (const auto &wire : std::as_const(wires)) {
            const auto r = uchar(wire.color.red());
            const auto g = uchar(wire.color.green());
            const auto b = uchar(wire.color.blue());
            const auto a = uchar(wire.color.alpha());
            for (int i = 0; i < kWireSegments; ++i) {
                vertices[cursor++].set(float(wire.points[i].x()), float(wire.points[i].y()), r, g, b, a);
                vertices[cursor++].set(float(wire.points[i + 1].x()), float(wire.points[i + 1].y()), r, g, b, a);
            }
        }
        if (m_editedConnection) {
            static const QColor kEditedWireColor(158, 158, 158, 220);
            const QPointF anchored = m_editedConnection->startPort()
                ? cachedScenePos(m_editedConnection->startPort())
                : cachedScenePos(m_editedConnection->endPort());
            const auto editedPoints = tessellateWire(anchored, m_editedWireFreeEnd);
            const auto r = uchar(kEditedWireColor.red());
            const auto g = uchar(kEditedWireColor.green());
            const auto b = uchar(kEditedWireColor.blue());
            const auto a = uchar(kEditedWireColor.alpha());
            for (int i = 0; i < kWireSegments; ++i) {
                vertices[cursor++].set(float(editedPoints[i].x()), float(editedPoints[i].y()), r, g, b, a);
                vertices[cursor++].set(float(editedPoints[i + 1].x()), float(editedPoints[i + 1].y()), r, g, b, a);
            }
        }
        static_cast<QSGGeometryNode *>(wireNode)->markDirty(QSGNode::DirtyGeometry);
        if (!static_cast<QSGGeometryNode *>(wireNode)->material()) {
            auto *material = new QSGVertexColorMaterial();
            static_cast<QSGGeometryNode *>(wireNode)->setMaterial(material);
            static_cast<QSGGeometryNode *>(wireNode)->setFlag(QSGNode::OwnsMaterial);
        }

        QSGGeometry *haloGeometry = geometryFor(static_cast<QSGGeometryNode *>(wireHaloNode),
                                                 QSGGeometry::defaultAttributes_ColoredPoint2D(),
                                                 highlightedCount * kWireSegments * 2);
        haloGeometry->setDrawingMode(QSGGeometry::DrawLines);
        haloGeometry->setLineWidth(10.0f);
        QSGGeometry::ColoredPoint2D *haloVertices = haloGeometry->vertexDataAsColoredPoint2D();
        int haloCursor = 0;
        for (const auto &wire : std::as_const(wires)) {
            if (!wire.highlighted) {
                continue;
            }
            for (int i = 0; i < kWireSegments; ++i) {
                haloVertices[haloCursor++].set(float(wire.points[i].x()), float(wire.points[i].y()),
                                                uchar(kHaloColor.red()), uchar(kHaloColor.green()), uchar(kHaloColor.blue()), uchar(kHaloColor.alpha()));
                haloVertices[haloCursor++].set(float(wire.points[i + 1].x()), float(wire.points[i + 1].y()),
                                                uchar(kHaloColor.red()), uchar(kHaloColor.green()), uchar(kHaloColor.blue()), uchar(kHaloColor.alpha()));
            }
        }
        static_cast<QSGGeometryNode *>(wireHaloNode)->markDirty(QSGNode::DirtyGeometry);
        if (!static_cast<QSGGeometryNode *>(wireHaloNode)->material()) {
            auto *haloMaterial = new QSGVertexColorMaterial();
            static_cast<QSGGeometryNode *>(wireHaloNode)->setMaterial(haloMaterial);
            static_cast<QSGGeometryNode *>(wireHaloNode)->setFlag(QSGNode::OwnsMaterial);
        }
    }

    // --- Hover highlight: a padded translucent quad under the hovered element only, in place
    // of the flat-color brightening this canvas used before real appearance rendering -- a
    // QSGTextureMaterial has no per-vertex tint to brighten, so this is a separate underlay,
    // the same technique the marquee/selection overlays already use. ---
    {
        GraphicElement *hovered = m_elementsById.value(m_hoveredId, nullptr);
        const int vertexCount = hovered ? 6 : 0;
        QSGGeometry *geometry = geometryFor(static_cast<QSGGeometryNode *>(hoverNode),
                                             QSGGeometry::defaultAttributes_ColoredPoint2D(), vertexCount);
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        if (hovered) {
            QSGGeometry::ColoredPoint2D *vertices = geometry->vertexDataAsColoredPoint2D();
            int cursor = 0;
            static const QColor kHoverColor(255, 255, 255, 90);
            appendQuad(vertices, cursor, hovered->sceneBoundingRect().adjusted(-4, -4, 4, 4), kHoverColor);
        }
        static_cast<QSGGeometryNode *>(hoverNode)->markDirty(QSGNode::DirtyGeometry);
        if (!static_cast<QSGGeometryNode *>(hoverNode)->material()) {
            auto *material = new QSGVertexColorMaterial();
            static_cast<QSGGeometryNode *>(hoverNode)->setMaterial(material);
            static_cast<QSGGeometryNode *>(hoverNode)->setFlag(QSGNode::OwnsMaterial);
        }
    }

    // --- Gates: one QSGGeometryNode, GL_TRIANGLES, textured. Real per-element appearance --
    // each element's own real, unmodified paint() (dispatched polymorphically) rendered
    // offscreen and cached via m_atlas; see this class's doc comment. Selection is the real
    // GraphicElement::setSelected() flag baked into that paint() call (real
    // ElementAppearance::render() draws its own selection-outline rectangle), not a separate
    // underlay quad. ---
    {
        struct PlacedTile {
            QRectF worldRect;
            QRectF uv;
        };
        QVector<PlacedTile> placed;
        placed.reserve(m_elements.size());

        for (auto *element : std::as_const(m_elements)) {
            if (isElementHidden(element)) {
                continue; // Show Gates/Wires toggle -- see this class's own doc comment
            }

            // Cheap fingerprint of everything appearanceKeyFor()/the localRect computation below
            // actually depend on -- see m_elementRenderCache's doc comment. A full match means
            // neither boundingRect() (a real port-walking recompute) nor appearanceKeyFor() (a
            // per-port string-building loop) nor the atlas lookup need to run again this frame.
            // Port status (inputs and outputs) is covered by isRenderDirty() instead of
            // re-reading every port's live status here -- GraphicElement::markRenderDirty() is
            // called directly from InputPort::setStatus()/OutputPort::setStatus() on a genuine
            // change, so this is an O(1) field read replacing an O(total ports) rebuild.
            const qint64 pixmapCacheKey = element->appearanceCacheKey();
            const qreal rotation = element->rotation();
            const bool flipX = element->isFlippedX();
            const bool flipY = element->isFlippedY();
            const bool selected = element->isSelected();
            const int inputSize = element->inputSize();
            const int outputSize = element->outputSize();
            const QString labelText = element->labelItem()->text();

            const auto cacheIt = m_elementRenderCache.constFind(element);
            const bool cacheHit = cacheIt != m_elementRenderCache.cend()
                && !element->isRenderDirty()
                && cacheIt->pixmapCacheKey == pixmapCacheKey
                && cacheIt->rotation == rotation
                && cacheIt->flipX == flipX
                && cacheIt->flipY == flipY
                && cacheIt->selected == selected
                && cacheIt->inputSize == inputSize
                && cacheIt->outputSize == outputSize
                && cacheIt->labelText == labelText;

            QRectF localRect;
            TextureAtlas::TileLocation tile;
            if (cacheHit) {
                localRect = cacheIt->localRect;
                tile = cacheIt->tile;
            } else {
                // Widened to the union with the label's own rect (Text::boundingRect() already
                // does this for itself + its empty-state hint, but the base GraphicElement::
                // boundingRect() doesn't include the label at all) so a long label isn't clipped
                // out of the captured tile.
                localRect = element->boundingRect().united(element->labelLocalBoundingRect());
                const QSize tileSize = localRect.size().toSize();
                const QString key = appearanceKeyFor(element);
                tile = m_atlas.lookup(key, tileSize, [this, element, &localRect](QPainter &painter) {
                    painter.translate(-localRect.topLeft());
                    element->paint(&painter);

                    // Port glyphs: real production shape and paint() (a circle for InputPort /
                    // triangle for OutputPort), positioned via the same pos()+transform()
                    // composition production painting uses (mirrors ICRenderer.cpp's identical
                    // technique for the IC hover-preview path). Port's own m_currentPen/m_brush
                    // are always kept live by setStatus()->updateTheme() -- InputPort::setStatus()/
                    // OutputPort::setStatus() both call updateTheme() on every real status change,
                    // and both port constructors set it once more directly for the one case
                    // setStatus() itself can't cover (the freshly-constructed default status,
                    // which never differs from itself) -- so calling port->paint() directly here
                    // needs no separate style computation. Port paints itself as a separate
                    // sibling item in the Widgets app, which this canvas's direct element->paint()
                    // call never reaches, so painting it explicitly here is required: otherwise an
                    // unconnected required input (which should show red/Error) has no visual cue.
                    const auto paintPort = [&](Port *port) {
                        if (isPortHidden(port)) {
                            return; // Show Wires toggle off -- see isPortHidden()'s own doc comment
                        }
                        painter.save();
                        painter.translate(port->pos());
                        painter.setTransform(port->transform(), true);
                        port->paint(&painter);
                        painter.restore();
                    };
                    for (auto *port : element->inputs())  { paintPort(port); }
                    for (auto *port : element->outputs()) { paintPort(port); }

                    // Label: GraphicElement::m_label is a plain value collaborator now (not a
                    // child QGraphicsItem the Widgets scene would traverse and paint on its own),
                    // so this canvas's direct element->paint() call never reaches it either.
                    // paint() itself already no-ops when invisible/empty, so this is safe to call
                    // unconditionally, matching ICRenderer.cpp's identical composition (rotation()
                    // applied first, then transform(), then pos() -- see
                    // GraphicElementLabel::mapToOwnerLocal()'s own doc comment for why this order).
                    auto *label = element->labelItem();
                    painter.save();
                    painter.translate(label->pos());
                    painter.setTransform(label->transform(), true);
                    if (!qFuzzyCompare(label->rotation(), 0.0)) {
                        painter.rotate(label->rotation());
                    }
                    label->paint(&painter);
                    painter.restore();

                    // Text's empty-state hint ("double-click to add text") is a second, separate
                    // GraphicElementLabel this element paints on top of an otherwise fully blank
                    // box -- same composition and same paint()-is-visibility-gated safety as the
                    // real label above.
                    if (auto *text = qobject_cast<Text *>(element)) {
                        auto *hint = text->emptyHintItem();
                        painter.save();
                        painter.translate(hint->pos());
                        painter.setTransform(hint->transform(), true);
                        if (!qFuzzyCompare(hint->rotation(), 0.0)) {
                            painter.rotate(hint->rotation());
                        }
                        hint->paint(&painter);
                        painter.restore();
                    }
                });
                m_elementRenderCache[element] = ElementRenderCache{
                    pixmapCacheKey, rotation, flipX, flipY, selected, inputSize, outputSize, labelText, localRect, tile};
                // Cleared regardless of tile.isValid() below, matching the cache entry itself
                // being stored either way -- an atlas-full/degenerate-rect element that stayed
                // dirty would otherwise retry the same failed rebuild every single repaint
                // forever, instead of settling into a stable (if unpainted) cache hit like a
                // cache-miss on the other fields already does today.
                element->clearRenderDirty();
            }
            if (!tile.isValid()) {
                continue; // atlas page full or a degenerate (empty) boundingRect -- skip, don't crash
            }
            placed.append(PlacedTile{localRect.translated(element->pos()), tile.uv});
        }

        QSGGeometry *geometry = geometryFor(static_cast<QSGGeometryNode *>(gateNode),
                                             QSGGeometry::defaultAttributes_TexturedPoint2D(), int(placed.size()) * 6);
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        QSGGeometry::TexturedPoint2D *vertices = geometry->vertexDataAsTexturedPoint2D();
        int cursor = 0;
        for (const auto &tile : std::as_const(placed)) {
            appendTexturedQuad(vertices, cursor, tile.worldRect, tile.uv);
        }
        static_cast<QSGGeometryNode *>(gateNode)->markDirty(QSGNode::DirtyGeometry);

        auto *material = static_cast<QSGTextureMaterial *>(static_cast<QSGGeometryNode *>(gateNode)->material());
        if (!material) {
            material = new QSGTextureMaterial();
            static_cast<QSGGeometryNode *>(gateNode)->setMaterial(material);
            static_cast<QSGGeometryNode *>(gateNode)->setFlag(QSGNode::OwnsMaterial);
        }
        // The atlas page may have been re-uploaded (new tile this frame); refresh the
        // material's texture pointer every frame regardless -- texture() only actually
        // re-uploads when its own dirty flag is set (see its doc comment), so calling it
        // unconditionally here is cheap and avoids tracking staleness a second time.
        material->setTexture(m_atlas.texture(window()));

        // If the atlas grew mid-loop above, every element processed *before* the element that
        // triggered it already had its UV baked into `placed`/this geometry's vertex data
        // against the old, smaller page size -- stale relative to the now-larger texture just
        // bound above. m_elementRenderCache would otherwise hide that from every *unchanged*
        // element too: its stored ElementRenderCache::tile is a resolved TileLocation with the
        // same now-stale UV already baked in, and a cache hit reuses it directly without ever
        // calling back into the atlas (see the cacheHit branch above) -- so it needs clearing
        // here too, not just a repaint, or an element whose own state never changes again would
        // keep showing the wrong, pre-grow UV forever. Scheduling another repaint (not fixable
        // within this same frame; see TextureAtlas::takeGrew()'s own doc comment) re-runs this
        // whole block next frame with every element back to a cache miss, recomputing its UV
        // against the final, now-stable page size.
        if (m_atlas.takeGrew()) {
            m_elementRenderCache.clear();
            update();
        }
    }

    // --- Rubber-band overlay: translucent quad over the live selection rect, only while
    // marking. Zero-vertex geometry when idle -- draws nothing, cheaper than removing/
    // re-adding the node every press/release. ---
    {
        const int vertexCount = m_markingSelectionBox ? 6 : 0;
        QSGGeometry *geometry = geometryFor(static_cast<QSGGeometryNode *>(overlayNode),
                                             QSGGeometry::defaultAttributes_ColoredPoint2D(), vertexCount);
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        if (m_markingSelectionBox) {
            QSGGeometry::ColoredPoint2D *vertices = geometry->vertexDataAsColoredPoint2D();
            int cursor = 0;
            static const QColor kMarqueeColor(33, 150, 243, 70);
            appendQuad(vertices, cursor, m_selectionRect, kMarqueeColor);
        }
        static_cast<QSGGeometryNode *>(overlayNode)->markDirty(QSGNode::DirtyGeometry);
        if (!static_cast<QSGGeometryNode *>(overlayNode)->material()) {
            auto *material = new QSGVertexColorMaterial();
            static_cast<QSGGeometryNode *>(overlayNode)->setMaterial(material);
            static_cast<QSGGeometryNode *>(overlayNode)->setFlag(QSGNode::OwnsMaterial);
        }
    }

    return root;
}
