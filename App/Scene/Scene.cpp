// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/Scene.h"

#include <algorithm>
#include <cmath>

#include <QClipboard>
#include <QDrag>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneHelpEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QPolygon>
#include <QScrollBar>

#include "App/Core/Common.h"
#include "App/Core/Constants.h"
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
#include "App/Wiring/Connection.h"
#include "App/Scene/Commands.h"
#include "App/Scene/GraphicsView.h"

Scene::Scene(QObject *parent)
    : QGraphicsScene(parent)
    , m_simulation(this, this)
{
    // The scene filters its own events to intercept mouse events before items see them
    // (e.g., to detect Ctrl+drag for cloning before Qt's default drag-selection activates)
    installEventFilter(this);

    // Attach the interaction layer: adds its rubber-band selection rectangle to the
    // scene and starts its drag throttle timer.
    m_interaction.attachToScene();

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
        if (auto *ic = qobject_cast<IC *>(elm)) {
            if (!ic->file().isEmpty()) {
                m_icRegistry.watchFile(ic->file());
            }
            connect(ic, &IC::requestOpenSubCircuit, this, &Scene::icOpenRequested);
            connect(ic, &IC::previewRequested, this, &Scene::icPreviewRequested);
            connect(ic, &IC::previewMoved, this, &Scene::icPreviewMoved);
            connect(ic, &IC::previewHideRequested, this, &Scene::icPreviewHideRequested);
            connect(ic, &IC::previewCancelRequested, this, &Scene::icPreviewCancelRequested);
        } else if (auto *tt = qobject_cast<TruthTable *>(elm)) {
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
    return m_itemRegistry.itemById(id);
}

bool Scene::contains(const int id) const
{
    return m_itemRegistry.contains(id);
}

int Scene::lastId() const
{
    return m_itemRegistry.lastId();
}

void Scene::setLastId(const int newLastId)
{
    m_itemRegistry.setLastId(newLastId);
}

int Scene::nextId()
{
    return m_itemRegistry.nextId();
}

void Scene::updateItemId(ItemWithId *item, const int newId)
{
    m_itemRegistry.updateItemId(item, newId);
}

void Scene::forgetItemId(const int id)
{
    m_itemRegistry.forgetItemId(id);
}

void Scene::registerItem(ItemWithId *item)
{
    m_itemRegistry.registerItem(item);
}

void Scene::unregisterItem(ItemWithId *item)
{
    m_itemRegistry.unregisterItem(item);
}

SerializationContext Scene::deserializationContext(QHash<quint64, Port *> &portMap, const QVersionNumber &version)
{
    SerializationContext context = {portMap, version, contextDir()};
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

    const int gridSize = Constants::gridSize;
    const int left = static_cast<int>(std::floor(rect.left() / gridSize)) * gridSize;
    const int top = static_cast<int>(std::floor(rect.top() / gridSize)) * gridSize;
    const int right = static_cast<int>(std::ceil(rect.right() / gridSize)) * gridSize;
    const int bottom = static_cast<int>(std::ceil(rect.bottom() / gridSize)) * gridSize;

    painter->setPen(m_dots);

    QPolygon points;
    points.reserve(((right - left) / gridSize + 1) * ((bottom - top) / gridSize + 1));

    for (int x = left; x <= right; x += gridSize) {
        for (int y = top; y <= bottom; y += gridSize) {
            points.append(QPoint(x, y));
        }
    }

    painter->drawPoints(points);
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

    // Re-initialize topological sort and simulation graph after any structural change.
    // If initialize() bails (e.g. the scene dropped to just the border rect), it left
    // the hot-path vectors empty but didn't touch m_initialized — bring the flag into
    // sync so the next tick treats the scene as uninitialised rather than trusting a
    // stale "already done" marker against empty vectors.
    if (!m_simulation.initialize()) {
        m_simulation.restart();
    }

    m_autosaveRequired = true;
}

const QVector<GraphicElement *> Scene::visibleElements() const
{
    const auto visibleRect = m_view->mapToScene(m_view->viewport()->geometry()).boundingRect();

    return elements(visibleRect);
}

const QVector<GraphicElement *> Scene::elements() const
{
    return sortByTopology(unsortedElements());
}

const QVector<GraphicElement *> Scene::unsortedElements() const
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
                        auto &vec = successors[elm];
                        if (!vec.contains(successor)) {
                            vec.append(successor);
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

QHash<QString, InputPort *> Scene::wirelessTxInputPorts(const QVector<GraphicElement *> &elements)
{
    QHash<QString, InputPort *> txMap;
    for (auto *elm : elements) {
        if (elm->wirelessMode() == WirelessMode::Tx && !elm->label().isEmpty() && elm->inputPort(0)) {
            if (!txMap.contains(elm->label())) {
                txMap.insert(elm->label(), elm->inputPort(0));
            }
        }
    }
    return txMap;
}

const QVector<Connection *> Scene::connections() const
{
    const auto items_ = items();
    QVector<Connection *> conns;
    conns.reserve(items_.size());

    for (auto *item : items_) {
        if (item->type() == Connection::Type) {
            conns.append(qgraphicsitem_cast<Connection *>(item));
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

QGraphicsItem *Scene::itemAt(const QPointF pos) const
{
    auto items_ = items(pos);
    // Also check a small surrounding rectangle so port hit-testing works when
    // the cursor is near but not exactly on the port's bounding box
    items_.append(itemsAt(pos));

    // Ports get priority so wire dragging begins/ends on the port, not the element body
    for (auto *item : std::as_const(items_)) {
        if (item->type() == Port::Type) {
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

QList<QGraphicsItem *> Scene::itemsAt(const QPointF pos) const
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

    if (m_interaction.isDraggingElement()) {
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

bool Scene::isConnectionAllowed(OutputPort *startPort, InputPort *endPort)
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
    m_interaction.applyTheme(theme);

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
    if (auto *item = itemAt(m_interaction.lastMousePos())) {
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

        if (ClipboardManager::canPaste(mimeData)) {
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
        // No explicit Simulation::restart() needed: DeleteItemsCommand::redo
        // calls setCircuitUpdateRequired() which already drives a full
        // initialize()/restart() cycle once the deletion completes.
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
    for (auto *element : unsortedElements()) {
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

void Scene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (SceneDropHandler::isSupportedDropFormat(event->mimeData())) {
        event->accept();
        return;
    }

    QGraphicsScene::dragEnterEvent(event);
}

void Scene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    if (SceneDropHandler::isSupportedDropFormat(event->mimeData())) {
        event->accept();
        return;
    }

    QGraphicsScene::dragMoveEvent(event);
}

void Scene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    sentryBreadcrumb("ui", QStringLiteral("Drop event"));

    if (event->mimeData()->hasFormat(MimeType::DragDropLegacy)
        || event->mimeData()->hasFormat(MimeType::DragDrop)) {
        m_dropHandler.handleNewElementDrop(event);
    }

    if (event->mimeData()->hasFormat(MimeType::CloneDragLegacy)
        || event->mimeData()->hasFormat(MimeType::CloneDrag)) {
        m_dropHandler.handleCloneDrag(event);
    }

    QGraphicsScene::dropEvent(event);
}

void Scene::keyPressEvent(QKeyEvent *event)
{
    // Skip keyboard triggers while Ctrl is held to avoid firing during Ctrl+Z/C/V shortcuts
    if (!(event->modifiers().testFlag(Qt::ControlModifier))) {
        for (auto *element : unsortedElements()) {
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
        for (auto *element : unsortedElements()) {
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
            if (auto *item = itemAt(mouseEvent->scenePos()); item && ((item->type() == GraphicElement::Type) || (item->type() == Connection::Type))) {
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
    if (!m_interaction.mousePress(event)) {
        QGraphicsScene::mousePressEvent(event);
    }
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // Guard against re-entrant calls. When ensureVisible() scrolls the viewport,
    // Qt synchronously calls scrollContentsBy() → replayLastMouseEvent() →
    // mouseMoveEvent() before ensureVisible() returns. At a viewport corner,
    // scrolling to satisfy the H-margin violates the V-margin, so the replay
    // triggers another ensureVisible() that does the opposite, oscillating
    // until a stack overflow. Dropping the re-entrant call breaks the loop.
    // The guard lives here, not in SceneInteraction, because it must wrap the
    // base-class call below where the re-entrancy actually originates.
    if (m_handlingMouseMove) {
        return;
    }
    m_handlingMouseMove = true;
    const auto resetFlag = qScopeGuard([this] { m_handlingMouseMove = false; });

    if (!m_interaction.mouseMove(event)) {
        QGraphicsScene::mouseMoveEvent(event);
    }
}

void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_interaction.mouseRelease(event)) {
        QGraphicsScene::mouseReleaseEvent(event);
    }
}

void Scene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_interaction.mouseDoubleClick(event)) {
        QGraphicsScene::mouseDoubleClickEvent(event);
    }
}

void Scene::helpEvent(QGraphicsSceneHelpEvent *event)
{
    // A help event fires exactly when Qt is about to show a tooltip (after the wake-up
    // delay). If the cursor is over a port, show its own label plus its connected peers'
    // labels in situ instead of the native tooltip, so they all appear together at the
    // same wake-up delay a tooltip would use. Other items keep their native tooltip.
    if (auto *port = qgraphicsitem_cast<Port *>(itemAt(event->scenePos()))) {
        m_connectionManager.showHoverLabels(port);
        return;
    }

    QGraphicsScene::helpEvent(event);
}

void Scene::addItem(QMimeData *mimeData)
{
    m_dropHandler.addFromMimeData(mimeData);
}
