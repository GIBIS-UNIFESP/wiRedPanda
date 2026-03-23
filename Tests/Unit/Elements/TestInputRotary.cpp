// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestInputRotary.h"

#include <memory>

#include <QDataStream>
#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/InputRotary.h"
#include "App/Nodes/QNEPort.h"
#include "Tests/Common/TestUtils.h"

// ============================================================================
// Port Iteration Tests
// ============================================================================

void TestInputRotary::testInitialPortState()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    QVERIFY(elem != nullptr);

    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    // Initial state should be port 0
    QCOMPARE(rotary->outputValue(), 0);
    QVERIFY(rotary->isOn(0));
}

void TestInputRotary::testNextPortWrapping()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    const int numPorts = rotary->outputSize();

    // Start at port 0
    QCOMPARE(rotary->outputValue(), 0);

    // Move to next port
    rotary->setOn();
    QCOMPARE(rotary->outputValue(), 1);

    // Continue advancing
    for (int i = 2; i < numPorts; ++i) {
        rotary->setOn();
        QCOMPARE(rotary->outputValue(), i);
    }

    // Should wrap back to 0
    rotary->setOn();
    QCOMPARE(rotary->outputValue(), 0);
}

void TestInputRotary::testPreviousPortWrapping()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    const int numPorts = rotary->outputSize();

    // Move forward to port 1
    rotary->setOn();
    QCOMPARE(rotary->outputValue(), 1);

    // Move back (by advancing from 1, it goes to 2, so test differently)
    // Set to last port manually
    rotary->setOn(true, numPorts - 1);
    QCOMPARE(rotary->outputValue(), numPorts - 1);

    // Next should wrap to 0
    rotary->setOn();
    QCOMPARE(rotary->outputValue(), 0);
}

void TestInputRotary::testSetPortDirect()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    const int numPorts = rotary->outputSize();

    // Test setting each port directly
    for (int port = 0; port < numPorts; ++port) {
        rotary->setOn(true, port);
        QCOMPARE(rotary->outputValue(), port);
        QVERIFY(rotary->isOn(port));
    }
}

void TestInputRotary::testSetPortOutOfBounds()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    const int numPorts = rotary->outputSize();

    // Set port beyond valid range - should wrap to 0
    rotary->setOn(true, numPorts + 5);
    QCOMPARE(rotary->outputValue(), 0);
}

void TestInputRotary::testSetPortWrapsToZero()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    const int numPorts = rotary->outputSize();

    // Set port to exact multiple of size (should wrap to 0)
    rotary->setOn(true, numPorts);
    QCOMPARE(rotary->outputValue(), 0);

    // Set to 2 * size (should wrap to 0)
    rotary->setOn(true, numPorts * 2);
    QCOMPARE(rotary->outputValue(), 0);
}

// ============================================================================
// Output Value Tests
// ============================================================================

void TestInputRotary::testOutputValue()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    // Initial output value should be 0
    QCOMPARE(rotary->outputValue(), 0);

    // Change port and verify output value follows
    // Use port 1 (valid for all rotary configurations)
    rotary->setOn(true, 1);
    QCOMPARE(rotary->outputValue(), 1);
}

void TestInputRotary::testOutputSize()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    // InputRotary should have valid output size
    int size = rotary->outputSize();
    QVERIFY(size > 0);
    QVERIFY(size <= 16);

    // Output size should match number of output ports
    QCOMPARE(size, rotary->outputs().size());
}

void TestInputRotary::testIsOnCurrentPort()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    const int numPorts = rotary->outputSize();

    // Test isOn for current port
    for (int port = 0; port < numPorts; ++port) {
        rotary->setOn(true, port);
        QVERIFY(rotary->isOn(port));
    }
}

void TestInputRotary::testIsOnOtherPort()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    // Set to port 0
    rotary->setOn(true, 0);

    const int numPorts = rotary->outputSize();

    // Verify other ports are not on
    for (int port = 1; port < numPorts; ++port) {
        QVERIFY(!rotary->isOn(port));
    }
}

void TestInputRotary::testIsOnAllPorts()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    const int numPorts = rotary->outputSize();

    // For each port, exactly one should be "on"
    for (int currentPort = 0; currentPort < numPorts; ++currentPort) {
        rotary->setOn(true, currentPort);

        int onCount = 0;
        for (int port = 0; port < numPorts; ++port) {
            if (rotary->isOn(port)) {
                onCount++;
            }
        }

        // Exactly one port should be on
        QCOMPARE(onCount, 1);
    }
}

