// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/GraphicsView.h"

#include <cmath>

#include <QApplication>
#include <QCursor>
#include <QDebug>
#include <QElapsedTimer>
#include <QGraphicsItem>
#include <QKeyEvent>
#include <QScrollBar>

#include "App/Core/SentryHelpers.h"
#include "App/Scene/Scene.h"

namespace {
// Zoom step ladder (factor 1.25 per step). The range is deliberately asymmetric: elements
// stay useful when tiny, so we allow far more zoom-out than in.
constexpr int kMaxViewZoomLevel = 7;   ///< 1.25^7 ≈ 4.8× — close enough to wire small ports/pins.
constexpr int kMinViewZoomLevel = -9;  ///< 0.8^9 ≈ 0.13× — below this the grid dots merge.
} // namespace

GraphicsView::GraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setAccessibleName(tr("Circuit canvas"));
    setWhatsThis(tr("The circuit canvas. Drag elements here from the palette to build a "
                     "circuit, drag between ports to wire them together, and drag a "
                     "selection to move it."));

    setAcceptDrops(true);
    // MouseTracking is needed so the scene receives mouseMoveEvents even when
    // no button is pressed (required for wire-routing cursor feedback)
    setMouseTracking(true);

    // A resize keeps the point under the cursor fixed; a zoom keeps its own pivot (zoomToLevel())
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    // MinimalViewport mode repaints each dirty item's rect individually rather than
    // merging them into one large bounding rect, reducing pixel fill when changed
    // items are spread across the scene.
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);

    setCacheMode(QGraphicsView::CacheBackground);
}

void GraphicsView::paintEvent(QPaintEvent *event)
{
    // Time the pass so Scene can adapt wire antialiasing to the measured paint workload
    // (see Scene's wire-antialiasing accessors).
    QElapsedTimer timer;
    timer.start();

    QGraphicsView::paintEvent(event);

    if (auto *scene_ = qobject_cast<Scene *>(scene())) {
        scene_->recordWirePaintPass(timer.nsecsElapsed());
    }
}

bool GraphicsView::canZoomIn() const
{
    return m_zoomLevel < kMaxViewZoomLevel;
}

bool GraphicsView::canZoomOut() const
{
    return m_zoomLevel > kMinViewZoomLevel;
}

void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        // Middle-button drag pans the canvas; capture the start position so
        // mouseMoveEvent can compute the delta to scroll by
        m_pan = true;
        m_panStartX = event->pos().x();
        m_panStartY = event->pos().y();
        viewport()->setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pan || m_space) {
        // Subtract delta from scroll values: moving right decreases scrollbar value
        // because scene coordinates grow leftward as the view scrolls right
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (event->pos().x() - m_panStartX));
        verticalScrollBar()->setValue(verticalScrollBar()->value() - (event->pos().y() - m_panStartY));
        // Update the anchor each frame so the next delta is relative to this position
        m_panStartX = event->pos().x();
        m_panStartY = event->pos().y();
        event->accept();
        return;
    }

    // Keep panStart updated even when not panning so that panning can begin
    // correctly mid-drag without a position jump
    m_panStartX = event->pos().x();
    m_panStartY = event->pos().y();

    QGraphicsView::mouseMoveEvent(event);
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        m_pan = false;
        viewport()->unsetCursor();
        event->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void GraphicsView::keyPressEvent(QKeyEvent *event)
{
    // Ignore auto-repeat so a held key counts as one logical press (matches Scene's trigger
    // handling and avoids spacebar-pan churn on X11, where auto-repeat arrives as release/press
    // pairs).
    if (event->isAutoRepeat()) {
        QGraphicsView::keyPressEvent(event);
        return;
    }

    if (event->key() == Qt::Key_Space) {
        // Spacebar held = pan mode (same as middle-mouse drag), a common convention
        // in design tools; the flag is checked in mouseMoveEvent
        m_space = true;
        viewport()->setCursor(Qt::ClosedHandCursor);
        event->accept();
    }

    QGraphicsView::keyPressEvent(event);
}

void GraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    // Ignore auto-repeat releases (X11 emits them mid-hold) so pan mode isn't dropped while the
    // spacebar is still physically held.
    if (event->isAutoRepeat()) {
        QGraphicsView::keyReleaseEvent(event);
        return;
    }

    if (event->key() == Qt::Key_Space) {
        m_space = false;
        viewport()->unsetCursor();
        event->accept();
    }

    QGraphicsView::keyReleaseEvent(event);
}

