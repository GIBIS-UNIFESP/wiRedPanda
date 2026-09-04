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
#include "App/Element/IC.h"
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

/// Delivers a QHoverEvent to \a canvas the same way sendMouseEvent() delivers mouse events --
/// via QCoreApplication::sendEvent(), exercising the real hoverMoveEvent() override.
void sendHoverMove(CanvasItem &canvas, const QPointF &worldPos)
{
    const QPointF screenPos = canvas.worldToScreen(worldPos);
    QHoverEvent event(QEvent::HoverMove, screenPos, screenPos, QPointF());
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

void TestCanvasItemInteraction::testAutoRepeatKeyPressDoesNotRetoggleTrigger()
{
    // Mirrors TestScene's testKeyTriggerIgnoresAutoRepeat(): keyPressEvent()'s own
    // top-of-function isAutoRepeat() guard means holding a trigger key must fire once, not
    // toggle an InputSwitch dozens of times a second on the OS's own key-repeat.
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

    QKeyEvent autoRepeatPress(QEvent::KeyPress, Qt::Key_T, Qt::NoModifier, QString(), /*autorep=*/true);
    QCoreApplication::sendEvent(&canvas, &autoRepeatPress);
    QVERIFY2(input->isOn(), "an auto-repeat press must not toggle the latched switch back off");
}

void TestCanvasItemInteraction::testKeyReleaseTurnsOffMomentaryInputButton()
{
    // Mirrors TestScene's testKeyReleaseEventReleasesInputButtonTrigger().
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *button = ElementFactory::buildElement(ElementType::InputButton);
    canvas.receiveCommand(new CanvasAddItemsCommand({button}, &canvas));
    button->setTrigger(QKeySequence(Qt::Key_B));
    auto *input = qobject_cast<GraphicElementInput *>(button);
    QVERIFY(input);

    sendKey(canvas, Qt::Key_B);
    QVERIFY(input->isOn());

    QKeyEvent release(QEvent::KeyRelease, Qt::Key_B, Qt::NoModifier);
    QCoreApplication::sendEvent(&canvas, &release);
    QVERIFY2(!input->isOn(), "releasing the trigger key must turn a momentary InputButton back off");
}

void TestCanvasItemInteraction::testCtrlAndAltArrowKeysDoNotNudgeSelection()
{
    // Mirrors TestScene's testNudgeSelectionIgnoresCtrlAndAltModifiers() -- Ctrl/Alt+arrow must
    // be left to other handlers (zoom, etc.), not nudge the selection.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw}, &canvas));
    const QPointF before = sw->pos();

    sendKey(canvas, Qt::Key_Right, Qt::ControlModifier);
    QCOMPARE(sw->pos(), before);

    sendKey(canvas, Qt::Key_Right, Qt::AltModifier);
    QCOMPARE(sw->pos(), before);
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

void TestCanvasItemInteraction::testCtrlDragClonesSelectionAtOffsetPosition()
{
    // This canvas has no QDrag/QMimeData (MimeType::CloneDrag) mechanism like production's
    // drag-and-drop clone path -- mousePressEvent()'s own doc comment above explains why:
    // driving the existing drag-to-move machinery directly on live duplicated elements gives
    // the identical net effect without a real QDrag. testCtrlClickClonesSelectionInPlace()
    // above only exercises the zero-distance case; this exercises the clone landing at
    // original + drag delta.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *elm = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));
    const QPointF origPos = elm->pos();

    const QPointF from = insideElement(elm);
    const QPointF dropOffset(96, 0); // multiple of the grid-snap unit, avoids snap perturbation
    dragFromTo(canvas, from, from + dropOffset, Qt::ControlModifier);

    QCOMPARE(canvas.elements().size(), 2);

    GraphicElement *clone = nullptr;
    for (auto *e : canvas.elements()) {
        if (e != elm) {
            clone = e;
        }
    }
    QVERIFY2(clone, "Ctrl+drag must add a new cloned element distinct from the original");
    QVERIFY2(clone->isSelected(), "The newly cloned element must end up selected");
    QCOMPARE(clone->pos(), origPos + dropOffset);

    // The original selected element must be untouched by the clone.
    QCOMPARE(elm->pos(), origPos);
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

void TestCanvasItemInteraction::testDoubleClickEmptyLabelFallsBackToElementBoundingRect()
{
    // This canvas has no Widgets-only InlineLabelEditor class -- the inline editor is a plain
    // QML TextField positioned by Main.qml at inlineEditRequested()'s own targetRect argument.
    // mouseDoubleClickEvent()'s fallback logic that computes that rect -- labelSceneBoundingRect()
    // is empty when the label has no text yet, so it falls back to sceneBoundingRect() -- lives
    // in CanvasItem.cpp itself; testDoubleClickLabeledElementEmitsInlineEditRequested() above
    // never inspects the rect argument.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *elm = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({elm}, &canvas));
    QVERIFY(elm->hasLabel());
    QVERIFY2(elm->label().isEmpty(), "Precondition: a freshly-placed element has no label text yet");
    QVERIFY2(elm->labelSceneBoundingRect().isEmpty(), "Precondition: an empty label has an empty bounding rect");

    QSignalSpy spy(&canvas, &CanvasItem::inlineEditRequested);
    sendMouseEvent(canvas, QEvent::MouseButtonDblClick, insideElement(elm), Qt::LeftButton, Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
    const QRectF targetRect = spy.first().at(2).toRectF();
    const QRectF expected(canvas.worldToScreen(elm->sceneBoundingRect().topLeft()),
                           elm->sceneBoundingRect().size() * canvas.zoomScale());
    QCOMPARE(targetRect, expected);
}

void TestCanvasItemInteraction::testDoubleClickICEmitsIcOpenRequested()
{
    // mouseDoubleClickEvent() special-cases IC (opening its sub-circuit tab via
    // icOpenRequested()) instead of falling through to the labelable-element inline-edit path
    // above. Mirrors IC::handleDoubleClick()'s own production behavior; unlike
    // testDoubleClickLabeledElementEmitsInlineEditRequested()'s element, an IC never gets
    // inline label editing on double-click.
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *ic = qobject_cast<IC *>(ElementFactory::buildElement(ElementType::IC));
    QVERIFY(ic);
    canvas.receiveCommand(new CanvasAddItemsCommand({ic}, &canvas));

    QSignalSpy inlineEditSpy(&canvas, &CanvasItem::inlineEditRequested);
    QSignalSpy icOpenSpy(&canvas, &CanvasItem::icOpenRequested);

    sendMouseEvent(canvas, QEvent::MouseButtonDblClick, insideElement(ic), Qt::LeftButton, Qt::LeftButton);

    QCOMPARE(icOpenSpy.count(), 1);
    QCOMPARE(icOpenSpy.first().at(0).toInt(), ic->id());
    QCOMPARE(icOpenSpy.first().at(1).toString(), ic->blobName());
    QCOMPARE(icOpenSpy.first().at(2).toString(), ic->file());
    QVERIFY2(inlineEditSpy.isEmpty(), "an IC must never get inline label editing on double-click");
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

// ===========================================================================
// Show Gates / Show Wires visibility
// ===========================================================================
// isElementHidden()/isPortHidden() are private, so these test the real, observable effect
// instead: a hidden element/port must stop being hit-testable (rebuildSpatialIndex() drops
// it), the same way a real user could tell the difference by clicking where it used to be.

void TestCanvasItemInteraction::testSetGatesVisibleHidesGatesFromHitTesting()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));
    QVERIFY(canvas.gatesVisible()); // default: visible

    auto *gate = ElementFactory::buildElement(ElementType::And); // ElementGroup::Gate
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch); // ElementGroup::Input
    sw->setPos(200, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({gate, sw}, &canvas));

    canvas.setGatesVisible(false);
    QVERIFY(!canvas.gatesVisible());

    clickAt(canvas, insideElement(gate));
    QVERIFY2(!gate->isSelected(), "a hidden gate must not be hit-testable");

    // Input/Output/Other groups are explicitly carved out -- always visible regardless.
    clickAt(canvas, insideElement(sw));
    QVERIFY2(sw->isSelected(), "InputSwitch (ElementGroup::Input) must stay hit-testable");

    canvas.setGatesVisible(true);
    clickAt(canvas, insideElement(gate));
    QVERIFY2(gate->isSelected(), "re-showing gates must restore hit-testability");
}

void TestCanvasItemInteraction::testSetWiresVisibleHidesNodesFromHitTesting()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));
    QVERIFY(canvas.wiresVisible()); // default: visible

    auto *node = ElementFactory::buildElement(ElementType::Node);
    canvas.receiveCommand(new CanvasAddItemsCommand({node}, &canvas));

    canvas.setWiresVisible(false);
    QVERIFY(!canvas.wiresVisible());

    clickAt(canvas, insideElement(node));
    QVERIFY2(!node->isSelected(), "a Node is meaningless without wires and must be hidden entirely");

    canvas.setWiresVisible(true);
    clickAt(canvas, insideElement(node));
    QVERIFY2(node->isSelected(), "re-showing wires must restore the Node's hit-testability");
}

