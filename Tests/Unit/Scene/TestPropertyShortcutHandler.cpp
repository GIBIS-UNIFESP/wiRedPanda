// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestPropertyShortcutHandler.h"

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/AudioBox.h"
#include "App/Element/GraphicElements/Buzzer.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/Display14.h"
#include "App/Element/GraphicElements/Display16.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Scene/PropertyShortcutHandler.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestPropertyShortcutHandler::testGateInputCountNext()
{
    WorkSpace workspace;
    auto *andGate = new And;
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
    auto *orGate = new Or;
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
    auto *clock = new Clock;
    workspace.scene()->addItem(clock);
    double originalFreq = clock->frequency();

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    clock->setSelected(true);

    handler.nextMainProperty();
    QCOMPARE(clock->frequency(), originalFreq + 0.5);
}

void TestPropertyShortcutHandler::testClockFrequencyPrev()
{
    WorkSpace workspace;
    auto *clock = new Clock;
    workspace.scene()->addItem(clock);
    double originalFreq = clock->frequency();

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    clock->setSelected(true);

    handler.prevMainProperty();
    QCOMPARE(clock->frequency(), originalFreq - 0.5);
}

void TestPropertyShortcutHandler::testBuzzerFrequency()
{
    WorkSpace workspace;
    auto *buzzer = new Buzzer;
    workspace.scene()->addItem(buzzer);
    double originalFreq = buzzer->frequency();

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    buzzer->setSelected(true);

    handler.nextMainProperty();
    QVERIFY(buzzer->frequency() > originalFreq);
    QVERIFY(buzzer->frequency() <= 20000.0);

    handler.prevMainProperty();
    QVERIFY(buzzer->frequency() >= 20.0);
}

void TestPropertyShortcutHandler::testLedColorSecondary()
{
    WorkSpace workspace;
    auto *led = new Led;
    workspace.scene()->addItem(led);
    QString originalColor = led->color();

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    led->setSelected(true);

    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();

    // Led color is secondary property
    handler.nextSecondaryProperty();
    QVERIFY(led->color() != originalColor);
    const QString cycledColor = led->color();

    // Regression: this mutation used to bypass the undo system entirely — every sibling
    // branch in adjustSecondaryProperty()/adjustMainProperty() wraps its change in an
    // UpdateCommand via applyWithUndo(), but the Led branch set the color directly.
    QCOMPARE(undoStack->count(), countBefore + 1);
    QVERIFY(undoStack->canUndo());
    undoStack->undo();
    QCOMPARE(led->color(), originalColor);
    undoStack->redo();
    QCOMPARE(led->color(), cycledColor);

    handler.prevSecondaryProperty();
    QCOMPARE(led->color(), originalColor);
}

void TestPropertyShortcutHandler::testDisplayColorPrev()
{
    WorkSpace workspace;
    auto *disp14 = new Display14;
    auto *disp7 = new Display7;
    workspace.scene()->addItem(disp14);
    workspace.scene()->addItem(disp7);

    PropertyShortcutHandler handler(workspace.scene());

    // Test Display14 color cycling via prevMainProperty
    workspace.scene()->clearSelection();
    disp14->setSelected(true);
    QString originalColor14 = disp14->color();
    handler.prevMainProperty();
    QVERIFY(disp14->color() != originalColor14);

    // Test Display7 color cycling via prevMainProperty
    workspace.scene()->clearSelection();
    disp7->setSelected(true);
    QString originalColor7 = disp7->color();
    handler.prevMainProperty();
    QVERIFY(disp7->color() != originalColor7);
}

void TestPropertyShortcutHandler::testInputRotaryOutputSize()
{
    WorkSpace workspace;
    auto *rotary = new InputRotary;
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
    auto *andGate = new And;
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
    auto *andGate = new And;
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
    auto *clock1 = new Clock;
    auto *clock2 = new Clock;
    workspace.scene()->addItem(clock1);
    workspace.scene()->addItem(clock2);
    const double originalFreq1 = clock1->frequency();
    const double originalFreq2 = clock2->frequency();

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    clock1->setSelected(true);
    clock2->setSelected(true);

    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();

    handler.nextMainProperty();

    QCOMPARE(clock1->frequency(), originalFreq1 + 0.5);
    QCOMPARE(clock2->frequency(), originalFreq2 + 0.5);
    QCOMPARE(undoStack->count(), countBefore + 1);

    undoStack->undo();
    QCOMPARE(clock1->frequency(), originalFreq1);
    QCOMPARE(clock2->frequency(), originalFreq2);
}

void TestPropertyShortcutHandler::testDisplay16ColorPrevMainProperty()
{
    // Only previous-color is handled for Display16 (no next-color cycling).
    WorkSpace workspace;
    auto *disp16 = new Display16;
    workspace.scene()->addItem(disp16);

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    disp16->setSelected(true);

    const QString originalColor = disp16->color();
    handler.prevMainProperty();
    QVERIFY(disp16->color() != originalColor);

    const QString afterPrev = disp16->color();
    handler.nextMainProperty();
    QCOMPARE(disp16->color(), afterPrev); // next is deliberately not implemented: no-op
}

void TestPropertyShortcutHandler::testAudioBoxMainPropertyIsNoOp()
{
    // AudioBox has no main-property cycling behavior defined -- confirm the shortcut is a
    // genuine no-op (no undo command pushed), not silently doing something unexpected.
    WorkSpace workspace;
    auto *audioBox = new AudioBox;
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
    auto *tt = new TruthTable;
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
    auto *led1 = new Led;
    auto *led2 = new Led;
    led2->setPos(100, 0);
    workspace.scene()->addItem(led1);
    workspace.scene()->addItem(led2);
    const QString originalColor1 = led1->color();
    const QString originalColor2 = led2->color();

    PropertyShortcutHandler handler(workspace.scene());
    workspace.scene()->clearSelection();
    led1->setSelected(true);
    led2->setSelected(true);

    auto *undoStack = workspace.scene()->undoStack();
    const int countBefore = undoStack->count();

    handler.nextSecondaryProperty();

    QVERIFY(led1->color() != originalColor1);
    QVERIFY(led2->color() != originalColor2);
    QCOMPARE(undoStack->count(), countBefore + 1);

    undoStack->undo();
    QCOMPARE(led1->color(), originalColor1);
    QCOMPARE(led2->color(), originalColor2);
}

void TestPropertyShortcutHandler::testMultiElementMorphNextIsSingleUndoStep()
{
    WorkSpace workspace;
    auto *and1 = new And;
    auto *and2 = new And;
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
    auto *and1 = new And;
    auto *and2 = new And;
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
