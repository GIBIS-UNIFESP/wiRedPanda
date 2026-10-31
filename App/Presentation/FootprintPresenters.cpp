// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Presentation/FootprintPresenters.h"

#include <QPainter>
#include <QPen>
#include <QPolygonF>
#include <QSvgRenderer>

#include "App/Element/Element.h"

namespace FootprintPresentersDetail {

/// Returns the vector renderer of the truth table's icon, built the first time, so it stays crisp at any zoom.
/// GUI-thread only.
QSvgRenderer &truthTableLogoRenderer()
{
    static QSvgRenderer renderer(QStringLiteral(":/Components/Logic/truthtable-rotated.svg"));
    return renderer;
}

/// Returns the vector renderer of the IC's mascot, built the first time, so it stays crisp at any zoom. GUI-thread
/// only.
QSvgRenderer &icLogoRenderer()
{
    static QSvgRenderer renderer(QStringLiteral(":/Components/Logic/ic-panda2.svg"));
    return renderer;
}

/// Draws a multiplexer's trapezoid between x = 22 and x = 42, from \a top to \a bottom: its narrow side, the right one
/// when \a narrowOnRight, is 8 px shorter at each end than the other. The outline is dark, the inside white.
void drawTrapezoid(QPainter &painter, const int top, const int bottom, const bool narrowOnRight)
{
    const int inset = 8;
    const int narrowLeft = narrowOnRight ? 0 : inset;
    const int narrowRight = narrowOnRight ? inset : 0;

    painter.setBrush(QColor(38, 38, 38));
    painter.setPen(QPen(QColor(38, 38, 38), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    QPolygonF outer;
    outer << QPointF(22, top + narrowLeft)
          << QPointF(22, bottom - narrowLeft)
          << QPointF(42, bottom - narrowRight)
          << QPointF(42, top + narrowRight);
    painter.drawPolygon(outer);

    // The white inside: inset from the outline, its slanted side's ends 2 px closer to the outline's
    painter.setBrush(QColor(252, 252, 252));
    painter.setPen(QPen(QColor(252, 252, 252), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    const int fill = 4;
    const int slantLeft = narrowOnRight ? 0 : 2;
    const int slantRight = narrowOnRight ? 2 : 0;
    QPolygonF inner;
    inner << QPointF(22 + fill, top + narrowLeft + fill - slantLeft)
          << QPointF(22 + fill, bottom - narrowLeft - fill + slantLeft)
          << QPointF(42 - fill, bottom - narrowRight - fill + slantRight)
          << QPointF(42 - fill, top + narrowRight + fill - slantRight);
    painter.drawPolygon(inner);
}

} // namespace FootprintPresentersDetail

BodyShape FootprintPresenter::measure(const Element &element) const
{
    const QRectF footprint = element.renderBodyBounds();
    return {footprint, footprint.center()};
}

void FootprintPresenter::paintBody(QPainter &painter, const Element &element, const BodyContext &context) const
{
    Q_UNUSED(context)
    const QRectF footprint = element.body().body;
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    // The footprint's top-left is negative where ports reach past the 64x64 square
    painter.translate(footprint.topLeft());
    paintFootprint(painter, element, QRectF(QPointF(), footprint.size()));
    painter.restore();
}

void MuxPresenter::paintFootprint(QPainter &painter, const Element &element, const QRectF &footprint) const
{
    Q_UNUSED(element)
    const int height = static_cast<int>(footprint.height());
    FootprintPresentersDetail::drawTrapezoid(painter, 8, height - 10, true);
}

void DemuxPresenter::paintFootprint(QPainter &painter, const Element &element, const QRectF &footprint) const
{
    Q_UNUSED(element)
    const int height = static_cast<int>(footprint.height());
    FootprintPresentersDetail::drawTrapezoid(painter, 8, height - 10, false);
}

void TruthTablePresenter::paintFootprint(QPainter &painter, const Element &element, const QRectF &footprint) const
{
    painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing, true);
    const QRectF bounds(QPointF(), footprint.size().toSize());

    // Main body: mid-gray rounded rectangle inset 7px on each side (14px total) so port connector
    // dots sit on top of the body border rather than floating in empty space.
    painter.setBrush(QColor(126, 126, 126));
    painter.setPen(QPen(QBrush(QColor(78, 78, 78)), 0.5, Qt::SolidLine));
    const QRectF finalRect(QPointF(7, 0), QSizeF(bounds.width() - 14, bounds.height()));
    painter.drawRoundedRect(finalRect, 3, 3);

    // Centre the truth-table icon inside the body, rendered as vectors at its native size.
    // The icon is decoration, like the baked-in SVG pin text: counter-orient it about its own
    // centre (rotate outer, flip inner — the inverse of the item's Flip∘Rotate) so it reads
    // upright at any element orientation.
    QSvgRenderer &logo = FootprintPresentersDetail::truthTableLogoRenderer();
    const QSizeF logoSize = logo.defaultSize();
    const QRectF logoRect(finalRect.center() - QPointF(logoSize.width() / 2, logoSize.height() / 2), logoSize);
    painter.save();
    painter.translate(logoRect.center());
    painter.rotate(-element.rotation());
    painter.scale(element.isFlippedX() ? -1 : 1, element.isFlippedY() ? -1 : 1);
    painter.translate(-logoRect.center());
    logo.render(&painter, logoRect);
    painter.restore();

    // Shadow strip at the bottom of the body for a subtle 3-D depth effect.
    painter.setBrush(QColor(78, 78, 78));
    painter.setPen(QPen(QBrush(QColor(78, 78, 78)), 0.5, Qt::SolidLine));
    QRectF shadowRect(finalRect.bottomLeft(), finalRect.bottomRight());
    shadowRect.adjust(0, -3, 0, 0);
    painter.drawRoundedRect(shadowRect, 3, 3);
}

void ICPresenter::paintFootprint(QPainter &painter, const Element &element, const QRectF &footprint) const
{
    painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing, true);

    const QColor bodyColor = element.isEmbedded() ? QColor(90, 126, 160) : QColor(126, 126, 126);
    const QColor outlineColor = element.isEmbedded() ? QColor(58, 82, 110) : QColor(78, 78, 78);

    // IC body: styled like a physical DIP package. 7px inset on each side (14px total) so the port
    // connector dots visually overlap the border, matching the TruthTable and physical DIP look.
    painter.setBrush(bodyColor);
    painter.setPen(QPen(QBrush(outlineColor), 0.5, Qt::SolidLine));
    const QRectF finalRect(QPointF(7, 0), QSizeF(footprint.width() - 14, footprint.height()));
    painter.drawRoundedRect(finalRect, 3, 3);

    // Centre the wiRedPanda mascot logo on the body, rendered as vectors at its native size.
    // The mascot is decoration, like the baked-in SVG pin text: counter-orient it about its own
    // centre (rotate outer, flip inner — the inverse of the item's Flip∘Rotate) so it reads
    // upright at any element orientation.
    QSvgRenderer &logo = FootprintPresentersDetail::icLogoRenderer();
    const QSizeF logoSize = logo.defaultSize();
    const QRectF logoRect(finalRect.center() - QPointF(logoSize.width() / 2, logoSize.height() / 2), logoSize);
    painter.save();
    painter.translate(logoRect.center());
    painter.rotate(-element.rotation());
    painter.scale(element.isFlippedX() ? -1 : 1, element.isFlippedY() ? -1 : 1);
    painter.translate(-logoRect.center());
    logo.render(&painter, logoRect);
    painter.restore();

    // Thin dark strip at the bottom edge to simulate the package shadow/bevel.
    painter.setBrush(outlineColor);
    painter.setPen(QPen(QBrush(outlineColor), 0.5, Qt::SolidLine));
    QRectF shadowRect(finalRect.bottomLeft(), finalRect.bottomRight());
    shadowRect.adjust(0, -3, 0, 0);
    painter.drawRoundedRect(shadowRect, 3, 3);

    // Orientation notch (semicircle) at the top centre, matching the physical DIP pin-1 convention.
    // drawChord angle parameters are in 1/16th-degree units; -180*16 = lower half-circle.
    const QRectF topCenter(finalRect.topLeft() + QPointF(18, -12), QSizeF(24, 24));
    painter.drawChord(topCenter, 0, -180 * 16);
}
