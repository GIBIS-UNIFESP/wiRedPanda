// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Canvas foundation: batched QSGNode rendering + spatial-index hit-testing,
 * driven by the real Simulation engine.
 */

#pragma once

#include <functional>
#include <limits>
#include <memory>
#include <optional>

#include <QColor>
#include <QHash>
#include <QImage>
#include <QPair>
#include <QPoint>
#include <QPointF>
#include <QQmlEngine>
#include <QQuickItem>
#include <QSet>
#include <QUndoStack>
#include <QVariant>
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
class QPainter;
class Simulation;
class SimulationHost;
struct SerializationContext;
enum class SerializationPurpose;

/**
 * \class CanvasItem
 * \brief The Qt Quick canvas's rendering + hit-testing core.
 *
 * \details A handful of *real* GraphicElement subclasses (InputSwitch/And/Led -- no synthetic
 * stand-ins) are constructed and wired together with real Connection objects, exactly as
 * production code does, without ever being added to a QGraphicsScene -- GraphicElement/Port/
 * Connection's geometry (pos(), boundingRect(), scenePos()) has had no QGraphicsScene
 * dependency at all since the domain-decoupling refactor. A real Simulation instance (bound
 * through the narrow SimulationHost interface, the same seam Scene itself uses) drives them
 * on its own 1ms timer. Each simulation-driven state change is picked up by this item's own
 * refresh timer and rendered as batched QSGGeometryNodes (gate bodies, wires, rubber-band
 * overlay). Hit-testing goes through SpatialIndex, replacing what QGraphicsScene::itemAt()
 * would have done -- clicking a switch toggles it through the same setOn(bool, int) API
 * production code uses, and the resulting Status change flows through the real Simulation to
 * the real Led, visibly proving the whole chain end to end.
 *
 * **Gate appearance**: bodies are real per-element appearance, not flat colored quads -- each
 * element's own real, unmodified paint() override (GraphicElement::paint() for the
 * free-inheritance majority, Display7::paint()/Mux::paint()/etc. for the families with custom
 * rendering, dispatched polymorphically once those families are ported) is called against an
 * offscreen QPainter and the result cached/uploaded via TextureAtlas -- the same technique
 * Node::renderWirelessPixmap() already uses in production, generalized. Cache key is built
 * from GraphicElement::appearanceCacheKey() (the live QPixmap's identity -- changes whenever
 * setPixmap()/setRenderPixmap() swaps it, which is how Mux/Demux/TruthTable/IC's
 * state-dependent procedural bodies already invalidate themselves for free) plus
 * rotation/flip/selected. Display7/Display14/Display16 need one more cache-key dimension
 * (their segment overlays paint on top of an unchanged base pixmap, so appearanceCacheKey()
 * alone doesn't capture them) -- not yet added.
 *
 * Gesture state machine (drag-to-move, rubber-band multi-select) ports the algorithm from
 * App/Scene/SceneInteraction.cpp's mousePress/mouseMove/mouseRelease onto this hit-test
 * backend: press on an element snapshots its pre-drag position (and the rest of the current
 * selection's, mirroring SceneInteraction's "include the clicked element even if not yet
 * selected" rule); move repositions live and rebuilds the spatial index; release commits an
 * undo entry only if something actually moved (click-without-drag pushes nothing). Press on
 * empty space starts a rubber-band rect; SpatialIndex::queryRect() replaces
 * QGraphicsScene::setSelectionArea().
 *
 * **Commands**: App/QuickShell/Canvas/CanvasCommands.h ports Commands.h's
 * MoveCommand/RotateCommand/FlipCommand as CanvasMoveCommand/CanvasRotateCommand/
 * CanvasFlipCommand, resolving their targets through this class's own itemById()/
 * SceneItemRegistry-backed id layer instead of a concrete Scene*. m_undoStack is the real
 * undo stack -- rotateRight()/rotateLeft()/flipHorizontally()/flipVertically() and the eight
 * align/distribute methods push these real commands the same way Scene's equivalents do.
 * Still not ported: the serialize/deserialize snapshot family (AddItemsCommand/
 * DeleteItemsCommand/UpdateCommand/ChangePortSizeCommand), clipboard, IC embedding, and the
 * topology-surgery family (SplitCommand/MorphCommand).
 *
 * Wire-creation-by-dragging ports ConnectionManager's startFromOutput/startFromInput/
 * tryComplete/cancel/detach/updateEditedEnd state machine the same way: press on a port
 * begins (or completes, or detaches-and-restarts) an in-progress Connection tracked in
 * m_editedConnection; move follows the free end; release completes it at whatever port is
 * under the cursor, or leaves it in progress if that's not a valid port. isConnectionAllowed()/
 * connectionRejectionReason() (App/Wiring/ConnectionValidity.h -- extracted from
 * ConnectionManager specifically so a portable consumer like this canvas doesn't need to link
 * App/Scene) are called directly, unmodified. SpatialIndex indexes individual ports (tagged
 * ids alongside elements/wires, kept from colliding via reserved high bits) on top of
 * whole-element boxes, since wire endpoints need port-level hit-testing the drag/rubber-band
 * gestures never required. What's NOT ported here: the real AddItemsCommand/DeleteItemsCommand
 * -- completing or detaching a wire here mutates m_connections directly.
 *
 * Port-hover peer highlighting mirrors ConnectionManager::setHoverPort()'s Widgets-only
 * feature: hovering a port computes it and every port connected to it via a wire, and
 * portHoverChanged() emits that set as a QVariantList of screen-position/name/theme-color
 * chips for a future chrome layer to render as a highlight ring + in-situ name label per port
 * -- no QuickAppController/QML overlay exists yet to consume them, so this canvas only computes
 * and emits; see the identical "signal emission is in scope, popup consumption is chrome work"
 * split already used for the IC hover-preview signals above.
 */
class CanvasItem : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

    // Drive Main.qml's canvas ScrollBars (GraphicsView has real QAbstractScrollArea scrollbars;
    // this canvas is pan/zoom-only, with no Flickable underneath for a QML ScrollBar to attach
    // to automatically) -- position/size use the same 0.0-1.0 normalized semantics as
    // Flickable's own visibleArea.xPosition/widthRatio, which is what ScrollBar.position/size
    // are designed to bind to. See scrollableWorldRect()'s own doc comment for what "size"
    // is a fraction of.
    Q_PROPERTY(qreal horizontalScrollPosition READ horizontalScrollPosition WRITE setHorizontalScrollPosition NOTIFY zoomChanged FINAL)
    Q_PROPERTY(qreal horizontalScrollSize READ horizontalScrollSize NOTIFY zoomChanged FINAL)
    Q_PROPERTY(qreal verticalScrollPosition READ verticalScrollPosition WRITE setVerticalScrollPosition NOTIFY zoomChanged FINAL)
    Q_PROPERTY(qreal verticalScrollSize READ verticalScrollSize NOTIFY zoomChanged FINAL)

    /// Exercises m_minimapRebuildCount directly to assert on renderMinimapImage()'s cache-skip
    /// behavior (pan/zoom-in-bounds must not rebuild; edits/zoom-out-past-bounds must).
    friend class TestCanvasItemSmoke;
    /// Exercises the private wire-drag API (startWireFromOutput()/cancelEditedWire()) directly
    /// to regression-test cancelEditedWire() against a dangling-pointer bug class, without
    /// fabricating spatial-index-compatible mouse events.
    friend class TestDanglingPointer;

