// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Mux.h"

#include <cmath>

#include "App/Core/Constants.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Wiring/Port.h"

template<>
struct ElementInfo<Mux> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Mux,
        .group = ElementGroup::Mux,
        .minInputSize = 3,
        .maxInputSize = 11,
        .minOutputSize = 1,
        .maxOutputSize = 1,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = QStringLiteral(":/Components/Logic/mux.svg");
        descriptor.titleText = QT_TRANSLATE_NOOP("Mux", "MULTIPLEXER");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Mux", "Mux");
        descriptor.trContext = "Mux";
        descriptor.defaultAppearances = QStringList({":/Components/Logic/mux.svg"});
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Mux(); });
        return true;
    }();
};

Mux::Mux(QGraphicsItem *parent)
    : Element(ElementType::Mux, parent)
{
    updatePortsProperties();
}

void Mux::layoutPorts()
{
    Element::layoutPorts();
    pictureReplaced();
}

int Mux::bodyHeight() const
{
    const int step = Constants::gridSize / 2; // 8
    const int numDataInputs = inputSize() - calculateSelectLines(inputSize());

    // Tall enough to fit all data ports with padding, snapped to a multiple of 16
    const int dataPortsSpan = (numDataInputs - 1) * step * 2; // spacing between first and last port
    const int elementHeight = (std::max)(64, dataPortsSpan + step * 6);
    return ((elementHeight + 15) / 16) * 16;
}

std::optional<Element::PortAttributes> Mux::inputAttributes(const int index) const
{
    const int numSelectLines = calculateSelectLines(inputSize());
    const int numDataInputs = inputSize() - numSelectLines;
    const int step = Constants::gridSize / 2; // 8
    const int elementHeight = bodyHeight();

    // Data inputs on the left, centred vertically
    if (index < numDataInputs) {
        const int y = (elementHeight / 2) - ((numDataInputs - 1) * step) + (index * step * 2);
        return PortAttributes{.name = QString("In%1").arg(index), .position = QPointF(16, y)};
    }

    // Select lines at the bottom, spaced by gridSize to avoid overlap
    const int select = index - numDataInputs;
    return PortAttributes{.name = QString("S%1").arg(select), .position = QPointF(32 + (select * step * 2), elementHeight - step)};
}

std::optional<Element::PortAttributes> Mux::outputAttributes(const int index) const
{
    // The output on the right, centred
    return attributesAt({{.name = "Out", .position = QPointF(48, bodyHeight() / 2)}}, index);
}

void Mux::updateLogic()
{
    if (!simUpdateInputsAllowUnknown()) {
        return;
    }

    // Calculate select lines from current input count
    const int numSelectLines = calculateSelectLines(inputSize());
    int numDataInputs = inputSize() - numSelectLines;

    // If any select line is Unknown/Error, the output is indeterminate
    for (int i = 0; i < numSelectLines; i++) {
        const Status sel = simInputs().at(numDataInputs + i);
        if (sel == Status::Unknown || sel == Status::Error) {
            setOutputValue(sel);
            return;
        }
    }

    const int selectValue = decodeSelectValue(numDataInputs, numSelectLines);

    // With a non-power-of-two data width the select lines can encode values
    // that address no data input (e.g. 5 data inputs, 3 select lines, select
    // = 6). Routing is indeterminate — answer Unknown instead of reading a
    // select line back as data or indexing past the input vector.
    if (selectValue >= numDataInputs) {
        setOutputValue(Status::Unknown);
        return;
    }

    setOutputValue(simInputs().at(selectValue));
}

int Mux::calculateSelectLines(int totalInputs)
{
    int k = 1;
    while ((1 << k) < totalInputs - k) {
        k++;
    }
    return k;
}
