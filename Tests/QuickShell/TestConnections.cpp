// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestConnections.h"

#include <memory>

#include <QTest>

#include "App/Core/Application.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"

void TestConnections::initTestCase()
{
    // Setup before all tests
}

// ============================================================================
// Basic Lifecycle Tests (4 tests)
// ============================================================================

void TestConnections::testCreateConnection()
{
    auto conn = std::make_unique<Connection>();

    QVERIFY(conn->startPort() == nullptr);
    QVERIFY(conn->endPort() == nullptr);
    QCOMPARE(conn->status(), Status::Unknown);
}

void TestConnections::testSetPorts()
{
    QuickCircuitBuilder builder;

    auto *andGate = new And();
    auto *orGate = new Or();
    builder.addOwnedElement(andGate);
    builder.addOwnedElement(orGate);

    auto conn = std::make_unique<Connection>();
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
    QuickCircuitBuilder builder;

    auto *andGate = new And();
    builder.addOwnedElement(andGate);

    auto conn = std::make_unique<Connection>();
    conn->setStartPort(andGate->outputPort());

    QCOMPARE(andGate->outputPort()->connections().size(), 1);

    conn.reset();

    // Port should have removed connection
    QCOMPARE(andGate->outputPort()->connections().size(), 0);
}

void TestConnections::testOtherPort()
{
    QuickCircuitBuilder builder;

    auto *sw = new InputSwitch();
    auto *led = new Led();
    builder.addOwnedElement(sw);
    builder.addOwnedElement(led);

    auto conn = std::make_unique<Connection>();
    conn->setStartPort(sw->outputPort());
    conn->setEndPort(led->inputPort());

    // Test getting the opposite port
    QVERIFY(conn->otherPort(sw->outputPort()) == led->inputPort());
    QVERIFY(conn->otherPort(led->inputPort()) == sw->outputPort());
}

// ============================================================================
// Connection Validation Tests
// ============================================================================

void TestConnections::testValidOutputToInput()
{
    QuickCircuitBuilder builder;

    InputSwitch sw;
    And andGate;

    builder.add(&sw, &andGate);
    auto *conn = builder.connect(&sw, 0, &andGate, 0);  // Valid: output to input

    QVERIFY(conn != nullptr);
    QVERIFY(conn->startPort() != nullptr);
    QVERIFY(conn->endPort() != nullptr);
    QVERIFY(conn->startPort()->isOutput());
    QVERIFY(conn->endPort()->isInput());
}

// ============================================================================
// Port Capacity Tests (4 tests)
// ============================================================================

void TestConnections::testSingleInputConnection()
{
    QuickCircuitBuilder builder;

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
    QuickCircuitBuilder builder;

    auto *sw1 = new InputSwitch();
    auto *sw2 = new InputSwitch();
    auto *andGate = new And();
    builder.addOwnedElement(sw1);
    builder.addOwnedElement(sw2);
    builder.addOwnedElement(andGate);

    // First connection - valid
    auto conn1 = std::make_unique<Connection>();
    conn1->setStartPort(sw1->outputPort());
    conn1->setEndPort(andGate->inputPort(0));

    QCOMPARE(andGate->inputPort(0)->connections().size(), 1);
    QVERIFY(andGate->inputPort(0)->isValid());

    // Second connection to SAME input - should make port INVALID
    auto conn2 = std::make_unique<Connection>();
    conn2->setStartPort(sw2->outputPort());
    conn2->setEndPort(andGate->inputPort(0));

    QCOMPARE(andGate->inputPort(0)->connections().size(), 2);
    QVERIFY(!andGate->inputPort(0)->isValid());  // INVALID due to multiple
}

