// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestPropertyShortcutHandler.h"

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Scene/PropertyShortcutHandler.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestPropertyShortcutHandler::testGateInputCountNext()
{
    WorkSpace workspace;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    workspace.scene()->addItem(andGate);
    int originalInputs = andGate->inputSize();

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    andGate->setSelected(true);

    handler.nextMainProperty();
    QCOMPARE(andGate->inputSize(), originalInputs + 1);
}

void TestPropertyShortcutHandler::testGateInputCountPrev()
{
    WorkSpace workspace;
    auto *orGate = ElementFactory::buildElement(ElementType::Or);
    workspace.scene()->addItem(orGate);

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    orGate->setSelected(true);

    // Increase first so we can decrease
    handler.nextMainProperty();
    int afterIncrease = orGate->inputSize();
    handler.prevMainProperty();
    QCOMPARE(orGate->inputSize(), afterIncrease - 1);
}

void TestPropertyShortcutHandler::testClockFrequencyNext()
{
    WorkSpace workspace;
    auto *clock = ElementFactory::buildElement(ElementType::Clock);
    workspace.scene()->addItem(clock);
    double originalFreq = clock->param<double>(ParamId::Frequency);

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    clock->setSelected(true);

    handler.nextMainProperty();
    QCOMPARE(clock->param<double>(ParamId::Frequency), originalFreq + 0.5);
}

void TestPropertyShortcutHandler::testClockFrequencyPrev()
{
    WorkSpace workspace;
    auto *clock = ElementFactory::buildElement(ElementType::Clock);
    workspace.scene()->addItem(clock);
    double originalFreq = clock->param<double>(ParamId::Frequency);

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    clock->setSelected(true);

    handler.prevMainProperty();
    QCOMPARE(clock->param<double>(ParamId::Frequency), originalFreq - 0.5);
}

void TestPropertyShortcutHandler::testBuzzerFrequency()
{
    WorkSpace workspace;
    auto *buzzer = ElementFactory::buildElement(ElementType::Buzzer);
    workspace.scene()->addItem(buzzer);
    double originalFreq = buzzer->param<double>(ParamId::Frequency);

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    buzzer->setSelected(true);

    handler.nextMainProperty();
    QVERIFY(buzzer->param<double>(ParamId::Frequency) > originalFreq);
    QVERIFY(buzzer->param<double>(ParamId::Frequency) <= 20000.0);

    handler.prevMainProperty();
    QVERIFY(buzzer->param<double>(ParamId::Frequency) >= 20.0);

    // The steps stop at the audible range's ends, 20 Hz and 20 kHz, which the buzzer's own spec does not bound
    QVERIFY(buzzer->setParam(ParamId::Frequency, 19950.0));
    handler.nextMainProperty();
    QCOMPARE(buzzer->param<double>(ParamId::Frequency), 20000.0);
    QVERIFY(buzzer->setParam(ParamId::Frequency, 50.0));
    handler.prevMainProperty();
    QCOMPARE(buzzer->param<double>(ParamId::Frequency), 20.0);
}

void TestPropertyShortcutHandler::testLedColorSecondary()
{
    WorkSpace workspace;
    auto *led = ElementFactory::buildElement(ElementType::Led);
    workspace.scene()->addItem(led);
    QString originalColor = led->param<QString>(ParamId::Colour);

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    led->setSelected(true);

    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();

    // Led color is secondary property
    handler.nextSecondaryProperty();
    QVERIFY(led->param<QString>(ParamId::Colour) != originalColor);
    const QString cycledColor = led->param<QString>(ParamId::Colour);

    // Regression: this mutation used to bypass the undo system entirely — every sibling
    // branch in adjustSecondaryProperty()/adjustMainProperty() wraps its change in an
    // UpdateCommand via applyWithUndo(), but the Led branch set the color directly.
    QCOMPARE(undoStack->count(), countBefore + 1);
    QVERIFY(undoStack->canUndo());
    undoStack->undo();
    QCOMPARE(led->param<QString>(ParamId::Colour), originalColor);
    undoStack->redo();
    QCOMPARE(led->param<QString>(ParamId::Colour), cycledColor);

    handler.prevSecondaryProperty();
    QCOMPARE(led->param<QString>(ParamId::Colour), originalColor);
}

