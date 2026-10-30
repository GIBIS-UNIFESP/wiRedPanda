// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Scene/TestScene.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <tuple>

#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QGraphicsProxyWidget>
#include <QGraphicsRectItem>
#include <QGraphicsSceneEvent>
#include <QImage>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPaintDevice>
#include <QPaintEngine>
#include <QPainter>
#include <QScopeGuard>
#include <QScrollBar>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QToolTip>
#include <QTransform>
#include <QUrl>

#include "App/Core/Application.h"
#include "App/Core/Constants.h"
#include "App/Core/ItemWithId.h"
#include "App/Core/MimeTypes.h"
#include "App/Core/ThemeManager.h"
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/AudioOutputElement.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/Element/GraphicElements/InputButton.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/Scene/ClipboardManager.h"
#include "App/Scene/Commands.h"
#include "App/Scene/ConnectionManager.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/InlineLabelEditor.h"
#include "App/Scene/PortHoverLabel.h"
#include "App/Scene/Scene.h"
#include "App/Scene/SceneDropHandler.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/Goldens.h"
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

    auto and1 = std::unique_ptr<Element>(ElementFactory::buildElement(ElementType::And));
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    auto *or1 = ElementFactory::buildElement(ElementType::Or);

    QVERIFY2(and1 != nullptr && and2 != nullptr && or1 != nullptr,
        "Failed to create element(s) for testRemoveElement");

    scene->addItem(and1.get());
    scene->addItem(and2);
    scene->addItem(or1);

    QCOMPARE(scene->elements().size(), 3);

    // Remove one element
    scene->removeItem(and1.get());
    and1.reset();

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
        if (auto *conn = qgraphicsitem_cast<Connection *>(item)) {
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
    QVERIFY(!items.isEmpty());
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
    QVERIFY(!items.isEmpty());
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
    auto *conn1 = new Connection();
    conn1->setStartPort(sw->outputPort());
    conn1->setEndPort(andGate->inputPort(0));
    scene->addItem(conn1);

    auto *conn2 = new Connection();
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
        auto *conn = qgraphicsitem_cast<Connection *>(elem);
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

    int elementCountBefore = static_cast<int>(scene->elements().size());
    QVERIFY(!andGate->isFlippedX());

    // Flip horizontally - should complete without errors
    scene->flipHorizontally();

    // Verify element still exists (not deleted), and was actually flipped.
    int elementCountAfter = static_cast<int>(scene->elements().size());
    QCOMPARE(elementCountAfter, elementCountBefore);
    QVERIFY(andGate->isFlippedX());
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

    int elementCountBefore = static_cast<int>(scene->elements().size());
    QVERIFY(!andGate->isFlippedY());

    // Flip vertically - should complete without errors
    scene->flipVertically();

    // Verify element still exists (not deleted), and was actually flipped.
    int elementCountAfter = static_cast<int>(scene->elements().size());
    QCOMPARE(elementCountAfter, elementCountBefore);
    QVERIFY(andGate->isFlippedY());
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

    // Real connections into/out of the element being rotated -- the point of this test is that
    // rotation doesn't sever or corrupt them, which a connection-free scene can't demonstrate.
    auto *conn1 = new Connection();
    conn1->setStartPort(inputSwitch->outputPort());
    conn1->setEndPort(andGate->inputPort(0));
    scene->addItem(conn1);

    auto *conn2 = new Connection();
    conn2->setStartPort(andGate->outputPort());
    conn2->setEndPort(led->inputPort());
    scene->addItem(conn2);

    int connectionsBeforeRotate = TestUtils::countConnections(scene);
    QCOMPARE(connectionsBeforeRotate, 2);

    // Select and rotate
    andGate->setSelected(true);
    scene->rotateRight();

    // Verify rotation succeeded
    QCOMPARE(andGate->rotation(), 90.0);

    // Connection count should remain the same, and each connection must still have both its
    // real ports -- rotation must not sever or corrupt them.
    int connectionsAfterRotate = TestUtils::countConnections(scene);
    QCOMPARE(connectionsAfterRotate, connectionsBeforeRotate);
    QCOMPARE(conn1->startPort(), inputSwitch->outputPort());
    QCOMPARE(conn1->endPort(), andGate->inputPort(0));
    QCOMPARE(conn2->startPort(), andGate->outputPort());
    QCOMPARE(conn2->endPort(), led->inputPort());
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

    // Wire inputSwitch -> andGate -> led so deleting andGate has real connections to clean up.
    auto *conn1 = new Connection(nullptr);
    conn1->setStartPort(inputSwitch->outputPort(0));
    conn1->setEndPort(andGate->inputPort(0));
    scene->addItem(conn1);

    auto *conn2 = new Connection(nullptr);
    conn2->setStartPort(andGate->outputPort(0));
    conn2->setEndPort(led->inputPort(0));
    scene->addItem(conn2);

    int initialConnections = TestUtils::countConnections(scene);
    QCOMPARE(initialConnections, 2);

    // Select and delete AND gate
    andGate->setSelected(true);
    scene->deleteAction();

    // Verify element removed
    QCOMPARE(scene->elements().size(), 2);

    // Both connections touched the deleted AND gate, so both must be gone -- not just "no more
    // than before" (which a leaked/still-dangling connection would also satisfy).
    int finalConnections = TestUtils::countConnections(scene);
    QCOMPARE(finalConnections, 0);
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
    auto *conn = new Connection(nullptr);
    conn->setStartPort(andGate->outputPort(0));
    conn->setEndPort(led->inputPort(0));
    scene->addItem(conn);

    const int andId = andGate->id();

    // Select AND gate
    andGate->setSelected(true);

    // Morph to next type
    scene->nextElm();

    // Scene::elements() structurally excludes Connections, so it can't reflect wire survival --
    // check the connection count and its actual endpoints. MorphCommand replaces andGate with a
    // new Element instance (the old pointer is stale), so re-resolve it by id.
    QCOMPARE(scene->elements().size(), 2);
    QCOMPARE(TestUtils::countConnections(scene.get()), 1);

    auto *morphedElm = dynamic_cast<Element *>(scene->itemById(andId));
    QVERIFY(morphedElm);
    QVERIFY(morphedElm->elementType() != ElementType::And);
    QCOMPARE(conn->startPort()->element(), morphedElm);
    QCOMPARE(conn->endPort()->element(), led);
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
    clock->setFrequency(1.0);
    scene->addItem(clock);

    // Select it
    clock->setSelected(true);

    // Get initial frequency
    double initialFreq = clock->frequency();
    QVERIFY(initialFreq > 0);

    // Call next main property
    scene->nextMainPropShortcut();

    // Clock should still exist
    QCOMPARE(scene->elements().size(), 1);

    // adjustMainProperty() cycles Clock::frequency() in 0.5 Hz steps.
    QCOMPARE(clock->frequency(), initialFreq + 0.5);
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
    const QString expectedColor = led->nextColor();

    // Cycle color -- Led's color is cycled by adjustSecondaryProperty() (nextSecndProp), not
    // adjustMainProperty(): the "main" property shortcut instead changes Led's *input size*
    // (Led shares that case with And/Or/Nand/.../TruthTable in PropertyShortcutHandler.cpp).
    scene->nextSecndPropShortcut();

    // LED should still exist
    QCOMPARE(scene->elements().size(), 1);
    QCOMPARE(led->color(), expectedColor);
}

void TestScene::testPropertyCycleAudio()
{
    auto scene = std::make_unique<Scene>();

    // Add a Buzzer element
    auto *buzzer = ElementFactory::buildElement(ElementType::Buzzer);
    scene->addItem(buzzer);

    // Select it
    buzzer->setSelected(true);

    auto *audioElm = dynamic_cast<AudioOutputElement *>(buzzer);
    QVERIFY(audioElm);
    const float initialVolume = audioElm->volume();
    const bool initialMuted = audioElm->isMuted();

    // Cycle audio property -- PropertyShortcutHandler::adjustSecondaryProperty() has no case
    // for ElementType::Buzzer (it falls into the shared no-op group with And/AudioBox/Clock/
    // etc.); volume/mute have no keyboard-shortcut cycling at all. Confirm that stays true
    // (a regression that added incorrect handling here would change these).
    scene->nextSecndPropShortcut();

    // Buzzer should still exist, unaffected.
    QCOMPARE(scene->elements().size(), 1);
    QCOMPARE(audioElm->volume(), initialVolume);
    QCOMPARE(audioElm->isMuted(), initialMuted);
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
    QVERIFY(!andGate->isVisible());
    QVERIFY(!orGate->isVisible());

    scene->showGates(true);
    QVERIFY(andGate->isVisible());
    QVERIFY(orGate->isVisible());

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
    auto *conn = new Connection(nullptr);
    conn->setStartPort(and1->outputPort(0));
    conn->setEndPort(and2->inputPort(0));
    scene->addItem(conn);

    // Toggle wire visibility
    scene->showWires(false);
    QVERIFY(!conn->isVisible());

    scene->showWires(true);
    QVERIFY(conn->isVisible());

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
    QVERIFY(!gate->isVisible());

    // Show gates
    scene->showGates(true);
    QVERIFY(gate->isVisible());

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
    auto *conn = new Connection(nullptr);
    conn->setStartPort(and1->outputPort(0));
    conn->setEndPort(and2->inputPort(0));
    scene->addItem(conn);

    // Hide wires
    scene->showWires(false);
    QVERIFY(!conn->isVisible());

    // Show wires
    scene->showWires(true);
    QVERIFY(conn->isVisible());

    // Connection should remain
    QCOMPARE(scene->elements().size(), 2);
}

void TestScene::testShowGatesWithMultipleElements()
{
    auto scene = std::make_unique<Scene>();

    // Add multiple gates
    QVector<Element *> gates;
    for (int i = 0; i < 5; ++i) {
        auto *gate = ElementFactory::buildElement(ElementType::And);
        gate->setPos(i * 100, 0);
        scene->addItem(gate);
        gates.append(gate);
    }

    QCOMPARE(scene->elements().size(), 5);

    // Toggle visibility
    scene->showGates(false);
    for (auto *gate : gates) {
        QVERIFY(!gate->isVisible());
    }

    scene->showGates(true);
    for (auto *gate : gates) {
        QVERIFY(gate->isVisible());
    }

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
    auto *conn1 = new Connection(nullptr);
    conn1->setStartPort(and1->outputPort(0));
    conn1->setEndPort(and2->inputPort(0));
    scene->addItem(conn1);

    auto *conn2 = new Connection(nullptr);
    conn2->setStartPort(and2->outputPort(0));
    conn2->setEndPort(and3->inputPort(0));
    scene->addItem(conn2);

    // Toggle wire visibility
    scene->showWires(false);
    QVERIFY(!conn1->isVisible());
    QVERIFY(!conn2->isVisible());

    scene->showWires(true);
    QVERIFY(conn1->isVisible());
    QVERIFY(conn2->isVisible());

    // All elements should remain
    QCOMPARE(scene->elements().size(), 3);
}

void TestScene::testShowWiresTogglesNodeVisibility()
{
    // Node elements are pure wire-routing helpers with no logical function; showWires()
    // must hide/show the Node itself (not just its ports, like other element types).
    auto scene = std::make_unique<Scene>();

    auto *node = ElementFactory::buildElement(ElementType::Node);
    scene->addItem(node);
    QVERIFY(node->isVisible());

    scene->showWires(false);
    QVERIFY(!node->isVisible());

    scene->showWires(true);
    QVERIFY(node->isVisible());
}

void TestScene::testLastIdTracksHighestAssignedId()
{
    auto scene = std::make_unique<Scene>();
    const int before = scene->lastId();

    auto *elm = ElementFactory::buildElement(ElementType::And);
    scene->addItem(elm); // an unassigned (id < 0) item is given the next sequential id

    QCOMPARE(elm->id(), before + 1);
    QCOMPARE(scene->lastId(), before + 1);
}

void TestScene::testInlineLabelEditorStartWithNullElementIsNoOp()
{
    WorkSpace workspace;
    InlineLabelEditor editor(workspace.scene());
    const auto itemCountBefore = workspace.scene()->items().size();

    editor.start(nullptr);

    QCOMPARE(workspace.scene()->items().size(), itemCountBefore);
}

void TestScene::testInlineLabelEditorStartWhileEditingCommitsPrevious()
{
    WorkSpace workspace;
    auto *elmA = new And;
    elmA->setLabel("Original A");
    workspace.scene()->addItem(elmA);
    auto *elmB = new And;
    elmB->setLabel("Original B");
    elmB->setPos(200, 0);
    workspace.scene()->addItem(elmB);

    InlineLabelEditor editor(workspace.scene());
    editor.start(elmA);

    QLineEdit *lineEdit = nullptr;
    for (auto *item : workspace.scene()->items()) {
        if (auto *proxy = qgraphicsitem_cast<QGraphicsProxyWidget *>(item)) {
            lineEdit = qobject_cast<QLineEdit *>(proxy->widget());
        }
    }
    QVERIFY(lineEdit != nullptr);
    lineEdit->setText("Renamed A");

    // Starting a second edit before committing the first must commit it first.
    editor.start(elmB);

    QCOMPARE(elmA->label(), QString("Renamed A"));
}

void TestScene::testDuplicateTxLabel_data()
{
    QTest::addColumn<QStringList>("edits"); ///< "<node>:<label>:<mode>", the nodes of testDuplicateTxLabel().
    QTest::addColumn<QString>("expected");

    QTest::newRow("a node keeps no channel on a Tx's label") << QStringList{"d:a:None"} << QString();
    QTest::newRow("a receiver takes a Tx's channel") << QStringList{"d:a:Rx"} << QString();
    QTest::newRow("a node becomes a Tx on a Tx's label") << QStringList{"d:a:Tx"} << QString("a");
    QTest::newRow("a Tx takes another Tx's label") << QStringList{"a:dup:Tx"} << QString("dup");
    QTest::newRow("a Tx keeps its own label") << QStringList{"a:a:Tx"} << QString();
    QTest::newRow("two edited nodes become one channel") << QStringList{"d:z:Tx", "e:z:Tx"} << QString("z");
    QTest::newRow("two edited Tx nodes take one new label") << QStringList{"a:n:Tx", "b:n:Tx"} << QString("n");
    QTest::newRow("Tx nodes already sharing a label refuse no other edit") << QStringList{"e:y:Tx"} << QString();
    QTest::newRow("an empty label is no channel") << QStringList{"d::Tx", "e::Tx"} << QString();
    QTest::newRow("a Tx leaving its channel frees it") << QStringList{"a:a:None", "d:a:Tx"} << QString();
}

