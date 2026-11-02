// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestFeatures.h"

#include <memory>

#include <QMetaEnum>
#include <QTemporaryDir>

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "Tests/Common/TestUtils.h"

void TestFeatures::initTestCase()
{
    QVERIFY(m_tempDir.isValid());
}

void TestFeatures::testAndGateFeatures()
{
    // AND gate is a basic logic gate
    auto andGate = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::And));

    // AND gate should not have audio, frequency, delay, or colors by default
    QVERIFY(!andGate->offersParam(ParamId::AudioFile));
    QVERIFY(!andGate->offersParam(ParamId::Frequency));
    QVERIFY(!andGate->offersParam(ParamId::Delay));

    // AND gate may have label
    QVERIFY(!andGate->hasLabel()); // Logic gates typically don't show labels
}

void TestFeatures::testInputElementFeatures()
{
    // Input element (like a button/switch)
    auto input = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::Node));

    // Verify features return boolean values
    bool hasLabel = input->hasLabel();
    bool hasColors = input->offersParam(ParamId::Colour);

    // Node element has label (used as wireless channel name) but not colors
    QVERIFY(hasLabel);
    QVERIFY(!hasColors);

    // Input should not have frequency or delay
    QVERIFY(!input->offersParam(ParamId::Frequency));
    QVERIFY(!input->offersParam(ParamId::Delay));
}

void TestFeatures::testDisplayElementFeatures()
{
    // Display element (LED, output display)
    auto display = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::Display7));

    // Display should have colors
    QVERIFY(display->offersParam(ParamId::Colour));

    // Display should have label
    QVERIFY(display->hasLabel());
}

void TestFeatures::testFrequencyFeature()
{
    // Clock element has frequency
    auto clock = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::Clock));

    // Clock should have frequency setting
    QVERIFY(clock->offersParam(ParamId::Frequency));

    // Create a non-clock element
    auto andGate = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(!andGate->offersParam(ParamId::Frequency));
}

void TestFeatures::testColorFeature()
{
    // Display element has colors
    auto display = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::Display7));
    bool displayHasColors = display->offersParam(ParamId::Colour);
    QVERIFY(displayHasColors);

    // Logic gate typically doesn't have colors
    auto andGate = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(!andGate->offersParam(ParamId::Colour));

    // Node (connector) does not have colors
    auto node = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::Node));
    bool nodeHasColors = node->offersParam(ParamId::Colour);
    QVERIFY(!nodeHasColors);
}

void TestFeatures::testAudioFeature()
{
    // The audio capabilities, over every type: the audio box has a sound file, and it and the buzzer a volume
    const QMetaEnum metaEnum = QMetaEnum::fromType<ElementType>();
    for (int i = 0; i < metaEnum.keyCount(); ++i) {
        const auto type = static_cast<ElementType>(metaEnum.value(i));
        if ((type == ElementType::Unknown) || (type == ElementType::JKLatch)) {
            continue;
        }
        const auto element = std::unique_ptr<Element>(ElementFactory::buildElement(type));
        QVERIFY2(element->offersParam(ParamId::AudioFile) == (type == ElementType::AudioBox), metaEnum.key(i));
        QVERIFY2(element->offersParam(ParamId::Volume) == ((type == ElementType::AudioBox) || (type == ElementType::Buzzer)),
                 metaEnum.key(i));
    }
}

void TestFeatures::testDelayFeature()
{
    // Most elements don't have delay by default
    auto andGate = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(!andGate->offersParam(ParamId::Delay));

    auto display = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::Display7));
    QVERIFY(!display->offersParam(ParamId::Delay));
}

void TestFeatures::testLabelFeature()
{
    // Node element supports labels (used as wireless channel name)
    auto node = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::Node));
    bool nodeHasLabel = node->hasLabel();
    QVERIFY(nodeHasLabel);

    auto display = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::Display7));
    bool displayHasLabel = display->hasLabel();
    QVERIFY(displayHasLabel);

    // Logic gates typically don't have visible labels
    auto andGate = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(!andGate->hasLabel());
}

void TestFeatures::testTriggerFeature()
{
    // Trigger feature check for various elements
    auto andGate = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(!andGate->hasTrigger());

    auto flipFlop = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::DFlipFlop));
    QVERIFY(!flipFlop->hasTrigger()); // Flip-flop may or may not have user-accessible trigger
}

void TestFeatures::testTruthTableFeature()
{
    // Truth table feature for logic elements
    auto andGate = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(!andGate->hasTruthTable()); // Generic logic gates don't need truth tables

    auto display = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::Display7));
    QVERIFY(!display->hasTruthTable());
}

void TestFeatures::testMultipleElementsFeatures()
{
    // Test feature consistency across multiple element types
    // AND gate - no special features
    auto andGate = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(andGate != nullptr);
    QVERIFY(!andGate->hasLabel());
    QVERIFY(!andGate->offersParam(ParamId::Colour));
    QVERIFY(!andGate->offersParam(ParamId::Frequency));
    QVERIFY(!andGate->offersParam(ParamId::Delay));

    // OR gate - no special features
    auto orGate = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::Or));
    QVERIFY(orGate != nullptr);
    QVERIFY(!orGate->hasLabel());
    QVERIFY(!orGate->offersParam(ParamId::Colour));
    QVERIFY(!orGate->offersParam(ParamId::Frequency));
    QVERIFY(!orGate->offersParam(ParamId::Delay));

    // NOT gate - no special features
    auto notGate = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::Not));
    QVERIFY(notGate != nullptr);
    QVERIFY(!notGate->hasLabel());
    QVERIFY(!notGate->offersParam(ParamId::Colour));
    QVERIFY(!notGate->offersParam(ParamId::Frequency));
    QVERIFY(!notGate->offersParam(ParamId::Delay));

    // Node - has label (for wireless channel name), no other special features
    auto node = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::Node));
    QVERIFY(node != nullptr);
    QVERIFY(node->hasLabel());
    QVERIFY(!node->offersParam(ParamId::Colour));
    QVERIFY(!node->offersParam(ParamId::Frequency));
    QVERIFY(!node->offersParam(ParamId::Delay));

    // Clock - has frequency and delay
    auto clock = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::Clock));
    QVERIFY(clock != nullptr);
    QVERIFY(clock->hasLabel());
    QVERIFY(!clock->offersParam(ParamId::Colour));
    QVERIFY(clock->offersParam(ParamId::Frequency));
    QVERIFY(clock->offersParam(ParamId::Delay));

    // Display7 - has label and colors
    auto display = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::Display7));
    QVERIFY(display != nullptr);
    QVERIFY(display->hasLabel());
    QVERIFY(display->offersParam(ParamId::Colour));
    QVERIFY(!display->offersParam(ParamId::Frequency));
    QVERIFY(!display->offersParam(ParamId::Delay));
}
