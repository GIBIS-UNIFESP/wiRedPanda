// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ICRenderer.h"

#include <QGraphicsScene>
#include <QPainter>
#include <QScopeGuard>
#include <QStyleOptionGraphicsItem>
#include <QSvgRenderer>

#include "App/Element/IC.h"
#include "App/Element/ICPreviewPopup.h"
#include "App/Wiring/Connection.h"

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
    const QSize size = ic.portsBoundingRect().united(QRectF(0, 0, 64, 64)).size().toSize();
    QPixmap sizingPixmap(size);
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
    QSvgRenderer &logo = icLogoRenderer();
    const QSizeF logoSize = logo.defaultSize();
    const QRectF logoRect(finalRect.center() - QPointF(logoSize.width() / 2, logoSize.height() / 2), logoSize);
    logo.render(painter, logoRect);

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

void ICRenderer::generatePreviewPixmap(IC &ic, const QList<QGraphicsItem *> &items)
{
    // Split the freshly-deserialized items into elements and connections.  The
    // boundary Input/Output elements are still in their designed form here; the
    // substitution to proxy Nodes happens later in processLoadedItems().
    QVector<GraphicElement *> elements;
    QVector<Connection *> connections;
    elements.reserve(items.size());
    connections.reserve(items.size());
    for (auto *item : items) {
        if (auto *conn = qgraphicsitem_cast<Connection *>(item)) {
            connections.append(conn);
        } else if (auto *elm = qgraphicsitem_cast<GraphicElement *>(item)) {
            elements.append(elm);
        }
    }

    // Skip for empty or very large circuits.
    if (elements.isEmpty() || elements.size() > ICPreviewPopup::MaxElementCount) {
        ic.m_previewPixmap = QPixmap();
        return;
    }

    // Temporarily borrow the items into a throwaway scene so QGraphicsScene::render()
    // can be used without disturbing the real scene.  The scope guard guarantees
    // cleanup even if render() throws.
    QGraphicsScene tempScene;
    tempScene.setBackgroundBrush(QColor(42, 42, 42));

    auto cleanup = qScopeGuard([&] {
        for (auto *elm  : std::as_const(elements))    { tempScene.removeItem(elm);  }
        for (auto *conn : std::as_const(connections)) { tempScene.removeItem(conn); }
    });

    for (auto *elm : std::as_const(elements)) {
        tempScene.addItem(elm);
    }
    for (auto *conn : std::as_const(connections)) {
        tempScene.addItem(conn);
    }

    // Compute the bounding rect with some padding
    const QRectF sourceRect = tempScene.itemsBoundingRect().adjusted(-16, -16, 16, 16);

    // Scale to fit within max preview dimensions while preserving aspect ratio
    QSize targetSize = sourceRect.size().toSize();
    targetSize.scale(ICPreviewPopup::MaxWidth, ICPreviewPopup::MaxHeight, Qt::KeepAspectRatio);

    if (targetSize.isEmpty()) {
        ic.m_previewPixmap = QPixmap();
        return;
    }

    // QPixmap(QSize) is uninitialised; tempScene.render() paints the background
    // brush over the source→target affine, but subpixel rounding can leave a
    // 1-pixel sliver unpainted at the right/bottom edge, exposing whatever was
    // in memory (commonly white on Windows).  Fill explicitly to avoid that.
    QPixmap preview(targetSize);
    preview.fill(QColor(42, 42, 42));

    QPainter painter(&preview);
    painter.setRenderHint(QPainter::Antialiasing);
    tempScene.render(&painter, QRectF(), sourceRect);
    painter.end();

    ic.m_previewPixmap = preview;
}
