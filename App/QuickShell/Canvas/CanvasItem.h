// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Phase 1/2 canvas prototype: batched QSGNode rendering (real per-element appearance
 * via TextureAtlas) + spatial-index hit-testing, driven by the real Simulation engine.
 */

#pragma once

#include <memory>

#include <QPointF>
#include <QQmlEngine>
#include <QQuickItem>
#include <QSet>
#include <QTimer>
#include <QUndoStack>
#include <QVector>

#include "App/QuickShell/Canvas/SpatialIndex.h"
#include "App/QuickShell/Canvas/TextureAtlas.h"

class Connection;
class GraphicElement;
class InputPort;
class OutputPort;
class Port;
class Simulation;
class SimulationHost;

/**
 * \class CanvasItem
 * \brief Qt Quick rewrite Phase 1/2 canvas prototype.
 *
 * \details Proves the plan's riskiest bet in one place: a handful of *real* GraphicElement
 * subclasses (InputSwitch/And/Led — no synthetic stand-ins) are constructed and wired
 * together with real Connection objects, exactly as production code does, but without ever
 * being added to a QGraphicsScene — GraphicElement/Port/Connection's geometry (pos(),
 * boundingRect(), scenePos()) all work from the QGraphicsItem parent-child chain alone, no
 * scene membership required. A real Simulation instance (bound through the narrow
 * SimulationHost interface, same seam Scene itself uses) drives them on its own 1ms timer.
 * Each simulation-driven state change is picked up by this item's own refresh timer and
 * rendered as batched QSGGeometryNodes (gate bodies, wires, rubber-band overlay). Hit-testing
 * goes through SpatialIndex, replacing what QGraphicsScene::itemAt() would have done —
 * clicking a switch toggles it through the same setOn(bool, int) API production code uses,
 * and the resulting Status change flows through the real Simulation to the real Led, visibly
 * proving the whole chain end to end.
 *
 * **Gate appearance (Phase 2)**: bodies are real per-element appearance, not flat colored
 * quads — each element's own real, unmodified `paint()` override (`GraphicElement::paint()`
 * for the free-inheritance majority, `Display7::paint()`/`Mux::paint()`/etc. for the families
 * with custom rendering, dispatched polymorphically) is called against an offscreen QPainter
 * and the result cached/uploaded via TextureAtlas — the same technique
 * `Node::renderWirelessPixmap()` already uses in production, generalized. Cache key is built
 * from `GraphicElement::appearanceCacheKey()` (the live `QPixmap`'s identity — changes
 * whenever `setPixmap()`/`setRenderPixmap()` swaps it, which is how Mux/Demux/TruthTable/IC's
 * state-dependent procedural bodies already invalidate themselves for free) plus
 * rotation/flip/selected. See the plan's "Phase 2 in depth" section for the full design and
 * its family-by-family verification; `Display7`/`Display14`/`Display16` need one more cache-
 * key dimension (their segment overlays paint on top of an unchanged base pixmap, so
 * `appearanceCacheKey()` alone doesn't capture them) — not yet added, tracked as those
 * families' own porting step.
 *
 * Gesture state machine (drag-to-move, rubber-band multi-select) ports the algorithm from
 * App/Scene/SceneInteraction.cpp's mousePress/mouseMove/mouseRelease onto this hit-test
 * backend: press on an element snapshots its pre-drag position (and the rest of the current
 * selection's, mirroring SceneInteraction's "include the clicked element even if not yet
 * selected" rule); move repositions live and rebuilds the spatial index; release commits a
 * local undo entry only if something actually moved (click-without-drag pushes nothing).
 * Press on empty space starts a rubber-band rect; SpatialIndex::queryRect() replaces
 * QGraphicsScene::setSelectionArea(). The real production Commands.h/QUndoStack integration
 * is NOT ported (MoveCommand needs a concrete Scene* for its ID-based element lookup —
 * CommandUtils::findElements(Scene*, ids) — which doesn't exist here yet; that's explicitly
 * Phase 3 scope per the plan's roadmap, "port Commands.h's base once, then each pattern").
 * This item's own m_undoStack is a minimal local stand-in scoped to proving the drag gesture
 * itself works, not a preview of the eventual real integration.
 *
 * Wire-creation-by-dragging ports ConnectionManager's startFromOutput/startFromInput/
 * tryComplete/cancel/detach/updateEditedEnd state machine the same way: press on a port
 * begins (or completes, or detaches-and-restarts) an in-progress Connection tracked in
 * m_editedConnection; move follows the free end; release completes it at whatever port is
 * under the cursor, or cancels it if that's not a valid port. isConnectionAllowed()/
 * connectionRejectionReason() are `static` with zero Scene coupling, so they're called
 * directly, unmodified. What's NOT ported: hover-port highlighting and the in-situ label
 * chips (PortHoverLabel — a separate Widgets-tooltip subsystem, chrome polish rather than
 * core wire-creation logic) and, again, the real AddItemsCommand/DeleteItemsCommand
 * (Phase 3 scope) — completing or detaching a wire here mutates m_connections directly.
 */
class CanvasItem : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit CanvasItem(QQuickItem *parent = nullptr);
    ~CanvasItem() override;

