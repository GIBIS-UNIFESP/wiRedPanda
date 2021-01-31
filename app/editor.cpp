// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editor.h"
#include "buzzer.h"
#include "commands.h"
#include "elementeditor.h"
#include "elementfactory.h"
#include "globalproperties.h"
#include "graphicelement.h"
#include "ic.h"
#include "icmanager.h"
#include "input.h"
#include "mainwindow.h"
#include "nodes/qneconnection.h"
#include "serializationfunctions.h"
#include "simulationcontroller.h"
#include "thememanager.h"

#include <QApplication>
#include <QClipboard>
#include <QDrag>
#include <QGraphicsItem>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QWheelEvent>
#include <QUndoCommand>
#include <QUndoStack>
#include <cmath>
#include <iostream>

Editor *Editor::globalEditor = nullptr;

Editor::Editor(QObject *parent)
    : QObject(parent)
    , scene(nullptr)
{
    if (!globalEditor) {
        globalEditor = this;
    }
    mainWindow = qobject_cast<MainWindow *>(parent);
    markingSelectionIC = false;
    _editedConn_id = 0;
    undoStack = new QUndoStack(this);
    scene = new Scene(this);

    icManager = new ICManager(mainWindow, this);

    install(scene);
    draggingElement = false;
    clear();
    timer.start();
    mShowWires = true;
    mShowGates = true;
    connect(this, &Editor::circuitHasChanged, simulationController, &SimulationController::reSortElms);
    connect(icManager, &ICManager::updatedIC, this, &Editor::redoSimulationController);
}

Editor::~Editor() = default;

//! CARMESIM
//#ifdef Q_OS_WIN
//#include <windows.h> // for Sleep
//#endif
// void _sleep(int ms)
//{
//    if (ms <= 0) { return; }

//#ifdef Q_OS_WIN
//    Sleep(uint(ms));
//#else
//    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
//    nanosleep(&ts, NULL);
//#endif
//}

void Editor::updateTheme()
{
    COMMENT("Update theme.", 0);
    if (ThemeManager::globalMngr) {
        const ThemeAttrs attrs = ThemeManager::globalMngr->getAttrs();
        if (!scene) {
            //! scene could be NULL here
            return;
        }
        scene->setBackgroundBrush(attrs.scene_bgBrush);
        scene->setDots(QPen(attrs.scene_bgDots));
        selectionRect->setBrush(QBrush(attrs.selectionBrush));
        selectionRect->setPen(QPen(attrs.selectionPen, 1, Qt::SolidLine));
        auto const scene_elements = scene->getElements();
        for (GraphicElement *elm : scene_elements) {
            elm->updateTheme();
        }
        auto const scene_connections = scene->getConnections();
        for (QNEConnection *conn : scene_connections) {
            conn->updateTheme();
        }
    }
    COMMENT("Finished updating theme. ", 0);
}

void Editor::mute(bool _mute)
{
    auto const scene_elems = scene->getElements();
    for (GraphicElement *elm : scene_elems) {
        auto *bz = dynamic_cast<Buzzer *>(elm);
        if (bz) {
            bz->mute(_mute);
        }
    }
}

void Editor::install(Scene *s)
{
    s->installEventFilter(this);
    simulationController = new SimulationController(s);
    simulationController->start();
    clear();
}

QNEConnection *Editor::getEditedConn() const
{
    return dynamic_cast<QNEConnection *>(ElementFactory::getItemById(_editedConn_id));
}

void Editor::setEditedConn(QNEConnection *editedConn)
{
    if (editedConn) {
        editedConn->setFocus();
        _editedConn_id = editedConn->id();
    } else {
        _editedConn_id = 0;
    }
}

void Editor::buildSelectionRect()
{
    COMMENT("Build Selection Rect.", 0);
    selectionRect = new QGraphicsRectItem();
    selectionRect->setFlag(QGraphicsItem::ItemIsSelectable, false);
    if (scene) {
        scene->addItem(selectionRect);
    }
    COMMENT("Finished building rect.", 0);
}

void Editor::clear()
{
    COMMENT("Clearing editor.", 0);
    //  fprintf(stderr, "Clearing editor\n");
    simulationController->stop();
    simulationController->clear();
    icManager->clear();
    ElementFactory::instance->clear();
    undoStack->clear();
    if (scene) {
        scene->clear();
    }
    COMMENT("Building rect.", 0);
    buildSelectionRect();
    if (scene) {
        auto const scene_views = scene->views();
        if (!scene_views.isEmpty()) {
            scene->setSceneRect(scene_views.front()->rect());
        }
    }
    COMMENT("Updating theme.", 0);
    updateTheme();
    simulationController->start();
    COMMENT("Emitting circuitHasChanged.", 0);
    emit circuitHasChanged();
    COMMENT("Finished clear.", 0);
}