public:
    /// \param parent Standard QQuickItem parent.
    /// \param buildDemo When true (the default -- and the only value QML's own construction
    /// of this QML_ELEMENT type ever uses), seeds the canvas with the demo circuit (a handful
    /// of gates plus an IC loaded from the bundled Examples/ directory) so a bare
    /// `CanvasItem {}` in QML has something to show. QuickWorkSpace passes false: a
    /// real document's canvas must start genuinely empty, exactly like production Scene's
    /// constructor never seeding demo content -- a QuickWorkSpace about to load a real .panda
    /// file (or represent a brand-new blank tab) that inherited the demo circuit would find
    /// its elements mixed in with the real ones, an easy mistake to reintroduce, so this is a
    /// real constructor parameter, not a debug-only flag to delete later.
    explicit CanvasItem(QQuickItem *parent = nullptr, bool buildDemo = true);
    ~CanvasItem() override;

    /// Shows or hides internal logic-gate elements (Input/Output/Other groups stay always
    /// visible, matching VisibilityManager::showGates()). Purely visual/hit-testing -- the
    /// simulation keeps running underneath regardless. Reached from Main.qml's View menu
    /// ("Show Gates") via QuickAppController::setGatesVisible() (feature-gap decision #1).
    Q_INVOKABLE void setGatesVisible(bool visible);
    /// Shows or hides connection wires, Node elements, and every other element's port handles.
    /// Mirrors VisibilityManager::showWires() exactly, including its Node-vs-other-element
    /// split (a Node is meaningless without wires, so it's hidden entirely; other elements
    /// keep their body visible and only lose their port glyphs).
    Q_INVOKABLE void setWiresVisible(bool visible);
    [[nodiscard]] bool gatesVisible() const { return m_gatesVisible; }
    [[nodiscard]] bool wiresVisible() const { return m_wiresVisible; }

    /// Refreshes every element/port/connection's theme-derived state after a live Light/Dark/
    /// System theme switch. Mirrors Scene::updateTheme() (element/connection loops), plus
    /// m_atlas.clear() -- appearanceKeyFor()'s cache key has no theme dimension, so without
    /// dropping every cached tile, already-rendered elements would keep showing the old
    /// theme's baked-in port-glyph colors indefinitely (until some unrelated state change,
    /// e.g. a status/rotation/selection change, happened to invalidate that element's own
    /// tile). Unlike Scene (connected to ThemeManager::themeChanged in its own constructor),
    /// this class has no live theme-change listener of its own -- callers must invoke this
    /// explicitly on a theme switch (see QuickAppController::themeChanged).
    void updateTheme();

    // --- Id/registry layer (mirrors Scene's exact API shape) ---

    /// Returns the item registered under \a id, or nullptr. Every command in a future
    /// Commands.h port resolves its targets through this, exactly as Scene::itemById() does
    /// for the real production commands.
    [[nodiscard]] ItemWithId *itemById(int id) const { return m_itemRegistry.itemById(id); }
    /// Returns a fresh, previously unused id.
    int nextId() { return m_itemRegistry.nextId(); }
    /// Raises the id registry's last-assigned id to \a newLastId (never lowers it). Mirrors
    /// Scene::setLastId(); used by QuickWorkSpace::load()/loadFromBlob() so ids allocated
    /// after a load don't collide with the highest id just loaded.
    void setLastId(int newLastId) { m_itemRegistry.setLastId(newLastId); }
    /// Pre-assigns \a newId to \a item before it is added (undo/redo restore path).
    void updateItemId(ItemWithId *item, int newId) { m_itemRegistry.updateItemId(item, newId); }
    /// Assigns \a element its id (a fresh one if unassigned, i.e. id() < 0; otherwise preserves
    /// it and advances the registry's counter past it -- the undo/redo restore path) and
    /// registers it in the id/registry, exactly mirroring Scene::addItem()'s own id-handling
    /// half. Does NOT touch m_elements -- callers still manage that list themselves (this is a
    /// typed overload of the id/registry step alone, not a replacement for the whole
    /// add-to-canvas flow, matching how Scene::addItem() itself only does this plus scene
    /// membership, kept separate concerns here since this canvas has no single polymorphic
    /// item list to add to).
    void addItem(GraphicElement *element);
    /// \overload
    void addItem(Connection *connection);
    /// Unregisters \a element's id from the registry -- mirrors Scene::removeItem()'s own
    /// id-handling half. Does NOT touch m_elements and does NOT delete \a element -- the
    /// caller is still responsible for both, same two-step contract Scene::removeItem()+
    /// delete already uses elsewhere in this codebase.
    void removeItem(GraphicElement *element);
    /// \overload
    void removeItem(Connection *connection);

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
    /// repaints. Mirrors Scene::receiveCommand(). CanvasCommands.h's classes need to reach
    /// this from outside the class, the same way Commands.cpp reaches Scene's equivalent.
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
    /// The pragma below is a known, recurring GCC 15 -Wnull-dereference false positive under
    /// any optimized preset (-O2/-O3, e.g. relwithdebinfo/release). A call-site null-guard does
    /// NOT silence it: the warning fires on evaluating .simulation() itself, purely from being
    /// inlined through std::unique_ptr::get()'s own internals, independent of what the caller
    /// does with the result. Wrapping the accessor's definition (not every call site) fixes
    /// every caller, present and future, in one place.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-dereference"
