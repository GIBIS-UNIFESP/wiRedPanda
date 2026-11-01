// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<ElementType::Xnor> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Xnor,
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
        descriptor.kernel = KernelKind::Xnor;
        descriptor.pixmapPath = QStringLiteral(":/Components/Logic/xnor.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("Xnor", "XNOR");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Xnor", "Xnor");
        descriptor.trContext = "Xnor";
        descriptor.appearanceSlots = {{"body", ":/Components/Logic/xnor.svg"}};
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Element(constraints.type); });
        return true;
    }();
};
