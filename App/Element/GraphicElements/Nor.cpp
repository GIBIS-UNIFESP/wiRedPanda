// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Nor.h"

#include <functional>

#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<Nor> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Nor,
        .group = ElementGroup::Gate,
        .minInputSize = 2,
        .maxInputSize = 8,
        .minOutputSize = 1,
        .maxOutputSize = 1,
        .canChangeSkin = true,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Logic/nor.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Nor", "NOR");
        meta.translatedName = QT_TRANSLATE_NOOP("Nor", "Nor");
        meta.trContext = "Nor";
        // Seed skin lists from the constructor-supplied pixmap path (see And.cpp for details).
        meta.defaultSkins = QStringList({":/Components/Logic/nor.svg"});
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new Nor(); });
        return true;
    }();
};

Nor::Nor(QGraphicsItem *parent)
    : GraphicElement(ElementType::Nor, parent)
{
    // Skip full initialisation when building a property-probe instance (see ElementFactory).
}

void Nor::updateLogic()
{
    if (!updateInputs()) {
        return;
    }
    const auto result = std::accumulate(simInputs().cbegin(), simInputs().cend(), false, std::bit_or<>());
    setOutputValue(!result);
}

