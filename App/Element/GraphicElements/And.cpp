// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/And.h"

#include "App/Element/ElementInfo.h"
#include "App/Element/LogicElements/LogicAnd.h"

template<>
struct ElementInfo<And> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::And,
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
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Logic/and.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("And", "AND");
        meta.translatedName = QT_TRANSLATE_NOOP("And", "And");
        meta.trContext = "And";
        // m_pixmapPath was set by the base-class constructor from the argument above.
        // Seed both skin lists from it so that index 0 always points to the default art.
        // m_alternativeSkins starts as a copy of m_defaultSkins; the user can replace
        // entries in m_alternativeSkins without losing the originals in m_defaultSkins.
        meta.defaultSkins = QStringList({":/Components/Logic/and.svg"});
        meta.logicCreator = [](GraphicElement *elm) { auto e = std::make_shared<LogicAnd>(elm->inputSize()); e->setPropagationDelay(10); return e; };
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new And(); });
        return true;
    }();
};

And::And(QGraphicsItem *parent)
    : GraphicElement(ElementType::And, parent)
{
    // skipInit is set by ElementFactory::ensurePropertiesCached() when it needs
    // to build a throw-away instance just to read static Q_PROPERTY values.
    // Returning early avoids expensive pixmap loads and port allocation for that case.
}
