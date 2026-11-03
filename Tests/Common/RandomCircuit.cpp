// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Common/RandomCircuit.h"

#include <algorithm>
#include <array>
#include <functional>

#include <QBitArray>
#include <QDir>
#include <QFileInfo>

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/IC.h"
#include "App/Element/Parameters.h"
#include "App/Element/WirelessNode.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/Goldens.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Common/TraceEngine.h"

namespace {

/// The most inputs a gate, a memory node or a multiplexer takes: a Mux's eleven, the widest node the spec allows.
constexpr int kMaxNodeInputs = 11;

/// The round budget of an instant (§2.9): a ripple chain is longer than it by one to four stages.
constexpr int kRoundBudget = 1000;

/// The chance, in percent, that an input is left unwired.
constexpr int kOpenInputPercent = 8;

/// The chance, in percent, that an input gets a second wire, a net with two drivers.
constexpr int kDoubleDrivenPercent = 4;

/// The chance, in percent, that a clock is locked.
constexpr int kLockedClockPercent = 20;

/// The chance, in percent, that a logic element has no delay in temporal mode; otherwise it has one of 1 to 30 ns.
constexpr int kZeroDelayPercent = 50;
constexpr int kMaxDelayNs = 30;

/// The chance, in percent, that an input changes level at a step of a walk.
constexpr int kInputChangePercent = 30;

/// Where a memory node's clock comes from: its own output 15% of the time, an input 40%, a clock the rest.
constexpr int kOwnOutputClockPercent = 15;
constexpr int kInputClockPercent = 40;

/// The chance, in percent, that a receiver takes a wire, which the editor refuses; most receivers stay unwired.
constexpr int kWiredReceiverPercent = 10;

/// The chance, in percent, that a circuit carries a chain of toggle flip-flops past the round budget.
constexpr int kRippleChainPercent = 2;

/// The number of wireless labels the nodes draw from.
constexpr int kWirelessLabels = 3;

constexpr std::array<ElementType, 6> kSourceTypes{ElementType::InputSwitch, ElementType::InputButton,
                                                  ElementType::InputRotary, ElementType::InputVcc,
                                                  ElementType::InputGnd, ElementType::Clock};

constexpr std::array<ElementType, 16> kLogicTypes{
    ElementType::And,        ElementType::Or,       ElementType::Not,     ElementType::Nand,
    ElementType::Nor,        ElementType::Xor,      ElementType::Xnor,    ElementType::DFlipFlop,
    ElementType::JKFlipFlop, ElementType::TFlipFlop, ElementType::SRFlipFlop, ElementType::DLatch,
    ElementType::SRLatch,    ElementType::Mux,      ElementType::Demux,   ElementType::TruthTable};

constexpr std::array<double, 8> kClockFrequencies{1, 2, 5, 10, 50, 100, 500, 1000};

/// One of \a options, chosen at random.
template <typename T, std::size_t N>
T pick(ChoiceSource &choices, const std::array<T, N> &options)
{
    return options[static_cast<std::size_t>(choices.below(static_cast<int>(N)))];
}

/// The status a walk prints for a value: 0 and 1 for the levels, x for Unknown, e for Error.
QChar statusChar(const Status status)
{
    switch (status) {
    case Status::Inactive:
        return '0';
    case Status::Active:
        return '1';
    case Status::Unknown:
        return 'x';
    case Status::Error:
        return 'e';
    }
    return '?';
}

bool isMemory(const Element *element)
{
    switch (element->elementType()) {
    case ElementType::DFlipFlop:
    case ElementType::JKFlipFlop:
    case ElementType::TFlipFlop:
    case ElementType::SRFlipFlop:
    case ElementType::DLatch:
    case ElementType::SRLatch:
        return true;
    default:
        return false;
    }
}

/// Whether \a port of \a consumer is a memory node's clock, named Clock, or Enable for a latch.
bool isMemoryClock(const Element *consumer, const int port)
{
    if (!isMemory(consumer)) {
        return false;
    }
    const QString name = consumer->inputPort(port)->name();
    return (name == QStringLiteral("Clock")) || (name == QStringLiteral("Enable"));
}

bool isReceiver(const Element *element)
{
    return (element->elementType() == ElementType::Node) && (WirelessNode::mode(*element) == WirelessMode::Rx);
}

/// Wires \a consumer's input \a port to a random output of a random driver, and counts a feedback wire.
void wireRandomDriver(CircuitBuilder &builder, const QList<Element *> &drivers,
                      const QHash<const Element *, int> &position, Element *consumer, const int port,
                      ChoiceSource &choices, RandomCircuitFeatures &features)
{
    Element *driver = drivers[choices.below(static_cast<int>(drivers.size()))];
    builder.connect(driver, choices.below(driver->outputSize()), consumer, port);
    if (position.value(driver) >= position.value(consumer)) {
        features.feedbackWires++;
    }
}

/// Wires a memory node's clock port, from its own output, an input or a clock, in the shares of §4.3.
void wireMemoryClock(CircuitBuilder &builder, RandomCircuit &circuit, const QList<Element *> &drivers,
                     Element *consumer, const int port, ChoiceSource &choices)
{
    RandomCircuitFeatures &features = circuit.features;
    const int roll = choices.below(100);
    if (roll < kOwnOutputClockPercent) {
        builder.connect(consumer, 0, consumer, port);
        features.memoryClockOwn++;
        features.feedbackWires++;
    } else if ((roll < kOwnOutputClockPercent + kInputClockPercent) || circuit.clocks.isEmpty()) {
        const QList<Element *> &sources = circuit.inputs.isEmpty() ? drivers : circuit.inputs;
        Element *source = sources[choices.below(static_cast<int>(sources.size()))];
        builder.connect(source, choices.below(source->outputSize()), consumer, port);
        features.memoryClockInput++;
    } else {
        Element *clock = circuit.clocks[choices.below(static_cast<int>(circuit.clocks.size()))];
        builder.connect(clock, 0, consumer, port);
        features.memoryClockClock++;
    }
}

/// Writes \a name's circuit, built by \a build, into \a dir as a .panda file, unless the file is already there.
QString writeBlock(const QString &dir, const QString &name,
                   const std::function<void(CircuitBuilder &, const QString &)> &build)
{
    const QString path = dir + '/' + name + ".panda";
    if (QFileInfo::exists(path)) {
        return path;
    }
    WorkSpace workspace;
    workspace.scene()->setContextDir(dir);
    CircuitBuilder builder(workspace.scene());
    build(builder, dir);
    workspace.save(path);
    return path;
}

Element *make(const ElementType type)
{
    return ElementFactory::buildElement(type);
}

/// A D-less toggle flip-flop: a switch on T and one on the clock, its output on an LED. The first building block.
void buildToggleFlipFlop(CircuitBuilder &builder, const QString &)
{
    auto *t = make(ElementType::InputSwitch);
    auto *clock = make(ElementType::InputSwitch);
    auto *flipFlop = make(ElementType::TFlipFlop);
    auto *led = make(ElementType::Led);
    builder.add(t, clock, flipFlop, led);
    builder.connect(t, 0, flipFlop, 0);
    builder.connect(clock, 0, flipFlop, 1);
    builder.connect(flipFlop, 0, led, 0);
}

void buildHalfAdder(CircuitBuilder &builder, const QString &)
{
    auto *a = make(ElementType::InputSwitch);
    auto *b = make(ElementType::InputSwitch);
    auto *andGate = make(ElementType::And);
    auto *xorGate = make(ElementType::Xor);
    auto *sum = make(ElementType::Led);
    auto *carry = make(ElementType::Led);
    builder.add(a, b, andGate, xorGate, sum, carry);
    builder.connect(a, 0, andGate, 0);
    builder.connect(b, 0, andGate, 1);
    builder.connect(a, 0, xorGate, 0);
    builder.connect(b, 0, xorGate, 1);
    builder.connect(xorGate, 0, sum, 0);
    builder.connect(andGate, 0, carry, 0);
}

void buildInverter(CircuitBuilder &builder, const QString &)
{
    auto *input = make(ElementType::InputSwitch);
    auto *inverter = make(ElementType::Not);
    auto *led = make(ElementType::Led);
    builder.add(input, inverter, led);
    builder.connect(input, 0, inverter, 0);
    builder.connect(inverter, 0, led, 0);
}

/// Two NANDs cross-coupled, a feedback loop inside the block, with a switch on each set and reset input.
void buildNandLatch(CircuitBuilder &builder, const QString &)
{
    auto *set = make(ElementType::InputSwitch);
    auto *reset = make(ElementType::InputSwitch);
    auto *first = make(ElementType::Nand);
    auto *second = make(ElementType::Nand);
    auto *q = make(ElementType::Led);
    auto *qBar = make(ElementType::Led);
    builder.add(set, reset, first, second, q, qBar);
    builder.connect(set, 0, first, 0);
    builder.connect(second, 0, first, 1);
    builder.connect(reset, 0, second, 0);
    builder.connect(first, 0, second, 1);
    builder.connect(first, 0, q, 0);
    builder.connect(second, 0, qBar, 0);
}

void buildOrOfThree(CircuitBuilder &builder, const QString &)
{
    auto *orGate = make(ElementType::Or);
    TestUtils::applyPortCounts(orGate, 3, 1);
    auto *led = make(ElementType::Led);
    builder.add(orGate, led);
    for (int port = 0; port < orGate->inputSize(); ++port) {
        auto *input = make(ElementType::InputSwitch);
        builder.add(input);
        builder.connect(input, 0, orGate, port);
    }
    builder.connect(orGate, 0, led, 0);
}

/// Two toggle flip-flop blocks, the second clocked by the first, which nests the first block's file.
void buildCounter(CircuitBuilder &builder, const QString &dir)
{
    auto *clock = make(ElementType::InputSwitch);
    auto *vcc = make(ElementType::InputVcc);
    auto *first = new IC();
    first->loadFile(dir + QStringLiteral("/toggle.panda"), dir);
    auto *second = new IC();
    second->loadFile(dir + QStringLiteral("/toggle.panda"), dir);
    auto *led1 = make(ElementType::Led);
    auto *led2 = make(ElementType::Led);
    builder.add(clock, vcc, first, second, led1, led2);
    builder.connect(vcc, 0, first, 0);
    builder.connect(clock, 0, first, 1);
    builder.connect(vcc, 0, second, 0);
    builder.connect(first, 0, second, 1);
    builder.connect(first, 0, led1, 0);
    builder.connect(second, 0, led2, 0);
}

void buildMux(CircuitBuilder &builder, const QString &)
{
    auto *mux = make(ElementType::Mux);
    TestUtils::applyPortCounts(mux, 3, 1);
    auto *led = make(ElementType::Led);
    builder.add(mux, led);
    for (int port = 0; port < mux->inputSize(); ++port) {
        auto *input = make(ElementType::InputSwitch);
        builder.add(input);
        builder.connect(input, 0, mux, port);
    }
    builder.connect(mux, 0, led, 0);
}

/// A transmitter and a receiver on one label: a switch's level crosses the block without a wire.
void buildWirelessChannel(CircuitBuilder &builder, const QString &)
{
    auto *input = make(ElementType::InputSwitch);
    auto *transmitter = make(ElementType::Node);
    WirelessNode::setMode(*transmitter, WirelessMode::Tx);
    transmitter->setLabel(QStringLiteral("channel"));
    auto *receiver = make(ElementType::Node);
    WirelessNode::setMode(*receiver, WirelessMode::Rx);
    receiver->setLabel(QStringLiteral("channel"));
    auto *led = make(ElementType::Led);
    builder.add(input, transmitter, receiver, led);
    builder.connect(input, 0, transmitter, 0);
    builder.connect(receiver, 0, led, 0);
}

/// Loads \a path as an IC, the instance of a building block, in the circuit \a builder builds.
IC *loadBlock(CircuitBuilder &builder, const QString &path, const QString &dir)
{
    auto *ic = new IC();
    ic->loadFile(path, dir);
    builder.addElement(ic);
    return ic;
}

} // namespace

