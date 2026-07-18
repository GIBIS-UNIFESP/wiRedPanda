// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Phase 1/2 canvas prototype: batched QSGNode rendering (real per-element appearance
 * via TextureAtlas) + spatial-index hit-testing, driven by the real Simulation engine.
 */

#pragma once

#include <functional>
#include <memory>
#include <optional>

#include <QPointF>
#include <QQmlEngine>
#include <QQuickItem>
#include <QSet>
#include <QTimer>
#include <QUndoStack>
#include <QVector>
#include <QVersionNumber>

#include "App/Core/Enums.h"
#include "App/Element/GraphicElement.h"
#include "App/QuickShell/Canvas/CanvasICRegistry.h"
#include "App/QuickShell/Canvas/SpatialIndex.h"
#include "App/QuickShell/Canvas/TextureAtlas.h"
#include "App/Scene/SceneItemRegistry.h"

class Connection;
class InputPort;
class ItemWithId;
class OutputPort;
class Port;
class QDataStream;
class QGraphicsItem;
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
    /// \param buildDemo When true (the default -- and the only value QML's own construction
    /// of this QML_ELEMENT type ever uses), seeds the canvas with Phase 1/2's demo circuit
    /// (a handful of gates plus an IC loaded from the bundled Examples/ directory) so a
    /// bare `CanvasItem {}` in QML has something to show. QuickWorkSpace (Phase 4) passes
    /// false: a real document's canvas must start genuinely empty, exactly like production
    /// Scene's constructor never seeding demo content -- a QuickWorkSpace about to load a
    /// real .panda file (or represent a brand-new blank tab) that inherited the demo circuit
    /// found its elements mixed in with the real ones, an easy mistake to reintroduce, so
    /// this is a real constructor parameter, not a debug-only flag to delete later.
    explicit CanvasItem(QQuickItem *parent = nullptr, bool buildDemo = true);
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
    /// Raises the id registry's last-assigned id to \a newLastId (never lowers it). Mirrors
    /// Scene::setLastId(); used by QuickWorkSpace::load()/loadFromBlob() so ids allocated
    /// after a load don't collide with the highest id just loaded.
    void setLastId(int newLastId);
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
    /// no default, matching Scene's own signature. blobRegistry points at this canvas's real
    /// CanvasICRegistry blob map; contextDir is this canvas's own contextDir()/setContextDir()
    /// (see those below) -- the gap this doc comment used to describe (contextDir always empty,
    /// breaking re-resolution of any nested relative-path file reference) is closed now that
    /// QuickWorkSpace::load() actually sets it, the same way Scene::setContextDir() does from
    /// WorkSpace::load().
    [[nodiscard]] SerializationContext deserializationContext(QHash<quint64, Port *> &portMap,
                                                               const QVersionNumber &version,
                                                               SerializationPurpose purpose);

    /// Directory of the .panda file this canvas's content was loaded from (or saved to), used
    /// to resolve relative-path file references (nested file-backed ICs, appearances, audio).
    /// Mirrors Scene::contextDir()/setContextDir(). Empty until a real top-level load/save
    /// sets it (QuickWorkSpace does, mirroring WorkSpace::setCurrentFile()).
    [[nodiscard]] QString contextDir() const { return m_contextDir; }
    void setContextDir(const QString &dir) { m_contextDir = dir; }

    /// Returns the currently selected elements. Mirrors Scene::selectedElements(); used by
    /// this class's local command classes and by keyPressEvent()'s shortcut dispatch.
    [[nodiscard]] QList<GraphicElement *> selectedElements() const;
    /// Returns all elements on the canvas, unsorted. Mirrors Scene::unsortedElements(); used
    /// by CanvasICRegistry's findICsByBlobName()/renameBlob() element scans.
    [[nodiscard]] const QVector<GraphicElement *> &elements() const { return m_elements; }
    /// Returns all wires on the canvas, unsorted. Used by QuickWorkSpace::save() to build the
    /// full elements+connections item list Serialization::serialize() needs -- CanvasItem has
    /// no single QGraphicsScene::items()-equivalent since elements/connections are tracked in
    /// two separately-typed vectors (see this class's doc comment).
    [[nodiscard]] const QVector<Connection *> &connections() const { return m_connections; }
    /// Pushes \a cmd onto this canvas's undo stack (immediately executes its redo()) and
    /// resyncs the spatial index/repaints via the indexChanged connection set up in the
    /// constructor. Mirrors Scene::receiveCommand().
    void receiveCommand(QUndoCommand *cmd);
    /// Returns this canvas's undo stack. Mirrors Scene::undoStack(); used by
    /// CanvasICRegistry::createEmbeddedIC() to macro a blob-register with an item-add.
    [[nodiscard]] QUndoStack *undoStack() { return &m_undoStack; }

    /// Returns the embedded-IC blob registry for this canvas. Mirrors Scene::icRegistry().
    [[nodiscard]] CanvasICRegistry *icRegistry() { return &m_icRegistry; }

    /// Returns the simulation engine driving this canvas. Mirrors Scene::simulation(); used by
    /// this class's local command classes to wrap topology-mutating redo()/undo() bodies in a
    /// SimulationBlocker, exactly as Commands.cpp does through Scene::simulation().
    ///
    /// The pragma works around a GCC/Clang false positive: -O3's inliner walks into
    /// std::unique_ptr::get()'s own implementation here and flags a "potential null
    /// pointer dereference" that has nothing to do with any caller's use of the
    /// returned pointer -- it reproduces on an untouched one-line accessor regardless
    /// of null-checking at the call site, across both Qt 6.9.3 and 6.12.0.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
    [[nodiscard]] Simulation *simulation() const { return m_simulation.get(); }
