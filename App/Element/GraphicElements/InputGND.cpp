// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<ElementType::InputGnd> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::InputGnd,
        .group = ElementGroup::StaticInput,
        .minOutputSize = 1,
        .maxOutputSize = 1,
        .canChangeAppearance = true,
        // Static inputs are not rotatable as a whole; instead their output port rotates
        // around the element centre so the wire attachment point tracks the correct position.
        .rotatesGraphic = false,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.kernel = KernelKind::Constant;
        descriptor.pixmapPath = QStringLiteral(":/Components/Input/0.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("InputGnd", "GROUND");
        descriptor.translatedName = QT_TRANSLATE_NOOP("InputGnd", "GND");
        descriptor.trContext = "InputGnd";
        descriptor.appearanceSlots = {{"body", ":/Components/Input/0.svg"}};
        // GND always outputs logic LOW: its port's default gives the wire its colour and the simulation its output
        // from the start
        descriptor.outputAttributes = [](const Element &, const int index) {
            static const QList<PortAttributes> attributes{{.defaultStatus = Status::Inactive}};
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
