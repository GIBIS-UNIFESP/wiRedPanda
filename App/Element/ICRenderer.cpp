// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/ICRenderer.h"

#include <cmath>

#include <QGraphicsScene>
#include <QPainter>
#include <QScopeGuard>
#include <QSize>
#include <QSizeF>

#include "App/Element/IC.h"
#include "App/Element/ICPreviewPopup.h"
#include "App/Wiring/Connection.h"

void ICRenderer::generatePreviewPixmap(IC &ic, const QList<QGraphicsItem *> &items)
{
    // Split the freshly-deserialized items into elements and connections.  The
    // boundary Input/Output elements are still in their designed form here; the
    // substitution to proxy Nodes happens later in processLoadedItems().
    QVector<Element *> elements;
    QVector<Connection *> connections;
    elements.reserve(items.size());
    connections.reserve(items.size());
    for (auto *item : items) {
        if (auto *conn = qgraphicsitem_cast<Connection *>(item)) {
            connections.append(conn);
        } else if (auto *elm = qgraphicsitem_cast<Element *>(item)) {
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

    // Defense-in-depth: a non-finite item geometry makes the bounding rect NaN,
    // and QSizeF::toSize() asserts (!qIsNaN) on a NaN dimension → process abort.
    // The load-side guards reject non-finite element position/rotation up front;
    // skip the (non-essential) preview here for any other geometry source rather
    // than crash.  Surfaced by libFuzzer (fuzz_ic_file).
    if (!std::isfinite(sourceRect.width()) || !std::isfinite(sourceRect.height())) {
        ic.m_previewPixmap = QPixmap();
        return;
    }

    // Scale to fit within max preview dimensions while preserving aspect ratio, in floating point: a finite extent
    // past int's range is undefined to round to whole pixels before it is scaled down
    QSizeF scaledSize = sourceRect.size();
    scaledSize.scale(ICPreviewPopup::MaxWidth, ICPreviewPopup::MaxHeight, Qt::KeepAspectRatio);
    const QSize targetSize = scaledSize.toSize();

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
