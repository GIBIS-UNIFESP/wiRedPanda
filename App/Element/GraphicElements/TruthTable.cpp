// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/TruthTable.h"

#include <QPainter>

#include "App/Core/Common.h"
#include "App/Element/ElementInfo.h"
#include "App/Element/LogicElements/LogicTruthTable.h"
#include "App/GlobalProperties.h"
#include "App/Nodes/QNEPort.h"
#include "App/Versions.h"

template<>
struct ElementInfo<TruthTable> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::TruthTable,
        .group = ElementGroup::IC,
        .minInputSize = 2,
        .maxInputSize = 8,
        .minOutputSize = 1,
        .maxOutputSize = 8,
        .canChangeSkin = true,
        .hasColors = false,
        .hasAudio = false,
        .hasAudioBox = false,
        .hasTrigger = false,
        .hasFrequency = false,
        .hasDelay = false,
        .hasLabel = true,
        .hasTruthTable = true,
        .rotatable = true,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Logic/truthtable-rotated.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("TruthTable", "TRUTH TABLE");
        meta.translatedName = QT_TRANSLATE_NOOP("TruthTable", "Truth Table");
        meta.trContext = "TruthTable";
        meta.defaultSkins = QStringList({":/Components/Logic/truthtable-rotated.svg"});
        meta.logicCreator = [](GraphicElement *elm) {
            auto *truthTable = qobject_cast<TruthTable *>(elm);
            if (!truthTable) {
                throw PANDACEPTION_WITH_CONTEXT("TruthTable", "Failed to cast element to TruthTable");
            }
            return std::make_shared<LogicTruthTable>(elm->inputSize(), elm->outputSize(), truthTable->key());
        };
        return meta;
    }

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new TruthTable(); });
        return true;
    }();
};

TruthTable::TruthTable(QGraphicsItem *parent)
    : GraphicElement(ElementType::TruthTable, parent)
{
    // 2048 bits = 256 rows × 8 output columns (max: 8 inputs × 8 outputs).
    // Laid out as [row0_out0, row0_out1, ..., row0_out7, row1_out0, ...].
    // All outputs default to 0 (all-false table).
    m_key.resize(2048);
    m_key.fill(0);
    TruthTable::updatePortsProperties();
}

void TruthTable::updatePortsProperties()
{
    int index = 0;
    // step = 8px (half the 16px grid), giving ports a 16px pitch (every other grid line)
    const int step = GlobalProperties::gridSize / 2;

    if (!m_inputPorts.isEmpty()) {
        // Center the input port column vertically within the 64px-minimum body height.
        // Formula: start at body centre (32) minus half the total column height, then
        // add one step back so the first port lands at the right offset.
        // Total height of n ports at 2*step pitch = n * 2*step; half = n*step.
        // e.g. 4 inputs → y_start = 32 - 4*8 + 8 = 8; ports at y=8,24,40,56
        int y = 32 - (m_inputPorts.size() * step) + step;

        for (auto *port : std::as_const(m_inputPorts)) {

            if (!isRotatable()) {
                port->setRotation(0);
            }

            port->setPos(0, y);

            y += step * 2;

            // Inputs are labeled A, B, C, ... (alphabetically, matching truth table convention)
            port->setName(QChar::fromLatin1(static_cast<char>('A' + index)));
            ++index;
        }
    }

    index = 0;

    if (!m_outputPorts.isEmpty()) {
        // Same centering formula as inputs; output column is on the right edge (x=64)
        int y = 32 - (m_outputPorts.size() * step) + step;

        for (auto *port : std::as_const(m_outputPorts)) {

            if (!isRotatable()) {
                port->setRotation(0);
            }

            port->setPos(64, y);

            y += step * 2;

            // Outputs are labeled S0, S1, ... to distinguish them from input names
            port->setName("S" + QString::number(index));
            ++index;
        }
    }

    const qreal bottom = portsBoundingRect().united(QRectF(0, 0, 64, 64)).bottom();
    m_label->setPos(30, bottom + 5);

    generatePixmap();
}

void TruthTable::generatePixmap()
{
    // The TruthTable renders a custom IC-style body rather than using a fixed SVG.
    // The pixmap is regenerated whenever port count changes because the body height
    // must grow to accommodate the port layout.
    const QSize size = portsBoundingRect().united(QRectF(0, 0, 64, 64)).size().toSize();

    QPixmap tempPixmap(size);

    tempPixmap.fill(Qt::transparent);

    QPainter tmpPainter(&tempPixmap);

    // Main body: mid-gray rounded rectangle inset 7px on each side to leave room
    // for port connectors on the left and right edges
    tmpPainter.setBrush(QColor(126, 126, 126));
    tmpPainter.setPen(QPen(QBrush(QColor(78, 78, 78)), 0.5, Qt::SolidLine));

    // 7px inset on each side (14px total width reduction) so port connector dots sit
    // on top of the body border rather than floating in empty space
    QPoint topLeft = tempPixmap.rect().topLeft();
    topLeft.setX(topLeft.x() + 7);
    QSize finalSize = tempPixmap.rect().size();
    finalSize.setWidth(finalSize.width() - 14);
    QRectF finalRect = QRectF(topLeft, finalSize);
    tmpPainter.drawRoundedRect(finalRect, 3, 3);

    // Center the truth-table icon inside the body
    QPixmap panda(":/Components/Logic/truthtable-rotated.svg");
    QPointF pandaOrigin = finalRect.center();
    pandaOrigin.setX(pandaOrigin.x() - panda.width() / 2);
    pandaOrigin.setY(pandaOrigin.y() - panda.height() / 2);
    tmpPainter.drawPixmap(pandaOrigin, panda);

    // Shadow strip at the bottom of the body to give a subtle 3-D depth effect
    tmpPainter.setBrush(QColor(78, 78, 78));
    tmpPainter.setPen(QPen(QBrush(QColor(78, 78, 78)), 0.5, Qt::SolidLine));

    // Collapse the two-point rect to a 3px-tall strip at the bottom edge for the shadow
    QRectF shadowRect(finalRect.bottomLeft(), finalRect.bottomRight());
    shadowRect.adjust(0, -3, 0, 0);
    tmpPainter.drawRoundedRect(shadowRect, 3, 3);

    m_pixmap = tempPixmap;

    GraphicElement::update();
}

void TruthTable::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    if (isSelected()) {
        painter->save();
        painter->setBrush(m_selectionBrush);
        painter->setPen(QPen(m_selectionPen, 0.5, Qt::SolidLine));
        // Expand the highlight rect to cover any ports that extend outside the 64x64 body
        painter->drawRoundedRect(portsBoundingRect().united(QRectF(0, 0, 64, 64)), 5, 5);
        painter->restore();
    }

    painter->drawPixmap(boundingRect().topLeft(), pixmap());
}

QBitArray &TruthTable::key()
{
    return m_key;
}

void TruthTable::setkey(const QBitArray &key)
{
    m_key = key;
}

void TruthTable::save(QDataStream &stream) const
{
    GraphicElement::save(stream);
    QMap<QString, QVariant> map;
    map.insert("key", m_key);
    stream << map;
}

void TruthTable::load(QDataStream &stream, QMap<quint64, QNEPort *> &portMap, const QVersionNumber version)
{
    GraphicElement::load(stream, portMap, version);

    if (version >= Versions::V_4_2) {
        // Truth-table key (the output bit-array) was first serialized in v4.2
        QMap<QString, QVariant> map; stream >> map;

        if (map.contains("key")) {
            setkey(map.value("key").toBitArray());
        }
    }
}