//! CARMESIM: reset scene upon deletion in order to avoid SIGSEGV
void Editor::deleteAction()
{
    const QList<QGraphicsItem *> &items = scene->selectedItems();
    scene->clearSelection();
    if (!items.isEmpty()) {
        receiveCommand(new DeleteItemsCommand(items, this));
        redoSimulationController();
    }
}

void Editor::redoSimulationController()
{
    //! Guarantees that the simulation keeps running, if it was running upon the
    //! element deletion.
    bool simulationWasRunning = simulationController->isRunning();
    //! Clear the simulation controller.
    //! This is needed to avoid a SIGSEGV caused by lack of synch. between an element's graphics code and logical code,
    //! that is, its sprite could get deleted but its logical implementation not know about it.
    //! Also, it is required whenever the contents of a box is updated.
    simulationController->clear();
    if (simulationWasRunning) {
        getSimulationController()->start();
    }
}

void Editor::showWires(bool checked)
{
    mShowWires = checked;
    auto const scene_items = scene->items();
    for (QGraphicsItem *item : scene_items) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if ((item->type() == QNEConnection::Type)) {
            item->setVisible(checked);
        } else if ((item->type() == GraphicElement::Type) && elm) {
            if (elm->elementType() == ElementType::NODE) {
                elm->setVisible(checked);
            } else {
                auto const elm_inputs = elm->inputs();
                for (QNEPort *in : elm_inputs) {
                    in->setVisible(checked);
                }
                auto const elm_outputs = elm->outputs();
                for (QNEPort *out : elm_outputs) {
                    out->setVisible(checked);
                }
            }
        }
    }
}

void Editor::showGates(bool checked)
{
    mShowGates = checked;
    auto const scene_items = scene->items();
    for (QGraphicsItem *item : scene_items) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if ((item->type() == GraphicElement::Type) && elm) {
            if ((elm->elementGroup() != ElementGroup::INPUT) && (elm->elementGroup() != ElementGroup::OUTPUT)) {
                item->setVisible(checked);
            }
        }
    }
}

void Editor::rotate(bool rotateRight)
{
    double angle = 90.0;
    if (!rotateRight) {
        angle = -angle;
    }
    QList<QGraphicsItem *> list = scene->selectedItems();
    QList<GraphicElement *> elms;
    for (QGraphicsItem *item : qAsConst(list)) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (elm && (elm->type() == GraphicElement::Type)) {
            elms.append(elm);
        }
    }
    if ((elms.size() > 1) || ((elms.size() == 1) && elms.front()->rotatable())) {
        receiveCommand(new RotateCommand(elms, angle));
    }
}

void Editor::flipH()
{
    QList<QGraphicsItem *> list = scene->selectedItems();
    QList<GraphicElement *> elms;
    for (QGraphicsItem *item : qAsConst(list)) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (elm && (elm->type() == GraphicElement::Type)) {
            elms.append(elm);
        }
    }
    if ((elms.size() > 1) || ((elms.size() == 1))) {
        receiveCommand(new FlipCommand(elms, 0));
    }
}

void Editor::flipV()
{
    QList<QGraphicsItem *> list = scene->selectedItems();
    QList<GraphicElement *> elms;
    for (QGraphicsItem *item : qAsConst(list)) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (elm && (elm->type() == GraphicElement::Type)) {
            elms.append(elm);
        }
    }
    if ((elms.size() > 1) || ((elms.size() == 1))) {
        receiveCommand(new FlipCommand(elms, 1));
    }
}

QList<QGraphicsItem *> Editor::itemsAt(QPointF pos)
{
    QRectF rect(pos - QPointF(4, 4), QSize(9, 9));
    return scene->items(rect.normalized());
}

QGraphicsItem *Editor::itemAt(QPointF pos)
{
    QList<QGraphicsItem *> items = scene->items(pos);
    items.append(itemsAt(pos));
    for (QGraphicsItem *item : qAsConst(items)) {
        if (item->type() == QNEPort::Type) {
            return item;
        }
    }
    for (QGraphicsItem *item : qAsConst(items)) {
        if (item->type() > QGraphicsItem::UserType) {
            return item;
        }
    }
    return nullptr;
}

ElementEditor *Editor::getElementEditor() const
{
    return _elementEditor;
}

QPointF Editor::getMousePos() const
{
    return mousePos;
}

SimulationController *Editor::getSimulationController() const
{
    return simulationController;
}

