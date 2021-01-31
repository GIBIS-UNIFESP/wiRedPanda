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

class Editor : public QObject
{
    Q_OBJECT
    QElapsedTimer timer;

public:
    explicit Editor(QObject *parent = nullptr);
    ~Editor() override;
    bool saveLocalIC(IC *ic, const QString& newICPath);
    bool saveLocal(const QString& newPath);
    void save(QDataStream &ds, const QString &dolphinFilename);
    void load(QDataStream &ds);
    void cut(const QList<QGraphicsItem *> &items, QDataStream &ds);
    void copy(const QList<QGraphicsItem *> &items, QDataStream &ds);
    void paste(QDataStream &ds);
    void selectAll();

signals:
    void scroll(int x, int y);
    void circuitHasChanged();

public slots:
    void clear();
    void showWires(bool checked);
    void showGates(bool checked);
    void rotate(bool rotateRight);

    void receiveCommand(QUndoCommand *cmd);
    void copyAction();
    void cutAction();
    void pasteAction();
    void deleteAction();
    void updateTheme();

    void mute(bool _mute = true);

private:
    QUndoStack *m_undoStack;
    Scene *m_scene;
    QList<QGraphicsItem *> itemsAt(QPointF pos);
    QGraphicsItem *itemAt(QPointF pos);
    int m_editedConn_id;
    int m_hoverPortElm_id;
    int m_hoverPort_nbr;
    ElementEditor *m_elementEditor;
    ICManager *m_icManager;

    bool m_markingSelectionIC;
    QGraphicsRectItem *m_selectionRect;
    QPointF m_selectionStartPoint;
    SimulationController *m_simulationController;
    QPointF m_mousePos, m_lastPos;
    void addItem(QGraphicsItem *item);
    bool m_draggingElement;
    QList<GraphicElement *> m_movedElements;
    QList<QPointF> m_oldPositions;
    MainWindow *m_mainWindow;
    /*  bool mControlKeyPressed; */
    bool m_showWires;
    bool m_showGates;

    bool mousePressEvt(QGraphicsSceneMouseEvent *mouseEvt);
    bool mouseMoveEvt(QGraphicsSceneMouseEvent *mouseEvt);
    bool mouseReleaseEvt(QGraphicsSceneMouseEvent *mouseEvt);
    bool dropEvt(QGraphicsSceneDragDropEvent *dde);
    bool dragMoveEvt(QGraphicsSceneDragDropEvent *dde);
    bool wheelEvt(QWheelEvent *wEvt);

    void ctrlDrag(QPointF pos);
    void install(Scene *s);

    QNEConnection *getEditedConn() const;
    void setEditedConn(QNEConnection *editedConn);

    /* QObject interface */
    void detachConnection(QNEInputPort *endPort);
    void startNewConnection(QNEOutputPort *startPort);
    void startNewConnection(QNEInputPort *endPort);

    void startSelectionRect();

    void makeConnection(QNEConnection *editedConn);
    void redoSimulationController();

public:
    bool eventFilter(QObject *obj, QEvent *evt) override;
    void setElementEditor(ElementEditor *value);
    QUndoStack *getUndoStack() const;
    Scene *getScene() const;
    void buildSelectionRect();
    void handleHoverPort();
    void releaseHoverPort();

    void setHoverPort(QNEPort *port);
    QNEPort *getHoverPort();

    void resizeScene();
    SimulationController *getSimulationController() const;
    void contextMenu(QPoint screenPos);
    void updateVisibility();
    QPointF getMousePos() const;

    ElementEditor *getElementEditor() const;

    static Editor *globalEditor;

    void deleteEditedConn();
    void flipH();
    void flipV();
};

#endif /* EDITOR_H */
