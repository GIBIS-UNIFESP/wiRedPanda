// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Canvas/CanvasItem.h"

#include <algorithm>

#include <QClipboard>
#include <QColor>
#include <QGuiApplication>
#include <QHoverEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QQuickWindow>
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGTextureMaterial>
#include <QSGVertexColorMaterial>

#include "App/Core/Constants.h"
#include "App/Core/Enums.h"
#include "App/Core/InstallRelativePaths.h"
#include "App/Core/ItemWithId.h"
#include "App/Core/MimeTypes.h"
#include "App/Core/SimulationHost.h"
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
#include "App/Scene/ConnectionManager.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

namespace {

/// Serves CanvasItem's live, in-memory element list to Simulation -- the same narrow seam
/// Scene itself binds through (see App/Core/SimulationHost.h), just with no QGraphicsScene
/// behind it. Binds to CanvasItem::m_elements BY REFERENCE, not a snapshot copy: Phase 3's
/// Add/Delete commands grow and shrink that vector after construction (CanvasItem::addItem()/
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
    const QVector<GraphicElement *> &m_elements;
};

/// Packs an id into the quint64 id space SpatialIndex uses, tagged by kind in the top bits so
/// elements/wires/ports never collide. elementId()/wireId() now take the real
/// ItemWithId::id() (assigned via CanvasItem::addItem(), see this class's doc comment on the
/// id/registry layer) rather than a synthetic array index -- ItemWithId::id() is always
/// non-negative once assigned, same precondition an array index had. portId() stays
/// index-based: Port (QGraphicsPathItem only, see Port.h) is not an ItemWithId, so it has no
/// real id to tag.
constexpr quint64 kElementTag = 0ULL << 62;
constexpr quint64 kWireTag = 1ULL << 62;
constexpr quint64 kPortTag = 2ULL << 62;
quint64 elementId(int id) { return kElementTag | quint64(id); }
quint64 wireId(int id) { return kWireTag | quint64(id); }
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

ItemWithId *CanvasItem::itemById(int id) const
{
    return m_itemRegistry.itemById(id);
}

int CanvasItem::nextId()
{
    return m_itemRegistry.nextId();
}

void CanvasItem::updateItemId(ItemWithId *item, int newId)
{
    m_itemRegistry.updateItemId(item, newId);
}

void CanvasItem::addItem(GraphicElement *element)
{
    if (!element) {
        return;
    }
    // Membership half (mirrors QGraphicsScene::addItem() being the other half of Scene::
    // addItem()'s job): idempotent, so callers that already appended element to m_elements
    // directly (buildDemoCircuit()) and callers relying on addItem() to do it
    // (CanvasCommandUtils::addItems(), Phase 3's Add/Delete command family) both work.
    if (!m_elements.contains(element)) {
        m_elements.append(element);
    }
    // Id-registration half. Unassigned (-1): give it the next id. Pre-assigned
    // (updateItemId() restore path): preserve it and advance the counter past it -- mirrors
    // Scene::addItem() exactly.
    if (element->id() < 0) {
        element->setId(nextId());
    } else {
        m_itemRegistry.setLastId(element->id());
    }
    m_itemRegistry.registerItem(element);
}

void CanvasItem::addItem(Connection *connection)
{
    if (!connection) {
        return;
    }
    if (!m_connections.contains(connection)) {
        m_connections.append(connection);
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
        m_itemRegistry.unregisterItem(element);
    }
}

void CanvasItem::removeItem(Connection *connection)
{
    if (connection) {
        m_connections.removeAll(connection);
        m_itemRegistry.unregisterItem(connection);
    }
}

SerializationContext CanvasItem::deserializationContext(QHash<quint64, Port *> &portMap,
                                                         const QVersionNumber &version,
                                                         SerializationPurpose purpose)
{
    // contextDir stays empty: this canvas has no notion of "the directory of the loaded
    // .panda file" yet -- nothing here loads a top-level file through this context (IC's own
    // loadFile()/loadFromBlob() calls, used by buildDemoCircuit()'s demo IC, resolve paths
    // through InstallRelativePaths/their own file argument instead). blobRegistry stays null
    // until the IC embedding sub-step gives this canvas a real blob map.
    return SerializationContext{.portMap = portMap, .version = version, .purpose = purpose};
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
    QList<QGraphicsItem *> items;
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
        if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item); elm && m_dragElements.contains(elm)) {
            m_draggingElement = false;
            m_dragElements.clear();
            m_dragStartPositions.clear();
            break;
        }
    }

    receiveCommand(new CanvasDeleteItemsCommand(items, this));
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

