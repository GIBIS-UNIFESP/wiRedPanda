// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

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

/*!
 * @class Scene
 * @brief Main circuit editing scene
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

    // --- Lifecycle ---

    explicit Scene(QObject *parent = nullptr);

    // --- View / Display Management ---

    GraphicsView *view() const;
    void setView(GraphicsView *view);
    void showGates(const bool checked);
    void showWires(const bool checked);

    // --- Element Access / Queries ---

    QList<QGraphicsItem *> items(Qt::SortOrder order = Qt::AscendingOrder) const;
    QList<QGraphicsItem *> items(const QPointF &pos, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape, Qt::SortOrder order = Qt::AscendingOrder, const QTransform &deviceTransform = QTransform()) const;
    QList<QGraphicsItem *> items(const QRectF &rect, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape, Qt::SortOrder order = Qt::AscendingOrder, const QTransform &deviceTransform = QTransform()) const;
    const QList<GraphicElement *> selectedElements() const;
    const QVector<GraphicElement *> elements() const;
    const QVector<GraphicElement *> elements(const QRectF &rect) const;
    const QVector<GraphicElement *> visibleElements() const;

    // --- Adding Items ---

    void addItem(QMimeData *mimeData);

    // --- Clipboard (Copy / Cut / Paste) ---

    void copyAction();
    void cutAction();
    void pasteAction();

    // --- Element Operations (Rotate / Flip / Delete / Mute) ---

    void deleteAction();
    void rotateRight();
    void rotateLeft();
    void flipHorizontally();
    void flipVertically();
    void mute(const bool mute = true);
    void selectAll();

    // --- Element Property Cycling ---

    void nextElm();
    void prevElm();
    void nextMainPropShortcut();
    void prevMainPropShortcut();
    void nextSecndPropShortcut();
    void prevSecndPropShortcut();

    // --- Undo / Redo ---

    QUndoStack *undoStack();
    QAction *redoAction() const;
    QAction *undoAction() const;
    void receiveCommand(QUndoCommand *cmd);

    // --- Simulation ---

    Simulation *simulation();
    void setCircuitUpdateRequired();

    // --- Autosave ---

    void setAutosaveRequired();

    // --- Theme ---

    void updateTheme();

    // --- Event Filter ---

    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    // --- Signals ---

    void circuitHasChanged();
    void contextMenuPos(QPoint screenPos, QGraphicsItem *itemAtMouse);

protected:
    // --- Qt event overrides ---

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
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
    void paste(QDataStream &stream, QVersionNumber version);
    void releaseHoverPort();
    void resizeScene();
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
