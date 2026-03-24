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
        .hasColors = false,
        .hasAudio = false,
        .hasAudioBox = false,
        .hasTrigger = false,
        .hasFrequency = false,
        .hasDelay = false,
        .hasLabel = false,
        .hasTruthTable = false,
        .rotatable = true,
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
    if (!simUpdateInputs()) {
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

