// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>

#include "App/Core/Common.h"
#include "App/Core/Constants.h"
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Simulation/KernelLogic.h"

template<>
struct ElementInfo<ElementType::Demux> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Demux,
        .group = ElementGroup::Mux,
        .minInputSize = 2,
        .maxInputSize = 4,
        .minOutputSize = 2,
        .maxOutputSize = 8,
    };
    static_assert(validate(constraints));

    /// The body's height for \a element's outputs, a multiple of 16 of at least 64: tall enough to fit the output ports
    /// with padding.
    static int bodyHeight(const Element &element)
    {
        const int step = Constants::gridSize / 2; // 8
        const int outputPortsSpan = (element.outputSize() - 1) * step * 2;
        const int height = (std::max)(64, outputPortsSpan + step * 6);
        return ((height + 15) / 16) * 16;
    }

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.kernel = KernelKind::Demux;
        descriptor.pixmapPath = QStringLiteral(":/Components/Logic/demux.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("Demux", "DEMULTIPLEXER");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Demux", "Demux");
        descriptor.trContext = "Demux";
        descriptor.appearanceSlots = {{"body", ":/Components/Logic/demux.svg"}};
        descriptor.inputAttributes = [](const Element &element, const int index) -> std::optional<PortAttributes> {
            const int step = Constants::gridSize / 2; // 8
            const int height = bodyHeight(element);

            // The data input on the left, centred
            if (index == 0) {
                return PortAttributes{.name = "In", .position = QPointF(16, height / 2)};
            }

            // Select lines at the bottom, spaced by gridSize to avoid overlap
            const int select = index - 1;
            if (select >= KernelLogic::demuxSelectLines(element.outputSize())) {
                return std::nullopt;
            }
            return PortAttributes{.name = QString("S%1").arg(select),
                                  .position = QPointF(32 + (select * step * 2), height - step)};
        };
        descriptor.outputAttributes = [](const Element &element, const int index) {
            const int step = Constants::gridSize / 2; // 8

            // Outputs on the right, centred vertically
            const int y = (bodyHeight(element) / 2) - ((element.outputSize() - 1) * step) + (index * step * 2);
            return PortAttributes{.name = QString("Out%1").arg(index), .position = QPointF(48, y)};
        };
        descriptor.bodyFollowsPorts = true;
        // The data input, then the select lines the outputs need
        descriptor.inputsForOutputs = [](const int outputs) { return 1 + KernelLogic::demuxSelectLines(outputs); };
        descriptor.checkLoad = [](const Element &element) {
            // The loader takes the port lists as the file gives them, and the kernel reads every select line the
            // outputs need; a save always writes exactly those inputs
            const int expectedInputs = 1 + KernelLogic::demuxSelectLines(element.outputSize());
            if (element.inputSize() != expectedInputs) {
                throw PANDACEPTION_WITH_CONTEXT(
                    "Demux",
                    QT_TRANSLATE_NOOP("Demux",
                                      "%1 with %2 input ports for %3 outputs, where it has %4 — stream may be corrupt"),
                    element.translatedName(),
                    QString::number(element.inputSize()),
                    QString::number(element.outputSize()),
                    QString::number(expectedInputs));
            }
        };
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Element(constraints.type); });
        return true;
    }();
};
