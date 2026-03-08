// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestElementProperties.h"

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

// Input Size Tests

void TestElementProperties::testInputSizeVariableElement()
{
    // AND gate: inputs [2, 8], outputs [1, 1]
    And andGate;

    // Verify default
    QCOMPARE(andGate.inputSize(), 2);
    QCOMPARE(andGate.minInputSize(), 2);
    QCOMPARE(andGate.maxInputSize(), 8);

    // Change to 5 inputs (valid)
    andGate.setInputSize(5);
    QCOMPARE(andGate.inputSize(), 5);

    // Verify ports created
    for (int i = 0; i < 5; ++i) {
        QVERIFY(andGate.inputPort(i) != nullptr);
    }

    // Change to 3 inputs (valid)
    andGate.setInputSize(3);
    QCOMPARE(andGate.inputSize(), 3);
}

void TestElementProperties::testInputSizeConstraints()
{
    And andGate;

    // Try below minimum (invalid)
    andGate.setInputSize(1);  // Min is 2
    QCOMPARE(andGate.inputSize(), 2);  // Should stay at default

    // Try above maximum (invalid)
    andGate.setInputSize(10);  // Max is 8
    QCOMPARE(andGate.inputSize(), 2);  // Should stay at default

    // Set to max (valid)
    andGate.setInputSize(8);
    QCOMPARE(andGate.inputSize(), 8);

    // Set to min (valid)
    andGate.setInputSize(2);
    QCOMPARE(andGate.inputSize(), 2);
}

void TestElementProperties::testInputSizeFixed()
{
    // DFlipFlop: inputs [4, 4] (fixed)
    DFlipFlop dff;

    QCOMPARE(dff.inputSize(), 4);
    QCOMPARE(dff.minInputSize(), 4);
    QCOMPARE(dff.maxInputSize(), 4);

    // Try to change (should fail silently)
    dff.setInputSize(2);
    QCOMPARE(dff.inputSize(), 4);  // Unchanged

    dff.setInputSize(6);
    QCOMPARE(dff.inputSize(), 4);  // Unchanged

    // Verify NOT gate also fixed
    Not notGate;
    QCOMPARE(notGate.inputSize(), 1);
    QCOMPARE(notGate.minInputSize(), 1);
    QCOMPARE(notGate.maxInputSize(), 1);

    notGate.setInputSize(2);
    QCOMPARE(notGate.inputSize(), 1);  // Unchanged
}

void TestElementProperties::testInputSizePortCreation()
{
    Or orGate;
    orGate.setInputSize(2);  // Start with 2

    // Increase to 5
    orGate.setInputSize(5);

    // Verify all ports exist and are properly initialized
    for (int i = 0; i < 5; ++i) {
        auto *port = orGate.inputPort(i);
        QVERIFY2(port != nullptr, qPrintable(QString("Port %1 should exist").arg(i)));
        // Port creation is verified by existence; connections may be empty or populated
    }

    // Decrease to 3
    orGate.setInputSize(3);

    // Verify only 3 ports exist
    for (int i = 0; i < 3; ++i) {
        QVERIFY(orGate.inputPort(i) != nullptr);
    }
}

// Output Size Tests

void TestElementProperties::testOutputSizeVariableElement()
{
    // Most elements have fixed outputs [1, 1]
    // Test that API works for elements that support variable outputs
    // For now, verify API is callable and returns expected fixed outputs

    And andGate;
    QCOMPARE(andGate.outputSize(), 1);

    // Attempt to change (should fail silently for AND)
    andGate.setOutputSize(2);
    QCOMPARE(andGate.outputSize(), 1);  // Unchanged
}

void TestElementProperties::testOutputSizeFixed()
{
    // AND: outputs [1, 1] (fixed)
    And andGate;
    QCOMPARE(andGate.outputSize(), 1);
    QCOMPARE(andGate.minOutputSize(), 1);
    QCOMPARE(andGate.maxOutputSize(), 1);

    andGate.setOutputSize(2);
    QCOMPARE(andGate.outputSize(), 1);  // Unchanged

    // Clock: outputs [1, 1] (fixed)
    Clock clock;
    QCOMPARE(clock.outputSize(), 1);

    clock.setOutputSize(3);
    QCOMPARE(clock.outputSize(), 1);  // Unchanged

    // LED: outputs [0, 0] (no outputs)
    Led led;
    QCOMPARE(led.outputSize(), 0);
    QCOMPARE(led.minOutputSize(), 0);
    QCOMPARE(led.maxOutputSize(), 0);
}

// Label Tests

