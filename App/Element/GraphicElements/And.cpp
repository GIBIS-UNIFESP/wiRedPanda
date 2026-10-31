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

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = QStringLiteral(":/Components/Logic/and.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("And", "AND");
        descriptor.translatedName = QT_TRANSLATE_NOOP("And", "And");
        descriptor.trContext = "And";
        // m_pixmapPath was set by the base-class constructor from the argument above.
        // Seed both appearance lists from it so that index 0 always points to the default art.
        // m_alternativeAppearances starts as a copy of m_defaultAppearances; the user can replace
        // entries in m_alternativeAppearances without losing the originals in m_defaultAppearances.
        descriptor.defaultAppearances = QStringList({":/Components/Logic/and.svg"});
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new And(); });
        return true;
    }();
};

And::And(QGraphicsItem *parent)
    : Element(ElementType::And, parent)
{
}

void And::updateLogic()
{
    if (!simUpdateInputsAllowUnknown()) {
        return;
    }
    setOutputValue(StatusOps::statusAndAll(simInputs()));
}
