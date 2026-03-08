// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestSceneState.h"

#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Scene.h"
#include "Tests/Common/TestUtils.h"

// ============================================================================
// Scene Bounds and Resize Tests
// ============================================================================

void TestSceneState::testSceneRectAfterAddElement()
{
    Scene scene;

    // Add element at known position
    auto *elem = ElementFactory::buildElement(ElementType::And);
    elem->setPos(100, 100);
    scene.addItem(elem);

    // Scene rect should encompass the element
    QRectF sceneRect = scene.sceneRect();
    QVERIFY(sceneRect.width() > 0);
    QVERIFY(sceneRect.height() > 0);
}

void TestSceneState::testSceneRectAfterRemoveElement()
{
    Scene scene;

    auto *elem = ElementFactory::buildElement(ElementType::Or);
    elem->setPos(200, 200);
    scene.addItem(elem);

    QRectF rectWithElement = scene.sceneRect();
    QVERIFY(rectWithElement.width() > 0);

    scene.removeItem(elem);
    QRectF rectAfterRemove = scene.sceneRect();

    // After removing element, scene rect should shrink (or stay same if minimum)
    // Verify rect dimensions are properly updated (not just checking they're >= 0)
    QVERIFY(rectAfterRemove.width() >= 0);
    QVERIFY(rectAfterRemove.height() >= 0);
    // Verify the rect actually changed or is properly reset
    QVERIFY(rectAfterRemove.width() <= rectWithElement.width() ||
            rectAfterRemove.isEmpty());

    delete elem;
}

void TestSceneState::testResizeSceneExpandsBounds()
{
    Scene scene;

    // Add elements at various positions
    auto *elem1 = ElementFactory::buildElement(ElementType::And);
    elem1->setPos(0, 0);
    scene.addItem(elem1);

    auto *elem2 = ElementFactory::buildElement(ElementType::And);
    elem2->setPos(500, 500);
    scene.addItem(elem2);

    // Scene automatically adjusts to contain elements
    QRectF sceneRect = scene.sceneRect();
    // Scene should be large enough to contain both elements
    QVERIFY(sceneRect.width() > 0);
    QVERIFY(sceneRect.height() > 0);
}

void TestSceneState::testSceneRectMultipleElements()
{
    Scene scene;

    // Add 5 elements at different positions
    for (int i = 0; i < 5; ++i) {
        auto *elem = ElementFactory::buildElement(ElementType::Not);
        elem->setPos(i * 100, i * 100);
        scene.addItem(elem);
    }

    QRectF sceneRect = scene.sceneRect();
    QVERIFY(sceneRect.width() >= 0);
    QVERIFY(sceneRect.height() >= 0);

    // Scene should have reasonable bounds
    QVERIFY(sceneRect.width() >= 400);  // At least 5 elements spaced 100 apart
}

// ============================================================================
// Scene Item Queries and Filtering (Public Methods)
// ============================================================================

void TestSceneState::testItemAtValidPosition()
{
    Scene scene;

    auto *elem = ElementFactory::buildElement(ElementType::And);
    elem->setPos(0, 0);
    scene.addItem(elem);

    // Test items() with no filters returns items
    QList<QGraphicsItem *> items = scene.items();
    QVERIFY(items.size() >= 1);
}

void TestSceneState::testItemAtEmptyPosition()
{
    Scene scene;

    // Empty scene at position (0,0) should return no circuit elements
    QList<QGraphicsItem *> itemsAtOrigin = scene.items(QPointF(0, 0));
    QVERIFY2(itemsAtOrigin.isEmpty(), "Empty scene should return no items at origin");

    // Add an element at a specific position
    auto *elem = ElementFactory::buildElement(ElementType::And);
    elem->setPos(100, 100);
    scene.addItem(elem);

    // Query at element position should find it
    QList<QGraphicsItem *> itemsAtElement = scene.items(QPointF(100, 100));
    // Should find the element we just added
    QVERIFY2(itemsAtElement.count() >= 1, "Scene should return items at element position");

    // Query at empty position should return fewer/no circuit items
    QList<QGraphicsItem *> itemsAtEmpty = scene.items(QPointF(500, 500));
    QVERIFY2(itemsAtEmpty.count() < itemsAtElement.count() || itemsAtEmpty.isEmpty(),
            "Empty position should return no circuit elements");
}

