// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Nand.h"

#include "App/Core/StatusOps.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<Nand> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Nand,
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
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Logic/nand.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Nand", "NAND");
        meta.translatedName = QT_TRANSLATE_NOOP("Nand", "Nand");
        meta.trContext = "Nand";
        // Seed skin lists from the constructor-supplied pixmap path (see And.cpp for details).
        meta.defaultSkins = QStringList({":/Components/Logic/nand.svg"});
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new Nand(); });
        return true;
    }();
};

Nand::Nand(QGraphicsItem *parent)
    : GraphicElement(ElementType::Nand, parent)
{
    // Skip full initialisation when building a property-probe instance (see ElementFactory).
}

void Nand::updateLogic()
{
    if (!simUpdateInputsAllowUnknown()) {
        return;
    }
    setOutputValue(StatusOps::statusNot(StatusOps::statusAndAll(simInputs())));
}