void TestScene::testDuplicateTxLabel()
{
    QFETCH(QStringList, edits);
    QFETCH(QString, expected);

    // a: a Tx on "a"; b and c: Tx nodes already sharing "dup"; d: a plain node labelled "a"; e: an unlabelled one
    WorkSpace workspace;
    auto *scene = workspace.scene();
    const QList<std::tuple<QString, QString, WirelessMode>> nodes{
        {"a", "a", WirelessMode::Tx}, {"b", "dup", WirelessMode::Tx}, {"c", "dup", WirelessMode::Tx},
        {"d", "a", WirelessMode::None}, {"e", "", WirelessMode::None},
    };
    QHash<QString, Node *> byName;
    for (const auto &[name, label, mode] : nodes) {
        auto *node = new Node;
        node->setWirelessMode(mode);
        node->setLabel(label);
        scene->addItem(node);
        byName.insert(name, node);
    }

    const QHash<QString, WirelessMode> modes{
        {"None", WirelessMode::None}, {"Tx", WirelessMode::Tx}, {"Rx", WirelessMode::Rx}};
    QList<Scene::WirelessEdit> wirelessEdits;
    for (const QString &edit : std::as_const(edits)) {
        const QStringList parts = edit.split(':');
        wirelessEdits.append({byName.value(parts.at(0)), parts.at(1), modes.value(parts.at(2))});
    }

    QCOMPARE(scene->duplicateTxLabel(wirelessEdits), expected);
}

void TestScene::testInlineLabelEditorRefusesADuplicateTxLabel()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *first = new Node;
    first->setWirelessMode(WirelessMode::Tx);
    first->setLabel("x");
    scene->addItem(first);
    auto *second = new Node;
    second->setWirelessMode(WirelessMode::Tx);
    second->setLabel("y");
    second->setPos(100, 0);
    scene->addItem(second);
    const int undoCount = scene->undoStack()->count();
    QSignalSpy messages(scene, &Scene::showStatusMessageRequested);

    InlineLabelEditor editor(scene);
    editor.start(second);
    QLineEdit *lineEdit = nullptr;
    for (auto *item : scene->QGraphicsScene::items()) {
        if (auto *proxy = qgraphicsitem_cast<QGraphicsProxyWidget *>(item)) {
            lineEdit = qobject_cast<QLineEdit *>(proxy->widget());
        }
    }
    QVERIFY(lineEdit != nullptr);
    lineEdit->setText("x");
    editor.commit();

    // The label stays, nothing is pushed, and the status bar says why
    QCOMPARE(second->label(), QString("y"));
    QCOMPARE(scene->undoStack()->count(), undoCount);
    QCOMPARE(messages.count(), 1);
    QVERIFY(messages.constFirst().constFirst().toString().contains("\"x\""));
}

void TestScene::testInlineLabelEditorReportsAPushThatDrops()
{
    // Return in the inline editor is a slot Qt calls, which pushes the label's change: a file-backed IC whose file
    // changed to bytes that do not read, its cache dropped, drops the push, reported there and thrown nowhere
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("sub.panda");
    {
        WorkSpace sub;
        sub.scene()->addItem(new InputSwitch());
        sub.scene()->addItem(new Led());
        sub.save(path);
    }
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *ic = new IC();
    scene->addItem(ic);
    ic->loadFile(path, dir.path());
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write("not a circuit");
    file.close();
    scene->icRegistry()->invalidate(path);
    const int undoCount = scene->undoStack()->count();

    InlineLabelEditor editor(scene);
    editor.start(ic);
    QLineEdit *lineEdit = nullptr;
    for (auto *item : scene->QGraphicsScene::items()) {
        if (auto *proxy = qgraphicsitem_cast<QGraphicsProxyWidget *>(item)) {
            lineEdit = qobject_cast<QLineEdit *>(proxy->widget());
        }
    }
    QVERIFY(lineEdit != nullptr);
    lineEdit->setText("renamed");
    // Emitted, not typed: a key event goes through notify(), whose last-resort catch would hide a throw
    try {
        emit lineEdit->returnPressed();
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("the label's edit threw: %1").arg(e.what())));
    }
    QCOMPARE(scene->undoStack()->count(), undoCount);
    QVERIFY(scene->elements().contains(ic));
}

namespace TestSceneTxHelpers {

Node *addWirelessNode(Scene *scene, const QString &label, const WirelessMode mode, const QPointF pos)
{
    auto *node = new Node;
    node->setWirelessMode(mode);
    node->setLabel(label);
    node->setPos(pos);
    scene->addItem(node);
    return node;
}

} // namespace TestSceneTxHelpers

void TestScene::testDuplicateTxLabelsNamesWhatArrived()
{
    using TestSceneTxHelpers::addWirelessNode;
    WorkSpace workspace;
    auto *scene = workspace.scene();
    addWirelessNode(scene, "b", WirelessMode::Tx, {0, 0});
    addWirelessNode(scene, "a", WirelessMode::Tx, {64, 0});
    addWirelessNode(scene, "c", WirelessMode::Tx, {128, 0});
    addWirelessNode(scene, "d", WirelessMode::Rx, {192, 0});
    auto *secondB = addWirelessNode(scene, "b", WirelessMode::Tx, {0, 64});
    auto *secondA = addWirelessNode(scene, "a", WirelessMode::Tx, {64, 64});
    auto *rxC = addWirelessNode(scene, "c", WirelessMode::Rx, {128, 64});
    auto *txD = addWirelessNode(scene, "d", WirelessMode::Tx, {192, 64});
    auto *unlabelled = addWirelessNode(scene, "", WirelessMode::Tx, {256, 64});
    addWirelessNode(scene, "", WirelessMode::Tx, {256, 0});

    // Only what arrived is named, sorted; an Rx shares no channel and an unlabelled Tx has none
    QCOMPARE(scene->duplicateTxLabels({secondB, secondA, rxC, txD, unlabelled}), QStringList({"a", "b"}));
    QCOMPARE(scene->duplicateTxLabels({secondB}), QStringList({"b"}));
    QCOMPARE(scene->duplicateTxLabels({rxC, txD, unlabelled}), QStringList());
    QCOMPARE(scene->duplicateTxLabels({}), QStringList());

    QCOMPARE(Scene::duplicateTxLabelsMessage({}), QString());
    QVERIFY(Scene::duplicateTxLabelsMessage({"b"}).contains("\"b\""));
    QVERIFY(Scene::duplicateTxLabelsMessage({"a", "b"}).contains("\"a\", \"b\""));
}

void TestScene::testPasteReportsDuplicateTxLabels()
{
    using TestSceneTxHelpers::addWirelessNode;
    WorkSpace source;
    addWirelessNode(source.scene(), "a", WirelessMode::Tx, {0, 0})->setSelected(true);
    addWirelessNode(source.scene(), "b", WirelessMode::Tx, {64, 0})->setSelected(true);
    source.scene()->clipboardManager()->copy();

    // Into a scene with neither channel, nothing is said
    WorkSpace empty;
    QSignalSpy emptyMessages(empty.scene(), &Scene::showStatusMessageRequested);
    empty.scene()->clipboardManager()->paste();
    QCOMPARE(empty.scene()->elements().size(), 2);
    QCOMPARE(emptyMessages.count(), 0);

    // Into one holding "b", the paste goes on and names it
    WorkSpace workspace;
    auto *scene = workspace.scene();
    addWirelessNode(scene, "b", WirelessMode::Tx, {0, 0});
    QSignalSpy messages(scene, &Scene::showStatusMessageRequested);
    scene->clipboardManager()->paste();
    QCOMPARE(scene->elements().size(), 3);
    QCOMPARE(messages.count(), 1);
    const QString message = messages.constFirst().constFirst().toString();
    QVERIFY2(message.contains("\"b\"") && !message.contains("\"a\""), qPrintable(message));
}

void TestScene::testDuplicateReportsDuplicateTxLabels()
{
    using TestSceneTxHelpers::addWirelessNode;
    WorkSpace workspace;
    auto *scene = workspace.scene();
    addWirelessNode(scene, "a", WirelessMode::Tx, {0, 0})->setSelected(true);
    QSignalSpy messages(scene, &Scene::showStatusMessageRequested);

    scene->duplicateAction();

    QCOMPARE(scene->elements().size(), 2);
    QCOMPARE(messages.count(), 1);
    QVERIFY(messages.constFirst().constFirst().toString().contains("\"a\""));
}

void TestScene::testCloneDragReportsDuplicateTxLabels()
{
    using TestSceneTxHelpers::addWirelessNode;
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *node = addWirelessNode(scene, "a", WirelessMode::Tx, {48, 48});

    QByteArray itemData;
    QDataStream writeStream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(writeStream);
    writeStream << node->pos();
    writeStream << node->pos();
    Serialization::serialize({node}, writeStream, {.purpose = SerializationPurpose::InMemorySnapshot});
    QMimeData mimeData;
    mimeData.setData(MimeType::CloneDrag, itemData);
    QSignalSpy messages(scene, &Scene::showStatusMessageRequested);

    QGraphicsSceneDragDropEvent event(QEvent::GraphicsSceneDrop);
    event.setMimeData(&mimeData);
    event.setScenePos(node->pos() + QPointF(96, 0));
    scene->dropEvent(&event);

    QCOMPARE(scene->elements().size(), 2);
    QCOMPARE(messages.count(), 1);
    QVERIFY(messages.constFirst().constFirst().toString().contains("\"a\""));
}

void TestScene::testInlineLabelEditorEmptyLabelUsesElementBoundingRectFallback()
{
    // labelSceneBoundingRect() is empty when the label has no text yet; start() must fall
    // back to the element's own sceneBoundingRect() to position the editor.
    WorkSpace workspace;
    auto *elm = new And;
    elm->setPos(100, 50);
    workspace.scene()->addItem(elm);
    QVERIFY(elm->label().isEmpty());
    QVERIFY(elm->labelSceneBoundingRect().isEmpty());

    InlineLabelEditor editor(workspace.scene());
    editor.start(elm);

    QGraphicsProxyWidget *proxy = nullptr;
    for (auto *item : workspace.scene()->items()) {
        if (auto *p = qgraphicsitem_cast<QGraphicsProxyWidget *>(item)) {
            proxy = p;
        }
    }
    QVERIFY(proxy != nullptr);
    QCOMPARE(proxy->pos(), elm->sceneBoundingRect().topLeft());
}

void TestScene::testCutWithNoSelectionClearsClipboard()
{
    QApplication::clipboard()->setText("something");
    QVERIFY(!QApplication::clipboard()->text().isEmpty());

    auto scene = std::make_unique<Scene>();
    scene->clearSelection();

    scene->clipboardManager()->cut();

    QVERIFY(QApplication::clipboard()->text().isEmpty());
}

void TestScene::testCloneDragWithNoSelectionIsNoOp()
{
    auto scene = std::make_unique<Scene>();
    auto *elm = ElementFactory::buildElement(ElementType::And);
    scene->addItem(elm);
    scene->clearSelection();

    scene->clipboardManager()->cloneDrag(QPointF(0, 0));

    QCOMPARE(scene->elements().size(), 1); // untouched
}

void TestScene::testDuplicateWithNoSelectionIsNoOp()
{
    auto scene = std::make_unique<Scene>();
    auto *elm = ElementFactory::buildElement(ElementType::And);
    scene->addItem(elm);
    scene->clearSelection();

    scene->clipboardManager()->duplicate();

    QCOMPARE(scene->elements().size(), 1);
}

namespace TestSceneSelectionHelpers {

/// Fifty And gates, in ten columns of five with every gate of a column at the same x, each labelled with its place in
/// id order, every one selected: Qt keeps a selection of fifty in a hash set, out of id order in every run.
QList<Element *> addSelectedColumns(Scene *scene)
{
    QList<Element *> elements;
    for (int column = 0; column < 10; ++column) {
        for (int row = 0; row < 5; ++row) {
            auto *element = ElementFactory::buildElement(ElementType::And);
            element->setPos(column * 160, row * 80);
            element->setLabel("e" + QString::number(elements.size()));
            scene->addItem(element);
            element->setSelected(true);
            elements << element;
        }
    }
    return elements;
}

/// The ids of \a elements, in their order.
QList<int> idsOf(const QList<Element *> &elements)
{
    QList<int> ids;
    for (const auto *element : elements) {
        ids << element->id();
    }
    return ids;
}

/// \a ids sorted.
QList<int> sortedIds(QList<int> ids)
{
    std::sort(ids.begin(), ids.end());
    return ids;
}

} // namespace TestSceneSelectionHelpers

void TestScene::testSelectionAccessorsAnswerInIdOrder()
{
    using namespace TestSceneSelectionHelpers;
    auto scene = std::make_unique<Scene>();
    const auto elements = addSelectedColumns(scene.get());
    auto *wire = new Connection();
    wire->setStartPort(elements.at(1)->outputPort(0));
    wire->setEndPort(elements.at(0)->inputPort(0));
    scene->addItem(wire);
    wire->setSelected(true);

    const QList<int> ids = idsOf(scene->selectedElements());
    QCOMPARE(ids.size(), 50);
    QCOMPARE(ids, sortedIds(ids));

    QList<int> itemIds;
    for (auto *item : scene->selectedItems()) {
        auto *withId = dynamic_cast<ItemWithId *>(item);
        QVERIFY(withId);
        itemIds << withId->id();
    }
    QCOMPARE(itemIds.size(), 51);
    QCOMPARE(itemIds, sortedIds(itemIds));
}

