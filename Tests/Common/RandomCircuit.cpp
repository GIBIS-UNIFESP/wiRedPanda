// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Common/RandomCircuit.h"

#include <algorithm>
#include <array>

#include <QBitArray>

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/Parameters.h"
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

constexpr std::array<ElementType, 6> kSourceTypes{ElementType::InputSwitch, ElementType::InputButton,
                                                  ElementType::InputRotary, ElementType::InputVcc,
                                                  ElementType::InputGnd, ElementType::Clock};

constexpr std::array<ElementType, 16> kLogicTypes{
    ElementType::And,     ElementType::Or,     ElementType::Not,       ElementType::Nand,
    ElementType::Nor,     ElementType::Xor,    ElementType::Xnor,      ElementType::DFlipFlop,
    ElementType::JKFlipFlop, ElementType::TFlipFlop, ElementType::SRFlipFlop, ElementType::DLatch,
    ElementType::SRLatch, ElementType::Mux,    ElementType::Demux,     ElementType::TruthTable};

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

/// Wires \a consumer's input \a port to a random output of a random driver, and counts a feedback wire.
void wireRandomDriver(CircuitBuilder &builder, const QList<Element *> &drivers,
                      const QHash<const Element *, int> &position, Element *consumer, const int port, ChoiceSource &choices, RandomCircuitFeatures &features)
{
    Element *driver = drivers[choices.below(static_cast<int>(drivers.size()))];
    builder.connect(driver, choices.below(driver->outputSize()), consumer, port);
    if (position.value(driver) >= position.value(consumer)) {
        features.feedbackWires++;
    }
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

RandomCircuit makeRandomCircuit(ChoiceSource &choices)
{
    RandomCircuit circuit;
    circuit.workspace = TestUtils::createWorkspace();
    CircuitBuilder builder(circuit.workspace->scene());
    RandomCircuitFeatures &features = circuit.features;

    const int sourceCount = 2 + choices.below(4);
    for (int i = 0; i < sourceCount; ++i) {
        const ElementType type = pick(choices, kSourceTypes);
        auto *source = ElementFactory::buildElement(type);
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
        auto *element = ElementFactory::buildElement(type);
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
        } else if ((type == ElementType::DFlipFlop) || (type == ElementType::JKFlipFlop)
                   || (type == ElementType::TFlipFlop) || (type == ElementType::SRFlipFlop)
                   || (type == ElementType::DLatch) || (type == ElementType::SRLatch)) {
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

    const int ledCount = 2 + choices.below(3);
    for (int i = 0; i < ledCount; ++i) {
        auto *led = ElementFactory::buildElement(ElementType::Led);
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
        for (int port = 0; port < consumer->inputSize(); ++port) {
            if (choices.chance(kOpenInputPercent)) {
                features.openInputs++;
                continue;
            }
            wireRandomDriver(builder, drivers, position, consumer, port, choices, features);
            if (choices.chance(kDoubleDrivenPercent)) {
                wireRandomDriver(builder, drivers, position, consumer, port, choices, features);
                features.doubleDriven++;
            }
        }
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
