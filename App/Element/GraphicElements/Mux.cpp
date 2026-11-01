// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>

#include "App/Core/Constants.h"
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Simulation/KernelLogic.h"

template<>
struct ElementInfo<ElementType::Mux> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Mux,
        .group = ElementGroup::Mux,
        .minInputSize = 3,
        .maxInputSize = 11,
        .minOutputSize = 1,
        .maxOutputSize = 1,
    };
    static_assert(validate(constraints));

    /// The body's height for \a element's data inputs, a multiple of 16 of at least 64: tall enough to fit the data
    /// ports with padding.
    static int bodyHeight(const Element &element)
    {
        const int step = Constants::gridSize / 2; // 8
        const int dataInputs = element.inputSize() - KernelLogic::muxSelectLines(element.inputSize());
        const int dataPortsSpan = (dataInputs - 1) * step * 2; // spacing between first and last port
        const int height = (std::max)(64, dataPortsSpan + step * 6);
        return ((height + 15) / 16) * 16;
    }

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.kernel = KernelKind::Mux;
        descriptor.pixmapPath = QStringLiteral(":/Components/Logic/mux.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("Mux", "MULTIPLEXER");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Mux", "Mux");
        descriptor.trContext = "Mux";
        descriptor.appearanceSlots = {{"body", ":/Components/Logic/mux.svg"}};
        descriptor.inputAttributes = [](const Element &element, const int index) {
            const int dataInputs = element.inputSize() - KernelLogic::muxSelectLines(element.inputSize());
            const int step = Constants::gridSize / 2; // 8
            const int height = bodyHeight(element);

            // Data inputs on the left, centred vertically
            if (index < dataInputs) {
                const int y = (height / 2) - ((dataInputs - 1) * step) + (index * step * 2);
                return PortAttributes{.name = QString("In%1").arg(index), .position = QPointF(16, y)};
            }

            // Select lines at the bottom, spaced by gridSize to avoid overlap
            const int select = index - dataInputs;
            return PortAttributes{.name = QString("S%1").arg(select),
                                  .position = QPointF(32 + (select * step * 2), height - step)};
        };
        descriptor.outputAttributes = [](const Element &element, const int index) {
            // The output on the right, centred
            return Element::attributesAt({{.name = "Out", .position = QPointF(48, bodyHeight(element) / 2)}}, index);
        };
        descriptor.bodyFollowsPorts = true;
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Element(constraints.type); });
        return true;
    }();
};