#endif
    [[nodiscard]] Simulation *simulation() const { return m_simulation.get(); }
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
    /// Marks the simulation topology stale (restart()) and resyncs the spatial index/repaints.
    /// Mirrors Scene::setCircuitUpdateRequired() -- this canvas has no autosave/visibility-
    /// dirty machinery to also flag, so it's just the simulation-rebuild half.
    void restartSimulation();

    /// Deletes the currently selected elements (and their attached wires, via
    /// CanvasDeleteItemsCommand's CanvasCommandUtils::loadList() expansion) as one undoable
    /// command. Mirrors Scene::deleteAction(). No-op if nothing is selected.
    void deleteSelected();

    /// Builds a SerializationContext for a load() call originating from this canvas. Mirrors
    /// Scene::deserializationContext(): blobRegistry points at this canvas's real
    /// CanvasICRegistry blob map; contextDir is this canvas's own contextDir()/setContextDir()
    /// (see those below), set by QuickWorkSpace::load() the same way Scene::setContextDir()
    /// does from WorkSpace::load() -- required for re-resolving any nested relative-path file
    /// reference.
    SerializationContext deserializationContext(QHash<quint64, Port *> &portMap,
                                                 const QVersionNumber &version,
                                                 SerializationPurpose purpose);

    /// Directory of the .panda file this canvas's content was loaded from (or saved to), used
    /// to resolve relative-path file references (nested file-backed ICs, appearances, audio).
    /// Mirrors Scene::contextDir()/setContextDir(). Empty until a real top-level load/save
    /// sets it (QuickWorkSpace does, mirroring WorkSpace::setCurrentFile()).
    [[nodiscard]] QString contextDir() const { return m_contextDir; }
    void setContextDir(const QString &dir) { m_contextDir = dir; }

    // --- Rotate / flip / align / distribute (ports Scene.h's equivalents) ---
    //
    // No chrome menu exists yet to trigger these, so keyPressEvent() wires the
    // same keyboard shortcuts the real MainWindowUI.cpp binds for the ones that have one
    // (Ctrl+R, Ctrl+Shift+R, Ctrl+H); flipVertically() has no keyboard shortcut in the real
    // app either (menu-only there too), so it's exercised directly rather than through a
    // synthesized key event.

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
    // QGraphicsScene::render() for its drag-ghost image, unusable here for the same reason
    // renderICPreviewImage()/renderExportImage() document; unlike those, there's no
    // default-safe fallback since the rendered ghost IS the feature, so it isn't ported here.
    // Blob-registry inclusion (copy/cut of embedded ICs, paste's blob import) is also not
    // ported: it needs this canvas's own ICRegistry. Selection itself is element-only here
    // (SpatialIndex::queryRect()'s rubber-band already excludes wire ids, see
    // updateSelectionRect()'s doc comment), so unlike the real ClipboardManager, copying two
    // wired elements without a chrome-driven wire selection won't carry the wire between them.

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

    /// Builds and adds one element of \a type at \a screenPos, mirroring SceneDropHandler::
    /// addFromMimeData()'s element-construction logic (built-in / file-based IC / embedded IC)
    /// without the QMimeData round-trip -- Quick's Drag/DropArea has no cross-widget-hierarchy
    /// boundary to serialize across the way Widgets' native drag-and-drop does, so the
    /// palette entry's fields are passed directly. \a icFileName is empty for built-in types,
    /// a bare (contextDir()-relative) file name for a file-based IC, or a blob name for an
    /// embedded IC (when \a isEmbedded is true). Does nothing if \a type is IC, \a isEmbedded
    /// is true, and \a icFileName doesn't name a blob in icRegistry() (a stale palette entry).
    /// \a screenPos is in this item's own screen/QML-facing coordinate space (canvasHost-local
    /// pixels, e.g. a DropArea's drop.x/drop.y or a viewport-center double-click target) --
    /// converted internally via screenToWorld() before placing the element, so drops/adds land
    /// at the correct world position under whatever pan/zoom is currently active.
    void addElementFromPalette(ElementType type, const QString &icFileName, bool isEmbedded, const QPointF &screenPos);

    /// Adds one InputSwitch at world origin, selected, clearing any prior selection -- exactly
    /// addElementFromPalette()'s selection/command shape, minus the palette-entry/IC/screen-
    /// position plumbing this caller doesn't need. Backs the ui-overview tour's
    /// "element-properties" step (App/Resources/Tours/ui-overview.json's "setupElementEditorDemo"
    /// click id) so the element editor panel has something real to show.
    void addTourDemoInputSwitch();

    /// Adds two Clocks -> And -> Led (all wired), clearing any prior selection -- port of
    /// MainWindow::runTourDemoAction()'s identical "setupWaveformDemo" case. Backs the
    /// ui-overview tour's "waveform-table" step, priming a real circuit for the
    /// AppController.openWaveform() click that immediately follows it in the same step's click
    /// list.
    void addTourDemoWaveformCircuit();

    /// Commits an inline label edit on \a element: pushes a CanvasUpdateCommand only if \a
    /// newLabel actually differs from the element's current label. Mirrors
    /// InlineLabelEditor::commit()'s undo-command logic (the QLineEdit-hosting QGraphicsProxyWidget
    /// itself has no Quick equivalent to port; ElementEditor.qml's inline TextInput calls this
    /// directly instead).
    Q_INVOKABLE void commitInlineLabelEdit(GraphicElement *element, const QString &newLabel);

    /// Pushes a CanvasMorphCommand morphing every selected element to \a type. Mirrors
    /// ElementContextMenu::exec()'s "Morph to..." submenu action -- unlike nextElm()/prevElm()'s
    /// fixed next/prev-in-cycle morph, this morphs to a caller-chosen target type.
    void morphSelectionTo(ElementType type);

    /// Renders \a ic's internalElements() into a scaled-to-fit preview image, mirroring
    /// ICRenderer::generatePreviewPixmap()'s scale/pad/render shape but using the same
    /// offscreen-QPainter-plus-real-paint() technique this class's own gate rendering already
    /// uses (not QGraphicsScene::render(), which crashes without a real QApplication -- see
    /// ICRenderer.cpp's own guard). \a ic must really be an IC (checked internally); returns a
    /// null QImage for a non-IC, an IC with no internal elements, or one exceeding the same
    /// element-count guard ICPreviewPopup::MaxElementCount uses in production. Shows
    /// internalElements() (the post-boundary-substitution simulation graph, with proxy Nodes
    /// at I/O boundaries) rather than the pre-substitution designed circuit
    /// generatePreviewPixmap() renders -- a real, small, deliberately-accepted visual
    /// difference, not an oversight (the pre-substitution item list isn't available at hover
    /// time without re-parsing the source file).
    [[nodiscard]] QImage renderICPreviewImage(GraphicElement *ic) const;

    /// Union of every element's and connection's bounding rect, in canvas coordinates. Mirrors
    /// Scene::itemsBoundingRect() -- confirmed to be plain inherited
    /// QGraphicsScene::itemsBoundingRect() (Scene has no override of its own), which is why
    /// this sums over connections() too, not just elements(): a wire's Bezier curve can extend
    /// past both endpoint gates' own bounds.
    [[nodiscard]] QRectF elementsBoundingRect() const;

    /// Maximum width/height (in pixels) renderExportImage() will allocate. Mirrors
    /// CircuitExporter::kMaxImageDimension in the Widgets reference implementation. Bounded
    /// well under 8192 (where width*height*4 bytes hits Qt's own default 256 MB
    /// QImageIOHandler allocation limit). Public: TestQuickFileHandlerSecurity checks real
    /// exported image dimensions against this same cap, not a duplicated magic number.
    static constexpr double kMaxImageDimension = 4096;

    /// Renders elements()+connections() into a transparent-filled QImage bounded by
    /// CircuitExporter::kMaxImageDimension per side (1:1 below that, scaled down to fit
    /// otherwise -- mirrors CircuitExporter::renderScaledImage()'s exact contract, including
    /// \a paddedRect meaning scene-coordinate content to render with the caller's own padding
    /// already applied). Uses the same offscreen-QPainter-plus-real-paint() technique as
    /// renderICPreviewImage(), generalized from a single IC's internalElements() to this
    /// canvas's whole element+connection set -- CircuitExporter itself calls
    /// QGraphicsScene::render(), unusable here for the same reason renderICPreviewImage() and
    /// every other QGraphicsScene-dependent production method this rewrite has ported needed a
    /// substitute technique instead of a literal port.
    [[nodiscard]] QImage renderExportImage(const QRectF &paddedRect) const;

    /// Renders elements()+connections() (padded by 64px, mirroring
    /// CircuitExporter::paddedBoundingRect()) and saves the result to \a filePath. Mirrors
    /// CircuitExporter::renderToImage()'s exact wrapper shape around renderScaledImage().
    /// Throws Pandaception if the image can't be saved (e.g. an unwritable path or unrecognized
    /// extension), matching production.
    void exportToImage(const QString &filePath) const;

    /// Renders elements()+connections() to a landscape A4 PDF at \a filePath. Mirrors
    /// CircuitExporter::renderToPdf() exactly (same QPrinter setup, same 64px padding via
    /// elementsBoundingRect().adjusted(-64, -64, 64, 64)) but paints via renderExportImage()'s
    /// shared paintElementsInto() helper instead of QGraphicsScene::render(). Throws
    /// Pandaception if the QPainter cannot begin painting to the printer, matching production.
    void exportToPdf(const QString &filePath) const;

    /// Paints elements()+connections() into \a painter, scaled and centered to fit \a source
    /// (canvas coordinates) into \a target (painter/device coordinates), preserving aspect
    /// ratio -- mirrors QGraphicsScene::render()'s own target/source/Qt::KeepAspectRatio
    /// contract (the default aspectRatioMode every CircuitExporter call site relies on implicitly),
    /// since that's the real method renderExportImage()/exportToPdf() stand in for. Shared by
    /// both so the fit-and-center math has exactly one implementation. Public (not just
    /// renderExportImage()/exportToPdf()'s private implementation detail) so QuickFileHandler's
    /// export_image (SVG/PDF paths) can paint at an arbitrary content-fitted rect instead of
    /// exportToPdf()'s fixed A4-landscape/64px-padding shape -- mirrors FileHandler.cpp's own
    /// direct Scene::render() calls for the same MCP command.
    void paintElementsInto(QPainter *painter, const QRectF &target, const QRectF &source) const;

    /// Clears the current selection (setSelected(false) on every element, deselect from
    /// m_selectedIds too), without pushing any command. Mirrors Scene::clearSelection() --
    /// used by QuickExportController's pre-export "hide selection handles from the exported
    /// file" step (ExportController::exportPdfDialog()/exportImageDialog()'s own first line),
    /// and by Main.qml's Edit > Clear Selection menu item (Q_INVOKABLE for that direct QML
    /// call; every other caller is C++).
    Q_INVOKABLE void clearSelection();

    /// Clears the current selection and selects just \a element, without pushing any command --
    /// same shape as selectAll() but for exactly one element. Used by QuickElementEditor::
    /// cycleSelection() (Tab/Shift+Tab element cycling, mirrors ElementTabNavigator::
    /// eventFilter()'s own scene->clearSelection()/elm->setSelected(true) pair).
    void selectOnly(GraphicElement *element);

    /// Last known cursor position in world (canvas/scene) coordinates, updated on every mouse
    /// press/move. Mirrors Scene::mousePos(); used to place pasted elements relative to the
    /// cursor.
    [[nodiscard]] QPointF mousePos() const { return m_lastMousePos; }

    // --- Pan/zoom (mirrors App/Scene/GraphicsView.h's discrete zoom ladder and pan gestures)
    //
    // Introduces a real screen/world coordinate split this canvas didn't have before: every
    // element/port/wire position (pos(), boundingRect(), SpatialIndex entries) stays in WORLD
    // coordinates, unaffected by pan/zoom, exactly as it always has. What's new is the mapping
    // between that world space and this item's own local (screen/QML-facing) coordinate space,
    // via screenToWorld()/worldToScreen() below -- the same job QGraphicsView performs
    // implicitly for Scene via its view transform, done explicitly here since this canvas has
    // no QGraphicsView underneath it. mousePressEvent()/mouseMoveEvent()/etc. convert incoming
    // event positions through screenToWorld() before any spatial reasoning (hit-testing via
    // m_index, drag deltas); updatePaintNode() applies the inverse via one QSGTransformNode
    // wrapping its existing (still world-coordinate) geometry, so no vertex-building code
    // needed to change. Signals that position QML overlays (elementContextMenuRequested/
    // emptyContextMenuRequested/inlineEditRequested) keep emitting screen coordinates --
    // worldToScreen() is applied at the point of emission -- since those are consumed directly
    // as canvasHost-local pixel coordinates by Main.qml, unaware of this canvas's internal
    // world space.

    /// Converts \a screenPt (this item's own local/QML-facing coordinates) to world (scene)
    /// coordinates, accounting for the current pan/zoom. Inverse of worldToScreen().
    [[nodiscard]] QPointF screenToWorld(const QPointF &screenPt) const;
    /// Converts \a worldPt (scene coordinates, matching every GraphicElement's pos()) to this
    /// item's own local/QML-facing screen coordinates, accounting for the current pan/zoom.
    /// Inverse of screenToWorld().
    [[nodiscard]] QPointF worldToScreen(const QPointF &worldPt) const;

    /// Current zoom scale factor (1.0 == 1:1). 1.25^zoomLevel, matching
    /// GraphicsView::zoomIn()/zoomOut()'s exact reciprocal step (1.25 in, 0.8 out) so a zoom
    /// in then out returns to exactly the original scale without floating-point drift.
    [[nodiscard]] qreal zoomScale() const;
    /// Returns true if zooming in further is possible. Mirrors GraphicsView::canZoomIn().
    [[nodiscard]] bool canZoomIn() const;
    /// Returns true if zooming out further is possible. Mirrors GraphicsView::canZoomOut().
    [[nodiscard]] bool canZoomOut() const;
    /// Increases the zoom level by one step, anchored on \a screenAnchor (defaults to this
    /// item's own center) -- the world point under \a screenAnchor stays under it after the
    /// zoom, mirroring GraphicsView's AnchorUnderMouse viewport behavior. Mirrors
    /// GraphicsView::zoomIn().
    void zoomIn(std::optional<QPointF> screenAnchor = std::nullopt);
    /// Decreases the zoom level by one step, anchored on \a screenAnchor. Mirrors
    /// GraphicsView::zoomOut().
    void zoomOut(std::optional<QPointF> screenAnchor = std::nullopt);
    /// Resets zoom to 1:1 and pan to the origin. Mirrors GraphicsView::resetZoom() -- also
    /// resets pan, unlike GraphicsView (whose scrollbars are separate state resetTransform()
    /// doesn't touch), since this canvas has no independent scroll-position concept to leave
    /// alone; a bare "reset zoom" that left pan wherever it was would be a surprising partial
    /// reset.
    void resetZoom();
    /// Scales and pans so the whole circuit -- or the current selection, if any -- fits within
    /// this item's own visible bounds, snapping to the nearest discrete zoom step that still
    /// fits. Mirrors GraphicsView::zoomToFit() exactly, including its selection-or-whole-circuit
    /// choice and floor-not-round snapping (never overshoots the viewport).
    void zoomToFit();

    /// Pans so \a worldPoint is centered in this item's own visible bounds, preserving the
    /// current zoom level. Mirrors GraphicsView::centerOn() -- used by the minimap's
    /// click/drag-to-navigate gesture (QuickMinimap::navigateTo()) -- but sets m_panOffset
    /// directly instead of GraphicsView's scrollbar-based implementation, the same way
    /// zoomIn()/zoomOut()'s anchor math does.
    void centerOn(const QPointF &worldPoint);

    /// The world-coordinate rect currently visible on screen: this item's own local bounds
    /// (0,0)-(width(),height()) mapped through screenToWorld(). Mirrors
    /// GraphicsView::mapToScene(viewport()->rect()).boundingRect().
    [[nodiscard]] QRectF visibleWorldRect() const;

    /// World-coordinate range the canvas ScrollBars travel over: elementsBoundingRect() unioned
    /// with visibleWorldRect() (same "never more restrictive than what's already on screen"
    /// precedent as minimapContentRect()), padded by kScrollMargin on every side so content
    /// flush against an edge isn't immediately at a scroll extreme. Unlike QGraphicsView's
    /// sceneRect(), this has no fixed, independently-settable bound -- it grows to keep
    /// following wherever the view already is, exactly like minimapContentRect()'s identical
    /// tradeoff (documented there); the scrollbars stay meaningful (draggable, appropriately
    /// sized) without needing a real "canvas extent" concept this class doesn't otherwise have.
    [[nodiscard]] QRectF scrollableWorldRect() const;
    [[nodiscard]] qreal horizontalScrollPosition() const;
    void setHorizontalScrollPosition(qreal position);
    [[nodiscard]] qreal horizontalScrollSize() const;
    [[nodiscard]] qreal verticalScrollPosition() const;
    void setVerticalScrollPosition(qreal position);
    [[nodiscard]] qreal verticalScrollSize() const;

    /// World-coordinate content rect for a minimap thumbnail of size \a targetWidth x
    /// \a targetHeight: elementsBoundingRect() unioned with visibleWorldRect() (so the minimap
    /// is never more zoomed-in than the main view, even on an empty or tiny circuit), then
    /// symmetrically expanded on its shorter axis to exactly match the target aspect ratio.
    /// Mirrors MinimapWidget::computeTransform()'s src computation, minus the
    /// QGraphicsScene::sceneRect() term -- CanvasItem has no equivalent stable "navigable area"
    /// concept distinct from its items' own bounds, so elementsBoundingRect() already serves
    /// the "defensive superset" role sceneRect() plays in production. Returns an empty rect if
    /// there is nothing to show yet (\a targetWidth/\a targetHeight <= 0, or both source terms
    /// are empty).
    [[nodiscard]] QRectF minimapContentRect(qreal targetWidth, qreal targetHeight) const;

    /// Renders a minimap thumbnail of elements()+connections() at \a targetWidth x
    /// \a targetHeight, fit via minimapContentRect() -- since that rect is pre-grown to the
    /// target aspect ratio, the fit is always exact (no letterboxing offset for the caller to
    /// track, unlike renderExportImage()/paintElementsInto()'s general case). Transparent
    /// background; QML paints the minimap's own background/border. Uses the same
    /// offscreen-QPainter-plus-real-paint() technique as renderICPreviewImage()/
    /// renderExportImage() (paintElementsInto()). Returns a null QImage if minimapContentRect()
    /// is empty.
    [[nodiscard]] QImage renderMinimapImage(qreal targetWidth, qreal targetHeight) const;