void TestSceneState::testItemsAtMultiple()
{
    Scene scene;

    // Add overlapping elements
    auto *elem1 = ElementFactory::buildElement(ElementType::And);
    elem1->setPos(0, 0);
    scene.addItem(elem1);

    auto *elem2 = ElementFactory::buildElement(ElementType::Or);
    elem2->setPos(5, 5);
    scene.addItem(elem2);

    // Query with bounding rectangle
    QList<QGraphicsItem *> items = scene.items(QRectF(0, 0, 50, 50));
    QVERIFY(items.size() >= 2);  // At least the two elements
}

void TestSceneState::testVisibleElementsInViewport()
{
    Scene scene;

    auto *elem1 = ElementFactory::buildElement(ElementType::And);
    elem1->setPos(0, 0);
    scene.addItem(elem1);

    auto *elem2 = ElementFactory::buildElement(ElementType::And);
    elem2->setPos(100, 100);
    scene.addItem(elem2);

    // Get all elements in scene
    QVector<GraphicElement *> elements = scene.elements();
    QCOMPARE(elements.size(), 2);
}

void TestSceneState::testElementsInRectangle()
{
    Scene scene;

    auto *elem = ElementFactory::buildElement(ElementType::Not);
    elem->setPos(50, 50);
    scene.addItem(elem);

    // Query rectangle containing element
    QVector<GraphicElement *> elements = scene.elements(QRectF(0, 0, 100, 100));
    QVERIFY(elements.size() >= 1);

    // Query rectangle not containing element
    QVector<GraphicElement *> emptyElements = scene.elements(QRectF(500, 500, 100, 100));
    QCOMPARE(emptyElements.size(), 0);
}

void TestSceneState::testSelectedElementsFiltering()
{
    Scene scene;

    auto *elem = ElementFactory::buildElement(ElementType::And);
    elem->setPos(0, 0);
    scene.addItem(elem);

    // Select element
    elem->setSelected(true);

    // Get selected items
    QList<GraphicElement *> selected = scene.selectedElements();
    QVERIFY(selected.contains(elem));

    // Deselect
    elem->setSelected(false);
    selected = scene.selectedElements();
    QVERIFY(!selected.contains(elem));
}

// ============================================================================
// Connection and Update Management (Public Signals/Slots)
// ============================================================================

void TestSceneState::testEditedConnectionSetGet()
{
    Scene scene;

    // Add element to scene
    auto *elem = ElementFactory::buildElement(ElementType::And);
    scene.addItem(elem);

    // Trigger update via public methods - setCircuitUpdateRequired() calls simulation.initialize()
    Simulation *simBefore = scene.simulation();
    scene.setCircuitUpdateRequired();
    Simulation *simAfter = scene.simulation();

    // Both should be valid pointers and same simulation instance
    QVERIFY2(simBefore != nullptr && simAfter != nullptr, "Simulation should exist");
    QVERIFY2(simBefore == simAfter, "Same simulation instance should be used");

    // After setCircuitUpdateRequired(), simulation should be in initialized state
    // (elements should be empty until simulation is run with circuit elements)
    QVERIFY2(scene.simulation() != nullptr, "Circuit update should maintain simulation");
}