void TestScene::testDuplicateNumbersTheCopiesInIdOrder()
{
    // The copies take new ids in the order the selection is written, which is the originals' id order, so the copy
    // of each original sits at the same rank among the copies as the original among the originals.
    using namespace TestSceneSelectionHelpers;
    auto scene = std::make_unique<Scene>();
    const auto originals = addSelectedColumns(scene.get());

    scene->clipboardManager()->duplicate();

    QList<Element *> copies;
    for (auto *element : TestUtils::elementsById(scene.get())) {
        if (!originals.contains(element)) {
            copies << element;
        }
    }
    QCOMPARE(copies.size(), originals.size());
    for (int i = 0; i < copies.size(); ++i) {
        QCOMPARE(copies.at(i)->label(), originals.at(i)->label());
    }
}

void TestScene::testDeleteUndoRestoresInIdOrder()
{
    // The delete keeps the selection in id order, and its undo adds the elements back in that order, so they stack
    // in id order again.
    using namespace TestSceneSelectionHelpers;
    auto scene = std::make_unique<Scene>();
    addSelectedColumns(scene.get());

    scene->deleteAction();
    QCOMPARE(scene->elements().size(), 0);
    scene->undoStack()->undo();

    QList<int> stacked;
    for (auto *item : scene->items(Qt::AscendingOrder)) {
        if (item->type() == Element::Type) {
            stacked << static_cast<Element *>(item)->id();
        }
    }
    QCOMPARE(stacked.size(), 50);
    QCOMPARE(stacked, sortedIds(stacked));
}

void TestScene::testDistributeKeepsTiesInIdOrder()
{
    // Each column's five gates share a left edge; distributing them horizontally gives each tie its place in id order,
    // so within a column a lower id lands further left.
    using namespace TestSceneSelectionHelpers;
    auto scene = std::make_unique<Scene>();
    const auto elements = addSelectedColumns(scene.get());

    scene->distributeHorizontally();

    for (int column = 0; column < 10; ++column) {
        for (int row = 1; row < 5; ++row) {
            const auto *previous = elements.at((column * 5) + row - 1);
            const auto *element = elements.at((column * 5) + row);
            QVERIFY2(previous->pos().x() < element->pos().x(),
                     qPrintable(QString("%1 at x=%2 is not left of %3 at x=%4")
                                    .arg(previous->label())
                                    .arg(previous->pos().x())
                                    .arg(element->label())
                                    .arg(element->pos().x())));
        }
    }
}

namespace TestSceneStackingHelpers {

/// The scene's elements and wires in the order Qt stacks them, bottom first, as "E<id>" and "W<id>".
QStringList stacked(Scene *scene)
{
    QStringList names;
    for (auto *item : scene->QGraphicsScene::items(Qt::AscendingOrder)) {
        if (item->type() == Element::Type) {
            names << 'E' + QString::number(static_cast<Element *>(item)->id());
        } else if (item->type() == Connection::Type) {
            names << 'W' + QString::number(static_cast<Connection *>(item)->id());
        }
    }
    return names;
}

} // namespace TestSceneStackingHelpers

void TestScene::testUndoRestoresTheBottomElementAtTheBottom_data()
{
    QTest::addColumn<QString>("edit");
    QTest::newRow("delete") << "delete";
    QTest::newRow("cut") << "cut";
    QTest::newRow("morph") << "morph";
}

void TestScene::testUndoRestoresTheBottomElementAtTheBottom()
{
    // The bottom of three overlapping elements, deleted, cut or morphed and the edit undone, is back at the bottom,
    // and a morph's element takes its place too.
    QFETCH(QString, edit);
    using namespace TestSceneStackingHelpers;
    auto scene = std::make_unique<Scene>();
    QList<Element *> elements;
    int x = 24;
    for (const auto type : {ElementType::And, ElementType::Or, ElementType::Led}) {
        auto *element = ElementFactory::buildElement(type);
        element->setPos(x, 24);
        x += 16;
        scene->addItem(element);
        elements << element;
    }
    const QStringList before = stacked(scene.get());
    auto *bottom = elements.constFirst();
    scene->clearSelection();
    bottom->setSelected(true);

    if (edit == "delete") {
        scene->deleteAction();
    } else if (edit == "cut") {
        scene->clipboardManager()->cut();
    } else {
        scene->receiveCommand(new MorphCommand({bottom}, ElementType::Xor, scene.get()));
        QCOMPARE(stacked(scene.get()), before);
    }
    QCOMPARE(scene->undoStack()->count(), 1);

    scene->undoStack()->undo();
    QCOMPARE(stacked(scene.get()), before);
}

void TestScene::testDeleteUndoKeepsTheStackingOrder()
{
    // A thousand gates in a chain, every third deleted with its wires and the delete undone: every element and wire is
    // back at its place among its own kind, each restored one below the ones above it that stayed.
    using namespace TestSceneStackingHelpers;
    auto scene = std::make_unique<Scene>();
    QList<Element *> gates;
    for (int i = 0; i < 1000; ++i) {
        auto *gate = ElementFactory::buildElement(ElementType::And);
        gate->setPos((i % 40) * 80, (i / 40) * 80);
        scene->addItem(gate);
        if (!gates.isEmpty()) {
            auto *wire = new Connection();
            wire->setStartPort(gates.constLast()->outputPort(0));
            wire->setEndPort(gate->inputPort(0));
            scene->addItem(wire);
        }
        gates << gate;
    }
    const QStringList before = stacked(scene.get());
    QCOMPARE(before.size(), 1999);

    scene->clearSelection();
    for (int i = 0; i < gates.size(); i += 3) {
        gates.at(i)->setSelected(true);
    }
    scene->deleteAction();
    QCOMPARE(scene->elements().size(), 666);

    scene->undoStack()->undo();
    QCOMPARE(stacked(scene.get()), before);
}

void TestScene::testPasteLegacyUnversionedBlobRegistryFormat()
{
    // Older app versions wrote only the unversioned BlobRegistry format (no explicit
    // QDataStream version set); paste() must still parse it as a fallback when the newer
    // BlobRegistryV2 format isn't present. The blob value must be a real, parseable panda
    // file: ICRegistry::registerBlob() (invoked via RegisterBlobCommand::redo(), which
    // paste() triggers) validates it and would otherwise throw on garbage bytes.
    WorkSpace subWorkspace;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    subWorkspace.scene()->addItem(andGate);
    QByteArray subCircuitBytes;
    {
        QDataStream subStream(&subCircuitBytes, QIODevice::WriteOnly);
        Serialization::writePandaHeader(subStream);
        subWorkspace.save(subStream);
    }

    QMap<QString, QByteArray> blobs;
    blobs["legacy_blob"] = subCircuitBytes;

    QByteArray regBytes;
    QDataStream regStream(&regBytes, QIODevice::WriteOnly);
    regStream << blobs; // no explicit setVersion(), matching the legacy writer

    auto *mimeData = new QMimeData;
    mimeData->setData(MimeType::BlobRegistry, regBytes);
    QApplication::clipboard()->setMimeData(mimeData);

    auto scene = std::make_unique<Scene>();
    scene->clipboardManager()->paste();

    QVERIFY(scene->icRegistry()->hasBlob("legacy_blob"));
}

void TestScene::testPasteClipboardLegacyItemFormat()
{
    // Older app versions wrote item data under the ClipboardLegacy mime type; paste() must
    // still read it when the modern Clipboard format isn't present.
    WorkSpace sourceWorkspace;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    sourceWorkspace.scene()->addItem(andGate);
    andGate->setSelected(true);
    sourceWorkspace.scene()->clipboardManager()->copy();

    const QByteArray itemData = QApplication::clipboard()->mimeData()->data(MimeType::Clipboard);
    QVERIFY(!itemData.isEmpty());

    auto *mimeData = new QMimeData;
    mimeData->setData(MimeType::ClipboardLegacy, itemData); // re-tag as the legacy format only
    QApplication::clipboard()->setMimeData(mimeData);

    WorkSpace destWorkspace;
    QCOMPARE(destWorkspace.scene()->elements().size(), 0);

    destWorkspace.scene()->clipboardManager()->paste();

    QCOMPARE(destWorkspace.scene()->elements().size(), 1);
    QCOMPARE(destWorkspace.scene()->elements().first()->elementType(), ElementType::And);
}

namespace {

// Puts on the clipboard a versioned blob registry holding \a blob as "pasted_blob", and \a items as the item data
void setClipboardWithBlob(const QByteArray &blob, const QByteArray &items)
{
    QMap<QString, QByteArray> blobs;
    blobs["pasted_blob"] = blob;
    QByteArray regBytes;
    QDataStream regStream(&regBytes, QIODevice::WriteOnly);
    regStream.setVersion(QDataStream::Qt_5_12);
    regStream << blobs;

    auto *mimeData = new QMimeData;
    mimeData->setData(MimeType::BlobRegistryV2, regBytes);
    mimeData->setData(MimeType::Clipboard, items);
    QApplication::clipboard()->setMimeData(mimeData);
}

// Pastes into \a scene, which must throw, and checks the paste left the scene as it was: no element, no blob, and an
// undo stack that records the next edit as a step of its own instead of into a macro the paste left open
void checkFailedPasteChangesNothing(Scene *scene)
{
    QVERIFY_THROWS_EXCEPTION(Pandaception, scene->clipboardManager()->paste());

    QCOMPARE(scene->elements().size(), 0);
    QVERIFY(!scene->icRegistry()->hasBlob("pasted_blob"));
    QCOMPARE(scene->undoStack()->count(), 0);

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    scene->receiveCommand(new AddItemsCommand({andGate}, scene));
    QCOMPARE(scene->undoStack()->count(), 1);
    scene->undoStack()->undo();
    QCOMPARE(scene->elements().size(), 0);
}

} // namespace

void TestScene::testPasteOfUnreadableItemsChangesNothing()
{
    // A clipboard whose blob registers but whose items do not read: the blob was registered, in an undo macro, before
    // the items were read, so the throw left the blob registered and the macro open, every later edit recorded into it
    WorkSpace subWorkspace;
    subWorkspace.scene()->addItem(ElementFactory::buildElement(ElementType::And));
    QByteArray blob;
    {
        QDataStream subStream(&blob, QIODevice::WriteOnly);
        Serialization::writePandaHeader(subStream);
        subWorkspace.save(subStream);
    }

    // A header, the selection's centre, then an element's type tag with nothing after it
    QByteArray items;
    {
        QDataStream itemStream(&items, QIODevice::WriteOnly);
        Serialization::writePandaHeader(itemStream);
        itemStream << QPointF() << static_cast<int>(Element::Type);
    }

    setClipboardWithBlob(blob, items);
    WorkSpace workspace;
    checkFailedPasteChangesNothing(workspace.scene());
}

void TestScene::testNoItemMakesEveryMoveWalkTheScene()
{
    // Each time an item with children moves, Qt calls isAncestorOf() on every item in the scene that has
    // ItemSendsScenePositionChanges, so one such item per port would make moving k elements cost k times the scene's
    // port count. No item sets it: wires follow their element from the element's own ItemPositionHasChanged.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *notGate = ElementFactory::buildElement(ElementType::Not);
    scene->addItem(sw);
    scene->addItem(notGate);
    auto *conn = new Connection();
    conn->setStartPort(sw->outputPort());
    conn->setEndPort(notGate->inputPort());
    scene->addItem(conn);

    for (auto *item : scene->items()) {
        QVERIFY2(!(item->flags() & QGraphicsItem::ItemSendsScenePositionChanges),
                 qPrintable(QString("item of type %1 sends scene position changes").arg(item->type())));
    }
}

void TestScene::testPasteOfUnreadableBlobChangesNothing()
{
    // A clipboard whose items read but whose blob does not: registering it threw inside the undo stack's push, with
    // the macro already open
    WorkSpace sourceWorkspace;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    sourceWorkspace.scene()->addItem(andGate);
    andGate->setSelected(true);
    sourceWorkspace.scene()->clipboardManager()->copy();
    const QByteArray items = QApplication::clipboard()->mimeData()->data(MimeType::Clipboard);
    QVERIFY(!items.isEmpty());

    setClipboardWithBlob("not a circuit", items);
    WorkSpace workspace;
    checkFailedPasteChangesNothing(workspace.scene());
}

