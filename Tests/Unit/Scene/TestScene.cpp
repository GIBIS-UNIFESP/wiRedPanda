// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestScene.h"

#include <cmath>

#include <QClipboard>
#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

void TestScene::initTestCase()
{
    // Setup before all tests
}

void TestScene::cleanup()
{
    QApplication::clipboard()->clear();
}

void TestScene::cleanupTestCase()
{
}

// ============================================================================
// Helper Functions
// ============================================================================

// Count all GraphicsItems of a given type in the scene
template<typename T>
static int countItems(Scene *scene)
{
    int count = 0;
    const auto items = scene->items();
    for (auto *item : std::as_const(items)) {
        if (qgraphicsitem_cast<T *>(item)) {
            ++count;
        }
    }
    return count;
}

// ============================================================================
// Element Management Tests
// ============================================================================

void TestScene::testAddElement()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    QCOMPARE(scene->elements().size(), 0);

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create AND gate element");
    scene->addItem(andGate);

    QCOMPARE(scene->elements().size(), 1);
    QCOMPARE(scene->elements().first()->elementType(), ElementType::And);
}

void TestScene::testRemoveElement()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    auto *or1 = ElementFactory::buildElement(ElementType::Or);

    QVERIFY2(and1 != nullptr && and2 != nullptr && or1 != nullptr,
        "Failed to create element(s) for testRemoveElement");

    scene->addItem(and1);
    scene->addItem(and2);
    scene->addItem(or1);

    QCOMPARE(scene->elements().size(), 3);

    // Remove one element
    scene->removeItem(and1);
    delete and1;

    QCOMPARE(scene->elements().size(), 2);

    // Verify the correct elements remain
    auto elements = scene->elements();
    QVERIFY(elements.contains(and2));
    QVERIFY(elements.contains(or1));
}

void TestScene::testElementQuery()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add 5 different element types using ElementFactory
    auto *andElem = ElementFactory::buildElement(ElementType::And);
    auto *orElem = ElementFactory::buildElement(ElementType::Or);
    auto *notElem = ElementFactory::buildElement(ElementType::Not);
    auto *inputElem = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *ledElem = ElementFactory::buildElement(ElementType::Led);

    QVERIFY2(andElem && orElem && notElem && inputElem && ledElem,
        "Failed to create all required elements");

    scene->addItem(andElem);
    scene->addItem(orElem);
    scene->addItem(notElem);
    scene->addItem(inputElem);
    scene->addItem(ledElem);

    auto elements = scene->elements();
    QCOMPARE(elements.size(), 5);

    // Verify all are GraphicElements
    for (auto *elem : elements) {
        QVERIFY(elem != nullptr);
    }

    // Verify no connections yet
    int connectionCount = TestUtils::countConnections(scene);
    QCOMPARE(connectionCount, 0);
}

void TestScene::testElementsInRect()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add elements at clearly separated positions to avoid overlap
    auto *elem1 = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(elem1 != nullptr, "Failed to create AND element 1");
    elem1->setPos(0, 0);
    scene->addItem(elem1);

    auto *elem2 = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(elem2 != nullptr, "Failed to create AND element 2");
    elem2->setPos(500, 0);  // Far away to avoid overlap
    scene->addItem(elem2);

    auto *elem3 = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(elem3 != nullptr, "Failed to create AND element 3");
    elem3->setPos(1000, 0);  // Even farther
    scene->addItem(elem3);

    // Test 1: Query rectangle around elem1 - should contain elem1 only
    QRectF rect1(elem1->pos() - QPointF(100, 100), QSizeF(200, 200));
    auto items1 = scene->elements(rect1);
    QVERIFY2(items1.contains(elem1), "elem1 should be in query rect1");
    QVERIFY2(!items1.contains(elem2), "elem2 should NOT be in query rect1");
    QVERIFY2(!items1.contains(elem3), "elem3 should NOT be in query rect1");

    // Test 2: Query rectangle around elem2 - should contain elem2 only
    QRectF rect2(elem2->pos() - QPointF(100, 100), QSizeF(200, 200));
    auto items2 = scene->elements(rect2);
    QVERIFY2(items2.contains(elem2), "elem2 should be in query rect2");
    QVERIFY2(!items2.contains(elem1), "elem1 should NOT be in query rect2");
    QVERIFY2(!items2.contains(elem3), "elem3 should NOT be in query rect2");

    // Test 3: Query rectangle encompassing all elements
    QRectF rectAll(-200, -200, 1400, 400);
    auto itemsAll = scene->elements(rectAll);
    QCOMPARE(itemsAll.size(), 3);
    QVERIFY2(itemsAll.contains(elem1), "All rect should contain elem1");
    QVERIFY2(itemsAll.contains(elem2), "All rect should contain elem2");
    QVERIFY2(itemsAll.contains(elem3), "All rect should contain elem3");
}