void CanvasItem::copyAction()
{
    QList<QGraphicsItem *> items;
    for (auto *elm : selectedElements()) {
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
    QGuiApplication::clipboard()->setMimeData(mimeData);
}

void CanvasItem::cutAction()
{
    QList<QGraphicsItem *> items;
    for (auto *elm : selectedElements()) {
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

    deleteSelected();

    auto *mimeData = new QMimeData();
    mimeData->setData(MimeType::Clipboard, itemData);
    QGuiApplication::clipboard()->setMimeData(mimeData);
}

void CanvasItem::pasteAction()
{
    const auto *mimeData = QGuiApplication::clipboard()->mimeData();
    if (!mimeData) {
        return;
    }

    // Blob-registry import (cross-tab paste of embedded ICs) deferred to the IC-embedding
    // sub-step -- this canvas has no ICRegistry yet.
    QByteArray itemData;
    if (mimeData->hasFormat(MimeType::ClipboardLegacy)) {
        itemData = mimeData->data(MimeType::ClipboardLegacy);
    }
    if (mimeData->hasFormat(MimeType::Clipboard)) {
        itemData = mimeData->data(MimeType::Clipboard);
    }
    if (itemData.isEmpty()) {
        return;
    }

    QDataStream stream(&itemData, QIODevice::ReadOnly);
    QVersionNumber version = Serialization::readPandaHeader(stream);
    deserializeAndAdd(stream, version);
}

void CanvasItem::duplicateAction()
{
    QList<QGraphicsItem *> items;
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
        item->setSelected(true);
    }
}

QList<QGraphicsItem *> CanvasItem::deserializeAndAdd(QDataStream &stream, const QVersionNumber &version,
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
        if (item->type() == GraphicElement::Type) {
            item->setPos(item->pos() + offset);
        }
    }
    rebuildSpatialIndex();

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
    for (int i = 0; i < m_elements.size(); ++i) {
        auto *element = m_elements.at(i);
        element->setSelected(true);
        m_selectedIds.insert(elementId(element->id()));
    }
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

    // Unwired too: these three exercise Phase 2's custom-paint families (vector trapezoid/
    // DIP-rect bodies drawn via drawBody()-equivalent methods, offscreen-rendered through the
    // same TextureAtlas as the icon-based elements above) rendering correctly through the
    // atlas -- proving the polymorphic paint()-dispatch design generalizes, not just that it
    // works for GraphicElement::paint()'s free-inheritance majority.
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
    // the already-portable InstallRelativePaths. This used to crash unconditionally in this
    // process (ICRenderer::generatePreviewPixmap() constructed a QGraphicsScene, which
    // depends on Widgets-private QApplication state a bare QGuiApplication never
    // initializes -- confirmed via GDB, not guessed) until ICRenderer.cpp gained a defensive
    // QApplication-existence guard; see project memory
    // project_ic_preview_pixmap_needs_qapplication.md for the full finding and fix, and the
    // plan's "Phase 2 in depth" section, corrected there. A missing Examples/ directory in
    // some other run environment still degrades to an unloaded (still paints fine, just
    // empty) IC rather than crashing.
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
        const quint64 id = elementId(element->id());
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
        m_index.insertShape(wireId(connection->id()), stroke.boundingRect(), stroke);
    }
}

