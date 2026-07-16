// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/Scene.h"

#include <algorithm>
#include <cmath>

#include <QClipboard>
#include <QDrag>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneHelpEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMimeData>
#include <QPolygon>
#include <QScrollBar>
#include <QUrl>

#include "App/Core/Common.h"
#include "App/Core/Constants.h"
#include "App/Core/ItemWithId.h"
#include "App/Core/MimeTypes.h"
#include "App/Core/Priorities.h"
#include "App/Core/SentryHelpers.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/GraphicElements/AudioOutputElement.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Commands.h"
#include "App/Scene/GraphicsView.h"
#include "App/Wiring/Connection.h"

namespace {
// Adaptive wire antialiasing constants (see the Scene.h accessor block for the design).
// All are time-based or dimensionless, so their meaning is machine-independent.

/// A paint pass slower than this hurts interactivity (~3 dropped frames at 60 Hz).
constexpr qint64 kWireAaBudgetNs = 50'000'000;

/// Consecutive over-budget passes required to degrade -- a one-off compositor stall or
/// background-load spike never flips quality.
constexpr int kWireAaDebouncePasses = 2;

/// No paint pass for this long means the current gesture / repaint burst is over.
constexpr qint64 kWireAaPassIdleMs = 300;

/// No wire status flip for this long means the simulation-driven storm is over. Must span
/// the quiet gap between clock edges (flips arrive in per-edge bursts): 3 s covers clocks
/// down to ~0.17 Hz, far below typical educational frequencies.
constexpr qint64 kWireAaFlipIdleMs = 3000;

/// Conservative upper bound on how much more an antialiased pass costs than a plain one
/// (measured ~5.5x on a wire-dominated scene). Deep headroom is budget / this, so a pass
/// restored from deep headroom fits the budget by construction and can't re-trip the
/// degrade -- the property that keeps a binary quality knob from oscillating.
constexpr qint64 kWireAaWorstCaseRatio = 10;

/// Consecutive deep-headroom passes required for the sustained-headroom restore
/// (~170 ms of simulation-driven passes at 60 Hz).
constexpr int kWireAaHeadroomRestorePasses = 10;
} // namespace

Scene::Scene(QObject *parent)
    : QGraphicsScene(parent)
    , m_simulation(this, this)
{
    // The scene filters its own events to intercept mouse events before items see them
    // (e.g., to detect Ctrl+drag for cloning before Qt's default drag-selection activates)
    installEventFilter(this);

    // Attach the interaction layer: adds its rubber-band selection rectangle to the
    // scene and starts its drag throttle timer.
    m_interaction.attachToScene();

    m_undoAction = new QAction(tr("&Undo"), this);
    m_undoAction->setEnabled(false);
    m_undoAction->setIcon(QIcon(":/Interface/Toolbar/undo.svg"));
    m_undoAction->setShortcut(QKeySequence::Undo);
    connect(&m_undoStack, &QUndoStack::canUndoChanged, m_undoAction, &QAction::setEnabled);
    connect(&m_undoStack, &QUndoStack::undoTextChanged, this, &Scene::updateUndoText);
    connect(m_undoAction, &QAction::triggered, &m_undoStack, &QUndoStack::undo);

    m_redoAction = new QAction(tr("&Redo"), this);
    m_redoAction->setEnabled(false);
    m_redoAction->setIcon(QIcon(":/Interface/Toolbar/redo.svg"));
    m_redoAction->setShortcut(QKeySequence::Redo);
    connect(&m_undoStack, &QUndoStack::canRedoChanged, m_redoAction, &QAction::setEnabled);
    connect(&m_undoStack, &QUndoStack::redoTextChanged, this, &Scene::updateRedoText);
    connect(m_redoAction, &QAction::triggered, &m_undoStack, &QUndoStack::redo);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &Scene::updateTheme);
    // Emit autosave signal only after each undo-stack index change (not on every internal state update)
    connect(&m_undoStack,              &QUndoStack::indexChanged,   this, &Scene::checkUpdateRequest);

    // Primary restore path for adaptive wire antialiasing: the timeout re-checks both
    // activity timestamps and restores only once their windows have truly elapsed.
    m_wireAaIdleTimer.setSingleShot(true);
    connect(&m_wireAaIdleTimer, &QTimer::timeout, this, [this] { checkWireIdleRestore(); });
    m_wireFlipTimer.start();
    m_wirePassTimer.start();
}

void Scene::checkUpdateRequest()
{
    // Coalesces multiple rapid undo-stack changes into a single autosave signal:
    // the flag is set by setCircuitUpdateRequired() and cleared here after emitting.
    if (m_autosaveRequired) {
        emit circuitHasChanged();
        m_autosaveRequired = false;
    }
}

