// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Text.h"

#include "App/Element/ElementInfo.h"
#include "App/Element/LogicElements/LogicNone.h"

template<>
struct ElementInfo<Text> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Text,
        .group = ElementGroup::Other,
        .minInputSize = 0,
        .maxInputSize = 0,
        .minOutputSize = 0,
        .maxOutputSize = 0,
        .canChangeSkin = false,
        .hasColors = false,
        .hasAudio = false,
        .hasAudioBox = false,
        .hasTrigger = false,
        .hasFrequency = false,
        .hasDelay = false,
        .hasLabel = true,
        .hasTruthTable = false,
        .rotatable = true,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Misc/text.png"); };
        meta.titleText = QT_TRANSLATE_NOOP("Text", "TEXT");
        meta.translatedName = QT_TRANSLATE_NOOP("Text", "Text");
        meta.trContext = "Text";
        meta.defaultSkins = QStringList({
            ":/Components/Misc/no_text.png",
            ":/Components/Misc/text.png",
        });
        meta.logicCreator = [](GraphicElement *) { return std::make_shared<LogicNone>(); };
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new Text(); });
        return true;
    }();
};

// Text is a purely decorative annotation element with no ports and no simulation role.
// It is mapped to LogicNone by ElementFactory::buildLogicElement().
Text::Text(QGraphicsItem *parent)
    : GraphicElement(ElementType::Text, parent)
{
    // Text has two built-in skins:
    //   index 0 — transparent placeholder shown when the label is empty
    //   index 1 — solid background shown when the label has content
    // Unlike other elements, the skin list is initialised with a QStringList literal
    // rather than appending m_pixmapPath, because the two paths differ from the
    // constructor argument (which is just used for the palette thumbnail).

    // Label must be visible by default because the text body IS the label content.
}
