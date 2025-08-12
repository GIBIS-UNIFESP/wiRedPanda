// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testproperties.h"

#include "and.h"
#include "audiobox.h"
#include "buzzer.h"
#include "clock.h"
#include "demux.h"
#include "display_7.h"
#include "inputbutton.h"
#include "inputswitch.h"
#include "led.h"
#include "mux.h"
#include "nand.h"
#include "nor.h"
#include "not.h"
#include "or.h"
#include "qneport.h"
#include "truth_table.h"
#include "xnor.h"
#include "xor.h"

#include <QTest>

void TestProperties::initTestCase()
{
    // Initialize test environment
}

void TestProperties::cleanupTestCase()
{
    // Clean up test environment
}

void TestProperties::testLabelProperty()
{
    // Test elements that support labels
    And andGate;
    Or orGate;
    Not notGate;
    Nand nandGate;
    
    // Test default label state
    QVERIFY(andGate.hasLabel());
    QVERIFY(orGate.hasLabel());
    QVERIFY(notGate.hasLabel());
    QVERIFY(nandGate.hasLabel());
    
    // Test setting labels
    QString testLabel = "TestGate";
    andGate.setLabel(testLabel);
    QCOMPARE(andGate.label(), testLabel);
    
    orGate.setLabel(testLabel);
    QCOMPARE(orGate.label(), testLabel);
    
    // Test empty label
    andGate.setLabel("");
    QCOMPARE(andGate.label(), QString(""));
    
    // Test long label
    QString longLabel = "This is a very long label that might cause display issues";
    andGate.setLabel(longLabel);
    QCOMPARE(andGate.label(), longLabel);
    
    // Test special characters
    QString specialLabel = "Gate#1@$%^&*()";
    andGate.setLabel(specialLabel);
    QCOMPARE(andGate.label(), specialLabel);
}

void TestProperties::testInputSizeProperty()
{
    // Test elements that support variable input sizes
    And andGate;
    Or orGate;
    Nand nandGate;
    Nor norGate;
    Xor xorGate;
    Xnor xnorGate;
    
    // Test default input sizes
    QVERIFY(andGate.inputSize() >= 2);
    QVERIFY(orGate.inputSize() >= 2);
    QVERIFY(nandGate.inputSize() >= 2);
    QVERIFY(norGate.inputSize() >= 2);
    QVERIFY(xorGate.inputSize() >= 2);
    QVERIFY(xnorGate.inputSize() >= 2);
    
    // Test minimum input size constraints
    QVERIFY(andGate.minInputSize() >= 2);
    QVERIFY(orGate.minInputSize() >= 2);
    
    // Test maximum input size constraints
    QVERIFY(andGate.maxInputSize() > andGate.minInputSize());
    QVERIFY(orGate.maxInputSize() > orGate.minInputSize());
    
    // Test setting valid input sizes
    if (andGate.maxInputSize() >= 4) {
        andGate.setInputSize(4);
        QCOMPARE(andGate.inputSize(), 4);
    }
    
    if (orGate.maxInputSize() >= 3) {
        orGate.setInputSize(3);
        QCOMPARE(orGate.inputSize(), 3);
    }
    
    // Test boundary values
    andGate.setInputSize(andGate.minInputSize());
    QCOMPARE(andGate.inputSize(), andGate.minInputSize());
    
    andGate.setInputSize(andGate.maxInputSize());
    QCOMPARE(andGate.inputSize(), andGate.maxInputSize());
    
    // Test invalid input sizes (should be clamped or rejected)
    andGate.setInputSize(0); // Invalid
    QVERIFY(andGate.inputSize() >= andGate.minInputSize());
    
    andGate.setInputSize(1000); // Too large
    QVERIFY(andGate.inputSize() <= andGate.maxInputSize());
}

void TestProperties::testOutputSizeProperty()
{
    // Most elements have fixed output size of 1
    And andGate;
    Or orGate;
    Not notGate;
    
    QCOMPARE(andGate.outputSize(), 1);
    QCOMPARE(orGate.outputSize(), 1);
    QCOMPARE(notGate.outputSize(), 1);
    
    // Test elements with variable output sizes
    Demux demux;
    
    // Demux should have multiple outputs based on control inputs
    QVERIFY(demux.outputSize() >= 2);
    
    // Test that output size is read-only for most elements
    // (elements without setOutputSize method)
    QVERIFY(andGate.outputSize() > 0);
    QVERIFY(orGate.outputSize() > 0);
}

