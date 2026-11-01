// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<ElementType::InputVcc> {
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
        descriptor.kernel = KernelKind::Constant;
        descriptor.pixmapPath = QStringLiteral(":/Components/Input/1.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("InputVcc", "VCC");
        descriptor.translatedName = QT_TRANSLATE_NOOP("InputVcc", "VCC");
        descriptor.trContext = "InputVcc";
        descriptor.appearanceSlots = {{"body", ":/Components/Input/1.svg"}};
        // VCC always outputs logic HIGH: its port's default gives the wire its colour and the simulation its output
        // from the start
        descriptor.outputAttributes = [](const Element &, const int index) {
            static const QList<PortAttributes> attributes{{.defaultStatus = Status::Active}};
            return Element::attributesAt(attributes, index);
        };
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Element(constraints.type); });
        return true;
    }();
};
