// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/Scene.h"

#include <algorithm>

#include <QClipboard>
#include <QDrag>
#include <QGraphicsSceneDragDropEvent>
#include <QKeyEvent>
#include <QMenu>

#include "App/Core/Common.h"
#include "App/Core/ItemWithId.h"
#include "App/Core/Priorities.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/GraphicElements/Buzzer.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Commands.h"
#include "App/Scene/GraphicsView.h"

Scene::Scene(QObject *parent)
    : QGraphicsScene(parent)
    , m_simulation(this)
{
    // The scene filters its own events to intercept mouse events before items see them
    // (e.g., to detect Ctrl+drag for cloning before Qt's default drag-selection activates)
    installEventFilter(this);

    // The rubber-band selection rectangle must not itself be selectable or it would
    // appear in selectedItems() and interfere with element operations
    m_selectionRect.setFlag(QGraphicsItem::ItemIsSelectable, false);
    addItem(&m_selectionRect);

    m_undoAction = undoStack()->createUndoAction(this, tr("&Undo"));
    m_undoAction->setIcon(QIcon(":/Interface/Toolbar/undo.svg"));
    m_undoAction->setShortcut(QKeySequence::Undo);

    m_redoAction = undoStack()->createRedoAction(this, tr("&Redo"));
    m_redoAction->setIcon(QIcon(":/Interface/Toolbar/redo.svg"));
    m_redoAction->setShortcut(QKeySequence::Redo);

    // Used to throttle expensive operations during drag (e.g., ensureVisible)
    m_timer.start();

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &Scene::updateTheme);
    // Emit autosave signal only after each undo-stack index change (not on every internal state update)
    connect(&m_undoStack,              &QUndoStack::indexChanged,   this, &Scene::checkUpdateRequest);
}

void Scene::addItem(QGraphicsItem *item)
{
    if (!item) {
        return;
    }
    QGraphicsScene::addItem(item);
    if (auto *iwid = dynamic_cast<ItemWithId *>(item)) {
        if (iwid->id() < 0) {
            // Unassigned item: give it the next scene-local ID
            iwid->setId(nextId());
        } else {
            // Pre-assigned item (restored by updateItemId for undo/redo): preserve it
            setLastId(iwid->id());
        }
        registerItem(iwid);
    }
}

void Scene::removeItem(QGraphicsItem *item)
{
    if (!item) {
        return;
    }
    if (auto *iwid = dynamic_cast<ItemWithId *>(item)) {
        unregisterItem(iwid);
    }
    QGraphicsScene::removeItem(item);
}

ItemWithId *Scene::itemById(const int id) const
{
    return m_elementRegistry.value(id, nullptr);
}

bool Scene::contains(const int id) const
{
    return m_elementRegistry.contains(id);
}

int Scene::lastId() const
{
    return m_lastId;
}

void Scene::setLastId(const int newLastId)
{
    m_lastId = qMax(m_lastId, newLastId);
}

int Scene::nextId()
{
    return ++m_lastId;
}

void Scene::updateItemId(ItemWithId *item, const int newId)
{
    // Called before addItem() to pre-assign a specific ID (undo/redo restore path).
    // The item is not yet in the registry; addItem() will preserve this positive ID.
    item->setId(newId);
    setLastId(newId);
}

void Scene::registerItem(ItemWithId *item)
{
    if (!item) {
        return;
    }
    m_elementRegistry[item->id()] = item;
}

void Scene::unregisterItem(ItemWithId *item)
{
    if (!item) {
        return;
    }
    m_elementRegistry.remove(item->id());
}

void Scene::checkUpdateRequest()
{
    // Coalesces multiple rapid undo-stack changes into a single autosave signal:
    // the flag is set by setCircuitUpdateRequired() and cleared here after emitting.
    if (m_autosaveRequired) {
        emit circuitHasChanged();
        m_autosaveRequired = false;
    }
}

