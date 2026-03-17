// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Or.h"

#include "App/Element/ElementInfo.h"
#include "App/Element/LogicElements/LogicOr.h"

template<>
struct ElementInfo<Or> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Or,
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
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Logic/or.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Or", "OR");
        meta.translatedName = QT_TRANSLATE_NOOP("Or", "Or");
        meta.trContext = "Or";
        // Seed skin lists from the constructor-supplied pixmap path (see And.cpp for details).
        meta.defaultSkins = QStringList({":/Components/Logic/or.svg"});
        meta.logicCreator = [](GraphicElement *elm) { auto e = std::make_shared<LogicOr>(elm->inputSize()); e->setPropagationDelay(10); return e; };
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new Or(); });
        return true;
    }();
};

Or::Or(QGraphicsItem *parent)
    : GraphicElement(ElementType::Or, parent)
{
    // Skip full initialisation when building a property-probe instance (see ElementFactory).
}
