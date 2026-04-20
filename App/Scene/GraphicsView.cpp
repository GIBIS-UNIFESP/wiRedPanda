// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/GraphicsView.h"

#include "App/Core/SentryHelpers.h"

#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QScrollBar>

GraphicsView::GraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setAcceptDrops(true);
    // MouseTracking is needed so the scene receives mouseMoveEvents even when
    // no button is pressed (required for wire-routing cursor feedback)
    setMouseTracking(true);

    // AnchorUnderMouse keeps the point under the cursor fixed during zoom/resize,
    // giving the intuitive "zoom into where you're looking" behavior
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    // MinimalViewport mode repaints each dirty item's rect individually rather than
    // merging them into one large bounding rect, reducing pixel fill when changed
    // items are spread across the scene.
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);

    setCacheMode(QGraphicsView::CacheBackground);
}

bool GraphicsView::canZoomIn() const
{
    // Level 3 ≈ 1.25^3 ≈ 1.95× — beyond this elements become too large to be useful
    return m_zoomLevel < 3;
}

bool GraphicsView::canZoomOut() const
{
    // Level −9 ≈ 0.8^9 ≈ 0.13× — below this the grid dots merge and navigation is impractical
    return m_zoomLevel > -9;
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
    bool zoomed = false;

    // When zoom is redirected (e.g., when a scroll-sensitive widget has focus),
    // emit signals instead of zooming so the parent can decide what to do
    if (zoomDirection > 0 && canZoomIn()) {
        if (m_redirectZoom) {
            emit scaleIn();
        } else {
            zoomIn();
        }
        zoomed = true;
    } else if (zoomDirection < 0 && canZoomOut()) {
        if (m_redirectZoom) {
            emit scaleOut();
        } else {
            zoomOut();
        }
        zoomed = true;
    }

    // AnchorUnderMouse handles the scale transform, but scroll bars may shift after
    // the scale is applied; centerOn() corrects any residual drift so the scene point
    // that was under the cursor before the wheel event remains under it afterwards.
    if (zoomed) {
        centerOn(mapToScene(event->position().toPoint()));
    }

    event->accept();
}

void GraphicsView::setRedirectZoom(const bool value)
{
    m_redirectZoom = value;
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
    // 1.25 and 0.8 are reciprocals (1/1.25 == 0.8) so zoom-in then zoom-out
    // returns to exactly the original scale without floating-point drift
    scale(1.25, 1.25);
    ++m_zoomLevel;
    sentryBreadcrumb("ui", QStringLiteral("Zoom in: level %1").arg(m_zoomLevel));
    emit zoomChanged();
}

void GraphicsView::zoomOut()
{
    scale(0.8, 0.8);
    m_zoomLevel--;
    sentryBreadcrumb("ui", QStringLiteral("Zoom out: level %1").arg(m_zoomLevel));
    emit zoomChanged();
}

void GraphicsView::resetZoom()
{
    resetTransform();
    m_zoomLevel = 0;
    sentryBreadcrumb("ui", QStringLiteral("Zoom reset"));
    emit zoomChanged();
}