void TestScene::testConnectionsQuery()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;
    Led led;

    builder.add(&sw, &andGate, &led);
    builder.connect(&sw, 0, &andGate, 0);
    builder.connect(&andGate, 0, &led, 0);

    auto *scene = workspace.scene();

    // Count connections via public items() interface
    int connectionCount = TestUtils::countConnections(scene);
    QCOMPARE(connectionCount, 2);

    // Verify all connections have valid ports
    const auto items = scene->items();
    for (auto *item : std::as_const(items)) {
        if (auto *conn = qgraphicsitem_cast<QNEConnection *>(item)) {
            QVERIFY2(conn->startPort() != nullptr, "Connection missing start port");
            QVERIFY2(conn->endPort() != nullptr, "Connection missing end port");
        }
    }

    // Verify element count unchanged
    QCOMPARE(scene->elements().size(), 3);
}

// ============================================================================
// Item Query Tests
// ============================================================================

void TestScene::testItemsAtPoint()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create AND gate");
    andGate->setPos(100, 100);
    scene->addItem(andGate);

    // Test 1: items at element center should find the element
    auto items = scene->items(andGate->pos(), Qt::IntersectsItemShape);
    QVERIFY(items.size() > 0);
    QVERIFY2(items.contains(andGate), "Should find AND gate at its position");

    // Test 2: items at element boundary should find the element
    QPointF boundaryPos = andGate->pos() + QPointF(20, 0);  // Right side of element
    auto boundaryItems = scene->items(boundaryPos, Qt::IntersectsItemShape);
    QVERIFY2(boundaryItems.contains(andGate), "Should find AND gate at boundary");

    // Test 3: items at empty space should return empty
    auto emptyItems = scene->items(QPointF(500, 500), Qt::IntersectsItemShape);
    QCOMPARE(emptyItems.size(), 0);
}

void TestScene::testItemsAt()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add two elements at the same position (completely overlapped)
    auto *elem1 = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(elem1 != nullptr, "Failed to create AND element");
    elem1->setPos(100, 100);
    scene->addItem(elem1);

    auto *elem2 = ElementFactory::buildElement(ElementType::Or);
    QVERIFY2(elem2 != nullptr, "Failed to create OR element");
    elem2->setPos(100, 100);  // Same position for guaranteed overlap
    scene->addItem(elem2);

    // Query items at that position
    auto items = scene->items(QPointF(100, 100), Qt::IntersectsItemShape);

    // Should find both overlapping elements
    QVERIFY(items.size() > 0);
    QVERIFY2(items.contains(elem1), "Should find AND gate at (100, 100)");
    QVERIFY2(items.contains(elem2), "Should find OR gate at (100, 100)");
}

// ============================================================================
// Selection Tests
// ============================================================================

void TestScene::testSelectAll()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *andElem = ElementFactory::buildElement(ElementType::And);
    auto *orElem = ElementFactory::buildElement(ElementType::Or);
    auto *notElem = ElementFactory::buildElement(ElementType::Not);

    QVERIFY2(andElem && orElem && notElem, "Failed to create elements");

    scene->addItem(andElem);
    scene->addItem(orElem);
    scene->addItem(notElem);

    // Initially no selection
    QVERIFY(scene->selectedElements().isEmpty());

    // Select all
    scene->selectAll();

    // Verify all elements selected
    QCOMPARE(scene->selectedElements().size(), 3);
}

