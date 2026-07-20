// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ICRenderer.h"

#include <cmath>

#include <QPainter>
#include <QSvgRenderer>

#include "App/Element/IC.h"

/// Shared, lazily-constructed vector renderer for the IC mascot logo — one per process, drawn
/// directly in drawBody() so the logo stays crisp at any zoom. GUI-thread only, like pixmapCache().
static QSvgRenderer &icLogoRenderer()
{
    static QSvgRenderer renderer(QStringLiteral(":/Components/Logic/ic-panda2.svg"));
    return renderer;
}

void ICRenderer::generatePixmap(IC &ic)
{
    // The body is now drawn as vectors in drawBody()/paint(); m_pixmap is kept only so that the
    // base pixmapCenter()/boundingRect() have the right size (its image content is never displayed).
    // It must encompass both the 64×64 body and any ports that extend beyond it.
    const QSizeF boundsSize = ic.renderBodyBounds().size();

    // Defense-in-depth: a non-finite boundary port position makes this size NaN, and
    // QSizeF::toSize() asserts (!qIsNaN) on a NaN dimension → process abort.  The load-side
    // guards reject non-finite element position/rotation up front; skip regenerating the
    // sizing pixmap here for any other geometry source rather than crash.
    if (!std::isfinite(boundsSize.width()) || !std::isfinite(boundsSize.height())) {
        return;
    }

    QPixmap sizingPixmap(boundsSize.toSize());
    sizingPixmap.fill(Qt::transparent);
    ic.m_appearance.setRenderPixmap(sizingPixmap);
    ic.update();
}

void ICRenderer::drawBody(IC &ic, QPainter *painter)
{
    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing, true);
    // boundingRect()'s top-left may be negative when ports extend past the 64×64 body; align the
    // local origin with it so the body lands exactly where the old rasterised pixmap was blitted.
    painter->translate(ic.boundingRect().topLeft());
    // The body footprint is the (correctly-sized) m_pixmap rect — exactly the area the old raster
    // occupied — so the geometry is reproduced 1:1 at any zoom.
    const QRectF bounds(ic.pixmap().rect());

    const QColor bodyColor = ic.isEmbedded() ? QColor(90, 126, 160) : QColor(126, 126, 126);
    const QColor outlineColor = ic.isEmbedded() ? QColor(58, 82, 110) : QColor(78, 78, 78);

    // IC body: styled like a physical DIP package. 7px inset on each side (14px total) so the port
    // connector dots visually overlap the border, matching the TruthTable and physical DIP look.
    painter->setBrush(bodyColor);
    painter->setPen(QPen(QBrush(outlineColor), 0.5, Qt::SolidLine));
    const QRectF finalRect(QPointF(7, 0), QSizeF(bounds.width() - 14, bounds.height()));
    painter->drawRoundedRect(finalRect, 3, 3);

    // Centre the wiRedPanda mascot logo on the body, rendered as vectors at its native size.
    // The mascot is decoration, like the baked-in SVG pin text: counter-orient it about its own
    // centre (rotate outer, flip inner — the inverse of the item's Flip∘Rotate) so it reads
    // upright at any element orientation.
    QSvgRenderer &logo = icLogoRenderer();
    const QSizeF logoSize = logo.defaultSize();
    const QRectF logoRect(finalRect.center() - QPointF(logoSize.width() / 2, logoSize.height() / 2), logoSize);
    painter->save();
    painter->translate(logoRect.center());
    painter->rotate(-ic.rotation());
    painter->scale(ic.isFlippedX() ? -1 : 1, ic.isFlippedY() ? -1 : 1);
    painter->translate(-logoRect.center());
    logo.render(painter, logoRect);
    painter->restore();

    // Thin dark strip at the bottom edge to simulate the package shadow/bevel.
    painter->setBrush(outlineColor);
    painter->setPen(QPen(QBrush(outlineColor), 0.5, Qt::SolidLine));
    QRectF shadowRect(finalRect.bottomLeft(), finalRect.bottomRight());
    shadowRect.adjust(0, -3, 0, 0);
    painter->drawRoundedRect(shadowRect, 3, 3);

    // Orientation notch (semicircle) at the top centre, matching the physical DIP pin-1 convention.
    // drawChord angle parameters are in 1/16th-degree units; -180*16 = lower half-circle.
    const QRectF topCenter(finalRect.topLeft() + QPointF(18, -12), QSizeF(24, 24));
    painter->drawChord(topCenter, 0, -180 * 16);

    painter->restore();
}
