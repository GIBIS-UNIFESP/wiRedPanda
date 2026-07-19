// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestCanvasItemInteraction.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QSignalSpy>
#include <QWheelEvent>

#include "App/Core/Constants.h"
#include "App/Core/Enums.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/QuickShell/Canvas/CanvasCommands.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

namespace {

/// Delivers \a event to \a canvas via QCoreApplication::sendEvent() -- QQuickItem::event() (its
/// own QObject::event() override) dispatches by QEvent::Type to the same protected virtual
/// handler (mousePressEvent()/keyPressEvent()/wheelEvent()/...) a real window's input pipeline
/// would call, so this exercises the identical code path a live gesture would, entirely
/// in-process (no QQuickWindow needed) -- see this class's own header doc comment.
void sendMouseEvent(CanvasItem &canvas, QEvent::Type type, const QPointF &worldPos,
                     Qt::MouseButton button, Qt::MouseButtons buttons,
                     Qt::KeyboardModifiers modifiers = Qt::NoModifier)
{
    const QPointF screenPos = canvas.worldToScreen(worldPos);
    QMouseEvent event(type, screenPos, screenPos, button, buttons, modifiers);
    QCoreApplication::sendEvent(&canvas, &event);
}

/// Plain click (press + release) at \a worldPos.
void clickAt(CanvasItem &canvas, const QPointF &worldPos, Qt::KeyboardModifiers modifiers = Qt::NoModifier)
{
    sendMouseEvent(canvas, QEvent::MouseButtonPress, worldPos, Qt::LeftButton, Qt::LeftButton, modifiers);
    sendMouseEvent(canvas, QEvent::MouseButtonRelease, worldPos, Qt::LeftButton, Qt::NoButton, modifiers);
}

/// Press at \a startWorld, one move to \a endWorld, release at \a endWorld -- a single move step
/// suffices here (unlike TestMainWindowGui.cpp's 5-step dragElement() helper, which matters for
/// a *real* window's incremental hit-testing): every gesture this drives (element drag,
/// rubber-band, wire-creation) reads only the latest move position, mirrored exactly by
/// mouseMoveEvent()'s own per-event (not per-pixel) state updates.
void dragFromTo(CanvasItem &canvas, const QPointF &startWorld, const QPointF &endWorld,
                 Qt::KeyboardModifiers modifiers = Qt::NoModifier)
{
    sendMouseEvent(canvas, QEvent::MouseButtonPress, startWorld, Qt::LeftButton, Qt::LeftButton, modifiers);
    sendMouseEvent(canvas, QEvent::MouseMove, endWorld, Qt::NoButton, Qt::LeftButton, modifiers);
    sendMouseEvent(canvas, QEvent::MouseButtonRelease, endWorld, Qt::LeftButton, Qt::NoButton, modifiers);
}

void sendKey(CanvasItem &canvas, int key, Qt::KeyboardModifiers modifiers = Qt::NoModifier)
{
    QKeyEvent event(QEvent::KeyPress, key, modifiers);
    QCoreApplication::sendEvent(&canvas, &event);
}

void sendWheel(CanvasItem &canvas, const QPointF &worldPos, int angleDeltaY)
{
    const QPointF screenPos = canvas.worldToScreen(worldPos);
    QWheelEvent event(screenPos, screenPos, QPoint(0, 0), QPoint(0, angleDeltaY),
                       Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QCoreApplication::sendEvent(&canvas, &event);
}

/// A point safely inside \a element's own hit-test box (rebuildSpatialIndex()'s
/// "boundingRect().translated(pos())"), away from its top-left corner/ports.
QPointF insideElement(GraphicElement *element)
{
    return element->pos() + QPointF(10, 10);
}

} // namespace

// ===========================================================================
// Keyboard shortcuts
// ===========================================================================

void TestCanvasItemInteraction::testDeleteKeyRemovesSelectedElement()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw}, &canvas));
    QCOMPARE(canvas.elements().size(), 1); // CanvasAddItemsCommand auto-selects it

    sendKey(canvas, Qt::Key_Delete);

    QCOMPARE(canvas.elements().size(), 0);
}

