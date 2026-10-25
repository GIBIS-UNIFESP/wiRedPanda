// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Wiring/TestPort.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include <QElapsedTimer>
#include <QList>
#include <QString>

#include "App/Core/ThemeManager.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/Display7.h"
#include "App/Wiring/Connection.h"
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

void TestPort::testDefaultSetWhileConnectedLeavesTheInputOnItsWire()
{
    // ~Preset is optional with an Active default; its driver reads Inactive.
    And driver;
    DFlipFlop flipFlop;
    flipFlop.setPos(150, 0);
    auto *preset = flipFlop.inputPort(2);
    auto conn = std::make_unique<Connection>();
    conn->setStartPort(driver.outputPort());
    conn->setEndPort(preset);
    driver.outputPort()->setStatus(Status::Inactive);
    QCOMPARE(preset->status(), Status::Inactive);

    // An element sets its ports' defaults each time it lays them out; a connected input stays on its wire.
    preset->setDefaultStatus(Status::Active);
    QCOMPARE(preset->defaultValue(), Status::Active);
    QCOMPARE(preset->status(), Status::Inactive);

    // It takes its default when the wire goes.
    conn->setEndPort(nullptr);
    QCOMPARE(preset->status(), Status::Active);
    conn->setStartPort(nullptr);
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

void TestPort::testFanOutGrowsLinearly()
{
    // Wiring one output to n inputs, moving those wires to a second output and deleting them, what an IC's reload does
    // with a clock or bus that drives thousands of gates inside it, must stay linear in n. The same wires spread over
    // n outputs, one each, do the same work with lists of one, so the fan-out is timed against them: how that ratio
    // grows from 250 wires to 64,000, 0.76 to 1.86 for linear attaching and detaching with eight copies of the test
    // running at once, and about 24 for a quadratic one. The sizes are far apart so that a loaded runner's noise stays
    // well under the quadratic growth. Ratios, not times, each the best of nine runs, the fan-out and the spread timed
    // in turn within each run, so that a load which comes and goes falls on both.
    const auto timeOnce = [](const int n, const bool fanOut) {
        const int sourceCount = fanOut ? 1 : n;
        std::vector<std::unique_ptr<OutputPort>> sources;
        std::vector<std::unique_ptr<OutputPort>> targets;
        std::vector<std::unique_ptr<InputPort>> inputs;
        for (int i = 0; i < sourceCount; ++i) {
            sources.push_back(std::make_unique<OutputPort>());
            targets.push_back(std::make_unique<OutputPort>());
        }
        for (int i = 0; i < n; ++i) {
            inputs.push_back(std::make_unique<InputPort>());
        }
        QList<Connection *> conns;
        conns.reserve(n);

        QElapsedTimer timer;
        timer.start();
        for (int i = 0; i < n; ++i) {
            auto *conn = new Connection();
            conn->setStartPort(sources[static_cast<std::size_t>(fanOut ? 0 : i)].get());
            conn->setEndPort(inputs[static_cast<std::size_t>(i)].get());
            conns.append(conn);
        }
        for (int i = 0; i < n; ++i) {
            conns[i]->setStartPort(targets[static_cast<std::size_t>(fanOut ? 0 : i)].get());
        }
        const bool moved = (targets.front()->connections().size() == (fanOut ? n : 1));
        qDeleteAll(conns);
        const qint64 elapsed = timer.nsecsElapsed();
        return (moved && targets.front()->connections().isEmpty()) ? elapsed : qint64(0);
    };
    // The best fan-out and the best spread of n wires, timed in turn
    const auto bestNs = [&timeOnce](const int n) {
        std::pair<qint64, qint64> best{std::numeric_limits<qint64>::max(), std::numeric_limits<qint64>::max()};
        for (int run = 0; run < 9; ++run) {
            const qint64 fanOut = timeOnce(n, true);
            const qint64 spread = timeOnce(n, false);
            if ((fanOut == 0) || (spread == 0)) {
                return std::pair<qint64, qint64>{0, 0};
            }
            best = {std::min(best.first, fanOut), std::min(best.second, spread)};
        }
        return best;
    };

    const auto [smallFanOut, smallSpread] = bestNs(250);
    const auto [largeFanOut, largeSpread] = bestNs(64000);
    QVERIFY2((smallFanOut > 0) && (largeFanOut > 0) && (smallSpread > 0) && (largeSpread > 0),
             "a port lost or kept a wire");
    const double growth = (static_cast<double>(largeFanOut) / static_cast<double>(largeSpread))
                        / (static_cast<double>(smallFanOut) / static_cast<double>(smallSpread));
    QVERIFY2(growth < 6.0, qPrintable(QString("against the same wires one per port, one port's wires took %1 times as "
                                              "long at 256 times the wires").arg(growth)));
}