SeededChoices::SeededChoices(const quint32 seed)
    : m_state(seed & 0x7fffffffU)
{
}

int SeededChoices::below(const int bound)
{
    return static_cast<int>((next() >> 8) % static_cast<quint32>(bound));
}

quint32 SeededChoices::next()
{
    m_state = (m_state * 1103515245U + 12345U) & 0x7fffffffU;
    return m_state;
}

QStringList buildingBlockFiles(const QString &dir)
{
    QDir().mkpath(dir);
    // The toggle flip-flop is written first: the counter nests its file.
    const QString toggle = writeBlock(dir, QStringLiteral("toggle"), buildToggleFlipFlop);
    return {toggle,
            writeBlock(dir, QStringLiteral("half-adder"), buildHalfAdder),
            writeBlock(dir, QStringLiteral("inverter"), buildInverter),
            writeBlock(dir, QStringLiteral("nand-latch"), buildNandLatch),
            writeBlock(dir, QStringLiteral("or-of-three"), buildOrOfThree),
            writeBlock(dir, QStringLiteral("counter"), buildCounter),
            writeBlock(dir, QStringLiteral("mux"), buildMux),
            writeBlock(dir, QStringLiteral("wireless-channel"), buildWirelessChannel)};
}

RandomCircuit makeRandomCircuit(ChoiceSource &choices, const QStringList &blockFiles)
{
    RandomCircuit circuit;
    circuit.workspace = TestUtils::createWorkspace();
    CircuitBuilder builder(circuit.workspace->scene());
    RandomCircuitFeatures &features = circuit.features;
    if (!blockFiles.isEmpty()) {
        circuit.workspace->scene()->setContextDir(QFileInfo(blockFiles.first()).absolutePath());
    }

    const int sourceCount = 2 + choices.below(4);
    for (int i = 0; i < sourceCount; ++i) {
        const ElementType type = pick(choices, kSourceTypes);
        auto *source = make(type);
        if (type == ElementType::Clock) {
            source->setParam(ParamId::Frequency, pick(choices, kClockFrequencies));
            source->setParam(ParamId::Delay, choices.below(100) / 100.0);
            const bool locked = choices.chance(kLockedClockPercent);
            source->setParam(ParamId::Locked, locked);
            features.lockedClocks += locked ? 1 : 0;
            circuit.clocks << source;
        } else if ((type == ElementType::InputSwitch) || (type == ElementType::InputButton)
                   || (type == ElementType::InputRotary)) {
            circuit.inputs << source;
        }
        builder.add(source);
        circuit.elements << source;
    }

    const int logicCount = 3 + choices.below(10);
    for (int i = 0; i < logicCount; ++i) {
        const ElementType type = pick(choices, kLogicTypes);
        auto *element = make(type);
        const int inMin = element->minInputSize();
        const int inMax = std::max(inMin, std::min(element->maxInputSize(), kMaxNodeInputs));
        const int outMin = element->minOutputSize();
        const int outMax = std::max(outMin, element->maxOutputSize());
        if ((inMax > inMin) || (outMax > outMin)) {
            TestUtils::applyPortCounts(element, inMin + choices.below(inMax - inMin + 1),
                                       outMin + choices.below(outMax - outMin + 1));
        }
        if (type == ElementType::TruthTable) {
            QBitArray key(element->param<QBitArray>(ParamId::Table).size());
            for (qsizetype bit = 0; bit < key.size(); ++bit) {
                key.setBit(bit, choices.chance(50));
            }
            element->setParam(ParamId::Table, key);
            features.truthTables++;
        } else if (isMemory(element)) {
            features.memoryElements++;
        } else if ((type == ElementType::Mux) || (type == ElementType::Demux)) {
            features.muxElements++;
        } else {
            features.gateElements++;
        }

        int delay = 0;
        if (!choices.chance(kZeroDelayPercent)) {
            delay = 1 + choices.below(kMaxDelayNs);
            features.delayedElements++;
        }
        circuit.delays.insert(element, delay);

        builder.add(element);
        circuit.elements << element;
        circuit.observed << element;
    }

    const int nodeCount = choices.below(4);
    for (int i = 0; i < nodeCount; ++i) {
        auto *node = make(ElementType::Node);
        const int roll = choices.below(3);
        const WirelessMode mode = (roll == 0) ? WirelessMode::None : ((roll == 1) ? WirelessMode::Tx : WirelessMode::Rx);
        WirelessNode::setMode(*node, mode);
        node->setLabel(QStringLiteral("L%1").arg(choices.below(kWirelessLabels)));
        features.wirelessNodes++;
        builder.add(node);
        circuit.elements << node;
        circuit.observed << node;
    }

    if (!blockFiles.isEmpty()) {
        const int icCount = choices.below(3);
        for (int i = 0; i < icCount; ++i) {
            auto *ic = loadBlock(builder, blockFiles[choices.below(static_cast<int>(blockFiles.size()))],
                                 circuit.workspace->scene()->contextDir());
            features.icInstances++;
            circuit.elements << ic;
            circuit.observed << ic;
        }
    }

    const int ledCount = 2 + choices.below(3);
    for (int i = 0; i < ledCount; ++i) {
        auto *led = make(ElementType::Led);
        builder.add(led);
        circuit.elements << led;
        circuit.observed << led;
    }

    // Every element with an output can drive an input, a feedback wire included, since it may come later in the list.
    QList<Element *> drivers;
    QHash<const Element *, int> position;
    for (int index = 0; index < circuit.elements.size(); ++index) {
        Element *element = circuit.elements[index];
        position.insert(element, index);
        if (element->outputSize() > 0) {
            drivers << element;
        }
    }

    for (int index = sourceCount; index < circuit.elements.size(); ++index) {
        Element *consumer = circuit.elements[index];
        const bool receiver = isReceiver(consumer);
        for (int port = 0; port < consumer->inputSize(); ++port) {
            if (receiver && !choices.chance(kWiredReceiverPercent)) {
                continue;
            }
            if (choices.chance(kOpenInputPercent)) {
                features.openInputs++;
                continue;
            }
            if (receiver) {
                features.wiredReceivers++;
            }
            if (isMemoryClock(consumer, port)) {
                wireMemoryClock(builder, circuit, drivers, consumer, port, choices);
                continue;
            }
            wireRandomDriver(builder, drivers, position, consumer, port, choices, features);
            if (choices.chance(kDoubleDrivenPercent)) {
                wireRandomDriver(builder, drivers, position, consumer, port, choices, features);
                features.doubleDriven++;
            }
        }
    }

    // A chain of toggle flip-flops, each clocked by the one before it, runs past the round budget. Its stages are
    // wired here, after the random wiring, so no random wire joins them.
    if (choices.chance(kRippleChainPercent)) {
        const int stages = kRoundBudget + 1 + choices.below(4);
        auto *clock = make(ElementType::Clock);
        clock->setParam(ParamId::Frequency, 1000.0);
        auto *vcc = make(ElementType::InputVcc);
        builder.add(clock, vcc);
        circuit.elements << clock << vcc;
        circuit.clocks << clock;
        Element *previous = clock;
        for (int stage = 0; stage < stages; ++stage) {
            auto *flipFlop = make(ElementType::TFlipFlop);
            builder.add(flipFlop);
            builder.connect(vcc, 0, flipFlop, 0);
            builder.connect(previous, 0, flipFlop, 1);
            circuit.elements << flipFlop;
            circuit.observed << flipFlop;
            circuit.delays.insert(flipFlop, 0);
            previous = flipFlop;
        }
        features.rippleChains++;
    }
    return circuit;
}

