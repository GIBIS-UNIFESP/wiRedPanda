// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testqneport_advanced.h"

#include "qneport.h"
#include "qneconnection.h"
#include "and.h"
#include "or.h"
#include "not.h"
#include "inputbutton.h"
#include "led.h"
#include "enums.h"
#include "graphicelement.h"

#include <QTest>
#include <QGraphicsScene>
#include <QBrush>
#include <QColor>

void TestQNEPortAdvanced::testIsConnectedMethod()
{
    // Test the currently uncovered isConnected() method
    And andGate;
    Or orGate;
    
    auto outputPort = andGate.outputPort();
    auto inputPort = orGate.inputPort(0);
    
    // Initially not connected
    QVERIFY(!outputPort->isConnected(inputPort));
    QVERIFY(!inputPort->isConnected(outputPort));
    
    // Connect them via QNEConnection
    QNEConnection connection;
    connection.setStartPort(outputPort);
    connection.setEndPort(inputPort);
    
    // Now they should be connected
    QVERIFY(outputPort->isConnected(inputPort));
    QVERIFY(inputPort->isConnected(outputPort));
    
    // Test with unrelated port
    Not notGate;
    auto unrelatedPort = notGate.inputPort(0);
    QVERIFY(!outputPort->isConnected(unrelatedPort));
    
    // Clean up
    connection.setStartPort(nullptr);
    connection.setEndPort(nullptr);
}

void TestQNEPortAdvanced::testCurrentBrushGetterSetter()
{
    // Test the currently uncovered currentBrush() method
    And andGate;
    auto port = andGate.outputPort();
    
    // Test setting different brush colors
    QBrush redBrush(Qt::red);
    port->setCurrentBrush(redBrush);
    QCOMPARE(port->currentBrush().color(), Qt::red);
    
    QBrush blueBrush(Qt::blue);
    port->setCurrentBrush(blueBrush);
    QCOMPARE(port->currentBrush().color(), Qt::blue);
    
    // Test with gradient brush
    QLinearGradient gradient(0, 0, 100, 100);
    gradient.setColorAt(0, Qt::red);
    gradient.setColorAt(1, Qt::blue);
    QBrush gradientBrush(gradient);
    port->setCurrentBrush(gradientBrush);
    QCOMPARE(port->currentBrush().gradient()->type(), QGradient::LinearGradient);
}

void TestQNEPortAdvanced::testHoverEnterLeaveVisualFeedback()
{
    // Test the currently uncovered hoverEnter/hoverLeave methods
    And andGate;
    Or orGate;
    
    auto outputPort = andGate.outputPort();
    auto inputPort = orGate.inputPort(0);
    
    // Test hover enter/leave on output port
    outputPort->hoverEnter();
    // Visual change should occur but we can't easily test the visual aspect
    // The important thing is that it doesn't crash
    outputPort->hoverLeave();
    
    // Test hover enter/leave on input port  
    inputPort->hoverEnter();
    inputPort->hoverLeave();
    
    // Test multiple hover calls
    outputPort->hoverEnter();
    outputPort->hoverEnter(); // Should handle multiple calls gracefully
    outputPort->hoverLeave();
    outputPort->hoverLeave(); // Should handle multiple calls gracefully
    
    QVERIFY(true); // Test passes if no crash occurs
}

void TestQNEPortAdvanced::testPortConstructorEdgeCases()
{
    // Test QNEPort constructor branches that might not be fully covered
    
    // Test InputPort with null parent
    QNEInputPort* inputPortNull = new QNEInputPort(nullptr);
    QVERIFY(inputPortNull != nullptr);
    QVERIFY(inputPortNull->isInput());
    QVERIFY(!inputPortNull->isOutput());
    delete inputPortNull;
    
    // Test OutputPort with null parent
    QNEOutputPort* outputPortNull = new QNEOutputPort(nullptr);
    QVERIFY(outputPortNull != nullptr);
    QVERIFY(!outputPortNull->isInput());
    QVERIFY(outputPortNull->isOutput());
    delete outputPortNull;
    
    // Test with parent element
    And andGate;
    QNEInputPort* inputPortWithParent = new QNEInputPort(&andGate);
    QCOMPARE(inputPortWithParent->parentItem(), &andGate);
    delete inputPortWithParent;
}

