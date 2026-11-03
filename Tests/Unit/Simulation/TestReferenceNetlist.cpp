// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestReferenceNetlist.h"

#include <functional>
#include <memory>

#include <QFile>
#include <QTemporaryDir>
#include <QTest>

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/IC.h"
#include "App/Element/WirelessNode.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/PandaFile.h"
#include "Tests/Common/RandomCircuit.h"
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

/// Saves the circuit \a build makes as the IC file \a name in \a dir, and returns its path.
QString saveDefinition(const QString &dir, const QString &name, const std::function<void(CircuitBuilder &)> &build)
{
    WorkSpace workspace;
    workspace.scene()->setContextDir(dir);
    CircuitBuilder builder(workspace.scene());
    build(builder);
    const QString path = dir + '/' + name + ".panda";
    workspace.save(path);
    return path;
}

/// Adds to \a builder an instance of the IC file \a path, loaded from \a dir.
IC *instance(CircuitBuilder &builder, const QString &path, const QString &dir)
{
    auto *ic = new IC();
    ic->loadFile(path, dir);
    builder.addElement(ic);
    return ic;
}

/// Two switches into an And, its output into an LED: the definition the IC tests instance.
void buildAndBlock(CircuitBuilder &builder)
{
    // The first switch lies below the second, so the pin order, which is by position, puts the second switch first
    auto *first = make(ElementType::InputSwitch);
    first->setPos(QPointF(0, 100));
    auto *second = make(ElementType::InputSwitch);
    second->setPos(QPointF(0, 0));
    auto *gate = make(ElementType::And);
    auto *led = make(ElementType::Led);
    builder.add(first, second, gate, led);
    builder.connect(first, 0, gate, 0);
    builder.connect(second, 0, gate, 1);
    builder.connect(gate, 0, led, 0);
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

void TestReferenceNetlist::testIcOutputReadsItsInsideDriver()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = saveDefinition(dir.path(), QStringLiteral("and-block"), buildAndBlock);

    auto workspace = TestUtils::createWorkspace();
    CircuitBuilder builder(workspace->scene());
    auto *first = make(ElementType::InputSwitch);
    auto *second = make(ElementType::InputSwitch);
    auto *ic = instance(builder, path, dir.path());
    auto *out = make(ElementType::Led);
    builder.add(first, second, out);
    builder.connect(first, 0, ic, 0);
    builder.connect(second, 0, ic, 1);
    builder.connect(ic, 0, out, 0);

    const ReferenceNetlist netlist(workspace->scene());
    const NetDriver driver = netlist.driver(out, 0);
    QVERIFY(driver.kind == NetKind::Slot);
    // The driver is the And inside the IC, not any element of the instance.
    QVERIFY(driver.element != ic);
    QVERIFY(driver.element->elementType() == ElementType::And);
    // The IC's pin 0 is the switch inside it that lies higher, the second, so the And's input 0, which is the first
    // switch inside, reads the outside switch on pin 1, which is the second: the pins cross
    const NetDriver inputZero = netlist.driver(driver.element, 0);
    QVERIFY(inputZero.kind == NetKind::Slot);
    QCOMPARE(inputZero.element, second);
    const NetDriver inputOne = netlist.driver(driver.element, 1);
    QVERIFY(inputOne.kind == NetKind::Slot);
    QCOMPARE(inputOne.element, first);
    QCOMPARE(netlist.errorNets(), 0);
}

void TestReferenceNetlist::testIcInputsReadTheOutsideDrivers()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = saveDefinition(dir.path(), QStringLiteral("and-block"), buildAndBlock);

    auto workspace = TestUtils::createWorkspace();
    CircuitBuilder builder(workspace->scene());
    auto *first = make(ElementType::InputSwitch);
    auto *ic = instance(builder, path, dir.path());
    auto *out = make(ElementType::Led);
    builder.add(first, out);
    builder.connect(first, 0, ic, 0);
    builder.connect(ic, 0, out, 0);

    // Pin 1 is left open. Pin 1 is the first switch inside, which feeds the And's input 0, so that input reads the
    // default through the IC; the outside switch on pin 0 feeds the And's input 1.
    const ReferenceNetlist netlist(workspace->scene());
    const NetDriver driver = netlist.driver(out, 0);
    QVERIFY(driver.kind == NetKind::Slot);
    const NetDriver open = netlist.driver(driver.element, 0);
    QVERIFY(open.kind == NetKind::Default);
    QCOMPARE(open.status, ic->inputPort(1)->defaultValue());
    const NetDriver wired = netlist.driver(driver.element, 1);
    QVERIFY(wired.kind == NetKind::Slot);
    QCOMPARE(wired.element, first);
    QCOMPARE(netlist.errorNets(), 0);
}