void TestScene::testUndoOfAPasteGrowsLinearly()
{
    // The undo of a paste removes every pasted item, which must stay linear in them. Pasted as the application pastes,
    // with rendering on and a shown view painting the paste before the undo, as a user's next action finds it: the
    // paste fits the scene rect, and the paint polishes and indexes the items. The undo is timed against the paste of
    // the same chain of wired gates: how that ratio grows from 250 elements to 16,000, 0.9 to 1.45 for a linear undo
    // with eight copies of the test running at once, and 10 to 15 for an undo that comes before Qt polishes and indexes
    // the paste. The sizes are far apart so that a loaded runner's noise stays well under that growth. Ratios of the
    // thread's CPU time (TestUtils::ThreadCpuTimer), each the best of nine runs. The window runs its event loop between
    // steps, as it would between a user's actions: a window that reads no message for five seconds is hung to Windows,
    // whose ghost of it then takes the desktop's foreground and refuses every other process a move of the cursor. Nor
    // does it take the foreground when shown, the desktop being the one the suite's other tests share.
    const TestUtils::ScopedRendering rendering;
    struct Times {
        qint64 paste = 0;
        qint64 undo = 0;
    };
    const auto bestTimes = [](const int n) {
        WorkSpace source;
        Element *previous = nullptr;
        for (int i = 0; i < n; ++i) {
            auto *notGate = ElementFactory::buildElement(ElementType::Not);
            notGate->setPos(64 * (i % 100), 64 * (i / 100));
            source.scene()->addItem(notGate);
            if (previous) {
                auto *conn = new Connection();
                conn->setStartPort(previous->outputPort());
                conn->setEndPort(notGate->inputPort());
                source.scene()->addItem(conn);
            }
            previous = notGate;
        }
        source.scene()->selectAll();
        source.scene()->clipboardManager()->copy();

        WorkSpace workspace;
        auto *scene = workspace.scene();
        workspace.setAttribute(Qt::WA_ShowWithoutActivating);
        workspace.show();
        if (!QTest::qWaitForWindowExposed(&workspace)) {
            return Times{};
        }
        Times best{std::numeric_limits<qint64>::max(), std::numeric_limits<qint64>::max()};
        for (int run = 0; run < 9; ++run) {
            TestUtils::ThreadCpuTimer timer;
            timer.start();
            scene->clipboardManager()->paste();
            best.paste = std::min(best.paste, timer.elapsed());
            if (scene->elements().size() != n) {
                return Times{};
            }
            QCoreApplication::processEvents();
            workspace.view()->viewport()->repaint();
            QCoreApplication::processEvents();
            timer.start();
            scene->undoStack()->undo();
            best.undo = std::min(best.undo, timer.elapsed());
            if (!scene->elements().isEmpty()) {
                return Times{};
            }
            QCoreApplication::processEvents();
        }
        return best;
    };

    // The large size first: the first paste a process times runs cold, on CI's runners up to 2.7 times as long
    // through every one of its nine runs, which at 250 elements read as the undo's growth. The large size's first
    // runs are slower too, and the best of nine leaves them out
    const Times large = bestTimes(16000);
    const Times small = bestTimes(250);
    QVERIFY2((small.paste > 0) && (large.paste > 0) && (small.undo > 0) && (large.undo > 0),
             "the paste or its undo lost or kept items");
    const double growth = (static_cast<double>(large.undo) / static_cast<double>(large.paste))
                        / (static_cast<double>(small.undo) / static_cast<double>(small.paste));
    // Reported on every run, so a runner's spread can be read from its logs, a pass's as well as a failure's
    const QString times = QString("paste %1 and undo %2 %5 at 250 elements, paste %3 and undo %4 %5 at 16,000")
                              .arg(small.paste).arg(small.undo).arg(large.paste).arg(large.undo)
                              .arg(TestUtils::ThreadCpuTimer::unit());
    qInfo().noquote() << QString("growth %1; %2").arg(growth).arg(times);
    QVERIFY2(growth < 5.0, qPrintable(QString("against the paste of the same items, its undo took %1 times as long "
                                              "at 64 times the elements (%2)").arg(growth).arg(times)));
}

// ============================================================================
// Topological Sort Tests (moved from TestCommon)
// ============================================================================

void TestScene::testSortSimpleChain()
{
    // Test: InputSwitch → AND → LED (simple chain)
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;
    Led led;

    builder.add(&sw, &andGate, &led);
    builder.connect(&sw, 0, &andGate, 0);
    builder.connect(&andGate, 0, &led, 0);

    // Get only our specific elements (don't rely on scene elements)
    QVector<Element *> elements{&sw, &andGate, &led};

    // Sort elements
    auto sorted = Scene::sortByTopology(elements);

    // Verify all elements are present
    QCOMPARE(sorted.size(), 3);

    // Verify ordering: LED (priority 1) should come before AND (priority 2) should come before SW (priority 3)
    // Priority is based on distance from outputs, higher priority = deeper in dependency chain
    int ledIndex = -1, andIndex = -1, swIndex = -1;
    for (int i = 0; i < sorted.size(); ++i) {
        if (sorted[i] == &led) {
            ledIndex = i;
        } else if (sorted[i] == &andGate) {
            andIndex = i;
        } else if (sorted[i] == &sw) {
            swIndex = i;
        }
    }

    QVERIFY2(ledIndex >= 0, "LED not found in sorted list");
    QVERIFY2(andIndex >= 0, "AND gate not found in sorted list");
    QVERIFY2(swIndex >= 0, "Switch not found in sorted list");

    // Priority-based sorting: higher priority (further from outputs) comes first
    // SW has priority 3 (highest), AND has priority 2, LED has priority 1 (lowest)
    // So sorted order should be: SW (index 0), AND (index 1), LED (index 2)
    QVERIFY2(swIndex < andIndex, "SW should come before AND");
    QVERIFY2(andIndex < ledIndex, "AND should come before LED");
}

void TestScene::testSortMultipleChains()
{
    // Test: Multiple independent chains
    // SW1 → AND → LED1
    // SW2 → OR → LED2
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    And andGate;
    Or orGate;
    Led led1, led2;

    builder.add(&sw1, &sw2, &andGate, &orGate, &led1, &led2);

    // First chain
    builder.connect(&sw1, 0, &andGate, 0);
    builder.connect(&andGate, 0, &led1, 0);

    // Second chain
    builder.connect(&sw2, 0, &orGate, 0);
    builder.connect(&orGate, 0, &led2, 0);

    // Use our specific elements
    QVector<Element *> elements{&sw1, &sw2, &andGate, &orGate, &led1, &led2};

    // Sort elements
    auto sorted = Scene::sortByTopology(elements);

    // Verify all elements are present
    QCOMPARE(sorted.size(), 6);

    // Both chains should be sorted independently
    // Count our specific elements
    int ledCount = 0;
    int andOrCount = 0;
    int switchCount = 0;

    for (auto *elem : std::as_const(sorted)) {
        if (elem == &led1 || elem == &led2) {
            ledCount++;
        } else if (elem == &andGate || elem == &orGate) {
            andOrCount++;
        } else if (elem == &sw1 || elem == &sw2) {
            switchCount++;
        }
    }

    QCOMPARE(ledCount, 2);
    QCOMPARE(andOrCount, 2);
    QCOMPARE(switchCount, 2);
}

void TestScene::testSortCycleDetection()
{
    // Test: Cycle handling (SW → AND → OR → back to AND input)
    // The cycle detection should prevent infinite recursion
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;
    Or orGate;

    builder.add(&sw, &andGate, &orGate);

    // Create connections: SW → AND → OR
    builder.connect(&sw, 0, &andGate, 0);
    builder.connect(&andGate, 0, &orGate, 0);

    // Create back edge: OR → AND (creates cycle)
    builder.connect(&orGate, 0, &andGate, 1);

    // Use our specific elements
    QVector<Element *> elements{&sw, &andGate, &orGate};

    // Sort should handle cycle without hanging
    auto sorted = Scene::sortByTopology(elements);

    // Verify all elements are present (no crash or infinite loop)
    QCOMPARE(sorted.size(), 3);

    // Verify each element appears exactly once (no duplicates due to cycle)
    int andCount = 0, orCount = 0, swCount = 0;
    for (auto *elem : std::as_const(sorted)) {
        if (elem == &andGate) andCount++;
        else if (elem == &orGate) orCount++;
        else if (elem == &sw) swCount++;
    }
    QCOMPARE(andCount, 1);  // Each appears exactly once
    QCOMPARE(orCount, 1);
    QCOMPARE(swCount, 1);

    // The forward edges (SW->AND, AND->OR) still determine order normally; the back edge
    // (OR->AND) that creates the cycle is simply not allowed to reorder anything, rather than
    // pulling orGate to the front as an earlier version of this comment assumed -- confirmed
    // empirically: sort order is sw, andGate, orGate.
    int orIndex = -1, andIndex = -1, swIndex = -1;
    for (int i = 0; i < sorted.size(); ++i) {
        if (sorted[i] == &orGate) orIndex = i;
        if (sorted[i] == &andGate) andIndex = i;
        if (sorted[i] == &sw) swIndex = i;
    }
    QVERIFY(orIndex >= 0);
    QVERIFY(andIndex >= 0);
    QVERIFY2(swIndex < andIndex, "sw should come before andGate (forward edge SW->AND)");
    QVERIFY2(andIndex < orIndex, "andGate should come before orGate (forward edge AND->OR)");
}

void TestScene::testSortDisconnectedComponents()
{
    // Test: Elements with no connections
    // SW1 (isolated) + (SW2 → AND → LED)
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    And andGate;
    Led led;

    builder.add(&sw1, &sw2, &andGate, &led);

    // Create chain: SW2 → AND → LED
    builder.connect(&sw2, 0, &andGate, 0);
    builder.connect(&andGate, 0, &led, 0);

    // SW1 is left disconnected

    // Use our specific elements
    QVector<Element *> elements{&sw1, &sw2, &andGate, &led};

    // Sort should handle disconnected components
    auto sorted = Scene::sortByTopology(elements);

    // All elements should be present
    QCOMPARE(sorted.size(), 4);

    // Find indices of each element in sorted order
    QHash<Element*, int> sortedIndex;
    for (int i = 0; i < sorted.size(); ++i) {
        sortedIndex[sorted[i]] = i;
    }

    QVERIFY(sortedIndex.contains(&sw1));
    QVERIFY(sortedIndex.contains(&sw2));
    QVERIFY(sortedIndex.contains(&andGate));
    QVERIFY(sortedIndex.contains(&led));

    // Verify the connected chain ordering based on priority (higher priority first)
    // Priority calculation: distance from outputs
    // SW2 (input) has highest priority (3 - furthest from output LED)
    // AND (logic gate) has medium priority (2 - between input and output)
    // LED (output) has lowest priority (1 - distance 0 from output)
    // Therefore sort order should be: SW2, AND, LED
    int ledIndex = sortedIndex.value(&led, -1);
    int andIndex = sortedIndex.value(&andGate, -1);
    int sw2Index = sortedIndex.value(&sw2, -1);
    int sw1Index = sortedIndex.value(&sw1, -1);

    // Verify priority-based ordering: higher priority elements come first
    QVERIFY2(sw2Index < andIndex, "SW2 (priority 3) should come before AND (priority 2)");
    QVERIFY2(andIndex < ledIndex, "AND (priority 2) should come before LED (priority 1)");

    // SW1 (isolated) can appear anywhere, but should be present
    QVERIFY2(sw1Index >= 0, "SW1 (isolated) should be in sorted list");
}

void TestScene::testForgetItemIdC7()
{
    // Pre-fix UpdateBlobCommand::reconnectConnections silently dropped
    // connections whose port disappeared on a blob shrink, leaving the
    // Connection's m_elementRegistry entry pointing at memory Qt's
    // parent-child cascade had already freed. The new helper drops the
    // entry by ID so subsequent itemById lookups return nullptr instead
    // of a dangling pointer.
    WorkSpace ws;
    auto *scene = ws.scene();
    auto *sw = new InputSwitch();
    auto *led = new Led();
    scene->addItem(sw);
    scene->addItem(led);

    auto *conn = new Connection();
    conn->setStartPort(sw->outputPort());
    conn->setEndPort(led->inputPort());
    scene->addItem(conn);

    const int connId = conn->id();
    QVERIFY(connId >= 0);
    QCOMPARE(scene->itemById(connId), conn);

    // Drop the registry entry without touching the item.
    scene->forgetItemId(connId);
    QVERIFY(scene->itemById(connId) == nullptr);

    // Lookup of a never-registered ID is a no-op.
    scene->forgetItemId(99999);
    QVERIFY(scene->itemById(99999) == nullptr);

    // Sibling registrations untouched.
    QCOMPARE(scene->itemById(sw->id()), static_cast<ItemWithId *>(sw));
    QCOMPARE(scene->itemById(led->id()), static_cast<ItemWithId *>(led));
}

void TestScene::testDrainConnectionsCleansRegistryHC()
{
    // Reproduces the registry-leak path suspected of causing WIREDPANDA-HC
    // (paint-time _purecall in QGraphicsItem::boundingRect, same family as
    // FH/EW/G1/GP).
    //
    // Port::drainConnections (called from ~InputPort/~OutputPort
    // during cascade-destruction of a deleted element) issues a bare
    // `delete conn`. Qt's ~QGraphicsItem then dispatches to the non-virtual
    // QGraphicsScene::removeItem, which removes the connection from the
    // BSP but skips Scene's override — this alone would leave
    // m_elementRegistry pointing at freed memory (a subsequent itemById/iter
    // that hits the stale entry would dereference a destructed object →
    // _purecall during paint), except that ItemWithId's own destructor
    // (running as part of the same `delete conn` cascade) self-unregisters
    // from whichever SceneItemRegistry it's mapped in — a structural fix
    // that covers this and every other cascade-delete path, not just this
    // one call site (unlike the earlier, narrower db565817c fix for
    // UpdateBlobCommand::reconnectConnections, which is now redundant).
    WorkSpace ws;
    auto *scene = ws.scene();
    auto *sw = new InputSwitch();
    auto *led = new Led();
    scene->addItem(sw);
    scene->addItem(led);

    auto *conn = new Connection();
    conn->setStartPort(sw->outputPort());
    conn->setEndPort(led->inputPort());
    scene->addItem(conn);

    const int connId = conn->id();
    QVERIFY(connId >= 0);
    QCOMPARE(scene->itemById(connId), conn);

    // Trigger drainConnections by destroying sw out-of-band — i.e. without
    // first deleting attached wires.  This mirrors the non-command paths
    // that existed in 5.0.1 (pre-0e5399d83 removeICFile, pre-78fc0882d
    // inline-IC save reload, pre-515d7a97f onFileChanged) where ports
    // could reach ~Port with a non-empty m_connections list.
    scene->removeItem(sw);
    delete sw;
    // ~InputSwitch → ~Element → ~QGraphicsItem cascades into
    // ~OutputPort → drainConnections(false) → `delete conn` (bare).

    // Expectation (post-fix): drainConnections routed the delete through
    // Scene::removeItem, so the registry no longer points at freed memory.
    // Pre-fix this assertion fails and proves the WIREDPANDA-HC hypothesis.
    QVERIFY(scene->itemById(connId) == nullptr);
}

