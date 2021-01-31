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
    /**
     * @brief saveLocalIC: saves an IC and its internal ICs recursively into the same local project subfolder.
     */
    bool saveLocalIC(IC *ic, const QString& newICPath);
    /**
     * @brief saveLocal: saves a project locally in a given directory so that all ICs and skins are also saved in local subfolders.
     * This is very useful to export projects that use ICs from other folders or projects.
     */
    bool saveLocal(const QString& newPath);
    /**
     * @brief save: saves the project through a binary data stream.
     */
    void save(QDataStream &ds, const QString &dolphinFilename);
    /**
     * @brief load: loads the project through a binary data stream.
     */
    void load(QDataStream &ds);
    //!
    //! \brief cut copies a selection of items to the given data stream and then issues a DeleteItemsCommand to remove those items from view
    //! \param items are the elements that will be removed from the editor
    //! \param ds is the data stream to where the items will be copied to
    //!
    void cut(const QList<QGraphicsItem *> &items, QDataStream &ds);
    //!
    //! \brief copy copies a list of items to a given data stream
    //!
    void copy(const QList<QGraphicsItem *> &items, QDataStream &ds);
    //!
    //! \brief paste moves a list of items from a data stream into the editor
    //!
    void paste(QDataStream &ds);
    //! Selects all elements on the editor. Issued when the user presses Ctrl + A.
    void selectAll();

signals:
    //! Signals a QWidget::scroll
    void scroll(int x, int y);
    //! Autosaves the progress at every new change in the circuit
    void circuitHasChanged();

public slots:
    //! Clears the entire editor, that is, clears the simulation controller, the IC manager, scene, undo stack and element factory.
    void clear();
    //!
    //! \brief showWires regulates the visibility of wires
    //! \param checked - If checked is true, then the wires are visible. If not, then the wires are invisible.
    //!
    void showWires(bool checked);
    //!
    //! \brief showGates regulates the visibility of gates
    //! \param checked - If checked is true, then the gates are visible. If not, then the gates are invisible.
    //!
    void showGates(bool checked);
    //!
    //! \brief rotate rotates the editor's selected items by a given amount of degrees, through a RotateCommand
    //! \param rotateRight is the amount of degrees that the selected elements are going to rotate, in clockwise motion.
    //!
    void rotate(bool rotateRight);

    void receiveCommand(QUndoCommand *cmd);
    //!
    //! \brief copyAction copies the editor's selected items to clipboard
    //!
    void copyAction();
    //!
    //! \brief cutAction copies the editor's selected items to clipboard and then remove them from view
    //!
    void cutAction();
    //!
    //! \brief pasteAction inserts items into view from the clipboard
    //!
    void pasteAction();
    //!
    //! \brief deleteAction deletes the selected items
    //!
    void deleteAction();
    void updateTheme();

    //!
    //! \brief mute mutes or unmutes all buzzers, depending from the flag passed onto the methods
    //!
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
