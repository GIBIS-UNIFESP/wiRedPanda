// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testqneconnection_safe.h"

#include "qneconnection.h"
#include "qneport.h" 
#include "and.h"
#include "or.h"
#include "enums.h"

#include <QTest>
#include <QGraphicsScene>
#include <QDataStream>
#include <QBuffer>

void TestQNEConnectionSafe::testBasicConstruction()
{
    // Test basic construction without ports
    QNEConnection connection;
    
    QCOMPARE(connection.type(), QNEConnection::Type);
    QVERIFY(connection.startPort() == nullptr);
    QVERIFY(connection.endPort() == nullptr);
    QCOMPARE(connection.status(), Status::Invalid);
    QVERIFY(!connection.highLight());
}

void TestQNEConnectionSafe::testPositionsAndPath()
{
    QNEConnection connection;
    
    // Test position setting
    QPointF startPos(10.0, 20.0);
    QPointF endPos(100.0, 150.0);
    
    connection.setStartPos(startPos);
    connection.setEndPos(endPos);
    connection.updatePath();
    
    // Path should be created
    QVERIFY(!connection.path().isEmpty());
    
    // Test bounding rect
    QRectF boundingRect = connection.boundingRect();
    QVERIFY(boundingRect.width() > 0);
    QVERIFY(boundingRect.height() > 0);
}

void TestQNEConnectionSafe::testStatusManagement()
{
    QNEConnection connection;
    
    // Test initial status
    QCOMPARE(connection.status(), Status::Invalid);
    
    // Test status changes
    connection.setStatus(Status::Active);
    QCOMPARE(connection.status(), Status::Active);
    
    connection.setStatus(Status::Inactive);
    QCOMPARE(connection.status(), Status::Inactive);
    
    connection.setStatus(Status::Invalid);
    QCOMPARE(connection.status(), Status::Invalid);
}

void TestQNEConnectionSafe::testVisualProperties()
{
    QNEConnection connection;
    
    // Test highlighting
    QVERIFY(!connection.highLight());
    
    connection.setHighLight(true);
    QVERIFY(connection.highLight());
    
    connection.setHighLight(false);
    QVERIFY(!connection.highLight());
    
    // Test theme updates (should not crash)
    connection.updateTheme();
    QVERIFY(true); // Test passes if no crash
}

void TestQNEConnectionSafe::testSerializationBasics()
{
    QNEConnection connection;
    
    // Test basic serialization without ports
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QDataStream stream(&buffer);
    
    connection.save(stream);
    
    // Should have written data
    QVERIFY(buffer.data().size() > 0);
    
    // Test deserialization  
    buffer.close();
    buffer.open(QIODevice::ReadOnly);
    QDataStream inStream(&buffer);
    
    QNEConnection newConnection;
    QMap<quint64, QNEPort *> emptyPortMap;
    newConnection.load(inStream, emptyPortMap);
    
    // Should complete without crashing
    QVERIFY(true);
}

void TestQNEConnectionSafe::testGraphicsItemProperties()
{
    QNEConnection connection;
    
    // Test type identification
    QCOMPARE(connection.type(), QNEConnection::Type);
    
    // Test graphics properties
    QVERIFY(connection.flags() & QGraphicsItem::ItemIsSelectable);
    QCOMPARE(connection.zValue(), -1.0);
    
    // Test selection
    connection.setSelected(true);
    QVERIFY(connection.isSelected());
    
    connection.setSelected(false);
    QVERIFY(!connection.isSelected());
}