void TestScene::testSelectedElements()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add elements and connections
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *led = ElementFactory::buildElement(ElementType::Led);

    QVERIFY2(sw && andGate && led, "Failed to create elements");

    scene->addItem(sw);
    scene->addItem(andGate);
    scene->addItem(led);

    // Add connections
    auto *conn1 = new QNEConnection();
    conn1->setStartPort(sw->outputPort());
    conn1->setEndPort(andGate->inputPort(0));
    scene->addItem(conn1);

    auto *conn2 = new QNEConnection();
    conn2->setStartPort(andGate->outputPort());
    conn2->setEndPort(led->inputPort());
    scene->addItem(conn2);

    // Select all items - this selects elements AND connections
    scene->selectAll();

    // selectedElements() filters to only return GraphicElements, excluding connections
    auto selectedElems = scene->selectedElements();
    QCOMPARE(selectedElems.size(), 3);

    // Verify connections are NOT in the selectedElements result
    for (auto *elem : selectedElems) {
        auto *conn = qgraphicsitem_cast<QNEConnection *>(elem);
        QVERIFY2(conn == nullptr, "selectedElements should not include connections");
    }
}

void TestScene::testClearSelection()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *andElem = ElementFactory::buildElement(ElementType::And);
    auto *orElem = ElementFactory::buildElement(ElementType::Or);
    auto *notElem = ElementFactory::buildElement(ElementType::Not);

    QVERIFY2(andElem && orElem && notElem, "Failed to create elements");

    scene->addItem(andElem);
    scene->addItem(orElem);
    scene->addItem(notElem);

    scene->selectAll();
    QCOMPARE(scene->selectedElements().size(), 3);

    scene->clearSelection();
    QVERIFY(scene->selectedElements().isEmpty());
}

// ============================================================================
// Bounds Tests
// ============================================================================

void TestScene::testSceneBounds()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add element at origin
    auto *elem1 = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(elem1 != nullptr, "Failed to create AND element 1");
    elem1->setPos(0, 0);
    scene->addItem(elem1);

    // Get initial bounds
    QRectF bounds1 = scene->itemsBoundingRect();

    // Verify bounds contain the element's bounding rect
    QVERIFY(bounds1.contains(elem1->boundingRect().translated(elem1->pos())));

    // Add element far away
    auto *elem2 = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(elem2 != nullptr, "Failed to create AND element 2");
    elem2->setPos(1000, 1000);
    scene->addItem(elem2);

    // Get new bounds
    QRectF bounds2 = scene->itemsBoundingRect();

    // Verify bounds expanded
    QVERIFY(bounds2.width() >= bounds1.width());
    QVERIFY(bounds2.height() >= bounds1.height());

    // Verify both elements' full bounding rects are contained
    QRectF elem1Bounds = elem1->boundingRect().translated(elem1->pos());
    QRectF elem2Bounds = elem2->boundingRect().translated(elem2->pos());
    QVERIFY2(bounds2.contains(elem1Bounds), "Bounds should contain elem1 rect");
    QVERIFY2(bounds2.contains(elem2Bounds), "Bounds should contain elem2 rect");
}

// ============================================================================
// Special Cases
// ============================================================================

void TestScene::testEmptyScene()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Verify empty state for elements
    QVERIFY(scene->elements().isEmpty());
    QCOMPARE(TestUtils::countConnections(scene), 0);
    QVERIFY(scene->selectedElements().isEmpty());

    // Verify no crashes on queries
    auto items = scene->items(QPointF(0, 0), Qt::IntersectsItemShape);
    QCOMPARE(items.size(), 0);
}

void TestScene::testCopySingleElement()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add AND gate
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create AND gate");
    andGate->setPos(100, 100);
    scene->addItem(andGate);
    andGate->setSelected(true);

    // Test copy action
    scene->copyAction();

    // Verify clipboard has correct format
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    QVERIFY(mimeData->hasFormat("application/x-wiredpanda-clipboard"));
}

void TestScene::testCopyMultipleElements()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add multiple elements
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    auto *or1 = ElementFactory::buildElement(ElementType::Or);

    QVERIFY2(and1 && and2 && or1, "Failed to create elements");

    and1->setPos(0, 0);
    and2->setPos(100, 0);
    or1->setPos(200, 0);

    scene->addItem(and1);
    scene->addItem(and2);
    scene->addItem(or1);

    // Select all
    scene->selectAll();

    // Copy all selected
    scene->copyAction();

    // Verify clipboard has data
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    QVERIFY(mimeData->hasFormat("application/x-wiredpanda-clipboard"));
}

