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
#include "App/Wiring/Port.h"

template<>
struct ElementInfo<TruthTable> {
    static constexpr ElementConstraints constraints{
        .type = ElementType::TruthTable,
        .group = ElementGroup::IC,
        .minInputSize = 2,
        .maxInputSize = 8,
        .minOutputSize = 1,
        .maxOutputSize = 8,
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
    } // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned ElementMetadata's QString/QStringList members, never reached after the return above.

    static inline const bool registered = []() {
        ElementMetadataRegistry::registerMetadata(metadata());
        ElementFactory::registerCreator(constraints.type, [] { return new TruthTable(); });
        return true;
    }();
};

namespace {

/// Rows in a full table: the row index is the inputs read MSB-first, so an element wired to the
/// maximum number of inputs addresses 2^maxInputSize of them.
constexpr qsizetype kTruthTableRows = qsizetype{1} << ElementInfo<TruthTable>::constraints.maxInputSize;

/// Bits in m_key: one per (output column, row). updateLogic() indexes kTruthTableRows * output
/// + row, so the largest index it can form is exactly this minus one -- the array has ZERO
/// slack, and derived constants are what keep that true if the port limits ever move.
constexpr qsizetype kTruthTableKeyBits = kTruthTableRows * ElementInfo<TruthTable>::constraints.maxOutputSize;

static_assert(kTruthTableKeyBits == 2048,
              "The .panda format stores this QBitArray verbatim and setkey() resizes every loaded "
              "key to it, so the size is part of the file format. Raising maxInputSize or "
              "maxOutputSize silently changes what is written and what older files decode to -- "
              "handle it as a format change, not a constant edit.");

} // namespace

TruthTable::TruthTable(QGraphicsItem *parent)
    : Element(ElementType::TruthTable, parent)
{
    // Laid out as [out0_row0..out0_rowN, out1_row0..out1_rowN, ...].
    // All outputs default to 0 (all-false table).
    m_key.resize(kTruthTableKeyBits);
    m_key.fill(0);
    TruthTable::updatePortsProperties();
}

void TruthTable::updatePortsProperties()
{
    // The ports take the base layout: columns centred on the 64 px body, at a 16 px pitch (every other grid line)
    Element::updatePortsProperties();

    setLabelAnchor(QPointF(30, renderBodyBounds().bottom() + 5));

    generatePixmap();
}

std::optional<Element::PortAttributes> TruthTable::inputAttributes(const int index) const
{
    // Inputs are labeled A, B, C, ... (alphabetically, matching truth table convention)
    return PortAttributes{.name = QString(QChar::fromLatin1(static_cast<char>('A' + index)))};
}

std::optional<Element::PortAttributes> TruthTable::outputAttributes(const int index) const
{
    // Outputs are labeled S0, S1, ... to distinguish them from input names
    return PortAttributes{.name = "S" + QString::number(index)};
}

/// Shared, lazily-constructed vector renderer for the truth-table icon — one per process, drawn
/// directly in drawBody() so it stays crisp at any zoom. GUI-thread only, like pixmapCache().
static QSvgRenderer &truthTableLogoRenderer()
{
    static QSvgRenderer renderer(QStringLiteral(":/Components/Logic/truthtable-rotated.svg"));
    return renderer;
}

QRectF TruthTable::boundingRect() const
{
    return renderBodyBounds();
}

void TruthTable::generatePixmap()
{
    // The TruthTable renders a custom IC-style body, now drawn as vectors in drawBody()/paint().
    // m_pixmap is kept only so the base pixmapCenter()/boundingRect() have the right size (its image
    // content is never displayed); a transparent pixmap of the body footprint is enough. It is
    // regenerated whenever the port count changes because the body height grows with the port layout.
    const QSize size = renderBodyBounds().size().toSize();
    QPixmap sizingPixmap(size);
    sizingPixmap.fill(Qt::transparent);
    m_appearance.setRenderPixmap(sizingPixmap);
    Element::update();
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
    // The icon is decoration, like the baked-in SVG pin text: counter-orient it about its own
    // centre (rotate outer, flip inner — the inverse of the item's Flip∘Rotate) so it reads
    // upright at any element orientation.
    QSvgRenderer &logo = truthTableLogoRenderer();
    const QSizeF logoSize = logo.defaultSize();
    const QRectF logoRect(finalRect.center() - QPointF(logoSize.width() / 2, logoSize.height() / 2), logoSize);
    painter->save();
    painter->translate(logoRect.center());
    painter->rotate(-rotation());
    painter->scale(isFlippedX() ? -1 : 1, isFlippedY() ? -1 : 1);
    painter->translate(-logoRect.center());
    logo.render(painter, logoRect);
    painter->restore();

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
        painter->drawRoundedRect(boundingRect(), 5, 5);
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
    // updateLogic() indexes the key at kTruthTableRows*output + row (up to the last bit) and
    // ToggleTruthTableOutputCommand toggles bits in place, so the key must hold exactly
    // kTruthTableKeyBits bits regardless of what a (possibly corrupt or crafted) .panda file
    // supplied — resize pads with zeros or truncates.
    m_key.resize(kTruthTableKeyBits);
}

void TruthTable::save(QDataStream &stream, SerializationOptions options) const
{
    Element::save(stream, options);
    QMap<QString, QVariant> map;
    map.insert("key", m_key);
    stream << map;
}

void TruthTable::load(QDataStream &stream, SerializationContext &context)
{
    Element::load(stream, context);

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
        const bool result = m_key.at(kTruthTableRows * i + static_cast<qsizetype>(pos));
        setOutputValue(i, result);
    }
}