void TestCanvasItemInteraction::testCopyPasteViaKeyboard()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw}, &canvas));

    sendKey(canvas, Qt::Key_C, Qt::ControlModifier);
    sendKey(canvas, Qt::Key_V, Qt::ControlModifier);

    QCOMPARE(canvas.elements().size(), 2);
}

void TestCanvasItemInteraction::testCutPasteViaKeyboard()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw}, &canvas));

    sendKey(canvas, Qt::Key_X, Qt::ControlModifier);
    QCOMPARE(canvas.elements().size(), 0);

    sendKey(canvas, Qt::Key_V, Qt::ControlModifier);
    QCOMPARE(canvas.elements().size(), 1);
}

void TestCanvasItemInteraction::testDuplicateViaKeyboard()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw}, &canvas));

    sendKey(canvas, Qt::Key_D, Qt::ControlModifier);

    QCOMPARE(canvas.elements().size(), 2);
}

void TestCanvasItemInteraction::testSelectAllViaKeyboard()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *a = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *b = ElementFactory::buildElement(ElementType::InputSwitch);
    b->setPos(200, 0);
    auto *c = ElementFactory::buildElement(ElementType::InputSwitch);
    c->setPos(400, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({a}, &canvas));
    canvas.receiveCommand(new CanvasAddItemsCommand({b}, &canvas));
    canvas.receiveCommand(new CanvasAddItemsCommand({c}, &canvas));
    for (auto *elm : canvas.elements()) {
        elm->setSelected(false);
    }
    QCOMPARE(canvas.selectedElements().size(), 0);

    sendKey(canvas, Qt::Key_A, Qt::ControlModifier);

    QCOMPARE(canvas.selectedElements().size(), 3);
}

void TestCanvasItemInteraction::testRotateRightAndLeftViaKeyboard()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *elm = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));

    sendKey(canvas, Qt::Key_R, Qt::ControlModifier);
    QCOMPARE(elm->rotation(), 90.0);

    sendKey(canvas, Qt::Key_R, Qt::ControlModifier | Qt::ShiftModifier);
    QCOMPARE(elm->rotation(), 0.0);
}

void TestCanvasItemInteraction::testFlipHorizontallyViaKeyboard()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *elm = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));
    QVERIFY(!elm->isFlippedX());

    sendKey(canvas, Qt::Key_H, Qt::ControlModifier);

    QVERIFY(elm->isFlippedX());
}

void TestCanvasItemInteraction::testZoomShortcuts()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    const qreal base = canvas.zoomScale();
    QCOMPARE(base, 1.0);

    sendKey(canvas, Qt::Key_Equal, Qt::ControlModifier);
    QVERIFY(canvas.zoomScale() > base);

    sendKey(canvas, Qt::Key_Minus, Qt::ControlModifier);
    QCOMPARE(canvas.zoomScale(), base);

    sendKey(canvas, Qt::Key_Equal, Qt::ControlModifier);
    sendKey(canvas, Qt::Key_Equal, Qt::ControlModifier);
    QVERIFY(canvas.zoomScale() > base);

    sendKey(canvas, Qt::Key_0, Qt::ControlModifier);
    QCOMPARE(canvas.zoomScale(), base);

    // Ctrl+Shift+F fits the whole circuit -- needs at least one element and a real size (set
    // above) to have anything to fit, mirroring zoomToFit()'s own early-return guard. A single
    // small element's bounding box is much smaller than the 800x600 viewport, so fitting it
    // zooms IN (snapped to the nearest discrete step that doesn't overshoot).
    auto *elm = ElementFactory::buildElement(ElementType::And);
    elm->setPos(2000, 2000); // far outside the default 1:1 view -- zoomToFit must still find it
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));

    sendKey(canvas, Qt::Key_F, Qt::ControlModifier | Qt::ShiftModifier);
    QVERIFY(canvas.zoomScale() > base);
}

