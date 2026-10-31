// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Not.h"

#include "App/Core/StatusOps.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<Not> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Not,
        .group = ElementGroup::Gate,
        .minInputSize = 1,
        .maxInputSize = 1,
        .minOutputSize = 1,
        .maxOutputSize = 1,
        .canChangeAppearance = true,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = QStringLiteral(":/Components/Logic/not.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("Not", "NOT");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Not", "Not");
        descriptor.trContext = "Not";
        // Seed appearance lists from the constructor-supplied pixmap path (see And.cpp for details).
        descriptor.defaultAppearances = QStringList({":/Components/Logic/not.svg"});
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Not(); });
        return true;
    }();
};

Not::Not(QGraphicsItem *parent)
    : Element(ElementType::Not, parent)
{
    // Skip full initialisation when building a property-probe instance (see ElementFactory).
}

void Not::updateLogic()
{
    if (!simUpdateInputsAllowUnknown()) {
        return;
    }
    setOutputValue(StatusOps::statusNot(simInputs().at(0)));
}