void TestPropertyShortcutHandler::testDisplayColorPrev()
{
    WorkSpace workspace;
    auto *disp14 = ElementFactory::buildElement(ElementType::Display14);
    auto *disp7 = ElementFactory::buildElement(ElementType::Display7);
    workspace.scene()->addItem(disp14);
    workspace.scene()->addItem(disp7);

    PropertyShortcutHandler handler(workspace.scene());

    // Test Display14 color cycling via prevMainProperty
    workspace.scene()->clearSelection();
    disp14->setSelected(true);
    QString originalColor14 = disp14->param<QString>(ParamId::Colour);
    handler.prevMainProperty();
    QVERIFY(disp14->param<QString>(ParamId::Colour) != originalColor14);

    // Test Display7 color cycling via prevMainProperty
    workspace.scene()->clearSelection();
    disp7->setSelected(true);
    QString originalColor7 = disp7->param<QString>(ParamId::Colour);
    handler.prevMainProperty();
    QVERIFY(disp7->param<QString>(ParamId::Colour) != originalColor7);
}

void TestPropertyShortcutHandler::testInputRotaryOutputSize()
{
    WorkSpace workspace;
    auto *rotary = ElementFactory::buildElement(ElementType::InputRotary);
    workspace.scene()->addItem(rotary);
    int originalOutputs = rotary->outputSize();

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    rotary->setSelected(true);

    handler.nextMainProperty();
    if (originalOutputs < rotary->maxOutputSize()) {
        QCOMPARE(rotary->outputSize(), originalOutputs + 1);
        handler.prevMainProperty();
        QCOMPARE(rotary->outputSize(), originalOutputs);
    } else {
        QCOMPARE(rotary->outputSize(), originalOutputs);
    }
}

void TestPropertyShortcutHandler::testMorphNextElement()
{
    WorkSpace workspace;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    workspace.scene()->addItem(andGate);

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    andGate->setSelected(true);

    handler.nextElement();
    // After morph, the And should have been replaced by Or
    auto selected = workspace.scene()->selectedElements();
    QCOMPARE(selected.size(), 1);
    QCOMPARE(selected.first()->elementType(), ElementType::Or);
}

void TestPropertyShortcutHandler::testMorphPrevElement()
{
    WorkSpace workspace;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    workspace.scene()->addItem(andGate);

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    andGate->setSelected(true);

    handler.prevElement();
    auto selected = workspace.scene()->selectedElements();
    QCOMPARE(selected.size(), 1);
    QCOMPARE(selected.first()->elementType(), ElementType::Xnor);
}

void TestPropertyShortcutHandler::testMultiElementShortcutIsSingleUndoStep()
{
    // Regression: cycling a property on a multi-selection used to push one undo command
    // per element, ungrouped — a single Ctrl+Z only reverted the last element touched.
    // It must now be a single undo step for the whole action, like every other
    // multi-element command in the app (e.g. the "Change wireless mode" macro).
    WorkSpace workspace;
    auto *clock1 = ElementFactory::buildElement(ElementType::Clock);
    auto *clock2 = ElementFactory::buildElement(ElementType::Clock);
    workspace.scene()->addItem(clock1);
    workspace.scene()->addItem(clock2);
    const double originalFreq1 = clock1->param<double>(ParamId::Frequency);
    const double originalFreq2 = clock2->param<double>(ParamId::Frequency);

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    clock1->setSelected(true);
    clock2->setSelected(true);

    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();

    handler.nextMainProperty();

    QCOMPARE(clock1->param<double>(ParamId::Frequency), originalFreq1 + 0.5);
    QCOMPARE(clock2->param<double>(ParamId::Frequency), originalFreq2 + 0.5);
    QCOMPARE(undoStack->count(), countBefore + 1);

    undoStack->undo();
    QCOMPARE(clock1->param<double>(ParamId::Frequency), originalFreq1);
    QCOMPARE(clock2->param<double>(ParamId::Frequency), originalFreq2);
}

