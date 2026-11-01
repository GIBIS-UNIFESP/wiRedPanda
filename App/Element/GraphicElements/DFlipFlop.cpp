// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<ElementType::DFlipFlop> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::DFlipFlop,
        .group = ElementGroup::Memory,
        .minInputSize = 4,
        .maxInputSize = 4,
        .minOutputSize = 2,
        .maxOutputSize = 2,
        .canChangeAppearance = true,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.kernel = KernelKind::DFlipFlop;
        // The Light theme's picture; presentation shows its Dark sibling in the Dark theme (ElementPictures::themed())
        descriptor.pixmapPath = QStringLiteral(":/Components/Memory/Light/D-flipflop.svg");
        descriptor.appearanceSlots = {{"body", descriptor.pixmapPath}};
        descriptor.titleText = QT_TRANSLATE_NOOP("DFlipFlop", "D-FLIP-FLOP");
        descriptor.translatedName = QT_TRANSLATE_NOOP("DFlipFlop", "D-Flip-Flop");
        descriptor.trContext = "DFlipFlop";
        descriptor.inputAttributes = [](const Element &, const int index) {
            // Standard D flip-flop pin layout: D and Clock on the left,
            // active-low ~Preset and ~Clear on top/bottom (asynchronous overrides).
            // ~Preset and ~Clear are optional; when unconnected they default to Active (HIGH = not asserted)
            // because these signals are active-low — leaving them floating means "no override"
            static const QList<PortAttributes> attributes{
                {.name = "Data", .position = QPointF(0, 16)},
                {.name = "Clock", .position = QPointF(0, 48)},
                {.name = "~Preset", .required = false, .defaultStatus = Status::Active, .position = QPointF(32, 0)},
                {.name = "~Clear", .required = false, .defaultStatus = Status::Active, .position = QPointF(32, 64)},
            };
            return Element::attributesAt(attributes, index);
        };
        descriptor.outputAttributes = [](const Element &, const int index) {
            // Initial state: Q=0, ~Q=1 (reset/cleared state)
            static const QList<PortAttributes> attributes{
                {.name = "Q", .defaultStatus = Status::Inactive, .position = QPointF(64, 16)},
                {.name = "~Q", .defaultStatus = Status::Active, .position = QPointF(64, 48)},
            };
            return Element::attributesAt(attributes, index);
        };
        // Its kernel's history at power-on: last clock, last D
        descriptor.kernelState = {Status::Inactive, Status::Active};
        return descriptor;
    }

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Element(constraints.type); });
        return true;
    }();
};
