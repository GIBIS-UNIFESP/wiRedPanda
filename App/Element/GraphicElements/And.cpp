// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/And.h"

#include "App/Core/StatusOps.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<And> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::And,
        .group = ElementGroup::Gate,
        .minInputSize = 2,
        .maxInputSize = 8,
        .minOutputSize = 1,
        .maxOutputSize = 1,
        .canChangeAppearance = true,
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
        // Seed both appearance lists from it so that index 0 always points to the default art.
        // m_alternativeAppearances starts as a copy of m_defaultAppearances; the user can replace
        // entries in m_alternativeAppearances without losing the originals in m_defaultAppearances.
        meta.defaultAppearances = QStringList({":/Components/Logic/and.svg"});
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
}

void And::updateLogic()
{
    if (!simUpdateInputsAllowUnknown()) {
        return;
    }
    setOutputValue(StatusOps::statusAndAll(simInputs()));
}