void TestPropertyShortcutHandler::testDisplay16ColorPrevMainProperty()
{
    // Only previous-color is handled for Display16 (no next-color cycling).
    WorkSpace workspace;
    auto *disp16 = ElementFactory::buildElement(ElementType::Display16);
    workspace.scene()->addItem(disp16);

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    disp16->setSelected(true);

    const QString originalColor = disp16->param<QString>(ParamId::Colour);
    handler.prevMainProperty();
    QVERIFY(disp16->param<QString>(ParamId::Colour) != originalColor);

    const QString afterPrev = disp16->param<QString>(ParamId::Colour);
    handler.nextMainProperty();
    QCOMPARE(disp16->param<QString>(ParamId::Colour), afterPrev); // next is deliberately not implemented: no-op
}

void TestPropertyShortcutHandler::testAudioBoxMainPropertyIsNoOp()
{
    // AudioBox has no main-property cycling behavior defined -- confirm the shortcut is a
    // genuine no-op (no undo command pushed), not silently doing something unexpected.
    WorkSpace workspace;
    auto *audioBox = ElementFactory::buildElement(ElementType::AudioBox);
    workspace.scene()->addItem(audioBox);

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    audioBox->setSelected(true);

    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();

    handler.nextMainProperty();

    QCOMPARE(undoStack->count(), countBefore);
}

void TestPropertyShortcutHandler::testTruthTableOutputSizeSecondaryProperty()
{
    WorkSpace workspace;
    auto *tt = ElementFactory::buildElement(ElementType::TruthTable);
    workspace.scene()->addItem(tt);
    const int originalOutputs = tt->outputSize();

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    tt->setSelected(true);

    handler.nextSecondaryProperty();
    QCOMPARE(tt->outputSize(), originalOutputs + 1);

    handler.prevSecondaryProperty();
    QCOMPARE(tt->outputSize(), originalOutputs);
}

void TestPropertyShortcutHandler::testMultiElementSecondaryPropertyIsSingleUndoStep()
{
    WorkSpace workspace;
    auto *led1 = ElementFactory::buildElement(ElementType::Led);
    auto *led2 = ElementFactory::buildElement(ElementType::Led);
    led2->setPos(100, 0);
    workspace.scene()->addItem(led1);
    workspace.scene()->addItem(led2);
    const QString originalColor1 = led1->param<QString>(ParamId::Colour);
    const QString originalColor2 = led2->param<QString>(ParamId::Colour);

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    led1->setSelected(true);
    led2->setSelected(true);

    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();

    handler.nextSecondaryProperty();

    QVERIFY(led1->param<QString>(ParamId::Colour) != originalColor1);
    QVERIFY(led2->param<QString>(ParamId::Colour) != originalColor2);
    QCOMPARE(undoStack->count(), countBefore + 1);

    undoStack->undo();
    QCOMPARE(led1->param<QString>(ParamId::Colour), originalColor1);
    QCOMPARE(led2->param<QString>(ParamId::Colour), originalColor2);
}

void TestPropertyShortcutHandler::testMultiElementMorphNextIsSingleUndoStep()
{
    WorkSpace workspace;
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    and2->setPos(200, 0);
    workspace.scene()->addItem(and1);
    workspace.scene()->addItem(and2);

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    and1->setSelected(true);
    and2->setSelected(true);

    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();

    handler.nextElement();

    const auto selected = workspace.scene()->selectedElements();
    QCOMPARE(selected.size(), 2);
    for (auto *elm : selected) {
        QCOMPARE(elm->elementType(), ElementType::Or);
    }
    QCOMPARE(undoStack->count(), countBefore + 1);

    undoStack->undo();
    QCOMPARE(workspace.scene()->elements().size(), 2);
}

