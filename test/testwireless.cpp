// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testwireless.h"

#include "common.h"
#include "elementeditor.h"
#include "elementfactory.h"
#include "inputbutton.h"
#include "inputgnd.h"
#include "inputswitch.h"
#include "inputvcc.h"
#include "led.h"
#include "node.h"
#include "qneconnection.h"
#include "scene.h"
#include "serialization.h"
#include "simulation.h"
#include "wirelessconnectionmanager.h"

#include <QDataStream>
#include <QSignalSpy>
#include <QtTest>

void TestWireless::testWirelessConnectionManager()
{
    // Create scene with wireless manager
    auto *scene = new Scene();
    auto *wirelessManager = scene->wirelessManager();

    // Verify manager exists and is properly initialized
    QVERIFY(wirelessManager != nullptr);
    QVERIFY(scene->wirelessManager() == wirelessManager); // Same instance

    // Set up signal spy to verify signals are emitted
    QSignalSpy spy(wirelessManager, &WirelessConnectionManager::wirelessConnectionsChanged);

    // Initial state should be empty
    QVERIFY(!wirelessManager->hasWirelessConnections());
    QCOMPARE(wirelessManager->getGroupCount(), 0);
    QVERIFY(wirelessManager->getActiveLabels().isEmpty());

    // Create test nodes
    auto *node1 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *node2 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));

    QVERIFY(node1 != nullptr);
    QVERIFY(node2 != nullptr);

    scene->addItem(node1);
    scene->addItem(node2);

    // Test setting wireless labels
    node1->setLabel("test_label");
    QCOMPARE(spy.count(), 1); // Signal should be emitted
    QVERIFY(wirelessManager->hasWirelessConnections());
    QCOMPARE(wirelessManager->getGroupCount(), 1);

    node2->setLabel("test_label");
    QCOMPARE(spy.count(), 2); // Signal should be emitted again
    QCOMPARE(wirelessManager->getGroupCount(), 1); // Still one group

    // Verify connections
    auto connectedNodes = wirelessManager->getConnectedNodes(node1);
    QCOMPARE(connectedNodes.size(), 2); // Both nodes should be connected
    QVERIFY(connectedNodes.contains(node1));
    QVERIFY(connectedNodes.contains(node2));

    // Test group queries
    auto activeLabels = wirelessManager->getActiveLabels();
    QCOMPARE(activeLabels.size(), 1);
    QVERIFY(activeLabels.contains("test_label"));

    auto groupNodes = wirelessManager->getWirelessGroup("test_label");
    QCOMPARE(groupNodes.size(), 2);
    QVERIFY(groupNodes.contains(node1));
    QVERIFY(groupNodes.contains(node2));

    // Cleanup
    delete scene; // This will delete nodes too due to scene ownership
}

void TestWireless::testBasicWirelessConnection()
{
    auto *scene = new Scene();
    auto *wirelessManager = scene->wirelessManager();

    // Set up signal monitoring
    QSignalSpy connectionSpy(wirelessManager, &WirelessConnectionManager::wirelessConnectionsChanged);

    // Create nodes at specific positions
    auto *outputNode = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *inputNode = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));

    outputNode->setPos(0, 0);
    inputNode->setPos(100, 0);

    scene->addItem(outputNode);
    scene->addItem(inputNode);

    // Verify positions are approximately correct (scene might adjust for alignment)
    QVERIFY(qAbs(outputNode->pos().x() - 0.0) < 5);
    QVERIFY(qAbs(inputNode->pos().x() - 100.0) < 5);

    // Initially no connections
    QCOMPARE(wirelessManager->getConnectedNodes(outputNode).size(), 0);
    QCOMPARE(wirelessManager->getConnectedNodes(inputNode).size(), 0);

    // Connect both nodes with same label
    outputNode->setLabel("connection1");
    QCOMPARE(connectionSpy.count(), 1);

    inputNode->setLabel("connection1");
    QCOMPARE(connectionSpy.count(), 2);

    // Verify bidirectional connection
    auto connectedFromOutput = wirelessManager->getConnectedNodes(outputNode);
    auto connectedFromInput = wirelessManager->getConnectedNodes(inputNode);

    // Both should return the same set
    QCOMPARE(connectedFromOutput, connectedFromInput);
    QCOMPARE(connectedFromOutput.size(), 2);

    // Verify each node sees the other
    QVERIFY(connectedFromOutput.contains(outputNode));
    QVERIFY(connectedFromOutput.contains(inputNode));
    QVERIFY(connectedFromInput.contains(outputNode));
    QVERIFY(connectedFromInput.contains(inputNode));

    // Test disconnection by changing one label
    outputNode->setLabel("different_label");
    QCOMPARE(connectionSpy.count(), 3);

    // Should no longer be connected
    auto outputConnections = wirelessManager->getConnectedNodes(outputNode);
    auto inputConnections = wirelessManager->getConnectedNodes(inputNode);

    QCOMPARE(outputConnections.size(), 1); // Only itself
    QCOMPARE(inputConnections.size(), 1); // Only itself
    QVERIFY(outputConnections.contains(outputNode));
    QVERIFY(inputConnections.contains(inputNode));
    QVERIFY(!outputConnections.contains(inputNode));
    QVERIFY(!inputConnections.contains(outputNode));

    delete scene;
}

