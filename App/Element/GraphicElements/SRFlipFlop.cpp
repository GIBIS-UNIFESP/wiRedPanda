// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/SRFlipFlop.h"

#include "App/Element/ElementInfo.h"
#include "App/Element/LogicElements/LogicSRFlipFlop.h"
#include "App/Nodes/QNEPort.h"

template<>
struct ElementInfo<SRFlipFlop> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::SRFlipFlop,
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
        meta.pixmapPath = []{ return SRFlipFlop::pixmapPath(); };
        meta.titleText = QT_TRANSLATE_NOOP("SRFlipFlop", "SR-FLIP-FLOP");
        meta.translatedName = QT_TRANSLATE_NOOP("SRFlipFlop", "SR-Flip-Flop");
        meta.trContext = "SRFlipFlop";
        meta.defaultSkins = QStringList({":/Components/Memory/Dark/SR-flipflop.svg"});
        meta.logicCreator = [](GraphicElement *) { auto e = std::make_shared<LogicSRFlipFlop>(); e->setPropagationDelay(10); return e; };
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new SRFlipFlop(); });
        return true;
    }();
};

SRFlipFlop::SRFlipFlop(QGraphicsItem *parent)
    : GraphicElement(ElementType::SRFlipFlop, parent)
{
    // Call the most-derived override explicitly to guarantee correct port positions
    // even if a subclass overrides updatePortsProperties() further.
    SRFlipFlop::updatePortsProperties();
}

void SRFlipFlop::updatePortsProperties()
{
    // SR flip-flop: S (Set) and R (Reset) are clocked inputs; Clock is required.
    // ~Preset and ~Clear provide asynchronous overrides (active-low, same as DFlipFlop).
    inputPort(0)->setPos( 0, 16);     inputPort(0)->setName("S");
    inputPort(1)->setPos( 0, 32);     inputPort(1)->setName("Clock");
    inputPort(2)->setPos( 0, 48);     inputPort(2)->setName("R");
    inputPort(3)->setPos(32,  0);     inputPort(3)->setName("~Preset");
    inputPort(4)->setPos(32, 64);     inputPort(4)->setName("~Clear");

    outputPort(0)->setPos(64, 16);    outputPort(0)->setName("Q");
    outputPort(1)->setPos(64, 48);    outputPort(1)->setName("~Q");

    // S, R, ~Preset, ~Clear are optional; only Clock is always required.
    // S and R unconnected default to Inactive (no set/reset action on clock edge).
    // ~Preset and ~Clear unconnected default to Active (HIGH = not asserted).
    inputPort(0)->setRequired(false);
    inputPort(2)->setRequired(false);
    inputPort(3)->setRequired(false);
    inputPort(4)->setRequired(false);

    inputPort(3)->setDefaultStatus(Status::Active);
    inputPort(4)->setDefaultStatus(Status::Active);

    // Initial state: Q=0, ~Q=1
    outputPort(0)->setDefaultStatus(Status::Inactive);
    outputPort(1)->setDefaultStatus(Status::Active);
}

void SRFlipFlop::updateTheme()
{
    // Reload the pixmap before delegating to the base class so the correct
    // theme-specific SVG is used before ports and colours are updated.
    setPixmap(pixmapPath());
    GraphicElement::updateTheme();
}
