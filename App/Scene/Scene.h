// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Main circuit editing scene with undo/redo and user interaction.
 */

#pragma once

#include <optional>

#include <QGraphicsScene>
#include <QHash>
#include <QMap>
#include <QMimeData>
#include <QPoint>
#include <QUndoCommand>
#include <QVersionNumber>

#include "App/Core/ContextDirProvider.h"
#include "App/Core/SimulationHost.h"
#include "App/Scene/ClipboardManager.h"
#include "App/Scene/ConnectionManager.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/InlineLabelEditor.h"
#include "App/Scene/PropertyShortcutHandler.h"
#include "App/Scene/SceneDropHandler.h"
#include "App/Scene/SceneInteraction.h"
#include "App/Scene/SceneItemRegistry.h"
#include "App/Scene/VisibilityManager.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Port.h"

class GraphicElement;
class GraphicsView;
class IC;
class ItemWithId;
class Connection;
class QPainter;
struct SerializationContext;
enum class SerializationPurpose;

/**
 * \class Scene
 * \brief Main circuit editing scene.
 *
 * The Scene class extends QGraphicsScene to provide the main circuit editing
 * environment. It manages circuit elements and their connections, handles user
 * interactions like selection, copying, pasting and rotation. Scene also implements
 * undo/redo functionality and handles various user interface events like drag-and-drop,
 * keyboard shortcuts, and mouse operations.
 */
class Scene : public QGraphicsScene, public ContextDirProvider, public SimulationHost
{
    Q_OBJECT

public:
    /// \brief Adds \a item to the scene and registers it in the per-scene ID registry.
    void addItem(QGraphicsItem *item);

    /// \brief Removes \a item from the scene and unregisters it from the per-scene ID registry.
    void removeItem(QGraphicsItem *item);

    /// Tightens the scene rect to item bounds while preserving the viewport position.
    void resizeScene();

    // --- Per-Scene Element Registry ---

    /// Returns the item registered under \a id, or nullptr if not found.
    [[nodiscard]] ItemWithId *itemById(int id) const;

    /// Returns true if an item with \a id is registered in this scene.
    [[nodiscard]] bool contains(int id) const;

    /// Returns the current highest ID in use by this scene.
    int lastId() const;

    /// Advances the scene's ID counter to at least \a newLastId.
    void setLastId(int newLastId);

    /// Returns the next available scene-local ID (does not assign it).
    int nextId();

    /**
     * \brief Reassigns the ID of \a item to \a newId without adding it to the scene.
     * \details Used by undo/redo commands (loadItems, SplitCommand, MorphCommand) to
     * restore an element's original ID before calling addItem(). The item must NOT
     * be in the scene registry yet when this is called.
     */
    void updateItemId(ItemWithId *item, int newId);

    /// Drops a stale registry entry by ID without dereferencing the item.
    /// Needed when Qt's parent-child cascade destroys an item: it removes
    /// the item from QGraphicsScene via the non-virtual base removeItem,
    /// bypassing our override and leaving the m_elementRegistry entry
    /// pointing at freed memory.
    void forgetItemId(int id);

    // --- Lifecycle ---

    /// Constructs a Scene and initialises the undo stack and simulation.
    explicit Scene(QObject *parent = nullptr);

    // --- View / Display Management ---

    /// Returns the GraphicsView currently displaying this scene.
    GraphicsView *view() const;
    /// Sets the GraphicsView that displays this scene to \a view.
    void setView(GraphicsView *view);

    /// Shows or hides gate elements. Delegates to VisibilityManager.
    void showGates(bool checked);

    /// Shows or hides connection wires. Delegates to VisibilityManager.
    void showWires(bool checked);

    /// Returns the visibility manager for this scene.
    VisibilityManager *visibilityManager() { return &m_visibilityManager; }

    // --- Element Access / Queries ---