void Scene::drawBackground(QPainter *painter, const QRectF &rect)
{
    // m11() is the X-axis scale factor of the view transform; below 0.3 the grid dots
    // would be sub-pixel and invisible anyway, so skip the per-pixel loop for performance
    if (view() and view()->transform().m11() < 0.3) {
        return;
    }

    painter->setRenderHint(QPainter::Antialiasing, true);
    QGraphicsScene::drawBackground(painter, rect);

    const int left = static_cast<int>(rect.left()) - (static_cast<int>(rect.left()) % gridSize);
    const int top = static_cast<int>(rect.top()) - (static_cast<int>(rect.top()) % gridSize);
    painter->setPen(m_dots);

    // TODO: replace this with a QPixmap for better performance
    for (int x = left; x < rect.right(); x += gridSize) {
        for (int y = top; y < rect.bottom(); y += gridSize) {
            painter->drawPoint(x, y);
        }
    }
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
    // Re-applying visibility ensures newly-added ports/wires respect the current
    // show/hide state; without this, ports on fresh elements would always appear visible
    showWires(m_showWires);
    showGates(m_showGates);

    update();

    // Re-initialize topological sort and simulation graph after any structural change
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

    return sortByTopology(elements_);
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

    return sortByTopology(elements_);
}

QVector<GraphicElement *> Scene::sortByTopology(QVector<GraphicElement *> elements)
{
    QHash<GraphicElement *, QVector<GraphicElement *>> successors;
    for (auto *elm : elements) {
        for (auto *port : elm->outputs()) {
            for (auto *conn : port->connections()) {
                if (auto *endPort = conn->endPort()) {
                    if (auto *successor = endPort->graphicElement()) {
                        if (!successors[elm].contains(successor)) {
                            successors[elm].append(successor);
                        }
                    }
                }
            }
        }
    }

    QHash<GraphicElement *, int> priorities;
    calculatePriorities(elements, successors, priorities);

    std::stable_sort(elements.begin(), elements.end(), [&priorities](const auto &e1, const auto &e2) {
        return priorities.value(e1) > priorities.value(e2);
    });

    return elements;
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
    // Also check a small surrounding rectangle so port hit-testing works when
    // the cursor is near but not exactly on the port's bounding box
    items_.append(itemsAt(pos));

    // Ports get priority so wire dragging begins/ends on the port, not the element body
    for (auto *item : std::as_const(items_)) {
        if (item->type() == QNEPort::Type) {
            return item;
        }
    }

    // Return any custom item (UserType < type); ignores built-in Qt items
    for (auto *item : std::as_const(items_)) {
        if (item->type() > QGraphicsItem::UserType) {
            return item;
        }
    }

    return nullptr;
}

QList<QGraphicsItem *> Scene::itemsAt(const QPointF pos)
{
    // 9×9 pixel hit area (4px margin around the exact point) compensates for the
    // small visual size of ports and makes them easier to click precisely
    QRectF rect(pos - QPointF(4, 4), QSize(9, 9));
    return items(rect.normalized());
}

void Scene::receiveCommand(QUndoCommand *cmd)
{
    m_undoStack.push(cmd);
    update();
}

void Scene::resizeScene()
{
    setSceneRect(itemsBoundingRect());

    // if (auto *item = itemAt(m_mousePos); item && (m_timer.elapsed() > 100) && m_draggingElement) {
    //     // FIXME: sometimes this goes into a infinite loop and crashes
    //     item->ensureVisible();
    //     m_timer.restart();
    // }
}

QNEConnection *Scene::editedConnection() const
{
    return dynamic_cast<QNEConnection *>(itemById(m_editedConnectionId));
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
    connection->setEndPort(endPort);
    connection->setStartPos(m_mousePos);

    addItem(connection);
    setEditedConnection(connection);
    connection->updatePath();
}