protected:
    /// \reimp Builds the batched geometry nodes (gates, wires, selection overlay) from current state.
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;
    /// \reimp Hit-tests via m_index; toggles a clicked input, starts/completes/detaches a
    /// wire on a port hit, or starts a drag/rubber-band.
    void mousePressEvent(QMouseEvent *event) override;
    /// \reimp Drives the in-progress element drag, in-progress wire, or rubber-band rect.
    void mouseMoveEvent(QMouseEvent *event) override;
    /// \reimp Commits the drag as a local undo entry if anything actually moved; completes
    /// an in-progress wire if released over a compatible port.
    void mouseReleaseEvent(QMouseEvent *event) override;
    /// \reimp Updates the hovered element for the next paint's highlight.
    void hoverMoveEvent(QHoverEvent *event) override;
    /// \reimp Clears hover highlight when the pointer leaves the canvas entirely.
    void hoverLeaveEvent(QHoverEvent *event) override;

private:
    void buildDemoCircuit();
    void rebuildSpatialIndex();
    /// Dispatches a press on \a element to whichever interactive-input behavior applies:
    /// InputSwitch toggles, InputRotary advances to its next port. InputButton is momentary
    /// (press = on, release = off, tracked separately via m_pressedInputButton) so it isn't
    /// handled here -- see mousePressEvent()/mouseReleaseEvent().
    void activateOnPress(GraphicElement *element);
    void startSelectionRect(const QPointF &anchor);
    void updateSelectionRect(const QPointF &current);
    void finishSelectionRect();

    // --- Wire-creation-by-dragging (ports ConnectionManager's workflow) ---
    void startWireFromOutput(OutputPort *startPort);
    void startWireFromInput(InputPort *endPort);
    void tryCompleteWire(const QPointF &pos);
    void cancelEditedWire();
    void detachWire(InputPort *endPort);
    void updateEditedWireEnd(const QPointF &pos);

    /// Builds an appearance-cache-key string for \a element's current pixmap identity,
    /// rotation, flip, and selection state -- see this class's doc comment for why that's
    /// enough for every element family except Display7/14/16 (not yet ported).
    static QString appearanceKeyFor(GraphicElement *element);

    std::unique_ptr<SimulationHost> m_host;
    std::unique_ptr<Simulation> m_simulation;
    QVector<GraphicElement *> m_elements; // owned; never added to a QGraphicsScene
    QVector<Connection *> m_connections;  // owned; never added to a QGraphicsScene
    QHash<quint64, GraphicElement *> m_elementsById;
    QHash<quint64, Port *> m_portsById; // ports aren't owned here, they're owned by m_elements
    SpatialIndex m_index;
    quint64 m_hoveredId = 0;
    QSet<quint64> m_selectedIds;

    /// The wire currently being dragged into place, or nullptr. Owned here directly (not
    /// added to m_connections) until tryCompleteWire() commits it -- mirrors
    /// ConnectionManager's m_editedConnectionId, just as a raw pointer instead of a
    /// Scene::itemById()-resolved id, since this transient state never needs to survive an
    /// undo/redo the way ConnectionManager's does.
    Connection *m_editedConnection = nullptr;
    /// Where the in-progress wire's free (not-yet-attached) end currently is, for rendering
    /// only -- Connection's own m_startPos/m_endPos are private with no public getters, and
    /// this canvas doesn't paint through Connection::paint() anyway (see the batched-
    /// rendering discussion in this class's doc comment), so tracking it here avoids adding
    /// getters to production Wiring/Connection.h for a Phase-1-prototype-only need.
    QPointF m_editedWireFreeEnd;

    /// The InputButton currently held down, or nullptr. Set on press, cleared (and released
    /// back to off) on release -- regardless of where the release happens, matching the real
    /// QGraphicsItem mouse-grab semantics InputButton::mouseReleaseEvent relies on (the item
    /// that was pressed keeps receiving the release even if the cursor has moved off it).
    GraphicElement *m_pressedInputButton = nullptr;

    /// Local undo stack for the drag gesture only -- see this class's doc comment for why
    /// this isn't the real Commands.h/MoveCommand integration (that needs a concrete Scene*,
    /// which is explicitly Phase 3 scope).
    QUndoStack m_undoStack;

    /// Per-drag snapshot: parallel to m_dragElements, captured at press, compared at release.
    /// No QPointer-style destroyed-mid-drag safety net yet (unlike SceneInteraction's
    /// m_dragSnapshot) -- this canvas has no delete gesture yet for that race to matter.
    QVector<GraphicElement *> m_dragElements;
    QVector<QPointF> m_dragStartPositions;
    QPointF m_dragAnchor;
    bool m_draggingElement = false;

    bool m_markingSelectionBox = false;
    QPointF m_selectionAnchor;
    QRectF m_selectionRect;

    /// Offscreen-render cache backing real per-element appearance (Phase 2) -- see this
    /// class's doc comment and TextureAtlas's own for the design.
    TextureAtlas m_atlas;

    /// Real Simulation state changes on its own 1ms QTimer, independent of this item's
    /// render loop; this timer periodically calls QQuickItem::update() so the batched nodes
    /// stay in sync with live simulation output. Matches Simulation's own documented ~60fps
    /// visual-throttle cadence (see Simulation.h's m_visualTickInterval).
    QTimer m_refreshTimer;
};
