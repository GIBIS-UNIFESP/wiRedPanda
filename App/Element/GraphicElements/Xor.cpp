// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Xor.h"

#include "App/Core/StatusOps.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<Xor> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Xor,
        .group = ElementGroup::Gate,
        .minInputSize = 2,
        .maxInputSize = 8,
        .minOutputSize = 1,
        .maxOutputSize = 1,
        .canChangeAppearance = true,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = []{ return QStringLiteral(":/Components/Logic/xor.svg"); };
        descriptor.titleText = QT_TRANSLATE_NOOP("Xor", "XOR");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Xor", "Xor");
        descriptor.trContext = "Xor";
        // Seed appearance lists from the constructor-supplied pixmap path (see And.cpp for details).
        descriptor.defaultAppearances = QStringList({":/Components/Logic/xor.svg"});
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Xor(); });
        return true;
    }();
};

Xor::Xor(QGraphicsItem *parent)
    : Element(ElementType::Xor, parent)
{
    // Skip full initialisation when building a property-probe instance (see ElementFactory).
}

void Xor::updateLogic()
{
    if (!simUpdateInputsAllowUnknown()) {
        return;
    }
    setOutputValue(StatusOps::statusXorAll(simInputs()));
}
