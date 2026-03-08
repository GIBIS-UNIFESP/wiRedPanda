// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Xor.h"

#include "App/Element/ElementInfo.h"
#include "App/Element/LogicElements/LogicXor.h"

template<>
struct ElementInfo<Xor> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Xor,
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
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Logic/xor.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Xor", "XOR");
        meta.translatedName = QT_TRANSLATE_NOOP("Xor", "Xor");
        meta.trContext = "Xor";
        // Seed skin lists from the constructor-supplied pixmap path (see And.cpp for details).
        meta.defaultSkins = QStringList({":/Components/Logic/xor.svg"});
        meta.logicCreator = [](GraphicElement *elm) { return std::make_shared<LogicXor>(elm->inputSize()); };
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new Xor(); });
        return true;
    }();
};

Xor::Xor(QGraphicsItem *parent)
    : GraphicElement(ElementType::Xor, parent)
{
    // Skip full initialisation when building a property-probe instance (see ElementFactory).
}
