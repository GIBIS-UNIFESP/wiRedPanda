// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Wiring/TestPort.h"

#include <algorithm>

#include <QGraphicsScene>

#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElements/Display7.h"
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
    // updateTheme() bypasses the item's own setPen() (and the BSP-tree re-index it
    // triggers) for every status, tracking colour via currentPen() instead -- this must
    // still reflect the correct colour for every status. OutputPort is always valid
    // regardless of connection state, so its status can be driven directly.
    //
    // Added to a real QGraphicsScene: Port::updateTheme() now skips its pen/brush work
    // entirely for a sceneless port -- real, always-wasted cost otherwise for CanvasItem's
    // Quick-rendered ports, which are never scene-attached and never painted via
    // QGraphicsView (see project memory project_quick_hotspot_fixes_2_landed.md). A real
    // port's currentPen() is only ever read by paint(), which itself only ever runs on a
    // scene-attached item, so a scene here matches the precondition production always
    // satisfies before this code path runs.
    const auto &theme = ThemeManager::attributes();
    QGraphicsScene scene;
    OutputPort port;
    scene.addItem(&port);

    port.setStatus(Status::Active);
    QCOMPARE(port.currentPen().color(), theme.m_portActivePen);

    port.setStatus(Status::Inactive);
    QCOMPARE(port.currentPen().color(), theme.m_portInactivePen);

    port.setStatus(Status::Error);
    QCOMPARE(port.currentPen().color(), theme.m_portErrorPen);

    port.setStatus(Status::Unknown);
    QCOMPARE(port.currentPen().color(), theme.m_portUnknownPen);
}

void TestPort::testPortSkipsThemeUpdateWhenSceneless()
{
    // The optimization the test above's scene setup exists to accommodate: a port that is
    // NOT in any QGraphicsScene (CanvasItem's real, permanent case -- its ports are never
    // scene-attached) must skip updateTheme()'s pen/brush computation entirely on a status
    // change, not just happen to still work. currentPen() must stay at its default
    // (never computed), proving the early-return actually ran rather than merely being
    // consistent with a different code path reaching the same values by coincidence.
    OutputPort port;
    QVERIFY(!port.scene());
    const QPen before = port.currentPen();

    port.setStatus(Status::Active);
    port.setStatus(Status::Error);
    port.setStatus(Status::Unknown);

    QCOMPARE(port.currentPen(), before);
}
