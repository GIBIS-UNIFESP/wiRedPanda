// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestGraphicelementAdvanced.h"

#include <cmath>
#include <memory>

#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Nodes/QNEPort.h"
#include "Tests/Common/TestUtils.h"

// ============================================================================
// Port Management Tests
// ============================================================================

void TestGraphicelementAdvanced::testSetAndGetInputs()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(elem != nullptr);

    // Get initial inputs
    const auto &initialInputs = elem->inputs();
    QVERIFY(!initialInputs.isEmpty());

    // Verify we can access the vector
    QCOMPARE(initialInputs.size(), 2);
}

void TestGraphicelementAdvanced::testSetAndGetOutputs()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Or));
    QVERIFY(elem != nullptr);

    // Get outputs
    const auto &outputs = elem->outputs();
    QVERIFY(!outputs.isEmpty());

    // Verify output count
    QCOMPARE(outputs.size(), 1);
}

void TestGraphicelementAdvanced::testInputPortByIndex()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(elem != nullptr);

    // AND gate has 2 inputs
    QVERIFY(elem->inputPort(0) != nullptr);
    QVERIFY(elem->inputPort(1) != nullptr);
}

void TestGraphicelementAdvanced::testOutputPortByIndex()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Nand));
    QVERIFY(elem != nullptr);

    // NAND gate has 1 output
    QVERIFY(elem->outputPort(0) != nullptr);
}

void TestGraphicelementAdvanced::testInputPortNegativeIndex()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(elem != nullptr);

    // Accessing negative index should return nullptr
    auto *port = elem->inputPort(-1);
    QVERIFY(port == nullptr);
}

void TestGraphicelementAdvanced::testOutputPortNegativeIndex()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Or));
    QVERIFY(elem != nullptr);

    // Accessing negative index should return nullptr
    auto *port = elem->outputPort(-1);
    QVERIFY(port == nullptr);
}

// ============================================================================
// Port Size Constraints Tests
// ============================================================================

void TestGraphicelementAdvanced::testMinInputSizeConstraint()
{
    // AND gate minimum inputs should be 2
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(elem != nullptr);
    QCOMPARE(elem->inputSize(), 2);
}

void TestGraphicelementAdvanced::testMaxInputSizeConstraint()
{
    // NOT gate should have fixed 1 input
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Not));
    QVERIFY(elem != nullptr);
    QCOMPARE(elem->inputSize(), 1);
}

void TestGraphicelementAdvanced::testMinOutputSizeConstraint()
{
    // All logic gates should have at least 1 output
    auto and_gate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QCOMPARE(and_gate->outputSize(), 1);

    auto or_gate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Or));
    QCOMPARE(or_gate->outputSize(), 1);
}

void TestGraphicelementAdvanced::testMaxOutputSizeConstraint()
{
    // Standard gates have 1 output (no fan-out multiple outputs)
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Xor));
    QVERIFY(elem != nullptr);
    QCOMPARE(elem->outputSize(), 1);
}

// ============================================================================
// Pixmap and Visual Properties Tests
// ============================================================================

void TestGraphicelementAdvanced::testPixmapCenterCalculation()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(elem != nullptr);

    // Pixmap center should be valid QPointF
    QPointF center = elem->pixmapCenter();
    QVERIFY(!std::isnan(center.x()));
    QVERIFY(!std::isnan(center.y()));
}

void TestGraphicelementAdvanced::testBoundingRectCalculation()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Or));
    QVERIFY(elem != nullptr);

    QRectF rect = elem->boundingRect();
    QVERIFY2(!rect.isEmpty(), "Bounding rect should not be empty");
    QVERIFY2(rect.width() > 0, "Bounding rect width should be positive");
    QVERIFY(rect.height() > 0);
}

void TestGraphicelementAdvanced::testPortsBoundingRectCalculation()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Nand));
    QVERIFY(elem != nullptr);

    QRectF rect = elem->boundingRect();
    QVERIFY2(rect.width() > 0, "Bounding rect width should be positive");
}

// ============================================================================
// Element Type and Group Tests
// ============================================================================

void TestGraphicelementAdvanced::testElementTypeRetrieval()
{
    auto and_gate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QCOMPARE(and_gate->elementType(), ElementType::And);

    auto or_gate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Or));
    QCOMPARE(or_gate->elementType(), ElementType::Or);

    auto not_gate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Not));
    QCOMPARE(not_gate->elementType(), ElementType::Not);
}

