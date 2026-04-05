// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Or.h"

#include "App/Core/StatusOps.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<Or> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Or,
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
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Logic/or.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Or", "OR");
        meta.translatedName = QT_TRANSLATE_NOOP("Or", "Or");
        meta.trContext = "Or";
        // Seed appearance lists from the constructor-supplied pixmap path (see And.cpp for details).
        meta.defaultAppearances = QStringList({":/Components/Logic/or.svg"});
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

void Or::updateLogic()
{
    if (!simUpdateInputsAllowUnknown()) {
        return;
    }
    setOutputValue(StatusOps::statusOrAll(simInputs()));
}

