/*
 * Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include "scene.h"

#include <QElapsedTimer>
#include <QObject>

class ElementEditor;
class IC;
class ICManager;
class MainWindow;
class QNEConnection;
class QNEInputPort;
class QNEOutputPort;
class QNEPort;
class QUndoCommand;
class QUndoStack;
class QWheelEvent;
class SimulationController;
class WorkSpace;

class Editor : public QObject
{
    Q_OBJECT

public:
    explicit Editor(QObject *parent = nullptr);
    ~Editor() override;

    /**
     * @brief save: saves the project through a binary data stream.
     */
    void save(QDataStream &ds, const QString &dolphinFilename);

    /**
     * @brief load: loads the project through a binary data stream.
     */
    void load(QDataStream &ds);

    /**
     * @brief setupWorkspace: Creates workspace elements: IC manager, undo stack, scene, and simulation controller.
     * @param connect_signals: true: connects IC manager and simulation controller signals/slots if this is the first tab (opening the program).
     */
    void setupWorkspace();

    ElementEditor *getElementEditor() const;
    ICManager *getICManager() const;
    QGraphicsRectItem *getSceneRect() const;
    QNEPort *getHoverPort();
    QPointF getMousePos() const;
    QUndoStack *getUndoStack() const;
    Scene *getScene() const;
    SimulationController *getSimulationController() const;
    bool eventFilter(QObject *obj, QEvent *evt) override;
    void buildSelectionRect();
    void checkUpdateRequest();
    void clearSelection();
    void contextMenu(QPoint screenPos);
    void copy(const QList<QGraphicsItem *> &items, QDataStream &ds);
    void copyAction();
    void cut(const QList<QGraphicsItem *> &items, QDataStream &ds);
    void cutAction();
    void deleteAction();
    void deleteEditedConn();
    void flipH();
    void flipV();
    void handleHoverPort();
    void mute(bool _mute = true);
    void paste(QDataStream &ds);
    void pasteAction();
    void receiveCommand(QUndoCommand *cmd);
    void releaseHoverPort();
    void resizeScene();
    void rotate(bool rotateRight);
    void selectAll();
    void selectWorkspace(WorkSpace *workspace);
    void setAutoSaveRequired();
    void setCircuitUpdateRequired();
    void setElementEditor(ElementEditor *value);
    void setHandlingEvents(bool value);
    void setHoverPort(QNEPort *port);
    void setICManager(ICManager *icManager);
    void setSimulationController(SimulationController *simulationController);
    void showGates(bool checked);
    void showWires(bool checked);
    void updateTheme();
    void updateVisibility();

signals:
    void circuitAppearenceHasChanged();
    void circuitHasChanged();
    void scroll(int x, int y);

private:
    QGraphicsItem *itemAt(QPointF pos);
    QList<QGraphicsItem *> itemsAt(QPointF pos);
    QNEConnection *getEditedConn() const;
    bool dragMoveEvt(QGraphicsSceneDragDropEvent *dde);
    bool dropEvt(QGraphicsSceneDragDropEvent *dde);
    bool mouseMoveEvt(QGraphicsSceneMouseEvent *mouseEvt);
    bool mousePressEvt(QGraphicsSceneMouseEvent *mouseEvt);
    bool mouseReleaseEvt(QGraphicsSceneMouseEvent *mouseEvt);
    bool wheelEvt(QWheelEvent *wEvt);
    void addItem(QGraphicsItem *item);
    void buildAndSetRectangle();
    void ctrlDrag(QPointF pos);
    void detachConnection(QNEInputPort *endPort);
    void makeConnection(QNEConnection *editedConn);
    void redoSimulationController();
    void setEditedConn(QNEConnection *editedConn);
    void startNewConnection(QNEInputPort *endPort);
    void startNewConnection(QNEOutputPort *startPort);
    void startSelectionRect();

    ElementEditor *m_elementEditor;
    ICManager *m_icManager;
    MainWindow *m_mainWindow;
    QElapsedTimer m_timer;
    QGraphicsRectItem *m_selectionRect;
    QList<GraphicElement *> m_movedElements;
    QList<QPointF> m_oldPositions;
    QPointF m_mousePos, m_lastPos;
    QPointF m_selectionStartPoint;
    QUndoStack *m_undoStack;
    Scene *m_scene;
    SimulationController *m_simulationController;
    bool m_autoSaveRequired;
    bool m_circuitUpdateRequired;
    bool m_draggingElement;
    bool m_handlingEvents;
    bool m_markingSelectionBox;
    bool m_showGates;
    bool m_showWires;
    int m_editedConn_id;
    int m_hoverPortElm_id;
    int m_hoverPort_nbr;
};

