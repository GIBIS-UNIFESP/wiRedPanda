// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Demux.h"

#include <cmath>

#include <QPainter>

#include "App/Element/ElementInfo.h"
#include "App/Element/LogicElements/LogicDemux.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Scene.h"

template<>
struct ElementInfo<Demux> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Demux,
        .group = ElementGroup::Mux,
        .minInputSize = 2,
        .maxInputSize = 4,
        .minOutputSize = 2,
        .maxOutputSize = 8,
        .canChangeSkin = true,
        .hasColors = false,
        .hasAudio = false,
        .hasAudioBox = false,
        .hasTrigger = false,
        .hasFrequency = false,
        .hasDelay = false,
        .hasLabel = false,
        .hasTruthTable = false,
        .rotatable = true,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Logic/demux.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Demux", "DEMULTIPLEXER");
        meta.translatedName = QT_TRANSLATE_NOOP("Demux", "Demux");
        meta.trContext = "Demux";
        meta.defaultSkins = QStringList({":/Components/Logic/demux.svg"});
        meta.logicCreator = [](GraphicElement *elm) { return std::make_shared<LogicDemux>(elm->outputSize()); };
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new Demux(); });
        return true;
    }();
};

Demux::Demux(QGraphicsItem *parent)
    : GraphicElement(ElementType::Demux, parent)
{
    Demux::updatePortsProperties();
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
    int numSelectLines = 1;
    while ((1 << numSelectLines) < size) {
        numSelectLines++;
    }
    int requiredInputs = 1 + numSelectLines;

    // Update output size FIRST to ensure outputSize() reflects new value
    // This prevents updatePortsProperties() from using stale output count
    GraphicElement::setOutputSize(size);

    // Then, update input size to match the select lines needed
    // This will trigger updatePortsProperties() with both input and output sizes correct
    GraphicElement::setInputSize(requiredInputs);

}

void Demux::updatePortsProperties()
{
    // Determine number of select lines based on number of outputs
    int numSelectLines = 1;
    while ((1 << numSelectLines) < outputSize()) {
        numSelectLines++;
    }

    const int step = Scene::gridSize / 2; // 8

    // Calculate element height to fit all output ports with padding
    int outputPortsSpan = (outputSize() - 1) * step * 2;
    int elementHeight = std::max(64, outputPortsSpan + step * 6);
    // Snap to multiple of 16
    elementHeight = ((elementHeight + 15) / 16) * 16;
    int centerY = elementHeight / 2;

    // Position data input on the left, centered
    inputPort(0)->setPos(16, centerY);
    inputPort(0)->setName("In");

    // Position select lines at the bottom, spaced by gridSize to avoid overlap
    int selectY = elementHeight - step;
    for (int i = 0; i < numSelectLines; ++i) {
        if ((1 + i) < inputSize()) {
            inputPort(1 + i)->setPos(32 + (i * step * 2), selectY);
            inputPort(1 + i)->setName(QString("S%1").arg(i));
        }
    }

    // Position output ports on the right, centered vertically
    int y = centerY - (outputSize() - 1) * step;
    for (int i = 0; i < outputSize(); ++i) {
        if (outputPort(i) != nullptr) {
            outputPort(i)->setPos(48, y);
            outputPort(i)->setName(QString("Out%1").arg(i));
        }
        y += step * 2;
    }

    generatePixmap();
}

void Demux::generatePixmap()
{
    const QRectF bounds = portsBoundingRect().united(QRectF(0, 0, 64, 64));
    const int height = static_cast<int>(bounds.height());

    QPixmap tempPixmap(64, height);
    tempPixmap.fill(Qt::transparent);

    QPainter painter(&tempPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

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

    m_pixmap = tempPixmap;
}

void Demux::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    if (isSelected()) {
        painter->save();
        painter->setBrush(m_selectionBrush);
        painter->setPen(QPen(m_selectionPen, 0.5, Qt::SolidLine));
        painter->drawRoundedRect(portsBoundingRect().united(QRectF(0, 0, 64, 64)), 5, 5);
        painter->restore();
    }

    painter->drawPixmap(boundingRect().topLeft(), pixmap());
}