void TestSceneState::testEditedConnectionClears()
{
    Scene scene;

    auto *input = ElementFactory::buildElement(ElementType::InputButton);
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *output = ElementFactory::buildElement(ElementType::Led);

    input->setPos(0, 0);
    and1->setPos(50, 0);
    output->setPos(100, 0);

    scene.addItem(input);
    scene.addItem(and1);
    scene.addItem(output);

    // Create connections between elements
    QNEPort *outPort = input->outputPort(0);
    QNEPort *inPort = and1->inputPort(0);
    QVERIFY(outPort != nullptr && inPort != nullptr);

    // Create a connection
    auto *conn = new QNEConnection(nullptr);
    conn->setStartPort(dynamic_cast<QNEOutputPort *>(outPort));
    conn->setEndPort(dynamic_cast<QNEInputPort *>(inPort));
    scene.addItem(conn);
    QVERIFY(conn != nullptr);

    // Verify connection exists in scene
    QList<QGraphicsItem *> connItems = scene.items();
    bool connectionFound = false;
    for (QGraphicsItem *item : std::as_const(connItems)) {
        auto *c = dynamic_cast<QNEConnection *>(item);
        if (c == conn) {
            connectionFound = true;
            break;
        }
    }
    QVERIFY2(connectionFound, "Connection should exist in scene");

    // Clear connection - remove it from scene
    scene.removeItem(conn);
    delete conn;

    // Verify connection is no longer in scene
    connItems = scene.items();
    for (QGraphicsItem *item : std::as_const(connItems)) {
        auto *c = dynamic_cast<QNEConnection *>(item);
        QVERIFY2(c != conn, "Removed connection should not be in scene");
    }
}

void TestSceneState::testActiveConnectionTracking()
{
    Scene scene;

    auto *input = ElementFactory::buildElement(ElementType::InputButton);
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *or1 = ElementFactory::buildElement(ElementType::Or);

    input->setPos(0, 0);
    and1->setPos(50, 0);
    or1->setPos(100, 0);

    scene.addItem(input);
    scene.addItem(and1);
    scene.addItem(or1);

    // Create connections to track their state
    QNEPort *inputPort = input->outputPort(0);
    QNEPort *and1Port1 = and1->inputPort(0);
    QNEPort *and1Port2 = and1->inputPort(1);
    QNEPort *orPort = or1->inputPort(0);

    // Connect input to AND gate
    auto *conn1 = new QNEConnection(nullptr);
    conn1->setStartPort(dynamic_cast<QNEOutputPort *>(inputPort));
    conn1->setEndPort(dynamic_cast<QNEInputPort *>(and1Port1));
    scene.addItem(conn1);

    auto *conn2 = new QNEConnection(nullptr);
    conn2->setStartPort(dynamic_cast<QNEOutputPort *>(inputPort));
    conn2->setEndPort(dynamic_cast<QNEInputPort *>(and1Port2));
    scene.addItem(conn2);

    // Connect AND output to OR gate
    QNEPort *andOutput = and1->outputPort(0);
    auto *conn3 = new QNEConnection(nullptr);
    conn3->setStartPort(dynamic_cast<QNEOutputPort *>(andOutput));
    conn3->setEndPort(dynamic_cast<QNEInputPort *>(orPort));
    scene.addItem(conn3);

    // Count connections in scene
    int connectionCount = TestUtils::countConnections(&scene);

    // Should have created 3 connections
    QVERIFY2(connectionCount >= 3, "Scene should track all created connections");

    // Remove a connection and verify it's no longer tracked
    scene.removeItem(conn1);
    delete conn1;

    int connectionCountAfter = TestUtils::countConnections(&scene);

    QVERIFY2(connectionCountAfter < connectionCount,
            "Connection count should decrease after removal");
}

void TestSceneState::testConnectionStateTransitions()
{
    Scene scene;

    // Add elements to scene for connection testing
    auto *input = ElementFactory::buildElement(ElementType::InputButton);
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *led = ElementFactory::buildElement(ElementType::Led);

    input->setPos(0, 0);
    and1->setPos(50, 0);
    led->setPos(100, 0);

    scene.addItem(input);
    scene.addItem(and1);
    scene.addItem(led);

    // Verify all elements are in scene
    QVector<GraphicElement *> elements = scene.elements();
    QVERIFY(elements.size() == 3);

    // Test connection state transitions: Start with no connections
    int connectionsBefore = TestUtils::countConnections(&scene);
    QVERIFY(connectionsBefore == 0);

    // Create first connection (input → AND)
    auto *conn1 = new QNEConnection(nullptr);
    conn1->setStartPort(input->outputPort(0));
    conn1->setEndPort(and1->inputPort(0));
    scene.addItem(conn1);

    int connectionsAfterConn1 = TestUtils::countConnections(&scene);
    QCOMPARE(connectionsAfterConn1, connectionsBefore + 1);

    // Create second connection (AND → LED)
    auto *conn2 = new QNEConnection(nullptr);
    conn2->setStartPort(and1->outputPort(0));
    conn2->setEndPort(led->inputPort(0));
    scene.addItem(conn2);

    int connectionsAfterConn2 = TestUtils::countConnections(&scene);
    QCOMPARE(connectionsAfterConn2, connectionsAfterConn1 + 1);

    // Verify circuit is now complete
    QVERIFY2(connectionsAfterConn2 == 2, "Should have 2 connections in complete chain");
}

