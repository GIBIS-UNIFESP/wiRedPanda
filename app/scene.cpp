// Copyright 2015 - 2022, GIBIS-UNIFESP and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scene.h"

#include "buzzer.h"
#include "commands.h"
#include "common.h"
#include "elementfactory.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "graphicelementinput.h"
#include "graphicsview.h"
#include "ic.h"
#include "qneconnection.h"
#include "serialization.h"
#include "thememanager.h"

#include <QClipboard>
#include <QDrag>
#include <QGraphicsSceneDragDropEvent>
#include <QKeyEvent>
#include <QMenu>

Scene::Scene(QObject *parent)
    : QGraphicsScene(parent)
    , m_simulation(this)
{
    installEventFilter(this);

    m_selectionRect.setFlag(QGraphicsItem::ItemIsSelectable, false);
    addItem(&m_selectionRect);

    m_undoAction = undoStack()->createUndoAction(this, tr("&Undo"));
    m_undoAction->setIcon(QIcon(":/toolbar/undo.svg"));
    m_undoAction->setShortcut(QKeySequence::Undo);

    m_redoAction = undoStack()->createRedoAction(this, tr("&Redo"));
    m_redoAction->setIcon(QIcon(":/toolbar/redo.svg"));
    m_redoAction->setShortcut(QKeySequence::Redo);

    m_timer.start();

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &Scene::updateTheme);
    connect(&m_undoStack,              &QUndoStack::indexChanged,   this, &Scene::checkUpdateRequest);
}

void Scene::checkUpdateRequest()
{
    if (m_autosaveRequired) {
        emit circuitHasChanged();
        m_autosaveRequired = false;
    }
}

void Scene::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    QGraphicsScene::drawBackground(painter, rect);

    const int gridSize = GlobalProperties::gridSize;
    const int left = static_cast<int>(rect.left()) - (static_cast<int>(rect.left()) % gridSize);
    const int top = static_cast<int>(rect.top()) - (static_cast<int>(rect.top()) % gridSize);
    QVector<QPoint> points;

    for (int x = left; x < rect.right(); x += gridSize) {
        for (int y = top; y < rect.bottom(); y += gridSize) {
            points.append(QPoint(x, y));
        }
    }

    painter->setPen(m_dots);
    painter->drawPoints(points.data(), points.size());
}

void Scene::setDots(const QPen &dots)
{
    m_dots = dots;
}

Simulation *Scene::simulation()
{
    return &m_simulation;
}

void Scene::setAutosaveRequired()
{
    m_autosaveRequired = true;
}

void Scene::setCircuitUpdateRequired()
{
    // set these again to avoid having new ports showing when elements are invisible
    showWires(m_showWires);
    showGates(m_showGates);

    m_simulation.initialize();

    m_autosaveRequired = true;
}

const QVector<GraphicElement *> Scene::visibleElements() const
{
    const auto visibleRect = m_view->mapToScene(m_view->viewport()->geometry()).boundingRect();

    return elements(visibleRect);
}

const QVector<GraphicElement *> Scene::elements() const
{
    const auto items_ = items();
    QVector<GraphicElement *> elements_;
    elements_.reserve(items_.size());

    for (auto *item : items_) {
        if (item->type() == GraphicElement::Type) {
            elements_.append(qgraphicsitem_cast<GraphicElement *>(item));
        }
    }

    return elements_;
}

const QVector<GraphicElement *> Scene::elements(const QRectF &rect) const
{
    const auto items_ = items(rect);
    QVector<GraphicElement *> elements_;
    elements_.reserve(items_.size());

    for (auto *item : items_) {
        if (item->type() == GraphicElement::Type) {
            elements_.append(qgraphicsitem_cast<GraphicElement *>(item));
        }
    }

    return elements_;
}

const QVector<QNEConnection *> Scene::connections()
{
    const auto items_ = items();
    QVector<QNEConnection *> conns;
    conns.reserve(items_.size());

    for (auto *item : items_) {
        if (item->type() == QNEConnection::Type) {
            conns.append(qgraphicsitem_cast<QNEConnection *>(item));
        }
    }

    return conns;
}

