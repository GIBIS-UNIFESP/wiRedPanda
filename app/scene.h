// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "qneport.h"
#include "simulation.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QGraphicsScene>
#include <QMimeData>
#include <QMultiMap>
#include <QSet>
#include <QUndoCommand>
#include <QVersionNumber>

class GraphicElement;
class GraphicsView;
class QNEConnection;

struct Destination {
    int connectionId;
    int nodeId;

    bool operator==(const Destination &other) const {
        return connectionId == other.connectionId && nodeId == other.nodeId;
    }
};

inline uint qHash(const Destination &d, uint seed = 0) {
    return qHash(QPair<int, int>(d.connectionId, d.nodeId), seed);
}

inline QDataStream &operator<<(QDataStream &out, const Destination &d) {
    out << d.connectionId << d.nodeId;
    return out;
}

inline QDataStream &operator>>(QDataStream &in, Destination &d) {
    in >> d.connectionId >> d.nodeId;
    return in;
}

inline QDebug operator<<(QDebug debug, const Destination &d) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "Destination(connectionId=" << d.connectionId << ", nodeId=" << d.nodeId << ")";
    return debug;
}

class Scene : public QGraphicsScene
{
    Q_OBJECT

public:
    using QGraphicsScene::addItem;

    explicit Scene(QObject *parent = nullptr);

    GraphicElement *element(const int id) const;
    GraphicsView *view() const;
    QAction *redoAction() const;
    QAction *undoAction() const;
    QList<QGraphicsItem *> items(Qt::SortOrder order = Qt::SortOrder(-1)) const;
    QList<QGraphicsItem *> items(const QPointF &pos, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape, Qt::SortOrder order = Qt::SortOrder(-1), const QTransform &deviceTransform = QTransform()) const;
    QList<QGraphicsItem *> items(const QRectF &rect, Qt::ItemSelectionMode mode = Qt::IntersectsItemShape, Qt::SortOrder order = Qt::SortOrder(-1), const QTransform &deviceTransform = QTransform()) const;
    QMap<int, QSet<Destination>> nodeMapping;
    QNEConnection *connection(const int connectionId) const;
    QSet<Destination> getNodeSet(const QString &nodeLabel, QList<int> excludeIds = {});
    QUndoStack *undoStack();
    Simulation *simulation();
    bool eventFilter(QObject *watched, QEvent *event) override;
    bool isSourceNode(GraphicElement *node);
    const QList<GraphicElement *> selectedElements() const;
    const QVector<GraphicElement *> elements() const;
    const QVector<GraphicElement *> elements(const QRectF &rect) const;
    const QVector<GraphicElement *> visibleElements() const;
    void addItem(QMimeData *mimeData);
    void copyAction();
    void cutAction();
    void deleteAction();
    void deleteEditedConnection();
    void deleteNodeAction(const QList<QGraphicsItem *> items);
    void deleteNodeSetConnections(QSet<Destination> *set, const int nodeToRemove = -1);
    void flipHorizontally();
    void flipVertically();
    void makeConnection(QNEConnection *connection);
    void makeConnectionNode(QNEConnection *connection);
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
    void setEditedConnection(QNEConnection *connection);
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
    void detachConnection(QNEInputPort *endPort);
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void handleHoverPort();
    void paste(QDataStream &stream, QVersionNumber version);
    void releaseHoverPort();
    void resizeScene();
    void rotate(const int angle);
    void setDots(const QPen &dots);
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
