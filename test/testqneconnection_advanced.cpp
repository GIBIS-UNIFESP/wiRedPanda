// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testqneconnection_advanced.h"

#include "qneconnection.h"
#include "qneport.h"
#include "and.h"
#include "or.h"
#include "not.h"
#include "inputbutton.h"
#include "led.h"
#include "enums.h"

#include <QTest>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QDataStream>
#include <QBuffer>
#include <QApplication>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QPixmap>

void TestQNEConnectionAdvanced::testLoadMethodEmptyPortMap()
{
    // Test load() with empty port map - this tests the isEmpty() branch
    And andGate;
    Or orGate;
    
    auto outputPort = andGate.outputPort();
    auto inputPort = orGate.inputPort(0);
    
    // Create connection data
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QDataStream outStream(&buffer);
    
    // Write port pointers directly (simulating legacy format)
    quint64 outputPtr = reinterpret_cast<quint64>(outputPort);
    quint64 inputPtr = reinterpret_cast<quint64>(inputPort);
    outStream << outputPtr << inputPtr;
    
    // Test loading with empty port map
    buffer.close();
    buffer.open(QIODevice::ReadOnly);
    QDataStream inStream(&buffer);
    
    QNEConnection connection;
    QMap<quint64, QNEPort *> emptyPortMap; // Empty map triggers legacy path
    
    connection.load(inStream, emptyPortMap);
    
    // Connection should be loaded but may not have valid port references
    // (depends on whether the pointers are still valid)
    QVERIFY(true); // Test passes if no crash occurs
}

void TestQNEConnectionAdvanced::testLoadMethodWithPortMap()
{
    // Test load() with populated port map - tests the !isEmpty() branch
    And andGate;
    Or orGate;
    
    auto outputPort = andGate.outputPort();
    auto inputPort = orGate.inputPort(0);
    
    // Create port map
    QMap<quint64, QNEPort *> portMap;
    quint64 outputId = reinterpret_cast<quint64>(outputPort);
    quint64 inputId = reinterpret_cast<quint64>(inputPort);
    portMap[outputId] = outputPort;
    portMap[inputId] = inputPort;
    
    // Create serialized data
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QDataStream outStream(&buffer);
    outStream << outputId << inputId;
    
    // Test loading with port map
    buffer.close();
    buffer.open(QIODevice::ReadOnly);
    QDataStream inStream(&buffer);
    
    QNEConnection connection;
    connection.load(inStream, portMap);
    
    // Should have properly connected ports
    QVERIFY(connection.startPort() != nullptr);
    QVERIFY(connection.endPort() != nullptr);
}

void TestQNEConnectionAdvanced::testLoadMethodMissingPortsInMap()
{
    // Test load() with incomplete port map - tests the contains() checks
    And andGate;
    
    auto outputPort = andGate.outputPort();
    
    // Create port map with only one port
    QMap<quint64, QNEPort *> partialPortMap;
    quint64 outputId = reinterpret_cast<quint64>(outputPort);
    partialPortMap[outputId] = outputPort;
    
    // Create data with two port IDs, but map only has one
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QDataStream outStream(&buffer);
    outStream << outputId << quint64(12345); // Invalid second ID
    
    buffer.close();
    buffer.open(QIODevice::ReadOnly);
    QDataStream inStream(&buffer);
    
    QNEConnection connection;
    connection.load(inStream, partialPortMap);
    
    // Should return early due to missing port in map
    QVERIFY(connection.startPort() == nullptr);
    QVERIFY(connection.endPort() == nullptr);
}

