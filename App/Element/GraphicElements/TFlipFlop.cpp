// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/TFlipFlop.h"

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Nodes/QNEPort.h"

template<>
struct ElementInfo<TFlipFlop> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::TFlipFlop,
        .group = ElementGroup::Memory,
        .minInputSize = 4,
        .maxInputSize = 4,
        .minOutputSize = 2,
        .maxOutputSize = 2,
        .canChangeSkin = true,
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
    : GraphicElement(ElementType::TFlipFlop, parent)
{
    // Call the most-derived override explicitly (see SRFlipFlop.cpp for rationale).
    TFlipFlop::updatePortsProperties();
}

void TFlipFlop::updatePortsProperties()
{
    // T (Toggle) flip-flop: toggles Q on each rising clock edge when T=1; holds when T=0.
    // ~Preset and ~Clear are asynchronous active-low overrides (same convention as DFlipFlop).
    inputPort(0)->setPos( 0, 16);     inputPort(0)->setName("T");
    inputPort(1)->setPos( 0, 48);     inputPort(1)->setName("Clock");
    inputPort(2)->setPos(32,  0);     inputPort(2)->setName("~Preset");
    inputPort(3)->setPos(32, 64);     inputPort(3)->setName("~Clear");

    outputPort(0)->setPos(64, 16);    outputPort(0)->setName("Q");
    outputPort(1)->setPos(64, 48);    outputPort(1)->setName("~Q");

    // T is optional; when unconnected its default Inactive (LOW) means "hold" on every clock edge.
    // ~Preset and ~Clear default to Active (HIGH = not asserted).
    inputPort(0)->setRequired(false);
    inputPort(2)->setRequired(false);
    inputPort(3)->setRequired(false);

    inputPort(2)->setDefaultStatus(Status::Active);
    inputPort(3)->setDefaultStatus(Status::Active);

    // Initial state: Q=0, ~Q=1
    outputPort(0)->setDefaultStatus(Status::Inactive);
    outputPort(1)->setDefaultStatus(Status::Active);
}

void TFlipFlop::updateTheme()
{
    // Reload the pixmap before delegating to the base class (see SRFlipFlop.cpp).
    setPixmap(pixmapPath());
    GraphicElement::updateTheme();
}

void TFlipFlop::updateLogic()
{
    if (!updateInputs()) {
        return;
    }
    bool q0 = outputValue(0);
    bool q1 = outputValue(1);
    const bool T = simInputs().at(0);
    const bool clk = simInputs().at(1);
    const bool prst = simInputs().at(2);
    const bool clr = simInputs().at(3);

    if (clk && !m_lastClk) {
        if (m_lastValue) {
            q0 = !q0;
            q1 = !q1;
        }
    }
    if (!prst || !clr) {
        q0 = !prst;
        q1 = !clr;
    }
    m_lastClk = clk;
    m_lastValue = T;
    setOutputValue(0, q0);
    setOutputValue(1, q1);
}

