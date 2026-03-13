// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestSceneConnections.h"

#include <QTemporaryDir>

#include "App/Element/ElementFactory.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Scene.h"
#include "Tests/Common/TestUtils.h"

void TestSceneConnections::initTestCase()
{
    // Initialize test environment
    QVERIFY(m_tempDir.isValid());
}

void TestSceneConnections::init()
{
    // Set up test for each case
    QVERIFY(m_tempDir.isValid());
}

// ============================================================
// Connection State and Query Tests
// ============================================================

void TestSceneConnections::testConnectionCountInitiallyZero()
{
    // Create a scene
    Scene scene;

    // Create two logic gates
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene.addItem(andGate);

    auto *orGate = ElementFactory::buildElement(ElementType::Or);
    orGate->setPos(150, 0);
    scene.addItem(orGate);

    // Initially, no connections
    // We verify by checking elements are present but not connected
    QCOMPARE(scene.elements().size(), 2);

    // Get ports
    const auto &andOutputs = andGate->outputs();
    const auto &orInputs = orGate->inputs();

    QCOMPARE(andOutputs.size(), 1);
    QCOMPARE(orInputs.size(), 2);

    // Output should have no connections initially
    QCOMPARE(andOutputs[0]->connections().size(), 0);
}

void TestSceneConnections::testConnectionQueryAfterWiring()
{
    // Create a scene
    Scene scene;

    // Create logic gates
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene.addItem(andGate);

    auto *orGate = ElementFactory::buildElement(ElementType::Or);
    orGate->setPos(150, 0);
    scene.addItem(orGate);

    // Create a connection manually
    auto *connection = new QNEConnection(nullptr);
    QVERIFY(connection != nullptr);

    // Connect output to input
    auto *output = andGate->outputPort(0);
    auto *input = orGate->inputPort(0);

    // Set connection endpoints
    connection->setStartPort(output);
    connection->setEndPort(input);

    // Add connection to scene
    scene.addItem(connection);

    // Verify connection exists
    QCOMPARE(output->connections().size(), 1);
    QCOMPARE(input->connections().size(), 1);
}

void TestSceneConnections::testConnectionListRetrieval()
{
    // Create a scene
    Scene scene;

    // Create multiple logic gates
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    scene.addItem(and1);

    auto *or1 = ElementFactory::buildElement(ElementType::Or);
    or1->setPos(150, 0);
    scene.addItem(or1);

    auto *and2 = ElementFactory::buildElement(ElementType::And);
    and2->setPos(300, 0);
    scene.addItem(and2);

    // Create multiple connections
    auto *conn1 = new QNEConnection(nullptr);
    conn1->setStartPort(and1->outputPort(0));
    conn1->setEndPort(or1->inputPort(0));
    scene.addItem(conn1);

    auto *conn2 = new QNEConnection(nullptr);
    conn2->setStartPort(or1->outputPort(0));
    conn2->setEndPort(and2->inputPort(0));
    scene.addItem(conn2);

    // Verify connection count
    QCOMPARE(and1->outputPort(0)->connections().size(), 1);
    QCOMPARE(or1->outputPort(0)->connections().size(), 1);

    // Verify graph connectivity
    QCOMPARE(scene.elements().size(), 3);
}

// ============================================================
// Connection Creation and Lifecycle Tests
// ============================================================

void TestSceneConnections::testConnectionCreationBetweenPorts()
{
    // Create a scene
    Scene scene;

    // Create two gates
    auto *gate1 = ElementFactory::buildElement(ElementType::And);
    scene.addItem(gate1);

    auto *gate2 = ElementFactory::buildElement(ElementType::Or);
    gate2->setPos(150, 0);
    scene.addItem(gate2);

    // Get ports
    auto *output = gate1->outputPort(0);
    auto *input = gate2->inputPort(0);

    QVERIFY(output != nullptr);
    QVERIFY(input != nullptr);

    // Create connection
    auto *connection = new QNEConnection(nullptr);
    connection->setStartPort(output);
    connection->setEndPort(input);
    scene.addItem(connection);

    // Verify connection was created
    QVERIFY(output->isConnected(input));
    QCOMPARE(input->connections().size(), 1);
}

