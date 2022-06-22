// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scene.h"

#include "buzzer.h"
#include "commands.h"
#include "common.h"
#include "elementfactory.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "graphicelementinput.h"
#include "ic.h"
#include "qneconnection.h"
#include "serializationfunctions.h"
#include "thememanager.h"

#include <QClipboard>
#include <QDrag>
#include <QGraphicsSceneDragDropEvent>
#include <QKeyEvent>
#include <QMenu>

Scene::Scene(QObject *parent)
    : QGraphicsScene(parent)
    , m_simulationController(this)
{
    installEventFilter(this);

    m_selectionRect.setFlag(QGraphicsItem::ItemIsSelectable, false);
    addItem(&m_selectionRect);

    m_undoAction = undoStack()->createUndoAction(this, tr("&Undo"));
    m_undoAction->setIcon(QIcon(":/toolbar/undo.png"));
    m_undoAction->setShortcut(QKeySequence::Undo);

    m_redoAction = undoStack()->createRedoAction(this, tr("&Redo"));
    m_redoAction->setIcon(QIcon(":/toolbar/redo.png"));
    m_redoAction->setShortcut(QKeySequence::Redo);

    connect(&ICManager::instance(),    &ICManager::updatedIC,       &m_simulationController, &SimulationController::restart);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,                    &Scene::updateTheme);
    connect(&m_undoStack,              &QUndoStack::indexChanged,   this,                    &Scene::checkUpdateRequest);
    connect(this,                      &Scene::circuitHasChanged,   &m_simulationController, &SimulationController::initialize);
}

void Scene::checkUpdateRequest()
{
    if (m_circuitUpdateRequired) {
        emit circuitHasChanged();
        m_circuitUpdateRequired = false;
    } else if (m_autosaveRequired) {
        emit circuitAppearenceHasChanged();
        m_autosaveRequired = false;
    }
}

void Scene::drawBackground(QPainter *painter, const QRectF &rect)
{
    const int gridSize = GlobalProperties::gridSize;
    painter->setRenderHint(QPainter::Antialiasing, true);
    QGraphicsScene::drawBackground(painter, rect);
    painter->setPen(m_dots);
    const int left = static_cast<int>(rect.left()) - (static_cast<int>(rect.left()) % gridSize);
    const int top = static_cast<int>(rect.top()) - (static_cast<int>(rect.top()) % gridSize);
    QVector<QPoint> points;

    for (int x = left; x < rect.right(); x += gridSize) {
        for (int y = top; y < rect.bottom(); y += gridSize) {
            points.append(QPoint(x, y));
        }
    }

    painter->drawPoints(points.data(), points.size());
}

void Scene::setDots(const QPen &dots)
{
    m_dots = dots;
}

SimulationController *Scene::simulationController()
{
    return &m_simulationController;
}

void Scene::setAutosaveRequired()
{
    m_autosaveRequired = true;
}

void Scene::setCircuitUpdateRequired()
{
    m_circuitUpdateRequired = true;
}

QVector<GraphicElement *> Scene::visibleElements()
{
    auto visibleRect = m_view->mapToScene(m_view->viewport()->geometry()).boundingRect();

    return elements(visibleRect);
}

QVector<GraphicElement *> Scene::elements()
{
    const auto items_ = items();
    QVector<GraphicElement *> elements_;
    elements_.reserve(items_.size());

    for (auto *item : items_) {
        if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item)) {
            elements_.append(elm);
        }
    }

    return elements_;
}

QVector<GraphicElement *> Scene::elements(const QRectF &rect)
{
    QVector<GraphicElement *> elements;
    const auto items_ = items(rect);
    elements.reserve(items_.size());
    for (auto *item : items_) {
        if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item)) {
            elements.append(elm);
        }
    }
    return elements;
}

QVector<QNEConnection *> Scene::connections()
{
    QVector<QNEConnection *> conns;
    const auto items_ = items();
    conns.reserve(items_.size());
    for (auto *item : items_) {
        if (auto *conn = qgraphicsitem_cast<QNEConnection *>(item)) {
            conns.append(conn);
        }
    }
    return conns;
}