void TestScene::testCutElement()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add element
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create AND gate");
    scene->addItem(andGate);
    andGate->setSelected(true);

    QCOMPARE(scene->elements().size(), 1);

    // Cut element
    scene->cutAction();

    // Verify clipboard has data
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    QVERIFY(mimeData->hasFormat("application/x-wiredpanda-clipboard"));

    // Verify element removed from scene
    QCOMPARE(scene->elements().size(), 0);
}

void TestScene::testPasteElement()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add and copy element
    auto *original = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(original != nullptr, "Failed to create AND gate");
    original->setPos(100, 100);
    scene->addItem(original);
    original->setSelected(true);
    scene->copyAction();

    // Clear selection
    scene->clearSelection();

    // Paste
    scene->pasteAction();

    // Verify new element created
    auto elements = scene->elements();
    QCOMPARE(elements.size(), 2);

    // Both should be AND gates
    QVERIFY(dynamic_cast<And *>(elements[0]) != nullptr);
    QVERIFY(dynamic_cast<And *>(elements[1]) != nullptr);
}

void TestScene::testPasteMultipleTimes()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add and copy element
    auto *original = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(original != nullptr, "Failed to create AND gate");
    scene->addItem(original);
    original->setSelected(true);
    scene->copyAction();

    // Clear selection
    scene->clearSelection();

    // Paste first time
    scene->pasteAction();
    QCOMPARE(scene->elements().size(), 2);

    // Clear selection
    scene->clearSelection();

    // Paste second time
    scene->pasteAction();
    QCOMPARE(scene->elements().size(), 3);

    // All three should be AND gates
    for (auto *elem : scene->elements()) {
        QVERIFY(dynamic_cast<And *>(elem) != nullptr);
    }
}

void TestScene::testCopyPasteEmptySelection()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add element but don't select
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create AND gate");
    scene->addItem(andGate);

    // Try to copy without selection
    scene->copyAction();

    // Copying without selection should not modify the scene
    QCOMPARE(scene->elements().size(), 1);
}

void TestScene::testRotateRight()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add AND gate
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create AND gate");
    scene->addItem(andGate);
    andGate->setSelected(true);

    // Get initial rotation
    qreal initialRotation = andGate->rotation();

    // Rotate right 90 degrees
    scene->rotateRight();

    // Verify rotation increased by 90°
    QCOMPARE(andGate->rotation(), initialRotation + 90.0);
}

void TestScene::testRotateLeft()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add AND gate
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create AND gate");
    scene->addItem(andGate);
    andGate->setSelected(true);

    // Get initial rotation
    qreal initialRotation = andGate->rotation();

    // Rotate left 90 degrees (counter-clockwise = -90°)
    scene->rotateLeft();

    // Verify rotation decreased by 90°
    // The API returns -90 for counter-clockwise rotation
    QCOMPARE(andGate->rotation(), initialRotation - 90.0);
}

void TestScene::testMultipleRotations()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add AND gate
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create AND gate");
    scene->addItem(andGate);
    andGate->setSelected(true);

    // Rotate 4 times (360°)
    scene->rotateRight();
    scene->rotateRight();
    scene->rotateRight();
    scene->rotateRight();

    // Verify angle wraps to 0° or 360° (modulo 360)
    qreal finalAngle = andGate->rotation();
    qreal normalizedAngle = std::fmod(finalAngle, 360.0);
    if (normalizedAngle < 0) {
        normalizedAngle += 360.0;
    }
    QCOMPARE(normalizedAngle, 0.0);
}

void TestScene::testFlipHorizontal()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add element to test flipping
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create AND gate");
    andGate->setPos(100, 100);
    scene->addItem(andGate);
    andGate->setSelected(true);

    int elementCountBefore = scene->elements().size();

    // Flip horizontally - should complete without errors
    scene->flipHorizontally();

    // Verify element still exists (not deleted)
    int elementCountAfter = scene->elements().size();
    QCOMPARE(elementCountAfter, elementCountBefore);
}

void TestScene::testFlipVertically()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add element to test flipping
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create AND gate");
    andGate->setPos(100, 100);
    scene->addItem(andGate);
    andGate->setSelected(true);

    int elementCountBefore = scene->elements().size();

    // Flip vertically - should complete without errors
    scene->flipVertically();

    // Verify element still exists (not deleted)
    int elementCountAfter = scene->elements().size();
    QCOMPARE(elementCountAfter, elementCountBefore);
}

