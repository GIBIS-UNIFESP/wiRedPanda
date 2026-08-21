// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestSceneState.h"

#include <memory>

#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Scene/ConnectionManager.h"
#include "App/Scene/PortHoverLabel.h"
#include "App/Scene/Scene.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
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

    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Or));
    elem->setPos(200, 200);
    scene.addItem(elem.get());

    QRectF rectWithElement = scene.sceneRect();
    QVERIFY(rectWithElement.width() > 0);

    scene.removeItem(elem.get());
    QRectF rectAfterRemove = scene.sceneRect();

    // After removing element, scene rect should shrink (or stay same if minimum)
    // Verify rect dimensions are properly updated (not just checking they're >= 0)
    QVERIFY(rectAfterRemove.width() >= 0);
    QVERIFY(rectAfterRemove.height() >= 0);
    // Verify the rect actually changed or is properly reset
    QVERIFY(rectAfterRemove.width() <= rectWithElement.width() ||
            rectAfterRemove.isEmpty());
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
    QVERIFY(!items.isEmpty());
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
    QVERIFY2(!itemsAtElement.isEmpty(), "Scene should return items at element position");

    // Query at empty position should return fewer/no circuit items
    QList<QGraphicsItem *> itemsAtEmpty = scene.items(QPointF(500, 500));
    QVERIFY2(itemsAtEmpty.size() < itemsAtElement.size() || itemsAtEmpty.isEmpty(),
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
    QVERIFY(!elements.isEmpty());

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
    // Actually drive ConnectionManager's in-progress-wire state, not just simulation() identity.
    Scene scene;

    auto *elem = ElementFactory::buildElement(ElementType::And);
    scene.addItem(elem);

    QVERIFY(!scene.connectionManager()->hasEditedConnection());
    QVERIFY(!scene.connectionManager()->editedConnection());

    scene.connectionManager()->startFromOutput(elem->outputPort(0));
    QVERIFY(scene.connectionManager()->hasEditedConnection());
    Connection *edited = scene.connectionManager()->editedConnection();
    QVERIFY(edited);
    QCOMPARE(edited->startPort(), elem->outputPort(0));

    scene.connectionManager()->cancel();
    QVERIFY(!scene.connectionManager()->hasEditedConnection());
    QVERIFY(!scene.connectionManager()->editedConnection());
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
    Port *outPort = input->outputPort(0);
    Port *inPort = and1->inputPort(0);
    QVERIFY(outPort != nullptr && inPort != nullptr);

    // Create a connection
    auto conn = std::make_unique<Connection>(nullptr);
    conn->setStartPort(dynamic_cast<OutputPort *>(outPort));
    conn->setEndPort(dynamic_cast<InputPort *>(inPort));
    scene.addItem(conn.get());

    // Verify connection exists in scene
    QList<QGraphicsItem *> connItems = scene.items();
    bool connectionFound = false;
    for (QGraphicsItem *item : std::as_const(connItems)) {
        auto *c = dynamic_cast<Connection *>(item);
        if (c == conn.get()) {
            connectionFound = true;
            break;
        }
    }
    QVERIFY2(connectionFound, "Connection should exist in scene");

    // Clear connection - remove it from scene
    scene.removeItem(conn.get());

    // Verify connection is no longer in scene
    connItems = scene.items();
    for (QGraphicsItem *item : std::as_const(connItems)) {
        auto *c = dynamic_cast<Connection *>(item);
        QVERIFY2(c != conn.get(), "Removed connection should not be in scene");
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
    Port *inputPort = input->outputPort(0);
    Port *and1Port1 = and1->inputPort(0);
    Port *and1Port2 = and1->inputPort(1);
    Port *orPort = or1->inputPort(0);

    // Connect input to AND gate
    auto conn1 = std::make_unique<Connection>(nullptr);
    conn1->setStartPort(dynamic_cast<OutputPort *>(inputPort));
    conn1->setEndPort(dynamic_cast<InputPort *>(and1Port1));
    scene.addItem(conn1.get());

    auto *conn2 = new Connection(nullptr);
    conn2->setStartPort(dynamic_cast<OutputPort *>(inputPort));
    conn2->setEndPort(dynamic_cast<InputPort *>(and1Port2));
    scene.addItem(conn2);

    // Connect AND output to OR gate
    Port *andOutput = and1->outputPort(0);
    auto *conn3 = new Connection(nullptr);
    conn3->setStartPort(dynamic_cast<OutputPort *>(andOutput));
    conn3->setEndPort(dynamic_cast<InputPort *>(orPort));
    scene.addItem(conn3);

    // Count connections in scene
    int connectionCount = TestUtils::countConnections(&scene);

    // Should have created 3 connections
    QVERIFY2(connectionCount >= 3, "Scene should track all created connections");

    // Remove a connection and verify it's no longer tracked
    scene.removeItem(conn1.get());
    conn1.reset();

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
    auto *conn1 = new Connection(nullptr);
    conn1->setStartPort(input->outputPort(0));
    conn1->setEndPort(and1->inputPort(0));
    scene.addItem(conn1);

    int connectionsAfterConn1 = TestUtils::countConnections(&scene);
    QCOMPARE(connectionsAfterConn1, connectionsBefore + 1);

    // Create second connection (AND → LED)
    auto *conn2 = new Connection(nullptr);
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

namespace {
int hoverLabelCount(Scene &scene)
{
    int count = 0;
    for (auto *item : scene.items()) {
        if (item->type() == PortHoverLabel::Type) {
            ++count;
        }
    }
    return count;
}
} // namespace

void TestSceneState::testHoverPortSetGet()
{
    // Drive ConnectionManager::showHoverLabels()/clearHover(), the actual public hover-label
    // API, instead of unrelated autosave/simulation-identity checks.
    Scene scene;

    auto *elem = ElementFactory::buildElement(ElementType::And);
    scene.addItem(elem);
    Port *outPort = elem->outputPort(0);
    outPort->setName("out"); // showHoverLabels() skips unnamed ports (most basic gates)

    QCOMPARE(hoverLabelCount(scene), 0);

    scene.connectionManager()->showHoverLabels(outPort);
    QCOMPARE(hoverLabelCount(scene), 1);

    scene.connectionManager()->clearHover();
    QCOMPARE(hoverLabelCount(scene), 0);
}

void TestSceneState::testHoverPortRelease()
{
    // A second showHoverLabels() call must replace the previous chips, not accumulate them,
    // and clearHover() must release everything -- not just leave the element count unaffected.
    Scene scene;

    auto *elem1 = ElementFactory::buildElement(ElementType::And);
    elem1->setPos(0, 0);
    scene.addItem(elem1);
    Port *port1 = elem1->outputPort(0);
    port1->setName("a");

    auto *elem2 = ElementFactory::buildElement(ElementType::And);
    elem2->setPos(100, 0);
    scene.addItem(elem2);
    Port *port2 = elem2->outputPort(0);
    port2->setName("b");

    scene.connectionManager()->showHoverLabels(port1);
    QCOMPARE(hoverLabelCount(scene), 1);

    scene.connectionManager()->showHoverLabels(port2);
    QCOMPARE(hoverLabelCount(scene), 1);

    scene.connectionManager()->clearHover();
    QCOMPARE(hoverLabelCount(scene), 0);
}

void TestSceneState::testHoverPortTracking()
{
    // Hovering a connected port must reveal labels for BOTH the hovered port itself and every
    // port on the other end of its wires (connectedPeers()), not just itself.
    Scene scene;

    auto *elem1 = ElementFactory::buildElement(ElementType::And);
    auto *elem2 = ElementFactory::buildElement(ElementType::Or);

    elem1->setPos(0, 0);
    elem2->setPos(50, 0);

    scene.addItem(elem1);
    scene.addItem(elem2);

    Port *port1 = elem1->outputPort(0);
    Port *port2 = elem2->inputPort(0);
    port1->setName("driver");
    port2->setName("receiver");

    auto *conn = new Connection(nullptr);
    conn->setStartPort(dynamic_cast<OutputPort *>(port1));
    conn->setEndPort(dynamic_cast<InputPort *>(port2));
    scene.addItem(conn);

    scene.connectionManager()->showHoverLabels(port1);

    QStringList labelTexts;
    for (auto *item : scene.items()) {
        if (auto *label = qgraphicsitem_cast<PortHoverLabel *>(item)) {
            labelTexts << label->text();
        }
    }
    labelTexts.sort();
    QCOMPARE(labelTexts, QStringList({"driver", "receiver"}));
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
    const auto initialItemCount = scene.items().size();

    // Add elements (track them so we can free only what we allocated;
    // scene.items() also returns the stack-allocated m_selectionRect, which
    // must not be deleted)
    QVector<GraphicElement *> added;
    for (int i = 0; i < 3; ++i) {
        auto *elem = ElementFactory::buildElement(ElementType::And);
        elem->setPos(i * 100, 0);
        scene.addItem(elem);
        added.append(elem);
    }

    // Remove and delete only the heap-allocated items we added
    for (auto *elem : added) {
        scene.removeItem(elem);
        delete elem;
    }

    // Scene should be back to its initial item count (the persistent selection rect)
    QCOMPARE(scene.items().size(), initialItemCount);
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

    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    scene.addItem(elem.get());
    auto countWithElem = scene.items().size();

    scene.removeItem(elem.get());
    auto countAfterRemove = scene.items().size();
    QVERIFY(countAfterRemove <= countWithElem);
}

void TestSceneState::testConnectionCountTracking()
{
    Scene scene;

    auto *input = ElementFactory::buildElement(ElementType::InputButton);
    auto *output = ElementFactory::buildElement(ElementType::Led);
    scene.addItem(input);
    scene.addItem(output);

    auto *conn = new Connection();
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

void TestSceneState::testElementSelectionStateToggle()
{
    // Renamed from testElementZOrderOnSelection: neither GraphicElement nor Scene's selection
    // handling ever calls setZValue() on selection -- confirmed by grepping App/ for
    // setZValue()/stackBefore() (the only callers are Connection's fixed behind-elements
    // value, PortHoverLabel, InlineLabelEditor, and Node's wireless indicator, none tied to
    // selection). There is no Z-order effect of selection to test here; this covers what the
    // body actually exercises -- independent per-element selection-state tracking.
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

    auto *conn = new Connection();
    conn->setStartPort(input->outputPort(0));
    conn->setEndPort(output->inputPort(0));
    scene.addItem(conn);

    // Verify items are in scene with proper relationships
    QVERIFY(scene.items().contains(input));
    QVERIFY(scene.items().contains(output));
    QVERIFY(scene.items().contains(conn));

    // The actual Z-order fact: Connection's constructor fixes its zValue() to -1, so it
    // always paints behind elements (default zValue 0, never explicitly set by GraphicElement).
    QCOMPARE(conn->zValue(), -1.0);
    QCOMPARE(input->zValue(), 0.0);
    QCOMPARE(output->zValue(), 0.0);
    QVERIFY(conn->zValue() < input->zValue());
}

void TestSceneState::testElementSelectionStateRoundTrip()
{
    // Renamed from testBringToFrontAfterSelection -- see testElementSelectionStateToggle's
    // comment: there is no bring-to-front/Z-order effect of selection anywhere in production
    // code. This covers what the body actually exercises: a plain select/deselect round trip.
    Scene scene;

    auto *elem = ElementFactory::buildElement(ElementType::And);
    elem->setPos(0, 0);
    scene.addItem(elem);

    elem->setSelected(true);
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

    auto elem = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::And));
    elem->setPos(100, 100);
    scene.addItem(elem.get());

    QRectF rectBefore = scene.sceneRect();

    scene.removeItem(elem.get());
    QRectF rectAfter = scene.sceneRect();

    // Scene should have valid bounds both before and after
    QVERIFY(rectBefore.isValid());
    QVERIFY(rectAfter.isValid() || rectAfter.isEmpty());
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

    auto *conn = new Connection();
    conn->setStartPort(input->outputPort(0));
    conn->setEndPort(output->inputPort(0));
    scene.addItem(conn);

    // Move input element
    input->setPos(50, 0);

    // Connection should still be valid
    QVERIFY(conn->startPort() != nullptr);
    QVERIFY(conn->endPort() != nullptr);
}
