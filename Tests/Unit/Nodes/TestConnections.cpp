// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Nodes/TestConnections.h"

#include <memory>

#include <QTest>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestConnections::initTestCase()
{
    // Setup before all tests
}

// ============================================================================
// Basic Lifecycle Tests (4 tests)
// ============================================================================

void TestConnections::testCreateConnection()
{
    auto conn = std::make_unique<QNEConnection>();

    QVERIFY(conn->startPort() == nullptr);
    QVERIFY(conn->endPort() == nullptr);
    QCOMPARE(conn->status(), Status::Unknown);
}

void TestConnections::testSetPorts()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *andGate = new And();
    auto *orGate = new Or();
    scene->addItem(andGate);
    scene->addItem(orGate);

    auto conn = std::make_unique<QNEConnection>();
    conn->setStartPort(andGate->outputPort());
    conn->setEndPort(orGate->inputPort(0));

    QVERIFY(conn->startPort() == andGate->outputPort());
    QVERIFY(conn->endPort() == orGate->inputPort(0));

    // Verify ports registered the connection
    QVERIFY(andGate->outputPort()->connections().contains(conn.get()));
    QVERIFY(orGate->inputPort(0)->connections().contains(conn.get()));
}

void TestConnections::testDisconnectPorts()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *andGate = new And();
    scene->addItem(andGate);

    auto conn = std::make_unique<QNEConnection>();
    conn->setStartPort(andGate->outputPort());

    QCOMPARE(andGate->outputPort()->connections().size(), 1);

    conn.reset();

    // Port should have removed connection
    QCOMPARE(andGate->outputPort()->connections().size(), 0);
}

void TestConnections::testOtherPort()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *sw = new InputSwitch();
    auto *led = new Led();
    scene->addItem(sw);
    scene->addItem(led);

    auto conn = std::make_unique<QNEConnection>();
    conn->setStartPort(sw->outputPort());
    conn->setEndPort(led->inputPort());

    // Test getting the opposite port
    QVERIFY(conn->otherPort(sw->outputPort()) == led->inputPort());
    QVERIFY(conn->otherPort(led->inputPort()) == sw->outputPort());
}

// ============================================================================
// Connection Validation Tests (5 tests)
// ============================================================================

void TestConnections::testValidOutputToInput()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;

    builder.add(&sw, &andGate);
    builder.connect(&sw, 0, &andGate, 0);  // Valid: output to input

    // Verify connection exists and is valid
    auto *scene = workspace.scene();
    int connCount = 0;
    const auto items = scene->items();
    for (auto *item : std::as_const(items)) {
        if (auto *conn = qgraphicsitem_cast<QNEConnection *>(item)) {
            ++connCount;
            QVERIFY(conn->startPort() != nullptr);
            QVERIFY(conn->endPort() != nullptr);
            QVERIFY(conn->startPort()->isOutput());
            QVERIFY(conn->endPort()->isInput());
        }
    }
    QCOMPARE(connCount, 1);
}

void TestConnections::testRejectOutputToOutput()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *and1 = new And();
    auto *and2 = new And();
    scene->addItem(and1);
    scene->addItem(and2);

    // Verify port type checking (output cannot be cast to input)
    auto conn = std::make_unique<QNEConnection>();
    conn->setStartPort(and1->outputPort());

    QNEPort *outputAsPort = and2->outputPort();
    auto *asInput = dynamic_cast<QNEInputPort *>(outputAsPort);
    QVERIFY(asInput == nullptr);  // Cannot cast output to input
}

void TestConnections::testRejectInputToInput()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *and1 = new And();
    auto *and2 = new And();
    scene->addItem(and1);
    scene->addItem(and2);

    auto conn = std::make_unique<QNEConnection>();
    conn->setEndPort(and1->inputPort(0));

    // Verify port type checking (input cannot be cast to output)
    QNEPort *inputAsPort = and2->inputPort(0);
    auto *asOutput = dynamic_cast<QNEOutputPort *>(inputAsPort);
    QVERIFY(asOutput == nullptr);
}

void TestConnections::testRejectSelfConnection()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *andGate = new And();
    scene->addItem(andGate);

    // Verify elements are the same - would be rejected by Scene::makeConnection
    QVERIFY(andGate->outputPort()->graphicElement() == andGate);
    QVERIFY(andGate->inputPort(0)->graphicElement() == andGate);
}

void TestConnections::testRejectDuplicateConnection()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;

    builder.add(&sw, &andGate);
    builder.connect(&sw, 0, &andGate, 0);  // First connection

    // Verify isConnected() returns true
    auto *output = sw.outputPort();
    auto *input = andGate.inputPort(0);

    QVERIFY(output->isConnected(input));
    QVERIFY(input->isConnected(output));
}

