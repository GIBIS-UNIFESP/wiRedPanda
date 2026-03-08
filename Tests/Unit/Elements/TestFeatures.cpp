// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestFeatures.h"

#include <memory>

#include <QTemporaryDir>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/GlobalProperties.h"
#include "Tests/Common/TestUtils.h"

void TestFeatures::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
    GlobalProperties::currentDir = m_tempDir.path();
}

void TestFeatures::testAndGateFeatures()
{
    // AND gate is a basic logic gate
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));

    // AND gate should not have audio, frequency, delay, or colors by default
    QVERIFY(!andGate->hasAudio());
    QVERIFY(!andGate->hasAudioBox());
    QVERIFY(!andGate->hasFrequency());
    QVERIFY(!andGate->hasDelay());

    // AND gate may have label
    QVERIFY(!andGate->hasLabel()); // Logic gates typically don't show labels
}

void TestFeatures::testInputElementFeatures()
{
    // Input element (like a button/switch)
    auto input = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));

    // Verify features return boolean values
    bool hasLabel = input->hasLabel();
    bool hasColors = input->hasColors();

    // Node element does not have label or color features by default
    QVERIFY(!hasLabel);
    QVERIFY(!hasColors);

    // Input should not have frequency or delay
    QVERIFY(!input->hasFrequency());
    QVERIFY(!input->hasDelay());
}

void TestFeatures::testDisplayElementFeatures()
{
    // Display element (LED, output display)
    auto display = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Display7));

    // Display should have colors
    QVERIFY(display->hasColors());

    // Display should have label
    QVERIFY(display->hasLabel());
}

void TestFeatures::testFrequencyFeature()
{
    // Clock element has frequency
    auto clock = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));

    // Clock should have frequency setting
    QVERIFY(clock->hasFrequency());

    // Create a non-clock element
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(!andGate->hasFrequency());
}

void TestFeatures::testColorFeature()
{
    // Display element has colors
    auto display = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Display7));
    bool displayHasColors = display->hasColors();
    QVERIFY(displayHasColors);

    // Logic gate typically doesn't have colors
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(!andGate->hasColors());

    // Node (connector) does not have colors
    auto node = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    bool nodeHasColors = node->hasColors();
    QVERIFY(!nodeHasColors);
}

void TestFeatures::testAudioFeature()
{
    // Audio feature is typically not set by default
    auto display = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Display7));
    QVERIFY(!display->hasAudio());

    // Create various elements and check audio feature
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(!andGate->hasAudio());

    auto node = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    QVERIFY(!node->hasAudio());
}

void TestFeatures::testDelayFeature()
{
    // Most elements don't have delay by default
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(!andGate->hasDelay());

    auto display = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Display7));
    QVERIFY(!display->hasDelay());
}

void TestFeatures::testLabelFeature()
{
    // Node element does not support labels by default
    auto node = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    bool nodeHasLabel = node->hasLabel();
    QVERIFY(!nodeHasLabel);

    auto display = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Display7));
    bool displayHasLabel = display->hasLabel();
    QVERIFY(displayHasLabel);

    // Logic gates typically don't have visible labels
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(!andGate->hasLabel());
}

void TestFeatures::testTriggerFeature()
{
    // Trigger feature check for various elements
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(!andGate->hasTrigger());

    auto flipFlop = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::DFlipFlop));
    QVERIFY(!flipFlop->hasTrigger()); // Flip-flop may or may not have user-accessible trigger
}

void TestFeatures::testTruthTableFeature()
{
    // Truth table feature for logic elements
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(!andGate->hasTruthTable()); // Generic logic gates don't need truth tables

    auto display = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Display7));
    QVERIFY(!display->hasTruthTable());
}

void TestFeatures::testMultipleElementsFeatures()
{
    // Test feature consistency across multiple element types
    // AND gate - no special features
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(andGate != nullptr);
    QVERIFY(!andGate->hasLabel());
    QVERIFY(!andGate->hasAudio());
    QVERIFY(!andGate->hasColors());
    QVERIFY(!andGate->hasFrequency());
    QVERIFY(!andGate->hasDelay());

    // OR gate - no special features
    auto orGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Or));
    QVERIFY(orGate != nullptr);
    QVERIFY(!orGate->hasLabel());
    QVERIFY(!orGate->hasAudio());
    QVERIFY(!orGate->hasColors());
    QVERIFY(!orGate->hasFrequency());
    QVERIFY(!orGate->hasDelay());

    // NOT gate - no special features
    auto notGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Not));
    QVERIFY(notGate != nullptr);
    QVERIFY(!notGate->hasLabel());
    QVERIFY(!notGate->hasAudio());
    QVERIFY(!notGate->hasColors());
    QVERIFY(!notGate->hasFrequency());
    QVERIFY(!notGate->hasDelay());

    // Node - no special features
    auto node = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    QVERIFY(node != nullptr);
    QVERIFY(!node->hasLabel());
    QVERIFY(!node->hasAudio());
    QVERIFY(!node->hasColors());
    QVERIFY(!node->hasFrequency());
    QVERIFY(!node->hasDelay());

    // Clock - has frequency and delay
    auto clock = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Clock));
    QVERIFY(clock != nullptr);
    QVERIFY(clock->hasLabel());
    QVERIFY(!clock->hasAudio());
    QVERIFY(!clock->hasColors());
    QVERIFY(clock->hasFrequency());
    QVERIFY(clock->hasDelay());

    // Display7 - has label and colors
    auto display = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Display7));
    QVERIFY(display != nullptr);
    QVERIFY(display->hasLabel());
    QVERIFY(!display->hasAudio());
    QVERIFY(display->hasColors());
    QVERIFY(!display->hasFrequency());
    QVERIFY(!display->hasDelay());
}

void TestFeatures::testFeatureConsistency()
{
    // Create the same element type multiple times and verify features are consistent
    auto display1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Display7));
    auto display2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Display7));

    // Same element types should have same features
    QCOMPARE(display1->hasLabel(), display2->hasLabel());
    QCOMPARE(display1->hasColors(), display2->hasColors());
    QCOMPARE(display1->hasAudio(), display2->hasAudio());
    QCOMPARE(display1->hasFrequency(), display2->hasFrequency());
    QCOMPARE(display1->hasDelay(), display2->hasDelay());

    // Test with logic gates
    auto and1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    auto and2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));

    QCOMPARE(and1->hasLabel(), and2->hasLabel());
    QCOMPARE(and1->hasColors(), and2->hasColors());
    QCOMPARE(and1->hasAudio(), and2->hasAudio());
    QCOMPARE(and1->hasFrequency(), and2->hasFrequency());
}