signals:
    /// Emitted whenever the zoom level or pan offset changes. Mirrors GraphicsView::zoomChanged()
    /// (extended to cover pan too, since this canvas has no separate scroll-position signal the
    /// way GraphicsView's scrollbars would have provided one).
    void zoomChanged();

    /// Emitted whenever the hovered-port highlight set changes: the port under the cursor plus
    /// every port connected to it via a wire, so the user can trace where a wire's far end
    /// lands without following it by hand. Mirrors ConnectionManager::setHoverPort()'s
    /// highlight/showHoverLabels() combined -- collapsed into one signal/one stage (no separate
    /// tooltip-delay reveal) since Quick has no QToolTip-timed helpEvent() to mirror. An empty
    /// list means "hide" (nothing hovered, or the port left the canvas). Each QVariantMap has
    /// screenX/screenY/radius (canvas-space -- this canvas has no pan/zoom yet, so "screen" and
    /// "world" coincide 1:1), side ("left"/"right", biased by isInput()/isOutput()), text (the
    /// port's name, empty for an unnamed port), and ringColor/labelBgColor/labelTextColor (real
    /// QColor values from ThemeManager, not generic Qt Quick palette roles -- see
    /// buildPortHoverChips()'s own doc comment for why).
    void portHoverChanged(const QVariantList &chips);

    /// Emitted whenever the set of selected elements changes, or when an already-selected
    /// element's properties change in a way a property inspector needs to re-read (the
    /// deliberate reselect-to-refresh toggle in adjustMainProperty()/adjustSecondaryProperty()).
    /// CanvasItem has no QGraphicsScene base to inherit QGraphicsScene::selectionChanged() from,
    /// so every selection-mutating method emits this explicitly. Mirrors
    /// QGraphicsScene::selectionChanged(), which ElementEditor::setScene() connects to in
    /// production.
    void selectionChanged();

    /// Emitted on right-click over an element (mousePressEvent() has already selected it if it
    /// wasn't already part of the selection, mirroring Scene::contextMenu()). \a pos is this
    /// item's own local/screen coordinates (not world -- Main.qml positions the popup menu
    /// directly at it). Mirrors Scene::contextMenuPos().
    void elementContextMenuRequested(GraphicElement *element, QPointF pos);
    /// Emitted on right-click over empty canvas. Mirrors Scene::contextMenu()'s "no item"
    /// branch (a separate, simpler Paste/Select-all menu built inline there rather than via
    /// ElementContextMenu::exec()). \a pos is screen coordinates, same as
    /// elementContextMenuRequested()'s.
    void emptyContextMenuRequested(QPointF pos);

    /// Emitted on double-click over a non-IC element with hasLabel() true. \a currentLabel is
    /// passed explicitly (rather than left for QML to read off \a element) since
    /// GraphicElement -- Layer 1 domain code -- has no Q_PROPERTY/Q_INVOKABLE QML can call;
    /// QML only ever holds \a element opaquely and passes it back into
    /// commitInlineLabelEdit(). \a targetRect is \a element's labelSceneBoundingRect(),
    /// converted from world to this item's own local/screen coordinates (worldToScreen()) to
    /// position an inline editor over -- Main.qml places the TextField directly at this rect,
    /// in canvasHost-local pixels. Mirrors GraphicElement::mouseDoubleClickEvent()'s inlineEditRequested()
    /// emission -- CanvasItem's own mouseDoubleClickEvent() override never forwards to the
    /// element's (it's dedicated to wire-splitting), so this replicates the same hasLabel()
    /// check directly instead of fabricating a QGraphicsSceneMouseEvent to feed the real method.
    void inlineEditRequested(GraphicElement *element, QString currentLabel, QRectF targetRect);

    /// IC hover-preview lifecycle, mirroring Scene::icPreviewRequested()/icPreviewMoved()/
    /// icPreviewHideRequested()/icPreviewCancelRequested() -- Scene re-emits these from each
    /// IC's own IC::previewRequested()/etc. signals (connected per-addItem()/removeItem());
    /// hoverMoveEvent()/hoverLeaveEvent() call IC's real signals directly, so these are emitted
    /// from those exact call sites instead of needing the same per-IC connect()/disconnect()
    /// bookkeeping Scene does. \a element is
    /// always really an IC (never a plain GraphicElement) -- GraphicElement*, not IC*, to match
    /// elementContextMenuRequested()'s precedent (IC isn't QML-registered either).
    void icPreviewRequested(GraphicElement *element, QPoint screenPos);
    void icPreviewMoved(GraphicElement *element, QPoint screenPos);
    void icPreviewHideRequested();
    void icPreviewCancelRequested(GraphicElement *element);

    /// Emitted from mouseDoubleClickEvent()'s IC branch, mirroring IC::handleDoubleClick()'s own
    /// requestOpenSubCircuit(id(), blobName(), file()) exactly (Scene::icOpenRequested() is the
    /// Widgets-side re-emission of the same signal) -- \a blobName is non-empty for an embedded
    /// IC, \a filePath is non-empty for a file-backed one, never both. QuickWorkspaceManager
    /// connects to this per-tab in createNewTab() and resolves it the same way
    /// SceneUiBinder.cpp's identical connection does: an embedded IC's blob is read from this
    /// canvas's own icRegistry() and opened inline (openICInTab()); a file-backed IC's path is
    /// opened as a new top-level tab (loadPandaFile()).
    void icOpenRequested(int elementId, QString blobName, QString filePath);