void TestPropertyShortcutHandler::testMultiElementMorphPrevIsSingleUndoStep()
{
    WorkSpace workspace;
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    and2->setPos(200, 0);
    workspace.scene()->addItem(and1);
    workspace.scene()->addItem(and2);

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    and1->setSelected(true);
    and2->setSelected(true);

    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();

    handler.prevElement();

    const auto selected = workspace.scene()->selectedElements();
    QCOMPARE(selected.size(), 2);
    for (auto *elm : selected) {
        QCOMPARE(elm->elementType(), ElementType::Xnor);
    }
    QCOMPARE(undoStack->count(), countBefore + 1);

    undoStack->undo();
    QCOMPARE(workspace.scene()->elements().size(), 2);
}

// ─── Undo probes: the shortcut cycles through redo and with a port change ───

void TestPropertyShortcutHandler::testMultiElementMorphNextRedoIsSingleStep()
{
    // A morph of two elements is one step each way: the undo brings both back, the redo morphs both again.
    WorkSpace workspace;
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    and2->setPos(200, 0);
    workspace.scene()->addItem(and1);
    workspace.scene()->addItem(and2);

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    and1->setSelected(true);
    and2->setSelected(true);

    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();
    handler.nextElement();
    QCOMPARE(undoStack->count(), countBefore + 1);

    undoStack->undo();
    for (auto *elm : workspace.scene()->elements()) {
        QCOMPARE(elm->elementType(), ElementType::And);
    }
    QCOMPARE(workspace.scene()->elements().size(), 2);

    undoStack->redo();
    QCOMPARE(workspace.scene()->elements().size(), 2);
    for (auto *elm : workspace.scene()->elements()) {
        QCOMPARE(elm->elementType(), ElementType::Or);
    }
    QCOMPARE(undoStack->count(), countBefore + 1);
}

void TestPropertyShortcutHandler::testMultiElementShortcutRedoIsSingleStep()
{
    // A property cycle over two elements is one step: the undo gives both their old value, and the redo gives both
    // the new one again, as one step.
    WorkSpace workspace;
    auto *clock1 = ElementFactory::buildElement(ElementType::Clock);
    auto *clock2 = ElementFactory::buildElement(ElementType::Clock);
    workspace.scene()->addItem(clock1);
    workspace.scene()->addItem(clock2);
    const double originalFreq1 = clock1->param<double>(ParamId::Frequency);
    const double originalFreq2 = clock2->param<double>(ParamId::Frequency);

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    clock1->setSelected(true);
    clock2->setSelected(true);

    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();
    handler.nextMainProperty();
    QCOMPARE(undoStack->count(), countBefore + 1);

    undoStack->undo();
    QCOMPARE(clock1->param<double>(ParamId::Frequency), originalFreq1);
    QCOMPARE(clock2->param<double>(ParamId::Frequency), originalFreq2);

    undoStack->redo();
    QCOMPARE(clock1->param<double>(ParamId::Frequency), originalFreq1 + 0.5);
    QCOMPARE(clock2->param<double>(ParamId::Frequency), originalFreq2 + 0.5);
    QCOMPARE(undoStack->count(), countBefore + 1);
    QCOMPARE(undoStack->index(), countBefore + 1);
}

void TestPropertyShortcutHandler::testCycleWithAPortChangeRedoesAsOneStep()
{
    // A property cycle that changes an element's input count is one step, and its undo and redo each change the count
    // back and forth with the step.
    WorkSpace workspace;
    auto *gate = ElementFactory::buildElement(ElementType::And);
    workspace.scene()->addItem(gate);
    const int gateId = gate->id();

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    gate->setSelected(true);

    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();
    const int inputsBefore = gate->inputSize();
    handler.nextMainProperty();
    auto *changed = dynamic_cast<Element *>(workspace.scene()->itemById(gateId));
    QVERIFY(changed != nullptr);
    const int inputsAfter = changed->inputSize();
    QVERIFY(inputsAfter != inputsBefore);
    QCOMPARE(undoStack->count(), countBefore + 1);

    undoStack->undo();
    QCOMPARE(dynamic_cast<Element *>(workspace.scene()->itemById(gateId))->inputSize(), inputsBefore);
    undoStack->redo();
    QCOMPARE(dynamic_cast<Element *>(workspace.scene()->itemById(gateId))->inputSize(), inputsAfter);
    QCOMPARE(undoStack->count(), countBefore + 1);
}