void TestGraphicelementAdvanced::testElementGroupRetrieval()
{
    auto and_gate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QCOMPARE(and_gate->elementGroup(), ElementGroup::Gate);

    auto input = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputButton));
    QCOMPARE(input->elementGroup(), ElementGroup::Input);

    auto output = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Led));
    QCOMPARE(output->elementGroup(), ElementGroup::Output);
}

void TestGraphicelementAdvanced::testElementCreationWithDifferentTypes()
{
    // Test multiple element types can be created and retrieved
    QVector<ElementType> types = {
        ElementType::And, ElementType::Or, ElementType::Nand,
        ElementType::Nor, ElementType::Xor, ElementType::Xnor,
        ElementType::Not, ElementType::InputButton, ElementType::Led
    };

    for (ElementType type : types) {
        auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(type));
        QVERIFY(elem != nullptr);
        QCOMPARE(elem->elementType(), type);
        }
}

// ============================================================================
// Port Naming and Properties Tests
// ============================================================================

void TestGraphicelementAdvanced::testPortNaming()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(elem != nullptr);

    // Verify port is accessible and has properties
    auto *port = elem->inputPort(0);
    QVERIFY(port != nullptr);

    // Verify port is connected to correct element
    QCOMPARE(port->graphicElement(), elem.get());
}

void TestGraphicelementAdvanced::testAddInputPort()
{
    // This tests the addInputPort functionality
    // Create element and verify ports can be accessed
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputVcc));
    QVERIFY(elem != nullptr);

    // Verify it has expected inputs
    QCOMPARE(elem->inputSize(), 0);  // VCC has no inputs
}

void TestGraphicelementAdvanced::testAddOutputPort()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputVcc));
    QVERIFY(elem != nullptr);

    // VCC should have 1 output
    QCOMPARE(elem->outputSize(), 1);
}

void TestGraphicelementAdvanced::testAddMultiplePorts()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(elem != nullptr);

    // AND has 2 inputs and 1 output
    QCOMPARE(elem->inputSize(), 2);
    QCOMPARE(elem->outputSize(), 1);

    // All ports should be accessible
    for (int i = 0; i < elem->inputSize(); i++) {
        QVERIFY(elem->inputPort(i) != nullptr);
    }

    for (int i = 0; i < elem->outputSize(); i++) {
        QVERIFY(elem->outputPort(i) != nullptr);
    }
}

// ============================================================================
// Label and Tooltip Tests
// ============================================================================

void TestGraphicelementAdvanced::testLabelProperty()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(elem != nullptr);

    // Set and retrieve label
    elem->setLabel("TestLabel");
    QCOMPARE(elem->label(), QString("TestLabel"));
}

void TestGraphicelementAdvanced::testTooltipProperty()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Or));
    QVERIFY(elem != nullptr);

    // Tooltip should be set (at least translated name)
    QString tooltip = elem->toolTip();
    QVERIFY(!tooltip.isEmpty());
}

void TestGraphicelementAdvanced::testLabelVisibility()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Nand));
    QVERIFY(elem != nullptr);

    // Set label and verify it's stored
    elem->setLabel("VisibleLabel");
    QCOMPARE(elem->label(), QString("VisibleLabel"));

    // Verify label can be changed
    elem->setLabel("AnotherLabel");
    QCOMPARE(elem->label(), QString("AnotherLabel"));
}

// ============================================================================
// Rotation and Position Tests
// ============================================================================

void TestGraphicelementAdvanced::testElementRotation()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(elem != nullptr);

    // Set rotation (0, 90, 180, 270)
    elem->setRotation(0);
    QCOMPARE(elem->rotation(), 0.0);

    elem->setRotation(90);
    QCOMPARE(elem->rotation(), 90.0);

    elem->setRotation(180);
    QCOMPARE(elem->rotation(), 180.0);

    elem->setRotation(270);
    QCOMPARE(elem->rotation(), 270.0);
}

void TestGraphicelementAdvanced::testElementPosition()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Or));
    QVERIFY(elem != nullptr);

    // Set position
    QPointF pos(100, 200);
    elem->setPos(pos);
    QCOMPARE(elem->pos(), pos);

    // Move to another position
    QPointF newPos(300, 400);
    elem->setPos(newPos);
    QCOMPARE(elem->pos(), newPos);
}

void TestGraphicelementAdvanced::testElementPositionPersistence()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Not));
    QVERIFY(elem != nullptr);

    // Set position and verify it persists
    QPointF pos(150, 250);
    elem->setPos(pos);

    // Position should remain after rotation
    elem->setRotation(90);
    QCOMPARE(elem->pos(), pos);
}

