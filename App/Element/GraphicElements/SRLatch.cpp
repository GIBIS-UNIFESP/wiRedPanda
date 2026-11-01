// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<ElementType::SRLatch> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::SRLatch,
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
        descriptor.kernel = KernelKind::SRLatch;
        // The Light theme's picture; presentation shows its Dark sibling in the Dark theme (ElementPictures::themed())
        descriptor.pixmapPath = QStringLiteral(":/Components/Memory/Light/SR-latch.svg");
        descriptor.appearanceSlots = {{"body", descriptor.pixmapPath}};
        descriptor.titleText = QT_TRANSLATE_NOOP("SRLatch", "SR-LATCH");
        descriptor.translatedName = QT_TRANSLATE_NOOP("SRLatch", "SR-Latch");
        descriptor.trContext = "SRLatch";
        descriptor.inputAttributes = [](const Element &, const int index) {
            // SR latch: no clock, no asynchronous overrides — simplest bistable element.
            // Both inputs are required (Set=1 → Q=1, Reset=1 → Q=0, both=1 is forbidden).
            static const QList<PortAttributes> attributes{
                {.name = "Set", .position = QPointF(0, 16)},
                {.name = "Reset", .position = QPointF(0, 48)},
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