void Scene::addItem(QGraphicsItem *item)
{
    if (!item) {
        return;
    }
    QGraphicsScene::addItem(item);
    if (auto *iwid = dynamic_cast<ItemWithId *>(item)) {
        if (iwid->id() < 0) {
            // Unassigned item: give it the next scene-local ID
            iwid->setId(nextId());
        } else {
            // Pre-assigned item (restored by updateItemId for undo/redo): preserve it
            setLastId(iwid->id());
        }
        registerItem(iwid);
    }

    // Register element-type-specific hooks
    if (item->type() == GraphicElement::Type) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (!elm) {
            return;
        }
        connect(elm, &GraphicElement::inlineEditRequested, this, [this](GraphicElement *element) {
            m_inlineLabelEditor.start(element);
        });
        if (auto *ic = qobject_cast<IC *>(elm)) {
            if (!ic->file().isEmpty()) {
                m_icRegistry.watchFile(ic->file());
            }
            connect(ic, &IC::requestOpenSubCircuit, this, &Scene::icOpenRequested);
            connect(ic, &IC::previewRequested, this, &Scene::icPreviewRequested);
            connect(ic, &IC::previewMoved, this, &Scene::icPreviewMoved);
            connect(ic, &IC::previewHideRequested, this, &Scene::icPreviewHideRequested);
            connect(ic, &IC::previewCancelRequested, this, &Scene::icPreviewCancelRequested);
        } else if (auto *tt = qobject_cast<TruthTable *>(elm)) {
            connect(tt, &TruthTable::requestOpenTruthTableEditor, this, &Scene::openTruthTableRequested);
        }
    }
}

void Scene::removeItem(QGraphicsItem *item)
{
    if (!item) {
        return;
    }
    if (auto *iwid = dynamic_cast<ItemWithId *>(item)) {
        unregisterItem(iwid);
    }
    QGraphicsScene::removeItem(item);
}

ItemWithId *Scene::itemById(const int id) const
{
    return m_itemRegistry.itemById(id);
}

bool Scene::contains(const int id) const
{
    return m_itemRegistry.contains(id);
}

int Scene::lastId() const
{
    return m_itemRegistry.lastId();
}

void Scene::setLastId(const int newLastId)
{
    m_itemRegistry.setLastId(newLastId);
}

int Scene::nextId()
{
    return m_itemRegistry.nextId();
}

void Scene::updateItemId(ItemWithId *item, const int newId)
{
    m_itemRegistry.updateItemId(item, newId);
}

void Scene::forgetItemId(const int id)
{
    m_itemRegistry.forgetItemId(id);
}

void Scene::registerItem(ItemWithId *item)
{
    m_itemRegistry.registerItem(item);
}

void Scene::unregisterItem(ItemWithId *item)
{
    m_itemRegistry.unregisterItem(item);
}

SerializationContext Scene::deserializationContext(QHash<quint64, Port *> &portMap, const QVersionNumber &version, SerializationPurpose purpose)
{
    SerializationContext context = {.portMap = portMap, .version = version, .purpose = purpose, .contextDir = contextDir()};
    context.blobRegistry = &m_icRegistry.blobMapRef();
    return context;
}

void Scene::drawBackground(QPainter *painter, const QRectF &rect)
{
    // Flush any pending visibility reapply before items are painted: drawBackground()
    // always runs before Qt's item-painting pass for this same frame (Qt calls
    // drawBackground(), then paints visible items, then drawForeground()), so applying
    // setVisible() here still takes effect for the frame about to be drawn. See
    // setPropertyUpdateRequired() for why this is deferred instead of done eagerly there.
    if (m_visibilityDirty) {
        m_visibilityDirty = false;
        m_visibilityManager.reapply();
    }

    // m11() is the X-axis scale factor of the view transform; below 0.3 the grid dots
    // would be sub-pixel and invisible anyway, so skip drawing for performance
    if (view() and view()->transform().m11() < 0.3) {
        return;
    }

    QGraphicsScene::drawBackground(painter, rect);

    const int gridSize = Constants::gridSize;
    const int left = static_cast<int>(std::floor(rect.left() / gridSize)) * gridSize;
    const int top = static_cast<int>(std::floor(rect.top() / gridSize)) * gridSize;
    const int right = static_cast<int>(std::ceil(rect.right() / gridSize)) * gridSize;
    const int bottom = static_cast<int>(std::ceil(rect.bottom() / gridSize)) * gridSize;

    // Bound the grid regardless of view zoom: \a rect isn't necessarily clipped to the
    // interactive view's viewport — a headless scene->render() call (CircuitExporter's
    // PDF/image export) passes the *entire* scene bounding rect as the exposed area, which
    // for a scene containing one element at an extreme-but-finite position (the only check
    // on load rejects non-finite, not large, coordinates) can span millions of grid cells.
    // Left unchecked, the loop below builds a point list with billions of entries — hanging
    // and exhausting memory well before drawPoints() ever runs. Compute in 64-bit: the naive
    // int product overflows for exactly this scenario, wrapping to a small/negative value
    // that would pass a small reserve() through to an unbounded loop anyway.
    const qint64 columns = (static_cast<qint64>(right) - left) / gridSize + 1;
    const qint64 rows = (static_cast<qint64>(bottom) - top) / gridSize + 1;
    constexpr qint64 kMaxGridPoints = 1'000'000;
    if (columns * rows > kMaxGridPoints) {
        return;
    }

    painter->setPen(m_dots);

    QPolygon points;
    // kMaxGridPoints (checked above) comfortably fits in int on every supported platform;
    // cast explicitly rather than relying on the implicit qint64->qsizetype narrowing, since
    // qsizetype is 32-bit on i386 (would warn under -Wconversion) but the same width as qint64
    // on 64-bit platforms (a same-width static_cast there would instead warn -Wuseless-cast).
    points.reserve(static_cast<int>(columns * rows));

    for (int x = left; x <= right; x += gridSize) {
        for (int y = top; y <= bottom; y += gridSize) {
            points.append(QPoint(x, y));
        }
    }

    painter->drawPoints(points);
}