void TestScene::testClipboardCanPasteMatchesPasteFormats()
{
    // Regression test (F4): the empty-canvas context menu gated Paste on the
    // *legacy* mime type only, while copy()/cut() write only the current one —
    // right-click Paste was permanently disabled for data this very app
    // copied. canPaste() is now the single source of truth shared with
    // paste(), which reads both formats.
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *sw = new InputSwitch();
    scene->addItem(sw);
    sw->setSelected(true);
    scene->copyAction();

    QVERIFY(ClipboardManager::canPaste(QApplication::clipboard()->mimeData()));

    QMimeData legacy;
    legacy.setData(MimeType::ClipboardLegacy, QByteArrayLiteral("x"));
    QVERIFY(ClipboardManager::canPaste(&legacy));

    QMimeData unrelated;
    unrelated.setText("not circuit data");
    QVERIFY(!ClipboardManager::canPaste(&unrelated));
    QVERIFY(!ClipboardManager::canPaste(nullptr));
}

void TestScene::testBuildDragImageClampsExtremeSelectionExtent()
{
    // Regression: cloneDrag()'s ghost image was sized proportionally to the scene distance
    // between selected elements — position validation on load only rejects non-finite
    // coordinates, never bounds magnitude (the same gap CircuitExporter::renderToImage hit).
    // One stray element at an extreme-but-finite position must not blow up the allocation.
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *sw1 = new InputSwitch();
    scene->addItem(sw1);
    auto *sw2 = new InputSwitch();
    scene->addItem(sw2);
    sw2->setPos(1e7, 1e7);

    const QRectF rect = sw1->sceneBoundingRect().united(sw2->sceneBoundingRect()).adjusted(-8, -8, 8, 8);

    // Identity transform mirrors 100% zoom, the worst case for this bug.
    const QImage image = ClipboardManager::buildDragImage(scene, QTransform(), rect);

    QVERIFY(!image.isNull());
    QVERIFY(image.width() <= ClipboardManager::kMaxDragImageDimension);
    QVERIFY(image.height() <= ClipboardManager::kMaxDragImageDimension);
}

void TestScene::testBuildDragImageClampsAnExtentBeyondIntRange()
{
    // An extent past int's range is scaled into the clamp before it is converted to whole pixels, so the drag image is
    // a clamped one, not a null image.
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *sw1 = new InputSwitch();
    scene->addItem(sw1);
    auto *sw2 = new InputSwitch();
    scene->addItem(sw2);
    sw2->setPos(1e12, 1e11);

    const QRectF rect = sw1->sceneBoundingRect().united(sw2->sceneBoundingRect()).adjusted(-8, -8, 8, 8);
    const QImage image = ClipboardManager::buildDragImage(scene, QTransform(), rect);

    QVERIFY(!image.isNull());
    QCOMPARE(image.width(), ClipboardManager::kMaxDragImageDimension);
    QVERIFY(image.height() <= ClipboardManager::kMaxDragImageDimension);
}

namespace TestSceneGridHelpers {

/// A paint engine that draws nothing and counts the points it is given.
class PointCountingEngine : public QPaintEngine
{
public:
    PointCountingEngine()
        : QPaintEngine(QPaintEngine::AllFeatures)
    {
    }

    bool begin(QPaintDevice *) override { return true; }
    bool end() override { return true; }
    void updateState(const QPaintEngineState &) override {}
    void drawPixmap(const QRectF &, const QPixmap &, const QRectF &) override {}
    void drawPoints(const QPointF *, int pointCount) override { points += pointCount; }
    void drawPoints(const QPoint *, int pointCount) override { points += pointCount; }
    Type type() const override { return QPaintEngine::User; }

    int points = 0;
};

/// A 64x64 device painted through a PointCountingEngine.
class PointCountingDevice : public QPaintDevice
{
public:
    QPaintEngine *paintEngine() const override { return &engine; }

    int metric(PaintDeviceMetric which) const override
    {
        switch (which) {
        case PdmWidth:
        case PdmHeight:
            return 64;
        case PdmDepth:
            return 32;
        case PdmDpiX:
        case PdmDpiY:
        case PdmPhysicalDpiX:
        case PdmPhysicalDpiY:
            return 96;
        case PdmDevicePixelRatioScaled:
            return static_cast<int>(devicePixelRatioFScale());
        default:
            return QPaintDevice::metric(which);
        }
    }

    mutable PointCountingEngine engine;
};

int gridPointsDrawn(Scene *scene, const QRectF &source)
{
    PointCountingDevice device;
    QPainter painter(&device);
    scene->render(&painter, QRectF(0, 0, 64, 64), source, Qt::IgnoreAspectRatio);
    painter.end();
    return device.engine.points;
}

} // namespace TestSceneGridHelpers

void TestScene::testGridIsNotDrawnBeyondIntRange()
{
    // The grid's dots are int points, so drawBackground() bounds the exposed rect in floating point before converting
    // its edges: a rect reaching past int's range, which a render of a far element exposes, spans far more grid cells
    // than the bound and draws none. Dots scaled into an image from so far are below a pixel, so the points the render
    // hands the paint engine are counted instead.
    using TestSceneGridHelpers::gridPointsDrawn;
    WorkSpace workspace;

    // The count sees the grid: a 64-unit square holds 5 by 5 of its points.
    QCOMPARE(gridPointsDrawn(workspace.scene(), QRectF(0, 0, 64, 64)), 25);
    QCOMPARE(gridPointsDrawn(workspace.scene(), QRectF(-1e12, 0, 2e12, 64)), 0);
}

void TestScene::testGeometryChangeKeepsSceneIndexConsistentKS()
{
    // Guards the invariant behind WIREDPANDA-KS (EXCEPTION_ACCESS_VIOLATION_READ inside
    // QGraphicsSceneBspTree::items during paint): every runtime change to an element's
    // boundingRect() must reindex the scene's BSP tree via prepareGeometryChange(), or the
    // stale bounds let a later paint walk the tree into a freed node. ElementAppearance's
    // setPixmap()/setRenderPixmap() already guard the pixmap-load paths; this exercises the
    // two remaining chokepoints — setPortSize() (port count) and applyOrientation() reached
    // directly via a rotate/flip — through the exact drawItems -> BSP paint pipeline, and
    // asserts the element stays correctly indexed and that render() does not fault.
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *elm = ElementFactory::buildElement(ElementType::And);
    QVERIFY2(elm != nullptr, "failed to create AND element");
    elm->setPos(0, 0);
    scene->addItem(elm);

    // Build the BSP index against the element's initial bounds.
    (void)scene->items(QRectF(-1000, -1000, 2000, 2000), Qt::IntersectsItemBoundingRect);

    // Port-count change: alters portsBoundingRect() and thus boundingRect().
    elm->setInputSize(elm->maxInputSize());
    QVERIFY2(scene->items(elm->sceneBoundingRect(), Qt::IntersectsItemBoundingRect).contains(elm),
        "element lost from scene BSP index after input-size change");

    // Orientation change: reaches applyOrientation() directly, which may swap in a
    // different-size SVG variant.
    elm->setRotation(90);
    elm->setFlippedX(true);
    QVERIFY2(scene->items(elm->sceneBoundingRect(), Qt::IntersectsItemBoundingRect).contains(elm),
        "element lost from scene BSP index after rotate/flip");

    // Drive the exact crashing path end-to-end: drawItems -> BSP traversal.
    QImage canvas(400, 400, QImage::Format_ARGB32);
    QPainter painter(&canvas);
    scene->render(&painter); // must not corrupt the index / fault
    painter.end();
}

void TestScene::testKeyTriggerIgnoresAutoRepeat()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *inputSwitch = qobject_cast<InputSwitch *>(ElementFactory::buildElement(ElementType::InputSwitch));
    QVERIFY(inputSwitch);
    inputSwitch->setTrigger(QKeySequence(Qt::Key_A));
    scene->addItem(inputSwitch);

    // Known starting state (setOn(bool) sets a definite level; the no-arg setOn() would toggle).
    inputSwitch->setOn(false);
    QVERIFY(!inputSwitch->isOn());

    // A genuine (non-repeat) key press toggles the latched switch on.
    QKeyEvent press(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, QString(), /*autorep*/ false);
    QApplication::sendEvent(scene, &press);
    QVERIFY(inputSwitch->isOn());

    // Auto-repeat presses (emitted while the key is held) must NOT toggle it again — otherwise
    // the switch would flip-flop many times a second while held.
    for (int i = 0; i < 5; ++i) {
        QKeyEvent repeat(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, QString(), /*autorep*/ true);
        QApplication::sendEvent(scene, &repeat);
        QVERIFY2(inputSwitch->isOn(), "auto-repeat press must not toggle the latched switch");
    }

    // A second genuine press toggles it back off, confirming the trigger still works normally.
    QKeyEvent press2(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, QString(), /*autorep*/ false);
    QApplication::sendEvent(scene, &press2);
    QVERIFY(!inputSwitch->isOn());
}