QList<GraphicElement *> Scene::selectedElements()
{
    QList<GraphicElement *> elements;
    const auto items = selectedItems();
    for (auto *item : items) {
        if (item->type() == GraphicElement::Type) {
            if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item)) {
                elements.append(elm);
            }
        }
    }
    return elements;
}

QGraphicsItem *Scene::itemAt(const QPointF pos)
{
    auto items_ = items(pos);
    items_.append(itemsAt(pos));
    for (auto *item : qAsConst(items_)) {
        if (item->type() == QNEPort::Type) {
            return item;
        }
    }
    for (auto *item : qAsConst(items_)) {
        if (item->type() > QGraphicsItem::UserType) {
            return item;
        }
    }
    return nullptr;
}

QList<QGraphicsItem *> Scene::itemsAt(const QPointF pos)
{
    QRectF rect(pos - QPointF(4, 4), QSize(9, 9));
    return items(rect.normalized());
}

void Scene::receiveCommand(QUndoCommand *cmd)
{
    m_undoStack.push(cmd);
}

void Scene::resizeScene()
{
    const auto elms = elements();

    if (!elms.isEmpty()) {
        QRectF rect = sceneRect();
        for (auto *elm : elms) {
            QRectF itemRect = elm->boundingRect().translated(elm->pos());
            rect = rect.united(itemRect.adjusted(-10, -10, 10, 10));
        }
        setSceneRect(rect);
    }

    auto *item = itemAt(m_mousePos);

    if (item && (m_timer.elapsed() > 100) && m_draggingElement) {
        m_view->ensureVisible(QRectF(m_mousePos - QPointF(4, 4), QSize(9, 9)).normalized());
        m_timer.restart();
    }
}

QNEConnection *Scene::editedConnection() const
{
    return dynamic_cast<QNEConnection *>(ElementFactory::itemById(m_editedConnectionId));
}

void Scene::deleteEditedConnection()
{
    if (auto *connection = editedConnection()) {
        removeItem(connection);
        delete connection;
    }
    setEditedConnection(nullptr);
}

void Scene::setEditedConnection(QNEConnection *connection)
{
    if (connection) {
        connection->setFocus();
        m_editedConnectionId = connection->id();
    } else {
        m_editedConnectionId = 0;
    }
}

void Scene::startNewConnection(QNEInputPort *endPort)
{
    QNEConnection *connection = ElementFactory::buildConnection();
    connection->setEnd(endPort);
    connection->setStartPos(m_mousePos);
    addItem(connection);
    setEditedConnection(connection);
    connection->updatePath();
}

void Scene::startNewConnection(QNEOutputPort *startPort)
{
    QNEConnection *connection = ElementFactory::buildConnection();
    connection->setStart(startPort);
    connection->setEndPos(m_mousePos);

    addItem(connection);
    setEditedConnection(connection);
    connection->updatePath();
}

QUndoStack *Scene::undoStack()
{
    return &m_undoStack;
}

void Scene::makeConnection(QNEConnection *connection)
{
    auto *port = dynamic_cast<QNEPort *>(itemAt(m_mousePos));

    if (!port || !connection) {
        return;
    }

    /* The mouse is released over a QNEPort. */
    QNEOutputPort *startPort = nullptr;
    QNEInputPort *endPort = nullptr;

    if (connection->start() != nullptr) {
        startPort = connection->start();
        endPort = dynamic_cast<QNEInputPort *>(port);
    } else if (connection->end() != nullptr) {
        startPort = dynamic_cast<QNEOutputPort *>(port);
        endPort = connection->end();
    }

    if (!startPort || !endPort) {
        return;
    }

    /* Verifying if the connection is valid. */
    if ((startPort->graphicElement() != endPort->graphicElement()) && !startPort->isConnected(endPort)) {
        /* Making connection. */
        connection->setStart(startPort);
        connection->setEnd(endPort);
        receiveCommand(new AddItemsCommand({connection}, this));
        setEditedConnection(nullptr);
    } else {
        deleteEditedConnection();
    }
}

