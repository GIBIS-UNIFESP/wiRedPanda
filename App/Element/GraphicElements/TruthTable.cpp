// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElements/TruthTable.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QSvgRenderer>

#include "App/Core/Common.h"
#include "App/Core/Constants.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/ElementInfo.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/IO/VersionInfo.h"
#include "App/Nodes/QNEPort.h"

template<>
struct ElementInfo<TruthTable> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::TruthTable,
        .group = ElementGroup::IC,
        .minInputSize = 2,
        .maxInputSize = 8,
        .minOutputSize = 1,
        .maxOutputSize = 8,
        .canChangeAppearance = true,
        .hasLabel = true,
        .hasTruthTable = true,
    };
    static_assert(validate(constraints));

    static ElementMetadata metadata()
    {
        auto meta = metadataFromConstraints(constraints);
        meta.pixmapPath = []{ return QStringLiteral(":/Components/Logic/truthtable-rotated.svg"); };
        meta.titleText = QT_TRANSLATE_NOOP("TruthTable", "TRUTH TABLE");
        meta.translatedName = QT_TRANSLATE_NOOP("TruthTable", "Truth Table");
        meta.trContext = "TruthTable";
        meta.defaultAppearances = QStringList({":/Components/Logic/truthtable-rotated.svg"});
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
    const int step = Constants::gridSize / 2;

    if (!inputs().isEmpty()) {
        // Center the input port column vertically within the 64px-minimum body height.
        // Formula: start at body centre (32) minus half the total column height, then
        // add one step back so the first port lands at the right offset.
        // Total height of n ports at 2*step pitch = n * 2*step; half = n*step.
        // e.g. 4 inputs → y_start = 32 - 4*8 + 8 = 8; ports at y=8,24,40,56
        int y = 32 - (static_cast<int>(inputs().size()) * step) + step;

        for (auto *port : inputs()) {

            if (!rotatesGraphic()) {
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

    if (!outputs().isEmpty()) {
        // Same centering formula as inputs; output column is on the right edge (x=64)
        int y = 32 - (static_cast<int>(outputs().size()) * step) + step;

        for (auto *port : outputs()) {

            if (!rotatesGraphic()) {
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

/// Shared, lazily-constructed vector renderer for the truth-table icon — one per process, drawn
/// directly in drawBody() so it stays crisp at any zoom. GUI-thread only, like pixmapCache().
static QSvgRenderer &truthTableLogoRenderer()
{
    static QSvgRenderer renderer(QStringLiteral(":/Components/Logic/truthtable-rotated.svg"));
    return renderer;
}

void TruthTable::generatePixmap()
{
    // The TruthTable renders a custom IC-style body, now drawn as vectors in drawBody()/paint().
    // m_pixmap is kept only so the base pixmapCenter()/boundingRect() have the right size (its image
    // content is never displayed); a transparent pixmap of the body footprint is enough. It is
    // regenerated whenever the port count changes because the body height grows with the port layout.
    const QSize size = portsBoundingRect().united(QRectF(0, 0, 64, 64)).size().toSize();
    QPixmap sizingPixmap(size);
    sizingPixmap.fill(Qt::transparent);
    m_appearance.setRenderPixmap(sizingPixmap);
    GraphicElement::update();
}

void TruthTable::drawBody(QPainter *painter)
{
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing, true);
    // boundingRect()'s top-left may be negative when ports extend past the 64×64 body; align the
    // local origin with it so the body lands exactly where the old rasterised pixmap was blitted.
    painter->translate(boundingRect().topLeft());
    // The body footprint is the (correctly-sized) m_pixmap rect — exactly the area the old raster
    // occupied — so the geometry is reproduced 1:1 at any zoom.
    const QRectF bounds(pixmap().rect());

    // Main body: mid-gray rounded rectangle inset 7px on each side (14px total) so port connector
    // dots sit on top of the body border rather than floating in empty space.
    painter->setBrush(QColor(126, 126, 126));
    painter->setPen(QPen(QBrush(QColor(78, 78, 78)), 0.5, Qt::SolidLine));
    const QRectF finalRect(QPointF(7, 0), QSizeF(bounds.width() - 14, bounds.height()));
    painter->drawRoundedRect(finalRect, 3, 3);

    // Centre the truth-table icon inside the body, rendered as vectors at its native size.
    QSvgRenderer &logo = truthTableLogoRenderer();
    const QSizeF logoSize = logo.defaultSize();
    const QRectF logoRect(finalRect.center() - QPointF(logoSize.width() / 2, logoSize.height() / 2), logoSize);
    logo.render(painter, logoRect);

    // Shadow strip at the bottom of the body for a subtle 3-D depth effect.
    painter->setBrush(QColor(78, 78, 78));
    painter->setPen(QPen(QBrush(QColor(78, 78, 78)), 0.5, Qt::SolidLine));
    QRectF shadowRect(finalRect.bottomLeft(), finalRect.bottomRight());
    shadowRect.adjust(0, -3, 0, 0);
    painter->drawRoundedRect(shadowRect, 3, 3);

    painter->restore();
}

void TruthTable::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    if (isSelected()) {
        painter->save();
        painter->setBrush(m_appearance.selectionBrush());
        painter->setPen(QPen(m_appearance.selectionPen(), 0.5, Qt::SolidLine));
        // Expand the highlight rect to cover any ports that extend outside the 64x64 body
        painter->drawRoundedRect(portsBoundingRect().united(QRectF(0, 0, 64, 64)), 5, 5);
        painter->restore();
    }

    // Draw the body as vectors (crisp at any zoom) rather than blitting a fixed-resolution pixmap.
    drawBody(painter);
}

void TruthTable::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    emit requestOpenTruthTableEditor();
}

QBitArray &TruthTable::key()
{
    return m_key;
}

void TruthTable::setkey(const QBitArray &key)
{
    m_key = key;
    // updateLogic() indexes the key at 256*output + row (up to bit 2047) and
    // ToggleTruthTableOutputCommand toggles bits in place, so the key must
    // hold exactly 2048 bits regardless of what a (possibly corrupt or
    // crafted) .panda file supplied — resize pads with zeros or truncates.
    m_key.resize(2048);
}

void TruthTable::save(QDataStream &stream) const
{
    GraphicElement::save(stream);
    QMap<QString, QVariant> map;
    map.insert("key", m_key);
    stream << map;
}

void TruthTable::load(QDataStream &stream, SerializationContext &context)
{
    GraphicElement::load(stream, context);

    if (VersionInfo::hasTruthTableData(context.version)) {
        // Truth-table key (the output bit-array) was first serialized in v4.2
        QMap<QString, QVariant> map = Serialization::readBoundedMetadata(stream);

        if (map.contains("key")) {
            setkey(map.value("key").toBitArray());
        }
    }
}

void TruthTable::updateLogic()
{
    if (!simUpdateInputsAllowUnknown()) {
        return;
    }

    // If any input is Unknown/Error, the row cannot be determined
    for (const auto s : simInputs()) {
        if (s == Status::Unknown || s == Status::Error) {
            for (int i = 0; i < outputSize(); ++i) {
                setOutputValue(i, s);
            }
            return;
        }
    }

    // The row index is the inputs read MSB-first (input 0 is the most
    // significant bit) — computed once with integer ops instead of building
    // and re-parsing a binary QString per output per tick.
    quint32 pos = 0;
    for (const auto s : simInputs()) {
        pos = (pos << 1) | ((s == Status::Active) ? 1U : 0U);
    }

    for (int i = 0; i < outputSize(); ++i) {
        const bool result = m_key.at(static_cast<qsizetype>(256 * i) + static_cast<qsizetype>(pos));
        setOutputValue(i, result);
    }
}
