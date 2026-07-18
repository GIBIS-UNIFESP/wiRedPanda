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
#include <QVersionNumber>

#include "App/QuickShell/Canvas/SpatialIndex.h"
#include "App/QuickShell/Canvas/TextureAtlas.h"
#include "App/Scene/SceneItemRegistry.h"

class Connection;
class GraphicElement;
class InputPort;
class ItemWithId;
class OutputPort;
class Port;
class Simulation;
class SimulationHost;
struct SerializationContext;
enum class SerializationPurpose;

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
 * selected" rule); move repositions live and rebuilds the spatial index; release commits an
 * undo entry only if something actually moved (click-without-drag pushes nothing).
 * Press on empty space starts a rubber-band rect; SpatialIndex::queryRect() replaces
 * QGraphicsScene::setSelectionArea().
 *
 * **Commands (Phase 3)**: App/QuickShell/Canvas/CanvasCommands.h ports Commands.h's
 * MoveCommand/RotateCommand/FlipCommand as CanvasMoveCommand/CanvasRotateCommand/
 * CanvasFlipCommand, resolving their targets through this class's own itemById()/
 * SceneItemRegistry-backed id layer instead of a concrete Scene* (see the "Real id/registry
 * layer" doc comment on this class's public section). m_undoStack is the real undo stack now,
 * not a stand-in -- rotateRight()/rotateLeft()/flipHorizontally()/flipVertically() and the
 * eight align/distribute methods push these real commands the same way Scene's equivalents
 * do. Still not ported: the serialize/deserialize snapshot family (AddItemsCommand/
 * DeleteItemsCommand/UpdateCommand/ChangePortSizeCommand), clipboard, IC embedding, and the
 * topology-surgery family (SplitCommand/MorphCommand) -- see the plan's "Phase 3 in depth"
 * section for the full sub-step breakdown.
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
 * (Phase 3 scope, not yet landed) — completing or detaching a wire here mutates
 * m_connections directly.
 */
class CanvasItem : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit CanvasItem(QQuickItem *parent = nullptr);
    ~CanvasItem() override;

    // --- Real id/registry layer (Phase 3 foundational sub-step) ---
    //
    // Commands.h's entire machinery (CommandUtils::findElm/findConn/findElements/findItems,
    // ElementsCommand::elements()) resolves its targets through Scene::itemById(), backed by
    // a real ItemWithId::id()/SceneItemRegistry pair every GraphicElement/Connection already
    // carries. Phase 1's SpatialIndex used a synthetic tagged-index id scheme instead (a small
    // integer per element/connection, tagged by kind in CanvasItem.cpp's anonymous namespace)
    // because nothing needed real ids yet; that placeholder is retired here in favor of the
    // real ItemWithId id space, mirrored on this class the same way Scene exposes it -- public,
    // like Scene's own equivalents, since this class's own local command classes
    // (App/QuickShell/Canvas/CanvasCommands.h, Phase 3's later sub-steps) resolve their
    // targets through it from outside this class, the same way Commands.cpp does through
    // Scene. Ports are NOT ItemWithId (see Port.h: QGraphicsPathItem only), so SpatialIndex's
    // port ids stay synthetic-indexed, unaffected by this.

    /// Returns the element or connection registered under \a id, or nullptr. Mirrors
    /// Scene::itemById().
    [[nodiscard]] ItemWithId *itemById(int id) const;
    /// Returns a fresh, previously unused id. Mirrors Scene::nextId().
    int nextId();
    /// Reassigns \a item's id to \a newId without registering it -- mirrors Scene::updateItemId(),
    /// used by undo/redo restore paths to preserve an item's original id across a
    /// serialize/deserialize round-trip.
    void updateItemId(ItemWithId *item, int newId);
    /// Assigns \a element a fresh id if unassigned (or preserves a pre-assigned one, e.g. from
    /// updateItemId()) and registers it. Mirrors the id-registration half of Scene::addItem().
    void addItem(GraphicElement *element);
    /// Assigns \a connection a fresh id if unassigned and registers it. Overload of the above
    /// for Connection, mirroring Scene::addItem()'s single QGraphicsItem*-typed overload split
    /// across this class's two separately-typed element/connection vectors.
    void addItem(Connection *connection);
    /// Unregisters \a element's id. Mirrors the id-unregistration half of Scene::removeItem() --
    /// called before deletion for clarity/symmetry with Scene's pattern, even though
    /// ItemWithId's destructor would self-unregister regardless.
    void removeItem(GraphicElement *element);
    /// Unregisters \a connection's id. Connection overload of the above.
    void removeItem(Connection *connection);
    /// Builds a deserialization context for this canvas -- thin wrapper around the already-
    /// portable SerializationContext, mirroring Scene::deserializationContext(). \a purpose has
    /// no default, matching Scene's own signature. blobRegistry stays null until the IC
    /// embedding sub-step (Phase 3's ICRegistry port) gives this canvas a real blob map.
    [[nodiscard]] SerializationContext deserializationContext(QHash<quint64, Port *> &portMap,
                                                               const QVersionNumber &version,
                                                               SerializationPurpose purpose);

    /// Returns the currently selected elements. Mirrors Scene::selectedElements(); used by
    /// this class's local command classes and by keyPressEvent()'s shortcut dispatch.
    [[nodiscard]] QList<GraphicElement *> selectedElements() const;
    /// Pushes \a cmd onto this canvas's undo stack (immediately executes its redo()) and
    /// resyncs the spatial index/repaints via the indexChanged connection set up in the
    /// constructor. Mirrors Scene::receiveCommand().
    void receiveCommand(QUndoCommand *cmd);

    /// Returns the simulation engine driving this canvas. Mirrors Scene::simulation(); used by
    /// this class's local command classes to wrap topology-mutating redo()/undo() bodies in a
    /// SimulationBlocker, exactly as Commands.cpp does through Scene::simulation().
    [[nodiscard]] Simulation *simulation() const { return m_simulation.get(); }
    /// Marks the simulation topology stale (restart()) and resyncs the spatial index/repaints.
    /// Mirrors Scene::setCircuitUpdateRequired() -- this canvas has no autosave/visibility-
    /// dirty machinery to also flag, so it's just the simulation-rebuild half.
    void restartSimulation();

    /// Deletes the currently selected elements (and their attached wires, via
    /// CanvasDeleteItemsCommand's CanvasCommandUtils::loadList() expansion) as one undoable
    /// command. Mirrors Scene::deleteAction(). No-op if nothing is selected.
    void deleteSelected();

    // --- Rotate / flip / align / distribute (ports Scene.h's equivalents) ---
    //
    // No chrome menu exists yet (Phase 4) to trigger these, so keyPressEvent() wires the same
    // keyboard shortcuts the real MainWindowUI.cpp binds for the ones that have one (Ctrl+R,
    // Ctrl+Shift+R, Ctrl+H -- see MainWindowUI.cpp's setupUi()); flipVertically() has no
    // keyboard shortcut in the real app either (menu-only there too), so it's exercised
    // directly rather than through a synthesized key event.

    /// Rotates the current selection 90 degrees clockwise.
    void rotateRight();
    /// Rotates the current selection 90 degrees counter-clockwise.
    void rotateLeft();
    /// Flips the current selection horizontally.
    void flipHorizontally();
    /// Flips the current selection vertically.
    void flipVertically();

    /// Aligns selected elements' left edges to the leftmost selected edge. No-op below 2 elements.
    void alignLeft();
    /// Aligns selected elements' right edges to the rightmost selected edge. No-op below 2 elements.
    void alignRight();
    /// Aligns selected elements' top edges to the topmost selected edge. No-op below 2 elements.
    void alignTop();
    /// Aligns selected elements' bottom edges to the bottommost selected edge. No-op below 2 elements.
    void alignBottom();
    /// Aligns selected elements' horizontal (X) centers so they share one vertical line.
    /// No-op below 2 elements.
    void alignHorizontalCenter();
    /// Aligns selected elements' vertical (Y) centers so they share one horizontal line.
    /// No-op below 2 elements.
    void alignVerticalCenter();
    /// Redistributes selected elements with equal horizontal gaps, keeping the leftmost and
    /// rightmost elements fixed as anchors. No-op below 3 elements.
    void distributeHorizontally();
    /// Redistributes selected elements with equal vertical gaps, keeping the topmost and
    /// bottommost elements fixed as anchors. No-op below 3 elements.
    void distributeVertically();

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
    /// \reimp Fires keyboard-triggered InputSwitch/InputButton elements (GraphicElement::
    /// trigger()); the dispatch point future Phase 3 sub-steps (undo/redo, clipboard,
    /// nudge, rotate/flip) extend, mirroring Scene::keyPressEvent()'s dispatch order.
    void keyPressEvent(QKeyEvent *event) override;
    /// \reimp Releases a momentary InputButton keyboard trigger (InputSwitch stays latched).
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    void buildDemoCircuit();
    void rebuildSpatialIndex();
    /// Shared implementation for rotateRight()/rotateLeft(); \a angle is the clockwise degrees
    /// to rotate (e.g. 90 or -90). No-op if nothing is selected. Mirrors Scene::rotate().
    void rotate(int angle);
    /// If \a event is a plain/Shift arrow key and something is selected, moves the selection by
    /// one grid step (Shift = a larger step) as a single undoable command and returns true.
    /// Mirrors Scene::nudgeSelection().
    bool nudgeSelection(QKeyEvent *event);
    /// Dispatches a press on \a element to whichever interactive-input behavior applies:
    /// InputSwitch toggles, InputRotary advances to its next port. InputButton is momentary
    /// (press = on, release = off, tracked separately via m_pressedInputButton) so it isn't
    /// handled here -- see mousePressEvent()/mouseReleaseEvent().
    void activateOnPress(GraphicElement *element);
    void startSelectionRect(const QPointF &anchor);
    void updateSelectionRect(const QPointF &current);
    void finishSelectionRect();

    /// Returns true if \a pos (canvas space) lands on one of \a owner's own ports. Reimplements
    /// IC::isCursorOverPort()'s decision against this canvas's own SpatialIndex/m_portsById
    /// rather than calling IC's private isCursorOverPort()/protected hoverXxxEvent()
    /// overrides directly -- same pattern as activateOnPress() reimplementing
    /// InputSwitch/InputRotary's mousePressEvent logic instead of calling it.
    bool isOverOwnPort(GraphicElement *owner, const QPointF &pos) const;

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

    /// Real id/registry layer backing itemById()/nextId()/updateItemId() -- see this class's
    /// doc comment above those declarations. SceneItemRegistry itself is ported unmodified
    /// (confirmed Widgets-free: a bare QHash<int, ItemWithId *> plus a monotonic counter).
    SceneItemRegistry m_itemRegistry;

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

    /// This canvas's real undo stack -- receiveCommand() pushes onto it, matching
    /// Scene::receiveCommand()/m_undoStack exactly. See this class's doc comment on
    /// CanvasCommands.h for which command families are ported onto it so far.
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
