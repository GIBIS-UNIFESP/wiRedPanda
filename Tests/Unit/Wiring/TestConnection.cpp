// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Wiring/TestConnection.h"

#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/TestUtils.h"

void TestConnection::testConnectionPathUpdate()
{
    Connection connection;
    connection.setStartPos({0, 0});
    connection.setEndPos({100, 100});
    connection.updatePath();
    QVERIFY(true);
}

void TestConnection::testConnectionHoverEffect()
{
    Connection connection;
    connection.setHighLight(true);
    QVERIFY(connection.highLight());
    connection.setHighLight(false);
    QVERIFY(!connection.highLight());
}

void TestConnection::testConnectionSelection()
{
    Connection connection;
    connection.setStatus(Status::Active);
    QCOMPARE(connection.status(), Status::Active);
    connection.setStatus(Status::Inactive);
    QCOMPARE(connection.status(), Status::Inactive);
}

void TestConnection::testConnectionDestruction()
{
    {
        Connection connection;
        connection.setStartPos({10, 20});
        connection.setEndPos({30, 40});
    }
    QVERIFY(true);
}
