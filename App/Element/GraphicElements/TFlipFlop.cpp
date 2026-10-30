// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/TFlipFlop.h"

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Wiring/Port.h"

template<>
struct ElementInfo<TFlipFlop> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::TFlipFlop,
        .group = ElementGroup::Memory,
        .minInputSize = 4,
        .maxInputSize = 4,
        .minOutputSize = 2,
        .maxOutputSize = 2,
        .canChangeAppearance = true,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return TFlipFlop::pixmapPath(); };
        meta.titleText = QT_TRANSLATE_NOOP("TFlipFlop", "T-FLIP-FLOP");
        meta.translatedName = QT_TRANSLATE_NOOP("TFlipFlop", "T-Flip-Flop");
        meta.trContext = "TFlipFlop";
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new TFlipFlop(); });
        return true;
    }();
};

TFlipFlop::TFlipFlop(QGraphicsItem *parent)
    : Element(ElementType::TFlipFlop, parent)
{
    // The ports take their places, names, requirements and defaults from this type's attributes
    updatePortsProperties();
}

std::optional<Element::PortAttributes> TFlipFlop::inputAttributes(const int index) const
{
    // T (Toggle) flip-flop: toggles Q on each rising clock edge when T=1; holds when T=0.
    // ~Preset and ~Clear are asynchronous active-low overrides (same convention as DFlipFlop).
    // T is optional; when unconnected its default Inactive (LOW) means "hold" on every clock edge.
    // ~Preset and ~Clear default to Active (HIGH = not asserted).
    static const QList<PortAttributes> attributes{
        {.name = "T", .required = false, .defaultStatus = Status::Inactive, .position = QPointF(0, 16)},
        {.name = "Clock", .position = QPointF(0, 48)},
        {.name = "~Preset", .required = false, .defaultStatus = Status::Active, .position = QPointF(32, 0)},
        {.name = "~Clear", .required = false, .defaultStatus = Status::Active, .position = QPointF(32, 64)},
    };
    return attributesAt(attributes, index);
}

std::optional<Element::PortAttributes> TFlipFlop::outputAttributes(const int index) const
{
    // Initial state: Q=0, ~Q=1
    static const QList<PortAttributes> attributes{
        {.name = "Q", .defaultStatus = Status::Inactive, .position = QPointF(64, 16)},
        {.name = "~Q", .defaultStatus = Status::Active, .position = QPointF(64, 48)},
    };
    return attributesAt(attributes, index);
}

void TFlipFlop::updateLogic()
{
    if (!simUpdateInputs()) {
        // Forget the last clock level while inputs are invalid (see DFlipFlop).
        m_simLastClk = Status::Unknown;
        return;
    }
    Status q0 = simOutputs().at(0);
    Status q1 = simOutputs().at(1);
    const Status T = simInputs().at(0);
    const Status clk = simInputs().at(1);
    const Status prst = simInputs().at(2);
    const Status clr = simInputs().at(3);

    if (clk == Status::Active && m_simLastClk == Status::Inactive) {
        if (m_simLastValue == Status::Active) {
            q0 = (q0 == Status::Active) ? Status::Inactive : Status::Active;
            q1 = (q1 == Status::Active) ? Status::Inactive : Status::Active;
        }
    }

    if (prst == Status::Inactive || clr == Status::Inactive) {
        q0 = (prst == Status::Inactive) ? Status::Active : Status::Inactive;
        q1 = (clr == Status::Inactive) ? Status::Active : Status::Inactive;
    }
    m_simLastClk = clk;
    m_simLastValue = T;
    setOutputValue(0, q0);
    setOutputValue(1, q1);
}

void TFlipFlop::resetSimState()
{
    Element::resetSimState();
    m_simLastClk   = Status::Inactive;
    m_simLastValue = Status::Active;
}

void TFlipFlop::saveSimState(QVector<Status> &out) const
{
    // Mirrors resetSimState(): the outputs via the base, then the edge-detection history, in a
    // fixed order that restoreSimState() reads back identically.
    Element::saveSimState(out);
    out.append(m_simLastClk);
    out.append(m_simLastValue);
}

void TFlipFlop::restoreSimState(const QVector<Status> &in, int &cursor)
{
    Element::restoreSimState(in, cursor);
    if (cursor < in.size()) { m_simLastClk = in.at(cursor++); }
    if (cursor < in.size()) { m_simLastValue = in.at(cursor++); }
}
