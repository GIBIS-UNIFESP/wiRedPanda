// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Demux.h"

#include <cmath>

#include <QPainter>

#include "App/Core/Common.h"
#include "App/Core/Constants.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/Wiring/Port.h"

template<>
struct ElementInfo<Demux> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Demux,
        .group = ElementGroup::Mux,
        .minInputSize = 2,
        .maxInputSize = 4,
        .minOutputSize = 2,
        .maxOutputSize = 8,
    };
    static_assert(validate(constraints));

    static ElementTypeDescriptor describe()
    {
        auto descriptor = descriptorFromConstraints(constraints);
        descriptor.pixmapPath = []{ return QStringLiteral(":/Components/Logic/demux.svg"); };
        descriptor.titleText = QT_TRANSLATE_NOOP("Demux", "DEMULTIPLEXER");
        descriptor.translatedName = QT_TRANSLATE_NOOP("Demux", "Demux");
        descriptor.trContext = "Demux";
        descriptor.defaultAppearances = QStringList({":/Components/Logic/demux.svg"});
        return descriptor;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementTypeDescriptor's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementTypeDescriptor::add(describe());
        ElementFactory::registerCreator(constraints.type, [] { return new Demux(); });
        return true;
    }();
};

Demux::Demux(QGraphicsItem *parent)
    : Element(ElementType::Demux, parent)
{
    updatePortsProperties();
}

void Demux::setInputSize(const int size)
{
    // Input size is derived from output size and cannot be set directly
    // This is a private override to prevent accidental calls
    Q_UNUSED(size)
    // Do nothing - this method should not be called
}

void Demux::setOutputSize(const int size)
{
    // Validate size is within bounds
    if ((size < minOutputSize()) || (size > maxOutputSize())) {
        return;  // Silently ignore invalid sizes, stays at current size
    }

    // Calculate required input ports: 1 data + log2(size) select lines
    const int numSelectLines = calculateSelectLines(size);
    int requiredInputs = 1 + numSelectLines;

    const ChangeBatch batch(*this, ElementChange::Ports);

    // Update output size FIRST to ensure outputSize() reflects new value
    // This prevents layoutPorts() from using stale output count
    Element::setOutputSize(size);

    // Then, update input size to match the select lines needed
    // This will trigger layoutPorts() with both input and output sizes correct
    Element::setInputSize(requiredInputs);
}

void Demux::layoutPorts()
{
    Element::layoutPorts();
    generatePixmap();
}

int Demux::bodyHeight() const
{
    const int step = Constants::gridSize / 2; // 8

    // Tall enough to fit all output ports with padding, snapped to a multiple of 16
    const int outputPortsSpan = (outputSize() - 1) * step * 2;
    const int elementHeight = (std::max)(64, outputPortsSpan + step * 6);
    return ((elementHeight + 15) / 16) * 16;
}

std::optional<Element::PortAttributes> Demux::inputAttributes(const int index) const
{
    const int step = Constants::gridSize / 2; // 8
    const int elementHeight = bodyHeight();

    // The data input on the left, centred
    if (index == 0) {
        return PortAttributes{.name = "In", .position = QPointF(16, elementHeight / 2)};
    }

    // Select lines at the bottom, spaced by gridSize to avoid overlap
    const int select = index - 1;
    if (select >= calculateSelectLines(outputSize())) {
        return std::nullopt;
    }
    return PortAttributes{.name = QString("S%1").arg(select), .position = QPointF(32 + (select * step * 2), elementHeight - step)};
}

std::optional<Element::PortAttributes> Demux::outputAttributes(const int index) const
{
    const int step = Constants::gridSize / 2; // 8

    // Outputs on the right, centred vertically
    const int y = (bodyHeight() / 2) - ((outputSize() - 1) * step) + (index * step * 2);
    return PortAttributes{.name = QString("Out%1").arg(index), .position = QPointF(48, y)};
}

