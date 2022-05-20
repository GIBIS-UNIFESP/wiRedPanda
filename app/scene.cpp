// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scene.h"

#include "application.h"
#include "buzzer.h"
#include "commands.h"
#include "common.h"
#include "elementfactory.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "ic.h"
#include "input.h"
#include "qneconnection.h"
#include "qneport.h"
#include "serializationfunctions.h"
#include "thememanager.h"

#include <QClipboard>
#include <QDrag>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsView>
#include <QIODevice>
#include <QKeyEvent>
#include <QMenu>
#include <QMimeData>
#include <QPainter>

Scene::Scene(QObject *parent)
    : QGraphicsScene(parent)
    , m_simulationController(this)
{
    m_selectionRect.setFlag(QGraphicsItem::ItemIsSelectable, false);
    addItem(&m_selectionRect);

    m_undoAction = undoStack()->createUndoAction(this, tr("&Undo"));
    m_undoAction->setIcon(QIcon(":/toolbar/undo.png"));
    m_undoAction->setShortcut(QKeySequence::Undo);

    m_redoAction = undoStack()->createRedoAction(this, tr("&Redo"));
    m_redoAction->setIcon(QIcon(":/toolbar/redo.png"));
    m_redoAction->setShortcut(QKeySequence::Redo);

    connect(&ICManager::instance(), &ICManager::updatedIC, this, &Scene::redoSimulationController);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &Scene::updateTheme);
    connect(&m_undoStack, &QUndoStack::indexChanged, this, &Scene::checkUpdateRequest);
    connect(this, &Scene::circuitHasChanged, &m_simulationController, &SimulationController::reSortElements);
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

int Scene::gridSize() const
{
    return m_gridSize;
}

