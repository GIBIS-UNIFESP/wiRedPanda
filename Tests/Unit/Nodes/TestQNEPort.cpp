// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Nodes/TestQNEPort.h"

#include <algorithm>

#include "App/Element/GraphicElements/Display7.h"
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

void TestQNEPort::testSetInputsReindexesPorts()
{
    // Regression test (F18): Display7's legacy pin remap permutes the input
    // vector via setInputs(). QNEConnection::save() derives connection serial
    // IDs from port->index() while GraphicElement::save() uses vector
    // positions — stale indices after the permutation cross-wired the display
    // pins on the next save/load round trip. setInputs() must restore the
    // index() == position invariant for any permutation.
    Display7 display;

    QVector<QNEInputPort *> reversed = display.inputs();
    std::reverse(reversed.begin(), reversed.end());
    display.setInputs(reversed);

    for (int i = 0; i < display.inputSize(); ++i) {
        QCOMPARE(display.inputPort(i)->index(), i);
    }
}