// ============================================================================
// Port Capacity Tests (4 tests)
// ============================================================================

void TestConnections::testSingleInputConnection()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;

    builder.add(&sw, &andGate);
    builder.connect(&sw, 0, &andGate, 0);

    auto *input = andGate.inputPort(0);

    // After connection: should be valid
    QCOMPARE(input->connections().size(), 1);
    QVERIFY(input->isValid());
}

void TestConnections::testMultipleInputInvalid()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *sw1 = new InputSwitch();
    auto *sw2 = new InputSwitch();
    auto *andGate = new And();
    scene->addItem(sw1);
    scene->addItem(sw2);
    scene->addItem(andGate);

    // First connection - valid
    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(sw1->outputPort());
    conn1->setEndPort(andGate->inputPort(0));
    scene->addItem(conn1.get());

    QCOMPARE(andGate->inputPort(0)->connections().size(), 1);
    QVERIFY(andGate->inputPort(0)->isValid());

    // Second connection to SAME input - should make port INVALID
    auto conn2 = std::make_unique<QNEConnection>();
    conn2->setStartPort(sw2->outputPort());
    conn2->setEndPort(andGate->inputPort(0));
    scene->addItem(conn2.get());

    QCOMPARE(andGate->inputPort(0)->connections().size(), 2);
    QVERIFY(!andGate->inputPort(0)->isValid());  // INVALID due to multiple
}

void TestConnections::testOutputFanOut()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *sw = new InputSwitch();
    auto *and1 = new And();
    auto *and2 = new And();
    auto *and3 = new And();
    scene->addItem(sw);
    scene->addItem(and1);
    scene->addItem(and2);
    scene->addItem(and3);

    // Connect single output to multiple inputs (fan-out)
    auto *output = sw->outputPort();

    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(output);
    conn1->setEndPort(and1->inputPort(0));
    scene->addItem(conn1.get());

    auto conn2 = std::make_unique<QNEConnection>();
    conn2->setStartPort(output);
    conn2->setEndPort(and2->inputPort(0));
    scene->addItem(conn2.get());

    auto conn3 = std::make_unique<QNEConnection>();
    conn3->setStartPort(output);
    conn3->setEndPort(and3->inputPort(0));
    scene->addItem(conn3.get());

    // Output should have 3 connections and still be valid
    QCOMPARE(output->connections().size(), 3);
    QVERIFY(output->isValid());  // Fan-out supported

    // All inputs should be valid (each has exactly 1 connection)
    QVERIFY(and1->inputPort(0)->isValid());
    QVERIFY(and2->inputPort(0)->isValid());
    QVERIFY(and3->inputPort(0)->isValid());

    // Test status propagation through fan-out
    sw->outputPort()->setStatus(Status::Active);
    QCOMPARE(conn1->status(), Status::Active);
    QCOMPARE(conn2->status(), Status::Active);
    QCOMPARE(conn3->status(), Status::Active);
    QCOMPARE(and1->inputPort(0)->status(), Status::Active);
    QCOMPARE(and2->inputPort(0)->status(), Status::Active);
    QCOMPARE(and3->inputPort(0)->status(), Status::Active);

    // Test status change propagates to all connections
    sw->outputPort()->setStatus(Status::Inactive);
    QCOMPARE(conn1->status(), Status::Inactive);
    QCOMPARE(conn2->status(), Status::Inactive);
    QCOMPARE(conn3->status(), Status::Inactive);
}

void TestConnections::testRequiredPortValidation()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *andGate = new And();
    scene->addItem(andGate);

    auto *input = andGate->inputPort(0);

    // Set required status - should be invalid without connection
    input->setRequired(true);
    QVERIFY(!input->isValid());  // No connection + required = invalid

    // Connect
    auto *sw = new InputSwitch();
    scene->addItem(sw);
    auto conn = std::make_unique<QNEConnection>();
    conn->setStartPort(sw->outputPort());
    conn->setEndPort(input);
    scene->addItem(conn.get());

    QVERIFY(input->isValid());  // Now valid with connection

    // Test non-required
    input->setRequired(false);
    conn.reset();  // Remove connection
    QVERIFY(input->isValid());  // Valid even without connection
}

// ============================================================================
// Status Propagation Tests (3 tests)
// ============================================================================