void TestScene::testArrowKeyNudgesSelection()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *element = ElementFactory::buildElement(ElementType::And);
    QVERIFY(element);
    scene->addItem(element);
    element->setPos(0, 0);
    element->setSelected(true);

    const int grid = Constants::gridSize;

    // Right arrow moves the selection one grid cell in +x.
    QKeyEvent right(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QApplication::sendEvent(scene, &right);
    QCOMPARE(element->pos(), QPointF(grid, 0));

    // Down arrow moves one grid cell in +y.
    QKeyEvent down(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QApplication::sendEvent(scene, &down);
    QCOMPARE(element->pos(), QPointF(grid, grid));

    // Up arrow moves one grid cell in -y.
    QKeyEvent up(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QApplication::sendEvent(scene, &up);
    QCOMPARE(element->pos(), QPointF(grid, 0));
    scene->undoStack()->undo(); // back to (grid, grid) so the rest of this test is unaffected

    // Shift+Left jumps four grid cells in -x.
    QKeyEvent shiftLeft(QEvent::KeyPress, Qt::Key_Left, Qt::ShiftModifier);
    QApplication::sendEvent(scene, &shiftLeft);
    QCOMPARE(element->pos(), QPointF(grid - 4 * grid, grid));

    // Each nudge is a separate undoable move.
    scene->undoStack()->undo();
    QCOMPARE(element->pos(), QPointF(grid, grid));

    // With nothing selected, arrow keys don't move the element.
    scene->clearSelection();
    const QPointF before = element->pos();
    QKeyEvent right2(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QApplication::sendEvent(scene, &right2);
    QCOMPARE(element->pos(), before);
}

void TestScene::testDroppedPandaFileDetection()
{
    // A .panda local file is recognised as openable.
    QMimeData pandaMime;
    pandaMime.setUrls({QUrl::fromLocalFile("/home/user/circuit.panda")});
    QCOMPARE(Scene::droppedPandaFile(&pandaMime), QStringLiteral("/home/user/circuit.panda"));

    // A non-.panda file is not.
    QMimeData txtMime;
    txtMime.setUrls({QUrl::fromLocalFile("/home/user/notes.txt")});
    QVERIFY(Scene::droppedPandaFile(&txtMime).isEmpty());

    // No URLs at all (e.g. an internal palette drag) is not.
    QMimeData noUrls;
    QVERIFY(Scene::droppedPandaFile(&noUrls).isEmpty());

    // The extension match is case-insensitive and picks the first .panda among several URLs.
    QMimeData mixed;
    mixed.setUrls({QUrl::fromLocalFile("/a/readme.md"), QUrl::fromLocalFile("/a/Design.PANDA")});
    QCOMPARE(Scene::droppedPandaFile(&mixed), QStringLiteral("/a/Design.PANDA"));
}

void TestScene::testMuteSilencesAllAudioOutputElements()
{
    // Regression: Scene::mute() used to special-case Buzzer only, so an AudioBox kept
    // playing audibly through a simulation pause (Simulation::stop() -> setMuted(true)).
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *buzzer = ElementFactory::buildElement(ElementType::Buzzer);
    auto *audioBox = ElementFactory::buildElement(ElementType::AudioBox);
    scene->addItem(buzzer);
    scene->addItem(audioBox);

    auto *buzzerAudio = qobject_cast<AudioOutputElement *>(buzzer);
    auto *audioBoxAudio = qobject_cast<AudioOutputElement *>(audioBox);
    QVERIFY(buzzerAudio);
    QVERIFY(audioBoxAudio);
    QVERIFY(!buzzerAudio->isMuted());
    QVERIFY(!audioBoxAudio->isMuted());

    scene->mute(true);
    QVERIFY(buzzerAudio->isMuted());
    QVERIFY2(audioBoxAudio->isMuted(), "Scene::mute() must silence AudioBox too, not just Buzzer");

    scene->mute(false);
    QVERIFY(!buzzerAudio->isMuted());
    QVERIFY(!audioBoxAudio->isMuted());
}

void TestScene::testWireAntialiasingDegradesOnSlowPassesAndRestores()
{
    constexpr qint64 kMs = 1'000'000; // recordWirePaintPass takes nanoseconds

    WorkSpace workspace;
    auto *scene = workspace.scene();

    // Full quality by default.
    QVERIFY(scene->wireAntialiasingEnabled());

    // One over-budget pass is not enough (debounce: a lone stall must not degrade)...
    scene->recordWirePaintPass(60 * kMs);
    QVERIFY(scene->wireAntialiasingEnabled());

    // ...and an in-budget pass in between resets the streak.
    scene->recordWirePaintPass(5 * kMs);
    scene->recordWirePaintPass(60 * kMs);
    QVERIFY(scene->wireAntialiasingEnabled());

    // Two consecutive over-budget passes degrade.
    scene->recordWirePaintPass(60 * kMs);
    QVERIFY(!scene->wireAntialiasingEnabled());

    // Anti-oscillation: degraded passes measuring well under budget -- but without deep
    // headroom -- must never restore, no matter how many. (The degraded renderer being
    // "fast enough" is a consequence of the degrade itself.)
    for (int i = 0; i < 50; ++i) {
        scene->recordWirePaintPass(20 * kMs);
    }
    QVERIFY(!scene->wireAntialiasingEnabled());

    // A non-headroom pass resets a deep-headroom streak.
    for (int i = 0; i < 9; ++i) {
        scene->recordWirePaintPass(2 * kMs);
    }
    scene->recordWirePaintPass(20 * kMs);
    for (int i = 0; i < 9; ++i) {
        scene->recordWirePaintPass(2 * kMs);
    }
    QVERIFY(!scene->wireAntialiasingEnabled());

    // Sustained deep headroom (10 consecutive passes under budget/worst-case-ratio)
    // restores: the next antialiased pass fits the budget by construction.
    scene->recordWirePaintPass(2 * kMs);
    QVERIFY(scene->wireAntialiasingEnabled());

    // Idle restore path: degrade again, then invoke the restore method as the idle
    // timer's timeout would.
    scene->recordWirePaintPass(60 * kMs);
    scene->recordWirePaintPass(60 * kMs);
    QVERIFY(!scene->wireAntialiasingEnabled());
    scene->restoreWireAntialiasing();
    QVERIFY(scene->wireAntialiasingEnabled());

    // Restoring resets the debounce: a single slow pass afterwards does not re-degrade.
    scene->recordWirePaintPass(60 * kMs);
    QVERIFY(scene->wireAntialiasingEnabled());
}

void TestScene::testPortAtFindsPortsOnly()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    andGate->setPos(100, 100);
    scene->addItem(andGate);

    auto *inputPort = static_cast<Port *>(andGate->inputPort(0));
    const QPointF portPos = inputPort->scenePos();

    // Exact hit on the port's origin.
    QCOMPARE(scene->portAt(portPos), inputPort);

    // Within the port's own ±5 hit square.
    QCOMPARE(scene->portAt(portPos + QPointF(3, 3)), inputPort);

    // Just outside the square but within the 4 px slop: the nearby phase finds it.
    QCOMPARE(scene->portAt(portPos + QPointF(8, 0)), inputPort);

    // The element body's centre is far from any port: elements are never returned.
    QVERIFY(!scene->portAt(andGate->sceneBoundingRect().center()));

    // Empty canvas.
    QVERIFY(!scene->portAt(QPointF(-500, -500)));
}

void TestScene::testResizeSceneIgnoresARubberBandOutsideTheCircuit()
{
    // The rubber-band selection item stays in the scene, hidden, with its last rectangle: it is not circuit, so
    // neither the circuit's bounds nor the scene rect reach it
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    andGate->setPos(100, 100);
    scene->addItem(andGate);

    TestUtils::ScenePointer pointer(scene);
    pointer.press(QPointF(3000, 3000));
    pointer.move(QPointF(3200, 3200));
    pointer.release(QPointF(3200, 3200));
    QCOMPARE(scene->circuitBoundingRect(), andGate->mapRectToScene(andGate->boundingRect() | andGate->childrenBoundingRect()));

    scene->setPropertyUpdateRequired();
    scene->resizeScene();
    QVERIFY2(!scene->sceneRect().contains(QPointF(3100, 3100)),
             qPrintable(QString("scene rect %1,%2 %3x%4")
                            .arg(scene->sceneRect().x()).arg(scene->sceneRect().y())
                            .arg(scene->sceneRect().width()).arg(scene->sceneRect().height())));
}

void TestScene::testCircuitBoundsLeaveOutAPortsHoverLabel()
{
    // A hovered port's label is drawn beside its element, in the scene but not circuit
    Scene scene;
    auto *andGate = ElementFactory::buildElement(ElementType::And);
    andGate->setPos(100, 100);
    scene.addItem(andGate);
    const QRectF circuit = scene.circuitBoundingRect();
    QCOMPARE(circuit, andGate->mapRectToScene(andGate->boundingRect() | andGate->childrenBoundingRect()));

    andGate->outputPort(0)->setName("out"); // an unnamed port shows no label
    const QRectF everyItemBefore = scene.itemsBoundingRect();
    scene.connectionManager()->showHoverLabels(andGate->outputPort(0));
    QVERIFY2(!everyItemBefore.contains(scene.itemsBoundingRect()), "the label must reach past the scene's items for this test to see it");
    QCOMPARE(scene.circuitBoundingRect(), circuit);
}

void TestScene::testResizeSceneWithoutAViewKeepsTheSceneRectOfAnEmptyCircuit()
{
    // With no view, an empty circuit leaves nothing to fit, and a null scene rect would hand it back to Qt
    Scene scene;
    const QRectF rect(-256, -256, 512, 512);
    scene.setSceneRect(rect);
    scene.resizeScene();
    QCOMPARE(scene.sceneRect(), rect);
}

void TestScene::testResizeSceneQuantizesSceneRect()
{
    constexpr qreal quantum = 256.0;
    const auto isQuantized = [](qreal edge) {
        return qFuzzyIsNull(std::fmod(edge, quantum));
    };

    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *andGate = ElementFactory::buildElement(ElementType::And);
    andGate->setPos(100, 100);
    scene->addItem(andGate);

    scene->resizeScene();
    const QRectF rect = scene->sceneRect();
    QVERIFY(rect.contains(scene->itemsBoundingRect()));
    QVERIFY(isQuantized(rect.left()));
    QVERIFY(isQuantized(rect.top()));
    QVERIFY(isQuantized(rect.right()));
    QVERIFY(isQuantized(rect.bottom()));

    // A small move well inside the current chunk must leave the rect bit-identical --
    // the exact-equality no-op in QGraphicsScene::setSceneRect() is what spares the
    // full BSP re-index. Invalidate the bounds cache as a real MoveCommand would, so
    // this exercises the quantization, not cache staleness.
    andGate->setPos(110, 110);
    scene->setPropertyUpdateRequired();
    scene->resizeScene();
    QCOMPARE(scene->sceneRect(), rect);

    // Growth past the current rect expands it, still on quantized edges. A real move
    // funnels through MoveCommand -> setPropertyUpdateRequired(), which dirties the
    // cached items bounding rect resizeScene() reads -- mirror that here, since a bare
    // setPos() bypasses the app's edit chokepoint.
    andGate->setPos(rect.right() + 300, rect.bottom() + 300);
    scene->setPropertyUpdateRequired();
    scene->resizeScene();
    const QRectF grown = scene->sceneRect();
    QVERIFY(grown.contains(scene->itemsBoundingRect()));
    QVERIFY(grown.right() > rect.right());
    QVERIFY(grown.bottom() > rect.bottom());
    QVERIFY(isQuantized(grown.right()));
    QVERIFY(isQuantized(grown.bottom()));
}

void TestScene::testCtrlClickTogglesElementSelection()
{
    // A Ctrl-modifier QGraphicsSceneMouseEvent synthesized via QTest doesn't reliably carry
    // the modifier through this sandbox's event pipeline (confirmed empirically: isSelected()
    // stayed unchanged after a QTest-driven Ctrl+click, meaning the modifier never arrived) --
    // call Scene's protected mousePressEvent() directly instead, via the friend seam.
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    auto *gate = new And();
    gate->setPos(100, 100);
    scene->addItem(gate);
    gate->setSelected(true);

    QGraphicsSceneMouseEvent event(QEvent::GraphicsSceneMousePress);
    event.setScenePos(gate->sceneBoundingRect().center());
    event.setButton(Qt::LeftButton);
    event.setButtons(Qt::LeftButton);
    event.setModifiers(Qt::ControlModifier);
    scene->mousePressEvent(&event);

    QVERIFY2(!gate->isSelected(), "Ctrl+click on a selected element must deselect it");
}

void TestScene::testPressOnPortWhileWireInProgressCompletesConnection()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    GraphicsView *view = workspace.view();

    auto *sw = new InputSwitch();
    sw->setPos(0, 0);
    scene->addItem(sw);
    auto *led = new Led();
    led->setPos(200, 0);
    scene->addItem(led);

    auto *outputPort = sw->outputPort(0);
    auto *inputPort = led->inputPort(0);

    scene->connectionManager()->startFromOutput(outputPort);
    QVERIFY(scene->connectionManager()->hasEditedConnection());

    const QPoint inputViewPos = view->mapFromScene(inputPort->sceneBoundingRect().center());
    QTest::mousePress(view->viewport(), Qt::LeftButton, Qt::NoModifier, inputViewPos);

    QVERIFY2(!scene->connectionManager()->hasEditedConnection(),
              "Pressing a compatible port while a wire is in progress must complete it immediately");
    QCOMPARE(TestUtils::countConnections(scene), 1);
}

void TestScene::testPressOnEmptyInputPortStartsWireFromInput()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    GraphicsView *view = workspace.view();

    auto *led = new Led();
    led->setPos(100, 100);
    scene->addItem(led);

    auto *inputPort = led->inputPort(0);
    const QPoint viewPos = view->mapFromScene(inputPort->sceneBoundingRect().center());
    QTest::mousePress(view->viewport(), Qt::LeftButton, Qt::NoModifier, viewPos);

    QVERIFY(scene->connectionManager()->hasEditedConnection());
    QCOMPARE(scene->connectionManager()->editedConnection()->endPort(), inputPort);
}

void TestScene::testPressOnOccupiedInputPortDetachesWire()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    GraphicsView *view = workspace.view();
    CircuitBuilder builder(scene);

    auto *sw = new InputSwitch();
    sw->setPos(0, 0);
    auto *led = new Led();
    led->setPos(200, 0);
    builder.add(sw, led);
    Connection *conn = builder.connect(sw, 0, led, 0);
    QVERIFY(conn);
    QCOMPARE(TestUtils::countConnections(scene), 1);

    auto *outputPort = sw->outputPort(0);
    auto *inputPort = led->inputPort(0);
    const QPoint viewPos = view->mapFromScene(inputPort->sceneBoundingRect().center());
    QTest::mousePress(view->viewport(), Qt::LeftButton, Qt::NoModifier, viewPos);

    // detach() deletes the old (completed) connection and starts a NEW in-progress wire
    // re-anchored at the SAME output port -- countConnections() alone can't tell them apart
    // (the in-progress wire is itself a real Connection item), so check that the input port's
    // original completed connection is really gone, and the new edited wire starts where the
    // old one did.
    QVERIFY2(inputPort->connections().isEmpty(), "The original completed connection must be detached from the input port");
    QVERIFY(scene->connectionManager()->hasEditedConnection());
    QCOMPARE(scene->connectionManager()->editedConnection()->startPort(), outputPort);
}

void TestScene::testRightClickOnElementOpensContextMenu()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    GraphicsView *view = workspace.view();

    auto *gate = new And();
    gate->setPos(50, 50);
    scene->addItem(gate);

    QSignalSpy contextMenuSpy(scene, &Scene::contextMenuPos);
    const QPoint viewPos = view->mapFromScene(gate->sceneBoundingRect().center());
    QTest::mousePress(view->viewport(), Qt::RightButton, Qt::NoModifier, viewPos);

    QCOMPARE(contextMenuSpy.count(), 1);
    QVERIFY2(gate->isSelected(), "Right-clicking an unselected element must select it before opening its menu");
}

void TestScene::testDraggingElementPastTimerThresholdEnsuresVisible()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    GraphicsView *view = workspace.view();
    view->resize(400, 300);

    auto *gate = new And();
    gate->setPos(50, 50);
    scene->addItem(gate);
    gate->setSelected(true);

    const QPoint startPos = view->mapFromScene(gate->sceneBoundingRect().center());
    QTest::mousePress(view->viewport(), Qt::LeftButton, Qt::NoModifier, startPos);

    const int hBefore = view->horizontalScrollBar()->value();
    const int vBefore = view->verticalScrollBar()->value();

    // Exceed the 50ms move-throttle so the next move actually calls ensureVisible().
    QTest::qWait(60);

    const QPoint farPos = startPos + QPoint(5000, 5000);
    QMouseEvent move(QEvent::MouseMove, farPos, view->viewport()->mapToGlobal(farPos),
                      Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(view->viewport(), &move);

    QVERIFY2(view->horizontalScrollBar()->value() != hBefore || view->verticalScrollBar()->value() != vBefore,
              "Dragging far outside the viewport past the throttle must scroll the view to follow it");

    QTest::mouseRelease(view->viewport(), Qt::LeftButton, Qt::NoModifier, farPos);
}

void TestScene::testDraggingElementInACentredSceneKeepsTheViewStill()
{
    // The scene's first rect is the view's before it is shown, smaller than the viewport, so Qt centres it and the
    // scroll bars, ranged 0 to 0, do not say where the view is
    WorkSpace workspace;
    workspace.resize(800, 600);
    workspace.show();
    QVERIFY(QTest::qWaitForWindowExposed(&workspace));
    Scene *scene = workspace.scene();
    GraphicsView *view = workspace.view();
    auto *gate = new And();
    gate->setPos(Element::snapToGrid(view->mapToScene(QPoint(600, 400))));
    scene->addItem(gate);
    const QPointF corner = view->mapToScene(QPoint(0, 0));
    QVERIFY(corner != QPointF(0, 0));

    const QPoint start = view->mapFromScene(gate->sceneBoundingRect().center());
    QTest::mousePress(view->viewport(), Qt::LeftButton, Qt::NoModifier, start);
    const QPoint end = start + QPoint(40, 32);
    QMouseEvent move(QEvent::MouseMove, end, view->viewport()->mapToGlobal(end), Qt::NoButton, Qt::LeftButton,
                     Qt::NoModifier);
    QApplication::sendEvent(view->viewport(), &move);
    QCOMPARE(view->mapToScene(QPoint(0, 0)), corner);
    QTest::mouseRelease(view->viewport(), Qt::LeftButton, Qt::NoModifier, end);
    QCOMPARE(view->mapToScene(QPoint(0, 0)), corner);
}

