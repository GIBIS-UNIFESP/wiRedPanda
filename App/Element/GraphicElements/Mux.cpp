// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Mux.h"

#include <cmath>

#include <QPainter>

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
        .canChangeAppearance = true,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Logic/mux.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Mux", "MULTIPLEXER");
        meta.translatedName = QT_TRANSLATE_NOOP("Mux", "Mux");
        meta.trContext = "Mux";
        meta.defaultAppearances = QStringList({":/Components/Logic/mux.svg"});
        return meta;
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementMetadata's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new Mux(); });
        return true;
    }();
};

Mux::Mux(QGraphicsItem *parent)
    : GraphicElement(ElementType::Mux, parent)
{
    Mux::updatePortsProperties();
}

void Mux::updatePortsProperties()
{
    // Determine number of select lines based on total input size
    const int numSelectLines = calculateSelectLines(inputSize());
    int numDataInputs = inputSize() - numSelectLines;

    const int step = Constants::gridSize / 2; // 8

    // Calculate element height to fit all data ports with padding
    int dataPortsSpan = (numDataInputs - 1) * step * 2; // spacing between first and last port
    int elementHeight = (std::max)(64, dataPortsSpan + step * 6);
    // Snap to multiple of 16
    elementHeight = ((elementHeight + 15) / 16) * 16;
    int centerY = elementHeight / 2;

    // Position data input ports on the left, centered vertically
    int y = centerY - (numDataInputs - 1) * step;
    for (int i = 0; i < numDataInputs; ++i) {
        inputPort(i)->setPos(16, y);
        inputPort(i)->setName(QString("In%1").arg(i));
        y += step * 2;
    }

    // Position select lines at the bottom, spaced by gridSize to avoid overlap
    int selectY = elementHeight - step;
    for (int i = 0; i < numSelectLines; ++i) {
        inputPort(numDataInputs + i)->setPos(32 + (i * step * 2), selectY);
        inputPort(numDataInputs + i)->setName(QString("S%1").arg(i));
    }

    // Position output port on the right, centered
    outputPort(0)->setPos(48, centerY);
    outputPort(0)->setName("Out");

    generatePixmap();
}

QRectF Mux::boundingRect() const
{
    return renderBodyBounds();
}

void Mux::generatePixmap()
{
    // The body is drawn as vectors in drawBody()/paint(); m_pixmap is kept only so the base
    // pixmapCenter()/boundingRect() have the right size (its image content is never displayed).
    const int height = static_cast<int>(renderBodyBounds().height());

    QPixmap sizingPixmap(64, height);
    sizingPixmap.fill(Qt::transparent);
    m_appearance.setRenderPixmap(sizingPixmap);
    update();
}

void Mux::drawBody(QPainter *painter)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    // boundingRect()'s top-left may be negative when ports extend past the 64×64 body; align the
    // local origin with it so the body lands exactly where the old rasterised pixmap was blitted.
    painter->translate(boundingRect().topLeft());

    const int height = pixmap().rect().height();

    // Trapezoid body: wider on left (input side), narrower on right (output side)
    const int bodyTop = 8;
    const int bodyBottom = height - 10;
    const int rightInset = 8; // fixed inset for the narrower right side

    // Outer dark shape
    painter->setBrush(QColor(38, 38, 38));
    painter->setPen(QPen(QColor(38, 38, 38), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    QPolygonF outer;
    outer << QPointF(22, bodyTop)
          << QPointF(22, bodyBottom)
          << QPointF(42, bodyBottom - rightInset)
          << QPointF(42, bodyTop + rightInset);
    painter->drawPolygon(outer);

    // Inner white fill
    painter->setBrush(QColor(252, 252, 252));
    painter->setPen(QPen(QColor(252, 252, 252), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    const int fillInset = 4;
    QPolygonF inner;
    inner << QPointF(22 + fillInset, bodyTop + fillInset)
          << QPointF(22 + fillInset, bodyBottom - fillInset)
          << QPointF(42 - fillInset, bodyBottom - rightInset - fillInset + 2)
          << QPointF(42 - fillInset, bodyTop + rightInset + fillInset - 2);
    painter->drawPolygon(inner);

    painter->restore();
}

void Mux::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    if (isSelected()) {
        painter->save();
        painter->setBrush(m_appearance.selectionBrush());
        painter->setPen(QPen(m_appearance.selectionPen(), 0.5, Qt::SolidLine));
        painter->drawRoundedRect(boundingRect(), 5, 5);
        painter->restore();
    }

    // Draw the body as vectors (crisp at any zoom) rather than blitting a fixed-resolution pixmap.
    drawBody(painter);
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
