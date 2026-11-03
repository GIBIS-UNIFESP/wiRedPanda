// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestReferenceNetlist.h"

#include <QTest>

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/WirelessNode.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/ReferenceNetlist.h"
#include "Tests/Common/TestUtils.h"

namespace {

Element *make(const ElementType type)
{
    return ElementFactory::buildElement(type);
}

Element *node(const WirelessMode mode, const QString &label)
{
    auto *element = make(ElementType::Node);
    WirelessNode::setMode(*element, mode);
    element->setLabel(label);
    return element;
}

} // namespace

void TestReferenceNetlist::testJunctionChainReadsTheSource()
{
    auto workspace = TestUtils::createWorkspace();
    CircuitBuilder builder(workspace->scene());
    auto *input = make(ElementType::InputSwitch);
    auto *first = node(WirelessMode::None, {});
    auto *second = node(WirelessMode::None, {});
    auto *led = make(ElementType::Led);
    builder.add(input, first, second, led);
    builder.connect(input, 0, first, 0);
    builder.connect(first, 0, second, 0);
    builder.connect(second, 0, led, 0);

    const ReferenceNetlist netlist(workspace->scene());
    const NetDriver driver = netlist.driver(led, 0);
    QVERIFY(driver.kind == NetKind::Slot);
    QCOMPARE(driver.element, input);
    QCOMPARE(driver.port, 0);
    QCOMPARE(netlist.errorNets(), 0);
}

void TestReferenceNetlist::testUnconnectedInputReadsTheDefault()
{
    auto workspace = TestUtils::createWorkspace();
    CircuitBuilder builder(workspace->scene());
    auto *input = make(ElementType::InputSwitch);
    auto *gate = make(ElementType::And);
    builder.add(input, gate);
    builder.connect(input, 0, gate, 0);

    const ReferenceNetlist netlist(workspace->scene());
    QVERIFY(netlist.driver(gate, 0).kind == NetKind::Slot);
    const NetDriver open = netlist.driver(gate, 1);
    QVERIFY(open.kind == NetKind::Default);
    QCOMPARE(open.status, gate->inputPort(1)->defaultValue());
    QCOMPARE(netlist.errorNets(), 0);
}

void TestReferenceNetlist::testTwoWiresIntoAnInputReadError()
{
    auto workspace = TestUtils::createWorkspace();
    CircuitBuilder builder(workspace->scene());
    auto *first = make(ElementType::InputSwitch);
    auto *second = make(ElementType::InputSwitch);
    auto *led = make(ElementType::Led);
    builder.add(first, second, led);
    builder.connect(first, 0, led, 0);
    builder.connect(second, 0, led, 0);

    const ReferenceNetlist netlist(workspace->scene());
    QVERIFY(netlist.driver(led, 0).kind == NetKind::Error);
    QCOMPARE(netlist.errorNets(), 1);
}

void TestReferenceNetlist::testReceiverReadsItsTransmitter()
{
    auto workspace = TestUtils::createWorkspace();
    CircuitBuilder builder(workspace->scene());
    auto *input = make(ElementType::InputSwitch);
    auto *transmitter = node(WirelessMode::Tx, QStringLiteral("a"));
    auto *receiver = node(WirelessMode::Rx, QStringLiteral("a"));
    auto *led = make(ElementType::Led);
    builder.add(input, transmitter, receiver, led);
    builder.connect(input, 0, transmitter, 0);
    builder.connect(receiver, 0, led, 0);

    const ReferenceNetlist netlist(workspace->scene());
    const NetDriver driver = netlist.driver(led, 0);
    QVERIFY(driver.kind == NetKind::Slot);
    QCOMPARE(driver.element, input);
    QCOMPARE(netlist.errorNets(), 0);
}