void Scene::detachConnection(QNEInputPort *endPort)
{
    auto *connection = endPort->connections().last();

    if (auto *startPort = connection->start()) {
        receiveCommand(new DeleteItemsCommand({connection}, this));
        startNewConnection(startPort);
    }
}

void Scene::updateTheme()
{
    qCDebug(zero) << tr("Updating theme.");
    const ThemeAttributes theme = ThemeManager::attributes();
    setBackgroundBrush(theme.m_sceneBgBrush);
    setDots(QPen(theme.m_sceneBgDots));
    m_selectionRect.setBrush(QBrush(theme.m_selectionBrush));
    m_selectionRect.setPen(QPen(theme.m_selectionPen, 1, Qt::SolidLine));
    const auto scene_elements = elements();
    for (auto *elm : scene_elements) {
        elm->updateTheme();
    }
    const auto scene_connections = connections();
    for (auto *conn : scene_connections) {
        conn->updateTheme();
    }
    qCDebug(zero) << tr("Finished updating theme.");
}

void Scene::showGates(const bool checked)
{
    m_showGates = checked;
    const auto sceneItems = items();
    for (auto *item : sceneItems) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if ((item->type() == GraphicElement::Type) && elm) {
            const auto group = elm->elementGroup();
            if ((group != ElementGroup::Input) && (group != ElementGroup::Output) && (group != ElementGroup::Other)) {
                item->setVisible(checked);
            }
        }
    }
}

void Scene::showWires(const bool checked)
{
    m_showWires = checked;
    const auto sceneItems = items();
    for (auto *item : sceneItems) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (item->type() == QNEConnection::Type) {
            item->setVisible(checked);
        } else if ((item->type() == GraphicElement::Type) && elm) {
            if (elm->elementType() == ElementType::Node) {
                elm->setVisible(checked);
            } else {
                const auto elmInputs = elm->inputs();
                for (auto *in : elmInputs) {
                    in->setVisible(checked);
                }
                const auto elmOutputs = elm->outputs();
                for (auto *out : elmOutputs) {
                    out->setVisible(checked);
                }
            }
        }
    }
}

void Scene::cloneDrag(const QPointF mousePos)
{
    qCDebug(zero) << tr("Ctrl + Drag action triggered.");
    const auto selectedElms = selectedElements();

    if (selectedElms.isEmpty()) {
        return;
    }

    const auto items_ = items();

    for (auto *item : items_) {
        if ((item->type() == GraphicElement::Type || item->type() == QNEConnection::Type) && !item->isSelected()) {
            item->hide();
        }
    }

    QRectF rect;

    for (auto *elm : selectedElms) {
        rect = rect.united(elm->sceneBoundingRect());
    }

    rect = rect.adjusted(-8, -8, 8, 8);

    auto mappedSize = m_view->transform().mapRect(rect).size().toSize();
    QImage image(mappedSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setOpacity(0.0);
    QRectF target = image.rect();
    QRectF source = rect;
    render(&painter, target, source);

    for (auto *item : items_) {
        if ((item->type() == GraphicElement::Type || item->type() == QNEConnection::Type) && !item->isSelected()) {
            item->show();
        }
    }

    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    stream << mousePos;
    copy(selectedItems(), stream);

    auto *mimeData = new QMimeData;
    mimeData->setData("application/ctrlDragData", itemData);

    auto *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(QPixmap::fromImage(image));
    QPointF offset = m_view->transform().map(mousePos - rect.topLeft());
    drag->setHotSpot(offset.toPoint());
    drag->exec(Qt::CopyAction, Qt::CopyAction);
}

void Scene::copy(const QList<QGraphicsItem *> &items, QDataStream &stream)
{
    QPointF center(static_cast<qreal>(0.0f), static_cast<qreal>(0.0f));
    float elm = 0;
    for (auto *item : items) {
        if (item->type() == GraphicElement::Type) {
            center += item->pos();
            elm++;
        }
    }
    stream << center / static_cast<qreal>(elm);
    SerializationFunctions::serialize(items, stream);
}

void Scene::handleHoverPort()
{
    auto *port = dynamic_cast<QNEPort *>(itemAt(m_mousePos));
    auto *hoverPort_ = hoverPort();
    if (hoverPort_ && (port != hoverPort_)) {
        releaseHoverPort();
    }
    if (port && (port->type() == QNEPort::Type)) {
        QNEConnection *editedConn = editedConnection();
        releaseHoverPort();
        setHoverPort(port);
        if (editedConn && editedConn->start() && (editedConn->start()->isOutput() == port->isOutput())) {
            QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));
        }
    }
}