const QList<GraphicElement *> Scene::selectedElements() const
{
    const auto items_ = selectedItems();
    QList<GraphicElement *> elements_;

    for (auto *item : items_) {
        if (item->type() == GraphicElement::Type) {
            elements_.append(qgraphicsitem_cast<GraphicElement *>(item));
        }
    }

    return elements_;
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
    setSceneRect(itemsBoundingRect());

    auto *item = itemAt(m_mousePos);

    if (item && (m_timer.elapsed() > 100) && m_draggingElement) {
        // FIXME: sometimes this goes into a infinite loop and crashes
        item->ensureVisible();
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
    auto *connection = new QNEConnection();
    connection->setEnd(endPort);
    connection->setStartPos(m_mousePos);

    addItem(connection);
    setEditedConnection(connection);
    connection->updatePath();
}

void Scene::startNewConnection(QNEOutputPort *startPort)
{
    auto *connection = new QNEConnection();
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

    for (auto *element : elements()) {
        element->updateTheme();
    }

    for (auto *conn : connections()) {
        conn->updateTheme();
    }

    qCDebug(zero) << tr("Finished updating theme.");
}

QList<QGraphicsItem *> Scene::items(Qt::SortOrder order) const
{
    return QGraphicsScene::items(order);
}

QList<QGraphicsItem *> Scene::items(const QPointF &pos, Qt::ItemSelectionMode mode, Qt::SortOrder order, const QTransform &deviceTransform) const
{
    return QGraphicsScene::items(pos, mode, order, deviceTransform);
}

QList<QGraphicsItem *> Scene::items(const QRectF &rect, Qt::ItemSelectionMode mode, Qt::SortOrder order, const QTransform &deviceTransform) const
{
    return QGraphicsScene::items(rect, mode, order, deviceTransform);
}

QList<QGraphicsItem *> Scene::items(const QPolygonF &polygon, Qt::ItemSelectionMode mode, Qt::SortOrder order, const QTransform &deviceTransform) const
{
    return QGraphicsScene::items(polygon, mode, order, deviceTransform);
}

QList<QGraphicsItem *> Scene::items(const QPainterPath &path, Qt::ItemSelectionMode mode, Qt::SortOrder order, const QTransform &deviceTransform) const
{
    return QGraphicsScene::items(path, mode, order, deviceTransform);
}

void Scene::showGates(const bool checked)
{
    m_showGates = checked;
    const auto items_ = items();

    for (auto *item : items_) {
        if (item->type() == GraphicElement::Type) {
            const auto group = qgraphicsitem_cast<GraphicElement *>(item)->elementGroup();

            if ((group != ElementGroup::Input) && (group != ElementGroup::Output) && (group != ElementGroup::Other)) {
                item->setVisible(checked);
            }

        }
    }
}

void Scene::showWires(const bool checked)
{
    m_showWires = checked;
    const auto items_ = items();

    for (auto *item : items_) {
        if (item->type() == QNEConnection::Type) {
            item->setVisible(checked);
            continue;
        }

        if (item->type() == GraphicElement::Type) {
            auto *element = qgraphicsitem_cast<GraphicElement *>(item);

            if (element->elementType() == ElementType::Node) {
                element->setVisible(checked);
            } else {
                for (auto *inputPort : element->inputs()) {
                    inputPort->setVisible(checked);
                }

                for (auto *outputPort : element->outputs()) {
                    outputPort->setVisible(checked);
                }
            }
        }
    }
}

void Scene::cloneDrag(const QPointF mousePos)
{
    qCDebug(zero) << tr("Ctrl + Drag action triggered.");
    const auto selectedElements_ = selectedElements();

    if (selectedElements_.isEmpty()) {
        return;
    }

    const auto items_ = items();

    for (auto *item : items_) {
        if ((item->type() == GraphicElement::Type || item->type() == QNEConnection::Type) && !item->isSelected()) {
            item->hide();
        }
    }

    QRectF rect;

    for (auto *element : selectedElements_) {
        rect = rect.united(element->sceneBoundingRect());
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
    stream.setVersion(QDataStream::Qt_5_12);
    stream << mousePos;
    copy(selectedItems(), stream);

    auto *mimeData = new QMimeData;
    mimeData->setData("wpanda/ctrlDragData", itemData);

    auto *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(QPixmap::fromImage(image));
    QPointF offset = m_view->transform().map(mousePos - rect.topLeft());
    drag->setHotSpot(offset.toPoint());
    drag->exec(Qt::CopyAction, Qt::CopyAction);
}

void Scene::copy(const QList<QGraphicsItem *> &items, QDataStream &stream)
{
    QPointF center(0.0, 0.0);
    int itemsQuantity = 0;

    for (auto *item : items) {
        if (item->type() == GraphicElement::Type) {
            center += item->pos();
            itemsQuantity++;
        }
    }

    stream << center / static_cast<qreal>(itemsQuantity);
    Serialization::serialize(items, stream);
}

void Scene::handleHoverPort()
{
    auto *port = dynamic_cast<QNEPort *>(itemAt(m_mousePos));
    auto *hoverPort_ = hoverPort();

    if (hoverPort_ && (hoverPort_ != port)) {
        releaseHoverPort();
    }

    if (port) {
        auto *editedConn = editedConnection();
        releaseHoverPort();
        setHoverPort(port);

        if (editedConn && editedConn->start() && (editedConn->start()->isOutput() == port->isOutput())) {
            m_view->viewport()->setCursor(Qt::ForbiddenCursor);
        }
    }
}

void Scene::releaseHoverPort()
{
    if (auto *hoverPort_ = hoverPort()) {
        hoverPort_->hoverLeave();
        setHoverPort(nullptr);
        m_view->viewport()->unsetCursor();
    }
}

void Scene::setHoverPort(QNEPort *port)
{
    if (!port) {
        m_hoverPortElmId = 0;
        m_hoverPortNumber = 0;
        return;
    }

    port->hoverEnter();
    auto *hoverElm = port->graphicElement();

    if (hoverElm && ElementFactory::contains(hoverElm->id())) {
        m_hoverPortElmId = hoverElm->id();

        for (int i = 0; i < (hoverElm->inputSize() + hoverElm->outputSize()); ++i) {
            if (i < hoverElm->inputSize()) {
                if (port == hoverElm->inputPort(i)) {
                    m_hoverPortNumber = i;
                }
            } else if (port == hoverElm->outputPort(i - hoverElm->inputSize())) {
                m_hoverPortNumber = i;
            }
        }
    }
}

QNEPort *Scene::hoverPort()
{
    QNEPort *hoverPort = nullptr;

    if (auto *hoverElm = dynamic_cast<GraphicElement *>(ElementFactory::itemById(m_hoverPortElmId))) {
        if (m_hoverPortNumber < hoverElm->inputSize()) {
            hoverPort = hoverElm->inputPort(m_hoverPortNumber);
        } else if (((m_hoverPortNumber - hoverElm->inputSize()) < hoverElm->outputSize())) {
            hoverPort = hoverElm->outputPort(m_hoverPortNumber - hoverElm->inputSize());
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
        } else if (item->type() == GraphicElement::Type) {
            clearSelection();
            item->setSelected(true);
            emit contextMenuPos(screenPos, item);
        }
    } else {
        QMenu menu;
        auto *pasteAction = menu.addAction(QIcon(QPixmap(":/toolbar/paste.svg")), tr("Paste"));
        const auto *mimeData = QApplication::clipboard()->mimeData();

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
    if (selectedElements().empty()) {
        QApplication::clipboard()->clear();
        return;
    }

    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_12);
    copy(selectedItems(), stream);

    auto *mimeData = new QMimeData;
    mimeData->setData("wpanda/copydata", itemData);

    QApplication::clipboard()->setMimeData(mimeData);
}

void Scene::cutAction()
{
    if (selectedElements().isEmpty()) {
        QApplication::clipboard()->clear();
        return;
    }

    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_12);
    cut(selectedItems(), stream);

    auto *mimeData = new QMimeData();
    mimeData->setData("wpanda/copydata", itemData);

    QApplication::clipboard()->setMimeData(mimeData);
}

void Scene::pasteAction()
{
    const auto *mimeData = QApplication::clipboard()->mimeData();

    if (mimeData->hasFormat("wpanda/copydata")) {
        QByteArray itemData = mimeData->data("wpanda/copydata");
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_5_12);
        paste(stream);
    }
}