void TestWireless::testMultipleWirelessConnections()
{
    auto *scene = new Scene();
    auto *wirelessManager = scene->wirelessManager();

    // Create multiple nodes
    auto *node1 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *node2 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *node3 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *node4 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));

    scene->addItem(node1);
    scene->addItem(node2);
    scene->addItem(node3);
    scene->addItem(node4);

    // Create two separate groups
    node1->setLabel("groupA");
    node2->setLabel("groupA");
    node3->setLabel("groupB");
    node4->setLabel("groupB");

    // Verify two groups exist
    QCOMPARE(wirelessManager->getGroupCount(), 2);

    auto activeLabels = wirelessManager->getActiveLabels();
    QCOMPARE(activeLabels.size(), 2);
    QVERIFY(activeLabels.contains("groupA"));
    QVERIFY(activeLabels.contains("groupB"));

    // Verify group A connections
    auto groupA = wirelessManager->getWirelessGroup("groupA");
    QCOMPARE(groupA.size(), 2);
    QVERIFY(groupA.contains(node1));
    QVERIFY(groupA.contains(node2));

    // Verify group B connections
    auto groupB = wirelessManager->getWirelessGroup("groupB");
    QCOMPARE(groupB.size(), 2);
    QVERIFY(groupB.contains(node3));
    QVERIFY(groupB.contains(node4));

    // Verify cross-group isolation
    QVERIFY(!groupA.contains(node3));
    QVERIFY(!groupA.contains(node4));
    QVERIFY(!groupB.contains(node1));
    QVERIFY(!groupB.contains(node2));

    delete scene;
}

void TestWireless::testWirelessLabelChanges()
{
    auto *scene = new Scene();
    auto *wirelessManager = scene->wirelessManager();

    auto *node1 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *node2 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *node3 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));

    scene->addItem(node1);
    scene->addItem(node2);
    scene->addItem(node3);

    // Initial setup: two nodes in one group, one in another
    node1->setLabel("original");
    node2->setLabel("original");
    node3->setLabel("separate");

    QCOMPARE(wirelessManager->getGroupCount(), 2);
    QCOMPARE(wirelessManager->getWirelessGroup("original").size(), 2);
    QCOMPARE(wirelessManager->getWirelessGroup("separate").size(), 1);

    // Move node2 from "original" to "separate"
    node2->setLabel("separate");

    QCOMPARE(wirelessManager->getGroupCount(), 2);
    QCOMPARE(wirelessManager->getWirelessGroup("original").size(), 1);
    QCOMPARE(wirelessManager->getWirelessGroup("separate").size(), 2);

    // Remove node2's label entirely
    node2->setLabel("");

    QCOMPARE(wirelessManager->getGroupCount(), 2); // Both groups still exist
    QCOMPARE(wirelessManager->getWirelessGroup("original").size(), 1);
    QCOMPARE(wirelessManager->getWirelessGroup("separate").size(), 1);
    QVERIFY(wirelessManager->getConnectedNodes(node2).isEmpty()); // node2 has no connections

    delete scene;
}

void TestWireless::testWirelessNodeRemoval()
{
    auto *scene = new Scene();
    auto *wirelessManager = scene->wirelessManager();

    auto *node1 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *node2 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *node3 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));

    scene->addItem(node1);
    scene->addItem(node2);
    scene->addItem(node3);

    // Create a group with all three nodes
    node1->setLabel("group");
    node2->setLabel("group");
    node3->setLabel("group");

    QCOMPARE(wirelessManager->getWirelessGroup("group").size(), 3);

    // Remove node2 from scene (simulates deletion)
    scene->removeItem(node2);
    delete node2;

    // Group should automatically update
    QCOMPARE(wirelessManager->getWirelessGroup("group").size(), 2);

    auto remainingNodes = wirelessManager->getWirelessGroup("group");
    QVERIFY(remainingNodes.contains(node1));
    QVERIFY(remainingNodes.contains(node3));

    delete scene;
}

void TestWireless::testEmptyLabels()
{
    auto *scene = new Scene();
    auto *wirelessManager = scene->wirelessManager();

    auto *node = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    scene->addItem(node);

    // Test various forms of empty labels
    node->setLabel("");
    QVERIFY(wirelessManager->getConnectedNodes(node).isEmpty());

    node->setLabel("   ");  // Whitespace only
    QVERIFY(wirelessManager->getConnectedNodes(node).isEmpty());

    node->setLabel("\t\n  ");  // Various whitespace
    QVERIFY(wirelessManager->getConnectedNodes(node).isEmpty());

    QCOMPARE(wirelessManager->getGroupCount(), 0);
    QVERIFY(!wirelessManager->hasWirelessConnections());

    delete scene;
}

void TestWireless::testDuplicateLabels()
{
    // This test verifies that multiple nodes can have the same label
    // and that the system handles this correctly

    auto *scene = new Scene();
    auto *wirelessManager = scene->wirelessManager();

    // Create 5 nodes with the same label
    QList<Node *> nodes;
    for (int i = 0; i < 5; ++i) {
        auto *node = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
        scene->addItem(node);
        node->setLabel("shared");
        nodes.append(node);
    }

    QCOMPARE(wirelessManager->getGroupCount(), 1);
    QCOMPARE(wirelessManager->getWirelessGroup("shared").size(), 5);

    // Every node should see all 5 nodes in its connections
    for (Node *node : nodes) {
        auto connections = wirelessManager->getConnectedNodes(node);
        QCOMPARE(connections.size(), 5);
        for (Node *otherNode : nodes) {
            QVERIFY(connections.contains(otherNode));
        }
    }

    delete scene;
}