void CanvasItem::activateOnPress(GraphicElement *element)
{
    if (auto *inputSwitch = qobject_cast<InputSwitch *>(element)) {
        inputSwitch->setOn(!inputSwitch->isOn(), 0);
        return;
    }
    if (auto *inputRotary = qobject_cast<InputRotary *>(element)) {
        // Mirrors InputRotary::mousePressEvent's "setOn(true, (m_currentPort + 1) %
        // outputSize())" -- outputValue() is the public equivalent of the private
        // m_currentPort it reads (InputRotary::outputValue() returns "the index of the
        // currently active output port").
        inputRotary->setOn(true, (inputRotary->outputValue() + 1) % inputRotary->outputSize());
    }
    // InputButton isn't dispatched through here: its momentary on-while-held behavior needs
    // a release counterpart too, so it's handled directly in mousePressEvent()/
    // mouseReleaseEvent() via m_pressedInputButton instead of this press-only dispatcher.
}

QString CanvasItem::appearanceKeyFor(GraphicElement *element)
{
    // appearanceCacheKey() alone already distinguishes different elements showing different
    // pixmaps (and correctly *shares* a cache entry between elements showing identical ones,
    // e.g. two unrotated And gates) since it tracks the live QPixmap's own identity -- see
    // this class's doc comment for why that's enough for every ported family except
    // Display7/14/16, whose segment overlays paint on top of an unchanged base pixmap.
    QString key = QStringLiteral("%1|%2|%3|%4|%5")
        .arg(element->appearanceCacheKey())
        .arg(element->rotation())
        .arg(element->isFlippedX())
        .arg(element->isFlippedY())
        .arg(element->isSelected());

    // Display7/14/16::paint() reads each input port's live Status directly to decide which
    // segment overlays to draw on top of the (unchanged) base pixmap -- append them explicitly
    // so a segment toggling actually invalidates the cached tile. No shared base class exists
    // to test for generically (see this method's usage site discussion), so this is a
    // deliberate, narrowly-scoped type check rather than a generic mechanism -- exactly the
    // one extra cache-key dimension "Phase 2 in depth" flagged this family as needing.
    if (qobject_cast<Display7 *>(element) || qobject_cast<Display14 *>(element) || qobject_cast<Display16 *>(element)) {
        for (int i = 0; i < element->inputSize(); ++i) {
            key += QLatin1Char('|');
            key += QString::number(int(element->inputPort(i)->status()));
        }
    }

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

    if (!ConnectionManager::isConnectionAllowed(startPort, endPort)) {
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
    m_lastMousePos = event->position();

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
        // SceneInteraction::mousePress's "item->setSelected(!item->isSelected())" -- including
        // the real QGraphicsItem::setSelected() flag, so real paint()'s own selection-outline
        // drawing (ElementAppearance::render()'s `if (selected) {...}` branch) reflects it.
        if (m_selectedIds.contains(topHit)) {
            m_selectedIds.remove(topHit);
            element->setSelected(false);
        } else {
            m_selectedIds.insert(topHit);
            element->setSelected(true);
        }
    }

    activateOnPress(element);
    if (auto *inputButton = qobject_cast<InputButton *>(element)) {
        // Momentary: on for as long as the button is held, matching real
        // InputButton::mousePressEvent()/mouseReleaseEvent(). See m_pressedInputButton's doc
        // comment for why release is tracked here rather than re-hit-tested on release.
        inputButton->setOn();
        m_pressedInputButton = inputButton;
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
    m_dragAnchor = event->position();
    m_draggingElement = true;

    update();
}

void CanvasItem::mouseMoveEvent(QMouseEvent *event)
{
    m_lastMousePos = event->position();

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

    if (m_pressedInputButton) {
        if (auto *inputButton = qobject_cast<InputButton *>(m_pressedInputButton)) {
            inputButton->setOff();
        }
        m_pressedInputButton = nullptr;
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
    // setSelectionArea() call has the same replace semantics by default. Clear the real
    // QGraphicsItem::setSelected() flag too, so real paint()'s selection outline matches.
    for (const quint64 id : std::as_const(m_selectedIds)) {
        if (auto *element = m_elementsById.value(id, nullptr)) {
            element->setSelected(false);
        }
    }
    m_selectedIds.clear();
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

    // Sync the real QGraphicsItem::setSelected() flag for exactly the delta -- elements
    // leaving the rubber band vs. entering it -- rather than clearing everything and
    // resetting, so this stays correct even if called every mouse-move during a drag.
    for (const quint64 id : std::as_const(m_selectedIds)) {
        if (!newSelection.contains(id)) {
            if (auto *element = m_elementsById.value(id, nullptr)) {
                element->setSelected(false);
            }
        }
    }
    for (const quint64 id : std::as_const(newSelection)) {
        if (!m_selectedIds.contains(id)) {
            if (auto *element = m_elementsById.value(id, nullptr)) {
                element->setSelected(true);
            }
        }
    }
    m_selectedIds = std::move(newSelection);
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

void CanvasItem::hoverMoveEvent(QHoverEvent *event)
{
    const auto hits = m_index.queryPoint(event->position());
    const quint64 newHoveredId = hits.isEmpty() ? 0 : hits.last();

    if (newHoveredId != m_hoveredId) {
        // Leaving the old element first, entering the new one after, mirrors real Scene
        // behavior (and IC's own preview show/hide request pairing -- see this class's doc
        // comment on the hover-preview signal chain).
        if (auto *oldIc = qobject_cast<IC *>(m_elementsById.value(m_hoveredId, nullptr))) {
            oldIc->previewHideRequested();
        }
        m_hoveredId = newHoveredId;
        if (auto *newIc = qobject_cast<IC *>(m_elementsById.value(newHoveredId, nullptr))) {
            if (isOverOwnPort(newIc, event->position())) {
                newIc->previewHideRequested();
            } else {
                newIc->previewRequested(newIc, event->globalPosition().toPoint());
            }
        }
        update();
    } else if (auto *ic = qobject_cast<IC *>(m_elementsById.value(m_hoveredId, nullptr))) {
        // Same IC, cursor still moving within it -- keep the pending-preview position current.
        if (isOverOwnPort(ic, event->position())) {
            ic->previewHideRequested();
        } else {
            ic->previewMoved(ic, event->globalPosition().toPoint());
        }
    }
}

void CanvasItem::hoverLeaveEvent(QHoverEvent *)
{
    if (m_hoveredId != 0) {
        if (auto *ic = qobject_cast<IC *>(m_elementsById.value(m_hoveredId, nullptr))) {
            ic->previewHideRequested();
        }
        m_hoveredId = 0;
        update();
    }
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

    // Arrow keys nudge the current selection by a grid step (Shift = a larger step) as one
    // undoable move; consumes the event only when it actually moves a selection.
    if (nudgeSelection(event)) {
        return;
    }

    // Rotate/flip: no chrome menu/QAction shortcut layer exists yet (Phase 4) to intercept
    // these before they reach here the way the real app's MainWindowUI.cpp-bound QActions do,
    // so they're matched directly -- see this class's doc comment on rotateRight()/etc. for
    // why flipVertically() has no binding here (it has none in the real app either).
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
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

    // Skip keyboard triggers while Ctrl is held, so the Ctrl+R/Ctrl+H shortcuts above (and
    // future Ctrl+Z/C/V shortcuts, later Phase 3 sub-steps) don't also fire an element's
    // trigger key -- mirrors Scene::keyPressEvent()'s identical guard.
    if (!event->modifiers().testFlag(Qt::ControlModifier)) {
        for (auto *element : std::as_const(m_elements)) {
            if (element->hasTrigger() && !element->trigger().isEmpty() && element->trigger().matches(event->key())) {
                if (auto *input = qobject_cast<GraphicElementInput *>(element); input && !input->isLocked()) {
                    input->setOn();
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

    if (!event->modifiers().testFlag(Qt::ControlModifier)) {
        for (auto *element : std::as_const(m_elements)) {
            if (element->hasTrigger() && !element->trigger().isEmpty() && element->trigger().matches(event->key())) {
                // Only InputButton (momentary) is released on key-up; InputSwitch stays latched.
                if (auto *input = qobject_cast<GraphicElementInput *>(element);
                    input && !input->isLocked() && (element->elementType() == ElementType::InputButton)) {
                    input->setOff();
                }
            }
        }
    }

    QQuickItem::keyReleaseEvent(event);
}

QSGNode *CanvasItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    // Element/wire positions can change now (drag-to-move rebuilds m_index on every move, see
    // mouseMoveEvent()); this method itself only ever reads current live state on each repaint,
    // it never assumes positions are static.
    auto *root = oldNode ? oldNode : new QSGNode();
    if (!oldNode) {
        auto *wireNode = new QSGGeometryNode();
        auto *hoverNode = new QSGGeometryNode(); // hover highlight, underneath the gate it highlights
        auto *gateNode = new QSGGeometryNode();  // real per-element appearance, textured
        auto *overlayNode = new QSGGeometryNode(); // live rubber-band rect, paints on top of everything
        root->appendChildNode(wireNode); // wires paint first
        root->appendChildNode(hoverNode);
        root->appendChildNode(gateNode);
        root->appendChildNode(overlayNode);
    }

    QSGNode *wireNode = root->firstChild();
    QSGNode *hoverNode = wireNode->nextSibling();
    QSGNode *gateNode = hoverNode->nextSibling();
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

    // --- Hover highlight: a padded translucent quad under the hovered element only, in place
    // of the flat-color brightening this prototype used before real appearance rendering --
    // a QSGTextureMaterial has no per-vertex tint to brighten, so this is a separate underlay,
    // the same technique the marquee/selection overlays already use. ---
    {
        GraphicElement *hovered = m_elementsById.value(m_hoveredId, nullptr);
        const int vertexCount = hovered ? 6 : 0;
        auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), vertexCount);
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        if (hovered) {
            QSGGeometry::ColoredPoint2D *vertices = geometry->vertexDataAsColoredPoint2D();
            int cursor = 0;
            static const QColor kHoverColor(255, 255, 255, 90);
            const QRectF worldRect = hovered->boundingRect().translated(hovered->pos());
            appendQuad(vertices, cursor, worldRect.adjusted(-4, -4, 4, 4), kHoverColor);
        }
        static_cast<QSGGeometryNode *>(hoverNode)->setGeometry(geometry);
        static_cast<QSGGeometryNode *>(hoverNode)->setFlag(QSGNode::OwnsGeometry);
        if (!static_cast<QSGGeometryNode *>(hoverNode)->material()) {
            auto *material = new QSGVertexColorMaterial();
            static_cast<QSGGeometryNode *>(hoverNode)->setMaterial(material);
            static_cast<QSGGeometryNode *>(hoverNode)->setFlag(QSGNode::OwnsMaterial);
        }
    }

    // --- Gates: one QSGGeometryNode, GL_TRIANGLES, textured. Real per-element appearance --
    // each element's own real, unmodified paint() (dispatched polymorphically) rendered
    // offscreen and cached via m_atlas; see this class's doc comment. Selection is now the
    // real QGraphicsItem::setSelected() flag baked into that paint() call (real
    // ElementAppearance::render() draws its own selection-outline rectangle), not a separate
    // underlay quad like Phase 1's flat-colored placeholder used. ---
    {
        struct PlacedTile {
            QRectF worldRect;
            QRectF uv;
        };
        QVector<PlacedTile> placed;
        placed.reserve(m_elements.size());

        for (auto *element : std::as_const(m_elements)) {
            const QRectF localRect = element->boundingRect();
            const QSize tileSize = localRect.size().toSize();
            const QString key = appearanceKeyFor(element);
            const auto tile = m_atlas.lookup(key, tileSize, [element](QPainter &painter) {
                painter.translate(-element->boundingRect().topLeft());
                element->paint(&painter, nullptr, nullptr);
            });
            if (!tile.isValid()) {
                continue; // atlas page full or a degenerate (empty) boundingRect -- skip, don't crash
            }
            placed.append(PlacedTile{localRect.translated(element->pos()), tile.uv});
        }

        auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), int(placed.size()) * 6);
        geometry->setDrawingMode(QSGGeometry::DrawTriangles);
        QSGGeometry::TexturedPoint2D *vertices = geometry->vertexDataAsTexturedPoint2D();
        int cursor = 0;
        for (const auto &tile : std::as_const(placed)) {
            appendTexturedQuad(vertices, cursor, tile.worldRect, tile.uv);
        }
        static_cast<QSGGeometryNode *>(gateNode)->setGeometry(geometry);
        static_cast<QSGGeometryNode *>(gateNode)->setFlag(QSGNode::OwnsGeometry);

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
