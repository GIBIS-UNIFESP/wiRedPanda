// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Not.h"

#include "App/Element/ElementInfo.h"
#include "App/Element/LogicElements/LogicNot.h"

template<>
struct ElementInfo<Not> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Not,
        .group = ElementGroup::Gate,
        .minInputSize = 1,
        .maxInputSize = 1,
        .minOutputSize = 1,
        .maxOutputSize = 1,
        .canChangeSkin = true,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Logic/not.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Not", "NOT");
        meta.translatedName = QT_TRANSLATE_NOOP("Not", "Not");
        meta.trContext = "Not";
        // Seed skin lists from the constructor-supplied pixmap path (see And.cpp for details).
        meta.defaultSkins = QStringList({":/Components/Logic/not.svg"});
        meta.logicCreator = [](GraphicElement *) { return std::make_shared<LogicNot>(); };
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new Not(); });
        return true;
    }();
};

Not::Not(QGraphicsItem *parent)
    : GraphicElement(ElementType::Not, parent)
{
    // Skip full initialisation when building a property-probe instance (see ElementFactory).
}