void Editor::addItem(QGraphicsItem *item)
{
    scene->addItem(item);
}

void Editor::deleteEditedConn()
{
    QNEConnection *editedConn = getEditedConn();
    if (editedConn) {
        scene->removeItem(editedConn);
        delete editedConn;
    }
    setEditedConn(nullptr);
}

void Editor::startNewConnection(QNEOutputPort *startPort)
{
    QNEConnection *editedConn = ElementFactory::buildConnection();
    editedConn->setStart(startPort);
    editedConn->setEndPos(mousePos);
    addItem(editedConn);
    setEditedConn(editedConn);
    editedConn->updatePath();
}

void Editor::startNewConnection(QNEInputPort *endPort)
{
    QNEConnection *editedConn = ElementFactory::buildConnection();
    editedConn->setEnd(endPort);
    editedConn->setStartPos(mousePos);
    addItem(editedConn);
    setEditedConn(editedConn);
    editedConn->updatePath();
}

void Editor::detachConnection(QNEInputPort *endPort)
{
    QNEConnection *editedConn = endPort->connections().last();
    QNEOutputPort *startPort = editedConn->start();
    if (startPort) {
        receiveCommand(new DeleteItemsCommand(editedConn, this));
        startNewConnection(startPort);
    }
}

void Editor::startSelectionRect()
{
    selectionStartPoint = mousePos;
    markingSelectionIC = true;
    selectionRect->setRect(QRectF(selectionStartPoint, selectionStartPoint));
    selectionRect->show();
    selectionRect->update();
}

bool Editor::mousePressEvt(QGraphicsSceneMouseEvent *mouseEvt)
{
    QGraphicsItem *item = itemAt(mousePos);
    if (item && (item->type() == QNEPort::Type)) {
        /* When the mouse pressed over an connected input port, the line
         * is disconnected and can be connected in an other port. */
        auto *pressedPort = qgraphicsitem_cast<QNEPort *>(item);
        QNEConnection *editedConn = getEditedConn();
        if (editedConn) {
            makeConnection(editedConn);
        } else {
            if (pressedPort->isOutput()) {
                auto *startPort = dynamic_cast<QNEOutputPort *>(pressedPort);
                if (startPort) {
                    startNewConnection(startPort);
                }
            } else {
                auto *endPort = dynamic_cast<QNEInputPort *>(pressedPort);
                if (endPort) {
                    if (endPort->connections().size() > 0) {
                        detachConnection(endPort);
                    } else {
                        startNewConnection(endPort);
                    }
                }
            }
        }
        return true;
    }
    if (getEditedConn()) {
        deleteEditedConn();
    } else if (!item && (mouseEvt->button() == Qt::LeftButton)) {
        /* Mouse pressed over board (Selection box). */
        startSelectionRect();
    }

    return false;
}

void Editor::resizeScene()
{
    QVector<GraphicElement *> elms = scene->getElements();
    if (!elms.isEmpty()) {
        QRectF rect = scene->sceneRect();
        for (GraphicElement *elm : qAsConst(elms)) {
            QRectF itemRect = elm->boundingRect().translated(elm->pos());
            rect = rect.united(itemRect.adjusted(-10, -10, 10, 10));
        }
        scene->setSceneRect(rect);
    }
    QGraphicsItem *item = itemAt(mousePos);
    if (item && (timer.elapsed() > 100) && draggingElement) {
        if (!scene->views().isEmpty()) {
            auto const scene_views = scene->views();
            QGraphicsView *view = scene_views.front();
            view->ensureVisible(QRectF(mousePos - QPointF(4, 4), QSize(9, 9)).normalized());
        }
        timer.restart();
    }
}

bool Editor::mouseMoveEvt(QGraphicsSceneMouseEvent *mouseEvt)
{
    Q_UNUSED(mouseEvt)
    QNEConnection *editedConn = getEditedConn();
    if (editedConn) {
        /* If a connection is being created, the ending coordinate follows the mouse position. */
        if (editedConn->start()) {
            editedConn->setEndPos(mousePos);
            editedConn->updatePath();
        } else if (editedConn->end()) {
            editedConn->setStartPos(mousePos);
            editedConn->updatePath();
        } else {
            deleteEditedConn();
        }
        return true;
    }
    if (markingSelectionIC) {
        /* If is marking the selectionBox, the last coordinate follows the mouse position. */
        QRectF rect = QRectF(selectionStartPoint, mousePos).normalized();
        selectionRect->setRect(rect);
        QPainterPath selectionBox;
        selectionBox.addRect(rect);
        scene->setSelectionArea(selectionBox);
    } else if (!markingSelectionIC) {
        /* Else, the selectionRect is hidden. */
        selectionRect->hide();
    }
    return false;
}

