// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestSceneConnections.h"

#include <algorithm>
#include <memory>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

// ============================================================
// Connection State and Query Tests
// ============================================================

void TestSceneConnections::testConnectionCountInitiallyZero()
{
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    auto orGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Or));
    orGate->setPos(150, 0);

    const auto &andOutputs = andGate->outputs();
    const auto &orInputs = orGate->inputs();

    QCOMPARE(andOutputs.size(), 1);
    QCOMPARE(orInputs.size(), 2);

    // Output should have no connections initially
    QCOMPARE(andOutputs[0]->connections().size(), 0);
}

void TestSceneConnections::testConnectionQueryAfterWiring()
{
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    auto orGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Or));
    orGate->setPos(150, 0);

    // Create a connection manually
    auto *connection = new Connection();

    // Connect output to input
    auto *output = andGate->outputPort(0);
    auto *input = orGate->inputPort(0);

    // Set connection endpoints
    connection->setStartPort(output);
    connection->setEndPort(input);

    // Verify connection exists
    QCOMPARE(output->connections().size(), 1);
    QCOMPARE(input->connections().size(), 1);
}

void TestSceneConnections::testConnectionListRetrieval()
{
    auto and1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    auto or1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Or));
    or1->setPos(150, 0);
    auto and2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    and2->setPos(300, 0);

    // Create multiple connections
    auto *conn1 = new Connection();
    conn1->setStartPort(and1->outputPort(0));
    conn1->setEndPort(or1->inputPort(0));

    auto *conn2 = new Connection();
    conn2->setStartPort(or1->outputPort(0));
    conn2->setEndPort(and2->inputPort(0));

    // Verify connection count
    QCOMPARE(and1->outputPort(0)->connections().size(), 1);
    QCOMPARE(or1->outputPort(0)->connections().size(), 1);
}

// ============================================================
// Connection Creation and Lifecycle Tests
// ============================================================

void TestSceneConnections::testConnectionCreationBetweenPorts()
{
    auto gate1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    auto gate2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Or));
    gate2->setPos(150, 0);

    // Get ports
    auto *output = gate1->outputPort(0);
    auto *input = gate2->inputPort(0);

    QVERIFY(output != nullptr);
    QVERIFY(input != nullptr);

    // Create connection
    auto *connection = new Connection();
    connection->setStartPort(output);
    connection->setEndPort(input);

    // Verify connection was created
    QVERIFY(output->isConnected(input));
    QCOMPARE(input->connections().size(), 1);
}

void TestSceneConnections::testConnectionWithMultiplePorts()
{
    auto andGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    andGate->setInputSize(4); // 4 inputs
    auto orGate = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Or));
    orGate->setInputSize(3); // 3 inputs
    orGate->setPos(150, 0);

    // Connect multiple ports
    for (int i = 0; i < (std::min)(andGate->inputs().size(), orGate->inputs().size()); ++i) {
        auto *conn = new Connection();
        conn->setStartPort(andGate->outputPort(0));
        conn->setEndPort(orGate->inputPort(i));
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
    auto gate1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    auto gate2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Or));
    gate2->setPos(150, 0);

    // Get port references
    auto *output = gate1->outputPort(0);
    auto *input = gate2->inputPort(0);

    // Create connection
    auto *connection = new Connection();
    connection->setStartPort(output);
    connection->setEndPort(input);

    // Verify references are preserved
    QCOMPARE(connection->startPort(), output);
    QCOMPARE(connection->endPort(), input);
}

void TestSceneConnections::testConnectionListUpdatesAfterAddition()
{
    auto gate1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    auto gate2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Or));
    gate2->setPos(150, 0);

    auto *input = gate2->inputPort(0);

    // Initially no connections
    int initialCount = static_cast<int>(input->connections().size());

    // Add connection
    auto *conn = new Connection();
    conn->setStartPort(gate1->outputPort(0));
    conn->setEndPort(input);

    // Connection list should be updated
    QCOMPARE(input->connections().size(), initialCount + 1);
}