void TestScene::testCtrlDoubleClickOnWireDoesNotInsertNode()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    GraphicsView *view = workspace.view();
    CircuitBuilder builder(scene);

    auto *sw = new InputSwitch();
    auto *led = new Led();
    builder.add(sw, led);
    builder.connect(sw, 0, led, 0);

    const int undoCountBefore = scene->undoStack()->count();
    const QPoint viewPos = view->mapFromScene(QPointF(100, 0));
    QTest::mouseDClick(view->viewport(), Qt::LeftButton, Qt::ControlModifier, viewPos);

    QCOMPARE(scene->undoStack()->count(), undoCountBefore);
    QCOMPARE(TestUtils::countConnections(scene), 1);
}

void TestScene::testDoubleClickOnConnectedWireInsertsSplitNode()
{
    // Connection::updatePath() (real Bézier geometry, needed for the double-click hit test
    // below to find the wire at all) is a no-op unless rendering is enabled -- see
    // Application::renderingEnabled, disabled globally for the test suite.
    const bool prevRendering = Application::renderingEnabled;
    Application::renderingEnabled = true;

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    GraphicsView *view = workspace.view();
    CircuitBuilder builder(scene);

    auto *sw = new InputSwitch();
    sw->setPos(0, 0);
    auto *led = new Led();
    led->setPos(200, 0);
    builder.add(sw, led);
    Connection *conn = builder.connect(sw, 0, led, 0);
    QVERIFY(conn);
    conn->updatePath();

    const int undoCountBefore = scene->undoStack()->count();
    const QPointF midScenePos = conn->path().pointAtPercent(0.5);
    const QPoint viewPos = view->mapFromScene(midScenePos);

    // SceneInteraction::mouseDoubleClick() looks up the connection via its own tracked
    // m_mousePos (last set by a mousePress/mouseMove), not the double-click event's own
    // position -- send a real click at the same spot first so it's primed correctly.
    QTest::mouseClick(view->viewport(), Qt::LeftButton, Qt::NoModifier, viewPos);
    QTest::mouseDClick(view->viewport(), Qt::LeftButton, Qt::NoModifier, viewPos);

    QCOMPARE(scene->undoStack()->count(), undoCountBefore + 1);
    QCOMPARE(TestUtils::countConnections(scene), 2);

    Application::renderingEnabled = prevRendering;
}

void TestScene::testAddItemWithNullItemIsNoOp()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    const auto countBefore = scene->items().size();
    scene->addItem(static_cast<QGraphicsItem *>(nullptr));
    QCOMPARE(scene->items().size(), countBefore);
}

void TestScene::testRemoveItemWithNullItemIsNoOp()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    const auto countBefore = scene->items().size();
    scene->removeItem(static_cast<QGraphicsItem *>(nullptr));
    QCOMPARE(scene->items().size(), countBefore);
}

void TestScene::testConnectionsReturnsRealConnections()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    CircuitBuilder builder(scene);

    auto *sw = new InputSwitch();
    auto *led = new Led();
    builder.add(sw, led);
    Connection *conn = builder.connect(sw, 0, led, 0);
    QVERIFY(conn);

    const auto conns = scene->connections();
    QCOMPARE(conns.size(), 1);
    QCOMPARE(conns.constFirst(), conn);
}

void TestScene::testUpdateThemeUpdatesConnectionTheme()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    CircuitBuilder builder(scene);

    auto *sw = new InputSwitch();
    auto *led = new Led();
    builder.add(sw, led);
    Connection *conn = builder.connect(sw, 0, led, 0);
    QVERIFY(conn);

    // Real behavioral check: updateTheme() must reach the connection loop, not just the
    // element loop. Connection::pen() itself only gets re-set on a width change (Error vs.
    // non-Error, see applyStatusPen()'s own comment), so compare statusPen() -- the pen
    // paint() actually draws with -- across a real theme switch instead.
    ThemeManager::setTheme(Theme::Light);
    scene->updateTheme();
    const QColor lightColor = conn->statusPen().color();

    ThemeManager::setTheme(Theme::Dark);
    scene->updateTheme();
    const QColor darkColor = conn->statusPen().color();

    QVERIFY2(lightColor != darkColor,
              "Scene::updateTheme() must refresh each connection's status pen from the new theme");

    ThemeManager::setTheme(Theme::System);
}

void TestScene::testCheckWireIdleRestoreReArmsWhileStillActive()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->recordWirePaintPass(60'000'000);
    scene->recordWirePaintPass(60'000'000);
    QVERIFY(!scene->wireAntialiasingEnabled());

    // Neither idle window has elapsed yet -- must re-arm the timer, not restore.
    scene->checkWireIdleRestore();
    QVERIFY2(!scene->wireAntialiasingEnabled(),
              "checkWireIdleRestore() must re-arm rather than restore before either idle window elapses");
}

void TestScene::testCheckWireIdleRestoreRestoresAfterBothWindowsElapse()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->recordWirePaintPass(60'000'000);
    scene->recordWirePaintPass(60'000'000);
    QVERIFY(!scene->wireAntialiasingEnabled());

    // Real wall-clock wait (QElapsedTimer has no fake-clock seam) -- a 3100ms wait against the
    // 3000ms kWireAaFlipIdleMs threshold left only a ~3.3% margin, flaky-prone on a loaded CI
    // runner. 600ms of slack (20%) is still cheap against this test's multi-second runtime.
    QTest::qWait(3600); // exceeds both kWireAaPassIdleMs (300ms) and kWireAaFlipIdleMs (3000ms)
    scene->checkWireIdleRestore();
    QVERIFY(scene->wireAntialiasingEnabled());
}

void TestScene::testRestoreWireAntialiasingIsNoOpWhenAlreadyEnabled()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    QVERIFY(scene->wireAntialiasingEnabled());
    scene->restoreWireAntialiasing();
    QVERIFY(scene->wireAntialiasingEnabled());
}

void TestScene::testContextMenuOnSelectedItemEmitsWithoutClearingSelection()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    auto *gate = new And();
    gate->setPos(50, 50);
    scene->addItem(gate);
    gate->setSelected(true);

    QGraphicsSceneMouseEvent moveEvent(QEvent::GraphicsSceneMouseMove);
    moveEvent.setScenePos(gate->sceneBoundingRect().center());
    scene->mouseMoveEvent(&moveEvent);

    QSignalSpy contextMenuSpy(scene, &Scene::contextMenuPos);
    scene->contextMenu(QPoint(10, 10));

    QCOMPARE(contextMenuSpy.count(), 1);
    QVERIFY2(gate->isSelected(), "Right-clicking an already-selected element must not clear its selection");
}

void TestScene::testContextMenuOnEmptyCanvasWithNothingToPasteOrSelect()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    QApplication::clipboard()->clear();

    // QMenu::exec() shows a popup, not a QWidget::isModal() dialog, so closeAnyModal() never
    // detects it (confirmed empirically -- it hung indefinitely) -- close it directly instead.
    TestUtils::AutoDismisser dismisser([](QWidget *w) {
        if (auto *menu = qobject_cast<QMenu *>(w)) {
            menu->close();
            return true;
        }
        return false;
    });
    scene->contextMenu(QPoint(10, 10));

#ifdef Q_OS_MACOS
    // With every action disabled (nothing to paste, nothing to select), macOS's native
    // NSMenu auto-enable/disable bookkeeping can close the menu before the polling dismisser
    // above ever gets a chance to observe it open -- confirmed against Apple's own documented
    // automatic-menu-item-validation behavior. dismissCount() of 0 is therefore also a valid
    // outcome here specifically.
    QVERIFY(dismisser.dismissCount() <= 1);
#else
    QCOMPARE(dismisser.dismissCount(), 1);
#endif
}

void TestScene::testContextMenuOnEmptyCanvasWithPasteableClipboard()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    auto *gate = new And();
    gate->setPos(50, 50);
    scene->addItem(gate);
    gate->setSelected(true);
    scene->copyAction();
    QVERIFY(ClipboardManager::canPaste(QApplication::clipboard()->mimeData()));

    // QMenu::exec() shows a popup, not a QWidget::isModal() dialog, so closeAnyModal() never
    // detects it (confirmed empirically -- it hung indefinitely) -- close it directly instead.
    TestUtils::AutoDismisser dismisser([](QWidget *w) {
        if (auto *menu = qobject_cast<QMenu *>(w)) {
            menu->close();
            return true;
        }
        return false;
    });
    scene->contextMenu(QPoint(10, 10));

    // Same macOS NSMenu auto-enable/disable race as the sibling test above: confirmed by CI
    // (macos-26 Qt6.9.3) to also intermittently close the menu here even with Paste enabled,
    // so dismissCount() of 0 is a valid outcome on that platform too.
    QVERIFY(dismisser.dismissCount() <= 1);
}

void TestScene::testContextMenuPasteOfAClipboardThatDoesNotReadReportsIt()
{
    // The canvas menu's Paste is a slot Qt calls: a clipboard that does not read is reported there, nothing added
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    QByteArray items;
    {
        QDataStream itemStream(&items, QIODevice::WriteOnly);
        Serialization::writePandaHeader(itemStream);
        itemStream << QPointF() << static_cast<int>(Element::Type);
    }
    auto *mimeData = new QMimeData;
    mimeData->setData(MimeType::Clipboard, items);
    QApplication::clipboard()->setMimeData(mimeData);
    QVERIFY(ClipboardManager::canPaste(QApplication::clipboard()->mimeData()));

    QString thrown;
    TestUtils::AutoDismisser dismisser([&thrown](QWidget *w) {
        auto *menu = qobject_cast<QMenu *>(w);
        if (!menu) {
            return false;
        }
        try {
            menu->actions().constFirst()->trigger();
        } catch (const std::exception &e) {
            thrown = e.what();
        }
        menu->close();
        return true;
    });
    scene->contextMenu(QPoint(10, 10));

    if (dismisser.dismissCount() == 0) {
        // macOS's native menu can close before the dismisser sees it (see the test above)
        QSKIP("The menu closed before it could be driven");
    }
    QVERIFY2(thrown.isEmpty(), qPrintable("Paste threw: " + thrown));
    QCOMPARE(scene->elements().size(), 0);
}

void TestScene::testDroppedPandaFileSkipsNonLocalUrls()
{
    QMimeData mixed;
    mixed.setUrls({QUrl("http://example.com/remote.panda"), QUrl::fromLocalFile("/a/local.panda")});
    QCOMPARE(Scene::droppedPandaFile(&mixed), QStringLiteral("/a/local.panda"));
}

void TestScene::testDragEnterEventAcceptsSupportedFormat()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    QMimeData mimeData;
    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    Serialization::writePandaHeader(payloadStream);
    payloadStream << QPoint(0, 0) << ElementType::And << QString();
    mimeData.setData(MimeType::DragDrop, payload);

    QGraphicsSceneDragDropEvent event(QEvent::GraphicsSceneDragEnter);
    event.setMimeData(&mimeData);
    event.setProposedAction(Qt::CopyAction);
    scene->dragEnterEvent(&event);

    // acceptProposedAction() narrows dropAction() down to exactly the proposed action --
    // the real, observable difference from the base-class fallback below (which leaves it at
    // its unnarrowed multi-flag default).
    QVERIFY(event.isAccepted());
    QCOMPARE(event.dropAction(), Qt::CopyAction);
}

void TestScene::testDragEnterEventFallsBackForUnsupportedFormat()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    QMimeData mimeData;
    mimeData.setData("text/plain", QByteArray("hello"));

    // Confirm this mimeData actually drives Scene::dragEnterEvent() into the QGraphicsScene
    // base-class fallback branch (App/Scene/Scene.cpp), rather than the early-return
    // acceptProposedAction() branch above. This is the stable, wiRedPanda-owned contract;
    // QGraphicsScene's own base-class dropAction() resolution afterwards is an undocumented
    // Qt implementation detail that has been observed to differ between Qt point releases
    // (e.g. 6.8.3 vs 6.9.3), so it isn't asserted here.
    QVERIFY(!SceneDropHandler::isSupportedDropFormat(&mimeData));
    QVERIFY(Scene::droppedPandaFile(&mimeData).isEmpty());

    QGraphicsSceneDragDropEvent event(QEvent::GraphicsSceneDragEnter);
    event.setMimeData(&mimeData);
    event.setProposedAction(Qt::CopyAction);
    scene->dragEnterEvent(&event);
}

void TestScene::testDragMoveEventAcceptsSupportedFormat()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    QMimeData mimeData;
    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    Serialization::writePandaHeader(payloadStream);
    payloadStream << QPoint(0, 0) << ElementType::And << QString();
    mimeData.setData(MimeType::DragDrop, payload);

    QGraphicsSceneDragDropEvent event(QEvent::GraphicsSceneDragMove);
    event.setMimeData(&mimeData);
    scene->dragMoveEvent(&event);

    QVERIFY(event.isAccepted());
}

void TestScene::testDragMoveEventFallsBackForUnsupportedFormat()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    QMimeData mimeData;
    mimeData.setData("text/plain", QByteArray("hello"));

    QGraphicsSceneDragDropEvent event(QEvent::GraphicsSceneDragMove);
    event.setMimeData(&mimeData);
    scene->dragMoveEvent(&event);

    QVERIFY(!event.isAccepted());
}

void TestScene::testDropEventHandlesCloneDragFormat()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    auto *gate = new And();
    gate->setPos(48, 48);
    scene->addItem(gate);
    gate->setSelected(true);

    const qsizetype countBefore = scene->elements().size();

    QByteArray itemData;
    QDataStream writeStream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(writeStream);
    const QPointF dragStartPos = gate->pos();
    writeStream << dragStartPos;
    writeStream << gate->pos();
    Serialization::serialize({gate}, writeStream, {.purpose = SerializationPurpose::InMemorySnapshot});

    QMimeData mimeData;
    mimeData.setData(MimeType::CloneDrag, itemData);

    QGraphicsSceneDragDropEvent event(QEvent::GraphicsSceneDrop);
    event.setMimeData(&mimeData);
    event.setScenePos(dragStartPos + QPointF(96, 0));
    scene->dropEvent(&event);

    QCOMPARE(scene->elements().size(), countBefore + 1);
}