void Scene::releaseHoverPort()
{
    auto *hoverPort_ = hoverPort();
    if (hoverPort_) {
        hoverPort_->hoverLeave();
        setHoverPort(nullptr);
        QApplication::restoreOverrideCursor();
    }
}

void Scene::setHoverPort(QNEPort *port)
{
    if (port) {
        auto *hoverElm = port->graphicElement();
        port->hoverEnter();
        if (hoverElm && ElementFactory::contains(hoverElm->id())) {
            m_hoverPortElmId = hoverElm->id();
            for (int i = 0; i < (hoverElm->inputSize() + hoverElm->outputSize()); ++i) {
                if (i < hoverElm->inputSize()) {
                    if (port == hoverElm->inputPort(i)) {
                        m_hoverPortNbr = i;
                    }
                } else if (port == hoverElm->outputPort(i - hoverElm->inputSize())) {
                    m_hoverPortNbr = i;
                }
            }
        }
    } else {
        m_hoverPortElmId = 0;
        m_hoverPortNbr = 0;
    }
}

QNEPort *Scene::hoverPort()
{
    QNEPort *hoverPort = nullptr;

    if (auto *hoverElm = dynamic_cast<GraphicElement *>(ElementFactory::itemById(m_hoverPortElmId))) {
        if (m_hoverPortNbr < hoverElm->inputSize()) {
            hoverPort = hoverElm->inputPort(m_hoverPortNbr);
        } else if (((m_hoverPortNbr - hoverElm->inputSize()) < hoverElm->outputSize())) {
            hoverPort = hoverElm->outputPort(m_hoverPortNbr - hoverElm->inputSize());
        }
    }

    if (!hoverPort) {
        setHoverPort(nullptr);
    }

    return hoverPort;
}

void Scene::startSelectionRect()
{
    m_selectionStartPoint = m_mousePos;
    m_markingSelectionBox = true;
    m_selectionRect.setRect(QRectF(m_selectionStartPoint, m_selectionStartPoint));
    m_selectionRect.show();
    m_selectionRect.update();
}

GraphicsView *Scene::view() const
{
    return m_view;
}

void Scene::setView(GraphicsView *view)
{
    m_view = view;
}

QAction *Scene::undoAction() const
{
    return m_undoAction;
}

QAction *Scene::redoAction() const
{
    return m_redoAction;
}

void Scene::contextMenu(const QPoint screenPos)
{
    if (auto *item = itemAt(m_mousePos)) {
        if (selectedItems().contains(item)) {
            emit contextMenuPos(screenPos, item);
        } else if ((item->type() == GraphicElement::Type)) {
            clearSelection();
            item->setSelected(true);
            emit contextMenuPos(screenPos, item);
        }
    } else {
        QMenu menu;
        auto *pasteAction = menu.addAction(QIcon(QPixmap(":/toolbar/paste.png")), tr("Paste"));
        const auto *clipboard = QApplication::clipboard();
        const auto *mimeData = clipboard->mimeData();
        if (mimeData->hasFormat("wpanda/copydata")) {
            connect(pasteAction, &QAction::triggered, this, &Scene::pasteAction);
        } else {
            pasteAction->setEnabled(false);
        }
        menu.exec(screenPos);
    }
}

void Scene::copyAction()
{
    auto elms = selectedElements();
    if (elms.empty()) {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->clear();
    } else {
        QClipboard *clipboard = QApplication::clipboard();
        auto *mimeData = new QMimeData;
        QByteArray itemData;
        QDataStream stream(&itemData, QIODevice::WriteOnly);
        copy(selectedItems(), stream);
        mimeData->setData("wpanda/copydata", itemData);
        clipboard->setMimeData(mimeData);
    }
}

