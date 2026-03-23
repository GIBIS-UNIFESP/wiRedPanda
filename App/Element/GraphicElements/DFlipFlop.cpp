// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/DFlipFlop.h"

#include "App/Element/ElementInfo.h"
#include "App/Element/LogicElements/LogicDFlipFlop.h"
#include "App/Nodes/QNEPort.h"

template<>
struct ElementInfo<DFlipFlop> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::DFlipFlop,
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
        meta.pixmapPath = []{ return DFlipFlop::pixmapPath(); };
        meta.titleText = QT_TRANSLATE_NOOP("DFlipFlop", "D-FLIP-FLOP");
        meta.translatedName = QT_TRANSLATE_NOOP("DFlipFlop", "D-Flip-Flop");
        meta.trContext = "DFlipFlop";
        meta.logicCreator = [](GraphicElement *) { return std::make_shared<LogicDFlipFlop>(); };
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new DFlipFlop(); });
        return true;
    }();
};

DFlipFlop::DFlipFlop(QGraphicsItem *parent)
    : GraphicElement(ElementType::DFlipFlop, parent)
{
    // Call the most-derived override explicitly (see SRFlipFlop.cpp for rationale).
    DFlipFlop::updatePortsProperties();
}

void DFlipFlop::updatePortsProperties()
{
    // Standard D flip-flop pin layout: D and Clock on the left,
    // active-low ~Preset and ~Clear on top/bottom (asynchronous overrides),
    // Q and ~Q on the right.
    inputPort(0)->setPos( 0, 16);     inputPort(0)->setName("Data");
    inputPort(1)->setPos( 0, 48);     inputPort(1)->setName("Clock");
    inputPort(2)->setPos(32,  0);     inputPort(2)->setName("~Preset");
    inputPort(3)->setPos(32, 64);     inputPort(3)->setName("~Clear");

    outputPort(0)->setPos(64, 16);    outputPort(0)->setName("Q");
    outputPort(1)->setPos(64, 48);    outputPort(1)->setName("~Q");

    // ~Preset and ~Clear are optional; when unconnected they default to Active (HIGH = not asserted)
    // because these signals are active-low — leaving them floating means "no override"
    inputPort(2)->setRequired(false);
    inputPort(3)->setRequired(false);

    inputPort(2)->setDefaultStatus(Status::Active);
    inputPort(3)->setDefaultStatus(Status::Active);

    // Initial state: Q=0, ~Q=1 (reset/cleared state)
    outputPort(0)->setDefaultStatus(Status::Inactive);
    outputPort(1)->setDefaultStatus(Status::Active);
}

void DFlipFlop::updateTheme()
{
    // Reload the pixmap before delegating to the base class (see SRFlipFlop.cpp).
    setPixmap(pixmapPath());
    GraphicElement::updateTheme();
}