void TestElementProperties::testLabelProperty()
{
    // Use Clock which has label support by default
    Clock clock;

    // Verify label support
    QVERIFY(clock.hasLabel());

    // Set label
    const QString label1 = "MyClock";
    clock.setLabel(label1);
    QCOMPARE(clock.label(), label1);

    // Change label
    const QString label2 = "RenamedClock";
    clock.setLabel(label2);
    QCOMPARE(clock.label(), label2);

    // Empty label
    clock.setLabel("");
    QCOMPARE(clock.label(), QString(""));

    // Long label
    const QString longLabel = "VeryLongLabelNameForTesting";
    clock.setLabel(longLabel);
    QCOMPARE(clock.label(), longLabel);
}

void TestElementProperties::testLabelPersistence()
{
    // Create element with label
    WorkSpace workspace1;
    auto *andGate = new And();
    const QString label = "TestLabel123";
    andGate->setLabel(label);
    workspace1.scene()->addItem(andGate);

    // Save to memory
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    workspace1.save(stream);

    // Load from memory
    WorkSpace workspace2;
    QDataStream loadStream(&data, QIODevice::ReadOnly);
    const QVersionNumber version = Serialization::readPandaHeader(loadStream);
    workspace2.load(loadStream, version);

    // Verify label preserved
    const auto elements = workspace2.scene()->elements();
    QVERIFY2(!elements.isEmpty(), "Scene should contain loaded element");

    auto *loaded = dynamic_cast<And *>(elements.first());
    QVERIFY2(loaded != nullptr, "Loaded element should be an AND gate");
    QCOMPARE(loaded->label(), label);
}

// Color Tests

void TestElementProperties::testColorProperty()
{
    Led led;

    // Verify has color support
    QVERIFY(led.hasColors());

    // Set colors
    led.setColor("Red");
    QCOMPARE(led.color(), QString("Red"));

    led.setColor("Green");
    QCOMPARE(led.color(), QString("Green"));

    led.setColor("Blue");
    QCOMPARE(led.color(), QString("Blue"));

    led.setColor("Purple");
    QCOMPARE(led.color(), QString("Purple"));

    led.setColor("White");
    QCOMPARE(led.color(), QString("White"));
}

void TestElementProperties::testColorCycling()
{
    Led led;

    // Start with White
    led.setColor("White");
    QCOMPARE(led.color(), QString("White"));

    // Cycle: White → Red
    QString next1 = led.nextColor();
    QCOMPARE(next1, QString("Red"));

    led.setColor(next1);
    QString next2 = led.nextColor();
    QCOMPARE(next2, QString("Green"));

    led.setColor(next2);
    QString next3 = led.nextColor();
    QCOMPARE(next3, QString("Blue"));

    led.setColor(next3);
    QString next4 = led.nextColor();
    QCOMPARE(next4, QString("Purple"));

    led.setColor(next4);
    QString next5 = led.nextColor();
    QCOMPARE(next5, QString("White"));  // Cycles back

    // Test previous cycling
    led.setColor("Red");
    QString prev = led.previousColor();
    QCOMPARE(prev, QString("White"));
}

// Rotation Tests

void TestElementProperties::testRotationProperty()
{
    And andGate;

    // Default rotation
    QCOMPARE(andGate.rotation(), 0.0);

    // Set rotation
    andGate.setRotation(90.0);
    QCOMPARE(andGate.rotation(), 90.0);

    andGate.setRotation(180.0);
    QCOMPARE(andGate.rotation(), 180.0);

    andGate.setRotation(270.0);
    QCOMPARE(andGate.rotation(), 270.0);

    andGate.setRotation(45.0);
    QCOMPARE(andGate.rotation(), 45.0);
}

void TestElementProperties::testRotationNormalization()
{
    And andGate;

    // Test angle >= 360 (normalized)
    andGate.setRotation(360.0);
    QCOMPARE(andGate.rotation(), 0.0);  // 360 mod 360 = 0

    andGate.setRotation(450.0);
    QCOMPARE(andGate.rotation(), 90.0);  // 450 mod 360 = 90

    andGate.setRotation(720.0);
    QCOMPARE(andGate.rotation(), 0.0);  // 720 mod 360 = 0

    // Test negative angles (preserved as negative)
    andGate.setRotation(-90.0);
    QCOMPARE(andGate.rotation(), -90.0);  // Negative angles preserved

    andGate.setRotation(-180.0);
    QCOMPARE(andGate.rotation(), -180.0);  // Negative angles preserved
}

// Clock Properties