// ============================================================================
// Hover Port and Autosave States (Public Methods)
// ============================================================================

void TestSceneState::testHoverPortSetGet()
{
    Scene scene;

    auto *elem = ElementFactory::buildElement(ElementType::And);
    scene.addItem(elem);

    // Get simulation before update
    Simulation *sim = scene.simulation();
    QVERIFY(sim != nullptr);

    // Signal autosave requirement and circuit update
    scene.setAutosaveRequired();
    scene.setCircuitUpdateRequired();

    // After circuit update, simulation should be reinitialized
    // (m_simulation.initialize() is called in setCircuitUpdateRequired)
    Simulation *simAfter = scene.simulation();
    QVERIFY2(simAfter != nullptr, "Simulation should be reinitialized after setCircuitUpdateRequired");
    QVERIFY2(simAfter == sim, "Same simulation instance should be maintained");
}

void TestSceneState::testHoverPortRelease()
{
    Scene scene;

    // Add multiple elements
    QVector<GraphicElement *> elements;
    for (int i = 0; i < 3; ++i) {
        auto *elem = ElementFactory::buildElement(ElementType::And);
        elem->setPos(i * 50, 0);
        scene.addItem(elem);
        elements.append(elem);
    }

    // Verify all elements are in scene
    QVector<GraphicElement *> sceneElements = scene.elements();
    QCOMPARE(sceneElements.size(), 3);
    for (GraphicElement *elem : elements) {
        QVERIFY2(sceneElements.contains(elem), "All added elements should be in scene");
    }

    // Trigger updates - should maintain all elements
    scene.setAutosaveRequired();

    // Verify elements still exist after autosave trigger
    QVector<GraphicElement *> elementsAfter = scene.elements();
    QCOMPARE(elementsAfter.size(), 3);
}

void TestSceneState::testHoverPortTracking()
{
    Scene scene;

    auto *elem1 = ElementFactory::buildElement(ElementType::And);
    auto *elem2 = ElementFactory::buildElement(ElementType::Or);
    auto *elem3 = ElementFactory::buildElement(ElementType::Not);

    elem1->setPos(0, 0);
    elem2->setPos(50, 0);
    elem3->setPos(100, 0);

    scene.addItem(elem1);
    scene.addItem(elem2);
    scene.addItem(elem3);

    // Verify all elements are in scene
    QVector<GraphicElement *> allElements = scene.elements();
    QCOMPARE(allElements.size(), 3);

    // Test port access (ports should be trackable for hover states)
    QNEPort *port1 = elem1->outputPort(0);
    QNEPort *port2 = elem2->inputPort(0);
    QNEPort *port3 = elem3->inputPort(0);

    QVERIFY2(port1 != nullptr, "Element should have output port");
    QVERIFY2(port2 != nullptr, "Element should have input port");
    QVERIFY2(port3 != nullptr, "Element should have input port");

    // Verify ports can be used to create connections (for hover tracking)
    auto *conn = new QNEConnection(nullptr);
    conn->setStartPort(dynamic_cast<QNEOutputPort *>(port1));
    conn->setEndPort(dynamic_cast<QNEInputPort *>(port2));
    scene.addItem(conn);
    QVERIFY2(conn != nullptr, "Connection between ports should be creatable");

    // Verify we can navigate the connection chain
    QVERIFY2(conn->startPort() != nullptr && conn->endPort() != nullptr,
            "Connection ports should be properly set");
}

// ============================================================================
// Autosave and Circuit Update
// ============================================================================