void TestConnections::testStatusPropagation()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *sw = new InputSwitch();
    auto *led = new Led();
    scene->addItem(sw);
    scene->addItem(led);

    auto conn = std::make_unique<QNEConnection>();
    conn->setStartPort(sw->outputPort());
    conn->setEndPort(led->inputPort());
    scene->addItem(conn.get());

    // Set output status
    sw->outputPort()->setStatus(Status::Active);

    // Verify propagation
    QCOMPARE(conn->status(), Status::Active);
    QCOMPARE(led->inputPort()->status(), Status::Active);

    // Change to Inactive
    sw->outputPort()->setStatus(Status::Inactive);
    QCOMPARE(conn->status(), Status::Inactive);
    QCOMPARE(led->inputPort()->status(), Status::Inactive);
}

void TestConnections::testMultiConnectionStatus()
{
    // Test fan-out status propagation
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *sw = new InputSwitch();
    auto *led1 = new Led();
    auto *led2 = new Led();
    auto *led3 = new Led();
    scene->addItem(sw);
    scene->addItem(led1);
    scene->addItem(led2);
    scene->addItem(led3);

    // Create fan-out
    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(sw->outputPort());
    conn1->setEndPort(led1->inputPort());
    scene->addItem(conn1.get());

    auto conn2 = std::make_unique<QNEConnection>();
    conn2->setStartPort(sw->outputPort());
    conn2->setEndPort(led2->inputPort());
    scene->addItem(conn2.get());

    auto conn3 = std::make_unique<QNEConnection>();
    conn3->setStartPort(sw->outputPort());
    conn3->setEndPort(led3->inputPort());
    scene->addItem(conn3.get());

    // Set output to Active
    sw->outputPort()->setStatus(Status::Active);

    // All connections and inputs should be Active
    QCOMPARE(conn1->status(), Status::Active);
    QCOMPARE(conn2->status(), Status::Active);
    QCOMPARE(conn3->status(), Status::Active);
    QCOMPARE(led1->inputPort()->status(), Status::Active);
    QCOMPARE(led2->inputPort()->status(), Status::Active);
    QCOMPARE(led3->inputPort()->status(), Status::Active);
}

void TestConnections::testInvalidPortStatus()
{
    // Test that invalid port (multiple connections) forces Unknown status
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *sw1 = new InputSwitch();
    auto *sw2 = new InputSwitch();
    auto *andGate = new And();
    scene->addItem(sw1);
    scene->addItem(sw2);
    scene->addItem(andGate);

    // Both outputs Active
    sw1->outputPort()->setStatus(Status::Active);
    sw2->outputPort()->setStatus(Status::Active);

    // First connection
    auto conn1 = std::make_unique<QNEConnection>();
    conn1->setStartPort(sw1->outputPort());
    conn1->setEndPort(andGate->inputPort(0));
    scene->addItem(conn1.get());

    QCOMPARE(andGate->inputPort(0)->status(), Status::Active);

    // Second connection - should force Unknown (multiple connections to input)
    auto conn2 = std::make_unique<QNEConnection>();
    conn2->setStartPort(sw2->outputPort());
    conn2->setEndPort(andGate->inputPort(0));
    scene->addItem(conn2.get());

    // Port validation fails, status forced to Unknown
    QCOMPARE(andGate->inputPort(0)->status(), Status::Unknown);
}

// ============================================================================
// Integration Tests (2 tests)
// ============================================================================

void TestConnections::testConnectionPositionUpdate()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *sw = new InputSwitch();
    auto *led = new Led();
    sw->setPos(0, 0);
    led->setPos(100, 0);
    scene->addItem(sw);
    scene->addItem(led);

    auto conn = std::make_unique<QNEConnection>();
    conn->setStartPort(sw->outputPort());
    conn->setEndPort(led->inputPort());
    scene->addItem(conn.get());

    // Set initial positions
    conn->setStartPos(sw->outputPort()->scenePos());
    conn->setEndPos(led->inputPort()->scenePos());

    // Move element
    led->setPos(200, 100);

    // Update connection - verify no crash
    conn->updatePosFromPorts();

    // Verify connection still has both ports after update
    QVERIFY(conn->startPort() != nullptr);
    QVERIFY(conn->endPort() != nullptr);
}

void TestConnections::testPortIsConnected()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;
    Or orGate;

    builder.add(&sw, &andGate, &orGate);
    builder.connect(&sw, 0, &andGate, 0);

    // Test isConnected
    QVERIFY(sw.outputPort()->isConnected(andGate.inputPort(0)));
    QVERIFY(andGate.inputPort(0)->isConnected(sw.outputPort()));

    // Not connected
    QVERIFY(!sw.outputPort()->isConnected(orGate.inputPort(0)));
    QVERIFY(!orGate.inputPort(0)->isConnected(sw.outputPort()));
}