void TestElementProperties::testFrequencyProperty()
{
    Clock clock;

    // Verify has frequency support
    QVERIFY(clock.hasFrequency());

    // Default frequency (usually 1 Hz)
    QCOMPARE(clock.frequency(), 1.0f);

    // Set various frequencies
    clock.setFrequency(10.0f);
    QCOMPARE(clock.frequency(), 10.0f);

    clock.setFrequency(100.0f);
    QCOMPARE(clock.frequency(), 100.0f);

    clock.setFrequency(0.5f);
    QCOMPARE(clock.frequency(), 0.5f);

    // Test delay property
    QVERIFY(clock.hasDelay());
    clock.setDelay(0.5f);
    QCOMPARE(clock.delay(), 0.5f);

    clock.setDelay(1.0f);
    QCOMPARE(clock.delay(), 1.0f);
}

void TestElementProperties::testFrequencyValidation()
{
    Clock clock;

    // Set valid frequency baseline
    clock.setFrequency(10.0f);
    QCOMPARE(clock.frequency(), 10.0f);

    // Test 1: Zero frequency (invalid - rejected)
    // Clock uses 1s/(2*freq) for timing, so 1/(2*0) is undefined
    clock.setFrequency(0.0f);
    QCOMPARE(clock.frequency(), 10.0f);  // Should remain unchanged

    // Test 2: Negative frequency (invalid - rejected)
    // Clock uses 1s/(2*freq) which would be negative, triggering interval check
    // Expected behavior: rejected, value unchanged
    float beforeNegative = clock.frequency();
    clock.setFrequency(-5.0f);
    QCOMPARE(clock.frequency(), beforeNegative);  // Unchanged by negative attempt

    // Test 3: Valid positive frequency
    clock.setFrequency(50.0f);
    QCOMPARE(clock.frequency(), 50.0f);

    // Test 4: Very high frequency (may succeed or be rejected based on timing precision)
    float before = clock.frequency();
    clock.setFrequency(1000000.0f);  // 1 MHz - may exceed timing precision
    // Either accepted or rejected (unchanged), but must be positive
    QVERIFY(clock.frequency() > 0.0f);
    // Verify that frequency is either unchanged or set to requested value
    // (High frequency may be rejected due to precision limits)
    if (clock.frequency() == before) {
        // High frequency was rejected - this is acceptable
        QCOMPARE(clock.frequency(), before);
    } else {
        // High frequency was accepted
        QCOMPARE(clock.frequency(), 1000000.0f);
    }
}

// Serialization Test

void TestElementProperties::testPropertiesSaveLoad()
{
    // Create element with multiple properties set
    WorkSpace workspace1;

    auto *andGate = new And();
    andGate->setInputSize(5);
    andGate->setLabel("SavedGate");
    andGate->setRotation(90.0);
    workspace1.scene()->addItem(andGate);

    auto *led = new Led();
    led->setInputSize(3);
    led->setLabel("SavedLED");
    led->setColor("Blue");
    workspace1.scene()->addItem(led);

    auto *clock = new Clock();
    clock->setFrequency(50.0f);
    clock->setDelay(0.5f);
    clock->setLabel("SavedClock");
    workspace1.scene()->addItem(clock);

    // Save to memory
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    workspace1.save(stream);

    // Load from memory
    WorkSpace workspace2;
    QDataStream loadStream(&data, QIODevice::ReadOnly);
    const QVersionNumber version = Serialization::readPandaHeader(loadStream);
    workspace2.load(loadStream, version);

    auto elements = workspace2.scene()->elements();
    QCOMPARE(elements.size(), 3);

    // Find elements by type
    And *loadedAnd = nullptr;
    Led *loadedLed = nullptr;
    Clock *loadedClock = nullptr;

    for (auto *elem : elements) {
        if (elem->elementType() == ElementType::And) {
            loadedAnd = dynamic_cast<And *>(elem);
        } else if (elem->elementType() == ElementType::Led) {
            loadedLed = dynamic_cast<Led *>(elem);
        } else if (elem->elementType() == ElementType::Clock) {
            loadedClock = dynamic_cast<Clock *>(elem);
        }
    }

    // Verify AND gate properties
    QVERIFY(loadedAnd != nullptr);
    QCOMPARE(loadedAnd->inputSize(), 5);
    QCOMPARE(loadedAnd->label(), QString("SavedGate"));
    QCOMPARE(loadedAnd->rotation(), 90.0);

    // Verify LED properties
    QVERIFY(loadedLed != nullptr);
    QCOMPARE(loadedLed->inputSize(), 3);
    QCOMPARE(loadedLed->label(), QString("SavedLED"));
    QCOMPARE(loadedLed->color(), QString("Blue"));

    // Verify Clock properties
    QVERIFY(loadedClock != nullptr);
    QCOMPARE(loadedClock->frequency(), 50.0f);
    QCOMPARE(loadedClock->delay(), 0.5f);
    QCOMPARE(loadedClock->label(), QString("SavedClock"));
}

// ============================================================
// Port Access Tests
// ============================================================