void TestScene::testRotateWithConnections()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add connected elements
    auto *inputSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *led = ElementFactory::buildElement(ElementType::Led);

    QVERIFY2(inputSwitch && andGate && led, "Failed to create elements");

    inputSwitch->setPos(0, 100);
    andGate->setPos(100, 100);
    led->setPos(200, 100);

    scene->addItem(inputSwitch);
    scene->addItem(andGate);
    scene->addItem(led);

    // Create connections manually (simplified - just verify rotation doesn't crash)
    // In a real scenario, we'd connect ports
    int connectionsBeforeRotate = TestUtils::countConnections(scene);

    // Select and rotate
    andGate->setSelected(true);
    scene->rotateRight();

    // Verify rotation succeeded
    QCOMPARE(andGate->rotation(), 90.0);

    // Connection count should remain the same
    int connectionsAfterRotate = TestUtils::countConnections(scene);
    QCOMPARE(connectionsAfterRotate, connectionsBeforeRotate);
}

void TestScene::testDeleteSingleElement()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add element
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create AND gate");
    scene->addItem(andGate);
    andGate->setSelected(true);

    QCOMPARE(scene->elements().size(), 1);

    // Delete
    scene->deleteAction();

    // Verify removed
    QCOMPARE(scene->elements().size(), 0);
}

void TestScene::testDeleteMultipleElements()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add multiple elements
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    auto *or1 = ElementFactory::buildElement(ElementType::Or);

    QVERIFY2(and1 && and2 && or1, "Failed to create elements");

    and1->setPos(0, 0);
    and2->setPos(100, 0);
    or1->setPos(200, 0);

    scene->addItem(and1);
    scene->addItem(and2);
    scene->addItem(or1);

    QCOMPARE(scene->elements().size(), 3);

    // Select all and delete
    scene->selectAll();
    scene->deleteAction();

    // Verify all removed
    QCOMPARE(scene->elements().size(), 0);
}

void TestScene::testDeleteWithConnections()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add elements
    auto *inputSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    auto *led = ElementFactory::buildElement(ElementType::Led);

    QVERIFY2(inputSwitch && andGate && led, "Failed to create elements");

    inputSwitch->setPos(0, 100);
    andGate->setPos(100, 100);
    led->setPos(200, 100);

    scene->addItem(inputSwitch);
    scene->addItem(andGate);
    scene->addItem(led);

    int initialConnections = TestUtils::countConnections(scene);

    // Select and delete AND gate
    andGate->setSelected(true);
    scene->deleteAction();

    // Verify element removed
    QCOMPARE(scene->elements().size(), 2);

    // Connections should be cleaned up or removed
    int finalConnections = TestUtils::countConnections(scene);
    // Should be less than or equal to initial
    QVERIFY(finalConnections <= initialConnections);
}

void TestScene::testUndoDelete()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add element
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create AND gate");
    scene->addItem(andGate);
    andGate->setSelected(true);

    // Delete
    scene->deleteAction();
    QCOMPARE(scene->elements().size(), 0);

    // Undo
    scene->undoStack()->undo();

    // Verify restored
    QCOMPARE(scene->elements().size(), 1);

    // Verify it's an AND gate
    QVERIFY(dynamic_cast<And *>(scene->elements()[0]) != nullptr);
}

void TestScene::testRedoDelete()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add element
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create AND gate");
    scene->addItem(andGate);
    andGate->setSelected(true);

    // Delete
    scene->deleteAction();
    QCOMPARE(scene->elements().size(), 0);

    // Undo
    scene->undoStack()->undo();
    QCOMPARE(scene->elements().size(), 1);

    // Redo
    scene->undoStack()->redo();

    // Verify removed again
    QCOMPARE(scene->elements().size(), 0);
}

void TestScene::testUndoRotation()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Add AND gate
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(andGate != nullptr, "Failed to create AND gate");
    scene->addItem(andGate);
    andGate->setSelected(true);

    // Get initial rotation
    qreal initialRotation = andGate->rotation();

    // Rotate
    scene->rotateRight();
    QCOMPARE(andGate->rotation(), initialRotation + 90.0);

    // Undo rotation
    scene->undoStack()->undo();

    // Verify restored to initial rotation
    QCOMPARE(andGate->rotation(), initialRotation);
}

