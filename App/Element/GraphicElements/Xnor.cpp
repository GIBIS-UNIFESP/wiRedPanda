// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Xnor.h"

#include "App/Element/ElementInfo.h"
#include "App/Element/LogicElements/LogicXnor.h"

template<>
struct ElementInfo<Xnor> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Xnor,
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
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Logic/xnor.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Xnor", "XNOR");
        meta.translatedName = QT_TRANSLATE_NOOP("Xnor", "Xnor");
        meta.trContext = "Xnor";
        // Seed skin lists from the constructor-supplied pixmap path (see And.cpp for details).
        meta.defaultSkins = QStringList({":/Components/Logic/xnor.svg"});
        meta.logicCreator = [](GraphicElement *elm) { auto e = std::make_shared<LogicXnor>(elm->inputSize()); e->setPropagationDelay(15); return e; };
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new Xnor(); });
        return true;
    }();
};

Xnor::Xnor(QGraphicsItem *parent)
    : GraphicElement(ElementType::Xnor, parent)
{
    // Skip full initialisation when building a property-probe instance (see ElementFactory).
}