void Editor::makeConnection(QNEConnection *editedConn)
{
    auto *port = dynamic_cast<QNEPort *>(itemAt(mousePos));
    if (port && editedConn) {
        /* The mouse is released over a QNEPort. */
        QNEOutputPort *startPort = nullptr;
        QNEInputPort *endPort = nullptr;
        if (editedConn->start() != nullptr) {
            startPort = editedConn->start();
            endPort = dynamic_cast<QNEInputPort *>(port);
        } else if (editedConn->end() != nullptr) {
            startPort = dynamic_cast<QNEOutputPort *>(port);
            endPort = editedConn->end();
        }
        if (!startPort || !endPort) {
            return;
        }
        /* Verifying if the connection is valid. */
        if ((startPort->graphicElement() != endPort->graphicElement()) && !startPort->isConnected(endPort)) {
            /* Making connection. */
            editedConn->setStart(startPort);
            editedConn->setEnd(endPort);
            receiveCommand(new AddItemsCommand(editedConn, this));
            setEditedConn(nullptr);
        } else {
            deleteEditedConn();
        }
    }
}

bool Editor::mouseReleaseEvt(QGraphicsSceneMouseEvent *mouseEvt)
{
    if (!mouseEvt) {
        return false;
    }
    /* When mouse is released the selection rect is hidden. */
    selectionRect->hide();
    markingSelectionIC = false;
    if (QApplication::overrideCursor()) {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
    }
    QNEConnection *editedConn = getEditedConn();
    if (editedConn && !(mouseEvt->buttons() & Qt::LeftButton)) {
        /* A connection is being created, and left button was released. */
        makeConnection(editedConn);
        return true;
    }
    return false;
}

void Editor::handleHoverPort()
{
    auto *port = dynamic_cast<QNEPort *>(itemAt(mousePos));
    QNEPort *hoverPort = getHoverPort();
    if (hoverPort && (port != hoverPort)) {
        releaseHoverPort();
    }
    if (port && (port->type() == QNEPort::Type)) {
        QNEConnection *editedConn = getEditedConn();
        releaseHoverPort();
        setHoverPort(port);
        if (editedConn && editedConn->start() && (editedConn->start()->isOutput() == port->isOutput())) {
            QApplication::setOverrideCursor(QCursor(Qt::ForbiddenCursor));
        }
    }
}

void Editor::releaseHoverPort()
{
    QNEPort *hoverPort = getHoverPort();
    if (hoverPort) {
        hoverPort->hoverLeave();
        setHoverPort(nullptr);
        QApplication::setOverrideCursor(QCursor());
    }
}

void Editor::setHoverPort(QNEPort *port)
{
    if (port) {
        GraphicElement *hoverElm = port->graphicElement();
        port->hoverEnter();
        if (hoverElm && ElementFactory::contains(hoverElm->id())) {
            _hoverPortElm_id = hoverElm->id();
            for (int i = 0; i < (hoverElm->inputSize() + hoverElm->outputSize()); ++i) {
                if (i < hoverElm->inputSize()) {
                    if (port == hoverElm->input(i)) {
                        _hoverPort_nbr = i;
                    }
                } else if (port == hoverElm->output(i - hoverElm->inputSize())) {
                    _hoverPort_nbr = i;
                }
            }
        }
    } else {
        _hoverPortElm_id = 0;
        _hoverPort_nbr = 0;
    }
}

QNEPort *Editor::getHoverPort()
{
    auto *hoverElm = dynamic_cast<GraphicElement *>(ElementFactory::getItemById(_hoverPortElm_id));
    QNEPort *hoverPort = nullptr;
    if (hoverElm) {
        if (_hoverPort_nbr < hoverElm->inputSize()) {
            hoverPort = hoverElm->input(_hoverPort_nbr);
        } else if (((_hoverPort_nbr - hoverElm->inputSize()) < hoverElm->outputSize())) {
            hoverPort = hoverElm->output(_hoverPort_nbr - hoverElm->inputSize());
        }
    }
    if (!hoverPort) {
        setHoverPort(nullptr);
    }
    return hoverPort;
}