void Scene::cutAction()
{
    auto *clipboard = QApplication::clipboard();
    auto *mimeData = new QMimeData();
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    cut(selectedItems(), stream);
    mimeData->setData("wpanda/copydata", itemData);
    clipboard->setMimeData(mimeData);
}

void Scene::pasteAction()
{
    const auto *clipboard = QApplication::clipboard();
    const auto *mimeData = clipboard->mimeData();
    if (mimeData->hasFormat("wpanda/copydata")) {
        QByteArray itemData = mimeData->data("wpanda/copydata");
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        paste(stream);
    }
}

void Scene::paste(QDataStream &stream)
{
    clearSelection();
    QPointF ctr;
    stream >> ctr;
    QPointF offset = m_mousePos - ctr - QPointF(static_cast<qreal>(32.0f), static_cast<qreal>(32.0f));
    double version = GlobalProperties::version;
    const auto itemList = SerializationFunctions::deserialize(stream, version);
    receiveCommand(new AddItemsCommand(itemList, this));
    for (auto *item : itemList) {
        if (item->type() == GraphicElement::Type) {
            item->setPos((item->pos() + offset));
            item->update();
            item->setSelected(true);
        }
    }
    resizeScene();
}

void Scene::cut(const QList<QGraphicsItem *> &items, QDataStream &stream)
{
    copy(items, stream);
    deleteAction();
}

void Scene::deleteAction()
{
    const auto &items = selectedItems();
    clearSelection();

    if (!items.isEmpty()) {
        receiveCommand(new DeleteItemsCommand(items, this));
        m_simulationController.restart();
    }
}

void Scene::selectAll()
{
    const auto sceneItems = items();

    for (auto *item : sceneItems) {
        item->setSelected(true);
    }
}

void Scene::rotateRight()
{
    rotate(90);
}

void Scene::rotateLeft()
{
    rotate(-90);
}

void Scene::rotate(const int angle) {
    const auto list = selectedItems();
    QList<GraphicElement *> elms;
    elms.reserve(list.size());

    for (auto *item : list) {
        if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item)) {
            elms.append(elm);
        }
    }

    if (!elms.isEmpty()) {
        receiveCommand(new RotateCommand(elms, angle, this));
    }
}

void Scene::mute(const bool mute)
{
    const auto elms = elements();

    for (auto *elm : elms) {
        if (auto *buzzer = dynamic_cast<Buzzer *>(elm)) {
            buzzer->mute(mute);
        }
    }
}

void Scene::flipHorizontally()
{
    const auto list = selectedItems();
    QList<GraphicElement *> elms;
    elms.reserve(list.size());

    for (auto *item : list) {
        if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item)) {
            elms.append(elm);
        }
    }

    if ((elms.size() > 1) || ((elms.size() == 1))) {
        receiveCommand(new FlipCommand(elms, 0, this));
    }
}

void Scene::flipVertically()
{
    const auto list = selectedItems();
    QList<GraphicElement *> elms;
    elms.reserve(list.size());

    for (auto *item : list) {
        if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item)) {
            elms.append(elm);
        }
    }

    if ((elms.size() > 1) || ((elms.size() == 1))) {
        receiveCommand(new FlipCommand(elms, 1, this));
    }
}

void Scene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    const auto formats = event->mimeData()->formats();

    if (formats.contains("application/x-dnditemdata") || formats.contains("application/ctrlDragData")) {
        event->accept();
        return;
    }

    QGraphicsScene::dragEnterEvent(event);
}

void Scene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    const auto formats = event->mimeData()->formats();

    if (formats.contains("application/x-dnditemdata") || formats.contains("application/ctrlDragData")) {
        event->accept();
        return;
    }

    QGraphicsScene::dragMoveEvent(event);
}

