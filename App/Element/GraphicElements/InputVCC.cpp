// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/InputVCC.h"

#include "App/Element/ElementInfo.h"
#include "App/Nodes/QNEPort.h"

template<>
struct ElementInfo<InputVcc> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::InputVcc,
        .group = ElementGroup::StaticInput,
        .minInputSize = 0,
        .maxInputSize = 0,
        .minOutputSize = 1,
        .maxOutputSize = 1,
        .canChangeSkin = true,
        .hasColors = false,
        .hasAudio = false,
        .hasAudioBox = false,
        .hasTrigger = false,
        .hasFrequency = false,
        .hasDelay = false,
        .hasLabel = false,
        .hasTruthTable = false,
        // Static inputs are not rotatable as a whole (see InputGND.cpp for explanation).
        .rotatable = false,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Input/1.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("InputVcc", "VCC");
        meta.translatedName = QT_TRANSLATE_NOOP("InputVcc", "VCC");
        meta.trContext = "InputVcc";
        meta.defaultSkins = QStringList({":/Components/Input/1.svg"});
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new InputVcc(); });
        return true;
    }();
};

InputVcc::InputVcc(QGraphicsItem *parent)
    : GraphicElement(ElementType::InputVcc, parent)
{
    // VCC always outputs logic HIGH; set the port default so both the visual
    // wire colour and the simulation output vector are correct from the start.
    m_outputPorts.constFirst()->setDefaultStatus(Status::Active);
}