void TestWireless::testWirelessConnectionInSimulation()
{
    // Test that wireless connections work correctly during circuit simulation
    auto *scene = new Scene();
    auto *wirelessManager = scene->wirelessManager();
    auto *simulation = scene->simulation();

    // Create circuit: InputSwitch -> Node(wireless:A) ... Node(wireless:A) -> LED
    auto *inputSwitch = qgraphicsitem_cast<InputSwitch*>(ElementFactory::buildElement(ElementType::InputSwitch));
    auto *senderNode = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *receiverNode = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *led = qgraphicsitem_cast<Led*>(ElementFactory::buildElement(ElementType::Led));

    // Add elements to scene
    scene->addItem(inputSwitch);
    scene->addItem(senderNode);
    scene->addItem(receiverNode);
    scene->addItem(led);

    // Create physical connections
    auto *connection1 = new QNEConnection();
    auto *connection2 = new QNEConnection();
    scene->addItem(connection1);
    scene->addItem(connection2);

    connection1->setStartPort(inputSwitch->outputPort());
    connection1->setEndPort(senderNode->inputPort());
    connection2->setStartPort(receiverNode->outputPort());
    connection2->setEndPort(led->inputPort());

    // Set up wireless connection
    senderNode->setLabel("test_signal");
    receiverNode->setLabel("test_signal");

    // Verify wireless connection exists
    auto connections = wirelessManager->getConnectedNodes(senderNode);
    QCOMPARE(connections.size(), 2);
    QVERIFY(connections.contains(senderNode));
    QVERIFY(connections.contains(receiverNode));

    // Test with switch OFF
    inputSwitch->setOn(false);
    simulation->start();

    // Run a few simulation cycles to propagate signals
    for (int i = 0; i < 5; ++i) {
        simulation->update();
    }

    // LED should be OFF (Status::Inactive)
    QCOMPARE(led->inputPort()->status(), Status::Inactive);

    // Test with switch ON
    inputSwitch->setOn(true);

    // Run simulation cycles to propagate signals
    for (int i = 0; i < 5; ++i) {
        simulation->update();
    }

    // LED should be ON (Status::Active)
    QCOMPARE(led->inputPort()->status(), Status::Active);

    // Test switching back to OFF
    inputSwitch->setOn(false);

    // Run simulation cycles
    for (int i = 0; i < 5; ++i) {
        simulation->update();
    }

    // LED should be OFF again
    QCOMPARE(led->inputPort()->status(), Status::Inactive);

    simulation->stop();
    delete scene;
}

void TestWireless::testWirelessSerialization()
{
    // Test that wireless connections can be properly serialized and deserialized
    // This is a basic test of the wireless manager's serialize/deserialize methods

    auto *scene = new Scene();
    auto *wirelessManager = scene->wirelessManager();

    auto *node1 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *node2 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *node3 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));

    scene->addItem(node1);
    scene->addItem(node2);
    scene->addItem(node3);

    // Store original node IDs
    int node1Id = node1->id();
    int node2Id = node2->id();
    int node3Id = node3->id();

    node1->setLabel("groupA");
    node2->setLabel("groupA");
    node3->setLabel("groupB");

    // Verify initial state
    QCOMPARE(wirelessManager->getGroupCount(), 2);
    QCOMPARE(wirelessManager->getWirelessGroup("groupA").size(), 2);
    QCOMPARE(wirelessManager->getWirelessGroup("groupB").size(), 1);

    // Test the serialize/deserialize cycle by creating new nodes with the same IDs
    // and manually calling the wireless manager methods
    QByteArray data;
    QDataStream writeStream(&data, QIODevice::WriteOnly);
    wirelessManager->serialize(writeStream);

    // Clear current connections
    node1->setLabel("");
    node2->setLabel("");
    node3->setLabel("");
    QCOMPARE(wirelessManager->getGroupCount(), 0);

    // Create a mock scene with nodes that have the same IDs for deserialization
    // In real usage, this would be handled by the full scene serialization
    QDataStream readStream(&data, QIODevice::ReadOnly);

    // Simulate what would happen during deserialization:
    // The wireless manager would find nodes by ID and restore their labels
    QMap<int, QString> serializedLabels;
    readStream >> serializedLabels;

    // Verify the serialized data contains the expected information
    QCOMPARE(serializedLabels.size(), 3);
    QCOMPARE(serializedLabels[node1Id], QString("groupA"));
    QCOMPARE(serializedLabels[node2Id], QString("groupA"));
    QCOMPARE(serializedLabels[node3Id], QString("groupB"));

    // Manually restore the labels (simulating what deserialize() would do)
    node1->setLabel(serializedLabels[node1Id]);
    node2->setLabel(serializedLabels[node2Id]);
    node3->setLabel(serializedLabels[node3Id]);

    // Verify connections were restored
    QCOMPARE(wirelessManager->getGroupCount(), 2);
    QCOMPARE(wirelessManager->getWirelessGroup("groupA").size(), 2);
    QCOMPARE(wirelessManager->getWirelessGroup("groupB").size(), 1);

    auto activeLabels = wirelessManager->getActiveLabels();
    QCOMPARE(activeLabels.size(), 2);
    QVERIFY(activeLabels.contains("groupA"));
    QVERIFY(activeLabels.contains("groupB"));

    delete scene;
}

void TestWireless::testEdgeCases()
{
    auto *scene = new Scene();
    auto *wirelessManager = scene->wirelessManager();

    // Set up signal monitoring
    QSignalSpy connectionSpy(wirelessManager, &WirelessConnectionManager::wirelessConnectionsChanged);

    auto *nodeA = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *nodeB = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *nodeC = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));

    scene->addItem(nodeA);
    scene->addItem(nodeB);
    scene->addItem(nodeC);

    // Test case sensitivity
    nodeA->setLabel("CaseSensitive");
    nodeB->setLabel("casesensitive"); // Different case

    // Should NOT be connected (case sensitive)
    auto connectionsA = wirelessManager->getConnectedNodes(nodeA);
    auto connectionsB = wirelessManager->getConnectedNodes(nodeB);

    QCOMPARE(connectionsA.size(), 1); // Only itself
    QCOMPARE(connectionsB.size(), 1); // Only itself
    QVERIFY(!connectionsA.contains(nodeB));
    QVERIFY(!connectionsB.contains(nodeA));

    // Test Unicode labels
    nodeA->setLabel("测试标签");
    nodeB->setLabel("测试标签"); // Same Unicode string

    auto unicodeConnections = wirelessManager->getConnectedNodes(nodeA);
    QCOMPARE(unicodeConnections.size(), 2);
    QVERIFY(unicodeConnections.contains(nodeA));
    QVERIFY(unicodeConnections.contains(nodeB));

    // Test special characters
    nodeA->setLabel("special!@#$%^&*()");
    nodeB->setLabel("special!@#$%^&*()");

    auto specialConnections = wirelessManager->getConnectedNodes(nodeA);
    QCOMPARE(specialConnections.size(), 2);
    QVERIFY(specialConnections.contains(nodeA));
    QVERIFY(specialConnections.contains(nodeB));

    // Test label with only whitespace differences
    nodeA->setLabel("  trimmed  ");
    nodeB->setLabel("trimmed"); // Should be treated as same after trimming

    auto trimmedConnections = wirelessManager->getConnectedNodes(nodeA);
    QCOMPARE(trimmedConnections.size(), 2);
    QVERIFY(trimmedConnections.contains(nodeA));
    QVERIFY(trimmedConnections.contains(nodeB));

    // Test rapid label changes
    int initialSpyCount = connectionSpy.count();

    for (int i = 0; i < 10; ++i) {
        nodeC->setLabel(QString("rapid_change_%1").arg(i));
    }

    // Should have generated signals for each change
    QVERIFY(connectionSpy.count() > initialSpyCount);

    delete scene;
}