void TestConnections::testOutputFanOut()
{
    QuickCircuitBuilder builder;

    auto *sw = new InputSwitch();
    auto *and1 = new And();
    auto *and2 = new And();
    auto *and3 = new And();
    builder.addOwnedElement(sw);
    builder.addOwnedElement(and1);
    builder.addOwnedElement(and2);
    builder.addOwnedElement(and3);

    // Connect single output to multiple inputs (fan-out)
    auto *output = sw->outputPort();

    auto conn1 = std::make_unique<Connection>();
    conn1->setStartPort(output);
    conn1->setEndPort(and1->inputPort(0));

    auto conn2 = std::make_unique<Connection>();
    conn2->setStartPort(output);
    conn2->setEndPort(and2->inputPort(0));

    auto conn3 = std::make_unique<Connection>();
    conn3->setStartPort(output);
    conn3->setEndPort(and3->inputPort(0));

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
    QuickCircuitBuilder builder;

    auto *andGate = new And();
    builder.addOwnedElement(andGate);

    auto *input = andGate->inputPort(0);

    // Set required status - should be unknown without connection
    input->setRequired(true);
    QVERIFY(!input->isValid());  // No connection + required = unknown

    // Connect
    auto *sw = new InputSwitch();
    builder.addOwnedElement(sw);
    auto conn = std::make_unique<Connection>();
    conn->setStartPort(sw->outputPort());
    conn->setEndPort(input);

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
    QuickCircuitBuilder builder;

    auto *sw = new InputSwitch();
    auto *led = new Led();
    builder.addOwnedElement(sw);
    builder.addOwnedElement(led);

    auto conn = std::make_unique<Connection>();
    conn->setStartPort(sw->outputPort());
    conn->setEndPort(led->inputPort());

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
    // testOutputFanOut already covers Active/Inactive propagation through a fan-out; this
    // covers the two remaining Status values (Error and Unknown) that a fan-out source can
    // also carry, which nothing else exercises through a multi-connection output.
    QuickCircuitBuilder builder;

    auto *sw = new InputSwitch();
    auto *led1 = new Led();
    auto *led2 = new Led();
    auto *led3 = new Led();
    builder.addOwnedElement(sw);
    builder.addOwnedElement(led1);
    builder.addOwnedElement(led2);
    builder.addOwnedElement(led3);

    // Create fan-out
    auto conn1 = std::make_unique<Connection>();
    conn1->setStartPort(sw->outputPort());
    conn1->setEndPort(led1->inputPort());

    auto conn2 = std::make_unique<Connection>();
    conn2->setStartPort(sw->outputPort());
    conn2->setEndPort(led2->inputPort());

    auto conn3 = std::make_unique<Connection>();
    conn3->setStartPort(sw->outputPort());
    conn3->setEndPort(led3->inputPort());

    sw->outputPort()->setStatus(Status::Error);

    QCOMPARE(conn1->status(), Status::Error);
    QCOMPARE(conn2->status(), Status::Error);
    QCOMPARE(conn3->status(), Status::Error);
    QCOMPARE(led1->inputPort()->status(), Status::Error);
    QCOMPARE(led2->inputPort()->status(), Status::Error);
    QCOMPARE(led3->inputPort()->status(), Status::Error);

    sw->outputPort()->setStatus(Status::Unknown);

    QCOMPARE(conn1->status(), Status::Unknown);
    QCOMPARE(conn2->status(), Status::Unknown);
    QCOMPARE(conn3->status(), Status::Unknown);
    QCOMPARE(led1->inputPort()->status(), Status::Unknown);
    QCOMPARE(led2->inputPort()->status(), Status::Unknown);
    QCOMPARE(led3->inputPort()->status(), Status::Unknown);
}

void TestConnections::testInvalidPortStatus()
{
    // Test that invalid port (multiple connections) forces Unknown status
    QuickCircuitBuilder builder;

    auto *sw1 = new InputSwitch();
    auto *sw2 = new InputSwitch();
    auto *andGate = new And();
    builder.addOwnedElement(sw1);
    builder.addOwnedElement(sw2);
    builder.addOwnedElement(andGate);

    // Both outputs Active
    sw1->outputPort()->setStatus(Status::Active);
    sw2->outputPort()->setStatus(Status::Active);

    // First connection
    auto conn1 = std::make_unique<Connection>();
    conn1->setStartPort(sw1->outputPort());
    conn1->setEndPort(andGate->inputPort(0));

    QCOMPARE(andGate->inputPort(0)->status(), Status::Active);

    // Second connection - invalid (multiple connections to input)
    auto conn2 = std::make_unique<Connection>();
    conn2->setStartPort(sw2->outputPort());
    conn2->setEndPort(andGate->inputPort(0));

    // Port validation fails, status forced to Error
    QCOMPARE(andGate->inputPort(0)->status(), Status::Error);
}

// ============================================================================
// Integration Tests (2 tests)
// ============================================================================

void TestConnections::testConnectionPositionUpdate()
{
    // updatePath() (which rebuilds the geometry boundingRect() reads) only runs when
    // Application::renderingEnabled -- off by default in the test binary (real Bézier path
    // construction is pure waste for tests that never paint) -- so it must be enabled here to
    // actually exercise the geometry update rather than trivially no-op.
    const bool prevRendering = Application::renderingEnabled;
    Application::renderingEnabled = true;

    QuickCircuitBuilder builder;

    auto *sw = new InputSwitch();
    auto *led = new Led();
    sw->setPos(0, 0);
    led->setPos(100, 0);
    builder.addOwnedElement(sw);
    builder.addOwnedElement(led);

    auto conn = std::make_unique<Connection>();
    conn->setStartPort(sw->outputPort());
    conn->setEndPort(led->inputPort());

    // Set initial positions
    conn->setStartPos(sw->outputPort()->scenePos());
    conn->setEndPos(led->inputPort()->scenePos());
    conn->updatePath();

    const QRectF boundsBeforeMove = conn->boundingRect();

    // Move element
    led->setPos(200, 100);

    conn->updatePosFromPorts();

    // Verify connection still has both ports after update
    QVERIFY(conn->startPort() != nullptr);
    QVERIFY(conn->endPort() != nullptr);

    // The connection's own geometry must actually track the port's new scene position, not
    // just survive the call -- boundingRect() is built from the cached start/end positions
    // updatePosFromPorts() writes.
    const QRectF boundsAfterMove = conn->boundingRect();
    QVERIFY2(boundsAfterMove != boundsBeforeMove, "updatePosFromPorts() must change the connection's geometry when a port moves");
    QVERIFY2(boundsAfterMove.contains(led->inputPort()->scenePos()), "the updated bounding rect must reach the moved input port");

    Application::renderingEnabled = prevRendering;
}

void TestConnections::testPortIsConnected()
{
    QuickCircuitBuilder builder;

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