void Scene::setDots(const QPen &dots)
{
    m_dots = dots;
}

Simulation *Scene::simulation()
{
    return &m_simulation;
}

void Scene::setAutosaveRequired()
{
    m_autosaveRequired = true;
}

void Scene::setCircuitUpdateRequired()
{
    setPropertyUpdateRequired();

    // Invalidate the cached topology; the next Simulation::update() (or start())
    // lazily rebuilds it. Structural edits often arrive in tight batches (MCP-driven
    // circuit construction, multi-element paste/delete) — eagerly rebuilding here on
    // every single one, as this used to do, turned an O(N) circuit build into O(N^2):
    // each element/connection add paid a full topology rebuild over everything already
    // in the scene. restart() alone is sufficient: it already unconditionally clears
    // m_initialized, so it stays correct even on the "scene dropped to just the border
    // rect" bail-out case that previously needed the eager initialize() call to detect.
    m_simulation.restart();
}

void Scene::setPropertyUpdateRequired()
{
    // Re-applying visibility ensures newly-added ports/wires respect the current
    // show/hide state; without this, ports on fresh elements would always appear visible.
    // Deferred to the next drawBackground() (see its comment) rather than done eagerly
    // here: this runs on every structural edit, and reapply() rescans the whole scene,
    // which is fine for one edit but pathological for a tight batch of them (MCP-driven
    // circuit construction, multi-element paste/delete) — O(N) per edit, O(N^2) overall.
    m_visibilityDirty = true;

    update();

    m_autosaveRequired = true;
    m_itemsBoundingRectDirty = true;
}

const QVector<GraphicElement *> Scene::visibleElements() const
{
    const auto visibleRect = m_view->mapToScene(m_view->viewport()->geometry()).boundingRect();

    return elements(visibleRect);
}

const QVector<GraphicElement *> Scene::elements() const
{
    return sortByTopology(unsortedElements());
}

const QVector<GraphicElement *> Scene::unsortedElements() const
{
    const auto items_ = items();
    QVector<GraphicElement *> elements_;
    elements_.reserve(items_.size());

    for (auto *item : items_) {
        if (item->type() == GraphicElement::Type) {
            elements_.append(qgraphicsitem_cast<GraphicElement *>(item));
        }
    }

    return elements_;
}

const QVector<GraphicElement *> Scene::elements(const QRectF &rect) const
{
    const auto items_ = items(rect);
    QVector<GraphicElement *> elements_;
    elements_.reserve(items_.size());

    for (auto *item : items_) {
        if (item->type() == GraphicElement::Type) {
            elements_.append(qgraphicsitem_cast<GraphicElement *>(item));
        }
    }

    return sortByTopology(elements_);
}

QVector<GraphicElement *> Scene::sortByTopology(QVector<GraphicElement *> elements)
{
    QHash<GraphicElement *, QVector<GraphicElement *>> successors;
    for (auto *elm : elements) {
        for (auto *port : elm->outputs()) {
            for (auto *conn : port->connections()) {
                if (auto *endPort = conn->endPort()) {
                    if (auto *successor = endPort->graphicElement()) {
                        auto &vec = successors[elm];
                        if (!vec.contains(successor)) {
                            vec.append(successor);
                        }
                    }
                }
            }
        }
    }

    QHash<GraphicElement *, int> priorities;
    calculatePriorities(elements, successors, priorities);

    std::stable_sort(elements.begin(), elements.end(), [&priorities](const auto &e1, const auto &e2) {
        return priorities.value(e1) > priorities.value(e2);
    });

    return elements;
}

QHash<QString, InputPort *> Scene::wirelessTxInputPorts(const QVector<GraphicElement *> &elements)
{
    QHash<QString, InputPort *> txMap;
    for (auto *elm : elements) {
        if (elm->wirelessMode() == WirelessMode::Tx && !elm->label().isEmpty() && elm->inputPort(0)) {
            if (!txMap.contains(elm->label())) {
                txMap.insert(elm->label(), elm->inputPort(0));
            }
        }
    }
    return txMap;
}

const QVector<Connection *> Scene::connections() const
{
    const auto items_ = items();
    QVector<Connection *> conns;
    conns.reserve(items_.size());

    for (auto *item : items_) {
        if (item->type() == Connection::Type) {
            conns.append(qgraphicsitem_cast<Connection *>(item));
        }
    }

    return conns;
}

