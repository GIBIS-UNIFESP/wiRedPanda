// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/Scene.h"

#include <algorithm>
#include <cmath>

#include <QClipboard>
#include <QDrag>
#include <QGraphicsSceneDragDropEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QScrollBar>

#include "App/Core/Common.h"
#include "App/Core/ItemWithId.h"
#include "App/Core/MimeTypes.h"
#include "App/Core/Priorities.h"
#include "App/Core/SentryHelpers.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/GraphicElements/Buzzer.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Commands.h"
#include "App/Scene/GraphicsView.h"

QString Scene::resolveContextDir(const QGraphicsItem *item)
{
    if (auto *s = dynamic_cast<Scene *>(item->scene())) {
        return s->contextDir();
    }
    // Element not yet added to a scene (mid-deserialization): use the
    // contextDir stored on the element during load().
    if (auto *ge = dynamic_cast<const GraphicElement *>(item)) {
        return ge->loadContextDir();
    }
    return {};
}

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

    m_undoAction = new QAction(tr("&Undo"), this);
    m_undoAction->setEnabled(false);
    m_undoAction->setIcon(QIcon(":/Interface/Toolbar/undo.svg"));
    m_undoAction->setShortcut(QKeySequence::Undo);
    connect(&m_undoStack, &QUndoStack::canUndoChanged, m_undoAction, &QAction::setEnabled);
    connect(&m_undoStack, &QUndoStack::undoTextChanged, this, &Scene::updateUndoText);
    connect(m_undoAction, &QAction::triggered, &m_undoStack, &QUndoStack::undo);

    m_redoAction = new QAction(tr("&Redo"), this);
    m_redoAction->setEnabled(false);
    m_redoAction->setIcon(QIcon(":/Interface/Toolbar/redo.svg"));
    m_redoAction->setShortcut(QKeySequence::Redo);
    connect(&m_undoStack, &QUndoStack::canRedoChanged, m_redoAction, &QAction::setEnabled);
    connect(&m_undoStack, &QUndoStack::redoTextChanged, this, &Scene::updateRedoText);
    connect(m_redoAction, &QAction::triggered, &m_undoStack, &QUndoStack::redo);

    // Used to throttle expensive operations during drag (e.g., ensureVisible)
    m_timer.start();

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &Scene::updateTheme);
    // Emit autosave signal only after each undo-stack index change (not on every internal state update)
    connect(&m_undoStack,              &QUndoStack::indexChanged,   this, &Scene::checkUpdateRequest);
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

    // Register element-type-specific hooks
    if (item->type() == GraphicElement::Type) {
        auto *elm = qgraphicsitem_cast<GraphicElement *>(item);
        if (!elm) {
            return;
        }
        if (elm->elementType() == ElementType::IC) {
            auto *ic = static_cast<IC *>(elm);
            if (!ic->file().isEmpty()) {
                m_icRegistry.watchFile(ic->file());
            }
            connect(ic, &IC::requestOpenSubCircuit, this, &Scene::icOpenRequested);
        } else if (elm->elementType() == ElementType::TruthTable) {
            auto *tt = static_cast<TruthTable *>(elm);
            connect(tt, &TruthTable::requestOpenTruthTableEditor, this, &Scene::openTruthTableRequested);
        }
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

SerializationContext Scene::deserializationContext(QMap<quint64, QNEPort *> &portMap, const QVersionNumber &version)
{
    SerializationContext context{portMap, version, contextDir()};
    context.blobRegistry = &m_icRegistry.blobMapRef();
    return context;
}

void Scene::drawBackground(QPainter *painter, const QRectF &rect)
{
    // m11() is the X-axis scale factor of the view transform; below 0.3 the grid dots
    // would be sub-pixel and invisible anyway, so skip drawing for performance
    if (view() and view()->transform().m11() < 0.3) {
        return;
    }

    QGraphicsScene::drawBackground(painter, rect);

    const int left = static_cast<int>(std::floor(rect.left() / gridSize)) * gridSize;
    const int top = static_cast<int>(std::floor(rect.top() / gridSize)) * gridSize;
    const int right = static_cast<int>(std::ceil(rect.right() / gridSize)) * gridSize;
    const int bottom = static_cast<int>(std::ceil(rect.bottom() / gridSize)) * gridSize;

    painter->setPen(m_dots);

    QVector<QPoint> points;
    points.reserve(((right - left) / gridSize + 1) * ((bottom - top) / gridSize + 1));

    for (int x = left; x <= right; x += gridSize) {
        for (int y = top; y <= bottom; y += gridSize) {
            points.append(QPoint(x, y));
        }
    }

    painter->drawPoints(points.data(), static_cast<int>(points.size()));
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
    m_visibilityManager.reapply();

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

QHash<QString, QNEInputPort *> Scene::wirelessTxInputPorts(const QVector<GraphicElement *> &elements)
{
    QHash<QString, QNEInputPort *> txMap;
    for (auto *elm : elements) {
        if (elm->wirelessMode() == WirelessMode::Tx && !elm->label().isEmpty() && elm->inputPort(0)) {
            if (!txMap.contains(elm->label())) {
                txMap.insert(elm->label(), elm->inputPort(0));
            }
        }
    }
    return txMap;
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

    // Elements take priority over connections since they render above wires
    for (auto *item : std::as_const(items_)) {
        if (item->type() == GraphicElement::Type) {
            return item;
        }
    }

    // Return any remaining custom item (connections, etc.)
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
    sentryBreadcrumb("command", QStringLiteral("Command: %1").arg(cmd->text()));
    m_undoStack.push(cmd);
    update();
}

void Scene::resizeScene()
{
    const auto bounds = itemsBoundingRect();

    if (m_draggingElement) {
        // While dragging, only expand the scene rect (union with current rect).
        // Never shrink during drag — shrinking shifts the viewport origin and
        // causes jarring visual jumps as the scene rect chases the items.
        setSceneRect(sceneRect().united(bounds));
    } else {
        // Tighten to item bounds, but ensure the scene rect stays larger than the
        // viewport. When the scene rect is smaller than (or barely larger than) the
        // viewport, Qt re-centers it, causing a visual jump. Adding margins ensures
        // enough scrollbar range to preserve the exact scroll position.
        auto tightRect = bounds;
        const auto viewList = views();
        if (!viewList.isEmpty()) {
            auto *view = viewList.first();
            constexpr qreal margin = 100.0;
            const auto visibleScene = view->mapToScene(view->viewport()->rect()).boundingRect()
                                         .adjusted(-margin, -margin, margin, margin);
            tightRect = tightRect.united(visibleScene);

            // Preserve exact scrollbar positions across the scene rect change.
            // centerOn() converts scene floats to integer pixels, causing 1px
            // drift — saving/restoring scrollbar values avoids the rounding.
            const int hVal = view->horizontalScrollBar()->value();
            const int vVal = view->verticalScrollBar()->value();
            setSceneRect(tightRect);
            view->horizontalScrollBar()->setValue(hVal);
            view->verticalScrollBar()->setValue(vVal);
        } else {
            setSceneRect(tightRect);
        }
    }
}

QUndoStack *Scene::undoStack()
{
    return &m_undoStack;
}

bool Scene::isConnectionAllowed(QNEOutputPort *startPort, QNEInputPort *endPort)
{
    return ConnectionManager::isConnectionAllowed(startPort, endPort);
}

void Scene::prevMainPropShortcut() { m_propertyShortcutHandler.prevMainProperty(); }

void Scene::nextMainPropShortcut() { m_propertyShortcutHandler.nextMainProperty(); }

void Scene::prevSecndPropShortcut() { m_propertyShortcutHandler.prevSecondaryProperty(); }

void Scene::nextSecndPropShortcut() { m_propertyShortcutHandler.nextSecondaryProperty(); }

void Scene::nextElm() { m_propertyShortcutHandler.nextElement(); }

void Scene::prevElm() { m_propertyShortcutHandler.prevElement(); }

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
    m_visibilityManager.showGates(checked);
}

void Scene::showWires(const bool checked)
{
    m_visibilityManager.showWires(checked);
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

void Scene::retranslateUi()
{
    updateUndoText(m_undoStack.undoText());
    updateRedoText(m_undoStack.redoText());
}

void Scene::updateUndoText(const QString &text)
{
    const QString prefix = tr("&Undo");
    m_undoAction->setText(text.isEmpty() ? prefix : prefix + QLatin1Char(' ') + text);
}

void Scene::updateRedoText(const QString &text)
{
    const QString prefix = tr("&Redo");
    m_redoAction->setText(text.isEmpty() ? prefix : prefix + QLatin1Char(' ') + text);
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

        if (mimeData->hasFormat(MimeType::ClipboardLegacy)) {
            connect(pasteAction, &QAction::triggered, this, &Scene::pasteAction);
        } else {
            pasteAction->setEnabled(false);
        }

        menu.exec(screenPos);
    }
}

void Scene::copyAction()
{
    sentryBreadcrumb("clipboard", QStringLiteral("Copy"));
    m_clipboardManager.copy();
}

void Scene::cutAction()
{
    sentryBreadcrumb("clipboard", QStringLiteral("Cut"));
    m_clipboardManager.cut();
}

void Scene::pasteAction()
{
    sentryBreadcrumb("clipboard", QStringLiteral("Paste"));
    m_clipboardManager.paste();
}

void Scene::deleteAction()
{
    sentryBreadcrumb("ui", QStringLiteral("Delete"));
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
    sentryBreadcrumb("ui", QStringLiteral("Rotate right"));
    rotate(90);
}

void Scene::rotateLeft()
{
    sentryBreadcrumb("ui", QStringLiteral("Rotate left"));
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
    sentryBreadcrumb("ui", QStringLiteral("Flip horizontal"));
    const auto elements_ = selectedElements();

    if (!elements_.isEmpty()) {
        receiveCommand(new FlipCommand(elements_, 0, this));
    }
}

void Scene::flipVertically()
{
    sentryBreadcrumb("ui", QStringLiteral("Flip vertical"));
    const auto elements_ = selectedElements();

    if (!elements_.isEmpty()) {
        receiveCommand(new FlipCommand(elements_, 1, this));
    }
}

bool Scene::isSupportedDropFormat(const QMimeData *mimeData)
{
    const auto &formats = mimeData->formats();
    return formats.contains(MimeType::DragDropLegacy)
           || formats.contains(MimeType::CloneDragLegacy)
           || formats.contains(MimeType::DragDrop)
           || formats.contains(MimeType::CloneDrag);
}

void Scene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (isSupportedDropFormat(event->mimeData())) {
        event->accept();
        return;
    }

    QGraphicsScene::dragEnterEvent(event);
}