bool Editor::dropEvt(QGraphicsSceneDragDropEvent *dde)
{
    /* Verify if mimetype is compatible. */
    if (dde->mimeData()->hasFormat("application/x-dnditemdata")) {
        /* Extracting mimedata from drop event. */
        QByteArray itemData = dde->mimeData()->data("application/x-dnditemdata");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        QPointF offset;
        QString label_auxData;
        qint32 type;
        dataStream >> offset >> type >> label_auxData;
        QPointF pos = dde->scenePos() - offset;
        dde->accept();

        GraphicElement *elm = ElementFactory::buildElement(static_cast<ElementType>(type));
        /* If element type is unknown, a default element is created with the pixmap received from mimedata */
        if (!elm) {
            return false;
        }
        if (elm->elementType() == ElementType::IC) {
            try {
                IC *box = dynamic_cast<IC *>(elm);
                if (box) {
                    QString fname = label_auxData;
                    if (!icManager->loadIC(box, fname, GlobalProperties::currentFile)) {
                        return false;
                    }
                }
            } catch (std::runtime_error &err) {
                QMessageBox::warning(mainWindow, tr("Error"), QString::fromStdString(err.what()));
                return false;
            }
        }
        int wdtOffset = (64 - elm->boundingRect().width()) / 2;
        if (wdtOffset > 0) {
            pos = pos + QPointF(wdtOffset, wdtOffset);
        }
        /*
         * TODO: Rotate all element icons, remake the port position logic, and remove the code below.
         * Rotating element in 90 degrees.
         */
        if (elm->rotatable() && (elm->elementType() != ElementType::NODE)) {
            elm->setRotation(90);
        }
        /* Adding the element to the scene. */
        receiveCommand(new AddItemsCommand(elm, this));
        /* Cleaning the selection. */
        scene->clearSelection();
        /* Setting created element as selected. */
        elm->setSelected(true);
        /* Adjusting the position of the element. */
        elm->setPos(pos);

        return true;
    }
    if (dde->mimeData()->hasFormat("application/ctrlDragData")) {
        QByteArray itemData = dde->mimeData()->data("application/ctrlDragData");
        QDataStream ds(&itemData, QIODevice::ReadOnly);
        QPointF offset;
        ds >> offset;
        offset = dde->scenePos() - offset;
        dde->accept();
        QPointF ctr;
        ds >> ctr;
        double version = GlobalProperties::version;
        QList<QGraphicsItem *> itemList = SerializationFunctions::deserialize(ds, version, GlobalProperties::currentFile);
        receiveCommand(new AddItemsCommand(itemList, this));
        scene->clearSelection();
        for (QGraphicsItem *item : qAsConst(itemList)) {
            if (item->type() == GraphicElement::Type) {
                item->setPos((item->pos() + offset));
                item->setSelected(true);
                item->update();
            }
        }
        resizeScene();

        return true;
    }
    return false;
}

bool Editor::dragMoveEvt(QGraphicsSceneDragDropEvent *dde)
{
    /* Accepting drag/drop event of the following mimedata format. */
    if (dde->mimeData()->hasFormat("application/x-dnditemdata")) {
        return true;
    }
    if (dde->mimeData()->hasFormat("application/ctrlDragData")) {
        return true;
    }
    return false;
}

bool Editor::wheelEvt(QWheelEvent *wEvt)
{
    if (wEvt) {
        QPoint numDegrees = wEvt->angleDelta() / 8;
        QPoint numSteps = numDegrees / 15;
        emit scroll(numSteps.x(), numSteps.y());
        wEvt->accept();
        return true;
    }
    return false;
}