protected:
    /// \reimp Builds the batched geometry nodes (gates, wires, selection overlay) from current state.
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;
    /// \reimp Hit-tests via m_index; toggles a clicked input, or starts a drag/rubber-band.
    void mousePressEvent(QMouseEvent *event) override;
    /// \reimp Drives the in-progress element drag or rubber-band rect.
    void mouseMoveEvent(QMouseEvent *event) override;
    /// \reimp Ends the in-progress element drag or rubber-band rect.
    void mouseReleaseEvent(QMouseEvent *event) override;
    /// \reimp Double-click on a fully-connected wire inserts a routing node, splitting it into
    /// two segments (CanvasSplitCommand). Mirrors SceneInteraction::mouseDoubleClick(); picks
    /// this canvas's deferred wire-splitting gesture back up.
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    /// \reimp Updates the hovered element for the next paint's highlight.
    void hoverMoveEvent(QHoverEvent *event) override;
    /// \reimp Clears hover highlight when the pointer leaves the canvas entirely.
    void hoverLeaveEvent(QHoverEvent *event) override;
    /// \reimp Fires keyboard-triggered InputSwitch/InputButton elements (GraphicElement::
    /// trigger()); also dispatches undo/redo, clipboard, nudge, and rotate/flip shortcuts,
    /// mirroring Scene::keyPressEvent()'s dispatch order.
    void keyPressEvent(QKeyEvent *event) override;
    /// \reimp Releases a momentary InputButton keyboard trigger (InputSwitch stays latched).
    void keyReleaseEvent(QKeyEvent *event) override;
    /// \reimp Zooms in/out by one step, anchored on the cursor position -- mirrors
    /// GraphicsView::wheelEvent()'s AnchorUnderMouse-plus-centerOn() correction.
    void wheelEvent(QWheelEvent *event) override;
    /// \reimp visibleWorldRect() (hence every horizontal/verticalScroll* property above) reads
    /// width()/height() directly in C++, a dependency QML's binding tracker can't see the way
    /// it sees a QML-level property read -- so a size-only change (e.g. this item not having
    /// its final width()/height() yet the first time a ScrollBar's `size:` binding evaluates,
    /// well before any real pan/zoom action ever fires zoomChanged()) would otherwise leave
    /// those bindings stuck on a stale, computed-against-zero-size result forever. Re-emitting
    /// zoomChanged() here (the same signal every other visibleWorldRect()-affecting mutation
    /// already emits) closes that gap the same way, rather than inventing a second signal.
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