// ===========================================================================
// Port-hover peer labels (CanvasItem::hoverMoveEvent()/updatePortHover())
// ===========================================================================
// TestScene's testHelpEventShowsPortHoverLabelsOverPort()/testHelpEventFallsBackForNonPortItem()
// drive Scene::helpEvent() (QGraphicsSceneHelpEvent) -- a Widgets-only event type this canvas
// has no equivalent for. The underlying feature -- hovering a port highlights it and its wired
// peers -- has a real Quick port instead (updatePortHover()/portHoverChanged()).

void TestCanvasItemInteraction::testHoverOverPortEmitsPortHoverChangedWithPeers()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    led->setPos(200, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw, led}, &canvas));

    auto *conn = new Connection();
    conn->setStartPort(sw->outputPort(0));
    conn->setEndPort(led->inputPort(0));
    canvas.addItem(conn);

    QSignalSpy hoverSpy(&canvas, &CanvasItem::portHoverChanged);

    sendHoverMove(canvas, sw->outputPort(0)->scenePos());

    QCOMPARE(hoverSpy.count(), 1);
    const QVariantList chips = hoverSpy.constFirst().at(0).toList();
    QCOMPARE(chips.size(), 2); // the hovered port itself, plus its one connected peer

    // Moving off the port clears the hover chips.
    sendHoverMove(canvas, QPointF(400, 400));
    QCOMPARE(hoverSpy.count(), 2);
    QVERIFY(hoverSpy.at(1).at(0).toList().isEmpty());
}

