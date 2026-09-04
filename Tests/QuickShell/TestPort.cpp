// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestPort.h"

#include <algorithm>
#include <memory>

#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

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

void TestPort::testNameGetterSetter()
{
    InputPort port;
    QVERIFY(port.name().isEmpty());

    port.setName(QStringLiteral("CLK"));
    QCOMPARE(port.name(), QStringLiteral("CLK"));
}

void TestPort::testDefaultValueGetterSetter()
{
    InputPort port;
    QCOMPARE(port.defaultValue(), Status::Unknown);

    port.setDefaultStatus(Status::Active);
    QCOMPARE(port.defaultValue(), Status::Active);
}

void TestPort::testIsRequiredGetterSetter()
{
    InputPort port;
    QVERIFY(port.isRequired()); // required by default

    port.setRequired(false);
    QVERIFY(!port.isRequired());

    port.setRequired(true);
    QVERIFY(port.isRequired());
}

void TestPort::testGlobalIndexForInputVsOutputPort()
{
    // For inputs, globalIndex() == index(); for outputs, index() is offset by the element's
    // input count so inputs and outputs share one combined 0..N-1 sequence.
    And andGate; // 2 inputs, 1 output

    QCOMPARE(andGate.inputPort(0)->globalIndex(), 0);
    QCOMPARE(andGate.inputPort(1)->globalIndex(), 1);
    QCOMPARE(andGate.outputPort(0)->globalIndex(), andGate.inputSize());
}

void TestPort::testMakeSerialIdPacksElementBaseAndGlobalIndex()
{
    QCOMPARE(Port::makeSerialId(7, 3), (quint64(7) << 16) | quint64(3));
    QCOMPARE(Port::makeSerialId(0, 0), quint64(0));
    // globalIndex is masked to 16 bits -- verify the mask, not just a value under it.
    QCOMPARE(Port::makeSerialId(1, 0x1FFFF), (quint64(1) << 16) | quint64(0xFFFF));
}

void TestPort::testSetGraphicElementBindsAccessor()
{
    And andGate;
    InputPort port;
    QVERIFY(port.graphicElement() == nullptr);

    port.setGraphicElement(&andGate);
    QCOMPARE(port.graphicElement(), &andGate);
}

void TestPort::testAttachConnectionIsIdempotent()
{
    // Connection::setStartPort() already calls attachConnection() internally; calling it again
    // directly (as IC rewiring can) must not duplicate the entry in connections().
    And andGate;
    auto *output = andGate.outputPort();

    auto conn = std::make_unique<Connection>();
    conn->setStartPort(output);
    QCOMPARE(output->connections().size(), 1);

    output->attachConnection(conn.get());
    QCOMPARE(output->connections().size(), 1);

    conn->setStartPort(nullptr);
}

void TestPort::testDetachConnectionRemovesFromListAndClearsPortReference()
{
    And andGate;
    auto *output = andGate.outputPort();

    auto conn = std::make_unique<Connection>();
    conn->setStartPort(output);
    QCOMPARE(output->connections().size(), 1);

    output->detachConnection(conn.get());

    QCOMPARE(output->connections().size(), 0);
    QVERIFY2(conn->startPort() == nullptr, "detachConnection() must null out the connection's own reference back to this port");
}

void TestPort::testIsConnectedTrueOnlyForPortsSharingAConnection()
{
    And and1;
    And and2;
    and2.setPos(150, 0);

    auto conn = std::make_unique<Connection>();
    conn->setStartPort(and1.outputPort());
    conn->setEndPort(and2.inputPort(0));

    QVERIFY(and1.outputPort()->isConnected(and2.inputPort(0)));
    QVERIFY(and2.inputPort(0)->isConnected(and1.outputPort()));
    QVERIFY2(!and1.outputPort()->isConnected(and2.inputPort(1)), "an unconnected port pair must not report as connected");

    conn->setStartPort(nullptr);
    conn->setEndPort(nullptr);
}

void TestPort::testInputPortIsValidRequiredVsOptionalUnconnected()
{
    And andGate;
    auto *input = andGate.inputPort(0);

    input->setRequired(true);
    QVERIFY2(!input->isValid(), "a required, unconnected input must be invalid");

    input->setRequired(false);
    QVERIFY2(input->isValid(), "an optional, unconnected input must be valid");

    auto *sourceGate = new And();
    auto conn = std::make_unique<Connection>();
    conn->setStartPort(sourceGate->outputPort());
    conn->setEndPort(input);
    QVERIFY2(input->isValid(), "an input with exactly one connection must be valid regardless of requiredness");

    conn->setStartPort(nullptr);
    conn->setEndPort(nullptr);
    delete sourceGate;
}

void TestPort::testOutputPortIsValidAlwaysTrueRegardlessOfConnections()
{
    And andGate;
    QVERIFY2(andGate.outputPort()->isValid(), "an unconnected output must still be valid -- fan-out has no connectivity constraint");

    And downstream;
    downstream.setPos(150, 0);
    auto conn = std::make_unique<Connection>();
    conn->setStartPort(andGate.outputPort());
    conn->setEndPort(downstream.inputPort(0));
    QVERIFY(andGate.outputPort()->isValid());

    conn->setStartPort(nullptr);
    conn->setEndPort(nullptr);
}

void TestPort::testHoverEnterAndLeaveChangeBrush()
{
    OutputPort port;
    const QBrush statusBrush(Qt::red);
    port.setCurrentBrush(statusBrush);
    QCOMPARE(port.brush(), statusBrush);

    port.hoverEnter();
    QCOMPARE(port.brush().color(), ThemeManager::attributes().m_portHoverPort);

    port.hoverLeave();
    QCOMPARE(port.brush(), statusBrush);
}

void TestPort::testOutputPortSetStatusMarksElementRenderDirty()
{
    // OutputPort::isValid() is always true, so setStatus()'s guard is the only gate: a
    // genuine value change must reach GraphicElement::markRenderDirty().
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
    // Regression for the isValid()-override edge case: a required, unconnected InputPort
    // already shows Error (see InputPort's constructor). Requesting a different status still
    // passes setStatus()'s "status == m_status" guard (it compares the *requested* value
    // against the *current* one), but isValid()'s override forces it right back to Error --
    // the final value never actually changes, so this must NOT mark the element render-dirty.
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