const QList<GraphicElement *> Scene::selectedElements() const
{
    const auto items_ = selectedItems();
    QList<GraphicElement *> elements_;

    for (auto *item : items_) {
        if (item->type() == GraphicElement::Type) {
            elements_.append(qgraphicsitem_cast<GraphicElement *>(item));
        }
    }

    return elements_;
}

QGraphicsItem *Scene::itemAt(const QPointF pos) const
{
    auto items_ = items(pos);
    // Also check a small surrounding rectangle so port hit-testing works when
    // the cursor is near but not exactly on the port's bounding box
    items_.append(itemsAt(pos));

    // Ports get priority so wire dragging begins/ends on the port, not the element body
    for (auto *item : std::as_const(items_)) {
        if (item->type() == Port::Type) {
            return item;
        }
    }

    // Elements take priority over connections since they render above wires
    for (auto *item : std::as_const(items_)) {
        if (item->type() == GraphicElement::Type) {
            return item;
        }
    }

    // Return any remaining custom item (connections, etc.)
    for (auto *item : std::as_const(items_)) {
        if (item->type() > QGraphicsItem::UserType) {
            return item;
        }
    }

    return nullptr;
}

QList<QGraphicsItem *> Scene::itemsAt(const QPointF pos) const
{
    // 9×9 pixel hit area (4px margin around the exact point) compensates for the
    // small visual size of ports and makes them easier to click precisely
    QRectF rect(pos - QPointF(4, 4), QSize(9, 9));
    return items(rect.normalized());
}

Port *Scene::portAt(const QPointF pos) const
{
    // Port-only fast path for per-mouse-move consumers (hover feedback, wire completion,
    // port tooltips). itemAt()'s generic queries use Qt::IntersectsItemShape, which
    // exact-shape-tests every element and wire whose bounding box overlaps -- for wires
    // that means re-stroking and clipping Bézier curves, and their fat bounding boxes
    // collect dozens of candidates near a wire bundle. A bounding-box query skips all of
    // that, and ports themselves are cheap to test exactly (shape() is a fixed square).
    //
    // Same 9x9 hit area as itemsAt(). An exactly-hit port wins over a merely-nearby one,
    // like itemAt()'s two-phase lookup; among nearby-only ports acceptance is by bounding
    // box rather than shape-vs-rect -- a difference of at most the ~1 unit of bounding-box
    // padding, well inside the deliberate 4 px slop.
    const QRectF rect(pos - QPointF(4, 4), QSize(9, 9));
    const auto candidates = items(rect.normalized(), Qt::IntersectsItemBoundingRect);

    Port *nearby = nullptr;

    for (auto *item : candidates) { // default order: topmost first
        if (item->type() != Port::Type) {
            continue;
        }

        // static_cast, not qgraphicsitem_cast: the type() check above already proves the
        // downcast, and the maybe-null result of qgraphicsitem_cast trips
        // -Wnull-dereference at -O3 on the immediate dereference below.
        auto *port = static_cast<Port *>(item);

        if (port->shape().contains(port->mapFromScene(pos))) {
            return port;
        }

        if (!nearby) {
            nearby = port;
        }
    }

    return nearby;
}

void Scene::receiveCommand(QUndoCommand *cmd)
{
    sentryBreadcrumb("command", QStringLiteral("Command: %1").arg(cmd->text()));
    m_undoStack.push(cmd);
    update();
}

QRectF Scene::cachedItemsBoundingRect() const
{
    // A live drag needs the item bounds to track the dragged element's currently-changing
    // position (see resizeScene()'s isDraggingElement() branch) -- a cached value would
    // stop the scene rect from expanding to follow it mid-drag.
    if (m_interaction.isDraggingElement()) {
        return itemsBoundingRect();
    }

    if (m_itemsBoundingRectDirty) {
        m_cachedItemsBoundingRect = itemsBoundingRect();
        m_itemsBoundingRectDirty = false;
    }

    return m_cachedItemsBoundingRect;
}

bool Scene::wireAntialiasingEnabled() const
{
    return m_wireAntialiasing;
}

void Scene::recordWirePaintPass(const qint64 elapsedNs)
{
    m_wirePassTimer.restart();

    if (m_wireAntialiasing) {
        if (elapsedNs > kWireAaBudgetNs) {
            if (++m_wireAaSlowPasses >= kWireAaDebouncePasses) {
                m_wireAntialiasing = false;
                m_wireAaSlowPasses = 0;
                m_wireAaHeadroomPasses = 0;
                m_wireAaIdleTimer.start(static_cast<int>(kWireAaPassIdleMs));
            }
        } else {
            m_wireAaSlowPasses = 0;
        }
        return;
    }

    // Degraded: sustained deep headroom is the only in-storm way back to full quality;
    // the idle path is handled by checkWireIdleRestore() re-arming itself.
    if (elapsedNs < kWireAaBudgetNs / kWireAaWorstCaseRatio) {
        if (++m_wireAaHeadroomPasses >= kWireAaHeadroomRestorePasses) {
            restoreWireAntialiasing();
        }
    } else {
        m_wireAaHeadroomPasses = 0;
    }
}