void TestCanvasItemInteraction::testHoverOverNonPortEmitsNoPortHoverChips()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *gate = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({gate}, &canvas));

    QSignalSpy hoverSpy(&canvas, &CanvasItem::portHoverChanged);

    // Inside the gate's body, away from any port -- no hover change to report.
    sendHoverMove(canvas, insideElement(gate));

    QCOMPARE(hoverSpy.count(), 0);
}

void TestCanvasItemInteraction::testHoverLeaveClearsPortHover()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw}, &canvas));

    sendHoverMove(canvas, sw->outputPort(0)->scenePos());

    QSignalSpy hoverSpy(&canvas, &CanvasItem::portHoverChanged);
    QHoverEvent leaveEvent(QEvent::HoverLeave, QPointF(), QPointF(), QPointF());
    QCoreApplication::sendEvent(&canvas, &leaveEvent);

    QCOMPARE(hoverSpy.count(), 1);
    QVERIFY(hoverSpy.constFirst().at(0).toList().isEmpty());
}

// ===========================================================================
// Wire-creation port-press edge cases (mirrors TestScene's identical-named tests)
// ===========================================================================
// testWireCreationByDraggingPortToPort() above only exercises the "drag straight from an
// output port to an input port" case. mousePressEvent()'s own port-press branch has three
// more distinct paths: completing an in-progress wire via a second port *press* (not a drag),
// starting a wire from an *input* port instead of an output, and detaching an already-wired
// input port's connection.

void TestCanvasItemInteraction::testPressOnPortWhileWireInProgressCompletesConnection()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    led->setPos(250, 100);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw, led}, &canvas));
    QCOMPARE(canvas.connections().size(), 0);

    // Two separate presses (no drag/release between them), unlike
    // testWireCreationByDraggingPortToPort()'s single continuous drag.
    sendMouseEvent(canvas, QEvent::MouseButtonPress, sw->outputPort(0)->scenePos(), Qt::LeftButton, Qt::LeftButton);
    sendMouseEvent(canvas, QEvent::MouseButtonPress, led->inputPort(0)->scenePos(), Qt::LeftButton, Qt::LeftButton);

    QCOMPARE(canvas.connections().size(), 1);
    QCOMPARE(led->inputPort(0)->connections().size(), 1);
}