void Scene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    if (isSupportedDropFormat(event->mimeData())) {
        event->accept();
        return;
    }

    QGraphicsScene::dragMoveEvent(event);
}

void Scene::handleNewElementDrop(QGraphicsSceneDragDropEvent *event)
{
    // Both MIME types carry the same payload; the newer format has a namespaced key
    QByteArray itemData;

    if (event->mimeData()->hasFormat(MimeType::DragDropLegacy)) {
        itemData = event->mimeData()->data(MimeType::DragDropLegacy);
    }

    if (event->mimeData()->hasFormat(MimeType::DragDrop)) {
        itemData = event->mimeData()->data(MimeType::DragDrop);
    }

    QDataStream stream(&itemData, QIODevice::ReadOnly);
    Serialization::readPandaHeader(stream);

    QPoint offset;      stream >> offset;
    ElementType type;   stream >> type;
    QString icFileName; stream >> icFileName;

    bool isEmbedded = false;
    QString blobName;
    if (!stream.atEnd()) { stream >> isEmbedded; }
    if (!stream.atEnd()) { stream >> blobName; }

    // Subtract the drag offset so the element lands under the original grab point
    QPointF pos = event->scenePos() - offset;
    qCDebug(zero) << type << " at position: " << pos.x() << ", " << pos.y() << ", label: " << icFileName;

    auto *element = ElementFactory::buildElement(type);
    qCDebug(zero) << "Valid element.";

    if (isEmbedded && type == ElementType::IC) {
        if (!m_icRegistry.initEmbeddedIC(static_cast<IC *>(element), blobName)) {
            delete element;
            return;
        }
    } else {
        element->loadFromDrop(icFileName, contextDir());
    }

    qCDebug(zero) << "Adding the element to the scene.";
    receiveCommand(new AddItemsCommand({element}, this));

    qCDebug(zero) << "Cleaning the selection.";
    clearSelection();

    qCDebug(zero) << "Setting created element as selected.";
    element->setSelected(true);

    qCDebug(zero) << "Adjusting the position of the element.";
    element->setPos(pos);
}