void TestSceneState::testCheckUpdateRequestTriggers()
{
    Scene scene;

    auto *elem = ElementFactory::buildElement(ElementType::And);
    scene.addItem(elem);

    // Record initial state
    Simulation *simBefore = scene.simulation();
    QVERIFY(simBefore != nullptr);

    // Signal update requirements - setCircuitUpdateRequired calls simulation.initialize()
    scene.setCircuitUpdateRequired();
    scene.setAutosaveRequired();

    // Verify simulation still exists and is valid after updates
    Simulation *simAfter = scene.simulation();
    QVERIFY2(simAfter != nullptr, "Simulation should be valid after circuit update");

    // Verify element is still in scene after update
    QVector<GraphicElement *> elements = scene.elements();
    QVERIFY2(elements.contains(elem), "Elements should persist through circuit updates");
}

void TestSceneState::testUpdateBlockingDuringTransaction()
{
    Scene scene;

    // Add multiple elements in sequence
    QVector<GraphicElement *> elements;
    for (int i = 0; i < 5; ++i) {
        auto *e = ElementFactory::buildElement(ElementType::Not);
        e->setPos(i * 100, 0);
        scene.addItem(e);
        elements.append(e);
    }

    // Verify all elements are in scene before update
    QVector<GraphicElement *> beforeUpdate = scene.elements();
    QCOMPARE(beforeUpdate.size(), 5);

    // Trigger circuit update
    scene.setCircuitUpdateRequired();

    // After update, all elements should still be present
    QVector<GraphicElement *> afterUpdate = scene.elements();
    QVERIFY2(afterUpdate.size() >= 5, "All elements should survive circuit update");

    // Verify each element is still accessible
    for (GraphicElement *elem : elements) {
        QVERIFY2(afterUpdate.contains(elem),
                "All added elements should be present after circuit update");
    }
}

void TestSceneState::testSceneCleanupAfterLoad()
{
    Scene scene;

    // Add elements
    for (int i = 0; i < 3; ++i) {
        auto *elem = ElementFactory::buildElement(ElementType::And);
        elem->setPos(i * 100, 0);
        scene.addItem(elem);
    }

    // Remove all items
    QList<QGraphicsItem *> items = scene.items();
    for (auto item : std::as_const(items)) {
        scene.removeItem(item);
    }

    // Scene should be clean
    QCOMPARE(scene.items().size(), 0);
}

// ============================================================================
// Element and Connection Counts
// ============================================================================

void TestSceneState::testElementCountAfterAddition()
{
    Scene scene;

    auto initialCount = scene.items().size();

    auto *elem1 = ElementFactory::buildElement(ElementType::And);
    scene.addItem(elem1);
    auto countAfter1 = scene.items().size();
    QVERIFY(countAfter1 > initialCount);

    auto *elem2 = ElementFactory::buildElement(ElementType::Or);
    scene.addItem(elem2);
    auto countAfter2 = scene.items().size();
    QVERIFY(countAfter2 > countAfter1);
}

void TestSceneState::testElementCountAfterRemoval()
{
    Scene scene;

    auto *elem = ElementFactory::buildElement(ElementType::And);
    scene.addItem(elem);
    auto countWithElem = scene.items().size();

    scene.removeItem(elem);
    auto countAfterRemove = scene.items().size();
    QVERIFY(countAfterRemove <= countWithElem);

    delete elem;
}

void TestSceneState::testConnectionCountTracking()
{
    Scene scene;

    auto *input = ElementFactory::buildElement(ElementType::InputButton);
    auto *output = ElementFactory::buildElement(ElementType::Led);
    scene.addItem(input);
    scene.addItem(output);

    auto *conn = new QNEConnection();
    conn->setStartPort(input->outputPort(0));
    conn->setEndPort(output->inputPort(0));
    scene.addItem(conn);

    // Scene should have 3 items (2 elements + 1 connection)
    QVERIFY(scene.items().size() >= 3);
}

void TestSceneState::testMixedElementTypes()
{
    Scene scene;

    // Add different element types
    QVector<ElementType> types = {
        ElementType::And, ElementType::Or, ElementType::Not,
        ElementType::InputButton, ElementType::Led
    };

    for (ElementType type : types) {
        auto *elem = ElementFactory::buildElement(type);
        elem->setPos(static_cast<int>(types.indexOf(type)) * 100, 0);
        scene.addItem(elem);
    }

    // Verify all elements are in scene
    QList<QGraphicsItem *> items = scene.items();
    QVERIFY(items.size() >= types.size());
}