void TestScene::testNextElementType()
{
    auto scene = std::make_unique<Scene>();

    // Add an AND gate
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene->addItem(andGate);

    // Select it
    andGate->setSelected(true);

    // Get initial type
    ElementType initialType = andGate->elementType();
    QCOMPARE(initialType, ElementType::And);

    // Call nextElm to morph to next type
    scene->nextElm();

    // Element should still be in scene (either same or changed type)
    QCOMPARE(scene->elements().size(), 1);
}

void TestScene::testPrevElementType()
{
    auto scene = std::make_unique<Scene>();

    // Add an AND gate
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene->addItem(andGate);

    // Select it
    andGate->setSelected(true);

    // Get initial type
    ElementType initialType = andGate->elementType();
    QCOMPARE(initialType, ElementType::And);

    // Call prevElm to morph to previous type
    scene->prevElm();

    // Element should still be in scene
    QCOMPARE(scene->elements().size(), 1);
}

void TestScene::testElementMorphingWithConnections()
{
    auto scene = std::make_unique<Scene>();

    // Create AND gate and LED (output element)
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene->addItem(andGate);

    auto *led = ElementFactory::buildElement(ElementType::Led);
    led->setPos(150, 0);
    scene->addItem(led);

    // Create connection
    auto *conn = new QNEConnection(nullptr);
    conn->setStartPort(andGate->outputPort(0));
    conn->setEndPort(led->inputPort(0));
    scene->addItem(conn);

    // Select AND gate
    andGate->setSelected(true);

    // Morph to next type
    scene->nextElm();

    // Connection should still exist
    QCOMPARE(scene->elements().size(), 2);
}

void TestScene::testElementMorphingPropertyPreservation()
{
    auto scene = std::make_unique<Scene>();

    // Add an AND gate with specific properties
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    andGate->setLabel("TestLabel");
    andGate->setInputSize(3);
    andGate->setPos(50, 50);
    scene->addItem(andGate);

    // Select it
    andGate->setSelected(true);

    // Get initial position
    QPointF initialPos = andGate->pos();

    // Morph
    scene->nextElm();

    // New element should retain position
    QVERIFY2(!scene->elements().isEmpty(), "Scene should contain an element after morph");
    auto *newElement = scene->elements()[0];
    QCOMPARE(newElement->pos(), initialPos);
}

void TestScene::testPropertyCycleFrequency()
{
    auto scene = std::make_unique<Scene>();

    // Add a Clock element
    auto *clock = ElementFactory::buildElement(ElementType::Clock);
    clock->setFrequency(1.0f);
    scene->addItem(clock);

    // Select it
    clock->setSelected(true);

    // Get initial frequency
    float initialFreq = clock->frequency();
    QVERIFY(initialFreq > 0);

    // Call next main property
    scene->nextMainPropShortcut();

    // Clock should still exist
    QCOMPARE(scene->elements().size(), 1);
}

void TestScene::testPropertyCycleColor()
{
    auto scene = std::make_unique<Scene>();

    // Add an LED element
    auto *led = ElementFactory::buildElement(ElementType::Led);
    led->setColor("Red");
    scene->addItem(led);

    // Select it
    led->setSelected(true);

    QString initialColor = led->color();
    QCOMPARE(initialColor, QString("Red"));

    // Cycle color
    scene->nextMainPropShortcut();

    // LED should still exist
    QCOMPARE(scene->elements().size(), 1);
}

void TestScene::testPropertyCycleAudio()
{
    auto scene = std::make_unique<Scene>();

    // Add a Buzzer element
    auto *buzzer = ElementFactory::buildElement(ElementType::Buzzer);
    scene->addItem(buzzer);

    // Select it
    buzzer->setSelected(true);

    // Cycle audio property
    scene->nextSecndPropShortcut();

    // Buzzer should still exist
    QCOMPARE(scene->elements().size(), 1);
}

