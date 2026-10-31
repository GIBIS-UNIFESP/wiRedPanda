// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/DLatch.h"

#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Wiring/Port.h"

template<>
struct ElementInfo<DLatch> {
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
        // The Light theme's picture; presentation shows its Dark sibling in the Dark theme (ElementPictures::themed())
        descriptor.pixmapPath = QStringLiteral(":/Components/Memory/Light/D-latch.svg");
        descriptor.defaultAppearances = QStringList({descriptor.pixmapPath});
        descriptor.titleText = QT_TRANSLATE_NOOP("DLatch", "D-LATCH");
        descriptor.translatedName = QT_TRANSLATE_NOOP("DLatch", "D-Latch");
        descriptor.trContext = "DLatch";
        return descriptor;
    }

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new DLatch(); });
        return true;
    }();
};

DLatch::DLatch(QGraphicsItem *parent)
    : Element(ElementType::DLatch, parent)
{
    // The ports take their places, names, requirements and defaults from this type's attributes
    updatePortsProperties();
}

std::optional<Element::PortAttributes> DLatch::inputAttributes(const int index) const
{
    // D latch has no clock — it is level-sensitive: Q tracks D while Enable is HIGH.
    // No asynchronous preset/clear inputs, so only 2 inputs total.
    static const QList<PortAttributes> attributes{
        {.name = "Data", .position = QPointF(0, 16)},
        {.name = "Enable", .position = QPointF(0, 48)},
    };
    return attributesAt(attributes, index);
}

std::optional<Element::PortAttributes> DLatch::outputAttributes(const int index) const
{
    // Initial state: Q=0, ~Q=1
    static const QList<PortAttributes> attributes{
        {.name = "Q", .defaultStatus = Status::Inactive, .position = QPointF(64, 16)},
        {.name = "~Q", .defaultStatus = Status::Active, .position = QPointF(64, 48)},
    };
    return attributesAt(attributes, index);
}

void DLatch::updateLogic()
{
    if (!simUpdateInputs()) {
        return;
    }
    Status q0 = simOutputs().at(0);
    Status q1 = simOutputs().at(1);
    const Status D = simInputs().at(0);
    const Status enable = simInputs().at(1);

    if (enable == Status::Active) {
        q0 = D;
        q1 = (D == Status::Active) ? Status::Inactive : Status::Active;
    }

    setOutputValue(0, q0);
    setOutputValue(1, q1);
}
