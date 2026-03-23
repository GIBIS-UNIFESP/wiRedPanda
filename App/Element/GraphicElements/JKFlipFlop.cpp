// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/JKFlipFlop.h"

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Nodes/QNEPort.h"

template<>
struct ElementInfo<JKFlipFlop> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::JKFlipFlop,
        .group = ElementGroup::Memory,
        .minInputSize = 5,
        .maxInputSize = 5,
        .minOutputSize = 2,
        .maxOutputSize = 2,
        .canChangeSkin = true,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return JKFlipFlop::pixmapPath(); };
        meta.titleText = QT_TRANSLATE_NOOP("JKFlipFlop", "JK-FLIP-FLOP");
        meta.translatedName = QT_TRANSLATE_NOOP("JKFlipFlop", "JK-Flip-Flop");
        meta.trContext = "JKFlipFlop";
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new JKFlipFlop(); });
        return true;
    }();
};

JKFlipFlop::JKFlipFlop(QGraphicsItem *parent)
    : GraphicElement(ElementType::JKFlipFlop, parent)
{
    // Call the most-derived override explicitly (see SRFlipFlop.cpp for rationale).
    JKFlipFlop::updatePortsProperties();
}

void JKFlipFlop::updatePortsProperties()
{
    // JK flip-flop: J (Set-like) and K (Reset-like) are clocked, with Clock required.
    // J=1,K=1 on a clock edge toggles output — making the JK superior to SR (no forbidden state).
    // ~Preset and ~Clear are asynchronous active-low overrides (same as DFlipFlop).
    inputPort(0)->setPos( 0, 16);     inputPort(0)->setName("J");
    inputPort(1)->setPos( 0, 32);     inputPort(1)->setName("Clock");
    inputPort(2)->setPos( 0, 48);     inputPort(2)->setName("K");
    inputPort(3)->setPos(32,  0);     inputPort(3)->setName("~Preset");
    inputPort(4)->setPos(32, 64);     inputPort(4)->setName("~Clear");

    outputPort(0)->setPos(64, 16);    outputPort(0)->setName("Q");
    outputPort(1)->setPos(64, 48);    outputPort(1)->setName("~Q");

    inputPort(0)->setRequired(false);
    inputPort(2)->setRequired(false);
    inputPort(3)->setRequired(false);
    inputPort(4)->setRequired(false);

    // J and K default to Active (HIGH) when unconnected, so the flip-flop toggles on every
    // clock edge by default — a useful demonstration behavior for the educational context.
    // ~Preset and ~Clear default to Active (HIGH = not asserted).
    inputPort(0)->setDefaultStatus(Status::Active);
    inputPort(2)->setDefaultStatus(Status::Active);
    inputPort(3)->setDefaultStatus(Status::Active);
    inputPort(4)->setDefaultStatus(Status::Active);

    // Initial state: Q=0, ~Q=1
    outputPort(0)->setDefaultStatus(Status::Inactive);
    outputPort(1)->setDefaultStatus(Status::Active);
}

void JKFlipFlop::updateTheme()
{
    // Reload the pixmap before delegating to the base class (see SRFlipFlop.cpp).
    setPixmap(pixmapPath());
    GraphicElement::updateTheme();
}

void JKFlipFlop::updateLogic()
{
    if (!updateInputs()) {
        return;
    }
    bool q0 = outputValue(0);
    bool q1 = outputValue(1);
    const bool j = simInputs().at(0);
    const bool clk = simInputs().at(1);
    const bool k = simInputs().at(2);
    const bool prst = simInputs().at(3);
    const bool clr = simInputs().at(4);

    if (clk && !m_lastClk) {
        if (m_lastJ && m_lastK) {
            std::swap(q0, q1);
        } else if (m_lastJ) {
            q0 = true;
            q1 = false;
        } else if (m_lastK) {
            q0 = false;
            q1 = true;
        }
    }
    if (!prst || !clr) {
        q0 = !prst;
        q1 = !clr;
    }
    m_lastClk = clk;
    m_lastJ = j;
    m_lastK = k;
    setOutputValue(0, q0);
    setOutputValue(1, q1);
}

