// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testdragdrop.h"

#include "and.h"
#include "commands.h"
#include "elementfactory.h"
#include "globalproperties.h"
#include "graphicsview.h"
#include "qneconnection.h"
#include "scene.h"
#include "serialization.h"
#include "workspace.h"

#include <QApplication>
#include <QDragEnterEvent>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QTest>

void TestDragDrop::initTestCase()
{
    // Verify we're running in offscreen mode
    QString platform = qApp->platformName();
    qDebug() << "Running on platform:" << platform;
    
    // This test suite is designed to work with offscreen
    if (platform == "offscreen") {
        qDebug() << "Running in offscreen mode - drag-drop simulation will use event posting";
    }
}

void TestDragDrop::cleanupTestCase()
{
    // Clean up
}

void TestDragDrop::testBasicMouseEvents()
{
    // First test if basic mouse events work in offscreen mode
    WorkSpace workspace;
    auto* scene = workspace.scene();
    
    // Add an element
    auto* element = new And();
    scene->addItem(element);
    element->setPos(100, 100);
    
    // Try to select it with a mouse event
    QGraphicsSceneMouseEvent pressEvent(QEvent::GraphicsSceneMousePress);
    pressEvent.setScenePos(QPointF(100, 100));
    pressEvent.setButton(Qt::LeftButton);
    pressEvent.setButtons(Qt::LeftButton);
    
    // Send event to scene
    QApplication::sendEvent(scene, &pressEvent);
    
    // In offscreen mode, selection might not work the same way
    // but we can verify the element exists
    QCOMPARE(scene->elements().size(), 1);
    QCOMPARE(scene->elements()[0], element);
}

void TestDragDrop::testMimeDataCreation()
{
    // Test creating mime data for drag-drop (this should work in offscreen)
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    
    // Write header
    stream << QString("WPanda");
    stream << GlobalProperties::version;
    
    // Write element data
    stream << QPoint(0, 0);        // offset
    stream << ElementType::And;     // element type
    stream << QString("");          // IC filename
    
    auto* mimeData = new QMimeData();
    mimeData->setData("application/x-wiredpanda-dragdrop", itemData);
    
    // Verify mime data
    QVERIFY(mimeData->hasFormat("application/x-wiredpanda-dragdrop"));
    QVERIFY(!mimeData->data("application/x-wiredpanda-dragdrop").isEmpty());
    
    delete mimeData;
}

void TestDragDrop::testDropEventSimulation()
{
    // Test simulating a drop event in offscreen mode
    WorkSpace workspace;
    auto* scene = workspace.scene();
    
    // Create mime data as if dragging an AND gate from toolbar
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    stream << QPoint(0, 0);
    stream << ElementType::And;
    stream << QString("");
    
    auto* mimeData = new QMimeData();
    mimeData->setData("application/x-wiredpanda-dragdrop", itemData);
    
    // Create and send drop event
    QGraphicsSceneDragDropEvent dropEvent(QEvent::GraphicsSceneDrop);
    dropEvent.setScenePos(QPointF(150, 150));
    dropEvent.setMimeData(mimeData);
    dropEvent.setProposedAction(Qt::CopyAction);
    dropEvent.acceptProposedAction();
    
    // Send event to scene
    QApplication::sendEvent(scene, &dropEvent);
    
    // Verify element was created
    QCOMPARE(scene->elements().size(), 1);
    QCOMPARE(scene->elements()[0]->elementType(), ElementType::And);
    
    // Position might not be exact in offscreen mode, but should be close
    auto pos = scene->elements()[0]->pos();
    QVERIFY(qAbs(pos.x() - 150) < 50);
    QVERIFY(qAbs(pos.y() - 150) < 50);
}