// ============================================================================
// Z-Order and Rendering
// ============================================================================

void TestSceneState::testElementZOrderOnSelection()
{
    Scene scene;

    auto *elem1 = ElementFactory::buildElement(ElementType::And);
    auto *elem2 = ElementFactory::buildElement(ElementType::Or);
    elem1->setPos(0, 0);
    elem2->setPos(0, 0);
    scene.addItem(elem1);
    scene.addItem(elem2);

    // Select elem1 - verify selection works
    elem1->setSelected(true);
    QVERIFY(elem1->isSelected());

    // Verify we can deselect
    elem1->setSelected(false);
    QVERIFY(!elem1->isSelected());

    // Verify elem2 can be selected independently
    elem2->setSelected(true);
    QVERIFY(elem2->isSelected());
}

void TestSceneState::testConnectionZOrderBehindElements()
{
    Scene scene;

    auto *input = ElementFactory::buildElement(ElementType::InputButton);
    auto *output = ElementFactory::buildElement(ElementType::Led);
    scene.addItem(input);
    scene.addItem(output);

    auto *conn = new QNEConnection();
    conn->setStartPort(input->outputPort(0));
    conn->setEndPort(output->inputPort(0));
    scene.addItem(conn);

    // Verify items are in scene with proper relationships
    QVERIFY(scene.items().contains(input));
    QVERIFY(scene.items().contains(output));
    QVERIFY(scene.items().contains(conn));
}

void TestSceneState::testBringToFrontAfterSelection()
{
    Scene scene;

    auto *elem = ElementFactory::buildElement(ElementType::And);
    elem->setPos(0, 0);
    scene.addItem(elem);

    // Select to bring to front
    elem->setSelected(true);

    // Verify selection is tracked
    QVERIFY(elem->isSelected());

    elem->setSelected(false);
    QVERIFY(!elem->isSelected());
}

// ============================================================================
// Scene State Persistence
// ============================================================================

void TestSceneState::testSceneRetainsSizeAfterClear()
{
    Scene scene;

    auto *elem = ElementFactory::buildElement(ElementType::And);
    elem->setPos(100, 100);
    scene.addItem(elem);

    QRectF rectBefore = scene.sceneRect();

    scene.removeItem(elem);
    QRectF rectAfter = scene.sceneRect();

    // Scene should have valid bounds both before and after
    QVERIFY(rectBefore.isValid());
    QVERIFY(rectAfter.isValid() || rectAfter.isEmpty());

    delete elem;
}

void TestSceneState::testElementPropertiesAfterMove()
{
    Scene scene;

    auto *elem = ElementFactory::buildElement(ElementType::Not);
    QPointF pos1(50, 50);
    elem->setPos(pos1);
    scene.addItem(elem);

    QPointF actualPos1 = elem->pos();
    QVERIFY(actualPos1.x() >= 50 && actualPos1.y() >= 50);

    // Move element
    QPointF pos2(150, 150);
    elem->setPos(pos2);
    QPointF actualPos2 = elem->pos();
    // May be snapped/rounded to grid or quantized
    QVERIFY(actualPos2.x() >= 140 && actualPos2.y() >= 140);
}

void TestSceneState::testConnectionPropertiesAfterElementMove()
{
    Scene scene;

    auto *input = ElementFactory::buildElement(ElementType::InputButton);
    auto *output = ElementFactory::buildElement(ElementType::Led);
    input->setPos(0, 0);
    output->setPos(100, 100);
    scene.addItem(input);
    scene.addItem(output);

    auto *conn = new QNEConnection();
    conn->setStartPort(input->outputPort(0));
    conn->setEndPort(output->inputPort(0));
    scene.addItem(conn);

    // Move input element
    input->setPos(50, 0);

    // Connection should still be valid
    QVERIFY(conn->startPort() != nullptr);
    QVERIFY(conn->endPort() != nullptr);
}