void TestCanvasItemInteraction::testPropertyAndTypeCycleShortcuts()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    // Main property ("]"/"["): And's input count, matching adjustMainProperty()'s dispatch.
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({andGate}, &canvas));
    QCOMPARE(andGate->inputSize(), andGate->minInputSize());

    sendKey(canvas, Qt::Key_BracketRight);
    QCOMPARE(andGate->inputSize(), andGate->minInputSize() + 1);

    sendKey(canvas, Qt::Key_BracketLeft);
    QCOMPARE(andGate->inputSize(), andGate->minInputSize());

    // Secondary property ("}"/"{"): Led's color, matching adjustSecondaryProperty()'s dispatch.
    andGate->setSelected(false);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    led->setPos(200, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({led}, &canvas));
    QVERIFY(led->hasColors());
    const QString initialColor = led->color();

    sendKey(canvas, Qt::Key_BraceRight);
    QVERIFY(led->color() != initialColor);

    sendKey(canvas, Qt::Key_BraceLeft);
    QCOMPARE(led->color(), initialColor);

    // Type cycle ("<"/">"): And -> Or -> And, matching Enums::nextElmType()/prevElmType().
    led->setSelected(false);
    andGate->setSelected(true);
    const int andId = andGate->id();

    sendKey(canvas, Qt::Key_Greater);
    auto *morphed = dynamic_cast<GraphicElement *>(canvas.itemById(andId));
    QVERIFY(morphed);
    QCOMPARE(morphed->elementType(), ElementType::Or);

    morphed->setSelected(true);
    sendKey(canvas, Qt::Key_Less);
    auto *morphedBack = dynamic_cast<GraphicElement *>(canvas.itemById(andId));
    QVERIFY(morphedBack);
    QCOMPARE(morphedBack->elementType(), ElementType::And);
}

void TestCanvasItemInteraction::testArrowKeyNudgesSelection()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw}, &canvas));
    const QPointF before = sw->pos();

    sendKey(canvas, Qt::Key_Right);
    QCOMPARE(sw->pos(), before + QPointF(Constants::gridSize, 0));

    sendKey(canvas, Qt::Key_Down, Qt::ShiftModifier);
    QCOMPARE(sw->pos(), before + QPointF(Constants::gridSize, Constants::gridSize * 4));

    canvas.undoStack()->undo();
    canvas.undoStack()->undo();
    QCOMPARE(sw->pos(), before);
}

void TestCanvasItemInteraction::testKeyboardTriggerFiresInputSwitch()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw}, &canvas));
    sw->setTrigger(QKeySequence(Qt::Key_T));

    auto *input = qobject_cast<GraphicElementInput *>(sw);
    QVERIFY(input);
    QVERIFY(!input->isOn());

    sendKey(canvas, Qt::Key_T);
    QVERIFY(input->isOn());
}

// ===========================================================================
// Mouse interactions
// ===========================================================================

void TestCanvasItemInteraction::testMouseClickSelectsElement()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *elm = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));
    elm->setSelected(false); // undo the add's auto-select so the click is what selects it

    clickAt(canvas, insideElement(elm));

    QCOMPARE(canvas.selectedElements().size(), 1);
    QVERIFY(elm->isSelected());
}

void TestCanvasItemInteraction::testMouseClickEmptyDeselects()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *elm = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));
    QCOMPARE(canvas.selectedElements().size(), 1);

    clickAt(canvas, QPointF(-500, -500));

    QCOMPARE(canvas.selectedElements().size(), 0);
}