void Scene::noteWireActivity()
{
    m_wireFlipTimer.restart();
}

void Scene::checkWireIdleRestore()
{
    const qint64 passRemainingMs = kWireAaPassIdleMs - m_wirePassTimer.elapsed();
    const qint64 flipRemainingMs = kWireAaFlipIdleMs - m_wireFlipTimer.elapsed();
    const qint64 remainingMs = qMax(passRemainingMs, flipRemainingMs);

    if (remainingMs <= 0) {
        restoreWireAntialiasing();
        return;
    }

    // Still active: re-arm for the remainder of the longer window (deadline pattern --
    // avoids restarting a timer on every one of the thousands of flips per second).
    m_wireAaIdleTimer.start(static_cast<int>(remainingMs));
}

void Scene::restoreWireAntialiasing()
{
    m_wireAaIdleTimer.stop();
    m_wireAaSlowPasses = 0;
    m_wireAaHeadroomPasses = 0;

    if (m_wireAntialiasing) {
        return;
    }

    m_wireAntialiasing = true;
    // Refinement repaint: without it the wires would stay visually jagged until the next
    // naturally-occurring repaint, which on an idle view may never come.
    update();
}

namespace {
/// Any *changed* scene rect makes Qt's BSP index unindex and re-insert every item in the
/// scene (QGraphicsSceneBspTreeIndex::updateSceneRect() -> resetIndex()) -- tens of
/// milliseconds on large circuits; only a bit-identical rect is a no-op. Snapping the rect
/// outward to this grid makes consecutive interaction steps (mouse moves of an edge drag,
/// zoom wheel notches) produce identical rects, turning per-event full re-indexes into at
/// most one per 256 scene units of actual growth or shrink. The scene rect's only
/// observable effect is scrollbar range, which already carries a 100-unit margin.
constexpr qreal kSceneRectQuantum = 256.0;

QRectF quantizeOutward(const QRectF &rect)
{
    const qreal left = std::floor(rect.left() / kSceneRectQuantum) * kSceneRectQuantum;
    const qreal top = std::floor(rect.top() / kSceneRectQuantum) * kSceneRectQuantum;
    const qreal right = std::ceil(rect.right() / kSceneRectQuantum) * kSceneRectQuantum;
    const qreal bottom = std::ceil(rect.bottom() / kSceneRectQuantum) * kSceneRectQuantum;
    return QRectF(QPointF(left, top), QPointF(right, bottom));
}
} // namespace

void Scene::resizeScene()
{
    const auto bounds = cachedItemsBoundingRect();

    if (m_interaction.isDraggingElement()) {
        // While dragging, only expand the scene rect (union with current rect).
        // Never shrink during drag — shrinking shifts the viewport origin and
        // causes jarring visual jumps as the scene rect chases the items.
        setSceneRect(quantizeOutward(sceneRect().united(bounds)));
    } else {
        // Tighten to item bounds, but ensure the scene rect stays larger than the
        // viewport. When the scene rect is smaller than (or barely larger than) the
        // viewport, Qt re-centers it, causing a visual jump. Adding margins ensures
        // enough scrollbar range to preserve the exact scroll position.
        auto tightRect = bounds;
        const auto viewList = views();
        if (!viewList.isEmpty()) {
            auto *view = viewList.first();
            constexpr qreal margin = 100.0;
            const auto visibleScene = view->mapToScene(view->viewport()->rect()).boundingRect()
                                         .adjusted(-margin, -margin, margin, margin);
            tightRect = tightRect.united(visibleScene);

            // Preserve exact scrollbar positions across the scene rect change.
            // centerOn() converts scene floats to integer pixels, causing 1px
            // drift — saving/restoring scrollbar values avoids the rounding.
            const int hVal = view->horizontalScrollBar()->value();
            const int vVal = view->verticalScrollBar()->value();
            setSceneRect(quantizeOutward(tightRect));
            view->horizontalScrollBar()->setValue(hVal);
            view->verticalScrollBar()->setValue(vVal);
        } else {
            setSceneRect(quantizeOutward(tightRect));
        }
    }
}

QUndoStack *Scene::undoStack()
{
    return &m_undoStack;
}

bool Scene::isConnectionAllowed(OutputPort *startPort, InputPort *endPort)
{
    return ConnectionManager::isConnectionAllowed(startPort, endPort);
}

void Scene::showStatusMessage(const QString &message)
{
    emit showStatusMessageRequested(message);
}

void Scene::prevMainPropShortcut() { m_propertyShortcutHandler.prevMainProperty(); }

void Scene::nextMainPropShortcut() { m_propertyShortcutHandler.nextMainProperty(); }

void Scene::prevSecndPropShortcut() { m_propertyShortcutHandler.prevSecondaryProperty(); }

