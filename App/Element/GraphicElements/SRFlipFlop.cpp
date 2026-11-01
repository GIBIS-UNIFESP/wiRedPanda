// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<ElementType::SRFlipFlop> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::SRFlipFlop,
        .group = ElementGroup::Memory,
        .minInputSize = 5,
        .maxInputSize = 5,
        .minOutputSize = 2,
        .maxOutputSize = 2,
        .canChangeAppearance = true,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.kernel = KernelKind::SRFlipFlop;
        // The Light theme's picture; presentation shows its Dark sibling in the Dark theme (ElementPictures::themed())
        descriptor.pixmapPath = QStringLiteral(":/Components/Memory/Light/SR-flipflop.svg");
        descriptor.appearanceSlots = {{"body", descriptor.pixmapPath}};
        descriptor.titleText = QT_TRANSLATE_NOOP("SRFlipFlop", "SR-FLIP-FLOP");
        descriptor.translatedName = QT_TRANSLATE_NOOP("SRFlipFlop", "SR-Flip-Flop");
        descriptor.trContext = "SRFlipFlop";
        descriptor.inputAttributes = [](const Element &, const int index) {
            // SR flip-flop: S (Set) and R (Reset) are clocked inputs; Clock is required.
            // ~Preset and ~Clear provide asynchronous overrides (active-low, same as DFlipFlop).
            // S, R, ~Preset, ~Clear are optional; only Clock is always required.
            // S and R unconnected default to Inactive (no set/reset action on clock edge).
            // ~Preset and ~Clear unconnected default to Active (HIGH = not asserted).
            static const QList<PortAttributes> attributes{
                {.name = "S", .required = false, .defaultStatus = Status::Inactive, .position = QPointF(0, 16)},
                {.name = "Clock", .position = QPointF(0, 32)},
                {.name = "R", .required = false, .defaultStatus = Status::Inactive, .position = QPointF(0, 48)},
                {.name = "~Preset", .required = false, .defaultStatus = Status::Active, .position = QPointF(32, 0)},
                {.name = "~Clear", .required = false, .defaultStatus = Status::Active, .position = QPointF(32, 64)},
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
        // Its kernel's history at power-on: last clock, last S, last R
        descriptor.kernelState = {Status::Inactive, Status::Inactive, Status::Inactive};
        return descriptor;
    }

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Element(constraints.type); });
        return true;
    }();
};