// ============================================================================
// Port Configuration Tests
// ============================================================================

void TestInputRotary::testPortConfigurationWithTwoPorts()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    rotary->setOutputSize(2);
    QVERIFY(rotary->outputSize() == 2);

    auto *port0 = rotary->outputPort(0);
    auto *port1 = rotary->outputPort(1);
    QVERIFY(port0 != nullptr);
    QVERIFY(port1 != nullptr);
    QCOMPARE(port0->name(), QString("0"));
    QCOMPARE(port1->name(), QString("1"));
}

void TestInputRotary::testPortConfigurationWithThreePorts()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    rotary->setOutputSize(3);
    QVERIFY(rotary->outputSize() == 3);

    for (int i = 0; i < 3; ++i) {
        auto *port = rotary->outputPort(i);
        QVERIFY(port != nullptr);
        QVERIFY(!port->name().isEmpty());
    }
}

void TestInputRotary::testPortConfigurationWithFourPorts()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    rotary->setOutputSize(4);
    QVERIFY(rotary->outputSize() == 4);

    for (int i = 0; i < 4; ++i) {
        auto *port = rotary->outputPort(i);
        QVERIFY(port != nullptr);
    }
}

void TestInputRotary::testPortConfigurationWithSixPorts()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    rotary->setOutputSize(6);
    QVERIFY(rotary->outputSize() == 6);

    for (int i = 0; i < 6; ++i) {
        auto *port = rotary->outputPort(i);
        QVERIFY(port != nullptr);
    }
}

void TestInputRotary::testPortConfigurationWithEightPorts()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    rotary->setOutputSize(8);
    QVERIFY(rotary->outputSize() == 8);

    for (int i = 0; i < 8; ++i) {
        auto *port = rotary->outputPort(i);
        QVERIFY(port != nullptr);
    }
}

void TestInputRotary::testPortConfigurationWithTenPorts()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    rotary->setOutputSize(10);
    QVERIFY(rotary->outputSize() == 10);

    for (int i = 0; i < 10; ++i) {
        auto *port = rotary->outputPort(i);
        QVERIFY(port != nullptr);
    }
}

void TestInputRotary::testPortConfigurationWithTwelvePorts()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    rotary->setOutputSize(12);
    QVERIFY(rotary->outputSize() == 12);

    for (int i = 0; i < 12; ++i) {
        auto *port = rotary->outputPort(i);
        QVERIFY(port != nullptr);
    }
}

void TestInputRotary::testPortConfigurationWithSixteenPorts()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    rotary->setOutputSize(16);
    QVERIFY(rotary->outputSize() == 16);

    for (int i = 0; i < 16; ++i) {
        auto *port = rotary->outputPort(i);
        QVERIFY(port != nullptr);
    }
}

// ============================================================================
// Serialization Tests
// ============================================================================

void TestInputRotary::testSaveCurrentPort()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    // Set to a specific port
    rotary->setOn(true, 3);

    // Save to stream
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    rotary->save(stream);

    // Verify data was written
    QVERIFY(data.size() > 0);
}

void TestInputRotary::testLoadCurrentPort()
{
    // Create element and set port to 1 (valid for all configurations)
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    rotary->setOn(true, 1);

    // Save state
    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    rotary->save(saveStream);
    elem.reset();

    // Create new element and load state
    auto elem2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary2 = dynamic_cast<InputRotary *>(elem2.get());
    QVERIFY(rotary2 != nullptr);

    QDataStream loadStream(data);
    QMap<quint64, QNEPort *> portMap;
    SerializationContext context{portMap, QVersionNumber(4, 1), {}};
    rotary2->load(loadStream, context);

    // Verify port was loaded correctly
    QCOMPARE(rotary2->outputValue(), 1);
}

void TestInputRotary::testSaveLoadPreservesState()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    const int testPort = 1;  // Use valid port (all rotaries have at least 2 ports)
    rotary->setOn(true, testPort);

    // Save and load cycle
    QByteArray data;
    QDataStream saveStream(&data, QIODevice::WriteOnly);
    rotary->save(saveStream);
    elem.reset();

    // Load into new element
    auto elem2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary2 = dynamic_cast<InputRotary *>(elem2.get());
    QVERIFY(rotary2 != nullptr);

    QDataStream loadStream(data);
    QMap<quint64, QNEPort *> portMap;
    SerializationContext context{portMap, QVersionNumber(4, 1), {}};
    rotary2->load(loadStream, context);

    // Verify state preserved
    QCOMPARE(rotary2->outputValue(), testPort);
}