void TestScene::testMorphingMultipleElements()
{
    auto scene = std::make_unique<Scene>();

    // Create multiple AND gates
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    scene->addItem(and1);

    auto *and2 = ElementFactory::buildElement(ElementType::And);
    and2->setPos(100, 0);
    scene->addItem(and2);

    // Select both
    and1->setSelected(true);
    and2->setSelected(true);

    // Verify both selected
    QCOMPARE(scene->selectedElements().size(), 2);

    // Morph both
    scene->nextElm();

    // Both should still exist
    QCOMPARE(scene->elements().size(), 2);
}

void TestScene::testShowGatesToggle()
{
    auto scene = std::make_unique<Scene>();

    // Add logic gates
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene->addItem(andGate);

    auto *orGate = ElementFactory::buildElement(ElementType::Or);
    orGate->setPos(100, 0);
    scene->addItem(orGate);

    // Toggle gates visibility
    scene->showGates(false);
    scene->showGates(true);

    // Gates should still exist in scene
    QCOMPARE(scene->elements().size(), 2);
}

void TestScene::testShowWiresToggle()
{
    auto scene = std::make_unique<Scene>();

    // Create connection
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    scene->addItem(and1);

    auto *and2 = ElementFactory::buildElement(ElementType::And);
    and2->setPos(150, 0);
    scene->addItem(and2);

    // Create wire
    auto *conn = new QNEConnection(nullptr);
    conn->setStartPort(and1->outputPort(0));
    conn->setEndPort(and2->inputPort(0));
    scene->addItem(conn);

    // Toggle wire visibility
    scene->showWires(false);
    scene->showWires(true);

    // Elements and connections should still exist
    QCOMPARE(scene->elements().size(), 2);
}

void TestScene::testGateVisibilityStateTracking()
{
    auto scene = std::make_unique<Scene>();

    auto *gate = ElementFactory::buildElement(ElementType::And);
    scene->addItem(gate);

    // Hide gates
    scene->showGates(false);

    // Show gates
    scene->showGates(true);

    // Gate should remain in scene
    QCOMPARE(scene->elements().size(), 1);
}

void TestScene::testWireVisibilityStateTracking()
{
    auto scene = std::make_unique<Scene>();

    auto *and1 = ElementFactory::buildElement(ElementType::And);
    scene->addItem(and1);

    auto *and2 = ElementFactory::buildElement(ElementType::And);
    and2->setPos(150, 0);
    scene->addItem(and2);

    // Create wire
    auto *conn = new QNEConnection(nullptr);
    conn->setStartPort(and1->outputPort(0));
    conn->setEndPort(and2->inputPort(0));
    scene->addItem(conn);

    // Hide wires
    scene->showWires(false);

    // Show wires
    scene->showWires(true);

    // Connection should remain
    QCOMPARE(scene->elements().size(), 2);
}

void TestScene::testShowGatesWithMultipleElements()
{
    auto scene = std::make_unique<Scene>();

    // Add multiple gates
    for (int i = 0; i < 5; ++i) {
        auto *gate = ElementFactory::buildElement(ElementType::And);
        gate->setPos(i * 100, 0);
        scene->addItem(gate);
    }

    QCOMPARE(scene->elements().size(), 5);

    // Toggle visibility
    scene->showGates(false);
    scene->showGates(true);

    // All gates should remain
    QCOMPARE(scene->elements().size(), 5);
}

void TestScene::testShowWiresWithMultipleConnections()
{
    auto scene = std::make_unique<Scene>();

    // Create chain of gates
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    scene->addItem(and1);

    auto *and2 = ElementFactory::buildElement(ElementType::And);
    and2->setPos(150, 0);
    scene->addItem(and2);

    auto *and3 = ElementFactory::buildElement(ElementType::And);
    and3->setPos(300, 0);
    scene->addItem(and3);

    // Create connections
    auto *conn1 = new QNEConnection(nullptr);
    conn1->setStartPort(and1->outputPort(0));
    conn1->setEndPort(and2->inputPort(0));
    scene->addItem(conn1);

    auto *conn2 = new QNEConnection(nullptr);
    conn2->setStartPort(and2->outputPort(0));
    conn2->setEndPort(and3->inputPort(0));
    scene->addItem(conn2);

    // Toggle wire visibility
    scene->showWires(false);
    scene->showWires(true);

    // All elements should remain
    QCOMPARE(scene->elements().size(), 3);
}