void GraphicsView::wheelEvent(QWheelEvent *event)
{
    const int zoomDirection = event->angleDelta().y();
    const QPoint cursor = event->position().toPoint();

    if (zoomDirection > 0 && canZoomIn()) {
        zoomToLevel(m_zoomLevel + 1, cursor);
        sentryBreadcrumb("ui", QStringLiteral("Zoom in: level %1").arg(m_zoomLevel));
    } else if (zoomDirection < 0 && canZoomOut()) {
        zoomToLevel(m_zoomLevel - 1, cursor);
        sentryBreadcrumb("ui", QStringLiteral("Zoom out: level %1").arg(m_zoomLevel));
    }

    event->accept();
}

void GraphicsView::setFastMode(const bool fastMode)
{
    // Fast mode disables expensive rendering hints to maintain frame rate when
    // the circuit is large or the machine is slow; used during active simulation
    setRenderHint(QPainter::Antialiasing, !fastMode);
    setRenderHint(QPainter::TextAntialiasing, !fastMode);
    setRenderHint(QPainter::SmoothPixmapTransform, !fastMode);
}

void GraphicsView::zoomIn()
{
    zoomToLevel(m_zoomLevel + 1, zoomPivot());
    sentryBreadcrumb("ui", QStringLiteral("Zoom in: level %1").arg(m_zoomLevel));
}

void GraphicsView::zoomOut()
{
    zoomToLevel(m_zoomLevel - 1, zoomPivot());
    sentryBreadcrumb("ui", QStringLiteral("Zoom out: level %1").arg(m_zoomLevel));
}

void GraphicsView::resetZoom()
{
    zoomToLevel(0, zoomPivot());
    sentryBreadcrumb("ui", QStringLiteral("Zoom reset"));
}

QPoint GraphicsView::zoomPivot() const
{
    // The cursor while it is over the view, as AnchorUnderMouse reads it, else the viewport's centre
    if (underMouse()) {
        return viewport()->mapFromGlobal(QCursor::pos());
    }
    return viewport()->rect().center();
}

void GraphicsView::zoomToLevel(const int level, const QPoint pivot)
{
    // The scene point under the pivot stays under it. The scale is the level's power of 1.25, so a level is always the
    // same scale however it was reached; the scene first grows over what the view will show, since Qt clamps the
    // scroll bars to the scene rect, and setTransform() pivots elsewhere (AnchorUnderMouse: the last mouse move's point,
    // or the last centre), so the view scrolls by however far the pinned point drifted from the pivot
    const QPointF pinned = mapToScene(pivot);
    const qreal scaleFactor = std::pow(1.25, level);
    if (auto *scene_ = qobject_cast<Scene *>(scene())) {
        scene_->growSceneOver(QRectF(pinned - (QPointF(pivot) / scaleFactor), QSizeF(viewport()->size()) / scaleFactor));
    }

    setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
    m_zoomLevel = level;

    const QPointF drift = viewportTransform().map(pinned) - QPointF(pivot);
    const int direction = isRightToLeft() ? -1 : 1;
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + (direction * qRound(drift.x())));
    verticalScrollBar()->setValue(verticalScrollBar()->value() + qRound(drift.y()));
    emit zoomChanged();
}

void GraphicsView::zoomToFit()
{
    if (!scene()) {
        return;
    }

    // Fit the current selection if there is one (zoom-to-selection); otherwise the whole circuit.
    QRectF target;
    const auto selected = scene()->selectedItems();
    if (!selected.isEmpty()) {
        for (auto *item : selected) {
            target |= item->sceneBoundingRect();
        }
    } else if (auto *scene_ = qobject_cast<Scene *>(scene())) {
        target = scene_->cachedCircuitBoundingRect();
    } else {
        target = scene()->itemsBoundingRect();
    }

    if (!target.isValid() || target.isEmpty()) {
        return;
    }

    // A little breathing room around the content.
    target.adjust(-16.0, -16.0, 16.0, 16.0);

    // fitInView gives the ideal continuous scale; snap it DOWN to the nearest discrete zoom step
    // so the whole target still fits (floor never overshoots the viewport) and the level ladder
    // stays consistent with zoomIn()/zoomOut()/canZoom*(). The step is clamped to the same range
    // those methods enforce.
    fitInView(target, Qt::KeepAspectRatio);
    const double fitScale = transform().m11();
    const int level = qBound(kMinViewZoomLevel, static_cast<int>(std::floor(std::log(fitScale) / std::log(1.25))), kMaxViewZoomLevel);

    resetTransform();
    const double scaleFactor = std::pow(1.25, level);
    scale(scaleFactor, scaleFactor);
    m_zoomLevel = level;
    centerOn(target.center());

    sentryBreadcrumb("ui", QStringLiteral("Zoom to fit: level %1").arg(m_zoomLevel));
    emit zoomChanged();
}