void Scene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
        QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        QPoint offset;
        ElementType type;
        QString labelAuxData;
        stream >> offset >> type >> labelAuxData;
        QPointF pos = event->scenePos() - offset;
        qCDebug(zero) << type << tr("at position:") << pos.x() << tr(",") << pos.y() << tr(", label:") << labelAuxData;
        auto *elm = ElementFactory::buildElement(type);
        qCDebug(zero) << tr("Valid element.");
        if (elm->elementType() == ElementType::IC) {
            if (auto *ic = dynamic_cast<IC *>(elm)) {
                ICManager::loadIC(ic, labelAuxData);
            }
        }
        qCDebug(zero) << tr("Adding the element to the scene.");
        receiveCommand(new AddItemsCommand({elm}, this));
        qCDebug(zero) << tr("Cleaning the selection.");
        clearSelection();
        qCDebug(zero) << tr("Setting created element as selected.");
        elm->setSelected(true);
        qCDebug(zero) << tr("Adjusting the position of the element.");
        elm->setPos(pos);
    }

    if (event->mimeData()->hasFormat("application/ctrlDragData")) {
        QByteArray itemData = event->mimeData()->data("application/ctrlDragData");
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        QPointF offset;
        QPointF ctr;
        stream >> offset >> ctr;
        offset = event->scenePos() - offset;
        double version = GlobalProperties::version;
        const auto itemList = SerializationFunctions::deserialize(stream, version);
        receiveCommand(new AddItemsCommand(itemList, this));
        clearSelection();
        for (auto *item : itemList) {
            if (item->type() == GraphicElement::Type) {
                item->setPos((item->pos() + offset));
                item->setSelected(true);
                item->update();
            }
        }
        resizeScene();
    }

    QGraphicsScene::dropEvent(event);
}

void Scene::keyPressEvent(QKeyEvent *event)
{
    if (!(event->modifiers() & Qt::ControlModifier)) {
        const auto elms = elements();
        for (auto *elm : elms) {
            if (elm->hasTrigger() && !elm->trigger().isEmpty() && elm->trigger().matches(event->key())) {
                if (auto *input = dynamic_cast<GraphicElementInput *>(elm);
                        input && !input->isLocked()) {
                    input->setOn();
                }
            }
        }
    }

    QGraphicsScene::keyPressEvent(event);
}

void Scene::keyReleaseEvent(QKeyEvent *event)
{
    if (!(event->modifiers() & Qt::ControlModifier)) {
        const auto elms = elements();
        for (auto *elm : elms) {
            if (elm->hasTrigger() && !elm->trigger().isEmpty() && elm->trigger().matches(event->key())) {
                if (auto *input = dynamic_cast<GraphicElementInput *>(elm);
                        input && !input->isLocked() && (elm->elementType() == ElementType::InputButton)) {
                    input->setOff();
                }
            }
        }
    }

    QGraphicsScene::keyReleaseEvent(event);
}