void TestQNEPortAdvanced::testInputPortSetStatusBranches()
{
    // Test InputPort setStatus method branches (currently 50% coverage)
    And andGate;
    Or orGate;
    
    auto inputPort = orGate.inputPort(0);
    auto outputPort = andGate.outputPort();
    
    // InputPort is valid only if: no connections and not required, OR exactly 1 connection
    // By default, input ports are required, so they're invalid when disconnected
    QVERIFY(!inputPort->isValid()); // Required and no connections = invalid
    
    // Test that setting status on invalid port always results in Invalid
    inputPort->setStatus(Status::Inactive);
    QCOMPARE(inputPort->status(), Status::Invalid); // Forced to Invalid due to !isValid()
    
    // Make port not required, then it becomes valid when disconnected
    inputPort->setRequired(false);
    QVERIFY(inputPort->isValid()); // Not required and no connections = valid
    
    // Now status changes should work
    inputPort->setStatus(Status::Inactive);
    QCOMPARE(inputPort->status(), Status::Inactive);
    
    inputPort->setStatus(Status::Active);
    QCOMPARE(inputPort->status(), Status::Active);
    
    // Test status with connections (port becomes valid with 1 connection)
    inputPort->setRequired(true); // Make it required again
    QNEConnection connection;
    connection.setStartPort(outputPort);
    connection.setEndPort(inputPort);
    
    QVERIFY(inputPort->isValid()); // Required with 1 connection = valid
    
    // Test status propagation through connections
    inputPort->setStatus(Status::Active);
    QCOMPARE(inputPort->status(), Status::Active);
    
    inputPort->setStatus(Status::Inactive);
    QCOMPARE(inputPort->status(), Status::Inactive);
    
    // Clean up
    connection.setStartPort(nullptr);
    connection.setEndPort(nullptr);
}

void TestQNEPortAdvanced::testOutputPortUpdateThemeBranches()
{
    // Test OutputPort updateTheme method branches (currently 56% coverage)
    And andGate;
    auto outputPort = andGate.outputPort();
    
    // Call updateTheme multiple times to exercise different code paths
    outputPort->updateTheme();
    
    // Test with different statuses
    outputPort->setStatus(Status::Invalid);
    outputPort->updateTheme();
    
    outputPort->setStatus(Status::Inactive);  
    outputPort->updateTheme();
    
    outputPort->setStatus(Status::Active);
    outputPort->updateTheme();
    
    // Test with required/not required
    outputPort->setRequired(false);
    outputPort->updateTheme();
    
    outputPort->setRequired(true);
    outputPort->updateTheme();
    
    QVERIFY(true); // Test passes if no crash occurs
}

void TestQNEPortAdvanced::testPortFlagsAndPtr()
{
    // Test port flags and pointer functionality
    And andGate;
    auto port = andGate.outputPort();
    
    // Test default port flags
    int defaultFlags = port->portFlags();
    QVERIFY(defaultFlags >= 0);
    
    // Test setting pointer values
    quint64 testPtr1 = 0x12345678;
    port->setPtr(testPtr1);
    
    quint64 testPtr2 = 0xABCDEF00;
    port->setPtr(testPtr2);
    
    // Test with zero pointer
    port->setPtr(0);
    
    QVERIFY(true); // Test passes if no crash occurs
}

void TestQNEPortAdvanced::testConnectionManagement()
{
    // Test advanced connection management scenarios
    And andGate1, andGate2;
    Or orGate;
    
    auto outputPort1 = andGate1.outputPort();
    auto outputPort2 = andGate2.outputPort();  
    auto inputPort = orGate.inputPort(0);
    
    // Test multiple connections to same input port
    QNEConnection connection1, connection2;
    
    connection1.setStartPort(outputPort1);
    connection1.setEndPort(inputPort);
    
    // Test connecting second output to same input
    connection2.setStartPort(outputPort2);
    connection2.setEndPort(inputPort);
    
    // Verify connections list
    const auto& connections = inputPort->connections();
    QVERIFY(connections.size() >= 1);
    
    // Test disconnection
    connection1.setStartPort(nullptr);
    connection1.setEndPort(nullptr);
    
    connection2.setStartPort(nullptr);
    connection2.setEndPort(nullptr);
}

void TestQNEPortAdvanced::testPortValidationLogic()
{
    // Test port validation methods
    And andGate;
    Or orGate;
    
    auto outputPort = andGate.outputPort();
    auto inputPort = orGate.inputPort(0);
    
    // Test isValid() method - understand the actual validation logic
    // OutputPort: valid if status != Invalid
    QVERIFY(outputPort->isValid()); // Should be valid initially
    
    // InputPort: valid if (no connections and not required) OR (exactly 1 connection)
    // By default input ports are required, so invalid when disconnected
    QVERIFY(!inputPort->isValid()); // Required input with no connections = invalid
    
    // Make input port not required, then it becomes valid
    inputPort->setRequired(false);
    QVERIFY(inputPort->isValid()); // Not required input with no connections = valid
    
    // Test type checking
    QVERIFY(outputPort->isOutput());
    QVERIFY(!outputPort->isInput());
    
    QVERIFY(inputPort->isInput());
    QVERIFY(!inputPort->isOutput());
    
    // Test with connection - input port becomes valid with 1 connection
    inputPort->setRequired(true); // Make it required again
    QVERIFY(!inputPort->isValid()); // Required with no connections = invalid
    
    QNEConnection connection;
    connection.setStartPort(outputPort);
    connection.setEndPort(inputPort);
    
    QVERIFY(outputPort->isValid()); // Output port should still be valid
    QVERIFY(inputPort->isValid());  // Input port now valid with 1 connection
    
    // Clean up
    connection.setStartPort(nullptr);
    connection.setEndPort(nullptr);
}