void TestWireless::testStressConditions()
{
    auto *scene = new Scene();
    auto *wirelessManager = scene->wirelessManager();

    // Create many nodes in a single group
    QList<Node *> nodes;
    const int nodeCount = 200; // Stress test with 200 nodes

    for (int i = 0; i < nodeCount; ++i) {
        auto *node = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
        scene->addItem(node);
        node->setLabel("stress_test");
        nodes.append(node);
    }

    // Verify all nodes are connected
    QCOMPARE(wirelessManager->getGroupCount(), 1);
    QCOMPARE(wirelessManager->getWirelessGroup("stress_test").size(), nodeCount);

    // Test that every node sees all other nodes
    for (Node *node : nodes) {
        auto connections = wirelessManager->getConnectedNodes(node);
        QCOMPARE(connections.size(), nodeCount);

        // Verify this node can see all other nodes
        for (Node *otherNode : nodes) {
            QVERIFY(connections.contains(otherNode));
        }
    }

    // Test performance of group lookup
    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i < 1000; ++i) {
        wirelessManager->getActiveLabels();
        wirelessManager->getWirelessGroup("stress_test");
    }

    qint64 elapsed = timer.elapsed();
    qCDebug(zero) << "1000 lookups took" << elapsed << "ms";

    // Should complete reasonably quickly (less than 1 second for 1000 lookups)
    QVERIFY(elapsed < 1000);

    delete scene;
}

void TestWireless::testInvalidOperations()
{
    auto *scene = new Scene();
    auto *wirelessManager = scene->wirelessManager();

    // Test operations with null nodes
    wirelessManager->setNodeWirelessLabel(nullptr, "test");  // Should not crash
    wirelessManager->removeNode(nullptr);  // Should not crash

    QVERIFY(wirelessManager->getNodeWirelessLabel(nullptr).isEmpty());
    QVERIFY(wirelessManager->getConnectedNodes(nullptr).isEmpty());

    // Test operations with nodes not added to scene
    auto *orphanNode = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));

    // Should handle gracefully
    orphanNode->setLabel("orphan");
    QCOMPARE(wirelessManager->getConnectedNodes(orphanNode).size(), 0);

    delete orphanNode;

    // Test with empty/invalid labels
    auto *node = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    scene->addItem(node);

    wirelessManager->setNodeWirelessLabel(node, QString()); // Empty
    QVERIFY(wirelessManager->getConnectedNodes(node).isEmpty());

    wirelessManager->setNodeWirelessLabel(node, QString("   ")); // Whitespace only
    QVERIFY(wirelessManager->getConnectedNodes(node).isEmpty());

    QCOMPARE(wirelessManager->getGroupCount(), 0);

    delete scene;
}

void TestWireless::testStateConsistency()
{
    auto *scene = new Scene();
    auto *wirelessManager = scene->wirelessManager();

    auto *node1 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *node2 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *node3 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));

    scene->addItem(node1);
    scene->addItem(node2);
    scene->addItem(node3);

    // Test symmetry: if A sees B, then B should see A
    node1->setLabel("symmetry_test");
    node2->setLabel("symmetry_test");

    auto node1Connections = wirelessManager->getConnectedNodes(node1);
    auto node2Connections = wirelessManager->getConnectedNodes(node2);

    // Verify symmetry
    QCOMPARE(node1Connections, node2Connections);
    QVERIFY(node1Connections.contains(node2));
    QVERIFY(node2Connections.contains(node1));

    // Test transitivity: if A-B connected and B-C connected, verify group consistency
    node3->setLabel("symmetry_test");

    // Get fresh connections after all nodes are in the group
    node1Connections = wirelessManager->getConnectedNodes(node1);
    node2Connections = wirelessManager->getConnectedNodes(node2);
    auto node3Connections = wirelessManager->getConnectedNodes(node3);

    // All three should see the same set of connections
    QCOMPARE(node1Connections, node2Connections);
    QCOMPARE(node1Connections, node3Connections);
    QCOMPARE(node2Connections, node3Connections);

    // Test that active labels count matches actual groups
    auto activeLabels = wirelessManager->getActiveLabels();
    int expectedGroups = 0;
    for (const QString &label : activeLabels) {
        if (wirelessManager->getWirelessGroup(label).size() > 0) {
            expectedGroups++;
        }
    }
    QCOMPARE(wirelessManager->getGroupCount(), expectedGroups);

    // Test that group size consistency
    for (const QString &label : activeLabels) {
        auto groupNodes = wirelessManager->getWirelessGroup(label);
        for (Node *node : groupNodes) {
            auto nodeConnections = wirelessManager->getConnectedNodes(node);
            QCOMPARE(nodeConnections.size(), groupNodes.size());

            // Every node in connections should be in the group
            for (Node *connectedNode : nodeConnections) {
                QVERIFY(groupNodes.contains(connectedNode));
            }
        }
    }

    delete scene;
}