void TestDragDrop::testQTestMouseDragSimulation()
{
    // Test if QTest::mousePress/Move/Release work in offscreen
    WorkSpace workspace;
    auto* scene = workspace.scene();
    GraphicsView* view = workspace.view();
    
    // Add an element to drag
    auto* element = new And();
    scene->addItem(element);
    element->setPos(50, 50);
    element->setSelected(true);
    
    QPoint startPoint(50, 50);
    QPoint endPoint(150, 150);
    
    // Note: In offscreen mode, these might not trigger actual drag
    // but we can test if they don't crash
    QTest::mousePress(view->viewport(), Qt::LeftButton, Qt::NoModifier, startPoint);
    QTest::mouseMove(view->viewport(), QPoint(100, 100));
    QTest::mouseMove(view->viewport(), endPoint);
    QTest::mouseRelease(view->viewport(), Qt::LeftButton, Qt::NoModifier, endPoint);
    
    // The element might not move in offscreen mode
    // But we can verify it still exists and no crash occurred
    QCOMPARE(scene->elements().size(), 1);
    QVERIFY(scene->elements().contains(element));
}

void TestDragDrop::testProgrammaticDragDrop()
{
    // For offscreen mode, we often need to use programmatic approaches
    WorkSpace workspace;
    auto* scene = workspace.scene();
    
    // Method 1: Direct element creation and positioning (most reliable)
    auto* element1 = new And();
    scene->addItem(element1);
    element1->setPos(100, 100);
    
    // Method 2: Using commands (also reliable)
    auto* element2 = new And();
    element2->setPos(200, 200);
    scene->receiveCommand(new AddItemsCommand({element2}, scene));
    
    // Verify both methods work
    QCOMPARE(scene->elements().size(), 2);
    QVERIFY(scene->elements().contains(element1));
    QVERIFY(scene->elements().contains(element2));
}

void TestDragDrop::testCloneDragSimulation()
{
    // Test clone drag (Ctrl+drag) simulation
    WorkSpace workspace;
    auto* scene = workspace.scene();
    
    // Create original element
    auto* original = new And();
    scene->addItem(original);
    original->setPos(100, 100);
    
    // In offscreen mode, we simulate clone operation programmatically
    // This is more reliable than trying to simulate Ctrl+drag
    
    // Serialize the original
    QByteArray itemData;
    QDataStream stream(&itemData, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    QList<QGraphicsItem*> items = {original};
    Serialization::serialize(items, stream);
    
    // Deserialize to create clone
    QDataStream readStream(&itemData, QIODevice::ReadOnly);
    Serialization::readPandaHeader(readStream);
    auto clonedItems = Serialization::deserialize(readStream, {}, GlobalProperties::version);
    
    // Add cloned items to scene
    for (auto* item : clonedItems) {
        scene->addItem(item);
        if (item->type() == GraphicElement::Type) {
            item->setPos(200, 200); // Offset position
        }
    }
    
    // Verify we have original + clone
    QCOMPARE(scene->elements().size(), 2);
}

void TestDragDrop::testConnectionCreation()
{
    // Test creating connections (dragging from port to port)
    WorkSpace workspace;
    auto* scene = workspace.scene();
    
    // Create two elements
    auto* and1 = new And();
    auto* and2 = new And();
    scene->addItem(and1);
    scene->addItem(and2);
    and1->setPos(50, 50);
    and2->setPos(200, 50);
    
    // In offscreen mode, create connection programmatically
    // This is more reliable than simulating port dragging
    auto* outputPort = and1->outputPort(0);
    auto* inputPort = and2->inputPort(0);
    
    // Create connection using scene's connection methods
    auto* conn = new QNEConnection();
    scene->addItem(conn);
    conn->setStartPort(outputPort);
    conn->setEndPort(inputPort);
    
    // Verify connection
    QVERIFY(outputPort->isConnected(inputPort));
    QVERIFY(inputPort->isConnected(outputPort));
    QCOMPARE(outputPort->connections().size(), 1);
    QCOMPARE(inputPort->connections().size(), 1);
}