// ============================================================
// Connection Validation Tests
// ============================================================

void TestSceneConnections::testValidConnectionBetweenInputOutput()
{
    auto output = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    auto input = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Led));
    input->setPos(150, 0);

    // Get ports
    auto *outputPort = output->outputPort(0);
    auto *inputPort = input->inputPort(0);

    QVERIFY(outputPort != nullptr);
    QVERIFY(inputPort != nullptr);

    // Create valid connection
    auto *connection = new Connection();
    connection->setStartPort(outputPort);
    connection->setEndPort(inputPort);

    // Verify connection is valid
    QVERIFY(connection->startPort() != nullptr);
    QVERIFY(connection->endPort() != nullptr);
}

void TestSceneConnections::testConnectionStatusTracking()
{
    // Create a driven source and a consumer gate
    auto vcc = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputVcc));
    auto gate2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Or));
    gate2->setPos(150, 0);

    // Create connection
    auto *connection = new Connection();
    connection->setStartPort(vcc->outputPort(0));
    connection->setEndPort(gate2->inputPort(0));

    // The wire tracks its driver's status from the moment it is attached
    Status status = connection->status();
    QCOMPARE(status, Status::Active);
}

void TestSceneConnections::testConnectionRemovalUpdatesCounters()
{
    auto gate1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    auto gate2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Or));
    gate2->setPos(150, 0);

    auto *input = gate2->inputPort(0);

    // Create and add connection
    auto connection = std::make_unique<Connection>();
    connection->setStartPort(gate1->outputPort(0));
    connection->setEndPort(input);

    int countWithConnection = static_cast<int>(input->connections().size());

    // Remove connection (destroying it detaches from both ports)
    connection.reset();

    int countAfterRemoval = static_cast<int>(input->connections().size());

    // Count should decrease
    QCOMPARE(countAfterRemoval, countWithConnection - 1);
}

// ============================================================
// Wireless Port Semantics Tests
// ============================================================

void TestSceneConnections::testWirelessRxPortIsNotRequired()
{
    // An Rx node's input port must have isRequired=false, so connecting
    // a wire to it is never mandatory for the circuit to be valid.
    auto nodeElm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    QVERIFY(nodeElm != nullptr);

    auto *node = qobject_cast<Node *>(nodeElm.get());
    QVERIFY(node != nullptr);

    // Verify default state
    QVERIFY(node->inputPort()->isRequired());

    // Switch to Rx — port becomes optional
    node->setWirelessMode(WirelessMode::Rx);
    QVERIFY(!node->inputPort()->isRequired());

    // A connection can still be made to the port (it's optional, not forbidden)
    auto src = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputButton));
    QVERIFY(src != nullptr);

    auto *conn = new Connection();
    conn->setStartPort(src->outputPort(0));
    conn->setEndPort(node->inputPort(0));

    QCOMPARE(node->inputPort(0)->connections().size(), 1);
    QVERIFY(!node->inputPort()->isRequired()); // still optional after wiring
}

void TestSceneConnections::testWirelessNoneModeNodePortIsRequired()
{
    // A Node in None mode must behave like a plain pass-through: its input port
    // is required and connections are tracked normally.
    auto nodeElm = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Node));
    QVERIFY(nodeElm != nullptr);

    auto *node = qobject_cast<Node *>(nodeElm.get());
    QVERIFY(node != nullptr);
    QCOMPARE(node->wirelessMode(), WirelessMode::None);
    QVERIFY(node->inputPort()->isRequired());

    auto src = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::InputSwitch));
    QVERIFY(src != nullptr);

    auto *conn = new Connection();
    conn->setStartPort(src->outputPort(0));
    conn->setEndPort(node->inputPort(0));

    QCOMPARE(node->inputPort(0)->connections().size(), 1);
    QVERIFY(node->inputPort()->isRequired()); // required status unchanged by a connection
}
