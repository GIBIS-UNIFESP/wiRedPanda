// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/InputGND.h"

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<InputGnd> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::InputGnd,
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
        // Static inputs are not rotatable as a whole; instead their output port rotates
        // around the element centre so the wire attachment point tracks the correct position.
        .rotatable = false,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Input/0.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("InputGnd", "GROUND");
        meta.translatedName = QT_TRANSLATE_NOOP("InputGnd", "GND");
        meta.trContext = "InputGnd";
        // Seed skin lists from the constructor-supplied pixmap path (see And.cpp for details).
        meta.defaultSkins = QStringList({":/Components/Input/0.svg"});
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new InputGnd(); });
        return true;
    }();
};

InputGnd::InputGnd(QGraphicsItem *parent)
    : GraphicElement(ElementType::InputGnd, parent)
{
}