void Scene::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    QGraphicsScene::drawBackground(painter, rect);
    painter->setPen(m_dots);
    qreal left = static_cast<int>(rect.left()) - (static_cast<int>(rect.left()) % m_gridSize);
    qreal top = static_cast<int>(rect.top()) - (static_cast<int>(rect.top()) % m_gridSize);
    QVector<QPointF> points;
    for (qreal x = left; x < rect.right(); x += m_gridSize) { // TODO: dont use qreal as counter
        for (qreal y = top; y < rect.bottom(); y += m_gridSize) { // TODO: dont use qreal as counter
            points.append(QPointF(x, y));
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
    const auto gviews = views();
    auto *graphicsView = gviews.first();
    if (!graphicsView->isActiveWindow()) {
        graphicsView = gviews.last();
    }
    auto visibleRect = graphicsView->mapToScene(graphicsView->viewport()->geometry()).boundingRect();

    return elements(visibleRect);
}

QVector<GraphicElement *> Scene::elements()
{
    QVector<GraphicElement *> elements_;
    const auto items_ = items();
    elements_.reserve(items_.size());
    for (auto *item : items_) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (elm) {
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
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (elm) {
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
        auto *conn = qgraphicsitem_cast<QNEConnection *>(item);
        if (conn) {
            conns.append(conn);
        }
    }
    return conns;
}

QVector<GraphicElement *> Scene::selectedElements()
{
    QVector<GraphicElement *> elements;
    const auto selectedItems_ = selectedItems();
    for (auto *item : selectedItems_) {
        if (item->type() == GraphicElement::Type) {
            auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
            if (elm) {
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
    QVector<GraphicElement *> elms = elements();
    if (!elms.isEmpty()) {
        QRectF rect = sceneRect();
        for (auto *elm : qAsConst(elms)) {
            QRectF itemRect = elm->boundingRect().translated(elm->pos());
            rect = rect.united(itemRect.adjusted(-10, -10, 10, 10));
        }
        setSceneRect(rect);
    }
    QGraphicsItem *item = itemAt(m_mousePos);
    if (item && (m_timer.elapsed() > 100) && m_draggingElement) {
        if (!views().isEmpty()) {
            const auto scene_views = views();
            QGraphicsView *view = scene_views.first();
            view->ensureVisible(QRectF(m_mousePos - QPointF(4, 4), QSize(9, 9)).normalized());
        }
        m_timer.restart();
    }
}

QNEConnection *Scene::editedConnection() const
{
    return dynamic_cast<QNEConnection *>(ElementFactory::itemById(m_editedConnection_id));
}

void Scene::deleteEditedConnection()
{
    QNEConnection *connection = editedConnection();
    if (connection) {
        removeItem(connection);
        delete connection;
    }
    setEditedConnection(nullptr);
}

void Scene::setEditedConnection(QNEConnection *connection)
{
    if (connection) {
        connection->setFocus();
        m_editedConnection_id = connection->id();
    } else {
        m_editedConnection_id = 0;
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

void Scene::setHandlingEvents(const bool value)
{
    m_handlingEvents = value;
}

QUndoStack *Scene::undoStack()
{
    return &m_undoStack;
}

void Scene::makeConnection(QNEConnection *connection)
{
    auto *port = dynamic_cast<QNEPort *>(itemAt(m_mousePos));
    if (port && connection) {
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
}

void Scene::detachConnection(QNEInputPort *endPort)
{
    QNEConnection *connection = endPort->connections().last();
    QNEOutputPort *startPort = connection->start();
    if (startPort) {
        receiveCommand(new DeleteItemsCommand({connection}, this));
        startNewConnection(startPort);
    }
}

void Scene::updateTheme()
{
    qCDebug(zero) << "Update theme.";
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
    qCDebug(zero) << "Finished updating theme.";
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
        if ((item->type() == QNEConnection::Type)) {
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

void Scene::redoSimulationController()
{
    bool simulationWasRunning = m_simulationController.isRunning();
    m_simulationController.clear();
    if (simulationWasRunning) {
        m_simulationController.start();
    }
}

void Scene::cloneDrag(const QPointF pos)
{
    qCDebug(zero) << "Ctrl + Drag action triggered.";
    QVector<GraphicElement *> selectedElms = selectedElements();
    if (!selectedElms.isEmpty()) {
        QRectF rect;
        for (auto *elm : qAsConst(selectedElms)) {
            rect = rect.united(elm->boundingRect().translated(elm->pos()));
        }
        rect = rect.adjusted(-8, -8, 8, 8);
        QImage image(rect.size().toSize(), QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        QPainter painter(&image);
        painter.setOpacity(0.25);
        render(&painter, image.rect(), rect);

        QByteArray itemData;
        QDataStream stream(&itemData, QIODevice::WriteOnly);

        QPointF offset = pos - rect.topLeft();
        stream << pos;

        copy(selectedItems(), stream);

        auto *mimeData = new QMimeData;
        mimeData->setData("application/ctrlDragData", itemData);

        auto *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(QPixmap::fromImage(image));
        drag->setHotSpot(offset.toPoint());
        drag->exec(Qt::CopyAction, Qt::CopyAction);
    }
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
    SerializationFunctions::serialize(selectedItems(), stream);
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
            m_hoverPortElm_id = hoverElm->id();
            for (int i = 0; i < (hoverElm->inputSize() + hoverElm->outputSize()); ++i) {
                if (i < hoverElm->inputSize()) {
                    if (port == hoverElm->input(i)) {
                        m_hoverPort_nbr = i;
                    }
                } else if (port == hoverElm->output(i - hoverElm->inputSize())) {
                    m_hoverPort_nbr = i;
                }
            }
        }
    } else {
        m_hoverPortElm_id = 0;
        m_hoverPort_nbr = 0;
    }
}

QNEPort *Scene::hoverPort()
{
    auto *hoverElm = dynamic_cast<GraphicElement *>(ElementFactory::itemById(m_hoverPortElm_id));
    QNEPort *hoverPort = nullptr;
    if (hoverElm) {
        if (m_hoverPort_nbr < hoverElm->inputSize()) {
            hoverPort = hoverElm->input(m_hoverPort_nbr);
        } else if (((m_hoverPort_nbr - hoverElm->inputSize()) < hoverElm->outputSize())) {
            hoverPort = hoverElm->output(m_hoverPort_nbr - hoverElm->inputSize());
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
    auto *item = itemAt(m_mousePos);
    if (item) {
        if (selectedItems().contains(item)) {
            emit contextMenuPos(screenPos);
        } else if ((item->type() == GraphicElement::Type)) {
            clearSelection();
            item->setSelected(true);
            emit contextMenuPos(screenPos);
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
    auto itemList = SerializationFunctions::deserialize(stream, version);
    receiveCommand(new AddItemsCommand(itemList, this));
    for (auto *item : qAsConst(itemList)) {
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
        redoSimulationController();
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
    double angle = 90.0;
    auto list = selectedItems();
    QList<GraphicElement *> elms;
    elms.reserve(list.size());
    for (auto *item : qAsConst(list)) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (elm && (elm->type() == GraphicElement::Type)) {
            elms.append(elm);
        }
    }
    if ((elms.size() > 1) || ((elms.size() == 1) && elms.first()->rotatable())) {
        receiveCommand(new RotateCommand(elms, static_cast<int>(angle), this));
    }
}

void Scene::mute(const bool mute)
{
    const auto scene_elems = elements();
    for (auto *elm : scene_elems) {
        if (auto *buzzer = dynamic_cast<Buzzer *>(elm)) {
            buzzer->mute(mute);
        }
    }
}

void Scene::rotateLeft()
{
    double angle = -90.0;
    auto list = selectedItems();
    QList<GraphicElement *> elms;
    elms.reserve(list.size());
    for (auto *item : qAsConst(list)) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (elm && (elm->type() == GraphicElement::Type)) {
            elms.append(elm);
        }
    }
    if ((elms.size() > 1) || ((elms.size() == 1) && elms.first()->rotatable())) {
        receiveCommand(new RotateCommand(elms, static_cast<int>(angle), this));
    }
}

void Scene::flipHorizontally()
{
    auto list = selectedItems();
    QList<GraphicElement *> elms;
    elms.reserve(list.size());
    for (auto *item : qAsConst(list)) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (elm && (elm->type() == GraphicElement::Type)) {
            elms.append(elm);
        }
    }

    if ((elms.size() > 1) || ((elms.size() == 1))) {
        receiveCommand(new FlipCommand(elms, 0, this));
    }
}

void Scene::flipVertically()
{
    auto list = selectedItems();
    QList<GraphicElement *> elms;
    elms.reserve(list.size());
    for (auto *item : qAsConst(list)) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (elm && (elm->type() == GraphicElement::Type)) {
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
        return event->accept();
    }

    QGraphicsScene::dragEnterEvent(event);
}

void Scene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    const auto formats = event->mimeData()->formats();

    if (formats.contains("application/x-dnditemdata") || formats.contains("application/ctrlDragData")) {
        return event->accept();
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
        qCDebug(zero) << "Dropped element of type:" << type << " at position:" << pos.x() << ", " << pos.y() << ", label:" << labelAuxData;
        auto *elm = ElementFactory::buildElement(type, true);
        qCDebug(zero) << "Valid element.";
        if (elm->elementType() == ElementType::IC) {
            if (auto *ic = dynamic_cast<IC *>(elm)) {
                ICManager::loadIC(ic, labelAuxData);
            }
        }
        int widthOffset = static_cast<int>((64 - elm->boundingRect().width()) / 2);
        if (widthOffset > 0) {
            pos += QPointF(widthOffset, widthOffset);
        }
        /*
   * TODO: Rotate all element icons, remake the port position logic, and remove the code below.
   * Rotating element in 90 degrees.
   */
        if (elm->rotatable() && (elm->elementType() != ElementType::Node) && (elm->elementGroup() != ElementGroup::Other)) {
            elm->setRotation(90);
        }
        qCDebug(zero) << "Adding the element to the scene.";
        receiveCommand(new AddItemsCommand({elm}, this));
        qCDebug(zero) << "Cleaning the selection.";
        clearSelection();
        qCDebug(zero) << "Setting created element as selected.";
        elm->setSelected(true);
        qCDebug(zero) << "Adjusting the position of the element.";
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
        auto itemList = SerializationFunctions::deserialize(stream, version);
        receiveCommand(new AddItemsCommand(itemList, this));
        clearSelection();
        for (auto *item : qAsConst(itemList)) {
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
                auto *input = dynamic_cast<Input *>(elm);
                if (input && !input->isLocked()) {
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
                auto *input = dynamic_cast<Input *>(elm);
                if (input && !input->isLocked() && (elm->elementType() == ElementType::InputButton)) {
                    input->setOff();
                }
            }
        }
    }

    QGraphicsScene::keyReleaseEvent(event);
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_mousePos = event->scenePos();
    resizeScene();
    handleHoverPort();

    if (event->modifiers() & Qt::ShiftModifier) {
        event->setModifiers(Qt::ControlModifier);
    }

    auto *item = itemAt(m_mousePos);

    if (item) {
        if (event->button() == Qt::LeftButton) {
            if ((event->modifiers() & Qt::ControlModifier) && (item->type() == GraphicElement::Type)) {
                item->setSelected(true);
                cloneDrag(event->scenePos());
                return;
            }

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
    resizeScene();
    handleHoverPort();

    if (event->modifiers() & Qt::ShiftModifier) {
        event->setModifiers(Qt::ControlModifier);
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
            bool valid = false; // std::any_of?
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

    if (auto *connection = editedConnection(); connection && (event->buttons() == Qt::NoButton)) {
        makeConnection(connection);
        return;
    }

    QGraphicsScene::mouseReleaseEvent(event);
}

void Scene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    auto *connection = dynamic_cast<QNEConnection *>(itemAt(m_mousePos));

    if (connection && connection->start() && connection->end()) {
        receiveCommand(new SplitCommand(connection, m_mousePos, this));
    }

    QGraphicsScene::mouseDoubleClickEvent(event);
}
