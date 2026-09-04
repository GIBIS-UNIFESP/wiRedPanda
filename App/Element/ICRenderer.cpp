// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ICRenderer.h"

#include <cmath>

#include <QPainter>
#include <QSvgRenderer>

#include "App/Core/ItemWithId.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

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

void ICRenderer::generatePreviewPixmap(IC &ic, const QList<ItemWithId *> &items)
{
    // Split the freshly-deserialized items into elements and connections.  The
    // boundary Input/Output elements are still in their designed form here; the
    // substitution to proxy Nodes happens later in processLoadedItems().
    QVector<GraphicElement *> elements;
    QVector<Connection *> connections;
    elements.reserve(items.size());
    connections.reserve(items.size());
    for (auto *item : items) {
        if (auto *conn = dynamic_cast<Connection *>(item)) {
            connections.append(conn);
        } else if (auto *elm = dynamic_cast<GraphicElement *>(item)) {
            elements.append(elm);
        }
    }

    // Skip for empty or very large circuits.
    if (elements.isEmpty() || elements.size() > MaxElementCount) {
        ic.m_previewPixmap = QPixmap();
        return;
    }

    // Compute the bounding rect with some padding. No QGraphicsScene to ask for this any more
    // (GraphicElement/Port/Connection aren't QGraphicsItem) -- union each element's own
    // sceneBoundingRect() and each connection's boundingRect() directly.
    QRectF sourceRect;
    for (auto *elm : std::as_const(elements)) {
        sourceRect = sourceRect.united(elm->sceneBoundingRect());
    }
    for (auto *conn : std::as_const(connections)) {
        sourceRect = sourceRect.united(conn->boundingRect());
    }
    sourceRect = sourceRect.adjusted(-16, -16, 16, 16);

    // Defense-in-depth: a non-finite item geometry makes the bounding rect NaN,
    // and QSizeF::toSize() asserts (!qIsNaN) on a NaN dimension → process abort.
    // The load-side guards reject non-finite element position/rotation up front;
    // skip the (non-essential) preview here for any other geometry source rather
    // than crash.  Surfaced by libFuzzer (fuzz_ic_file).
    if (!std::isfinite(sourceRect.width()) || !std::isfinite(sourceRect.height())) {
        ic.m_previewPixmap = QPixmap();
        return;
    }

    // Scale to fit within max preview dimensions while preserving aspect ratio
    QSize targetSize = sourceRect.size().toSize();
    targetSize.scale(MaxWidth, MaxHeight, Qt::KeepAspectRatio);

    if (targetSize.isEmpty() || sourceRect.width() <= 0 || sourceRect.height() <= 0) {
        ic.m_previewPixmap = QPixmap();
        return;
    }

    QPixmap preview(targetSize);
    preview.fill(QColor(42, 42, 42));

    QPainter painter(&preview);
    painter.setRenderHint(QPainter::Antialiasing);

    // Maps sourceRect (scene coordinates) onto the full target pixmap, preserving aspect
    // ratio -- the same source→target affine QGraphicsScene::render(painter, {}, sourceRect)
    // used to build for us. Built the same translate/scale-chain way as
    // GraphicElement::rotateFlipTransform() (verified there): the LAST call applies FIRST to a
    // raw point, so a scene point is translated to sourceRect-relative first, then scaled.
    const qreal scale = std::min(targetSize.width() / sourceRect.width(),
                                  targetSize.height() / sourceRect.height());
    QTransform viewTransform;
    viewTransform.scale(scale, scale);
    viewTransform.translate(-sourceRect.left(), -sourceRect.top());

    for (auto *conn : std::as_const(connections)) {
        // Connection::paint() draws in its own world/canvas frame already (path() is stored
        // in scene coordinates, unlike GraphicElement::paint()/Port::paint()) -- just apply
        // the source→target view mapping.
        painter.setTransform(viewTransform);
        conn->paint(&painter);
    }

    for (auto *elm : std::as_const(elements)) {
        // local→scene, exactly matching GraphicElement::pointToScene(): rotate-then-flip about
        // pixmapCenter() (rotatable elements only) applied first, then translate by pos().
        // Built via operator* (verified: (A*B).map(p) == B.map(A.map(p)), left operand first).
        QTransform localToScene = QTransform::fromTranslate(elm->pos().x(), elm->pos().y());
        if (elm->rotatesGraphic()) {
            localToScene = elm->rotateFlipTransform() * localToScene;
        }

        painter.setTransform(localToScene * viewTransform);
        elm->paint(&painter);

        const auto paintPort = [&](Port *port) {
            QTransform portLocalToOwnerLocal = port->transform() * QTransform::fromTranslate(port->pos().x(), port->pos().y());
            painter.setTransform(portLocalToOwnerLocal * localToScene * viewTransform);
            port->paint(&painter);
        };
        for (auto *port : elm->inputs())  { paintPort(port); }
        for (auto *port : elm->outputs()) { paintPort(port); }

        auto *label = elm->labelItem();
        QTransform labelLocalToOwnerLocal = label->transform() * QTransform::fromTranslate(label->pos().x(), label->pos().y());
        if (!qFuzzyCompare(label->rotation(), 0.0)) {
            QTransform rotate;
            rotate.rotate(label->rotation());
            labelLocalToOwnerLocal = rotate * labelLocalToOwnerLocal;
        }
        painter.setTransform(labelLocalToOwnerLocal * localToScene * viewTransform);
        label->paint(&painter);
    }

    painter.end();

    ic.m_previewPixmap = preview;
}