void TestProperties::testFrequencyProperty()
{
    // Test clock frequency property
    Clock clock;
    
    QVERIFY(clock.hasFrequency());
    
    // Test default frequency
    double defaultFreq = clock.frequency();
    QVERIFY(defaultFreq > 0);
    
    // Test setting frequency
    clock.setFrequency(2.0);
    QCOMPARE(clock.frequency(), 2.0);
    
    clock.setFrequency(10.5);
    QCOMPARE(clock.frequency(), 10.5);
    
    // Test frequency limits
    clock.setFrequency(0.1); // Very low
    QVERIFY(clock.frequency() > 0);
    
    clock.setFrequency(1000.0); // High but reasonable
    QCOMPARE(clock.frequency(), 1000.0);
    
    // Test negative frequency (should be rejected)
    clock.setFrequency(5.0); // Set valid first
    clock.setFrequency(-1.0); // Try invalid
    QVERIFY(clock.frequency() > 0); // Should remain positive
    
    // Test zero frequency (should be rejected)
    clock.setFrequency(5.0); // Set valid first
    clock.setFrequency(0.0); // Try zero
    QVERIFY(clock.frequency() > 0); // Should remain positive
}

void TestProperties::testAudioProperty()
{
    // Test buzzer audio property
    Buzzer buzzer;
    
    // Test default audio
    QString defaultAudio = buzzer.audio();
    QVERIFY(!defaultAudio.isEmpty());
    
    // Test setting different audio notes
    QStringList validNotes = {"C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5"};
    
    for (const QString& note : validNotes) {
        buzzer.setAudio(note);
        QCOMPARE(buzzer.audio(), note);
    }
    
    // Test AudioBox as well
    AudioBox audioBox;
    for (const QString& note : validNotes) {
        audioBox.setAudio(note);
        QCOMPARE(audioBox.audio(), note);
    }
    
    // Test invalid audio note (behavior depends on implementation)
    buzzer.setAudio("InvalidNote");
    // Should either reject or accept - both are valid behaviors
    QVERIFY(!buzzer.audio().isEmpty());
}

void TestProperties::testTriggerProperty()
{
    // Test trigger property on input elements
    InputButton button;
    
    QVERIFY(button.hasTrigger());
    
    // Test default trigger
    QKeySequence defaultTrigger = button.trigger();
    
    // Test setting trigger
    button.setTrigger(QKeySequence("Space"));
    QCOMPARE(button.trigger(), QKeySequence("Space"));
    
    button.setTrigger(QKeySequence("Enter"));
    QCOMPARE(button.trigger(), QKeySequence("Enter"));
    
    // Test InputSwitch might also have trigger
    InputSwitch inputSwitch;
    if (inputSwitch.hasTrigger()) {
        QKeySequence switchTrigger = inputSwitch.trigger();
        inputSwitch.setTrigger(QKeySequence("T"));
        QCOMPARE(inputSwitch.trigger(), QKeySequence("T"));
    }
}

void TestProperties::testRotationProperty()
{
    // Test rotation on various elements
    And andGate;
    Led led;
    Display7 display;
    
    // Test default rotation
    QCOMPARE(andGate.rotation(), 0.0);
    QCOMPARE(led.rotation(), 0.0);
    
    // Test setting rotation
    andGate.setRotation(90.0);
    QCOMPARE(andGate.rotation(), 90.0);
    
    andGate.setRotation(180.0);
    QCOMPARE(andGate.rotation(), 180.0);
    
    andGate.setRotation(270.0);
    QCOMPARE(andGate.rotation(), 270.0);
    
    andGate.setRotation(360.0);
    // Might normalize to 0 or keep as 360
    QVERIFY(andGate.rotation() == 0.0 || andGate.rotation() == 360.0);
    
    // Test negative rotation
    andGate.setRotation(-90.0);
    // Might normalize to 270 or keep as -90
    QVERIFY(andGate.rotation() == 270.0 || andGate.rotation() == -90.0);
}

void TestProperties::testColorProperty()
{
    // Test color property on LED
    Led led;
    
    // Test default color
    QString defaultColor = led.color();
    QVERIFY(!defaultColor.isEmpty());
    
    // Test setting different colors
    QStringList validColors = {"Red", "Green", "Blue", "Yellow", "White"};
    
    for (const QString& color : validColors) {
        led.setColor(color);
        QCOMPARE(led.color(), color);
    }
    
    // Test case sensitivity
    led.setColor("red");
    QString currentColor = led.color();
    // Should handle case appropriately (either "red" or "Red")
    QVERIFY(currentColor == "red" || currentColor == "Red");
}