void Scene::paste(QDataStream &stream)
{
    clearSelection();

    QPointF ctr;
    stream >> ctr;

    const QPointF offset = m_mousePos - ctr - QPointF(32.0, 32.0);
    const double version = GlobalProperties::version;
    const auto itemList = Serialization::deserialize(stream, {}, version);

    receiveCommand(new AddItemsCommand(itemList, this));

    for (auto *item : itemList) {
        if (item->type() == GraphicElement::Type) {
            item->setPos((item->pos() + offset));
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
    const auto selectedItems_ = selectedItems();
    clearSelection();

    if (!selectedItems_.isEmpty()) {
        receiveCommand(new DeleteItemsCommand(selectedItems_, this));
        m_simulation.restart();
    }
}

void Scene::selectAll()
{
    const auto items_ = items();

    for (auto *item : items_) {
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

void Scene::rotate(const int angle)
{ 
    const auto elements_ = selectedElements();

    if (!elements_.isEmpty()) {
        receiveCommand(new RotateCommand(elements_, angle, this));
    }
}

void Scene::mute(const bool mute)
{
    for (auto *element : elements()) {
        if (element->elementType() == ElementType::Buzzer) {
            qobject_cast<Buzzer *>(element)->mute(mute);
        }
    }
}

void Scene::flipHorizontally()
{
    const auto elements_ = selectedElements();

    if (elements_.isEmpty()) {
        receiveCommand(new FlipCommand(elements_, 0, this));
    }
}

void Scene::flipVertically()
{
    const auto elements_ = selectedElements();

    if (!elements_.isEmpty()) {
        receiveCommand(new FlipCommand(elements_, 1, this));
    }
}

void Scene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    const auto formats = event->mimeData()->formats();

    if (formats.contains("wpanda/x-dnditemdata") || formats.contains("wpanda/ctrlDragData")) {
        event->accept();
        return;
    }

    QGraphicsScene::dragEnterEvent(event);
}

void Scene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    const auto formats = event->mimeData()->formats();

    if (formats.contains("wpanda/x-dnditemdata") || formats.contains("wpanda/ctrlDragData")) {
        event->accept();
        return;
    }

    QGraphicsScene::dragMoveEvent(event);
}

void Scene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasFormat("wpanda/x-dnditemdata")) {
        QByteArray itemData = event->mimeData()->data("wpanda/x-dnditemdata");
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_5_12);

        QPoint offset;
        ElementType type;
        QString labelAuxData;
        stream >> offset >> type >> labelAuxData;

        QPointF pos = event->scenePos() - offset;
        qCDebug(zero) << type << tr(" at position: ") << pos.x() << tr(", ") << pos.y() << tr(", label: ") << labelAuxData;

        auto *element = ElementFactory::buildElement(type);
        qCDebug(zero) << tr("Valid element.");

        if (element->elementType() == ElementType::IC) {
            if (auto *ic = qobject_cast<IC *>(element)) {
                ic->loadFile(labelAuxData);
            }
        }

        qCDebug(zero) << tr("Adding the element to the scene.");
        receiveCommand(new AddItemsCommand({element}, this));

        qCDebug(zero) << tr("Cleaning the selection.");
        clearSelection();

        qCDebug(zero) << tr("Setting created element as selected.");
        element->setSelected(true);

        qCDebug(zero) << tr("Adjusting the position of the element.");
        element->setPos(pos);
    }

    if (event->mimeData()->hasFormat("wpanda/ctrlDragData")) {
        QByteArray itemData = event->mimeData()->data("wpanda/ctrlDragData");
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        stream.setVersion(QDataStream::Qt_5_12);

        QPointF offset;
        QPointF ctr;
        stream >> offset >> ctr;
        offset = event->scenePos() - offset;

        const double version = GlobalProperties::version;
        const auto itemList = Serialization::deserialize(stream, {}, version);

        receiveCommand(new AddItemsCommand(itemList, this));
        clearSelection();

        for (auto *item : itemList) {
            if (item->type() == GraphicElement::Type) {
                item->setPos((item->pos() + offset));
                item->setSelected(true);
            }
        }

        resizeScene();
    }

    QGraphicsScene::dropEvent(event);
}