void TestSceneConnections::testConnectionWithMultiplePorts()
{
    // Create a scene
    Scene scene;

    // Create gates with multiple ports
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    andGate->setInputSize(4);  // 4 inputs
    scene.addItem(andGate);

    auto *orGate = ElementFactory::buildElement(ElementType::Or);
    orGate->setInputSize(3);  // 3 inputs
    orGate->setPos(150, 0);
    scene.addItem(orGate);

    // Connect multiple ports
    for (int i = 0; i < qMin(andGate->inputs().size(), orGate->inputs().size()); ++i) {
        auto *conn = new QNEConnection(nullptr);
        conn->setStartPort(andGate->outputPort(0));
        conn->setEndPort(orGate->inputPort(i));
        scene.addItem(conn);
    }

    // Verify connections
    auto output = andGate->outputPort(0);
    QCOMPARE(output->connections().size(), 3);

    // Each input should track its connections
    for (int i = 0; i < orGate->inputs().size(); ++i) {
        auto *input = orGate->inputPort(i);
        QCOMPARE(input->connections().size(), 1);
    }
}

void TestSceneConnections::testConnectionPreservesPortReferences()
{
    // Create a scene
    Scene scene;

    // Create gates
    auto *gate1 = ElementFactory::buildElement(ElementType::And);
    scene.addItem(gate1);

    auto *gate2 = ElementFactory::buildElement(ElementType::Or);
    gate2->setPos(150, 0);
    scene.addItem(gate2);

    // Get port references
    auto *output = gate1->outputPort(0);
    auto *input = gate2->inputPort(0);

    // Create connection
    auto *connection = new QNEConnection(nullptr);
    connection->setStartPort(output);
    connection->setEndPort(input);

    // Verify references are preserved
    QCOMPARE(connection->startPort(), output);
    QCOMPARE(connection->endPort(), input);
}

void TestSceneConnections::testConnectionListUpdatesAfterAddition()
{
    // Create a scene
    Scene scene;

    // Create gates
    auto *gate1 = ElementFactory::buildElement(ElementType::And);
    scene.addItem(gate1);

    auto *gate2 = ElementFactory::buildElement(ElementType::Or);
    gate2->setPos(150, 0);
    scene.addItem(gate2);

    auto *input = gate2->inputPort(0);

    // Initially no connections
    int initialCount = input->connections().size();

    // Add connection
    auto *conn = new QNEConnection(nullptr);
    conn->setStartPort(gate1->outputPort(0));
    conn->setEndPort(input);
    scene.addItem(conn);

    // Connection list should be updated
    QCOMPARE(input->connections().size(), initialCount + 1);
}

// ============================================================
// Connection Validation Tests
// ============================================================

void TestSceneConnections::testValidConnectionBetweenInputOutput()
{
    // Create a scene
    Scene scene;

    // Create gates
    auto *output = ElementFactory::buildElement(ElementType::And);
    scene.addItem(output);

    auto *input = ElementFactory::buildElement(ElementType::Led);
    input->setPos(150, 0);
    scene.addItem(input);

    // Get ports
    auto *outputPort = output->outputPort(0);
    auto *inputPort = input->inputPort(0);

    QVERIFY(outputPort != nullptr);
    QVERIFY(inputPort != nullptr);

    // Create valid connection
    auto *connection = new QNEConnection(nullptr);
    connection->setStartPort(outputPort);
    connection->setEndPort(inputPort);
    scene.addItem(connection);

    // Verify connection is valid
    QVERIFY(connection->startPort() != nullptr);
    QVERIFY(connection->endPort() != nullptr);
}

void TestSceneConnections::testConnectionStatusTracking()
{
    // Create a scene
    Scene scene;

    // Create gates
    auto *gate1 = ElementFactory::buildElement(ElementType::And);
    scene.addItem(gate1);

    auto *gate2 = ElementFactory::buildElement(ElementType::Or);
    gate2->setPos(150, 0);
    scene.addItem(gate2);

    // Create connection
    auto *connection = new QNEConnection(nullptr);
    connection->setStartPort(gate1->outputPort(0));
    connection->setEndPort(gate2->inputPort(0));
    scene.addItem(connection);

    // Initial status should be inactive or active
    Status status = connection->status();
    QCOMPARE(status, Status::Inactive);
}

void TestSceneConnections::testConnectionRemovalUpdatesCounters()
{
    // Create a scene
    Scene scene;

    // Create gates
    auto *gate1 = ElementFactory::buildElement(ElementType::And);
    scene.addItem(gate1);

    auto *gate2 = ElementFactory::buildElement(ElementType::Or);
    gate2->setPos(150, 0);
    scene.addItem(gate2);

    auto *input = gate2->inputPort(0);

    // Create and add connection
    auto *connection = new QNEConnection(nullptr);
    connection->setStartPort(gate1->outputPort(0));
    connection->setEndPort(input);
    scene.addItem(connection);

    int countWithConnection = input->connections().size();

    // Remove connection
    scene.removeItem(connection);
    delete connection;

    int countAfterRemoval = input->connections().size();

    // Count should decrease
    QCOMPARE(countAfterRemoval, countWithConnection - 1);
}