void Scene::nextSecndPropShortcut() { m_propertyShortcutHandler.nextSecondaryProperty(); }

void Scene::nextElm() { m_propertyShortcutHandler.nextElement(); }

void Scene::prevElm() { m_propertyShortcutHandler.prevElement(); }

void Scene::updateTheme()
{
    qCDebug(zero) << "Updating theme.";
    const ThemeAttributes theme = ThemeManager::attributes();
    setBackgroundBrush(theme.m_sceneBgBrush);
    setDots(QPen(theme.m_sceneBgDots));
    m_interaction.applyTheme(theme);

    for (auto *element : elements()) {
        element->updateTheme();
    }

    for (auto *conn : connections()) {
        conn->updateTheme();
    }

    qCDebug(zero) << "Finished updating theme.";
}

QList<QGraphicsItem *> Scene::items(Qt::SortOrder order) const
{
    return QGraphicsScene::items(order);
}

QList<QGraphicsItem *> Scene::items(QPointF pos, Qt::ItemSelectionMode mode, Qt::SortOrder order, const QTransform &deviceTransform) const
{
    return QGraphicsScene::items(pos, mode, order, deviceTransform);
}

QList<QGraphicsItem *> Scene::items(const QRectF &rect, Qt::ItemSelectionMode mode, Qt::SortOrder order, const QTransform &deviceTransform) const
{
    return QGraphicsScene::items(rect, mode, order, deviceTransform);
}

void Scene::showGates(const bool checked)
{
    m_visibilityManager.showGates(checked);
}

void Scene::showWires(const bool checked)
{
    m_visibilityManager.showWires(checked);
}

GraphicsView *Scene::view() const
{
    return m_view;
}

void Scene::setView(GraphicsView *view)
{
    m_view = view;
}

QAction *Scene::undoAction() const
{
    return m_undoAction;
}

QAction *Scene::redoAction() const
{
    return m_redoAction;
}

void Scene::retranslateUi()
{
    updateUndoText(m_undoStack.undoText());
    updateRedoText(m_undoStack.redoText());
}

void Scene::updateUndoText(const QString &text)
{
    const QString prefix = tr("&Undo");
    m_undoAction->setText(text.isEmpty() ? prefix : prefix + QLatin1Char(' ') + text);
}

void Scene::updateRedoText(const QString &text)
{
    const QString prefix = tr("&Redo");
    m_redoAction->setText(text.isEmpty() ? prefix : prefix + QLatin1Char(' ') + text);
}

void Scene::contextMenu(const QPoint screenPos)
{
    if (auto *item = itemAt(m_interaction.lastMousePos())) {
        // Right-clicking a selected item opens the property context menu for that selection
        if (selectedItems().contains(item)) {
            emit contextMenuPos(screenPos, item);
        } else if (item->type() == GraphicElement::Type) {
            // Right-clicking an unselected element clears the old selection and selects
            // only this item, then opens its context menu
            clearSelection();
            item->setSelected(true);
            emit contextMenuPos(screenPos, item);
        }
    } else {
        // Right-click on empty canvas: Paste plus Select all.
        QMenu menu;
        auto *pasteAction = menu.addAction(QIcon(QPixmap(":/Interface/Toolbar/paste.svg")), tr("Paste"));
        const auto *mimeData = QApplication::clipboard()->mimeData();

        if (ClipboardManager::canPaste(mimeData)) {
            connect(pasteAction, &QAction::triggered, this, &Scene::pasteAction);
        } else {
            pasteAction->setEnabled(false);
        }

        auto *selectAllAction = menu.addAction(tr("Select all"));
        selectAllAction->setEnabled(!elements().isEmpty());
        connect(selectAllAction, &QAction::triggered, this, &Scene::selectAll);

        menu.exec(screenPos);
    }
}

void Scene::copyAction()
{
    sentryBreadcrumb("clipboard", QStringLiteral("Copy"));
    m_clipboardManager.copy();
}

void Scene::cutAction()
{
    sentryBreadcrumb("clipboard", QStringLiteral("Cut"));
    m_clipboardManager.cut();
}

void Scene::pasteAction()
{
    sentryBreadcrumb("clipboard", QStringLiteral("Paste"));
    m_clipboardManager.paste();
}

void Scene::duplicateAction()
{
    sentryBreadcrumb("clipboard", QStringLiteral("Duplicate"));
    m_clipboardManager.duplicate();
}

void Scene::deleteAction()
{
    sentryBreadcrumb("ui", QStringLiteral("Delete"));
    const auto selectedItems_ = selectedItems();
    // Clear selection before the command so that the scene's selectedItems()
    // list is empty during the command's redo() — avoids double-processing
    clearSelection();

    if (!selectedItems_.isEmpty()) {
        receiveCommand(new DeleteItemsCommand(selectedItems_, this));
        // No explicit Simulation::restart() needed: DeleteItemsCommand::redo
        // calls setCircuitUpdateRequired() which already drives a full
        // initialize()/restart() cycle once the deletion completes.
    }
}

