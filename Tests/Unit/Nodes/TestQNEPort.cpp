// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Nodes/TestQNEPort.h"

#include "App/Nodes/QNEPort.h"
#include "Tests/Common/TestUtils.h"

void TestQNEPort::testPortTooltip()
{
    // QNEPort is abstract — use QNEInputPort
    QNEInputPort port;
    port.setToolTip("Test tooltip");
    QCOMPARE(port.toolTip(), QString("Test tooltip"));
}

void TestQNEPort::testPortHoverState()
{
    QNEInputPort port;
    QVERIFY(port.isInput());
    QVERIFY(!port.isOutput());
}

