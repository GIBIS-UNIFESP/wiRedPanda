// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Text.h"

#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<Text> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Text,
        .group = ElementGroup::Other,
        .hasLabel = true,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Misc/text.png"); };
        meta.titleText = QT_TRANSLATE_NOOP("Text", "TEXT");
        meta.translatedName = QT_TRANSLATE_NOOP("Text", "Text");
        meta.trContext = "Text";
        // Text has two built-in skins:
        //   index 0 — transparent placeholder shown when the label is empty
        //   index 1 — solid background shown when the label has content
        meta.defaultSkins = QStringList({
            ":/Components/Misc/no_text.png",
            ":/Components/Misc/text.png",
        });
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new Text(); });
        return true;
    }();
};

// Text is a purely decorative annotation element with no ports and no simulation role.
Text::Text(QGraphicsItem *parent)
    : GraphicElement(ElementType::Text, parent)
{
}

