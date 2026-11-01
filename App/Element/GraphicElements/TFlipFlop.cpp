// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"

template<>
struct ElementInfo<ElementType::TFlipFlop> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::TFlipFlop,
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
        descriptor.kernel = KernelKind::TFlipFlop;
        // The Light theme's picture; presentation shows its Dark sibling in the Dark theme (ElementPictures::themed())
        descriptor.pixmapPath = QStringLiteral(":/Components/Memory/Light/T-flipflop.svg");
        descriptor.appearanceSlots = {{"body", descriptor.pixmapPath}};
        descriptor.titleText = QT_TRANSLATE_NOOP("TFlipFlop", "T-FLIP-FLOP");
        descriptor.translatedName = QT_TRANSLATE_NOOP("TFlipFlop", "T-Flip-Flop");
        descriptor.trContext = "TFlipFlop";
        descriptor.inputAttributes = [](const Element &, const int index) {
            // T (Toggle) flip-flop: toggles Q on each rising clock edge when T=1; holds when T=0.
            // ~Preset and ~Clear are asynchronous active-low overrides (same convention as DFlipFlop).
            // T is optional; when unconnected its default Inactive (LOW) means "hold" on every clock edge.
            // ~Preset and ~Clear default to Active (HIGH = not asserted).
            static const QList<PortAttributes> attributes{
                {.name = "T", .required = false, .defaultStatus = Status::Inactive, .position = QPointF(0, 16)},
                {.name = "Clock", .position = QPointF(0, 48)},
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
        // Its kernel's history at power-on: last clock, last T
        descriptor.kernelState = {Status::Inactive, Status::Active};
        return descriptor;
    }

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Element(constraints.type); });
        return true;
    }();
};