void Scene::selectAll()
{
    const auto items_ = items();

    for (auto *item : items_) {
        item->setSelected(true);
    }
}

void Scene::rotateRight()
{
    sentryBreadcrumb("ui", QStringLiteral("Rotate right"));
    rotate(90);
}

void Scene::rotateLeft()
{
    sentryBreadcrumb("ui", QStringLiteral("Rotate left"));
    rotate(-90);
}

void Scene::rotate(const int angle)
{
    const auto elements_ = selectedElements();

    if (!elements_.isEmpty()) {
        receiveCommand(new RotateCommand(elements_, angle, this));
    }
}

void Scene::mute(const bool mute)
{
    for (auto *element : unsortedElements()) {
        if (auto *audioElement = qobject_cast<AudioOutputElement *>(element)) {
            audioElement->mute(mute);
        }
    }
}

void Scene::flipHorizontally()
{
    sentryBreadcrumb("ui", QStringLiteral("Flip horizontal"));
    const auto elements_ = selectedElements();

    if (!elements_.isEmpty()) {
        receiveCommand(new FlipCommand(elements_, 0, this));
    }
}

void Scene::flipVertically()
{
    sentryBreadcrumb("ui", QStringLiteral("Flip vertical"));
    const auto elements_ = selectedElements();

    if (!elements_.isEmpty()) {
        receiveCommand(new FlipCommand(elements_, 1, this));
    }
}

namespace {

/// Snapshots each element's current position, applies \a newPositions (already computed by the
/// caller), then pushes one MoveCommand for the whole batch -- the same pattern arrow-key
/// nudging already uses (see Scene::keyPressEvent()).
void moveElementsTo(Scene *scene, const QList<GraphicElement *> &elements, const QList<QPointF> &newPositions)
{
    QList<QPointF> oldPositions;
    oldPositions.reserve(elements.size());
    for (auto *elm : elements) {
        oldPositions.append(elm->pos());
    }

    for (int i = 0; i < elements.size(); ++i) {
        elements.at(i)->setPos(newPositions.at(i));
    }

    scene->receiveCommand(new MoveCommand(elements, oldPositions, scene));
}

} // namespace

void Scene::alignLeft()
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

void Scene::alignRight()
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

void Scene::alignTop()
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

void Scene::alignBottom()
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

void Scene::alignHorizontalCenter()
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

void Scene::alignVerticalCenter()
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

void Scene::distributeHorizontally()
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

void Scene::distributeVertically()
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

QString Scene::droppedPandaFile(const QMimeData *mimeData)
{
    if (!mimeData->hasUrls()) {
        return {};
    }
    for (const QUrl &url : mimeData->urls()) {
        if (!url.isLocalFile()) {
            continue;
        }
        const QString path = url.toLocalFile();
        if (path.endsWith(QLatin1String(".panda"), Qt::CaseInsensitive)) {
            return path;
        }
    }
    return {};
}

void Scene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (SceneDropHandler::isSupportedDropFormat(event->mimeData())
        || !droppedPandaFile(event->mimeData()).isEmpty()) {
        event->acceptProposedAction();
        return;
    }

    QGraphicsScene::dragEnterEvent(event);
}

void Scene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    if (SceneDropHandler::isSupportedDropFormat(event->mimeData())
        || !droppedPandaFile(event->mimeData()).isEmpty()) {
        event->acceptProposedAction();
        return;
    }

    QGraphicsScene::dragMoveEvent(event);
}

void Scene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    sentryBreadcrumb("ui", QStringLiteral("Drop event"));

    // A .panda file dragged from the file manager opens as a project (like File > Open).
    const QString pandaFile = droppedPandaFile(event->mimeData());
    if (!pandaFile.isEmpty()) {
        event->acceptProposedAction();
        emit fileDropRequested(pandaFile);
        return;
    }

    if (event->mimeData()->hasFormat(MimeType::DragDropLegacy)
        || event->mimeData()->hasFormat(MimeType::DragDrop)) {
        m_dropHandler.handleNewElementDrop(event);
    }

    if (event->mimeData()->hasFormat(MimeType::CloneDragLegacy)
        || event->mimeData()->hasFormat(MimeType::CloneDrag)) {
        m_dropHandler.handleCloneDrag(event);
    }

    QGraphicsScene::dropEvent(event);
}

bool Scene::nudgeSelection(QKeyEvent *event)
{
    // Only plain / Shift+arrow — leave Ctrl/Alt combinations to other handlers.
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
        return false; // nothing selected — let the arrow key scroll the view
    }

    // One grid cell by default; Shift jumps four cells for coarse positioning. Both are
    // multiples of the grid, so grid-aligned elements stay aligned.
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

    receiveCommand(new MoveCommand(selected, oldPositions, this));
    resizeScene();
    event->accept();
    return true;
}

