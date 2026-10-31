// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/JKFlipFlop.h"

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Wiring/Port.h"

template<>
struct ElementInfo<JKFlipFlop> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::JKFlipFlop,
        .group = ElementGroup::Memory,
        .minInputSize = 5,
        .maxInputSize = 5,
        .minOutputSize = 2,
        .maxOutputSize = 2,
        .canChangeAppearance = true,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        // The Light theme's picture; presentation shows its Dark sibling in the Dark theme (ElementPictures::themed())
        descriptor.pixmapPath = QStringLiteral(":/Components/Memory/Light/JK-flipflop.svg");
        descriptor.defaultAppearances = QStringList({descriptor.pixmapPath});
        descriptor.titleText = QT_TRANSLATE_NOOP("JKFlipFlop", "JK-FLIP-FLOP");
        descriptor.translatedName = QT_TRANSLATE_NOOP("JKFlipFlop", "JK-Flip-Flop");
        descriptor.trContext = "JKFlipFlop";
        return descriptor;
    }

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new JKFlipFlop(); });
        return true;
    }();
};

JKFlipFlop::JKFlipFlop(QGraphicsItem *parent)
    : Element(ElementType::JKFlipFlop, parent)
{
    // The ports take their places, names, requirements and defaults from this type's attributes
    updatePortsProperties();
}

std::optional<Element::PortAttributes> JKFlipFlop::inputAttributes(const int index) const
{
    // JK flip-flop: J (Set-like) and K (Reset-like) are clocked, with Clock required.
    // J=1,K=1 on a clock edge toggles output — making the JK superior to SR (no forbidden state).
    // ~Preset and ~Clear are asynchronous active-low overrides (same as DFlipFlop).
    // J and K default to Active (HIGH) when unconnected, so the flip-flop toggles on every
    // clock edge by default — a useful demonstration behavior for the educational context.
    // ~Preset and ~Clear default to Active (HIGH = not asserted).
    static const QList<PortAttributes> attributes{
        {.name = "J", .required = false, .defaultStatus = Status::Active, .position = QPointF(0, 16)},
        {.name = "Clock", .position = QPointF(0, 32)},
        {.name = "K", .required = false, .defaultStatus = Status::Active, .position = QPointF(0, 48)},
        {.name = "~Preset", .required = false, .defaultStatus = Status::Active, .position = QPointF(32, 0)},
        {.name = "~Clear", .required = false, .defaultStatus = Status::Active, .position = QPointF(32, 64)},
    };
    return attributesAt(attributes, index);
}

std::optional<Element::PortAttributes> JKFlipFlop::outputAttributes(const int index) const
{
    // Initial state: Q=0, ~Q=1
    static const QList<PortAttributes> attributes{
        {.name = "Q", .defaultStatus = Status::Inactive, .position = QPointF(64, 16)},
        {.name = "~Q", .defaultStatus = Status::Active, .position = QPointF(64, 48)},
    };
    return attributesAt(attributes, index);
}

void JKFlipFlop::updateLogic()
{
    if (!simUpdateInputs()) {
        // Forget the last clock level while inputs are invalid (see DFlipFlop).
        m_simLastClk = Status::Unknown;
        return;
    }
    Status q0 = simOutputs().at(0);
    Status q1 = simOutputs().at(1);
    const Status j = simInputs().at(0);
    const Status clk = simInputs().at(1);
    const Status k = simInputs().at(2);
    const Status prst = simInputs().at(3);
    const Status clr = simInputs().at(4);

    if (clk == Status::Active && m_simLastClk == Status::Inactive) {
        if (m_simLastJ == Status::Active && m_simLastK == Status::Active) {
            std::swap(q0, q1);
        } else if (m_simLastJ == Status::Active) {
            q0 = Status::Active;
            q1 = Status::Inactive;
        } else if (m_simLastK == Status::Active) {
            q0 = Status::Inactive;
            q1 = Status::Active;
        }
    }

    if (prst == Status::Inactive || clr == Status::Inactive) {
        q0 = (prst == Status::Inactive) ? Status::Active : Status::Inactive;
        q1 = (clr == Status::Inactive) ? Status::Active : Status::Inactive;
    }
    m_simLastClk = clk;
    m_simLastJ = j;
    m_simLastK = k;
    setOutputValue(0, q0);
    setOutputValue(1, q1);
}

void JKFlipFlop::resetSimState()
{
    Element::resetSimState();
    m_simLastClk = Status::Inactive;
    m_simLastJ   = Status::Active;
    m_simLastK   = Status::Active;
}

void JKFlipFlop::saveSimState(QVector<Status> &out) const
{
    // Mirrors resetSimState(): the outputs via the base, then the edge-detection history, in a
    // fixed order that restoreSimState() reads back identically.
    Element::saveSimState(out);
    out.append(m_simLastClk);
    out.append(m_simLastJ);
    out.append(m_simLastK);
}

void JKFlipFlop::restoreSimState(const QVector<Status> &in, int &cursor)
{
    Element::restoreSimState(in, cursor);
    if (cursor < in.size()) { m_simLastClk = in.at(cursor++); }
    if (cursor < in.size()) { m_simLastJ = in.at(cursor++); }
    if (cursor < in.size()) { m_simLastK = in.at(cursor++); }
}