#pragma GCC diagnostic pop
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

    // --- Type/property cycling (ports PropertyShortcutHandler's equivalents via Scene.h) ---
    //
    // Keyboard shortcuts match SceneUiBinder.cpp's real QShortcut bindings exactly: "[" / "]"
    // for main property, "{" / "}" for secondary property, "<" / ">" for type-cycling -- wired
    // in keyPressEvent() since no chrome QShortcut layer exists yet.

    /// Morphs each selected element to the next type in Enums::nextElmType()'s cycle, via
    /// CanvasMorphCommand. Re-selects the morphed element by id (MorphCommand replaces it
    /// in-place under the same id -- simpler and more robust than Scene::nextElm()'s
    /// position-based itemAt() re-lookup, which this canvas has no equivalent of anyway).
    void nextElm();
    /// Morphs each selected element to the previous type in Enums::prevElmType()'s cycle.
    void prevElm();
    /// Decrements the primary property (input/output count, frequency, or color, depending on
    /// element type) of each selected element, via CanvasUpdateCommand/CanvasChangePortSizeCommand.
    void prevMainPropShortcut();
    /// Increments the primary property of each selected element.
    void nextMainPropShortcut();
    /// Decrements the secondary property (currently: Led color, TruthTable output count) of
    /// each selected element.
    void prevSecndPropShortcut();
    /// Increments the secondary property of each selected element.
    void nextSecndPropShortcut();

    // --- Clipboard / mute / select-all (ports ClipboardManager's/Scene's equivalents) ---
    //
    // Ports copy/cut/paste/duplicate only -- ClipboardManager::cloneDrag() (Ctrl+drag) needs
    // QGraphicsScene::render() for its drag-ghost image, the same category of dependency
    // Phase 2 hit and fixed in ICRenderer::generatePreviewPixmap(), except here there's no
    // default-safe fallback since the rendered ghost IS the feature; tracked as a named
    // follow-up, not ported here (see the plan's "Phase 3 in depth" section). Blob-registry
    // inclusion (copy/cut of embedded ICs, paste's blob import) is also deferred: it needs
    // this canvas's own ICRegistry, which doesn't exist until the IC-embedding sub-step.
    // Selection itself is element-only here (SpatialIndex::queryRect()'s rubber-band already
    // excludes wire ids, see updateSelectionRect()'s doc comment), so unlike the real
    // ClipboardManager, copying two wired elements without a chrome-driven wire selection
    // won't carry the wire between them -- an inherited Phase 1 selection-model limitation,
    // not a new gap from this port.

    /// Copies the selected elements to the system clipboard. Mirrors Scene::copyAction().
    void copyAction();
    /// Cuts the selected elements (copy + delete). Mirrors Scene::cutAction().
    void cutAction();
    /// Pastes elements from the system clipboard, offset from the last known cursor position.
    /// Mirrors Scene::pasteAction().
    void pasteAction();
    /// Duplicates the current selection in place (one grid step down-right), without touching
    /// the system clipboard; the copies become the new selection. Mirrors Scene::duplicateAction().
    void duplicateAction();

    /// Mutes or unmutes every AudioOutputElement on the canvas. Mirrors Scene::mute().
    void mute(bool mute = true);
    /// Selects every element on the canvas. Mirrors Scene::selectAll().
    void selectAll();

    /// Builds and adds one element of \a type at \a pos, mirroring SceneDropHandler::
    /// addFromMimeData()'s element-construction logic (built-in / file-based IC / embedded IC)
    /// without the QMimeData round-trip -- Quick's Drag/DropArea has no cross-widget-hierarchy
    /// boundary to serialize across the way Widgets' native drag-and-drop does, so the
    /// palette entry's fields are passed directly. \a icFileName is empty for built-in types,
    /// a bare (contextDir()-relative) file name for a file-based IC, or a blob name for an
    /// embedded IC (when \a isEmbedded is true). Does nothing if \a type is IC, \a isEmbedded
    /// is true, and \a icFileName doesn't name a blob in icRegistry() (a stale palette entry).
    void addElementFromPalette(ElementType type, const QString &icFileName, bool isEmbedded, const QPointF &pos);

    /// Commits an inline label edit on \a element: pushes a CanvasUpdateCommand only if \a
    /// newLabel actually differs from the element's current label. Mirrors
    /// InlineLabelEditor::commit()'s undo-command logic (the QLineEdit-hosting QGraphicsProxyWidget
    /// itself has no Quick equivalent to port; Main.qml's inline TextField calls this directly
    /// instead, so it needs Q_INVOKABLE like CanvasItem's other QML-called methods).
    Q_INVOKABLE void commitInlineLabelEdit(GraphicElement *element, const QString &newLabel);

    /// Pushes a CanvasMorphCommand morphing every selected element to \a type. Mirrors
    /// ElementContextMenu::exec()'s "Morph to..." submenu action -- unlike nextElm()/prevElm()'s
    /// fixed next/prev-in-cycle morph, this morphs to a caller-chosen target type.
    void morphSelectionTo(ElementType type);

    /// Last known cursor position in canvas coordinates, updated on every mouse press/move.
    /// Mirrors Scene::mousePos(); used to place pasted elements relative to the cursor.
    [[nodiscard]] QPointF mousePos() const { return m_lastMousePos; }

