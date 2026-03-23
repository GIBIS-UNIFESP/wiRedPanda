// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/DLatch.h"

#include "App/Element/ElementInfo.h"
#include "App/Nodes/QNEPort.h"

template<>
struct ElementInfo<DLatch> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::DLatch,
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
        meta.pixmapPath = []{ return DLatch::pixmapPath(); };
        meta.titleText = QT_TRANSLATE_NOOP("DLatch", "D-LATCH");
        meta.translatedName = QT_TRANSLATE_NOOP("DLatch", "D-Latch");
        meta.trContext = "DLatch";
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new DLatch(); });
        return true;
    }();
};

DLatch::DLatch(QGraphicsItem *parent)
    : GraphicElement(ElementType::DLatch, parent)
{
    // Call the most-derived override explicitly (see SRFlipFlop.cpp for rationale).
    DLatch::updatePortsProperties();
}

void DLatch::updatePortsProperties()
{
    // D latch has no clock — it is level-sensitive: Q tracks D while Enable is HIGH.
    // No asynchronous preset/clear inputs, so only 2 inputs total.
    inputPort(0)->setPos(0, 16);      inputPort(0)->setName("Data");
    inputPort(1)->setPos(0, 48);      inputPort(1)->setName("Enable");

    outputPort(0)->setPos(64, 16);    outputPort(0)->setName("Q");
    outputPort(1)->setPos(64, 48);    outputPort(1)->setName("~Q");

    // Initial state: Q=0, ~Q=1
    outputPort(0)->setDefaultStatus(Status::Inactive);
    outputPort(1)->setDefaultStatus(Status::Active);
}

void DLatch::updateTheme()
{
    // Reload the pixmap before delegating to the base class (see SRFlipFlop.cpp).
    setPixmap(pixmapPath());
    GraphicElement::updateTheme();
}

void DLatch::updateLogic()
{
    if (!updateInputs()) {
        return;
    }
    bool q0 = outputValue(0);
    bool q1 = outputValue(1);
    const bool D = simInputs().at(0);
    const bool enable = simInputs().at(1);

    if (enable) {
        q0 = D;
        q1 = !D;
    }
    setOutputValue(0, q0);
    setOutputValue(1, q1);
}

