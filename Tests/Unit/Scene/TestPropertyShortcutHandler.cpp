// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestPropertyShortcutHandler.h"

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Buzzer.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/Display14.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Scene/PropertyShortcutHandler.h"
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

    // Led color is secondary property
    handler.nextSecondaryProperty();
    QVERIFY(led->color() != originalColor);

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