void Scene::startNewConnection(QNEOutputPort *startPort)
{
    auto *connection = new QNEConnection();
    connection->setStartPort(startPort);
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
    auto *port = qgraphicsitem_cast<QNEPort *>(itemAt(m_mousePos));

    if (!port || !connection) {
        return;
    }

    /* The mouse is released over a QNEPort. */
    QNEOutputPort *startPort = nullptr;
    QNEInputPort *endPort = nullptr;

    // A wire being dragged from an output needs the drop target to be an input, and vice versa.
    // The dynamic_cast returns nullptr if the port type doesn't match, which is caught below.
    if (connection->startPort() != nullptr) {
        startPort = connection->startPort();
        endPort = dynamic_cast<QNEInputPort *>(port);
    } else if (connection->endPort() != nullptr) {
        startPort = dynamic_cast<QNEOutputPort *>(port);
        endPort = connection->endPort();
    }

    if (!startPort || !endPort) {
        return;
    }

    /* Verifying if the connection is valid. */
    // Self-loops (same element on both ends) and duplicate connections are forbidden
    if ((startPort->graphicElement() != endPort->graphicElement()) && !startPort->isConnected(endPort)) {
        /* Making connection. */
        connection->setStartPort(startPort);
        connection->setEndPort(endPort);
        receiveCommand(new AddItemsCommand({connection}, this));
        setEditedConnection(nullptr);
    } else {
        deleteEditedConnection();
    }
}

void Scene::detachConnection(QNEInputPort *endPort)
{
    const auto connections = endPort->connections();
    if (connections.isEmpty()) {
        return;
    }
    // Take the last connection — an input port normally has at most one, but
    // .last() is the safe choice if the model ever allows multiple
    auto *connection = connections.last();

    if (auto *startPort = connection->startPort()) {
        // Delete the existing wire, then immediately start a new in-progress wire
        // anchored to the same output port, so the user can re-route it
        receiveCommand(new DeleteItemsCommand({connection}, this));
        startNewConnection(startPort);
    }
}

void Scene::prevMainPropShortcut()
{
    // Keyboard shortcut to decrement the "primary" configurable property of each
    // selected element: input count for logic gates, output count for rotary inputs,
    // clock frequency (step 0.5 Hz), buzzer audio, or display color
    for (auto *element : selectedElements()) {
        switch (element->elementType()) {
        // Logic Elements
        case ElementType::And:
        case ElementType::Or:
        case ElementType::Nand:
        case ElementType::Nor:
        case ElementType::Xor:
        case ElementType::Xnor:
        // Output and truthtable
        case ElementType::Led:
        case ElementType::TruthTable:
            if (element->inputSize() > element->minInputSize())
                receiveCommand(new ChangeInputSizeCommand(QList<GraphicElement *>{element},
                                                          element->inputSize() - 1, this));
            break;

        // Input ports
        case ElementType::InputRotary:
            if (element->outputSize() > element->minOutputSize())
                receiveCommand(new ChangeOutputSizeCommand(QList<GraphicElement *>{element},
                                                           element->outputSize() - 1, this));
            break;

        case ElementType::Clock:
            if (element->hasFrequency())
                element->setFrequency(element->frequency() - 0.5f);
            break;

        case ElementType::Buzzer:
            if (element->hasAudio())
                element->setAudio(element->previousAudio());
            break;

        case ElementType::Display16:
        case ElementType::Display14:
        case ElementType::Display7:
            if (element->hasColors())
                element->setColor(element->previousColor());
            break;

        default: // Not implemented
            break;
        }

        // Toggling selection off and on forces the property inspector to refresh
        element->setSelected(false);
        element->setSelected(true);
    }
}

void Scene::nextMainPropShortcut()
{
    // Mirror of prevMainPropShortcut() — increments the same per-type primary property
    for (auto *element : selectedElements()) {
        switch (element->elementType()) {
        // Logic Elements
        case ElementType::And:
        case ElementType::Or:
        case ElementType::Nand:
        case ElementType::Nor:
        case ElementType::Xor:
        case ElementType::Xnor:
        // Output and truthtable
        case ElementType::Led:
        case ElementType::TruthTable:
            if (element->inputSize() < element->maxInputSize())
                receiveCommand(new ChangeInputSizeCommand(QList<GraphicElement *>{element},
                                                          element->inputSize() + 1, this));
            break;

        // Input ports
        case ElementType::InputRotary:
            if (element->outputSize() < element->maxOutputSize())
                receiveCommand(new ChangeOutputSizeCommand(QList<GraphicElement *>{element},
                                                           element->outputSize() + 1, this));
            break;

        case ElementType::Clock:
            if (element->hasFrequency())
                element->setFrequency(element->frequency() + 0.5f);
            break;

        case ElementType::Buzzer:
            if (element->hasAudio())
                element->setAudio(element->nextAudio());
            break;

        case ElementType::Display14:
        case ElementType::Display7:
            if (element->hasColors())
                element->setColor(element->nextColor());
            break;

        default: // Not implemented
            break;
        }

        element->setSelected(false);
        element->setSelected(true);
    }
}

