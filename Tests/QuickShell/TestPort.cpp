// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestPort.h"

#include <algorithm>

#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/Wiring/Port.h"

void TestPort::testPortHoverState()
{
    InputPort port;
    QVERIFY(port.isInput());
    QVERIFY(!port.isOutput());
}

void TestPort::testSetInputsReindexesPorts()
{
    // Regression test (F18): Display7's legacy pin remap permutes the input
    // vector via setInputs(). Connection::save() derives connection serial
    // IDs from port->index() while GraphicElement::save() uses vector
    // positions — stale indices after the permutation cross-wired the display
    // pins on the next save/load round trip. setInputs() must restore the
    // index() == position invariant for any permutation.
    Display7 display;

    QVector<InputPort *> reversed = display.inputs();
    std::reverse(reversed.begin(), reversed.end());
    display.setInputs(reversed);

    for (int i = 0; i < display.inputSize(); ++i) {
        QCOMPARE(display.inputPort(i)->index(), i);
    }
}

void TestPort::testPortCurrentPenTracksStatusColor()
{
    // updateTheme() computes currentPen() unconditionally now that Port is a plain class with
    // no QGraphicsScene attachment concept -- this must reflect the correct colour for every
    // status. OutputPort is always valid regardless of connection state, so its status can be
    // driven directly.
    const auto &theme = ThemeManager::attributes();
    OutputPort port;

    port.setStatus(Status::Active);
    QCOMPARE(port.currentPen().color(), theme.m_portActivePen);

    port.setStatus(Status::Inactive);
    QCOMPARE(port.currentPen().color(), theme.m_portInactivePen);

    port.setStatus(Status::Error);
    QCOMPARE(port.currentPen().color(), theme.m_portErrorPen);

    port.setStatus(Status::Unknown);
    QCOMPARE(port.currentPen().color(), theme.m_portUnknownPen);
}