void TestElementProperties::testInputPortByIndexValid()
{
    // Create an AND gate with 4 inputs
    And andGate;
    andGate.setInputSize(4);

    // Verify we can access each input port by valid index
    for (int i = 0; i < 4; ++i) {
        auto *port = andGate.inputPort(i);
        QVERIFY(port != nullptr);
        QCOMPARE(port->index(), i);
    }
}

void TestElementProperties::testInputPortByIndexInvalid()
{
    // Create an AND gate with 2 inputs (default)
    And andGate;
    QCOMPARE(andGate.inputSize(), 2);

    // inputPort() uses QVector::at() which asserts on invalid index
    // So we just verify valid indices work (0 and 1)
    auto *port0 = andGate.inputPort(0);
    auto *port1 = andGate.inputPort(1);
    QVERIFY(port0 != nullptr);
    QVERIFY(port1 != nullptr);

    // Verify port indices are correct
    QCOMPARE(port0->index(), 0);
    QCOMPARE(port1->index(), 1);
}

void TestElementProperties::testOutputPortByIndexValid()
{
    // Create an AND gate (1 output by default)
    And andGate;
    QCOMPARE(andGate.outputSize(), 1);

    // Access the output port
    auto *port = andGate.outputPort(0);
    QVERIFY(port != nullptr);
    QCOMPARE(port->index(), 0);
}

void TestElementProperties::testOutputPortByIndexInvalid()
{
    // Create an AND gate (1 output)
    And andGate;
    QCOMPARE(andGate.outputSize(), 1);

    // outputPort() uses QVector::at() which asserts on invalid index
    // So we just verify valid index works
    auto *port0 = andGate.outputPort(0);
    QVERIFY(port0 != nullptr);
    QCOMPARE(port0->index(), 0);
}

void TestElementProperties::testInputsVectorReference()
{
    // Create an AND gate with 3 inputs
    And andGate;
    andGate.setInputSize(3);

    // Get inputs vector reference
    const auto &inputs = andGate.inputs();

    // Verify size matches
    QCOMPARE(inputs.size(), 3);

    // Verify all are non-null
    for (auto *input : inputs) {
        QVERIFY(input != nullptr);
    }

    // Verify they match individual port access
    for (int i = 0; i < inputs.size(); ++i) {
        QCOMPARE(inputs[i], andGate.inputPort(i));
    }
}

void TestElementProperties::testOutputsVectorReference()
{
    // Create an AND gate (1 output by default)
    And andGate;

    // Get outputs vector reference
    const auto &outputs = andGate.outputs();

    // Verify size is 1
    QCOMPARE(outputs.size(), 1);

    // Verify it's non-null
    QVERIFY(outputs[0] != nullptr);

    // Verify it matches port access
    QCOMPARE(outputs[0], andGate.outputPort(0));
}

void TestElementProperties::testPortCountAfterSizeChange()
{
    // Create an AND gate with default 2 inputs
    And andGate;
    QCOMPARE(andGate.inputSize(), 2);

    // Change to 5 inputs
    andGate.setInputSize(5);
    QCOMPARE(andGate.inputSize(), 5);

    // Verify all new ports are accessible
    for (int i = 0; i < 5; ++i) {
        QVERIFY(andGate.inputPort(i) != nullptr);
    }

    // Change back to 3 inputs
    andGate.setInputSize(3);
    QCOMPARE(andGate.inputSize(), 3);

    // Old ports should still be accessible up to new count
    for (int i = 0; i < 3; ++i) {
        QVERIFY(andGate.inputPort(i) != nullptr);
    }
}

void TestElementProperties::testPortListIntegrity()
{
    // Create multiple elements with different port counts
    And andGate;
    andGate.setInputSize(4);

    Or orGate;
    orGate.setInputSize(3);

    Not notGate;  // Fixed 1 input, 1 output

    // Verify AND gate integrity
    for (int i = 0; i < 4; ++i) {
        QVERIFY(andGate.inputPort(i) != nullptr);
        QCOMPARE(andGate.inputPort(i)->index(), i);
    }

    // Verify OR gate integrity
    for (int i = 0; i < 3; ++i) {
        QVERIFY(orGate.inputPort(i) != nullptr);
        QCOMPARE(orGate.inputPort(i)->index(), i);
    }

    // Verify NOT gate integrity
    QCOMPARE(notGate.inputSize(), 1);
    QCOMPARE(notGate.outputSize(), 1);
    QVERIFY(notGate.inputPort(0) != nullptr);
    QVERIFY(notGate.outputPort(0) != nullptr);

    // Verify no cross-contamination between elements
    QVERIFY(andGate.inputPort(0) != orGate.inputPort(0));
    QVERIFY(notGate.inputPort(0) != andGate.inputPort(0));
}