void Scene::prevSecndPropShortcut()
{
    for (auto *element : selectedElements()) {
        switch (element->elementType()) {
        case ElementType::TruthTable:
            if (element->outputSize() > element->minOutputSize())
                receiveCommand(new ChangeOutputSizeCommand(QList<GraphicElement *>{element},
                                                           element->outputSize() - 1, this));
            break;

        case ElementType::Led:
            if (element->hasColors())
                element->setColor(element->previousColor());
            break;

        default:
            break;
        }

        element->setSelected(false);
        element->setSelected(true);
    }
}

void Scene::nextSecndPropShortcut()
{
    for (auto *element : selectedElements()) {
        switch (element->elementType()) {
        case ElementType::TruthTable:
            if (element->outputSize() < element->maxOutputSize())
                receiveCommand(new ChangeOutputSizeCommand(QList<GraphicElement *>{element},
                                                           element->outputSize() + 1, this));
            break;

        case ElementType::Led:
            if (element->hasColors())
                element->setColor(element->nextColor());
            break;

        default:
            break;
        }

        element->setSelected(false);
        element->setSelected(true);
    }
}

void Scene::nextElm()
{
    for (auto *element : selectedElements()) {
        const QPointF elmPosition = element->scenePos();
        auto nextType = Enums::nextElmType(element->elementType());

        // ElementType::Unknown signals there is no "next" in the cycle for this type
        if (nextType == ElementType::Unknown) { continue; }

        receiveCommand(new MorphCommand(QList<GraphicElement *>{element},
                       Enums::nextElmType(element->elementType()), this));

        // MorphCommand replaces the element in-place; re-select via position because
        // the old pointer is now invalid after the command's redo()
        auto *item = itemAt(elmPosition);
        if (item) {
            item->setSelected(true);
        }
    }
}

void Scene::prevElm()
{
    for (auto *element : selectedElements()) {
        const QPointF elmPosition = element->scenePos();
        auto prevType = Enums::prevElmType(element->elementType());

        if (prevType == ElementType::Unknown) { continue; }

        receiveCommand(new MorphCommand(QList<GraphicElement *>{element},
                                        Enums::prevElmType(element->elementType()), this));

        auto *item = itemAt(elmPosition);
        if (item) {
            item->setSelected(true);
        }
    }
}

void Scene::updateTheme()
{
    qCDebug(zero) << "Updating theme.";
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

    qCDebug(zero) << "Finished updating theme.";
}

QList<QGraphicsItem *> Scene::items(Qt::SortOrder order) const
{
    return QGraphicsScene::items(order);
}

QList<QGraphicsItem *> Scene::items(QPointF pos, Qt::ItemSelectionMode mode, Qt::SortOrder order, const QTransform &deviceTransform) const
{
    return QGraphicsScene::items(pos, mode, order, deviceTransform);
}

QList<QGraphicsItem *> Scene::items(const QRectF &rect, Qt::ItemSelectionMode mode, Qt::SortOrder order, const QTransform &deviceTransform) const
{
    return QGraphicsScene::items(rect, mode, order, deviceTransform);
}