void TestCanvasItemInteraction::testPressOnEmptyInputPortStartsWireFromInput()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    led->setPos(250, 100);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw, led}, &canvas));

    // Reversed direction: start from the (empty) input port, complete at the output port.
    sendMouseEvent(canvas, QEvent::MouseButtonPress, led->inputPort(0)->scenePos(), Qt::LeftButton, Qt::LeftButton);
    sendMouseEvent(canvas, QEvent::MouseButtonPress, sw->outputPort(0)->scenePos(), Qt::LeftButton, Qt::LeftButton);

    QCOMPARE(canvas.connections().size(), 1);
    auto *conn = canvas.connections().constFirst();
    QCOMPARE(conn->startPort(), sw->outputPort(0));
    QCOMPARE(conn->endPort(), led->inputPort(0));
}

void TestCanvasItemInteraction::testPressOnOccupiedInputPortDetachesWire()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    led->setPos(250, 100);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw, led}, &canvas));
    dragFromTo(canvas, sw->outputPort(0)->scenePos(), led->inputPort(0)->scenePos());
    QCOMPARE(canvas.connections().size(), 1);

    // Pressing an already-wired input port detaches the existing connection (and starts a new
    // edited wire from the same output, re-completable by pressing the input port again).
    sendMouseEvent(canvas, QEvent::MouseButtonPress, led->inputPort(0)->scenePos(), Qt::LeftButton, Qt::LeftButton);
    QCOMPARE(canvas.connections().size(), 0);

    sendMouseEvent(canvas, QEvent::MouseButtonPress, led->inputPort(0)->scenePos(), Qt::LeftButton, Qt::LeftButton);
    QCOMPARE(canvas.connections().size(), 1);
}

// ===========================================================================
// Right-click context menu (CanvasItem::handleRightClick())
// ===========================================================================

void TestCanvasItemInteraction::testRightClickOnElementOpensContextMenu()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *gate = ElementFactory::buildElement(ElementType::And);
    canvas.receiveCommand(new CanvasAddItemsCommand({gate}, &canvas));
    // CanvasAddItemsCommand selects everything it adds (mirrors production's own
    // "newly added elements start selected" UX) -- deselect to set up this test's real
    // precondition instead of relying on that as a side effect.
    gate->setSelected(false);
    QVERIFY(!gate->isSelected());

    QSignalSpy spy(&canvas, &CanvasItem::elementContextMenuRequested);
    sendMouseEvent(canvas, QEvent::MouseButtonPress, insideElement(gate), Qt::RightButton, Qt::RightButton);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).value<GraphicElement *>(), gate);
    QVERIFY2(gate->isSelected(), "Right-clicking an unselected element must select it before opening its menu");
}

void TestCanvasItemInteraction::testRightClickOnEmptyCanvasEmitsEmptyContextMenuRequested()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    QSignalSpy elementSpy(&canvas, &CanvasItem::elementContextMenuRequested);
    QSignalSpy emptySpy(&canvas, &CanvasItem::emptyContextMenuRequested);
    sendMouseEvent(canvas, QEvent::MouseButtonPress, QPointF(400, 300), Qt::RightButton, Qt::RightButton);

    QCOMPARE(emptySpy.count(), 1);
    QCOMPARE(elementSpy.count(), 0);
}

void TestCanvasItemInteraction::testCtrlDoubleClickOnWireDoesNotInsertSplitNode()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    canvas.setSize(QSizeF(800, 600));

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    led->setPos(250, 0);
    canvas.receiveCommand(new CanvasAddItemsCommand({sw, led}, &canvas));
    dragFromTo(canvas, sw->outputPort(0)->scenePos(), led->inputPort(0)->scenePos());
    QCOMPARE(canvas.connections().size(), 1);

    const int undoCountBefore = canvas.undoStack()->count();
    const QPointF midWire = (sw->outputPort(0)->scenePos() + led->inputPort(0)->scenePos()) / 2.0;
    sendMouseEvent(canvas, QEvent::MouseButtonDblClick, midWire, Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier);

    QCOMPARE(canvas.undoStack()->count(), undoCountBefore);
    QCOMPARE(canvas.connections().size(), 1);
}