void Scene::handleCloneDrag(QGraphicsSceneDragDropEvent *event)
{
    QByteArray itemData;

    if (event->mimeData()->hasFormat(MimeType::CloneDragLegacy)) {
        itemData = event->mimeData()->data(MimeType::CloneDragLegacy);
    }

    if (event->mimeData()->hasFormat(MimeType::CloneDrag)) {
        itemData = event->mimeData()->data(MimeType::CloneDrag);
    }

    QDataStream stream(&itemData, QIODevice::ReadOnly);
    QVersionNumber version = Serialization::readPandaHeader(stream);

    // offset = mouse position at drag-start; recompute drop offset from current position
    QPointF offset; stream >> offset;
    QPointF ctr;    stream >> ctr;
    offset = event->scenePos() - offset;

    QMap<quint64, QNEPort *> portMap;
    auto context = deserializationContext(portMap, version);
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

void Scene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    sentryBreadcrumb("ui", QStringLiteral("Drop event"));

    if (event->mimeData()->hasFormat(MimeType::DragDropLegacy)
        || event->mimeData()->hasFormat(MimeType::DragDrop)) {
        handleNewElementDrop(event);
    }

    if (event->mimeData()->hasFormat(MimeType::CloneDragLegacy)
        || event->mimeData()->hasFormat(MimeType::CloneDrag)) {
        handleCloneDrag(event);
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
                m_clipboardManager.cloneDrag(mouseEvent->scenePos());
                return true;
            }
        }
    }

    return QGraphicsScene::eventFilter(watched, event);
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_mousePos = event->scenePos();
    m_connectionManager.updateHover(m_mousePos);

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
            if (m_draggingElement) {
                sentryBreadcrumb("ui", QStringLiteral("Drag started: %1 element(s)").arg(selectedElements_.size()));
            }

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
            if (m_connectionManager.hasEditedConnection()) {
                // An in-progress wire exists; try to complete it at this port
                m_connectionManager.tryComplete(m_mousePos);
                return;
            }

            auto *pressedPort = qgraphicsitem_cast<QNEPort *>(item);

            if (auto *startPort = dynamic_cast<QNEOutputPort *>(pressedPort)) {
                m_connectionManager.startFromOutput(startPort);
                return;
            }

            if (auto *endPort = dynamic_cast<QNEInputPort *>(pressedPort)) {
                // Empty input port: begin a new wire; occupied port: detach the existing wire
                endPort->connections().isEmpty() ? m_connectionManager.startFromInput(endPort) : m_connectionManager.detach(endPort);
                return;
            }
        }
    }

    // Clicking on empty space while a wire is being drawn cancels it
        m_connectionManager.cancel();

    if (!item && (event->button() == Qt::LeftButton)) {
        startSelectionRect();
    }

    if (event->button() == Qt::RightButton) {
        contextMenu(event->screenPos());
        return;
    }

    QGraphicsScene::mousePressEvent(event);
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // Guard against re-entrant calls. When ensureVisible() scrolls the viewport,
    // Qt synchronously calls scrollContentsBy() → replayLastMouseEvent() →
    // mouseMoveEvent() before ensureVisible() returns. At a viewport corner,
    // scrolling to satisfy the H-margin violates the V-margin, so the replay
    // triggers another ensureVisible() that does the opposite, oscillating
    // until a stack overflow. Dropping the re-entrant call breaks the loop.
    if (m_handlingMouseMove) {
        return;
    }
    m_handlingMouseMove = true;
    const auto resetFlag = qScopeGuard([this] { m_handlingMouseMove = false; });

    m_mousePos = event->scenePos();
    m_connectionManager.updateHover(m_mousePos);

    // Expand scene rect while dragging so elements can be moved beyond the current boundary,
    // and auto-scroll the viewport to follow the cursor.
    if (m_draggingElement) {
        resizeScene();

        if (m_timer.elapsed() > 50) {
            const auto viewList = views();
            if (!viewList.isEmpty()) {
                viewList.first()->ensureVisible(m_mousePos.x(), m_mousePos.y(), 1, 1, 50, 50);
            }
            m_timer.restart();
        }
    }

    // --- In-progress wire routing ---
    if (m_connectionManager.hasEditedConnection()) {
        m_connectionManager.updateEditedEnd(m_mousePos);
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
        bool moved = false;

        if (!m_movedElements.empty()) {
            // Only push a MoveCommand if at least one element actually changed position;
            // avoids polluting the undo stack with no-op moves (e.g., click without drag)
            moved = std::any_of(m_movedElements.cbegin(), m_movedElements.cend(), [this](auto *elm) {
                return (elm->pos() != m_oldPositions.at(m_movedElements.indexOf(elm)));
            });

            if (moved) {
                receiveCommand(new MoveCommand(m_movedElements, m_oldPositions, this));
            }
        }

        sentryBreadcrumb("ui", moved ? QStringLiteral("Drag ended: moved") : QStringLiteral("Drag ended: no move"));
        m_draggingElement = false;
        m_movedElements.clear();

        // Only tighten scene rect after an actual drag; a click-without-move
        // should not trigger a rect change that could shift the viewport.
        if (moved) {
            resizeScene();
        }
    }

    m_selectionRect.hide();
    m_markingSelectionBox = false;

    // Complete an in-progress wire on mouse release (when no button is held)
    // — this handles the drag-to-connect gesture (press output → drag → release on input)
    if (m_connectionManager.hasEditedConnection() && (event->buttons() == Qt::NoButton)) {
        m_connectionManager.tryComplete(m_mousePos);
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
    QByteArray itemData = mimeData->hasFormat(MimeType::DragDrop)
        ? mimeData->data(MimeType::DragDrop)
        : mimeData->data(MimeType::DragDropLegacy);
    QDataStream stream(&itemData, QIODevice::ReadOnly);
    Serialization::readPandaHeader(stream);

    QPoint offset;      stream >> offset;
    ElementType type;   stream >> type;
    QString icFileName; stream >> icFileName;

    bool isEmbedded = false;
    QString blobName;
    if (!stream.atEnd()) { stream >> isEmbedded; }
    if (!stream.atEnd()) { stream >> blobName; }

    auto *element = ElementFactory::buildElement(type);
    qCDebug(zero) << "Valid element.";

    if (isEmbedded && type == ElementType::IC) {
        if (!m_icRegistry.initEmbeddedIC(static_cast<IC *>(element), blobName)) {
            delete element;
            return;
        }
    } else {
        element->loadFromDrop(icFileName, contextDir());
    }

    qCDebug(zero) << "Adding the element to the scene.";
    receiveCommand(new AddItemsCommand({element}, this));

    qCDebug(zero) << "Cleaning the selection.";
    clearSelection();

    qCDebug(zero) << "Setting created element as selected.";
    element->setSelected(true);

    mimeData->deleteLater();
}

