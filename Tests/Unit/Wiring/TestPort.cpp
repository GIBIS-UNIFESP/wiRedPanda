// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Wiring/TestPort.h"

#include "App/Wiring/Port.h"
#include "Tests/Common/TestUtils.h"

void TestPort::testPortTooltip()
{
    // Port is abstract — use InputPort
    InputPort port;
    port.setToolTip("Test tooltip");
    QCOMPARE(port.toolTip(), QString("Test tooltip"));
}

void TestPort::testPortHoverState()
{
    InputPort port;
    QVERIFY(port.isInput());
    QVERIFY(!port.isOutput());
}