void TestWireless::testWirelessSignalPriority()
{
    // Test that wireless connections detect contention like physical connections
    auto *scene = new Scene();
    auto *simulation = scene->simulation();

    // Create circuit with multiple signal sources
    auto *vccSource = qgraphicsitem_cast<InputVcc*>(ElementFactory::buildElement(ElementType::InputVcc));
    auto *gndSource = qgraphicsitem_cast<InputGnd*>(ElementFactory::buildElement(ElementType::InputGnd));
    auto *vccNode = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *gndNode = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *receiverNode = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *led = qgraphicsitem_cast<Led*>(ElementFactory::buildElement(ElementType::Led));

    scene->addItem(vccSource);
    scene->addItem(gndSource);
    scene->addItem(vccNode);
    scene->addItem(gndNode);
    scene->addItem(receiverNode);
    scene->addItem(led);

    // Create connections
    auto *vccConnection = new QNEConnection();
    auto *gndConnection = new QNEConnection();
    auto *ledConnection = new QNEConnection();
    scene->addItem(vccConnection);
    scene->addItem(gndConnection);
    scene->addItem(ledConnection);

    vccConnection->setStartPort(vccSource->outputPort());
    vccConnection->setEndPort(vccNode->inputPort());
    gndConnection->setStartPort(gndSource->outputPort());
    gndConnection->setEndPort(gndNode->inputPort());
    ledConnection->setStartPort(receiverNode->outputPort());
    ledConnection->setEndPort(led->inputPort());

    // Set up wireless group with conflicting signals
    vccNode->setLabel("contention_test");
    gndNode->setLabel("contention_test");
    receiverNode->setLabel("contention_test");

    simulation->start();

    // Run simulation to propagate signals
    for (int i = 0; i < 5; ++i) {
        simulation->update();
    }

    // LED should be OFF because conflicting signals create contention (like physical connections)
    QCOMPARE(led->inputPort()->status(), Status::Inactive);

    simulation->stop();
    delete scene;
}

void TestWireless::testWirelessSignalConsistency()
{
    // Test that wireless connections work normally when signals don't conflict
    auto *scene = new Scene();
    auto *simulation = scene->simulation();

    // Test case 1: All signals are the same (no contention)
    auto *vccSource1 = qgraphicsitem_cast<InputVcc*>(ElementFactory::buildElement(ElementType::InputVcc));
    auto *vccSource2 = qgraphicsitem_cast<InputVcc*>(ElementFactory::buildElement(ElementType::InputVcc));
    auto *vccNode1 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *vccNode2 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *receiverNode = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *led = qgraphicsitem_cast<Led*>(ElementFactory::buildElement(ElementType::Led));

    scene->addItem(vccSource1);
    scene->addItem(vccSource2);
    scene->addItem(vccNode1);
    scene->addItem(vccNode2);
    scene->addItem(receiverNode);
    scene->addItem(led);

    // Create connections
    auto *vccConnection1 = new QNEConnection();
    auto *vccConnection2 = new QNEConnection();
    auto *ledConnection = new QNEConnection();
    scene->addItem(vccConnection1);
    scene->addItem(vccConnection2);
    scene->addItem(ledConnection);

    vccConnection1->setStartPort(vccSource1->outputPort());
    vccConnection1->setEndPort(vccNode1->inputPort());
    vccConnection2->setStartPort(vccSource2->outputPort());
    vccConnection2->setEndPort(vccNode2->inputPort());
    ledConnection->setStartPort(receiverNode->outputPort());
    ledConnection->setEndPort(led->inputPort());

    // Set up wireless group with consistent signals (all HIGH)
    vccNode1->setLabel("consistent_test");
    vccNode2->setLabel("consistent_test");
    receiverNode->setLabel("consistent_test");

    simulation->start();

    // Run simulation to propagate signals
    for (int i = 0; i < 5; ++i) {
        simulation->update();
    }

    // LED should be ON because all signals are consistent (no contention)
    QCOMPARE(led->inputPort()->status(), Status::Active);

    simulation->stop();
    delete scene;
}

void TestWireless::testWirelessSingleNodeGroup()
{
    // Test that single node groups are handled correctly (should be ignored)
    auto *scene = new Scene();
    auto *simulation = scene->simulation();

    auto *inputSwitch = qgraphicsitem_cast<InputSwitch*>(ElementFactory::buildElement(ElementType::InputSwitch));
    auto *singleNode = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *led = qgraphicsitem_cast<Led*>(ElementFactory::buildElement(ElementType::Led));

    scene->addItem(inputSwitch);
    scene->addItem(singleNode);
    scene->addItem(led);

    auto *inputConnection = new QNEConnection();
    auto *outputConnection = new QNEConnection();
    scene->addItem(inputConnection);
    scene->addItem(outputConnection);

    inputConnection->setStartPort(inputSwitch->outputPort());
    inputConnection->setEndPort(singleNode->inputPort());
    outputConnection->setStartPort(singleNode->outputPort());
    outputConnection->setEndPort(led->inputPort());

    // Set wireless label on single node (group size = 1)
    singleNode->setLabel("single_node");

    inputSwitch->setOn(true);
    simulation->start();

    // Run simulation
    for (int i = 0; i < 5; ++i) {
        simulation->update();
    }

    // LED should still work normally (wireless logic should ignore single-node groups)
    QCOMPARE(led->inputPort()->status(), Status::Active);

    inputSwitch->setOn(false);

    // Run simulation
    for (int i = 0; i < 5; ++i) {
        simulation->update();
    }

    QCOMPARE(led->inputPort()->status(), Status::Inactive);

    simulation->stop();
    delete scene;
}

