// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<ElementType::JKFlipFlop> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::JKFlipFlop,
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
        descriptor.kernel = KernelKind::JKFlipFlop;
        // The Light theme's picture; presentation shows its Dark sibling in the Dark theme (ElementPictures::themed())
        descriptor.pixmapPath = QStringLiteral(":/Components/Memory/Light/JK-flipflop.svg");
        descriptor.appearanceSlots = {{"body", descriptor.pixmapPath}};
        descriptor.titleText = QT_TRANSLATE_NOOP("JKFlipFlop", "JK-FLIP-FLOP");
        descriptor.translatedName = QT_TRANSLATE_NOOP("JKFlipFlop", "JK-Flip-Flop");
        descriptor.trContext = "JKFlipFlop";
        descriptor.inputAttributes = [](const Element &, const int index) {
            // JK flip-flop: J (Set-like) and K (Reset-like) are clocked, with Clock required.
            // J=1,K=1 on a clock edge toggles output — making the JK superior to SR (no forbidden state).
            // ~Preset and ~Clear are asynchronous active-low overrides (same as DFlipFlop).
            // J and K default to Active (HIGH) when unconnected, so the flip-flop toggles on every
            // clock edge by default — a useful demonstration behavior for the educational context.
            // ~Preset and ~Clear default to Active (HIGH = not asserted).
            static const QList<PortAttributes> attributes{
                {.name = "J", .required = false, .defaultStatus = Status::Active, .position = QPointF(0, 16)},
                {.name = "Clock", .position = QPointF(0, 32)},
                {.name = "K", .required = false, .defaultStatus = Status::Active, .position = QPointF(0, 48)},
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
        // Its kernel's history at power-on: last clock, last J, last K
        descriptor.kernelState = {Status::Inactive, Status::Active, Status::Active};
        return descriptor;
    }

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Element(constraints.type); });
        return true;
    }();
};
