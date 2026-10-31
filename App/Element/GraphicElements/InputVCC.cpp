// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/InputVCC.h"

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Wiring/Port.h"

template<>
struct ElementInfo<InputVcc> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::InputVcc,
        .group = ElementGroup::StaticInput,
        .minOutputSize = 1,
        .maxOutputSize = 1,
        .canChangeAppearance = true,
        // Static inputs are not rotatable as a whole (see InputGND.cpp for explanation).
        .rotatesGraphic = false,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = QStringLiteral(":/Components/Input/1.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("InputVcc", "VCC");
        descriptor.translatedName = QT_TRANSLATE_NOOP("InputVcc", "VCC");
        descriptor.trContext = "InputVcc";
        // Seed appearance lists from the constructor-supplied pixmap path (see And.cpp for details).
        descriptor.defaultAppearances = QStringList({":/Components/Input/1.svg"});
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new InputVcc(); });
        return true;
    }();
};

InputVcc::InputVcc(QGraphicsItem *parent)
    : Element(ElementType::InputVcc, parent)
{
    // VCC always outputs logic HIGH; set the port default so both the visual
    // wire colour and the simulation output vector are correct from the start.
    outputs().constFirst()->setDefaultStatus(Status::Active);
}