void TestScene::testDropEventOpensDroppedPandaFile()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    QMimeData mimeData;
    mimeData.setUrls({QUrl::fromLocalFile("/home/user/circuit.panda")});

    QGraphicsSceneDragDropEvent event(QEvent::GraphicsSceneDrop);
    event.setMimeData(&mimeData);
    event.setProposedAction(Qt::CopyAction);

    QSignalSpy fileDropSpy(scene, &Scene::fileDropRequested);
    scene->dropEvent(&event);

    QCOMPARE(fileDropSpy.count(), 1);
    QCOMPARE(fileDropSpy.constFirst().at(0).toString(), QStringLiteral("/home/user/circuit.panda"));
    QCOMPARE(event.dropAction(), Qt::CopyAction);
}

void TestScene::testDropThatDoesNotReadIsReported_data()
{
    QTest::addColumn<QString>("format");
    QTest::addColumn<QByteArray>("payload");

    // A new IC whose file is gone
    QByteArray icPayload;
    QDataStream icStream(&icPayload, QIODevice::WriteOnly);
    Serialization::writePandaHeader(icStream);
    icStream << QPoint(0, 0) << ElementType::IC << QString("gone.panda");
    QTest::newRow("an IC whose file is gone") << QString(MimeType::DragDrop) << icPayload;

    // A clone's items, from another wiRedPanda, that stop halfway
    QByteArray items;
    QDataStream itemsStream(&items, QIODevice::WriteOnly);
    And gate;
    Serialization::serialize({&gate}, itemsStream, {.purpose = SerializationPurpose::InMemorySnapshot});
    QByteArray clonePayload;
    QDataStream cloneStream(&clonePayload, QIODevice::WriteOnly);
    Serialization::writePandaHeader(cloneStream);
    cloneStream << QPointF(0, 0) << QPointF(0, 0);
    clonePayload.append(items.first(items.size() / 2));
    QTest::newRow("a clone's items that do not read") << QString(MimeType::CloneDrag) << clonePayload;
}

void TestScene::testDropThatDoesNotReadIsReported()
{
    // A drop is an event handler Qt calls: a payload that does not read is reported where it lands, once, nothing
    // added and nothing thrown. Called directly, a throw fails on every platform; sent through Qt's dispatch, the frame
    // the user's drop comes through, a throw would end the process on macOS, where notify() cannot catch it
    QFETCH(QString, format);
    QFETCH(QByteArray, payload);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    scene->setContextDir(dir.path());
    QMimeData mimeData;
    mimeData.setData(format, payload);

    const bool interactive = Application::interactiveMode;
    Application::interactiveMode = true;
    const auto reports = [] {
        QList<QMessageBox *> boxes;
        for (auto *widget : QApplication::topLevelWidgets()) {
            if (auto *box = qobject_cast<QMessageBox *>(widget); box && box->isVisible()) {
                boxes.append(box);
            }
        }
        return boxes;
    };
    const auto closeReports = [&reports] {
        for (auto *box : reports()) {
            box->close();
        }
    };
    const auto restore = qScopeGuard([&] {
        Application::interactiveMode = interactive;
        closeReports();
    });

    QGraphicsSceneDragDropEvent direct(QEvent::GraphicsSceneDrop);
    direct.setMimeData(&mimeData);
    direct.setProposedAction(Qt::CopyAction);
    try {
        scene->dropEvent(&direct);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("the drop threw: %1").arg(e.what())));
    }
    QCOMPARE(reports().size(), 1);
    closeReports();

    QGraphicsSceneDragDropEvent dispatched(QEvent::GraphicsSceneDrop);
    dispatched.setMimeData(&mimeData);
    dispatched.setProposedAction(Qt::CopyAction);
    QApplication::sendEvent(scene, &dispatched);
    QCOMPARE(reports().size(), 1);

    QCOMPARE(scene->elements().size(), 0);
    QCOMPARE(scene->undoStack()->count(), 0);
}

void TestScene::testNudgeSelectionIgnoresCtrlAndAltModifiers()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    auto *gate = new And();
    gate->setPos(0, 0);
    scene->addItem(gate);
    gate->setSelected(true);

    QKeyEvent ctrlRight(QEvent::KeyPress, Qt::Key_Right, Qt::ControlModifier);
    QVERIFY2(!scene->nudgeSelection(&ctrlRight), "Ctrl+arrow must be left to other handlers, not nudge the selection");
    QCOMPARE(gate->pos(), QPointF(0, 0));

    QKeyEvent altRight(QEvent::KeyPress, Qt::Key_Right, Qt::AltModifier);
    QVERIFY(!scene->nudgeSelection(&altRight));
    QCOMPARE(gate->pos(), QPointF(0, 0));
}

void TestScene::testKeyReleaseEventReleasesInputButtonTrigger()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    auto *button = new InputButton();
    button->setTrigger(QKeySequence(Qt::Key_B));
    scene->addItem(button);

    QKeyEvent press(QEvent::KeyPress, Qt::Key_B, Qt::NoModifier);
    QApplication::sendEvent(scene, &press);
    QVERIFY(button->isOn());

    QKeyEvent release(QEvent::KeyRelease, Qt::Key_B, Qt::NoModifier);
    QApplication::sendEvent(scene, &release);
    QVERIFY2(!button->isOn(), "Releasing the trigger key must turn a momentary InputButton back off");
}

void TestScene::testEventFilterRemapsShiftClickToCtrlClick()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    auto *gate = new And();
    gate->setPos(100, 100);
    scene->addItem(gate);
    gate->setSelected(true);

    QGraphicsSceneMouseEvent event(QEvent::GraphicsSceneMousePress);
    event.setScenePos(gate->sceneBoundingRect().center());
    event.setButton(Qt::LeftButton);
    event.setButtons(Qt::LeftButton);
    event.setModifiers(Qt::ShiftModifier);

    // eventFilter() remaps Shift to Ctrl in place, then forwards to the base class filter --
    // observe the remap directly on the same event object it mutates.
    scene->eventFilter(scene, &event);
    QCOMPARE(event.modifiers(), Qt::ControlModifier);
}

void TestScene::testHelpEventShowsPortHoverLabelsOverPort()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    auto *led = new Led();
    led->setPos(100, 100);
    scene->addItem(led);
    auto *inputPort = led->inputPort(0);

    QGraphicsSceneHelpEvent event(QEvent::GraphicsSceneHelp);
    event.setScenePos(inputPort->sceneBoundingRect().center());
    scene->helpEvent(&event);

    // The real, load-bearing check: helpEvent() over a port returns immediately after routing
    // to ConnectionManager's hover labels, never falling through to QGraphicsScene::helpEvent()
    // (which ignore()s the event when nothing else provides a tooltip).
    QVERIFY(event.isAccepted());
}

void TestScene::testHelpEventShowsTheTopmostItemsText()
{
    // Away from a port the topmost item under the pointer with text shows it: an element's help text, asked for then,
    // so in the language current at that moment; another item's tooltip; and over nothing with text, no tooltip.
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    auto *gate = new And();
    gate->setPos(100, 100);
    scene->addItem(gate);
    const QPointF overGate = gate->sceneBoundingRect().center();
    QVERIFY2(!scene->portAt(overGate), "Precondition: no port must be under the gate's centre");

    const auto help = [scene](const QPointF scenePos) {
        QGraphicsSceneHelpEvent event(QEvent::GraphicsSceneHelp);
        event.setScenePos(scenePos);
        event.setScreenPos(QPoint(10, 10));
        scene->helpEvent(&event);
        return event.isAccepted();
    };
    auto hideTip = qScopeGuard([] { QToolTip::hideText(); });

    TestUtils::TagEveryStringTranslator tagging;
    QCoreApplication::installTranslator(&tagging);
    const bool shownTagged = help(overGate);
    QCoreApplication::removeTranslator(&tagging);
    QVERIFY(shownTagged);
    QCOMPARE(QToolTip::text(), QStringLiteral("[tr And]"));

    QVERIFY(help(overGate));
    QCOMPARE(QToolTip::text(), QStringLiteral("And"));

    // An item of another kind above the gate shows its own tooltip
    auto *cover = new QGraphicsRectItem(gate->sceneBoundingRect());
    cover->setToolTip(QStringLiteral("cover"));
    cover->setZValue(gate->zValue() + 1);
    scene->QGraphicsScene::addItem(cover);
    QVERIFY(help(overGate));
    QCOMPARE(QToolTip::text(), QStringLiteral("cover"));

    // One without text lets the gate's show through
    cover->setToolTip(QString());
    QVERIFY(help(overGate));
    QCOMPARE(QToolTip::text(), QStringLiteral("And"));

    // Over empty space nothing shows, and the tooltip shown goes
    QVERIFY(!help(QPointF(-500, -500)));
    QVERIFY(TestUtils::waitFor([] { return !QToolTip::isVisible(); }));
}

void TestScene::testHelpEventOverASegmentPortNamesItInTheCurrentLanguage()
{
    // A port's hover label reads its name when the pointer rests on it, so a segment's derived name is in the language
    // current then, not the one it was built in.
    WorkSpace workspace;
    Scene *scene = workspace.scene();

    auto *display = new Display7();
    display->setPos(100, 100);
    scene->addItem(display);
    auto *segment = display->inputPort(0);

    const auto hoverLabels = [scene] {
        QStringList texts;
        for (auto *item : scene->items()) {
            if (auto *label = qgraphicsitem_cast<PortHoverLabel *>(item)) {
                texts << label->text();
            }
        }
        return texts;
    };

    TestUtils::TagEveryStringTranslator tagging;
    QCoreApplication::installTranslator(&tagging);
    QGraphicsSceneHelpEvent event(QEvent::GraphicsSceneHelp);
    event.setScenePos(segment->scenePos());
    scene->helpEvent(&event);
    QCoreApplication::removeTranslator(&tagging);

    QCOMPARE(hoverLabels(), QStringList{QStringLiteral("G ([tr middle])")});
}

void TestScene::testElementCountAgreesAfterEveryMutation()
{
    // The status bar shows elementCount(), which the item registry keeps so the status bar never sorts the circuit.
    // After each path that adds or removes an element, the count must equal a walk of the scene. A debug build also
    // asserts this on every walk; this test checks it in every build.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    const auto agrees = [scene] {
        return scene->elementCount() == scene->elements().size();
    };
    QCOMPARE(scene->elementCount(), 0);

    auto *sw = new InputSwitch();
    auto *led = new Led();
    scene->addItem(sw);
    scene->addItem(led);
    QCOMPARE(scene->elementCount(), 2);
    QVERIFY2(agrees(), "after adding two elements");

    auto *gate = new And();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{gate}, scene));
    QCOMPARE(scene->elementCount(), 3);
    QVERIFY2(agrees(), "after adding a gate through a command");

    auto *conn = new Connection();
    conn->setStartPort(sw->outputPort());
    conn->setEndPort(led->inputPort());
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{conn}, scene));
    QCOMPARE(scene->elementCount(), 3);
    QVERIFY2(agrees(), "after adding a wire through a command");

    gate->setSelected(true);
    scene->deleteAction();
    QCOMPARE(scene->elementCount(), 2);
    QVERIFY2(agrees(), "after deleting the gate");
    scene->undoStack()->undo();
    QCOMPARE(scene->elementCount(), 3);
    QVERIFY2(agrees(), "after undoing the delete");
    scene->undoStack()->redo();
    QCOMPARE(scene->elementCount(), 2);
    QVERIFY2(agrees(), "after redoing the delete");

    scene->clearSelection();
    sw->setSelected(true);
    scene->copyAction();
    QVERIFY2(agrees(), "after copy");
    scene->cutAction();
    QCOMPARE(scene->elementCount(), 1);
    QVERIFY2(agrees(), "after cut");
    scene->pasteAction();
    QCOMPARE(scene->elementCount(), 2);
    QVERIFY2(agrees(), "after paste");

    scene->selectAll();
    scene->duplicateAction();
    QCOMPARE(scene->elementCount(), 4);
    QVERIFY2(agrees(), "after duplicate");
    scene->selectAll();
    scene->rotateRight();
    QVERIFY2(agrees(), "after rotating");

    auto *removed = new Led();
    scene->addItem(removed);
    scene->removeItem(removed);
    QVERIFY2(agrees(), "after removing an element");
    delete removed;
    QVERIFY2(agrees(), "after deleting a removed element");

    auto *destroyed = new Led();
    scene->addItem(destroyed);
    delete destroyed;
    QCOMPARE(scene->elementCount(), 4);
    QVERIFY2(agrees(), "after destroying an element still in the scene");

    scene->forgetItemId(led->id());
    QCOMPARE(scene->elementCount(), 4);
    QVERIFY2(agrees(), "after forgetting an element's id");

    // A loaded file: each IC counts once, and the circuit inside it not at all.
    WorkSpace loaded;
    loaded.load(TestUtils::examplesDir() + "nested.panda");
    const auto elements = loaded.scene()->elements();
    QVERIFY(std::any_of(elements.cbegin(), elements.cend(), [](const auto *elm) {
        return elm->elementType() == ElementType::IC;
    }));
    QCOMPARE(loaded.scene()->elementCount(), elements.size());
}

void TestScene::testElementCountKeepsAForgottenElement()
{
    // Forgetting an element's id leaves the element in the scene, so it stays in the count; destroying it afterwards
    // still takes it out.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *sw = new InputSwitch();
    auto *led = new Led();
    scene->addItem(sw);
    scene->addItem(led);

    scene->forgetItemId(sw->id());
    QCOMPARE(scene->elementCount(), 2);
    QCOMPARE(scene->elementCount(), scene->elements().size());

    delete sw;
    QCOMPARE(scene->elementCount(), 1);
    QCOMPARE(scene->elementCount(), scene->elements().size());
}
