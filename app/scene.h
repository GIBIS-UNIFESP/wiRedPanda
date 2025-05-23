// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "qneport.h"
#include "simulation.h"

#include <QElapsedTimer>
#include <QGraphicsScene>
#include <QMimeData>
#include <QUndoCommand>
#include <QVersionNumber>

class GraphicElement;
class GraphicsView;
class QNEConnection;

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

    explicit Scene(QObject *parent = nullptr);

    GraphicsView *view() const;
    QAction *redoAction() const;
    QAction *undoAction() const;
    QList<QGraphicsItem *> items(Qt::SortOrder order = Qt::SortOrder(-1)) const;
    QList<QGraphicsItem *> items(const QPointF &pos, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape, Qt::SortOrder order = Qt::SortOrder(-1), const QTransform &deviceTransform = QTransform()) const;
    QList<QGraphicsItem *> items(const QRectF &rect, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape, Qt::SortOrder order = Qt::SortOrder(-1), const QTransform &deviceTransform = QTransform()) const;
    QUndoStack *undoStack();
    Simulation *simulation();
    bool eventFilter(QObject *watched, QEvent *event) override;
    const QList<GraphicElement *> selectedElements() const;
    const QVector<GraphicElement *> elements() const;
    const QVector<GraphicElement *> elements(const QRectF &rect) const;
    const QVector<GraphicElement *> visibleElements() const;
    void addItem(QMimeData *mimeData);
    void copyAction();
    void cutAction();
    void deleteAction();
    void flipHorizontally();
    void flipVertically();
    void mute(const bool mute = true);
    void nextElm();
    void nextMainPropShortcut();
    void nextSecndPropShortcut();
    void pasteAction();
    void prevElm();
    void prevMainPropShortcut();
    void prevSecndPropShortcut();
    void receiveCommand(QUndoCommand *cmd);
    void rotateLeft();
    void rotateRight();
    void selectAll();
    void setAutosaveRequired();
    void setCircuitUpdateRequired();
    void setView(GraphicsView *view);
    void showGates(const bool checked);
    void showWires(const bool checked);
    void updateTheme();

signals:
    void circuitHasChanged();
    void contextMenuPos(QPoint screenPos, QGraphicsItem *itemAtMouse);

protected:
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

    GraphicsView *m_view = nullptr;
    QAction *m_redoAction;
    QAction *m_undoAction;
    QElapsedTimer m_timer;
    QGraphicsRectItem m_selectionRect;
    QList<GraphicElement *> m_movedElements;
    QList<QPointF> m_oldPositions;
    QPen m_dots;
    QPointF m_mousePos;
    QPointF m_selectionStartPoint;
    QUndoStack m_undoStack;
    Simulation m_simulation;
    bool m_autosaveRequired = false;
    bool m_draggingElement = false;
    bool m_markingSelectionBox = false;
    bool m_showGates = true;
    bool m_showWires = true;
    int m_editedConnectionId = 0;
    int m_hoverPortElmId = 0;
    int m_hoverPortNumber = 0;
};
