// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/SRLatch.h"

#include "App/Element/ElementInfo.h"
#include "App/Element/LogicElements/LogicSRLatch.h"
#include "App/Nodes/QNEPort.h"

template<>
struct ElementInfo<SRLatch> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::SRLatch,
        .group = ElementGroup::Memory,
        .minInputSize = 2,
        .maxInputSize = 2,
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
        meta.pixmapPath = []{ return SRLatch::pixmapPath(); };
        meta.titleText = QT_TRANSLATE_NOOP("SRLatch", "SR-LATCH");
        meta.translatedName = QT_TRANSLATE_NOOP("SRLatch", "SR-Latch");
        meta.trContext = "SRLatch";
        meta.defaultSkins = QStringList({":/Components/Memory/Dark/SR-latch.svg"});
        meta.logicCreator = [](GraphicElement *) { return std::make_shared<LogicSRLatch>(); };
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new SRLatch(); });
        return true;
    }();
};

SRLatch::SRLatch(QGraphicsItem *parent)
    : GraphicElement(ElementType::SRLatch, parent)
{
    // Call the most-derived override explicitly (see SRFlipFlop.cpp for rationale).
    SRLatch::updatePortsProperties();
}

void SRLatch::updatePortsProperties()
{
    // SR latch: no clock, no asynchronous overrides — simplest bistable element.
    // Both inputs are required (Set=1 → Q=1, Reset=1 → Q=0, both=1 is forbidden).
    inputPort(0)->setPos(0, 16);      inputPort(0)->setName("Set");
    inputPort(1)->setPos(0, 48);      inputPort(1)->setName("Reset");

    outputPort(0)->setPos(64, 16);    outputPort(0)->setName("Q");
    outputPort(1)->setPos(64, 48);    outputPort(1)->setName("~Q");

    // Initial state: Q=0, ~Q=1
    outputPort(0)->setDefaultStatus(Status::Inactive);
    outputPort(1)->setDefaultStatus(Status::Active);
}

void SRLatch::updateTheme()
{
    // Reload the pixmap before delegating to the base class (see SRFlipFlop.cpp).
    setPixmap(pixmapPath());
    GraphicElement::updateTheme();
}