void Editor::ctrlDrag(QPointF pos)
{
    COMMENT("Ctrl + Drag action triggered.", 0);
    QVector<GraphicElement *> selectedElms = scene->selectedElements();
    if (!selectedElms.isEmpty()) {
        QRectF rect;
        for (GraphicElement *elm : qAsConst(selectedElms)) {
            rect = rect.united(elm->boundingRect().translated(elm->pos()));
        }
        rect = rect.adjusted(-8, -8, 8, 8);
        QImage image(rect.size().toSize(), QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        QPainter painter(&image);
        painter.setOpacity(0.25);
        scene->render(&painter, image.rect(), rect);

        QByteArray itemData;
        QDataStream dataStream(&itemData, QIODevice::WriteOnly);

        QPointF offset = pos - rect.topLeft();
        dataStream << pos;

        copy(scene->selectedItems(), dataStream);

        auto *mimeData = new QMimeData;
        mimeData->setData("application/ctrlDragData", itemData);

        auto *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->setPixmap(QPixmap::fromImage(image));
        drag->setHotSpot(offset.toPoint());
        drag->exec(Qt::CopyAction, Qt::CopyAction);
    }
}

QUndoStack *Editor::getUndoStack() const
{
    return undoStack;
}

Scene *Editor::getScene() const
{
    return scene;
}

void Editor::cut(const QList<QGraphicsItem *> &items, QDataStream &ds)
{
    copy(items, ds);
    deleteAction();
}

void Editor::copy(const QList<QGraphicsItem *> &items, QDataStream &ds)
{
    QPointF center(static_cast<qreal>(0.0f), static_cast<qreal>(0.0f));
    float elm = 0;
    for (QGraphicsItem *item : items) {
        if (item->type() == GraphicElement::Type) {
            center += item->pos();
            elm++;
        }
    }
    ds << center / static_cast<qreal>(elm);
    SerializationFunctions::serialize(scene->selectedItems(), ds);
}

void Editor::paste(QDataStream &ds)
{
    scene->clearSelection();
    QPointF ctr;
    ds >> ctr;
    QPointF offset = mousePos - ctr - QPointF(static_cast<qreal>(32.0f), static_cast<qreal>(32.0f));
    double version = GlobalProperties::version;
    QList<QGraphicsItem *> itemList = SerializationFunctions::deserialize(ds, version, GlobalProperties::currentFile);
    receiveCommand(new AddItemsCommand(itemList, this));
    for (QGraphicsItem *item : qAsConst(itemList)) {
        if (item->type() == GraphicElement::Type) {
            item->setPos((item->pos() + offset));
            item->update();
            item->setSelected(true);
            // If input or output, set label
            // Parei aqui...
            //      if( ( item->elementGroup( ) == ElementGroup::INPUT ) || ( item->elementGroup( ) == ElementGroup::OUTPUT ) ) {
            //      }
        }
    }
    resizeScene();
}

void Editor::selectAll()
{
    auto const scene_items = scene->items();
    for (QGraphicsItem *item : scene_items) {
        item->setSelected(true);
    }
}

bool Editor::saveLocalIC(IC *ic, QString newICPath)
{
    try {
        if (ic) {
            COMMENT("Getting new paths for the ics.", 0)
            QString fname = ic->getFile();
            COMMENT("IC file name: " << fname.toStdString(), 0);
            auto icPrototype = icManager->getPrototype(fname);
            QString newFilePath = newICPath + "/boxes/" + QFileInfo(fname).fileName();
            COMMENT("newFilePath: " << newFilePath.toStdString(), 0);
            QFile fl(newFilePath);
            if (!fl.exists()) {
                COMMENT("Copying file to local dir. File does not exist yet.", 0);
                QFile::copy(fname, newFilePath);
                if (icPrototype->updateLocalIC(newFilePath, newICPath)) {
                    if (!ic->setFile(newFilePath)) {
                        std::cerr << "Error changing boxes name." << std::endl;
                        return false;
                    }
                } else {
                    std::cerr << "Error while saving boxes at the editor." << std::endl;
                    return false;
                }
            } else {
                if (!ic->setFile(newFilePath)) {
                    std::cerr << "Error changing boxes name." << std::endl;
                    return false;
                }
            }
        }
        return true;
    } catch (std::runtime_error &err) {
        QMessageBox::warning(mainWindow, tr("Error"), QString::fromStdString(err.what()));
        return false;
    }
}

bool Editor::saveLocal(QString newPath)
{
    if (!scene) {
        return true;
    }
    auto const scene_elements = scene->getElements();
    COMMENT("new path: " << newPath.toStdString(), 0);
    for (GraphicElement *elm : scene_elements) {
        elm->updateSkinsPath(newPath + "/skins/");
        if (elm->elementType() == ElementType::IC) {
            if (!saveLocalIC(dynamic_cast<IC *>(elm), newPath)) {
                return false;
            }
        }
    }
    return true;
}

void Editor::save(QDataStream &ds, const QString &dolphinFilename)
{
    ds << QApplication::applicationName() + " " + QString::number(GlobalProperties::version);
    ds << dolphinFilename;
    ds << scene->sceneRect();
    SerializationFunctions::serialize(scene->items(), ds);
}

void Editor::load(QDataStream &ds)
{
    COMMENT("Loading file.", 0);
    clear();
    COMMENT("Clear!", 0);
    simulationController->stop();
    COMMENT("Stoped simulation.", 0);
    double version = SerializationFunctions::loadVersion(ds);
    COMMENT("Version: " << version, 0);
    QString dolphinFilename(SerializationFunctions::loadDolphinFilename(ds, version));
    mainWindow->setDolphinFilename(dolphinFilename);
    COMMENT("Dolphin name: " << dolphinFilename.toStdString(), 0);
    QRectF rect(SerializationFunctions::loadRect(ds, version));
    COMMENT("Header Ok. Version: " << version, 0);
    QList<QGraphicsItem *> items = SerializationFunctions::deserialize(ds, version, GlobalProperties::currentFile);
    COMMENT("Finished loading items.", 0);
    if (scene) {
        for (QGraphicsItem *item : qAsConst(items)) {
            scene->addItem(item);
        }
        scene->setSceneRect(scene->itemsBoundingRect());
        if (!scene->views().empty()) {
            auto const scene_views = scene->views();
            QGraphicsView *view = scene_views.first();
            rect = rect.united(view->rect());
            rect.moveCenter(QPointF(0, 0));
            scene->setSceneRect(scene->sceneRect().united(rect));
            view->centerOn(scene->itemsBoundingRect().center());
        }
    }
    // SerializationFunctions::load( ds, GlobalProperties::currentFile, scene );
    simulationController->start();
    if (scene) {
        scene->clearSelection();
    }
    COMMENT("Emiting circuit has changed.", 0);
    emit circuitHasChanged();
    COMMENT("Finished loading file.", 0);
}

void Editor::setElementEditor(ElementEditor *value)
{
    _elementEditor = value;
    _elementEditor->setScene(scene);
    _elementEditor->setEditor(this);
    connect(_elementEditor, &ElementEditor::sendCommand, this, &Editor::receiveCommand);
}

QPointF roundTo(QPointF point, int multiple)
{
    int x = static_cast<int>(point.x());
    int y = static_cast<int>(point.y());
    int nx = multiple * std::floor(x / multiple);
    int ny = multiple * std::floor(y / multiple);
    return QPointF(nx, ny);
}

void Editor::contextMenu(QPoint screenPos)
{
    QGraphicsItem *item = itemAt(mousePos);
    if (item) {
        if (scene->selectedItems().contains(item)) {
            _elementEditor->contextMenu(screenPos);
        } else if ((item->type() == GraphicElement::Type)) {
            scene->clearSelection();
            item->setSelected(true);
            _elementEditor->contextMenu(screenPos);
        }
    } else {
        QMenu menu;
        QAction *pasteAction = menu.addAction(QIcon(QPixmap(":/toolbar/paste.png")), tr("Paste"));
        const QClipboard *clipboard = QApplication::clipboard();
        const QMimeData *mimeData = clipboard->mimeData();
        if (mimeData->hasFormat("wpanda/copydata")) {
            connect(pasteAction, &QAction::triggered, this, &Editor::pasteAction);
        } else {
            pasteAction->setEnabled(false);
        }
        menu.exec(screenPos);
    }
}

void Editor::updateVisibility()
{
    showGates(mShowGates);
    showWires(mShowWires);
}

void Editor::receiveCommand(QUndoCommand *cmd)
{
    undoStack->push(cmd);
}

void Editor::copyAction()
{
    QVector<GraphicElement *> elms = scene->selectedElements();
    if (elms.empty()) {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->clear();
    } else {
        QClipboard *clipboard = QApplication::clipboard();
        auto *mimeData = new QMimeData;
        QByteArray itemData;
        QDataStream dataStream(&itemData, QIODevice::WriteOnly);
        copy(scene->selectedItems(), dataStream);
        mimeData->setData("wpanda/copydata", itemData);
        clipboard->setMimeData(mimeData);
    }
}

void Editor::cutAction()
{
    QClipboard *clipboard = QApplication::clipboard();
    auto *mimeData = new QMimeData();
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    cut(scene->selectedItems(), dataStream);
    mimeData->setData("wpanda/copydata", itemData);
    clipboard->setMimeData(mimeData);
}

void Editor::pasteAction()
{
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    if (mimeData->hasFormat("wpanda/copydata")) {
        QByteArray itemData = mimeData->data("wpanda/copydata");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        paste(dataStream);
    }
}

bool Editor::eventFilter(QObject *obj, QEvent *evt)
{
    if (!evt) {
        return false;
    }
    if (obj == scene) {
        auto *dde = dynamic_cast<QGraphicsSceneDragDropEvent *>(evt);
        auto *mouseEvt = dynamic_cast<QGraphicsSceneMouseEvent *>(evt);
        auto *wEvt = dynamic_cast<QWheelEvent *>(evt);
        auto *keyEvt = dynamic_cast<QKeyEvent *>(evt);
        if (mouseEvt) {
            mousePos = mouseEvt->scenePos();
            resizeScene();
            handleHoverPort();
            if (mouseEvt->modifiers() & Qt::ShiftModifier) {
                mouseEvt->setModifiers(Qt::ControlModifier);
                return QObject::eventFilter(obj, evt);
            }
        }
        bool ret = false;
        if (mouseEvt && ((evt->type() == QEvent::GraphicsSceneMousePress) || (evt->type() == QEvent::GraphicsSceneMouseDoubleClick))) {
            QGraphicsItem *item = itemAt(mousePos);
            if (item && (mouseEvt->button() == Qt::LeftButton)) {
                if ((mouseEvt->modifiers() & Qt::ControlModifier) && (item->type() == GraphicElement::Type)) {
                    item->setSelected(true);
                    ctrlDrag(mouseEvt->scenePos());
                    return true;
                }
                draggingElement = true;
                /* STARTING MOVING ELEMENT */
                /*        qDebug() << "IN"; */
                QList<QGraphicsItem *> list = scene->selectedItems();
                list.append(itemsAt(mousePos));
                movedElements.clear();
                oldPositions.clear();
                for (QGraphicsItem *it : qAsConst(list)) {
                    auto *elm = qgraphicsitem_cast<GraphicElement *>(it);
                    if (elm) {
                        movedElements.append(elm);
                        oldPositions.append(elm->pos());
                    }
                }
            } else if (mouseEvt->button() == Qt::RightButton) {
                contextMenu(mouseEvt->screenPos());
            }
        }
        if (evt->type() == QEvent::GraphicsSceneMouseRelease) {
            if (draggingElement && (mouseEvt->button() == Qt::LeftButton)) {
                if (!movedElements.empty()) {
                    /*
                     *          if( movedElements.size( ) != oldPositions.size( ) ) {
                     *            throw std::runtime_error( ERRORMSG(tr( "Invalid coordinates." ).toStdString( ) ));
                     *          }
                     *          qDebug() << "OUT";
                     */
                    bool valid = false;
                    for (int elm = 0; elm < movedElements.size(); ++elm) {
                        if (movedElements[elm]->pos() != oldPositions[elm]) {
                            valid = true;
                            break;
                        }
                    }
                    if (valid) {
                        receiveCommand(new MoveCommand(movedElements, oldPositions));
                    }
                }
                draggingElement = false;
                movedElements.clear();
            }
        }
        switch (static_cast<int>(evt->type())) {
        case QEvent::GraphicsSceneMousePress: {
            ret = mousePressEvt(mouseEvt);
            break;
        }
        case QEvent::GraphicsSceneMouseMove: {
            ret = mouseMoveEvt(mouseEvt);
            break;
        }
        case QEvent::GraphicsSceneMouseRelease: {
            ret = mouseReleaseEvt(mouseEvt);
            break;
        }
        case QEvent::GraphicsSceneDrop: {
            ret = dropEvt(dde);
            break;
        }
        case QEvent::GraphicsSceneDragMove:
        case QEvent::GraphicsSceneDragEnter: {
            ret = dragMoveEvt(dde);
            break;
        }
        case QEvent::GraphicsSceneWheel: {
            ret = wheelEvt(wEvt);
            break;
        }
        case QEvent::GraphicsSceneMouseDoubleClick: {
            auto *connection = dynamic_cast<QNEConnection *>(itemAt(mousePos));
            if (connection && (connection->type() == QNEConnection::Type)) {
                /* Mouse pressed over a connection. */
                if (connection) {
                    if (connection->start() && connection->end()) {
                        receiveCommand(new SplitCommand(connection, mousePos, this));
                    }
                }
                evt->accept();
                return true;
            }
            break;
        }
        case QEvent::KeyPress: {
            if (keyEvt && !(keyEvt->modifiers() & Qt::ControlModifier)) {
                auto const scene_elems = scene->getElements();
                for (GraphicElement *elm : scene_elems) {
                    if (elm->hasTrigger() && !elm->getTrigger().isEmpty()) {
                        auto *in = dynamic_cast<Input *>(elm);
                        if (in && elm->getTrigger().matches(keyEvt->key())) {
                            if (elm->elementType() == ElementType::SWITCH) {
                                in->setOn(!in->getOn());
                            } else {
                                in->setOn(true);
                            }
                        }
                    }
                }
            }
            break;
        }
        case QEvent::KeyRelease: {
            if (keyEvt && !(keyEvt->modifiers() & Qt::ControlModifier)) {
                auto const scene_elems = scene->getElements();
                for (GraphicElement *elm : scene_elems) {
                    if (elm->hasTrigger() && !elm->getTrigger().isEmpty()) {
                        auto *in = dynamic_cast<Input *>(elm);
                        if (in && (elm->getTrigger().matches(keyEvt->key()) == QKeySequence::ExactMatch)) {
                            if (elm->elementType() != ElementType::SWITCH) {
                                in->setOn(false);
                            }
                        }
                    }
                }
            }
            break;
        }
        }
        if (ret) {
            return ret;
        }
    }
    return QObject::eventFilter(obj, evt);
}