void TestQNEConnectionAdvanced::testLoadMethodPortTypeCombinations()
{
    // Test different port type combinations in load method
    And andGate;
    Or orGate;
    
    auto outputPort = andGate.outputPort();
    auto inputPort = orGate.inputPort(0);
    
    QMap<quint64, QNEPort *> portMap;
    quint64 outputId = reinterpret_cast<quint64>(outputPort);
    quint64 inputId = reinterpret_cast<quint64>(inputPort);
    portMap[outputId] = outputPort;
    portMap[inputId] = inputPort;
    
    // Test case 1: port1=input, port2=output (tests first if condition)
    {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        QDataStream outStream(&buffer);
        outStream << inputId << outputId; // Input first, output second
        
        buffer.close();
        buffer.open(QIODevice::ReadOnly);
        QDataStream inStream(&buffer);
        
        QNEConnection connection;
        connection.load(inStream, portMap);
        
        QCOMPARE(connection.startPort(), outputPort);
        QCOMPARE(connection.endPort(), inputPort);
    }
    
    // Test case 2: port1=output, port2=input (tests else if condition)
    {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        QDataStream outStream(&buffer);
        outStream << outputId << inputId; // Output first, input second
        
        buffer.close();
        buffer.open(QIODevice::ReadOnly);
        QDataStream inStream(&buffer);
        
        QNEConnection connection;
        connection.load(inStream, portMap);
        
        QCOMPARE(connection.startPort(), outputPort);
        QCOMPARE(connection.endPort(), inputPort);
    }
}

// Note: sceneEvent() tests removed as method is protected in QNEConnection
// Instead, we'll test public methods that exercise the same code paths

void TestQNEConnectionAdvanced::testSetPortsWithNullValues()
{
    QNEConnection connection;
    
    // Test setting null ports (should handle gracefully)
    connection.setStartPort(nullptr);
    connection.setEndPort(nullptr);
    
    QVERIFY(connection.startPort() == nullptr);
    QVERIFY(connection.endPort() == nullptr);
    
    // Test updating positions with null ports
    connection.updatePosFromPorts();
    QVERIFY(true); // Should not crash
    
    // Test angle calculation with null ports
    QCOMPARE(connection.angle(), 0.0);
}

void TestQNEConnectionAdvanced::testSetPortsReplacingExistingPorts()
{
    QNEConnection connection;
    
    {
        And andGate1, andGate2;
        Or orGate1, orGate2;
        
        auto outputPort1 = andGate1.outputPort();
        auto outputPort2 = andGate2.outputPort();
        auto inputPort1 = orGate1.inputPort(0);
        auto inputPort2 = orGate2.inputPort(0);
        
        // Set initial ports
        connection.setStartPort(outputPort1);
        connection.setEndPort(inputPort1);
        
        // Replace start port (tests oldPort && (oldPort != port) condition)
        connection.setStartPort(outputPort2);
        QCOMPARE(connection.startPort(), outputPort2);
        
        // Replace end port
        connection.setEndPort(inputPort2);
        QCOMPARE(connection.endPort(), inputPort2);
        
        // Test setting same port again (oldPort == port condition)
        connection.setStartPort(outputPort2);
        QCOMPARE(connection.startPort(), outputPort2);
        
        // Clean up before destruction
        connection.setStartPort(nullptr);
        connection.setEndPort(nullptr);
    }
}

void TestQNEConnectionAdvanced::testStatusTransitionsWithEdgeCases()
{
    QNEConnection connection;
    
    // Test all status transitions
    connection.setStatus(Status::Invalid);
    QCOMPARE(connection.status(), Status::Invalid);
    
    connection.setStatus(Status::Inactive);
    QCOMPARE(connection.status(), Status::Inactive);
    
    connection.setStatus(Status::Active);
    QCOMPARE(connection.status(), Status::Active);
    
    // Test setting same status (early return condition)
    connection.setStatus(Status::Active);
    QCOMPARE(connection.status(), Status::Active);
    
    // Test with connected end port in separate scope to avoid double-free
    {
        And andGate;
        Or orGate;
        auto outputPort = andGate.outputPort();
        auto inputPort = orGate.inputPort(0);
        
        connection.setStartPort(outputPort);
        connection.setEndPort(inputPort);
        
        // Test status propagation to end port
        connection.setStatus(Status::Active);
        QCOMPARE(inputPort->status(), Status::Active);
        
        connection.setStatus(Status::Inactive);
        QCOMPARE(inputPort->status(), Status::Inactive);
        
        // Explicitly disconnect before destruction to avoid double-free
        connection.setStartPort(nullptr);
        connection.setEndPort(nullptr);
    }
}

