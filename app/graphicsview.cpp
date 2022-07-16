// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
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
}

bool GraphicsView::canZoomIn() const
{
    return m_zoomLevel < 3;
}

bool GraphicsView::canZoomOut() const
{
    return m_zoomLevel > -3;
}

void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        m_pan = true;
        m_panStartX = event->pos().x();
        m_panStartY = event->pos().y();
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
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
        QApplication::restoreOverrideCursor();
        event->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void GraphicsView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) {
        m_space = true;
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        event->accept();
    }

    QGraphicsView::keyPressEvent(event);
}

void GraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) {
        m_space = false;
        QApplication::restoreOverrideCursor();
        event->accept();
    }

    QGraphicsView::keyReleaseEvent(event);
}

void GraphicsView::wheelEvent(QWheelEvent *event)
{
    if (QApplication::keyboardModifiers() == Qt::ControlModifier) {
        double deltaY = event->angleDelta().y();
        (deltaY > 0) ? m_zoomLevel++ : m_zoomLevel--;
        double factor = (deltaY > 0) ? 1.25 : 0.8;
        scale(factor, factor);
        return;
    }

    QGraphicsView::wheelEvent(event);
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
    m_zoomLevel++;
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
