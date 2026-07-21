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

void TestPort::testOutputPortSetStatusMarksElementRenderDirty()
{
    // OutputPort::isValid() is always true, so setStatus()'s guard is the only gate:
    // a genuine value change must reach GraphicElement::markRenderDirty().
    Display7 display;
    OutputPort port;
    port.setGraphicElement(&display);

    display.clearRenderDirty();
    QVERIFY(!display.isRenderDirty());

    port.setStatus(Status::Active);
    QVERIFY(display.isRenderDirty());
}

void TestPort::testOutputPortSetStatusNoOpDoesNotMarkElementRenderDirty()
{
    // setStatus()'s early-return guard (status == m_status) must prevent a spurious
    // markRenderDirty() call on the very common no-op repaint case.
    Display7 display;
    OutputPort port;
    port.setGraphicElement(&display);
    port.setStatus(Status::Active);

    display.clearRenderDirty();
    QVERIFY(!display.isRenderDirty());

    port.setStatus(Status::Active);
    QVERIFY(!display.isRenderDirty());
}

void TestPort::testInputPortSetStatusMarksElementRenderDirty()
{
    // An optional, unconnected InputPort is valid, so setStatus()'s isValid() override
    // doesn't kick in -- a requested change is a real change.
    Display7 display;
    InputPort port;
    port.setGraphicElement(&display);
    port.setRequired(false);

    display.clearRenderDirty();
    QVERIFY(!display.isRenderDirty());

    port.setStatus(Status::Active);
    QCOMPARE(port.status(), Status::Active);
    QVERIFY(display.isRenderDirty());
}

void TestPort::testInputPortInvalidOverrideNoOpDoesNotMarkElementRenderDirty()
{
    // Regression for the isValid()-override edge case found while implementing this: a
    // required, unconnected InputPort already shows Error (see InputPort's constructor).
    // Requesting a different status still passes setStatus()'s "status == m_status" guard,
    // but isValid()'s override forces it right back to Error -- the final value never
    // actually changes, so this must NOT mark the element render-dirty.
    Display7 display;
    InputPort port;
    port.setGraphicElement(&display);
    QCOMPARE(port.status(), Status::Error);

    display.clearRenderDirty();
    QVERIFY(!display.isRenderDirty());

    port.setStatus(Status::Active);
    QCOMPARE(port.status(), Status::Error);
    QVERIFY(!display.isRenderDirty());
}