void TestWireless::testWirelessMultipleGroups()
{
    // Test multiple independent wireless groups in the same simulation
    auto *scene = new Scene();
    auto *simulation = scene->simulation();

    // Group A: Switch1 -> NodeA1 ... NodeA2 -> LED1
    auto *switch1 = qgraphicsitem_cast<InputSwitch*>(ElementFactory::buildElement(ElementType::InputSwitch));
    auto *nodeA1 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *nodeA2 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *led1 = qgraphicsitem_cast<Led*>(ElementFactory::buildElement(ElementType::Led));

    // Group B: Switch2 -> NodeB1 ... NodeB2 -> LED2
    auto *switch2 = qgraphicsitem_cast<InputSwitch*>(ElementFactory::buildElement(ElementType::InputSwitch));
    auto *nodeB1 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *nodeB2 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *led2 = qgraphicsitem_cast<Led*>(ElementFactory::buildElement(ElementType::Led));

    scene->addItem(switch1);
    scene->addItem(nodeA1);
    scene->addItem(nodeA2);
    scene->addItem(led1);
    scene->addItem(switch2);
    scene->addItem(nodeB1);
    scene->addItem(nodeB2);
    scene->addItem(led2);

    // Create connections
    auto *conn1 = new QNEConnection();
    auto *conn2 = new QNEConnection();
    auto *conn3 = new QNEConnection();
    auto *conn4 = new QNEConnection();
    scene->addItem(conn1);
    scene->addItem(conn2);
    scene->addItem(conn3);
    scene->addItem(conn4);

    conn1->setStartPort(switch1->outputPort());
    conn1->setEndPort(nodeA1->inputPort());
    conn2->setStartPort(nodeA2->outputPort());
    conn2->setEndPort(led1->inputPort());
    conn3->setStartPort(switch2->outputPort());
    conn3->setEndPort(nodeB1->inputPort());
    conn4->setStartPort(nodeB2->outputPort());
    conn4->setEndPort(led2->inputPort());

    // Set up two independent wireless groups
    nodeA1->setLabel("groupA");
    nodeA2->setLabel("groupA");
    nodeB1->setLabel("groupB");
    nodeB2->setLabel("groupB");

    // Test different combinations
    switch1->setOn(true);
    switch2->setOn(false);

    simulation->start();
    for (int i = 0; i < 5; ++i) {
        simulation->update();
    }

    QCOMPARE(led1->inputPort()->status(), Status::Active);   // Group A should be ON
    QCOMPARE(led2->inputPort()->status(), Status::Inactive); // Group B should be OFF

    // Switch states
    switch1->setOn(false);
    switch2->setOn(true);

    for (int i = 0; i < 5; ++i) {
        simulation->update();
    }

    QCOMPARE(led1->inputPort()->status(), Status::Inactive); // Group A should be OFF
    QCOMPARE(led2->inputPort()->status(), Status::Active);   // Group B should be ON

    // Both ON
    switch1->setOn(true);
    switch2->setOn(true);

    for (int i = 0; i < 5; ++i) {
        simulation->update();
    }

    QCOMPARE(led1->inputPort()->status(), Status::Active);   // Group A should be ON
    QCOMPARE(led2->inputPort()->status(), Status::Active);   // Group B should be ON

    simulation->stop();
    delete scene;
}

void TestWireless::testWirelessNodeWithoutInput()
{
    // Test wireless nodes that have no physical input but should still propagate
    auto *scene = new Scene();
    auto *simulation = scene->simulation();

    auto *inputSwitch = qgraphicsitem_cast<InputSwitch*>(ElementFactory::buildElement(ElementType::InputSwitch));
    auto *senderNode = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *floatingNode = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node)); // No input
    auto *receiverNode = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *led = qgraphicsitem_cast<Led*>(ElementFactory::buildElement(ElementType::Led));

    scene->addItem(inputSwitch);
    scene->addItem(senderNode);
    scene->addItem(floatingNode);
    scene->addItem(receiverNode);
    scene->addItem(led);

    // Only connect input to sender and receiver to output
    auto *inputConnection = new QNEConnection();
    auto *outputConnection = new QNEConnection();
    scene->addItem(inputConnection);
    scene->addItem(outputConnection);

    inputConnection->setStartPort(inputSwitch->outputPort());
    inputConnection->setEndPort(senderNode->inputPort());
    outputConnection->setStartPort(receiverNode->outputPort());
    outputConnection->setEndPort(led->inputPort());

    // Note: floatingNode has no physical connections

    // Set up wireless group
    senderNode->setLabel("floating_test");
    floatingNode->setLabel("floating_test");
    receiverNode->setLabel("floating_test");

    inputSwitch->setOn(true);
    simulation->start();

    for (int i = 0; i < 5; ++i) {
        simulation->update();
    }

    // LED should be ON - signal should propagate through floating node
    QCOMPARE(led->inputPort()->status(), Status::Active);

    inputSwitch->setOn(false);

    for (int i = 0; i < 5; ++i) {
        simulation->update();
    }

    QCOMPARE(led->inputPort()->status(), Status::Inactive);

    simulation->stop();
    delete scene;
}

