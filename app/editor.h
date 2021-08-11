/*
 * Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef EDITOR_H
#define EDITOR_H

#include <QElapsedTimer>
#include <QObject>

#include "scene.h"

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
    void cut(const QList<QGraphicsItem *> &items, QDataStream &ds);
    void copy(const QList<QGraphicsItem *> &items, QDataStream &ds);
    void paste(QDataStream &ds);
    void selectAll();

    bool eventFilter(QObject *obj, QEvent *evt) override;
    void setElementEditor(ElementEditor *value);
    QUndoStack *getUndoStack() const;
    Scene *getScene() const;
    QGraphicsRectItem *getSceneRect() const;
    ICManager *getICManager() const;
    void setICManager(ICManager *icManager);
    void buildSelectionRect();
    void handleHoverPort();
    void releaseHoverPort();

    void setHoverPort(QNEPort *port);
    QNEPort *getHoverPort();

    void resizeScene();
    SimulationController *getSimulationController() const;
    void setSimulationController(SimulationController *simulationController);
    void contextMenu(QPoint screenPos);
    void updateVisibility();
    QPointF getMousePos() const;
    ElementEditor *getElementEditor() const;
    void selectWorkspace(WorkSpace *workspace);

    void deleteEditedConn();
    void flipH();
    void flipV();

    void setCircuitUpdateRequired();
    void setAutoSaveRequired();
    void clearSelection();
    void setHandlingEvents(bool value);
signals:
    void scroll(int x, int y);
    void circuitHasChanged();
    void circuitAppearenceHasChanged();

public slots:
    void buildAndSetRectangle();
    void showWires(bool checked);
    void showGates(bool checked);
    void rotate(bool rotateRight);
    void checkUpdateRequest(int index);
    /**
     * @brief setupWorkspace: Creates workspace elements: IC manager, undo stack, scene, and simulation controller.
     * @param connect_signals: true: connects IC manager and simulation controller signals/slots if this is the first tab (opening the program).
     */
    void setupWorkspace();

    void receiveCommand(QUndoCommand *cmd);
    void copyAction();
    void cutAction();
    void pasteAction();
    void deleteAction();
    void updateTheme();
    void openConfigAction( );

    void mute(bool _mute = true);

private:
    QElapsedTimer m_timer;

    QUndoStack *m_undoStack;
    Scene *m_scene;
    int m_editedConn_id;
    int m_hoverPortElm_id;
    int m_hoverPort_nbr;
    ElementEditor *m_elementEditor;
    ICManager *m_icManager;

    bool m_markingSelectionBox;
    QGraphicsRectItem *m_selectionRect;
    QPointF m_selectionStartPoint;
    SimulationController *m_simulationController;
    QPointF m_mousePos, m_lastPos;
    bool m_draggingElement;
    QList<GraphicElement *> m_movedElements;
    QList<QPointF> m_oldPositions;
    MainWindow *m_mainWindow;
    /*  bool mControlKeyPressed; */
    bool m_showWires;
    bool m_showGates;
    bool m_circuitUpdateRequired;
    bool m_autoSaveRequired;
    bool m_handlingEvents;

    QList<QGraphicsItem *> itemsAt(QPointF pos);
    QGraphicsItem *itemAt(QPointF pos);
    void addItem(QGraphicsItem *item);
    bool mousePressEvt(QGraphicsSceneMouseEvent *mouseEvt);
    bool mouseMoveEvt(QGraphicsSceneMouseEvent *mouseEvt);
    bool mouseReleaseEvt(QGraphicsSceneMouseEvent *mouseEvt);
    bool dropEvt(QGraphicsSceneDragDropEvent *dde);
    bool dragMoveEvt(QGraphicsSceneDragDropEvent *dde);
    bool wheelEvt(QWheelEvent *wEvt);
    void ctrlDrag(QPointF pos);

    QNEConnection *getEditedConn() const;
    void setEditedConn(QNEConnection *editedConn);

    /* QObject interface */
    void detachConnection(QNEInputPort *endPort);
    void startNewConnection(QNEOutputPort *startPort);
    void startNewConnection(QNEInputPort *endPort);

    void startSelectionRect();

    void makeConnection(QNEConnection *editedConn);
    void redoSimulationController();
};

#endif /* EDITOR_H */