void TestQNEConnectionAdvanced::testItemChangeSelectionHandling()
{
    QNEConnection connection;
    
    // Test with connected ports in separate scope
    {
        And andGate;
        Or orGate;
        
        auto outputPort = andGate.outputPort();
        auto inputPort = orGate.inputPort(0);
        
        connection.setStartPort(outputPort);
        connection.setEndPort(inputPort);
        
        // Test selection change to true
        connection.setSelected(true);
        QVERIFY(connection.isSelected());
        
        // Test selection change to false  
        connection.setSelected(false);
        QVERIFY(!connection.isSelected());
        
        // Clean up connections before scope ends
        connection.setStartPort(nullptr);
        connection.setEndPort(nullptr);
    }
    
    // Test with null ports (edge case)
    connection.setSelected(true);
    connection.setSelected(false);
    QVERIFY(true); // Should not crash with null ports
}

void TestQNEConnectionAdvanced::testAngleCalculationEdgeCases()
{
    QNEConnection connection;
    
    {
        QGraphicsScene scene;
        
        // Create elements on heap so scene can manage them properly
        And* andGate = new And();
        Not* notGate1 = new Not();
        Not* notGate2 = new Not();
        
        scene.addItem(andGate);
        scene.addItem(notGate1);
        scene.addItem(notGate2);
        
        // Position elements
        andGate->setPos(0, 0);
        notGate1->setPos(100, 0);
        notGate2->setPos(200, 0);
        
        // Test with only start port
        connection.setStartPort(andGate->outputPort());
        QCOMPARE(connection.angle(), 0.0);
        
        // Test with swapped ports (port2 is output) - tests the swap condition
        auto input1 = notGate1->inputPort(0);
        auto output2 = notGate2->outputPort();
        
        connection.setStartPort(nullptr); // Clear first
        connection.setEndPort(input1);
        connection.setStartPort(dynamic_cast<QNEOutputPort*>(output2));
        
        // This should trigger the port swap logic in angle()
        double angle = connection.angle();
        QVERIFY(angle >= 0.0 && angle <= 360.0);
        
        // Clean up connections before scene destruction
        connection.setStartPort(nullptr);
        connection.setEndPort(nullptr);
        
        // Scene destructor will clean up the elements
    }
}

void TestQNEConnectionAdvanced::testOtherPortEdgeCases()
{
    QNEConnection connection;
    
    // Test with connected ports in separate scope
    {
        And andGate;
        Or orGate;
        
        auto outputPort = andGate.outputPort();
        auto inputPort = orGate.inputPort(0);
        
        connection.setStartPort(outputPort);
        connection.setEndPort(inputPort);
        
        // Test otherPort with start port
        QCOMPARE(connection.otherPort(outputPort), inputPort);
        
        // Test otherPort with end port
        QCOMPARE(connection.otherPort(inputPort), outputPort);
        
        // Test otherPort with unrelated port
        Not notGate;
        auto unrelatedPort = notGate.inputPort(0);
        QCOMPARE(connection.otherPort(unrelatedPort), outputPort); // Returns start port
        
        // Clean up before scope ends
        connection.setStartPort(nullptr);
        connection.setEndPort(nullptr);
    }
    
    // Test otherPort with null connection
    QNEConnection emptyConnection;
    And testGate;
    auto testPort = testGate.outputPort();
    QVERIFY(emptyConnection.otherPort(testPort) == nullptr); // Returns start port (nullptr)
}

void TestQNEConnectionAdvanced::testPaintMethodWithDifferentStates()
{
    QGraphicsScene scene;
    QNEConnection connection;
    scene.addItem(&connection);
    
    // Set up connection path
    connection.setStartPos(QPointF(0, 0));
    connection.setEndPos(QPointF(100, 100));
    connection.updatePath();
    
    // Test painting with highlight
    QPixmap pixmap(200, 200);
    pixmap.fill(Qt::white);
    QPainter painter(&pixmap);
    
    QStyleOptionGraphicsItem option;
    
    // Test normal state
    connection.paint(&painter, &option, nullptr);
    
    // Test highlighted state (tests m_highLight condition)
    connection.setHighLight(true);
    connection.paint(&painter, &option, nullptr);
    
    // Test selected state (tests isSelected() ternary condition)
    connection.setSelected(true);
    connection.paint(&painter, &option, nullptr);
    
    // Test both highlighted and selected
    connection.setHighLight(true);
    connection.setSelected(true);
    connection.paint(&painter, &option, nullptr);
    
    QVERIFY(true); // Test passes if no crash
}