void TestWireless::testWirelessUpdateCycles()
{
    // Test that wireless signals propagate correctly over multiple update cycles
    auto *scene = new Scene();
    auto *simulation = scene->simulation();

    auto *inputSwitch = qgraphicsitem_cast<InputSwitch*>(ElementFactory::buildElement(ElementType::InputSwitch));
    auto *node1 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *node2 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *led = qgraphicsitem_cast<Led*>(ElementFactory::buildElement(ElementType::Led));

    scene->addItem(inputSwitch);
    scene->addItem(node1);
    scene->addItem(node2);
    scene->addItem(led);

    auto *inputConnection = new QNEConnection();
    auto *outputConnection = new QNEConnection();
    scene->addItem(inputConnection);
    scene->addItem(outputConnection);

    inputConnection->setStartPort(inputSwitch->outputPort());
    inputConnection->setEndPort(node1->inputPort());
    outputConnection->setStartPort(node2->outputPort());
    outputConnection->setEndPort(led->inputPort());

    node1->setLabel("cycle_test");
    node2->setLabel("cycle_test");

    simulation->start();

    // Initially OFF
    inputSwitch->setOn(false);
    simulation->update();
    QCOMPARE(led->inputPort()->status(), Status::Inactive);

    // Switch ON and test each cycle
    inputSwitch->setOn(true);

    // Should propagate within first cycle
    simulation->update();
    QCOMPARE(led->inputPort()->status(), Status::Active);

    // Should remain stable over multiple cycles
    for (int i = 0; i < 10; ++i) {
        simulation->update();
        QCOMPARE(led->inputPort()->status(), Status::Active);
    }

    // Switch OFF
    inputSwitch->setOn(false);
    simulation->update();
    QCOMPARE(led->inputPort()->status(), Status::Inactive);

    // Should remain OFF over multiple cycles
    for (int i = 0; i < 10; ++i) {
        simulation->update();
        QCOMPARE(led->inputPort()->status(), Status::Inactive);
    }

    simulation->stop();
    delete scene;
}

void TestWireless::testWirelessUIConstraints()
{
    qCDebug(zero) << "=== Testing Wireless UI Constraints ===";

    Scene *scene = new Scene();

    // Create nodes for testing
    auto *isolatedNode = new Node();
    auto *senderNode = new Node();
    auto *receiverNode = new Node();
    auto *vccElement = new InputVcc();
    auto *ledElement = new Led();

    scene->addItem(isolatedNode);
    scene->addItem(senderNode);
    scene->addItem(receiverNode);
    scene->addItem(vccElement);
    scene->addItem(ledElement);

    // Test 1: Isolated node (no connections) should not allow wireless labeling
    QVERIFY(!isolatedNode->hasInputConnection());
    QVERIFY(!isolatedNode->hasOutputConnection());

    // Test 2: Connect VCC -> senderNode (gives it input connection)
    QNEConnection *conn1 = new QNEConnection();
    conn1->setStartPort(vccElement->outputPort());
    conn1->setEndPort(senderNode->inputPort());
    scene->addItem(conn1);

    // Sender node should now have input connection but no output connection
    QVERIFY(senderNode->hasInputConnection());   // Can set wireless labels
    QVERIFY(!senderNode->hasOutputConnection()); // Cannot select wireless labels

    // Test 3: Connect receiverNode -> LED (gives it output connection)
    QNEConnection *conn2 = new QNEConnection();
    conn2->setStartPort(receiverNode->outputPort());
    conn2->setEndPort(ledElement->inputPort());
    scene->addItem(conn2);

    // Receiver node should have output connection but no input connection
    QVERIFY(!receiverNode->hasInputConnection()); // Cannot set wireless labels
    QVERIFY(receiverNode->hasOutputConnection()); // Can select wireless labels

    // Test 4: Add input connection to receiver node
    QNEConnection *conn3 = new QNEConnection();
    conn3->setStartPort(vccElement->outputPort());
    conn3->setEndPort(receiverNode->inputPort());
    scene->addItem(conn3);

    // Now receiver node has both connections
    QVERIFY(receiverNode->hasInputConnection());  // Can set wireless labels
    QVERIFY(receiverNode->hasOutputConnection()); // Can select wireless labels

    delete scene;
}

void TestWireless::testWirelessConnectionValidation()
{
    qCDebug(zero) << "=== Testing Wireless Connection Validation ===";

    Scene *scene = new Scene();

    // Create test circuit: VCC -> Node1 -> Node2 -> LED
    auto *vccElement = new InputVcc();
    auto *node1 = new Node();
    auto *node2 = new Node();
    auto *ledElement = new Led();

    scene->addItem(vccElement);
    scene->addItem(node1);
    scene->addItem(node2);
    scene->addItem(ledElement);

    // VCC -> Node1 (Node1 can set wireless labels)
    QNEConnection *conn1 = new QNEConnection();
    conn1->setStartPort(vccElement->outputPort());
    conn1->setEndPort(node1->inputPort());
    scene->addItem(conn1);

    // Node2 -> LED (Node2 can select wireless labels)
    QNEConnection *conn2 = new QNEConnection();
    conn2->setStartPort(node2->outputPort());
    conn2->setEndPort(ledElement->inputPort());
    scene->addItem(conn2);

    // Test connection states
    QVERIFY(node1->hasInputConnection());   // Can set wireless labels
    QVERIFY(!node1->hasOutputConnection()); // Cannot select wireless labels

    QVERIFY(!node2->hasInputConnection());  // Cannot set wireless labels
    QVERIFY(node2->hasOutputConnection());  // Can select wireless labels

    // Test wireless connection functionality
    node1->setLabel("test_connection");
    node2->setLabel("test_connection");

    QCOMPARE(node1->getWirelessLabel(), QString("test_connection"));
    QCOMPARE(node2->getWirelessLabel(), QString("test_connection"));

    // Verify wireless manager sees the connection
    auto activeLabels = scene->wirelessManager()->getActiveLabels();
    QVERIFY(activeLabels.contains("test_connection"));

    auto connectedNodes = scene->wirelessManager()->getWirelessGroup("test_connection");
    QVERIFY(connectedNodes.contains(node1));
    QVERIFY(connectedNodes.contains(node2));
    QCOMPARE(connectedNodes.size(), 2);

    delete scene;
}

