// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Main circuit editing scene with undo/redo and user interaction.
 */

#pragma once

#include <QElapsedTimer>
#include <QGraphicsScene>
#include <QMimeData>
#include <QUndoCommand>
#include <QVersionNumber>

#include "App/Nodes/QNEPort.h"
#include "App/Simulation/Simulation.h"

class GraphicElement;
class GraphicsView;
class QNEConnection;
class QPainter;

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
class Scene : public QGraphicsScene
{
    Q_OBJECT

public:
    using QGraphicsScene::addItem;

    static constexpr int gridSize = 16; ///< Scene grid unit in pixels (elements snap to gridSize/2).

    // --- Lifecycle ---

    /// Constructs a Scene and initialises the undo stack and simulation.
    explicit Scene(QObject *parent = nullptr);

    // --- View / Display Management ---

    /// Tightens the scene rect to item bounds while preserving the viewport position.
    void resizeScene();

    /// Returns the GraphicsView currently displaying this scene.
    GraphicsView *view() const;
    /// Sets the GraphicsView that displays this scene to \a view.
    void setView(GraphicsView *view);

    /**
     * \brief Shows or hides gate elements.
     * \param checked \c true to show gates.
     */
    void showGates(const bool checked);

    /**
     * \brief Shows or hides connection wires.
     * \param checked \c true to show wires.
     */
    void showWires(const bool checked);

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
    /// Returns \a elements sorted in topological dependency order (inputs first).
    static QVector<GraphicElement *> sortByTopology(QVector<GraphicElement *> elements);
    /// Returns all visible (non-hidden) graphic elements in the scene.
    const QVector<GraphicElement *> visibleElements() const;

    // --- Adding Items ---

    /// Deserializes and adds items from \a mimeData to the scene.
    void addItem(QMimeData *mimeData);

    // --- Clipboard (Copy / Cut / Paste) ---

    /// Copies the selected items to the internal clipboard.
    void copyAction();
    /// Cuts the selected items to the internal clipboard.
    void cutAction();
    /// Pastes items from the internal clipboard into the scene.
    void pasteAction();

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

    // --- Autosave ---

    /// Schedules an autosave of the current circuit state.
    void setAutosaveRequired();

    // --- Theme ---

    /// Propagates the current theme to all elements and connections.
    void updateTheme();

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

private:
    // --- Helpers ---

    static void copy(const QList<QGraphicsItem *> &items, QDataStream &stream);

    QGraphicsItem *itemAt(const QPointF pos);
    QList<QGraphicsItem *> itemsAt(const QPointF pos);
    QNEConnection *editedConnection() const;
    QNEPort *hoverPort();
    const QVector<QNEConnection *> connections();
    void checkUpdateRequest();
    void cloneDrag(const QPointF mousePos);
    void contextMenu(const QPoint screenPos);
    void cut(const QList<QGraphicsItem *> &items, QDataStream &stream);
    void deleteEditedConnection();
    void detachConnection(QNEInputPort *endPort);
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void handleHoverPort();
    void makeConnection(QNEConnection *connection);
    void paste(QDataStream &stream, const QVersionNumber &version);
    void releaseHoverPort();
    void rotate(const int angle);
    void setDots(const QPen &dots);
    void setEditedConnection(QNEConnection *connection);
    void setHoverPort(QNEPort *port);
    void startNewConnection(QNEInputPort *endPort);
    void startNewConnection(QNEOutputPort *startPort);
    void startSelectionRect();

    // --- Members ---

    // View
    GraphicsView *m_view = nullptr;

    // Undo/redo
    QUndoStack m_undoStack;
    QAction *m_redoAction;
    QAction *m_undoAction;

    // Simulation
    Simulation m_simulation;

    // Rendering
    QPen m_dots;
    QGraphicsRectItem m_selectionRect;

    // Mouse / interaction state
    QElapsedTimer m_timer;
    QPointF m_mousePos;
    QPointF m_selectionStartPoint;
    QList<GraphicElement *> m_movedElements;
    QList<QPointF> m_oldPositions;
    bool m_draggingElement = false;
    bool m_markingSelectionBox = false;

    // Visibility flags
    bool m_showGates = true;
    bool m_showWires = true;

    // Autosave
    bool m_autosaveRequired = false;

    // Connection editing
    int m_editedConnectionId = 0;

    // Hover port tracking
    int m_hoverPortElmId = 0;
    int m_hoverPortNumber = 0;
};