void TestReferenceNetlist::testDuplicatedChannelReadsError()
{
    auto workspace = TestUtils::createWorkspace();
    CircuitBuilder builder(workspace->scene());
    auto *first = make(ElementType::InputSwitch);
    auto *second = make(ElementType::InputSwitch);
    auto *firstTx = node(WirelessMode::Tx, QStringLiteral("a"));
    auto *secondTx = node(WirelessMode::Tx, QStringLiteral("a"));
    auto *receiver = node(WirelessMode::Rx, QStringLiteral("a"));
    auto *led = make(ElementType::Led);
    builder.add(first, second, firstTx, secondTx, receiver, led);
    builder.connect(first, 0, firstTx, 0);
    builder.connect(second, 0, secondTx, 0);
    builder.connect(receiver, 0, led, 0);

    const ReferenceNetlist netlist(workspace->scene());
    QVERIFY(netlist.driver(led, 0).kind == NetKind::Error);
    // Both transmitters' label is one net, warned once however many receivers read it.
    QCOMPARE(netlist.errorNets(), 1);
}

void TestReferenceNetlist::testWiredReceiverReadsError()
{
    auto workspace = TestUtils::createWorkspace();
    CircuitBuilder builder(workspace->scene());
    auto *input = make(ElementType::InputSwitch);
    auto *other = make(ElementType::InputSwitch);
    auto *transmitter = node(WirelessMode::Tx, QStringLiteral("a"));
    auto *receiver = node(WirelessMode::Rx, QStringLiteral("a"));
    auto *led = make(ElementType::Led);
    builder.add(input, other, transmitter, receiver, led);
    builder.connect(input, 0, transmitter, 0);
    builder.connect(other, 0, receiver, 0);
    builder.connect(receiver, 0, led, 0);

    const ReferenceNetlist netlist(workspace->scene());
    QVERIFY(netlist.driver(led, 0).kind == NetKind::Error);
    QCOMPARE(netlist.errorNets(), 1);
}

void TestReferenceNetlist::testAliasLoopReadsUnknown()
{
    auto workspace = TestUtils::createWorkspace();
    CircuitBuilder builder(workspace->scene());
    auto *first = node(WirelessMode::None, {});
    auto *second = node(WirelessMode::None, {});
    auto *led = make(ElementType::Led);
    builder.add(first, second, led);
    builder.connect(first, 0, second, 0);
    builder.connect(second, 0, first, 0);
    builder.connect(second, 0, led, 0);

    const ReferenceNetlist netlist(workspace->scene());
    QVERIFY(netlist.driver(led, 0).kind == NetKind::Unknown);
    QCOMPARE(netlist.errorNets(), 0);
}

void TestReferenceNetlist::testOrderDoesNotChangeTheDrivers()
{
    // The same circuit, its elements added in reverse: each LED reads the same kind of driver either way.
    auto describe = [](const bool reverse) {
        auto workspace = TestUtils::createWorkspace();
        CircuitBuilder builder(workspace->scene());
        auto *input = make(ElementType::InputSwitch);
        auto *transmitter = node(WirelessMode::Tx, QStringLiteral("a"));
        auto *receiver = node(WirelessMode::Rx, QStringLiteral("a"));
        auto *junction = node(WirelessMode::None, {});
        auto *led = make(ElementType::Led);
        auto *other = make(ElementType::Led);
        if (reverse) {
            builder.add(other, led, junction, receiver, transmitter, input);
        } else {
            builder.add(input, transmitter, receiver, junction, led, other);
        }
        builder.connect(input, 0, transmitter, 0);
        builder.connect(receiver, 0, junction, 0);
        builder.connect(junction, 0, led, 0);
        const ReferenceNetlist netlist(workspace->scene());
        QStringList kinds;
        for (Element *element : {led, other}) {
            const NetDriver driver = netlist.driver(element, 0);
            kinds << QString::number(static_cast<int>(driver.kind));
            if (driver.element) {
                kinds << QString::number(driver.element->elementType() == ElementType::InputSwitch ? 1 : 0);
            }
        }
        return kinds.join(',');
    };
    QCOMPARE(describe(true), describe(false));
}
