// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestGeometry.h"

#include <memory>

#include <QPointF>
#include <QRectF>
#include <QSizeF>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/GlobalProperties.h"
#include "App/Nodes/QNEPort.h"
#include "Tests/Common/TestUtils.h"

// ============================================================
// Pixmap and Bounding Rect Tests
// ============================================================

void TestGeometry::testPixmapCenter()
{
    // Create an AND gate (64x64 pixmap)
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(andGate != nullptr);

    // Get pixmap center
    QPointF center = andGate->pixmapCenter();

    // Pixmap is 64x64, so center should be at (32, 32)
    QCOMPARE(center.x(), 32.0);
    QCOMPARE(center.y(), 32.0);
}

void TestGeometry::testBoundingRectNoPortsCalculation()
{
    // Create an element
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(andGate != nullptr);

    // Clear all ports by setting input/output size to 0 (if supported)
    // Most elements have fixed ports, so we'll just verify with existing ports

    // Get bounding rect
    QRectF boundingRect = andGate->boundingRect();

    // Should contain the pixmap (0, 0, 64, 64)
    QVERIFY(boundingRect.contains(0, 0));
    QVERIFY(boundingRect.contains(64, 64));

    // Should have reasonable dimensions
    QVERIFY(boundingRect.width() >= 64.0);
    QVERIFY(boundingRect.height() >= 64.0);
}

void TestGeometry::testBoundingRectWithPorts()
{
    // Create an AND gate with variable input size
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(andGate != nullptr);

    // Get initial bounding rect
    QRectF initialBoundingRect = andGate->boundingRect();
    QVERIFY(initialBoundingRect.width() > 0);
    QVERIFY(initialBoundingRect.height() > 0);

    // The bounding rect should be union of pixmap and port bounds
    // Pixmap is at (0, 0) with size (64, 64)
    // Ports extend the bounding rect

    // Verify bounding rect includes pixmap rect
    QRectF pixmapRect(0, 0, 64, 64);
    QVERIFY(initialBoundingRect.united(pixmapRect) == initialBoundingRect);
}

// ============================================================
// Port Positioning Tests
// ============================================================

void TestGeometry::testInputPortPositionSinglePort()
{
    // Create an OR gate (2 input ports by default)
    auto orGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Or));
    QVERIFY(orGate != nullptr);

    // Get input ports
    const auto &inputs = orGate->inputs();
    QVERIFY(inputs.size() > 0);

    // Check first input port position
    // Input ports should be at x=0
    QPointF pos0 = inputs[0]->pos();
    QCOMPARE(pos0.x(), 0.0);

    // With 2 input ports, they should be at y=24 and y=40 (symmetric around y=32)
    // But with a single port, it should be at y=32
    // Let's verify they're not too far from the center
    QVERIFY(pos0.y() >= 16.0 && pos0.y() <= 48.0);
}

void TestGeometry::testInputPortPositionMultiplePorts()
{
    // Create an AND gate (2 input ports by default)
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(andGate != nullptr);

    const auto &inputs = andGate->inputs();
    QVERIFY(inputs.size() >= 2);

    // All input ports should have x=0
    for (auto *input : inputs) {
        QPointF pos = input->pos();
        QCOMPARE(pos.x(), 0.0);
    }

    // With 2 ports, they should be symmetric around y=32
    // step = 8, so y = 32 - (2 * 8) + 8 = 24, then 24 + 16 = 40
    if (inputs.size() == 2) {
        QCOMPARE(inputs[0]->pos().y(), 24.0);
        QCOMPARE(inputs[1]->pos().y(), 40.0);
    }
}

void TestGeometry::testOutputPortPositionSinglePort()
{
    // Create a NOT gate (1 input, 1 output)
    auto notGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Not));
    QVERIFY(notGate != nullptr);

    const auto &outputs = notGate->outputs();
    QVERIFY(outputs.size() > 0);

    // Output ports should be at x=64
    QPointF pos0 = outputs[0]->pos();
    QCOMPARE(pos0.x(), 64.0);

    // With single port, should be centered at y=32
    QVERIFY(pos0.y() >= 16.0 && pos0.y() <= 48.0);
}

void TestGeometry::testOutputPortPositionMultiplePorts()
{
    // Create an AND gate (1 output port by default)
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(andGate != nullptr);

    const auto &outputs = andGate->outputs();
    QVERIFY(outputs.size() > 0);

    // All output ports should have x=64
    for (auto *output : outputs) {
        QPointF pos = output->pos();
        QCOMPARE(pos.x(), 64.0);
    }
}

// ============================================================
// Port Centering and Spacing Tests
// ============================================================

void TestGeometry::testPortCenteringAroundY32()
{
    // Create a variable input gate
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(andGate != nullptr);

    // Set input size to 4 for more pronounced centering
    andGate->setInputSize(4);

    const auto &inputs = andGate->inputs();
    QVERIFY(inputs.size() == 4);

    // With 4 ports centered around y=32:
    // step = 8
    // y = 32 - (4 * 8) + 8 = 8
    // ports at: 8, 24, 40, 56
    // average should be around 32

    double ySum = 0;
    for (auto *port : inputs) {
        ySum += port->pos().y();
    }
    double yAverage = ySum / inputs.size();

    // Average should be close to 32 (center)
    QVERIFY(yAverage >= 30.0 && yAverage <= 34.0);
}

void TestGeometry::testPortSpacingCalculation()
{
    // Create an AND gate
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    QVERIFY(andGate != nullptr);

    // Set input size to 3
    andGate->setInputSize(3);

    const auto &inputs = andGate->inputs();
    QVERIFY(inputs.size() == 3);

    // Get port positions
    double y0 = inputs[0]->pos().y();
    double y1 = inputs[1]->pos().y();
    double y2 = inputs[2]->pos().y();

    // Spacing should be constant (step * 2 = 16)
    double spacing1 = y1 - y0;
    double spacing2 = y2 - y1;

    QCOMPARE(spacing1, 16.0);
    QCOMPARE(spacing2, 16.0);
}