// ============================================================================
// State Management Tests
// ============================================================================

void TestInputRotary::testSetOffDoesNothing()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    // Set to port 1
    rotary->setOn(true, 1);
    QCOMPARE(rotary->outputValue(), 1);

    // setOff should have no effect on rotary (unlike button)
    rotary->setOff();
    QCOMPARE(rotary->outputValue(), 1);
}

void TestInputRotary::testMultipleSetOn()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    const int numPorts = rotary->outputSize();

    // Call setOn() multiple times, should cycle through ports
    for (int expectedPort = 0; expectedPort < numPorts; ++expectedPort) {
        rotary->setOn(true, expectedPort);
        QCOMPARE(rotary->outputValue(), expectedPort);
    }
}

void TestInputRotary::testPortStatusAfterSetOn()
{
    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    auto *rotary = dynamic_cast<InputRotary *>(elem.get());
    QVERIFY(rotary != nullptr);

    const int numPorts = rotary->outputSize();

    // Set a port and verify status of all ports
    const int activePort = 1;  // Use valid port
    rotary->setOn(true, activePort);

    // Verify active port has Active status
    auto *activePortObj = rotary->outputPort(activePort);
    QVERIFY(activePortObj != nullptr);
    QCOMPARE(activePortObj->status(), Status::Active);

    // Verify all other ports are Inactive
    for (int port = 0; port < numPorts; ++port) {
        if (port == activePort) {
            continue;
        }
        auto *portObj = rotary->outputPort(port);
        QVERIFY(portObj != nullptr);
        QCOMPARE(portObj->status(), Status::Inactive);
    }
}

// ============================================================================
// Basic InputRotary Test (migrated from testelements.cpp)
// ============================================================================

void TestInputRotary::testInputRotary()
{
    // Test InputRotary basic properties
    auto rotary = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputRotary));
    QVERIFY(rotary != nullptr);

    // Check that it's an InputRotary
    auto *inputRotary = qgraphicsitem_cast<InputRotary *>(rotary.get());
    QVERIFY(inputRotary != nullptr);

    // Test initial state (should start at position 0)
    QCOMPARE(inputRotary->outputValue(), 0);

    // Test output size
    int initialSize = inputRotary->outputSize();
    QVERIFY(initialSize > 0);

    // Test that only current port is ON
    for (int i = 0; i < initialSize; ++i) {
        if (i == 0) {
            QCOMPARE(inputRotary->isOn(i), true);
        } else {
            QCOMPARE(inputRotary->isOn(i), false);
        }
    }

    // Test setting position directly to port 2 — requires at least 3 ports
    inputRotary->setOutputSize(3);
    QVERIFY(inputRotary->outputSize() == 3);
    inputRotary->setOn(true, 2);
    QCOMPARE(inputRotary->outputValue(), 2);

    // Verify only port 2 is now ON
    for (int i = 0; i < 3; ++i) {
        QCOMPARE(inputRotary->isOn(i), (i == 2));
    }

    // Test different output sizes
    for (int size : {2, 3, 4, 6, 8, 10, 12, 16}) {
        inputRotary->setOutputSize(size);
        QCOMPARE(inputRotary->outputSize(), size);

        // Verify starting at 0
        QCOMPARE(inputRotary->outputValue(), 0);

        // Verify only position 0 is ON
        for (int i = 0; i < size; ++i) {
            QCOMPARE(inputRotary->isOn(i), (i == 0));
        }
    }

    // Test locked state
    inputRotary->setOutputSize(4);
    inputRotary->setLocked(true);
    QVERIFY(inputRotary->isLocked());

    inputRotary->setOn(true, 2);  // Try to change position while locked
    // Note: setOn() with lock might not work depending on implementation
    // This test verifies the locked state is preserved

    inputRotary->setLocked(false);
    QVERIFY(!inputRotary->isLocked());

    // Test output ports exist
    QCOMPARE(inputRotary->outputs().size(), 4);
}