    /// \reimp
    QList<QGraphicsItem *> items(Qt::SortOrder order = Qt::AscendingOrder) const;
    /// \reimp
    QList<QGraphicsItem *> items(QPointF pos, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape, Qt::SortOrder order = Qt::AscendingOrder, const QTransform &deviceTransform = QTransform()) const;
    /// \reimp
    QList<QGraphicsItem *> items(const QRectF &rect, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape, Qt::SortOrder order = Qt::AscendingOrder, const QTransform &deviceTransform = QTransform()) const;
    /// Returns the list of currently selected graphic elements.
    const QList<GraphicElement *> selectedElements() const;
    /// Returns all graphic elements in the scene.
    const QVector<GraphicElement *> elements() const;
    /// Returns all graphic elements within \a rect.
    const QVector<GraphicElement *> elements(const QRectF &rect) const;
    /// Returns the path of the first local `.panda` file in \a mimeData's URL list (the file
    /// a file-manager drop would open), or an empty string if there is none.
    static QString droppedPandaFile(const QMimeData *mimeData);
    /// Returns all graphic elements without the topological sort — for hot
    /// paths (key triggers, mute) that don't care about evaluation order.
    const QVector<GraphicElement *> unsortedElements() const;
    /// Returns \a elements sorted in topological dependency order (inputs first).
    static QVector<GraphicElement *> sortByTopology(QVector<GraphicElement *> elements);
    /**
     * \brief Returns a map from wireless channel label to the Tx node's input port.
     * \details Scans \a elements for nodes in WirelessMode::Tx, keyed by label.
     * If two Tx nodes share the same label the first one wins.  Used by codegens
     * to resolve Rx node signals without duplicating the wireless scan.
     */
    static QHash<QString, InputPort *> wirelessTxInputPorts(const QVector<GraphicElement *> &elements);
    /// Returns all visible (non-hidden) graphic elements in the scene.
    const QVector<GraphicElement *> visibleElements() const;

    /**
     * \brief Returns \c true if a wire from \a startPort to \a endPort is permitted.
     * \details Delegates to ConnectionManager::isConnectionAllowed().
     */
    static bool isConnectionAllowed(OutputPort *startPort, InputPort *endPort);

    /// Emits showStatusMessageRequested() so the bound chrome can surface \a message in the
    /// status bar (used to explain a rejected wire instead of dropping it silently).
    void showStatusMessage(const QString &message);

    // --- Connection Manager ---

    /// Returns the connection manager that handles wire creation, deletion, and hover feedback.
    ConnectionManager *connectionManager() { return &m_connectionManager; }

    // --- Clipboard Manager ---

    /// Returns the clipboard manager that handles copy, cut, paste and clone-drag.
    ClipboardManager *clipboardManager() { return &m_clipboardManager; }

    // --- Property Shortcut Handler ---

    /// Returns the handler for keyboard shortcuts that cycle element properties.
    PropertyShortcutHandler *propertyShortcutHandler() { return &m_propertyShortcutHandler; }

    // --- Hit-testing ---

    /// Returns the topmost item at \a pos, prioritising ports over elements.
    QGraphicsItem *itemAt(QPointF pos) const;

    /// Returns the last known mouse position in scene coordinates.
    [[nodiscard]] QPointF mousePos() const { return m_interaction.lastMousePos(); }

    /// Opens the element/selection context menu at \a screenPos (driven by SceneInteraction on right-click).
    void contextMenu(const QPoint screenPos);

    // --- Adding Items ---

    /// Deserializes and adds items from \a mimeData to the scene. If \a scenePos is set,
    /// the new element is placed there (used by the palette's double-click / search add so
    /// it lands in view); otherwise it keeps its default position (drop path).
    void addItem(QMimeData *mimeData, std::optional<QPointF> scenePos = std::nullopt);

    // --- Clipboard (Copy / Cut / Paste) ---

    /// Copies the selected items to the internal clipboard.
    void copyAction();
    /// Cuts the selected items to the internal clipboard.
    void cutAction();
    /// Pastes items from the internal clipboard into the scene.
    void pasteAction();
    /// Duplicates the selection in place (leaves the system clipboard untouched).
    void duplicateAction();

    // --- Element Operations (Rotate / Flip / Delete / Mute) ---

    /// Deletes the currently selected items.
    void deleteAction();
    /// Rotates selected elements 90 degrees clockwise.
    void rotateRight();
    /// Rotates selected elements 90 degrees counter-clockwise.
    void rotateLeft();
    /// Flips selected elements horizontally.
    void flipHorizontally();
    /// Flips selected elements vertically.
    void flipVertically();