signals:
    /// Emitted whenever the set of selected elements changes, or when an already-selected
    /// element's properties change in a way a property inspector needs to re-read (the
    /// deliberate reselect-to-refresh toggle in adjustMainProperty()/adjustSecondaryProperty()).
    /// CanvasItem has no QGraphicsScene base to inherit QGraphicsScene::selectionChanged()
    /// from (Scene::selectedElements() changes are otherwise silent here), so every selection-
    /// mutating method emits this explicitly. Mirrors QGraphicsScene::selectionChanged(),
    /// which ElementEditor::setScene() connects to in production.
    void selectionChanged();

    /// Emitted on right-click over an element (mousePressEvent() has already selected it if it
    /// wasn't already part of the selection, mirroring Scene::contextMenu()). \a pos is
    /// canvas-local. Mirrors Scene::contextMenuPos().
    void elementContextMenuRequested(GraphicElement *element, QPointF pos);
    /// Emitted on right-click over empty canvas. Mirrors Scene::contextMenu()'s "no item"
    /// branch (a separate, simpler Paste/Select-all menu built inline there rather than via
    /// ElementContextMenu::exec()).
    void emptyContextMenuRequested(QPointF pos);

    /// Emitted on double-click over an element with hasLabel() true. \a currentLabel is
    /// passed explicitly (rather than left for QML to read off \a element) since
    /// GraphicElement -- Layer 1 domain code -- has no Q_PROPERTY/Q_INVOKABLE QML can call;
    /// QML only ever holds \a element opaquely and passes it back into
    /// commitInlineLabelEdit(). \a targetRect is \a element's labelSceneBoundingRect()
    /// (canvas-local, matching mousePos()/palette-drop coordinates) to position an inline
    /// editor over. Mirrors GraphicElement::mouseDoubleClickEvent()'s inlineEditRequested()
    /// emission -- CanvasItem's own mouseDoubleClickEvent() override never forwards to the
    /// element's (it's dedicated to wire-splitting), so this replicates the same hasLabel()
    /// check directly instead of fabricating a QGraphicsSceneMouseEvent to feed the real method.
    void inlineEditRequested(GraphicElement *element, QString currentLabel, QRectF targetRect);

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
    /// \reimp Double-click on a fully-connected wire inserts a routing node, splitting it into
    /// two segments (CanvasSplitCommand). Mirrors SceneInteraction::mouseDoubleClick(); picks
    /// Phase 1's deferred wire-splitting gesture back up.
    void mouseDoubleClickEvent(QMouseEvent *event) override;
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
    /// Shared by nextMainPropShortcut()/prevMainPropShortcut(); \a dir is -1 (prev) or +1
    /// (next). Mirrors PropertyShortcutHandler::adjustMainProperty().
    void adjustMainProperty(int dir);
    /// Shared by nextSecndPropShortcut()/prevSecndPropShortcut(); \a dir is -1 or +1. Mirrors
    /// PropertyShortcutHandler::adjustSecondaryProperty().
    void adjustSecondaryProperty(int dir);
    /// Snapshots \a element, applies \a mutate, then pushes a CanvasUpdateCommand. Mirrors
    /// PropertyShortcutHandler::applyWithUndo().
    void applyPropertyWithUndo(GraphicElement *element, const std::function<void()> &mutate);
    /// Dispatches a press on \a element to whichever interactive-input behavior applies:
    /// InputSwitch toggles, InputRotary advances to its next port. InputButton is momentary
    /// (press = on, release = off, tracked separately via m_pressedInputButton) so it isn't
    /// handled here -- see mousePressEvent()/mouseReleaseEvent().
    void activateOnPress(GraphicElement *element);
    void startSelectionRect(const QPointF &anchor);
    void updateSelectionRect(const QPointF &current);
    void finishSelectionRect();
    /// Mirrors Scene::contextMenu(QPoint)'s hit-test/select/emit logic: right-clicking a
    /// selected element emits elementContextMenuRequested() as-is; right-clicking an
    /// unselected element clears the selection, selects just that element, then emits the
    /// same signal; right-clicking empty space emits emptyContextMenuRequested().
    void handleRightClick(const QPointF &pos);

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

    /// Shared by pasteAction()/duplicateAction(): deserializes from \a stream and adds the
    /// result via CanvasAddItemsCommand. Without \a fixedOffset the new items are placed
    /// relative to the cursor (paste); with it they are shifted by exactly that vector from
    /// the originals (duplicate). Returns the items added. Mirrors ClipboardManager::
    /// deserializeAndAdd().
    QList<QGraphicsItem *> deserializeAndAdd(QDataStream &stream, const QVersionNumber &version,
                                             std::optional<QPointF> fixedOffset = std::nullopt);

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

    /// Embedded-IC blob registry backing icRegistry() -- see CanvasICRegistry's own doc
    /// comment for what's ported vs. deliberately narrower than production ICRegistry.
    CanvasICRegistry m_icRegistry{this};

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

    /// Last known cursor position, updated on every mouse press/move -- backs mousePos().
    QPointF m_lastMousePos;

    /// Backs contextDir()/setContextDir() -- see their doc comment.
    QString m_contextDir;

    /// Offscreen-render cache backing real per-element appearance (Phase 2) -- see this
    /// class's doc comment and TextureAtlas's own for the design.
    TextureAtlas m_atlas;

    /// Real Simulation state changes on its own 1ms QTimer, independent of this item's
    /// render loop; this timer periodically calls QQuickItem::update() so the batched nodes
    /// stay in sync with live simulation output. Matches Simulation's own documented ~60fps
    /// visual-throttle cadence (see Simulation.h's m_visualTickInterval).
    QTimer m_refreshTimer;
};