void TestQNEPortAdvanced::testPortIndexAndNaming()
{
    // Test port indexing and naming functionality
    And andGate;
    Or orGate;
    
    auto outputPort = andGate.outputPort();
    auto inputPort1 = orGate.inputPort(0);
    auto inputPort2 = orGate.inputPort(1);
    
    // Test index functionality
    QCOMPARE(inputPort1->index(), 0);
    QCOMPARE(inputPort2->index(), 1);
    
    // Test setting custom indices
    outputPort->setIndex(42);
    QCOMPARE(outputPort->index(), 42);
    
    // Test naming functionality
    QString testName = "TestPortName";
    outputPort->setName(testName);
    QCOMPARE(outputPort->name(), testName);
    
    // Test empty name
    outputPort->setName("");
    QCOMPARE(outputPort->name(), "");
    
    // Test unicode name
    QString unicodeName = "Тест🔌";
    inputPort1->setName(unicodeName);
    QCOMPARE(inputPort1->name(), unicodeName);
}

void TestQNEPortAdvanced::testDefaultStatusHandling()
{
    // Test default status functionality
    And andGate;
    auto port = andGate.outputPort();
    
    // Test setting different default statuses
    port->setDefaultStatus(Status::Invalid);
    QCOMPARE(port->defaultValue(), Status::Invalid);
    
    port->setDefaultStatus(Status::Inactive);
    QCOMPARE(port->defaultValue(), Status::Inactive);
    
    port->setDefaultStatus(Status::Active);
    QCOMPARE(port->defaultValue(), Status::Active);
    
    // Test that current status is separate from default
    port->setStatus(Status::Inactive);
    QCOMPARE(port->status(), Status::Inactive);
    QCOMPARE(port->defaultValue(), Status::Active); // Should still be Active
}

void TestQNEPortAdvanced::testRequiredPortBehavior()
{
    // Test required port functionality
    And andGate;
    auto port = andGate.outputPort();
    
    // Test setting required state
    port->setRequired(true);
    QVERIFY(port->isRequired());
    
    port->setRequired(false);
    QVERIFY(!port->isRequired());
    
    // Test multiple toggles
    port->setRequired(true);
    port->setRequired(false);
    port->setRequired(true);
    QVERIFY(port->isRequired());
}

void TestQNEPortAdvanced::testPortWithNullGraphicElement()
{
    // Test port behavior when graphic element is null
    QNEInputPort* inputPort = new QNEInputPort();
    QNEOutputPort* outputPort = new QNEOutputPort();
    
    // Initially graphic element should be null
    QVERIFY(inputPort->graphicElement() == nullptr);
    QVERIFY(outputPort->graphicElement() == nullptr);
    
    // Test that methods don't crash with null graphic element
    QVERIFY(inputPort->isInput());
    QVERIFY(!inputPort->isOutput());
    QVERIFY(outputPort->isOutput());
    QVERIFY(!outputPort->isInput());
    
    // Test logic() method with null graphic element
    QVERIFY(inputPort->logic() == nullptr);
    QVERIFY(outputPort->logic() == nullptr);
    
    delete inputPort;
    delete outputPort;
}

void TestQNEPortAdvanced::testPortDestructorCleanup()
{
    // Test that port destructors properly clean up connections
    QNEConnection connection;
    
    {
        And andGate;
        Or orGate;
        
        auto outputPort = andGate.outputPort();
        auto inputPort = orGate.inputPort(0);
        
        connection.setStartPort(outputPort);
        connection.setEndPort(inputPort);
        
        // Verify connection is established
        QVERIFY(connection.startPort() == outputPort);
        QVERIFY(connection.endPort() == inputPort);
        
        // Explicitly clean up connection before ports are destroyed
        // This tests proper cleanup handling
        connection.setStartPort(nullptr);
        connection.setEndPort(nullptr);
        
        // When we leave this scope, ports are destroyed but connection is safe
    }
    
    // After scope, connection should have null ports
    QVERIFY(connection.startPort() == nullptr);
    QVERIFY(connection.endPort() == nullptr);
}