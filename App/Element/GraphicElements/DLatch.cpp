// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<ElementType::DLatch> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::DLatch,
        .group = ElementGroup::Memory,
        .minInputSize = 2,
        .maxInputSize = 2,
        .minOutputSize = 2,
        .maxOutputSize = 2,
        .canChangeAppearance = true,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.kernel = KernelKind::DLatch;
        // The Light theme's picture; presentation shows its Dark sibling in the Dark theme (ElementPictures::themed())
        descriptor.pixmapPath = QStringLiteral(":/Components/Memory/Light/D-latch.svg");
        descriptor.appearanceSlots = {{"body", descriptor.pixmapPath}};
        descriptor.titleText = QT_TRANSLATE_NOOP("DLatch", "D-LATCH");
        descriptor.translatedName = QT_TRANSLATE_NOOP("DLatch", "D-Latch");
        descriptor.trContext = "DLatch";
        descriptor.inputAttributes = [](const Element &, const int index) {
            // D latch has no clock — it is level-sensitive: Q tracks D while Enable is HIGH.
            // No asynchronous preset/clear inputs, so only 2 inputs total.
            static const QList<PortAttributes> attributes{
                {.name = "Data", .position = QPointF(0, 16)},
                {.name = "Enable", .position = QPointF(0, 48)},
            };
            return Element::attributesAt(attributes, index);
        };
        descriptor.outputAttributes = [](const Element &, const int index) {
            // Initial state: Q=0, ~Q=1
            static const QList<PortAttributes> attributes{
                {.name = "Q", .defaultStatus = Status::Inactive, .position = QPointF(64, 16)},
                {.name = "~Q", .defaultStatus = Status::Active, .position = QPointF(64, 48)},
            };
            return Element::attributesAt(attributes, index);
        };
        return descriptor;
    }

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Element(constraints.type); });
        return true;
    }();
};