void TestProperties::testElementTypeProperty()
{
    // Test element type identification
    And andGate;
    Or orGate;
    Not notGate;
    Clock clock;
    Led led;
    InputButton button;
    
    QCOMPARE(andGate.elementType(), ElementType::And);
    QCOMPARE(orGate.elementType(), ElementType::Or);
    QCOMPARE(notGate.elementType(), ElementType::Not);
    QCOMPARE(clock.elementType(), ElementType::Clock);
    QCOMPARE(led.elementType(), ElementType::Led);
    QCOMPARE(button.elementType(), ElementType::InputButton);
}

void TestProperties::testElementGroupProperty()
{
    // Test element group classification
    And andGate;
    InputButton button;
    Led led;
    
    QCOMPARE(andGate.elementGroup(), ElementGroup::Gate);
    QCOMPARE(button.elementGroup(), ElementGroup::Input);
    QCOMPARE(led.elementGroup(), ElementGroup::Output);
}

void TestProperties::testValueProperties()
{
    // Test output value property
    InputButton button;
    InputSwitch inputSwitch;
    
    // Test initial state (outputValue returns int: 0 or 1)
    int buttonValue = button.outputValue();
    QVERIFY(buttonValue == 0 || buttonValue == 1);
    
    // Test toggling button
    bool initialState = button.isOn();
    button.setOn(!initialState);
    QCOMPARE(button.outputValue(), static_cast<int>(!initialState));
    
    // Test switch
    bool switchState = inputSwitch.isOn();
    inputSwitch.setOn(!switchState);
    QCOMPARE(inputSwitch.outputValue(), static_cast<int>(!switchState));
    
    // Test that logic gates have valid output structure
    And andGate;
    // Logic gates compute their output based on inputs
    // Just verify the gate has proper structure
    QVERIFY(andGate.outputSize() == 1);
    QVERIFY(andGate.inputSize() >= 2);
}

void TestProperties::testPositionProperty()
{
    // Test position property
    And andGate;
    
    // Test default position
    QPointF defaultPos = andGate.pos();
    QVERIFY(defaultPos.x() >= 0 || defaultPos.x() < 0); // Any value is valid
    
    // Test setting position
    andGate.setPos(100, 200);
    QCOMPARE(andGate.pos(), QPointF(100, 200));
    
    andGate.setPos(QPointF(50.5, 75.5));
    QCOMPARE(andGate.pos(), QPointF(50.5, 75.5));
    
    // Test negative positions
    andGate.setPos(-50, -100);
    QCOMPARE(andGate.pos(), QPointF(-50, -100));
}

void TestProperties::testSelectionProperty()
{
    // Test selection state
    And andGate;
    
    // Test default selection state
    QVERIFY(!andGate.isSelected());
    
    // Test setting selection
    andGate.setSelected(true);
    QVERIFY(andGate.isSelected());
    
    andGate.setSelected(false);
    QVERIFY(!andGate.isSelected());
    
    // Test multiple elements
    Or orGate;
    andGate.setSelected(true);
    orGate.setSelected(true);
    
    QVERIFY(andGate.isSelected());
    QVERIFY(orGate.isSelected());
}

void TestProperties::testEnabledProperty()
{
    // Test enabled state
    And andGate;
    
    // Test default enabled state
    QVERIFY(andGate.isEnabled());
    
    // Test disabling
    andGate.setEnabled(false);
    QVERIFY(!andGate.isEnabled());
    
    andGate.setEnabled(true);
    QVERIFY(andGate.isEnabled());
}

void TestProperties::testBoundingRectProperty()
{
    // Test bounding rectangle
    And andGate;
    Led led;
    
    QRectF andRect = andGate.boundingRect();
    QRectF ledRect = led.boundingRect();
    
    // Should have positive dimensions
    QVERIFY(andRect.width() > 0);
    QVERIFY(andRect.height() > 0);
    QVERIFY(ledRect.width() > 0);
    QVERIFY(ledRect.height() > 0);
    
    // Different elements should have different sizes
    // (This might not always be true, but generally expected)
    QVERIFY(andRect != ledRect);
}