    // --- Element Operations (Align / Distribute) ---

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
    /// Redistributes selected elements with equal horizontal gaps between edges, keeping the
    /// leftmost and rightmost elements fixed as anchors. No-op below 3 elements.
    void distributeHorizontally();
    /// Redistributes selected elements with equal vertical gaps between edges, keeping the
    /// topmost and bottommost elements fixed as anchors. No-op below 3 elements.
    void distributeVertically();

    /// Mutes or unmutes selected elements according to \a mute.
    void mute(const bool mute = true);
    /// Selects all items in the scene.
    void selectAll();

    // --- Element Property Cycling ---

    /// Cycles selection forward to the next element.
    void nextElm();
    /// Cycles selection backward to the previous element.
    void prevElm();
    /// Advances the main property of selected elements to the next value.
    void nextMainPropShortcut();
    /// Retreats the main property of selected elements to the previous value.
    void prevMainPropShortcut();
    /// Advances the secondary property of selected elements to the next value.
    void nextSecndPropShortcut();
    /// Retreats the secondary property of selected elements to the previous value.
    void prevSecndPropShortcut();

    // --- Undo / Redo ---

    /// Returns the scene's undo stack.
    QUndoStack *undoStack();
    /// Returns the redo QAction bound to the undo stack.
    QAction *redoAction() const;
    /// Returns the undo QAction bound to the undo stack.
    QAction *undoAction() const;
    /// Pushes \a cmd onto the undo stack (immediately executes its redo()).
    void receiveCommand(QUndoCommand *cmd);

    // --- Simulation ---

    /// Returns the simulation engine associated with this scene.
    Simulation *simulation();
    /// Marks the simulation mapping as stale so it is rebuilt on the next tick.
    void setCircuitUpdateRequired();
    /// Refreshes visuals and marks the scene dirty without rebuilding the simulation
    /// graph, for property-only edits (label, color, delay, ...) that don't add,
    /// remove, or reconnect elements.
    void setPropertyUpdateRequired();

    /// \reimp SimulationHost
    QList<QGraphicsItem *> simulationItems() const override { return items(); }
    /// \reimp SimulationHost
    void setMuted(const bool muted) override { mute(muted); }

    // --- Context Directory ---

    /// Returns the directory of the .panda file associated with this scene.
    QString contextDir() const override { return m_contextDir; }
    /// Sets the directory of the .panda file associated with this scene.
    void setContextDir(const QString &dir) { m_contextDir = dir; }

    // --- IC Registry ---

    /// Returns the IC definition registry for this scene.
    ICRegistry *icRegistry() { return &m_icRegistry; }

    /// Creates a deserialization context with the scene's contextDir and blob registry.
    /// \a purpose has no default -- every caller must state whether this deserializes a
    /// genuine .panda file or an in-session snapshot; see SerializationPurpose.
    SerializationContext deserializationContext(QHash<quint64, Port *> &portMap, const QVersionNumber &version, SerializationPurpose purpose);

    // --- Autosave ---

    /// Schedules an autosave of the current circuit state.
    void setAutosaveRequired();

    // --- Theme ---

    /// Propagates the current theme to all elements and connections.
    void updateTheme();

    // --- Retranslation ---

    /// Updates undo/redo action text to reflect the current UI language.
    void retranslateUi();

    // --- Event Filter ---

    /// \reimp
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    // --- Signals ---

    /// Emitted whenever the circuit changes (element added/removed/moved).
    void circuitHasChanged();

    /**
     * \brief Emitted when a context menu should appear.
     * \param screenPos    Screen coordinates of the right-click.
     * \param itemAtMouse  Item under the mouse pointer (may be nullptr).
     */
    void contextMenuPos(QPoint screenPos, QGraphicsItem *itemAtMouse);

    /**
     * \brief Emitted after a TruthTable element's output bit is toggled (redo or undo).
     * \param element The TruthTable element that changed.
     */
    void truthTableElementChanged(GraphicElement *element);

    /// Emitted when an IC element is double-clicked to request opening its sub-circuit.
    void icOpenRequested(int elementId, const QString &blobName, const QString &filePath);