void TestReferenceNetlist::testWirelessIsPerScope()
{
    // The block sends its input on channel "a" and receives it back on "a" inside itself: its LED reads its own input.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = saveDefinition(dir.path(), QStringLiteral("channel-block"), [](CircuitBuilder &builder) {
        auto *input = make(ElementType::InputSwitch);
        auto *transmitter = node(WirelessMode::Tx, QStringLiteral("a"));
        auto *receiver = node(WirelessMode::Rx, QStringLiteral("a"));
        auto *led = make(ElementType::Led);
        builder.add(input, transmitter, receiver, led);
        builder.connect(input, 0, transmitter, 0);
        builder.connect(receiver, 0, led, 0);
    });

    auto workspace = TestUtils::createWorkspace();
    CircuitBuilder builder(workspace->scene());
    auto *outer = make(ElementType::InputSwitch);
    auto *outerTransmitter = node(WirelessMode::Tx, QStringLiteral("a"));
    auto *ic = instance(builder, path, dir.path());
    auto *out = make(ElementType::Led);
    builder.add(outer, outerTransmitter, out);
    builder.connect(outer, 0, ic, 0);
    builder.connect(outer, 0, outerTransmitter, 0);
    builder.connect(ic, 0, out, 0);

    // The outer transmitter on "a" is not a second driver of the inside receiver's channel: each scope has its own.
    const ReferenceNetlist netlist(workspace->scene());
    const NetDriver driver = netlist.driver(out, 0);
    QVERIFY(driver.kind == NetKind::Slot);
    QCOMPARE(driver.element, outer);
    QCOMPARE(netlist.errorNets(), 0);
}

void TestReferenceNetlist::testReaderAgreesWithTheLoader()
{
    // The second reader and the app's loader read the same building blocks: each pin count the reader finds is the one
    // the loader gives the IC, on both sides
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QStringList files = buildingBlockFiles(dir.path());
    QVERIFY(!files.isEmpty());
    for (const QString &file : files) {
        QFile input(file);
        QVERIFY(input.open(QIODevice::ReadOnly));
        const auto read = readPandaFile(input.readAll());
        QVERIFY2(read.has_value(), qPrintable(file));

        auto ic = std::make_unique<IC>();
        ic->loadFile(file, dir.path());
        QCOMPARE(read->pinOrder(true).size(), static_cast<qsizetype>(ic->inputSize()));
        QCOMPARE(read->pinOrder(false).size(), static_cast<qsizetype>(ic->outputSize()));
    }
}

void TestReferenceNetlist::testPinOrderFallsBackToPositions()
{
    // A file without a saved order gives its pins by position: the switch that lies higher is the first pin
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = saveDefinition(dir.path(), QStringLiteral("and-block"), buildAndBlock);
    QFile input(path);
    QVERIFY(input.open(QIODevice::ReadOnly));
    auto read = readPandaFile(input.readAll());
    QVERIFY(read.has_value());

    read->metadata.remove(QStringLiteral("inputOrder"));
    const QVector<quint64> order = read->pinOrder(true);
    QCOMPARE(order.size(), 2);
    const PandaElement *higher = nullptr;
    for (const PandaElement &element : read->elements) {
        if ((element.type == ElementType::InputSwitch) && (element.pos.y() == 0)) {
            higher = &element;
        }
    }
    QVERIFY(higher);
    QCOMPARE(order.first(), higher->outputKeys.first());
}