QStringList describeRandomCircuit(const RandomCircuit &circuit)
{
    QHash<const Element *, int> index;
    for (int i = 0; i < circuit.elements.size(); ++i) {
        index.insert(circuit.elements[i], i);
    }

    QStringList lines;
    for (int i = 0; i < circuit.elements.size(); ++i) {
        const Element *element = circuit.elements[i];
        QString line = QString("E%1 %2 in=%3 out=%4")
                           .arg(i)
                           .arg(ElementFactory::typeToText(element->elementType()))
                           .arg(element->inputSize())
                           .arg(element->outputSize());
        if (element->elementType() == ElementType::Clock) {
            line += QString(" freq=%1 phase=%2 locked=%3")
                        .arg(element->param<double>(ParamId::Frequency))
                        .arg(element->param<double>(ParamId::Delay))
                        .arg(element->param<bool>(ParamId::Locked));
        }
        lines << line;
        if (circuit.delays.contains(element)) {
            lines << QString("D%1 %2").arg(i).arg(circuit.delays.value(element));
        }
    }
    for (int j = 0; j < circuit.elements.size(); ++j) {
        const Element *consumer = circuit.elements[j];
        for (int port = 0; port < consumer->inputSize(); ++port) {
            for (const Connection *wire : consumer->inputPort(port)->connections()) {
                const Element *driver = wire->startPort()->element();
                int outPort = 0;
                for (int out = 0; out < driver->outputSize(); ++out) {
                    if (driver->outputPort(out) == wire->startPort()) {
                        outPort = out;
                    }
                }
                lines << QString("W %1.%2 -> %3.%4").arg(index.value(driver)).arg(outPort).arg(j).arg(port);
            }
        }
    }
    return lines;
}

QStringList randomCircuitWalk(RandomCircuit &circuit, TraceEngine &engine, ChoiceSource &choices, const int steps)
{
    QStringList lines;
    for (int step = 1; step <= steps; ++step) {
        for (auto *input : std::as_const(circuit.inputs)) {
            if (!choices.chance(kInputChangePercent)) {
                continue;
            }
            if (input->elementType() == ElementType::InputRotary) {
                input->setOn(true, choices.below(input->outputSize()));
            } else {
                input->setOn(choices.chance(50));
            }
        }
        engine.step();

        QString shown;
        for (const auto *element : std::as_const(circuit.observed)) {
            for (const Status status : engine.observed(element)) {
                shown += statusChar(status);
            }
            shown += ' ';
        }
        lines << QString("  %1: %2").arg(step).arg(shown.trimmed());
    }
    return lines;
}