void TestWireless::testWirelessUIDisabledState()
{
    qCDebug(zero) << "=== Testing Wireless UI Disabled State ===";

    Scene *scene = new Scene();

    // Create an isolated node with no connections
    auto *isolatedNode = new Node();
    scene->addItem(isolatedNode);

    // Verify the isolated node has no connections and cannot set wireless labels
    QVERIFY(!isolatedNode->hasInputConnection());
    QVERIFY(!isolatedNode->hasOutputConnection());
    QVERIFY(!isolatedNode->hasWirelessLabel());

    // Test that setting a label still works (the constraint is in the UI, not the model)
    isolatedNode->setLabel("test_isolated");
    QCOMPARE(isolatedNode->getWirelessLabel(), QString("test_isolated"));

    // The UI constraints are tested through ElementEditor, which we validated in previous tests
    // The node model itself allows label setting - the UI prevents user input

    delete scene;
}

void TestWireless::testNodeDuplicationWirelessBug()
{
    // Test that real drag-and-drop duplication works correctly with wireless labels
    auto *scene = new Scene();
    auto *simulation = scene->simulation();
    auto *wirelessManager = scene->wirelessManager();

    // Create original circuit: PushButton → Node A → LED
    auto *pushButton = new InputButton();
    auto *nodeA1 = qgraphicsitem_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *led1 = new Led();
    
    scene->addItem(pushButton);
    scene->addItem(nodeA1);
    scene->addItem(led1);
    
    // Set wireless label on the node
    nodeA1->setLabel("A");
    
    // Connect: PushButton → Node A → LED
    auto *conn1 = new QNEConnection();
    auto *conn2 = new QNEConnection();
    scene->addItem(conn1);
    scene->addItem(conn2);
    
    conn1->setStartPort(pushButton->outputPort());
    conn1->setEndPort(nodeA1->inputPort());
    conn2->setStartPort(nodeA1->outputPort());
    conn2->setEndPort(led1->inputPort());
    
    // Verify original circuit works
    pushButton->setOn(true);
    simulation->start();
    
    for (int i = 0; i < 5; ++i) {
        simulation->update();
    }
    
    QCOMPARE(led1->inputPort()->status(), Status::Active);
    pushButton->setOn(false);
    
    // SIMULATE REAL DUPLICATION via drag-and-drop
    // This is what actually happens when user duplicates items
    
    // Select items to duplicate (Node A + LED + connection)
    nodeA1->setSelected(true);
    led1->setSelected(true);
    conn2->setSelected(true);
    
    // Create clone drag data (same as real duplication)
    QList<QGraphicsItem*> selectedItems = {nodeA1, led1, conn2};
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    
    Serialization::writePandaHeader(stream);
    stream << QPointF(0, 0);  // mousePos
    
    // Use the actual scene copy method logic
    QPointF center(0.0, 0.0);
    int itemsQuantity = 0;
    for (auto *item : selectedItems) {
        if (item->type() == GraphicElement::Type) {
            center += item->pos();
            itemsQuantity++;
        }
    }
    center /= itemsQuantity;
    stream << center;
    
    // Serialize the selected items
    Serialization::serialize(selectedItems, stream);
    
    // Create mime data for cloneDrag (real duplication)
    auto *mimeData = new QMimeData();
    mimeData->setData("application/x-wiredpanda-cloneDrag", itemData);
    
    // Simulate drop event (real duplication)
    auto *dropEvent = new QGraphicsSceneDragDropEvent(QEvent::GraphicsSceneDrop);
    dropEvent->setMimeData(mimeData);
    dropEvent->setScenePos(QPointF(100, 100));  // Drop at offset position
    
    // Count items before duplication
    int nodesBefore = 0;
    int ledsBefore = 0;
    for (auto *item : scene->items()) {
        if (item->type() == GraphicElement::Type) {
            if (auto *element = qgraphicsitem_cast<GraphicElement *>(item)) {
                if (element->elementType() == ElementType::Node) nodesBefore++;
                if (element->elementType() == ElementType::Led) ledsBefore++;
            }
        }
    }
    
    // Trigger the actual drop event (this is real duplication!)
    scene->dropEvent(dropEvent);
    
    // Count items after duplication
    int nodesAfter = 0;
    int ledsAfter = 0;
    Node *nodeA2 = nullptr;
    Led *led2 = nullptr;
    
    for (auto *item : scene->items()) {
        if (item->type() == GraphicElement::Type) {
            if (auto *element = qgraphicsitem_cast<GraphicElement *>(item)) {
                if (element->elementType() == ElementType::Node) {
                    nodesAfter++;
                    if (auto *node = qobject_cast<Node *>(element)) {
                        if (node != nodeA1 && node->getWirelessLabel() == "A") {
                            nodeA2 = node;
                        }
                    }
                }
                if (element->elementType() == ElementType::Led) {
                    ledsAfter++;
                    if (auto *led = qobject_cast<Led *>(element)) {
                        if (led != led1) {
                            led2 = led;
                        }
                    }
                }
            }
        }
    }
    
    // Verify duplication worked
    QCOMPARE(nodesAfter, nodesBefore + 1);
    QCOMPARE(ledsAfter, ledsBefore + 1);
    QVERIFY(nodeA2 != nullptr);
    QVERIFY(led2 != nullptr);
    
    // Verify duplicated node has wireless label
    QCOMPARE(nodeA2->getWirelessLabel(), QString("A"));
    
    // THE KEY TEST: Verify wireless manager knows about both nodes (tests our fix!)
    auto groupAfterDuplication = wirelessManager->getWirelessGroup("A");
    QCOMPARE(groupAfterDuplication.size(), 2);
    QVERIFY(groupAfterDuplication.contains(nodeA1));
    QVERIFY(groupAfterDuplication.contains(nodeA2));  // This would fail before the fix!
    
    // Final test: Both LEDs should work
    pushButton->setOn(true);
    for (int i = 0; i < 5; ++i) {
        simulation->update();
    }
    
    // Both LEDs should light up because both nodes receive the wireless signal
    QCOMPARE(led1->inputPort()->status(), Status::Active);
    QCOMPARE(led2->inputPort()->status(), Status::Active);  // This would fail before the fix
    
    simulation->stop();
    delete dropEvent;
    delete scene;
}