private:
    void buildDemoCircuit();
    void rebuildSpatialIndex();
    /// Incrementally refreshes \a element's own SpatialIndex box, each of its ports' boxes
    /// (invalidating their m_portScenePosCache entry too, since they just moved), and every
    /// wire attached to those ports -- via Port::connections(), not a scan over m_connections,
    /// so this stays O(this element's own port/wire fan-out), not O(every wire on the canvas).
    /// Used by mouseMoveEvent()'s drag branch in place of a full rebuildSpatialIndex() call on
    /// every mouse-move sample.
    void updateSpatialIndexFor(GraphicElement *element);
    /// Returns \a port's SpatialIndex id, assigning a fresh one on first sight. Port is not an
    /// ItemWithId (see Port.h), so unlike elements/connections it has no real id of its own to
    /// tag -- this is the single source of truth for that id, used by both rebuildSpatialIndex()
    /// and updateSpatialIndexFor() so the two paths can never disagree about which id names a
    /// given Port* (required for updateSpatialIndexFor()'s incremental update to be consistent
    /// with a full rebuild, e.g. across an undo/redo round-trip).
    quint64 spatialIdFor(Port *port);
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
    /// same signal; right-clicking empty space emits emptyContextMenuRequested(). \a screenPos
    /// is this item's own local coordinates (screenToWorld() is applied internally for
    /// hit-testing; the signals themselves keep emitting screenPos unconverted).
    void handleRightClick(const QPointF &screenPos);

    /// Returns true if \a pos (world/scene coordinates) lands on one of \a owner's own ports. Reimplements
    /// IC::isCursorOverPort()'s decision against this canvas's own SpatialIndex/m_portsById
    /// rather than calling IC's private isCursorOverPort()/protected hoverXxxEvent()
    /// overrides directly -- same pattern as activateOnPress() reimplementing
    /// InputSwitch/InputRotary's mousePressEvent logic instead of calling it.
    bool isOverOwnPort(GraphicElement *owner, const QPointF &pos) const;

    /// Shared by paintElementsInto()/paintElementsSimplifiedInto(): sets \a painter's transform
    /// so \a source (canvas coordinates) fits centered into \a target (painter/device
    /// coordinates), preserving aspect ratio -- mirrors QGraphicsScene::render()'s own
    /// target/source/Qt::KeepAspectRatio contract. Caller owns the surrounding
    /// painter->save()/restore() pair.
    void applyContentFitTransform(QPainter *painter, const QRectF &target, const QRectF &source) const;
    /// Large-circuit fallback used by renderMinimapImage() when the circuit is bigger than
    /// kMinimapSimplifiedThreshold elements: each element becomes a flat filled rect (no real
    /// paint()/SVG rasterization, no port glyphs/label text-shaping) and each wire a plain
    /// straight line between its two ports' real scenePos() (no Bezier tessellation) --
    /// individual element/wire detail is imperceptible at the minimap's thumbnail scale
    /// regardless, and this trades it away for real per-call cost too small to matter even on
    /// an 8000+-element circuit.
    void paintElementsSimplifiedInto(QPainter *painter, const QRectF &target, const QRectF &source) const;

    // --- Wire-creation-by-dragging (ports ConnectionManager's workflow) ---
    void startWireFromOutput(OutputPort *startPort);
    void startWireFromInput(InputPort *endPort);
    void tryCompleteWire(const QPointF &pos);
    void cancelEditedWire();
    void detachWire(InputPort *endPort);
    void updateEditedWireEnd(const QPointF &pos);

    /// Re-derives the port under \a pos (same topmost-hit lookup tryCompleteWire() uses) and,
    /// if it differs from the currently tracked hover port, emits portHoverChanged() with a
    /// freshly built chip list. A no-op if the hovered port hasn't changed, mirroring
    /// ConnectionManager::updateHover()'s identical "only tear down and rebuild... when the
    /// port under the cursor actually changes" guard.
    void updatePortHover(const QPointF &pos);
    /// Emits portHoverChanged({}) if a port was tracked as hovered, then clears the tracking.
    /// Mirrors ConnectionManager::clearHover().
    void clearPortHover();
    /// Builds the highlight/label payload for \a hoverPort itself plus every port connected to
    /// it. Mirrors ConnectionManager::connectedPeers() + showHoverLabels()'s combined chip
    /// data. Returns an empty list for a null \a hoverPort. Uses real ThemeManager colors, not
    /// generic Qt Quick palette roles: this app's Fusion-style palette leaves the generic
    /// tooltip roles unthemed for the dark palette (white-on-white, invisible label text);
    /// ThemeManager's own m_portHoverPort/m_portHoverLabelBg/m_portHoverLabelText -- the colors
    /// PortHoverLabel's original Widgets implementation painted with -- sidestep that gap
    /// entirely.
    [[nodiscard]] QVariantList buildPortHoverChips(Port *hoverPort) const;

    /// Builds an appearance-cache-key string for \a element's current pixmap identity,
    /// rotation, flip, and selection state -- see this class's doc comment for why that's
    /// enough for every element family except Display7/14/16 (not yet ported). Not static:
    /// needs m_wiresVisible, since that state decides whether this element's own port glyphs
    /// get baked into its tile at all (see isPortHidden()).
    QString appearanceKeyFor(GraphicElement *element) const;

    /// Shared by pasteAction()/duplicateAction(): deserializes from \a stream and adds the
    /// result via CanvasAddItemsCommand. Without \a fixedOffset the new items are placed
    /// relative to the cursor (paste); with it they are shifted by exactly that vector from
    /// the originals (duplicate). Returns the items added. Mirrors ClipboardManager::
    /// deserializeAndAdd().
    QList<ItemWithId *> deserializeAndAdd(QDataStream &stream, const QVersionNumber &version,
                                          std::optional<QPointF> fixedOffset = std::nullopt);

    /// Returns true if \a element should be skipped entirely by rendering/hit-testing under
    /// the current Show Gates/Wires state -- mirrors VisibilityManager::showGates()'s
    /// Input/Output/Other-always-visible carve-out and showWires()'s "a Node is meaningless
    /// without wires" whole-element hide.
    [[nodiscard]] bool isElementHidden(const GraphicElement *element) const;
    /// Returns true if \a port's own glyph should be skipped under the current Show Wires
    /// state -- mirrors VisibilityManager::showWires()'s "for other elements, hide only their
    /// port handles" branch (a Node's ports are already covered by isElementHidden() hiding
    /// the whole element, so this always returns false for a Node's own ports).
    [[nodiscard]] bool isPortHidden(const Port *port) const;

    std::unique_ptr<SimulationHost> m_host;
    std::unique_ptr<Simulation> m_simulation;
    QVector<GraphicElement *> m_elements; // owned; never added to a QGraphicsScene
    QVector<Connection *> m_connections;  // owned; never added to a QGraphicsScene
    // O(1) membership companions to m_elements/m_connections: addItem()'s idempotency check
    // (needed for callers like buildDemoCircuit() that pre-populate the vector directly) was a
    // QVector::contains() linear scan, making a large sequential file load O(n^2) in the
    // element/connection count. Kept in lockstep with the vectors at every add/remove site.
    QSet<GraphicElement *> m_elementSet;
    QSet<Connection *> m_connectionSet;
    QHash<quint64, GraphicElement *> m_elementsById;
    QHash<quint64, Port *> m_portsById; // ports aren't owned here, they're owned by m_elements
    SpatialIndex m_index;

    /// Real id/registry layer -- see itemById()/nextId()/updateItemId()/addItem()/removeItem()
    /// above. Ported unmodified from Scene: a bare QHash<int, ItemWithId*> plus a monotonic
    /// counter, with no back-reference to whatever owns it. elementId()/wireId() (the
    /// SpatialIndex tagged-id scheme) tag the real ItemWithId::id() this assigns, not a loop
    /// index.
    SceneItemRegistry m_itemRegistry;

    /// Embedded-IC blob registry backing icRegistry() -- see CanvasICRegistry's own doc
    /// comment for what's ported vs. deliberately narrower than production ICRegistry.
    CanvasICRegistry m_icRegistry{this};

    quint64 m_hoveredId = 0;
    QSet<quint64> m_selectedIds;

    /// The port currently hovered for peer-highlighting purposes, or nullptr. A raw pointer,
    /// not an id-resolved lookup (unlike a future real id/registry layer's version of this) --
    /// this canvas has no delete gesture yet for a dangling-pointer race to matter.
    Port *m_hoveredPort = nullptr;

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
    /// getters to production Wiring/Connection.h for this canvas-only need.
    QPointF m_editedWireFreeEnd;

    /// The InputButton currently held down, or nullptr. Set on press, cleared (and released
    /// back to off) on release -- regardless of where the release happens, matching the real
    /// mouse-grab semantics InputButton::mouseReleaseEvent relies on (the item that was
    /// pressed keeps receiving the release even if the cursor has moved off it).
    GraphicElement *m_pressedInputButton = nullptr;

    /// This canvas's real undo stack -- receiveCommand() pushes onto it, matching
    /// Scene::receiveCommand()/m_undoStack exactly. See this class's doc comment on
    /// CanvasCommands.h for which command families are ported onto it so far.
    QUndoStack m_undoStack;

    /// Per-drag snapshot: parallel to m_dragElements, captured at press. No QPointer-style
    /// destroyed-mid-drag safety net yet (unlike SceneInteraction's m_dragSnapshot) -- this
    /// canvas has no delete gesture yet for that race to matter.
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

    /// Offscreen-render cache backing real per-element appearance -- see this class's doc
    /// comment and TextureAtlas's own for the design.
    TextureAtlas m_atlas;

    /// The dot-grid's repeating tile texture (see updatePaintNode()'s grid block for why it's a
    /// tile, not a per-dot quad) -- not owned by any QSGTextureMaterial (Qt's own contract:
    /// "the material does not take ownership of texture"), so this class owns and deletes it,
    /// same as TextureAtlas::m_texture's identical pattern. Rebuilt only when m_zoomLevel or the
    /// dot's theme color no longer matches what it was built for.
    QSGTexture *m_gridDotTexture = nullptr;
    int m_gridDotTextureZoomLevel = std::numeric_limits<int>::min();
    QColor m_gridDotTextureColor;

    /// Per-element memoization of updatePaintNode()'s gate-rendering loop: without it,
    /// appearanceKeyFor()'s per-port string-building and GraphicElement::boundingRect()'s
    /// port-walking cost are paid for every element on every single frame regardless of
    /// whether anything actually changed. Every field here is exactly what appearanceKeyFor()/
    /// the gate loop's own localRect computation reads -- comparing them directly (an int64, a
    /// handful of bool/int reads, no string construction) is cheap, so the cache can never be
    /// stale relative to what it memoizes. Port status (inputs and outputs) is covered by
    /// GraphicElement::isRenderDirty(), an O(1) field read pushed by InputPort::setStatus()/
    /// OutputPort::setStatus() on a genuine change; port *count* (boundingRect()'s own
    /// dependency via portsBoundingRect()) is covered explicitly by inputSize/outputSize below.
    /// Show Gates/Wires-driven hidden-port state isn't a per-element field at all:
    /// setGatesVisible()/setWiresVisible() clear this whole cache on toggle instead (a per-
    /// element dirty flag can't reactively observe a global toggle).
    struct ElementRenderCache {
        qint64 pixmapCacheKey = 0;
        qreal rotation = 0.0;
        bool flipX = false;
        bool flipY = false;
        bool selected = false;
        int inputSize = 0;
        int outputSize = 0;
        /// Mirrors appearanceKeyFor()'s own label-text key dimension (Text's empty-state hint
        /// visibility is entirely derived from this same text, so no separate dimension needed).
        QString labelText;
        QRectF localRect;
        TextureAtlas::TileLocation tile;
    };
    QHash<GraphicElement *, ElementRenderCache> m_elementRenderCache;

    /// Per-port memoization of updatePaintNode()'s wire-rendering loop: Port::scenePos() is
    /// otherwise recomputed for every port of every connection on every frame even though a
    /// port only moves when its owning element is moved/rotated/flipped or the topology
    /// changes. Invalidated
    /// wholesale by rebuildSpatialIndex() (add/delete/paste/undo/redo/rotate/flip/morph --
    /// every structural change) and per-port by updateSpatialIndexFor() (element drag) --
    /// either way, invalidation always rides along an event that was already happening, never
    /// a new one; wire *color* stays a fresh per-frame read regardless, since that's what a
    /// live simulation actually changes.
    QHash<Port *, QPointF> m_portScenePosCache;

    /// Memoizes renderMinimapImage() itself: an uncached render can cost over a second on a
    /// large circuit, and would otherwise re-trigger on every pan/zoom pause even though
    /// minimapContentRect()'s viewport-union is a no-op whenever the viewport is already inside
    /// the circuit's own bounds (the common case for any circuit big enough for this cost to
    /// matter). `mutable` since renderMinimapImage() is const; invalidated the same way
    /// m_portScenePosCache is -- wholesale inside rebuildSpatialIndex(). A target-size change (the
    /// minimap widget itself being resized) also forces a rebuild, checked directly in
    /// renderMinimapImage() rather than via a separate invalidation path.
    mutable QImage m_cachedMinimapImage;
    mutable QRectF m_cachedMinimapContentRect;
    mutable QSizeF m_cachedMinimapTargetSize;
    /// Real-rebuild counter for renderMinimapImage() -- test-only observability (see
    /// TestCanvasItemSmoke's minimap-caching tests), incremented only when the cache above is
    /// actually rebuilt, never on a skipped/reused call.
    mutable int m_minimapRebuildCount = 0;

    /// Backs spatialIdFor() -- see its own doc comment. Pruned per-port by removeItem(
    /// GraphicElement*) so it never grows unbounded across a long editing session's add/delete
    /// churn, mirroring m_elementRenderCache's identical pruning.
    QHash<Port *, quint64> m_portSpatialIds;
    quint64 m_nextPortSpatialId = 0;

    /// Show Gates / Show Wires toggle state -- see setGatesVisible()/setWiresVisible().
    /// Mirrors VisibilityManager::m_showGates/m_showWires's own defaults (both start visible).
    bool m_gatesVisible = true;
    bool m_wiresVisible = true;

    /// Discrete zoom step (0 == 1:1, positive == zoomed in) -- backs zoomScale()/canZoomIn()/
    /// canZoomOut(). Mirrors GraphicsView::m_zoomLevel and its exact -9..7 range (see
    /// CanvasItem.cpp's kMaxZoomLevel/kMinZoomLevel).
    int m_zoomLevel = 0;
    /// World point currently at this item's own local (0, 0) -- backs screenToWorld()/
    /// worldToScreen(). Mirrors what GraphicsView's scrollbar values + view transform jointly
    /// represent, collapsed into one QPointF since this canvas has no separate widget/viewport
    /// distinction to split it across.
    QPointF m_panOffset;

    /// True while the user is middle-click or space-drag panning. Mirrors GraphicsView::m_pan.
    bool m_panning = false;
    /// True while the space bar is held (enables pan mode). Mirrors GraphicsView::m_space.
    bool m_spacePanHeld = false;
    /// Screen-space cursor position at the start of (or during, updated every move) the
    /// current pan gesture -- mirrors GraphicsView::m_panStartX/m_panStartY, kept as one
    /// QPointF instead of two ints since this canvas already uses QPointF for every other
    /// screen/world position.
    QPointF m_panAnchor;
};
