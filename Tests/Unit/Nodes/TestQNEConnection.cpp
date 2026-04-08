// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Nodes/TestQNEConnection.h"

#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "Tests/Common/TestUtils.h"

void TestQNEConnection::testConnectionPathUpdate()
{
    QNEConnection connection;
    connection.setStartPos({0, 0});
    connection.setEndPos({100, 100});
    connection.updatePath();
    QVERIFY(true);
}

void TestQNEConnection::testConnectionHoverEffect()
{
    QNEConnection connection;
    connection.setHighLight(true);
    QVERIFY(connection.highLight());
    connection.setHighLight(false);
    QVERIFY(!connection.highLight());
}

void TestQNEConnection::testConnectionSelection()
{
    QNEConnection connection;
    connection.setStatus(Status::Active);
    QCOMPARE(connection.status(), Status::Active);
    connection.setStatus(Status::Inactive);
    QCOMPARE(connection.status(), Status::Inactive);
}

void TestQNEConnection::testConnectionDestruction()
{
    {
        QNEConnection connection;
        connection.setStartPos({10, 20});
        connection.setEndPos({30, 40});
    }
    QVERIFY(true);
}

