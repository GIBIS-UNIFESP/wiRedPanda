// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/SRFlipFlop.h"

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Wiring/Port.h"

template<>
struct ElementInfo<SRFlipFlop> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::SRFlipFlop,
        .group = ElementGroup::Memory,
        .minInputSize = 5,
        .maxInputSize = 5,
        .minOutputSize = 2,
        .maxOutputSize = 2,
        .canChangeAppearance = true,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return SRFlipFlop::pixmapPath(); };
        meta.titleText = QT_TRANSLATE_NOOP("SRFlipFlop", "SR-FLIP-FLOP");
        meta.translatedName = QT_TRANSLATE_NOOP("SRFlipFlop", "SR-Flip-Flop");
        meta.trContext = "SRFlipFlop";
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new SRFlipFlop(); });
        return true;
    }();
};

SRFlipFlop::SRFlipFlop(QGraphicsItem *parent)
    : Element(ElementType::SRFlipFlop, parent)
{
    // The ports take their places, names, requirements and defaults from this type's attributes
    updatePortsProperties();
}

std::optional<Element::PortAttributes> SRFlipFlop::inputAttributes(const int index) const
{
    // SR flip-flop: S (Set) and R (Reset) are clocked inputs; Clock is required.
    // ~Preset and ~Clear provide asynchronous overrides (active-low, same as DFlipFlop).
    // S, R, ~Preset, ~Clear are optional; only Clock is always required.
    // S and R unconnected default to Inactive (no set/reset action on clock edge).
    // ~Preset and ~Clear unconnected default to Active (HIGH = not asserted).
    static const QList<PortAttributes> attributes{
        {.name = "S", .required = false, .defaultStatus = Status::Inactive, .position = QPointF(0, 16)},
        {.name = "Clock", .position = QPointF(0, 32)},
        {.name = "R", .required = false, .defaultStatus = Status::Inactive, .position = QPointF(0, 48)},
        {.name = "~Preset", .required = false, .defaultStatus = Status::Active, .position = QPointF(32, 0)},
        {.name = "~Clear", .required = false, .defaultStatus = Status::Active, .position = QPointF(32, 64)},
    };
    return attributesAt(attributes, index);
}

std::optional<Element::PortAttributes> SRFlipFlop::outputAttributes(const int index) const
{
    // Initial state: Q=0, ~Q=1
    static const QList<PortAttributes> attributes{
        {.name = "Q", .defaultStatus = Status::Inactive, .position = QPointF(64, 16)},
        {.name = "~Q", .defaultStatus = Status::Active, .position = QPointF(64, 48)},
    };
    return attributesAt(attributes, index);
}

void SRFlipFlop::updateLogic()
{
    if (!simUpdateInputs()) {
        // Forget the last clock level while inputs are invalid (see DFlipFlop).
        m_simLastClk = Status::Unknown;
        return;
    }
    Status q0 = simOutputs().at(0);
    Status q1 = simOutputs().at(1);
    const Status s = simInputs().at(0);
    const Status clk = simInputs().at(1);
    const Status r = simInputs().at(2);
    const Status prst = simInputs().at(3);
    const Status clr = simInputs().at(4);

    if (clk == Status::Active && m_simLastClk == Status::Inactive) {
        if (m_simLastS == Status::Active && m_simLastR == Status::Active) {
            q0 = Status::Active;
            q1 = Status::Active;
        } else if (m_simLastS != m_simLastR) {
            q0 = m_simLastS;
            q1 = m_simLastR;
        }
    }

    if (prst == Status::Inactive || clr == Status::Inactive) {
        q0 = (prst == Status::Inactive) ? Status::Active : Status::Inactive;
        q1 = (clr == Status::Inactive) ? Status::Active : Status::Inactive;
    }
    m_simLastClk = clk;
    m_simLastS = s;
    m_simLastR = r;
    setOutputValue(0, q0);
    setOutputValue(1, q1);
}

void SRFlipFlop::resetSimState()
{
    Element::resetSimState();
    m_simLastClk = Status::Inactive;
    m_simLastS   = Status::Inactive;
    m_simLastR   = Status::Inactive;
}

void SRFlipFlop::saveSimState(QVector<Status> &out) const
{
    // Mirrors resetSimState(): the outputs via the base, then the edge-detection history, in a
    // fixed order that restoreSimState() reads back identically.
    Element::saveSimState(out);
    out.append(m_simLastClk);
    out.append(m_simLastS);
    out.append(m_simLastR);
}

void SRFlipFlop::restoreSimState(const QVector<Status> &in, int &cursor)
{
    Element::restoreSimState(in, cursor);
    if (cursor < in.size()) { m_simLastClk = in.at(cursor++); }
    if (cursor < in.size()) { m_simLastS = in.at(cursor++); }
    if (cursor < in.size()) { m_simLastR = in.at(cursor++); }
}
