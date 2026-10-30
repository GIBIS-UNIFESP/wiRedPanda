// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/DFlipFlop.h"

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Wiring/Port.h"

template<>
struct ElementInfo<DFlipFlop> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::DFlipFlop,
        .group = ElementGroup::Memory,
        .minInputSize = 4,
        .maxInputSize = 4,
        .minOutputSize = 2,
        .maxOutputSize = 2,
        .canChangeAppearance = true,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = []{ return DFlipFlop::pixmapPath(); };
        descriptor.titleText = QT_TRANSLATE_NOOP("DFlipFlop", "D-FLIP-FLOP");
        descriptor.translatedName = QT_TRANSLATE_NOOP("DFlipFlop", "D-Flip-Flop");
        descriptor.trContext = "DFlipFlop";
        return descriptor;
    }

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new DFlipFlop(); });
        return true;
    }();
};

DFlipFlop::DFlipFlop(QGraphicsItem *parent)
    : Element(ElementType::DFlipFlop, parent)
{
    // The ports take their places, names, requirements and defaults from this type's attributes
    updatePortsProperties();
}

std::optional<Element::PortAttributes> DFlipFlop::inputAttributes(const int index) const
{
    // Standard D flip-flop pin layout: D and Clock on the left,
    // active-low ~Preset and ~Clear on top/bottom (asynchronous overrides).
    // ~Preset and ~Clear are optional; when unconnected they default to Active (HIGH = not asserted)
    // because these signals are active-low — leaving them floating means "no override"
    static const QList<PortAttributes> attributes{
        {.name = "Data", .position = QPointF(0, 16)},
        {.name = "Clock", .position = QPointF(0, 48)},
        {.name = "~Preset", .required = false, .defaultStatus = Status::Active, .position = QPointF(32, 0)},
        {.name = "~Clear", .required = false, .defaultStatus = Status::Active, .position = QPointF(32, 64)},
    };
    return attributesAt(attributes, index);
}

std::optional<Element::PortAttributes> DFlipFlop::outputAttributes(const int index) const
{
    // Initial state: Q=0, ~Q=1 (reset/cleared state)
    static const QList<PortAttributes> attributes{
        {.name = "Q", .defaultStatus = Status::Inactive, .position = QPointF(64, 16)},
        {.name = "~Q", .defaultStatus = Status::Active, .position = QPointF(64, 48)},
    };
    return attributesAt(attributes, index);
}

void DFlipFlop::updateLogic()
{
    if (!simUpdateInputs()) {
        // Forget the last clock level while inputs are invalid: edge
        // detection requires lastClk == Inactive, so recovery with the clock
        // already high cannot fabricate a rising edge from stale state.
        m_simLastClk = Status::Unknown;
        return;
    }
    Status q0 = simOutputs().at(0);
    Status q1 = simOutputs().at(1);
    const Status D = simInputs().at(0);
    const Status clk = simInputs().at(1);
    const Status prst = simInputs().at(2);
    const Status clr = simInputs().at(3);

    if (clk == Status::Active && m_simLastClk == Status::Inactive) {
        q0 = m_simLastValue;
        q1 = (m_simLastValue == Status::Active) ? Status::Inactive : Status::Active;
    }

    if (prst == Status::Inactive || clr == Status::Inactive) {
        q0 = (prst == Status::Inactive) ? Status::Active : Status::Inactive;
        q1 = (clr == Status::Inactive) ? Status::Active : Status::Inactive;
    }
    m_simLastClk = clk;
    m_simLastValue = D;
    setOutputValue(0, q0);
    setOutputValue(1, q1);
}

void DFlipFlop::resetSimState()
{
    // Reset Q/~Q to power-on defaults via the base class, then restore the
    // edge-detection variables so column 0 of the next sweep is treated as
    // a fresh start (no phantom rising edge from a prior run).
    Element::resetSimState();
    m_simLastClk   = Status::Inactive;
    m_simLastValue = Status::Active;
}

void DFlipFlop::saveSimState(QVector<Status> &out) const
{
    // Mirrors resetSimState(): the outputs via the base, then the edge-detection history, in a
    // fixed order that restoreSimState() reads back identically.
    Element::saveSimState(out);
    out.append(m_simLastClk);
    out.append(m_simLastValue);
}

void DFlipFlop::restoreSimState(const QVector<Status> &in, int &cursor)
{
    Element::restoreSimState(in, cursor);
    if (cursor < in.size()) { m_simLastClk = in.at(cursor++); }
    if (cursor < in.size()) { m_simLastValue = in.at(cursor++); }
}