void Scene::keyPressEvent(QKeyEvent *event)
{
    if (!(event->modifiers().testFlag(Qt::ControlModifier))) {
        for (auto *element : elements()) {
            if (element->hasTrigger() && !element->trigger().isEmpty() && element->trigger().matches(event->key())) {
                if (auto *input = qobject_cast<GraphicElementInput *>(element);
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
    if (!(event->modifiers().testFlag(Qt::ControlModifier))) {
        for (auto *element : elements()) {
            if (element->hasTrigger() && !element->trigger().isEmpty() && element->trigger().matches(event->key())) {
                if (auto *input = qobject_cast<GraphicElementInput *>(element);
                        input && !input->isLocked() && (element->elementType() == ElementType::InputButton)) {
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
        if (mouseEvent->modifiers().testFlag(Qt::ShiftModifier)) {
            mouseEvent->setModifiers(Qt::ControlModifier);
            return false;
        }

        if ((mouseEvent->button() == Qt::LeftButton)
                && (mouseEvent->modifiers().testFlag(Qt::ControlModifier))) {
            if (auto *item = itemAt(mouseEvent->scenePos());
                    item && (item->type() == GraphicElement::Type || item->type() == QNEConnection::Type)) {
                item->setSelected(true);
                cloneDrag(mouseEvent->scenePos());
                return true;
            }
        }
    }

    return QGraphicsScene::eventFilter(watched, event);
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_mousePos = event->scenePos();
    handleHoverPort();

    auto *item = itemAt(m_mousePos);

    if (item) {
        if (event->button() == Qt::LeftButton) {
            if (event->modifiers().testFlag(Qt::ControlModifier)) {
                item->setSelected(!item->isSelected());
            }

            auto selectedElements_ = selectedElements();

            if (auto *element = qgraphicsitem_cast<GraphicElement *>(item)) {
                selectedElements_ << element;
            }

            m_draggingElement = (item->type() == GraphicElement::Type && !selectedElements_.isEmpty());

            m_movedElements.clear();
            m_oldPositions.clear();

            for (auto *element : qAsConst(selectedElements_)) {
                m_movedElements.append(element);
                m_oldPositions.append(element->pos());
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
        const QRectF rect = QRectF(m_selectionStartPoint, m_mousePos).normalized();
        m_selectionRect.setRect(rect);
        QPainterPath selectionBox;
        selectionBox.addRect(rect);
        setSelectionArea(selectionBox);
    }

    QGraphicsScene::mouseMoveEvent(event);
}

void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_draggingElement && (event->button() == Qt::LeftButton)) {
        if (!m_movedElements.empty()) {
            const bool valid = std::any_of(m_movedElements.cbegin(), m_movedElements.cend(), [this](auto *elm) {
                return (elm->pos() != m_oldPositions.at(m_movedElements.indexOf(elm)));
            });

            if (valid) { receiveCommand(new MoveCommand(m_movedElements, m_oldPositions, this)); }
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
    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        return;
    }

    if (auto *connection = qgraphicsitem_cast<QNEConnection *>(itemAt(m_mousePos));
            connection && connection->start() && connection->end()) {
        receiveCommand(new SplitCommand(connection, m_mousePos, this));
    }

    QGraphicsScene::mouseDoubleClickEvent(event);
}

void Scene::addItem(QGraphicsItem *item)
{
    if (auto *element = dynamic_cast<Buzzer *>(item); element && element->label().isEmpty()) {
        element->setLabel(element->objectName() + "_" + QString::number(++m_buzzerLabelNumber));
    }

    QGraphicsScene::addItem(item);
}

void Scene::addItem(QMimeData *mimeData)
{
    QByteArray itemData = mimeData->data("wpanda/x-dnditemdata");
    QDataStream stream(&itemData, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_5_12);

    QPoint offset;
    ElementType type;
    QString labelAuxData;
    stream >> offset >> type >> labelAuxData;

    auto *element = ElementFactory::buildElement(type);
    qCDebug(zero) << tr("Valid element.");

    if (element->elementType() == ElementType::IC) {
        if (auto *ic = qobject_cast<IC *>(element)) {
            ic->loadFile(labelAuxData);
        }
    }

    qCDebug(zero) << tr("Adding the element to the scene.");
    receiveCommand(new AddItemsCommand({element}, this));

    qCDebug(zero) << tr("Cleaning the selection.");
    clearSelection();

    qCDebug(zero) << tr("Setting created element as selected.");
    element->setSelected(true);

    mimeData->deleteLater();
}