void Scene::keyPressEvent(QKeyEvent *event)
{
    // Ignore auto-repeat: holding a trigger key must fire once, not oscillate an InputSwitch
    // (whose keyboard trigger toggles on every press) dozens of times a second.
    if (event->isAutoRepeat()) {
        QGraphicsScene::keyPressEvent(event);
        return;
    }

    // Arrow keys nudge the current selection by a grid step (Shift = a larger step) as one
    // undoable move; consumes the event only when it actually moves a selection.
    if (nudgeSelection(event)) {
        return;
    }

    // Skip keyboard triggers while Ctrl is held to avoid firing during Ctrl+Z/C/V shortcuts
    if (!(event->modifiers().testFlag(Qt::ControlModifier))) {
        for (auto *element : unsortedElements()) {
            if (element->hasTrigger() && !element->trigger().isEmpty() && element->trigger().matches(event->key())) {
                if (auto *input = qobject_cast<GraphicElementInput *>(element); input && !input->isLocked()) {
                    input->setOn();
                }
            }
        }
    }

    QGraphicsScene::keyPressEvent(event);
}

void Scene::keyReleaseEvent(QKeyEvent *event)
{
    // Ignore auto-repeat: on X11 a held key emits release/press pairs, which would otherwise
    // release a momentary InputButton mid-hold.
    if (event->isAutoRepeat()) {
        QGraphicsScene::keyReleaseEvent(event);
        return;
    }

    if (!(event->modifiers().testFlag(Qt::ControlModifier))) {
        for (auto *element : unsortedElements()) {
            if (element->hasTrigger() && !element->trigger().isEmpty() && element->trigger().matches(event->key())) {
                // Only InputButton (momentary) is released on key-up; InputSwitch stays latched
                if (auto *input = qobject_cast<GraphicElementInput *>(element); input && !input->isLocked() && (element->elementType() == ElementType::InputButton)) {
                    input->setOff();
                }
            }
        }
    }

    QGraphicsScene::keyReleaseEvent(event);
}

bool Scene::eventFilter(QObject *watched, QEvent *event)
{
    if (auto *mouseEvent = dynamic_cast<QGraphicsSceneMouseEvent *>(event)) {
        // Qt's default QGraphicsScene selection behaviour treats Shift+click as
        // "extend selection to range" (like a list widget), which is meaningless for
        // a free-form canvas.  Remapping Shift→Ctrl activates the Ctrl toggle-select
        // path instead, which is what users expect from a design tool.
        if (mouseEvent->modifiers().testFlag(Qt::ShiftModifier)) {
            mouseEvent->setModifiers(Qt::ControlModifier);
            return QGraphicsScene::eventFilter(watched, event);
        }

        // Intercept Ctrl+Left-click on an element to begin a clone-drag instead of
        // the default rubber-band selection; return true to swallow the event
        if ((mouseEvent->button() == Qt::LeftButton) && mouseEvent->modifiers().testFlag(Qt::ControlModifier)) {
            if (auto *item = itemAt(mouseEvent->scenePos()); item && ((item->type() == GraphicElement::Type) || (item->type() == Connection::Type))) {
                item->setSelected(true);
                m_clipboardManager.cloneDrag(mouseEvent->scenePos());
                return true;
            }
        }
    }

    return QGraphicsScene::eventFilter(watched, event);
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_interaction.mousePress(event)) {
        QGraphicsScene::mousePressEvent(event);
    }
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // Guard against re-entrant calls. When ensureVisible() scrolls the viewport,
    // Qt synchronously calls scrollContentsBy() → replayLastMouseEvent() →
    // mouseMoveEvent() before ensureVisible() returns. At a viewport corner,
    // scrolling to satisfy the H-margin violates the V-margin, so the replay
    // triggers another ensureVisible() that does the opposite, oscillating
    // until a stack overflow. Dropping the re-entrant call breaks the loop.
    // The guard lives here, not in SceneInteraction, because it must wrap the
    // base-class call below where the re-entrancy actually originates.
    if (m_handlingMouseMove) {
        return;
    }
    m_handlingMouseMove = true;
    const auto resetFlag = qScopeGuard([this] { m_handlingMouseMove = false; });

    if (!m_interaction.mouseMove(event)) {
        QGraphicsScene::mouseMoveEvent(event);
    }
}

void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_interaction.mouseRelease(event)) {
        QGraphicsScene::mouseReleaseEvent(event);
    }
}

void Scene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_interaction.mouseDoubleClick(event)) {
        QGraphicsScene::mouseDoubleClickEvent(event);
    }
}

void Scene::helpEvent(QGraphicsSceneHelpEvent *event)
{
    // A help event fires exactly when Qt is about to show a tooltip (after the wake-up
    // delay). If the cursor is over a port, show its own label plus its connected peers'
    // labels in situ instead of the native tooltip, so they all appear together at the
    // same wake-up delay a tooltip would use. Other items keep their native tooltip.
    if (auto *port = portAt(event->scenePos())) {
        m_connectionManager.showHoverLabels(port);
        return;
    }

    QGraphicsScene::helpEvent(event);
}

void Scene::addItem(QMimeData *mimeData, std::optional<QPointF> scenePos)
{
    m_dropHandler.addFromMimeData(mimeData, scenePos);
}