void Scene::showGates(const bool checked)
{
    m_showGates = checked;
    const auto items_ = items();

    for (auto *item : items_) {
        if (item->type() == GraphicElement::Type) {
            auto *element = qgraphicsitem_cast<GraphicElement *>(item);
            if (!element) {
                continue;
            }
            const auto group = element->elementGroup();

            // Only hide/show internal logic gates; Input, Output and Other elements
            // (e.g., labels, ICs) are always kept visible regardless of this toggle
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

            // Node elements are purely wire-routing helpers with no logical function;
            // hiding wires should hide nodes too since they're meaningless without wires
            if (element->elementType() == ElementType::Node) {
                element->setVisible(checked);
            } else {
                // For other elements, hide only their port handles (the connectable dots),
                // not the element body itself, so the gate symbols remain visible
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
    qCDebug(zero) << "Ctrl + Drag action triggered.";
    const auto selectedElements_ = selectedElements();

    if (selectedElements_.isEmpty()) {
        return;
    }

    // --- Build drag pixmap ---
    // Temporarily hide non-selected items so the rendered image shows only
    // the selection, giving the drag ghost the correct visual appearance
    const auto items_ = items();

    for (auto *item : items_) {
        if (((item->type() == GraphicElement::Type) || (item->type() == QNEConnection::Type)) && !item->isSelected()) {
            item->hide();
        }
    }

    QRectF rect;

    for (auto *element : selectedElements_) {
        rect = rect.united(element->sceneBoundingRect());
    }

    // 8px padding avoids clipping port handles at the bounding-rect edges
    rect = rect.adjusted(-8, -8, 8, 8);

    auto mappedSize = m_view->transform().mapRect(rect).size().toSize();
    QImage image(mappedSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    // Opacity 0 makes the ghost transparent; the drag cursor shape still appears
    painter.setOpacity(0.0);
    QRectF target = image.rect();
    QRectF source = rect;
    render(&painter, target, source);

    // Restore hidden items before the drag begins so the scene looks normal
    for (auto *item : items_) {
        if (((item->type() == GraphicElement::Type) || (item->type() == QNEConnection::Type)) && !item->isSelected()) {
            item->show();
        }
    }

    // --- Serialize selection for drop target ---
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    // Embed the mouse-press position so the drop handler can compute the correct offset
    stream << mousePos;
    copy(selectedItems(), stream);

    auto *mimeData = new QMimeData();
    mimeData->setData("application/x-wiredpanda-cloneDrag", itemData);

    auto *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(QPixmap::fromImage(image));
    // Hot-spot aligns the drag image to the original element positions under the cursor
    QPointF offset = m_view->transform().map(mousePos - rect.topLeft());
    drag->setHotSpot(offset.toPoint());
    drag->exec(Qt::CopyAction, Qt::CopyAction);
}

void Scene::copy(const QList<QGraphicsItem *> &items, QDataStream &stream)
{
    // Compute the centroid of all selected elements (not connections) so that
    // paste() can place the clipboard contents relative to the cursor position
    QPointF center(0.0, 0.0);
    int itemsQuantity = 0;

    for (auto *item : items) {
        if (item->type() == GraphicElement::Type) {
            center += item->pos();
            ++itemsQuantity;
        }
    }

    stream << center / static_cast<qreal>(itemsQuantity);
    Serialization::serialize(items, stream);
}

void Scene::handleHoverPort()
{
    auto *port = qgraphicsitem_cast<QNEPort *>(itemAt(m_mousePos));
    auto *hoverPort_ = hoverPort();

    if (hoverPort_ && (hoverPort_ != port)) {
        releaseHoverPort();
    }

    if (port) {
        auto *editedConn = editedConnection();
        releaseHoverPort();
        setHoverPort(port);

        if (editedConn && editedConn->startPort() && (editedConn->startPort()->isOutput() == port->isOutput())) {
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

    // Store element ID + port index rather than raw pointers so the hover state
    // remains valid across undo/redo operations that may recreate the element
    if (hoverElm && this->contains(hoverElm->id())) {
        m_hoverPortElmId = hoverElm->id();

        // Encode inputs first (indices 0..inputSize-1), then outputs (inputSize..total-1)
        // so a single integer uniquely identifies any port on an element
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

    if (auto *hoverElm = dynamic_cast<GraphicElement *>(itemById(m_hoverPortElmId))) {
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
        // Right-clicking a selected item opens the property context menu for that selection
        if (selectedItems().contains(item)) {
            emit contextMenuPos(screenPos, item);
        } else if (item->type() == GraphicElement::Type) {
            // Right-clicking an unselected element clears the old selection and selects
            // only this item, then opens its context menu
            clearSelection();
            item->setSelected(true);
            emit contextMenuPos(screenPos, item);
        }
    } else {
        // Right-click on empty canvas: show a minimal paste-only context menu
        QMenu menu;
        auto *pasteAction = menu.addAction(QIcon(QPixmap(":/Interface/Toolbar/paste.svg")), tr("Paste"));
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
    Serialization::writePandaHeader(stream);
    copy(selectedItems(), stream);

    auto *mimeData = new QMimeData();
    mimeData->setData("application/x-wiredpanda-clipboard", itemData);

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
    Serialization::writePandaHeader(stream);
    cut(selectedItems(), stream);

    auto *mimeData = new QMimeData();
    mimeData->setData("application/x-wiredpanda-clipboard", itemData);

    QApplication::clipboard()->setMimeData(mimeData);
}

void Scene::pasteAction()
{
    const auto *mimeData = QApplication::clipboard()->mimeData();

    QByteArray itemData;

    if (mimeData->hasFormat("wpanda/copydata")) {
        itemData = mimeData->data("wpanda/copydata");
    }

    if (mimeData->hasFormat("application/x-wiredpanda-clipboard")) {
        itemData = mimeData->data("application/x-wiredpanda-clipboard");
    }

    if (!itemData.isEmpty()) {
        QDataStream stream(&itemData, QIODevice::ReadOnly);
        QVersionNumber version = Serialization::readPandaHeader(stream);
        paste(stream, version);
    }
}

void Scene::paste(QDataStream &stream, const QVersionNumber &version)
{
    clearSelection();

    QPointF center; stream >> center;

    QMap<quint64, QNEPort *> portMap;
    SerializationContext context{portMap, version, Serialization::contextDir};
    const auto itemList = Serialization::deserialize(stream, context);
    // Shift pasted elements so their centroid lands at the cursor position,
    // then nudge 32 px diagonally so repeated pastes are visually offset and
    // don't completely overlap the original selection.
    const QPointF offset = m_mousePos - center - QPointF(32.0, 32.0);

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
    // Clear selection before the command so that the scene's selectedItems()
    // list is empty during the command's redo() — avoids double-processing
    clearSelection();

    if (!selectedItems_.isEmpty()) {
        receiveCommand(new DeleteItemsCommand(selectedItems_, this));
        // Restart the simulation so it doesn't reference any deleted element pointers
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
            auto *buzzer = qobject_cast<Buzzer *>(element);
            if (buzzer) {
                buzzer->mute(mute);
            }
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

    if (formats.contains("wpanda/x-dnditemdata")
        || formats.contains("wpanda/ctrlDragData")
        || formats.contains("application/x-wiredpanda-dragdrop")
        || formats.contains("application/x-wiredpanda-cloneDrag")) {
        event->accept();
        return;
    }

    QGraphicsScene::dragEnterEvent(event);
}

void Scene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    const auto formats = event->mimeData()->formats();

    if (formats.contains("wpanda/x-dnditemdata")
        || formats.contains("wpanda/ctrlDragData")
        || formats.contains("application/x-wiredpanda-dragdrop")
        || formats.contains("application/x-wiredpanda-cloneDrag")) {
        event->accept();
        return;
    }

    QGraphicsScene::dragMoveEvent(event);
}

void Scene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    // --- New element drop from toolbox ---
    // Both MIME types carry the same payload; the newer format has a namespaced key
    if (event->mimeData()->hasFormat("wpanda/x-dnditemdata")
        || event->mimeData()->hasFormat("application/x-wiredpanda-dragdrop")) {
        QByteArray itemData;

        if (event->mimeData()->hasFormat("wpanda/x-dnditemdata")) {
            itemData = event->mimeData()->data("wpanda/x-dnditemdata");
        }

        if (event->mimeData()->hasFormat("application/x-wiredpanda-dragdrop")) {
            itemData = event->mimeData()->data("application/x-wiredpanda-dragdrop");
        }

        QDataStream stream(&itemData, QIODevice::ReadOnly);
        Serialization::readPandaHeader(stream);

        QPoint offset;      stream >> offset;
        ElementType type;   stream >> type;
        QString icFileName; stream >> icFileName;

        // Subtract the drag offset so the element lands under the original grab point
        QPointF pos = event->scenePos() - offset;
        qCDebug(zero) << type << " at position: " << pos.x() << ", " << pos.y() << ", label: " << icFileName;

        auto *element = ElementFactory::buildElement(type);
        qCDebug(zero) << "Valid element.";

        element->loadFromDrop(icFileName, Serialization::contextDir);

        qCDebug(zero) << "Adding the element to the scene.";
        receiveCommand(new AddItemsCommand({element}, this));

        qCDebug(zero) << "Cleaning the selection.";
        clearSelection();

        qCDebug(zero) << "Setting created element as selected.";
        element->setSelected(true);

        qCDebug(zero) << "Adjusting the position of the element.";
        element->setPos(pos);
    }

    // --- Clone drag (Ctrl+drag of existing selection) ---
    if (event->mimeData()->hasFormat("wpanda/ctrlDragData")
        || event->mimeData()->hasFormat("application/x-wiredpanda-cloneDrag")) {
        QByteArray itemData;

        if (event->mimeData()->hasFormat("wpanda/ctrlDragData")) {
            itemData = event->mimeData()->data("wpanda/ctrlDragData");
        }

        if (event->mimeData()->hasFormat("application/x-wiredpanda-cloneDrag")) {
            itemData = event->mimeData()->data("application/x-wiredpanda-cloneDrag");
        }

        QDataStream stream(&itemData, QIODevice::ReadOnly);
        QVersionNumber version = Serialization::readPandaHeader(stream);

        // offset = mouse position at drag-start; recompute drop offset from current position
        QPointF offset; stream >> offset;
        QPointF ctr;    stream >> ctr;
        offset = event->scenePos() - offset;

        QMap<quint64, QNEPort *> portMap;
        SerializationContext context{portMap, version, Serialization::contextDir};
        const auto itemList = Serialization::deserialize(stream, context);

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
    // Skip keyboard triggers while Ctrl is held to avoid firing during Ctrl+Z/C/V shortcuts
    if (!(event->modifiers().testFlag(Qt::ControlModifier))) {
        for (auto *element : elements()) {
            if (element->hasTrigger() && !element->trigger().isEmpty() && element->trigger().matches(event->key())) {
                if (auto *input = qobject_cast<GraphicElementInput *>(element); input && !input->isLocked()) {
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
                // Only InputButton (momentary) is released on key-up; InputSwitch stays latched
                if (auto *input = qobject_cast<GraphicElementInput *>(element); input && !input->isLocked() && (element->elementType() == ElementType::InputButton)) {
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
        // Qt's default QGraphicsScene selection behaviour treats Shift+click as
        // "extend selection to range" (like a list widget), which is meaningless for
        // a free-form canvas.  Remapping Shift→Ctrl activates the Ctrl toggle-select
        // path instead, which is what users expect from a design tool.
        if (mouseEvent->modifiers().testFlag(Qt::ShiftModifier)) {
            mouseEvent->setModifiers(Qt::ControlModifier);
            return QGraphicsScene::eventFilter(watched, event);
        }

        // Intercept Ctrl+Left-click on an element to begin a clone-drag instead of
        // the default rubber-band selection; return true to swallow the event
        if ((mouseEvent->button() == Qt::LeftButton) && mouseEvent->modifiers().testFlag(Qt::ControlModifier)) {
            if (auto *item = itemAt(mouseEvent->scenePos()); item && ((item->type() == GraphicElement::Type) || (item->type() == QNEConnection::Type))) {
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
        // --- Element selection and drag preparation ---
        if (event->button() == Qt::LeftButton) {
            if (event->modifiers().testFlag(Qt::ControlModifier)) {
                // Ctrl+click toggles individual item membership in the selection
                item->setSelected(!item->isSelected());
            }

            auto selectedElements_ = selectedElements();

            // Include the clicked element even if it isn't yet selected, so a
            // single-click drag of an unselected element works immediately
            if (auto *element = qgraphicsitem_cast<GraphicElement *>(item)) {
                selectedElements_ << element;
            }

            m_draggingElement = ((item->type() == GraphicElement::Type) && !selectedElements_.isEmpty());

            // Snapshot positions now; MoveCommand compares these against release-time positions
            m_movedElements.clear();
            m_oldPositions.clear();

            for (auto *element : std::as_const(selectedElements_)) {
                m_movedElements.append(element);
                m_oldPositions.append(element->pos());
            }
        }

        // --- Wire connection handling ---
        if (item->type() == QNEPort::Type) {
            /* When the mouse is pressed over an connected input port, the line
             * is disconnected and can be connected to another port. */
            if (auto *connection = editedConnection()) {
                // An in-progress wire exists; try to complete it at this port
                makeConnection(connection);
                return;
            }

            auto *pressedPort = qgraphicsitem_cast<QNEPort *>(item);

            if (auto *startPort = dynamic_cast<QNEOutputPort *>(pressedPort)) {
                startNewConnection(startPort);
                return;
            }

            if (auto *endPort = dynamic_cast<QNEInputPort *>(pressedPort)) {
                // Empty input port: begin a new wire; occupied port: detach the existing wire
                endPort->connections().isEmpty() ? startNewConnection(endPort) : detachConnection(endPort);
                return;
            }
        }
    }

    // Clicking on empty space while a wire is being drawn cancels it
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

    // Expand scene rect while dragging so elements can be moved beyond the current boundary
    if (m_draggingElement) {
        resizeScene();
    }

    // --- In-progress wire routing ---
    if (auto *connection = editedConnection()) {
        if (connection->startPort()) {
            // Wire anchored at start: free end follows the mouse
            connection->setEndPos(m_mousePos);
            connection->updatePath();
            return;
        }

        if (connection->endPort()) {
            // Wire anchored at end (dragged from input): free start follows the mouse
            connection->setStartPos(m_mousePos);
            connection->updatePath();
            return;
        }

        // Connection lost both ports (e.g., element deleted mid-drag) — clean up
        deleteEditedConnection();
        return;
    }

    // --- Rubber-band selection rectangle ---
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
            // Only push a MoveCommand if at least one element actually changed position;
            // avoids polluting the undo stack with no-op moves (e.g., click without drag)
            const bool valid = std::any_of(m_movedElements.cbegin(), m_movedElements.cend(), [this](auto *elm) {
                return (elm->pos() != m_oldPositions.at(m_movedElements.indexOf(elm)));
            });

            if (valid) {
                receiveCommand(new MoveCommand(m_movedElements, m_oldPositions, this));
            }
        }

        m_draggingElement = false;
        m_movedElements.clear();
    }

    m_selectionRect.hide();
    m_markingSelectionBox = false;

    // Complete an in-progress wire on mouse release (when no button is held)
    // — this handles the drag-to-connect gesture (press output → drag → release on input)
    if (auto *connection = editedConnection(); connection && (event->buttons() == Qt::NoButton)) {
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

    // Double-click on a fully connected wire inserts a routing node at the click point,
    // splitting the wire into two segments; guard ensures it's not a dangling wire
    if (auto *connection = qgraphicsitem_cast<QNEConnection *>(itemAt(m_mousePos)); connection && connection->startPort() && connection->endPort()) {
        receiveCommand(new SplitCommand(connection, m_mousePos, this));
    }

    QGraphicsScene::mouseDoubleClickEvent(event);
}

void Scene::addItem(QMimeData *mimeData)
{
    QByteArray itemData = mimeData->data("wpanda/x-dnditemdata");
    QDataStream stream(&itemData, QIODevice::ReadOnly);
    Serialization::readPandaHeader(stream);

    QPoint offset;      stream >> offset;
    ElementType type;   stream >> type;
    QString icFileName; stream >> icFileName;

    auto *element = ElementFactory::buildElement(type);
    qCDebug(zero) << "Valid element.";

    element->loadFromDrop(icFileName, Serialization::contextDir);

    qCDebug(zero) << "Adding the element to the scene.";
    receiveCommand(new AddItemsCommand({element}, this));

    qCDebug(zero) << "Cleaning the selection.";
    clearSelection();

    qCDebug(zero) << "Setting created element as selected.";
    element->setSelected(true);

    mimeData->deleteLater();
}