void TestCanvasItemInteraction::testShiftClickTogglesMultiSelect()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *a = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *b = ElementFactory::buildElement(ElementType::And);
    b->setPos(200, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({a}, &canvas));
    canvas.receiveCommand(new CanvasAddItemsCommand({b}, &canvas));
    a->setSelected(false);
    b->setSelected(false);

    clickAt(canvas, insideElement(a));
    QCOMPARE(canvas.selectedElements().size(), 1);

    clickAt(canvas, insideElement(b), Qt::ShiftModifier);
    QCOMPARE(canvas.selectedElements().size(), 2);

    // Shift-clicking an already-selected element toggles it back off.
    clickAt(canvas, insideElement(b), Qt::ShiftModifier);
    QCOMPARE(canvas.selectedElements().size(), 1);
    QVERIFY(a->isSelected());
}

void TestCanvasItemInteraction::testCtrlClickClonesSelectionInPlace()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *elm = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));
    QCOMPARE(canvas.elements().size(), 1);

    // Ctrl+click(-and-drag) clones the current selection in place -- a zero-distance Ctrl-click
    // still leaves a real, undoable duplicate behind (via the add command alone), matching
    // mousePressEvent()'s own documented "zero-distance case".
    clickAt(canvas, insideElement(elm), Qt::ControlModifier);

    QCOMPARE(canvas.elements().size(), 2);
}

void TestCanvasItemInteraction::testMouseDragMovesElement()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *elm = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));
    const QPointF origPos = elm->pos();

    const QPointF from = insideElement(elm);
    dragFromTo(canvas, from, from + QPointF(50, 0));

    QVERIFY(elm->pos().x() > origPos.x());

    canvas.undoStack()->undo();
    QCOMPARE(elm->pos(), origPos);
}

void TestCanvasItemInteraction::testRubberBandSelectionSelectsEnclosedElements()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    sw->setPos(100, 100);
    andGate->setPos(150, 100);
    led->setPos(200, 100);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw}, &canvas));
    canvas.receiveCommand(new CanvasAddItemsCommand({andGate}, &canvas));
    canvas.receiveCommand(new CanvasAddItemsCommand({led}, &canvas));
    for (auto *elm : canvas.elements()) {
        elm->setSelected(false);
    }

    // Drag a rubber-band from above-left to below-right of all three elements.
    dragFromTo(canvas, QPointF(50, 50), QPointF(300, 200));

    QCOMPARE(canvas.selectedElements().size(), 3);
}

void TestCanvasItemInteraction::testWireCreationByDraggingPortToPort()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    sw->setPos(50, 100);
    led->setPos(250, 100);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw}, &canvas));
    canvas.receiveCommand(new CanvasAddItemsCommand({led}, &canvas));
    QCOMPARE(canvas.connections().size(), 0);

    const QPointF outPortPos = sw->outputPort(0)->scenePos();
    const QPointF inPortPos = led->inputPort(0)->scenePos();

    dragFromTo(canvas, outPortPos, inPortPos);

    QCOMPARE(canvas.connections().size(), 1);
    QCOMPARE(led->inputPort(0)->connections().size(), 1);
}

void TestCanvasItemInteraction::testDoubleClickLabeledElementEmitsInlineEditRequested()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    // And (and the other plain gates) has no label -- InputSwitch is one of the element types
    // that does (see GraphicElement::hasLabel()'s per-type ElementInfo metadata).
    auto *elm = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));
    QVERIFY(elm->hasLabel());

    QSignalSpy spy(&canvas, &CanvasItem::inlineEditRequested);

    sendMouseEvent(canvas, QEvent::MouseButtonDblClick, insideElement(elm), Qt::LeftButton, Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<GraphicElement *>(), elm);
}

void TestCanvasItemInteraction::testWheelZoomsInAndOut()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    const qreal base = canvas.zoomScale();
    const QPointF center(400, 300);

    sendWheel(canvas, center, 120);
    QVERIFY(canvas.zoomScale() > base);

    sendWheel(canvas, center, -120);
    QCOMPARE(canvas.zoomScale(), base);
}