    /// Emitted when a .panda file is dropped onto the canvas from the file manager.
    void fileDropRequested(const QString &filePath);

    /// Requests that \a message be shown transiently in the status bar (e.g. why a wire
    /// was rejected). Routed to the status bar by SceneUiBinder.
    void showStatusMessageRequested(const QString &message);

    // --- IC hover-preview lifecycle (re-emitted from IC; see SceneUiBinder) ---
    void icPreviewRequested(IC *ic, const QPoint &screenPos);
    void icPreviewMoved(IC *ic, const QPoint &screenPos);
    void icPreviewHideRequested();
    void icPreviewCancelRequested(IC *ic);

    /// Emitted when a TruthTable element is double-clicked to request opening its editor.
    void openTruthTableRequested();

protected:
    // --- Qt event overrides ---

    /// \reimp
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    /// \reimp
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
    /// \reimp
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;
    /// \reimp
    void keyPressEvent(QKeyEvent *event) override;
    /// \reimp
    void keyReleaseEvent(QKeyEvent *event) override;
    /// \reimp
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    /// \reimp
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    /// \reimp
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    /// \reimp
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    /// \reimp
    void helpEvent(QGraphicsSceneHelpEvent *event) override;

private:
    // --- Helpers ---

    /// If \a event is a plain/Shift arrow key and something is selected, moves the selection by
    /// one grid step (Shift = a larger step) as a single undoable command and returns true.
    bool nudgeSelection(QKeyEvent *event);
    QList<QGraphicsItem *> itemsAt(const QPointF pos) const;
    const QVector<Connection *> connections() const;
    void checkUpdateRequest();
    void updateUndoText(const QString &text);
    void updateRedoText(const QString &text);
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void rotate(const int angle);
    void setDots(const QPen &dots);

    /// Registers \a item's current ID in the scene registry. Called by addItem().
    void registerItem(ItemWithId *item);
    /// Removes \a item's ID from the scene registry. Called by removeItem().
    void unregisterItem(ItemWithId *item);

    // --- Members ---

    // View
    GraphicsView *m_view = nullptr;

    // Undo/redo
    QUndoStack m_undoStack;
    QAction *m_redoAction;
    QAction *m_undoAction;

    // Simulation
    Simulation m_simulation;

    // Per-scene element registry (delegated to SceneItemRegistry)
    SceneItemRegistry m_itemRegistry;

    // Rendering
    QPen m_dots;

    // Mouse-move re-entrancy guard. Stays on Scene (not SceneInteraction) because it
    // must wrap the base QGraphicsScene::mouseMoveEvent call, where the re-entrancy
    // (ensureVisible → replayLastMouseEvent → mouseMoveEvent) actually originates.
    bool m_handlingMouseMove = false;

    // Context directory (directory of the .panda file owning this scene)
    QString m_contextDir;

    // IC definition registry (caches definitions, manages file watching)
    ICRegistry m_icRegistry = ICRegistry(this);

    // Autosave
    bool m_autosaveRequired = false;

    // Connection editing (delegated to ConnectionManager)
    ConnectionManager m_connectionManager = ConnectionManager(this);

    // Clipboard operations (delegated to ClipboardManager)
    ClipboardManager m_clipboardManager = ClipboardManager(this);

    // Property shortcut dispatch (delegated to PropertyShortcutHandler)
    PropertyShortcutHandler m_propertyShortcutHandler = PropertyShortcutHandler(this);

    // Visibility control (delegated to VisibilityManager)
    VisibilityManager m_visibilityManager = VisibilityManager(this);

    // Set by setPropertyUpdateRequired() whenever a structural edit may have left some
    // item's visibility out of sync with the current show-wires/show-gates toggles;
    // reapplied lazily in drawBackground() (see its comment) rather than eagerly.
    bool m_visibilityDirty = false;

    // Drag-and-drop payload decoding (delegated to SceneDropHandler)
    SceneDropHandler m_dropHandler = SceneDropHandler(this);

    // On-canvas inline label rename/edit (delegated to InlineLabelEditor)
    InlineLabelEditor m_inlineLabelEditor = InlineLabelEditor(this);

    // Mouse-driven editing gestures + rubber-band selection (delegated to SceneInteraction)
    SceneInteraction m_interaction{this};
};