void Demux::generatePixmap()
{
    // The body is drawn as vectors in drawBody()/paint(); m_pixmap is kept only so the base
    // pixmapCenter()/measureBoundingRect() have the right size (its image content is never displayed).
    const int height = static_cast<int>(renderBodyBounds().height());

    QPixmap sizingPixmap(64, height);
    sizingPixmap.fill(Qt::transparent);
    m_appearance.setRenderPixmap(sizingPixmap);
}

void Demux::drawBody(QPainter &painter) const
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    // boundingRect()'s top-left may be negative when ports extend past the 64×64 body; align the
    // local origin with it so the body lands exactly where the old rasterised pixmap was blitted.
    painter.translate(boundingRect().topLeft());

    const int height = pixmap().rect().height();

    // Trapezoid body: narrower on left (input side), wider on right (output side)
    const int bodyTop = 8;
    const int bodyBottom = height - 10;
    const int leftInset = 8; // fixed inset for the narrower left side

    // Outer dark shape
    painter.setBrush(QColor(38, 38, 38));
    painter.setPen(QPen(QColor(38, 38, 38), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    QPolygonF outer;
    outer << QPointF(22, bodyTop + leftInset)
          << QPointF(22, bodyBottom - leftInset)
          << QPointF(42, bodyBottom)
          << QPointF(42, bodyTop);
    painter.drawPolygon(outer);

    // Inner white fill
    painter.setBrush(QColor(252, 252, 252));
    painter.setPen(QPen(QColor(252, 252, 252), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    const int fillInset = 4;
    QPolygonF inner;
    inner << QPointF(22 + fillInset, bodyTop + leftInset + fillInset - 2)
          << QPointF(22 + fillInset, bodyBottom - leftInset - fillInset + 2)
          << QPointF(42 - fillInset, bodyBottom - fillInset)
          << QPointF(42 - fillInset, bodyTop + fillInset);
    painter.drawPolygon(inner);

    painter.restore();
}

void Demux::updateLogic()
{
    if (!simUpdateInputsAllowUnknown()) {
        return;
    }

    const Status data = simInputs().at(0);

    // Calculate select lines from current output count
    const int numSelectLines = calculateSelectLines(outputSize());

    // If any select line is Unknown/Error, all outputs are indeterminate
    for (int i = 0; i < numSelectLines; i++) {
        const Status sel = simInputs().at(1 + i);
        if (sel == Status::Unknown || sel == Status::Error) {
            for (int j = 0; j < outputSize(); j++) {
                setOutputValue(j, sel);
            }
            return;
        }
    }

    const int selectValue = decodeSelectValue(1, numSelectLines);

    // Mirror Mux: a select value that addresses no output (possible whenever
    // the output count is not a power of two) is indeterminate routing — all
    // outputs become Unknown instead of silently dropping the data.
    if (selectValue >= outputSize()) {
        for (int i = 0; i < outputSize(); i++) {
            setOutputValue(i, Status::Unknown);
        }
        return;
    }

    for (int i = 0; i < outputSize(); i++) {
        setOutputValue(i, (i == selectValue) ? data : Status::Inactive);
    }
}

void Demux::loadState(QDataStream &stream, SerializationContext &context)
{
    Element::loadState(stream, context);

    // The loader takes the port lists as the file gives them, and updateLogic() reads every
    // select line the outputs need; a save always writes exactly those inputs.
    const int expectedInputs = 1 + calculateSelectLines(outputSize());
    if (inputSize() != expectedInputs) {
        throw PANDACEPTION("%1 with %2 input ports for %3 outputs, where it has %4 — stream may be corrupt",
                           translatedName(),
                           QString::number(inputSize()),
                           QString::number(outputSize()),
                           QString::number(expectedInputs));
    }
}

int Demux::calculateSelectLines(int dataCount)
{
    int k = 1;
    while ((1 << k) < dataCount) {
        k++;
    }
    return k;
}