bool Scene::eventFilter(QObject *watched, QEvent *event)
{
    if (auto *mouseEvent = dynamic_cast<QGraphicsSceneMouseEvent *>(event)) {
        if (mouseEvent->modifiers() & Qt::ShiftModifier) {
            mouseEvent->setModifiers(Qt::ControlModifier);
            return false;
        }

        auto *item = itemAt(mouseEvent->scenePos());

        if (item
            && (item->type() == GraphicElement::Type || item->type() == QNEConnection::Type)
            && (mouseEvent->button() == Qt::LeftButton)
            && (mouseEvent->modifiers() & Qt::ControlModifier)) {
            item->setSelected(true);
            cloneDrag(mouseEvent->scenePos());
            return true;
        }
    }

    return QGraphicsScene::eventFilter(watched, event);
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_mousePos = event->scenePos();
    resizeScene();
    handleHoverPort();

    auto *item = itemAt(m_mousePos);

    if (item) {
        if (event->button() == Qt::LeftButton) {
            m_draggingElement = true;
            auto list = selectedItems();
            list.append(itemsAt(m_mousePos));
            m_movedElements.clear();
            m_oldPositions.clear();

            for (auto *item : qAsConst(list)) {
                if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item)) {
                    m_movedElements.append(elm);
                    m_oldPositions.append(elm->pos());
                }
            }
        }

        if (item->type() == QNEPort::Type) {
            /* When the mouse is pressed over an connected input port, the line
             * is disconnected and can be connected to another port. */
            if (auto *connection = editedConnection()) {
                makeConnection(connection);
                return;
            }

            auto *pressedPort = qgraphicsitem_cast<QNEPort *>(item);

            if (auto *startPort = dynamic_cast<QNEOutputPort *>(pressedPort)) {
                startNewConnection(startPort);
                return;
            }

            if (auto *endPort = dynamic_cast<QNEInputPort *>(pressedPort)) {
                endPort->connections().isEmpty() ? startNewConnection(endPort) : detachConnection(endPort);
                return;
            }
        }
    }

    if (editedConnection()) {
        deleteEditedConnection();
    }

    if (!item && (event->button() == Qt::LeftButton)) {
        startSelectionRect();
    }

    if (event->button() == Qt::RightButton) {
        contextMenu(event->screenPos());
    }

    QGraphicsScene::mousePressEvent(event);
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    m_mousePos = event->scenePos();
    handleHoverPort();

    if (m_draggingElement) {
        resizeScene();
    }

    if (auto *connection = editedConnection()) {
        if (connection->start()) {
            connection->setEndPos(m_mousePos);
            connection->updatePath();
            return;
        }

        if (connection->end()) {
            connection->setStartPos(m_mousePos);
            connection->updatePath();
            return;
        }

        deleteEditedConnection();
        return;
    }

    if (m_markingSelectionBox) {
        QRectF rect = QRectF(m_selectionStartPoint, m_mousePos).normalized();
        m_selectionRect.setRect(rect);
        QPainterPath selectionBox;
        selectionBox.addRect(rect);
        setSelectionArea(selectionBox);
    } else {
        m_selectionRect.hide();
    }

    QGraphicsScene::mouseMoveEvent(event);
}

void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_draggingElement && (event->button() == Qt::LeftButton)) {
        if (!m_movedElements.empty()) {
            bool valid = false; // TODO: std::any_of?
            for (int elm = 0; elm < m_movedElements.size(); ++elm) {
                if (m_movedElements[elm]->pos() != m_oldPositions[elm]) {
                    valid = true;
                    break;
                }
            }
            if (valid) {
                receiveCommand(new MoveCommand(m_movedElements, m_oldPositions, this));
            }
        }
        m_draggingElement = false;
        m_movedElements.clear();
    }

    m_selectionRect.hide();
    m_markingSelectionBox = false;

    if (auto *connection = editedConnection();
            connection && (event->buttons() == Qt::NoButton)) {
        makeConnection(connection);
        return;
    }

    QGraphicsScene::mouseReleaseEvent(event);
}

void Scene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        return;
    }

    if (auto *connection = dynamic_cast<QNEConnection *>(itemAt(m_mousePos));
            connection && connection->start() && connection->end()) {
        receiveCommand(new SplitCommand(connection, m_mousePos, this));
    }

    QGraphicsScene::mouseDoubleClickEvent(event);
}

void Scene::addItem(QGraphicsItem *item)
{
    if (auto *elm = dynamic_cast<Buzzer *>(item)) {
        elm->setLabel(elm->objectName() + "_" + QString::number(++m_labelNumber));
    }

    QGraphicsScene::addItem(item);
}

void Scene::addItem(QMimeData *mimeData)
{
    QByteArray itemData = mimeData->data("application/x-dnditemdata");
    QDataStream stream(&itemData, QIODevice::ReadOnly);
    QPoint offset;
    ElementType type;
    QString labelAuxData;
    stream >> offset >> type >> labelAuxData;
    auto *elm = ElementFactory::buildElement(type);
    qCDebug(zero) << tr("Valid element.");
    if (elm->elementType() == ElementType::IC) {
        if (auto *ic = dynamic_cast<IC *>(elm)) {
            ICManager::loadIC(ic, labelAuxData);
        }
    }
    qCDebug(zero) << tr("Adding the element to the scene.");
    receiveCommand(new AddItemsCommand({elm}, this));
    qCDebug(zero) << tr("Cleaning the selection.");
    clearSelection();
    qCDebug(zero) << tr("Setting created element as selected.");
    elm->setSelected(true);
    qCDebug(zero) << tr("Adjusting the position of the element.");

    mimeData->deleteLater();
}
