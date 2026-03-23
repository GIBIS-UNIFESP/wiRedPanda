// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/Mux.h"

#include <cmath>

#include <QPainter>

#include "App/Element/ElementInfo.h"
#include "App/Element/LogicElements/LogicMux.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Scene.h"

template<>
struct ElementInfo<Mux> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::Mux,
        .group = ElementGroup::Mux,
        .minInputSize = 3,
        .maxInputSize = 11,
        .minOutputSize = 1,
        .maxOutputSize = 1,
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
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Logic/mux.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("Mux", "MULTIPLEXER");
        meta.translatedName = QT_TRANSLATE_NOOP("Mux", "Mux");
        meta.trContext = "Mux";
        meta.defaultSkins = QStringList({":/Components/Logic/mux.svg"});
        meta.logicCreator = [](GraphicElement *elm) { return std::make_shared<LogicMux>(elm->inputSize()); };
        return meta;
    }

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
    int numSelectLines = 1;
    while (true) {
        int numDataInputs = inputSize() - numSelectLines;
        if ((1 << numSelectLines) >= numDataInputs) {
            break;
        }
        numSelectLines++;
    }
    int numDataInputs = inputSize() - numSelectLines;

    const int step = Scene::gridSize / 2; // 8

    // Calculate element height to fit all data ports with padding
    int dataPortsSpan = (numDataInputs - 1) * step * 2; // spacing between first and last port
    int elementHeight = std::max(64, dataPortsSpan + step * 6);
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

void Mux::generatePixmap()
{
    const QRectF bounds = portsBoundingRect().united(QRectF(0, 0, 64, 64));
    const int height = static_cast<int>(bounds.height());

    QPixmap tempPixmap(64, height);
    tempPixmap.fill(Qt::transparent);

    QPainter painter(&tempPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Trapezoid body: wider on left (input side), narrower on right (output side)
    const int bodyTop = 8;
    const int bodyBottom = height - 10;
    const int rightInset = 8; // fixed inset for the narrower right side

    // Outer dark shape
    painter.setBrush(QColor(38, 38, 38));
    painter.setPen(QPen(QColor(38, 38, 38), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    QPolygonF outer;
    outer << QPointF(22, bodyTop)
          << QPointF(22, bodyBottom)
          << QPointF(42, bodyBottom - rightInset)
          << QPointF(42, bodyTop + rightInset);
    painter.drawPolygon(outer);

    // Inner white fill
    painter.setBrush(QColor(252, 252, 252));
    painter.setPen(QPen(QColor(252, 252, 252), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    const int fillInset = 4;
    QPolygonF inner;
    inner << QPointF(22 + fillInset, bodyTop + fillInset)
          << QPointF(22 + fillInset, bodyBottom - fillInset)
          << QPointF(42 - fillInset, bodyBottom - rightInset - fillInset + 2)
          << QPointF(42 - fillInset, bodyTop + rightInset + fillInset - 2);
    painter.drawPolygon(inner);

    m_pixmap = tempPixmap;
}

void Mux::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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

