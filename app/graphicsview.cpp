// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "graphicsview.h"

#include <QApplication>
#include <QDebug>
#include <QKeyEvent>
#include <QScrollBar>

GraphicsView::GraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setAcceptDrops(true);
    setMouseTracking(true);

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    setCacheMode(QGraphicsView::CacheBackground);
}

bool GraphicsView::canZoomIn() const
{
    return m_zoomLevel < 3;
}

bool GraphicsView::canZoomOut() const
{
    return m_zoomLevel > -9;
}

void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
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
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (event->pos().x() - m_panStartX));
        verticalScrollBar()->setValue(verticalScrollBar()->value() - (event->pos().y() - m_panStartY));
        m_panStartX = event->pos().x();
        m_panStartY = event->pos().y();
        event->accept();
        return;
    }

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

    if (zoomDirection > 0 && canZoomIn()) {
        if (m_redirectZoom) {
            emit scaleIn();
        } else {
            zoomIn();
        }
    } else if (zoomDirection < 0 && canZoomOut()) {
        if (m_redirectZoom) {
            emit scaleOut();
        } else {
            zoomOut();
        }
    }

    centerOn(QCursor::pos());

    event->accept();
}

void GraphicsView::setRedirectZoom(const bool value)
{
    m_redirectZoom = value;
}

void GraphicsView::setFastMode(const bool fastMode)
{
    setRenderHint(QPainter::Antialiasing, !fastMode);
    setRenderHint(QPainter::TextAntialiasing, !fastMode);
    setRenderHint(QPainter::SmoothPixmapTransform, !fastMode);
}

void GraphicsView::zoomIn()
{
    scale(1.25, 1.25);
    ++m_zoomLevel;
    emit zoomChanged();
}

void GraphicsView::zoomOut()
{
    scale(0.8, 0.8);
    m_zoomLevel--;
    emit zoomChanged();
}

void GraphicsView::resetZoom()
{
    resetTransform();
    m_zoomLevel = 0;
    emit zoomChanged();
}
