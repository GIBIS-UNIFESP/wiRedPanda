// Copyright 2015 - 2021, GIBIS-Unifesp and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "graphicsview.h"

#include <QApplication>
#include <QKeyEvent>
#include <QScrollBar>

#include "graphicsviewzoom.h"

GraphicsView::GraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    m_pan = false;
    m_space = false;
    m_panStartX = 0;
    m_panStartY = 0;
    setAcceptDrops(true);
    m_gvzoom = new GraphicsViewZoom(this);
}

void GraphicsView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::MiddleButton) {
        m_pan = true;
        m_panStartX = e->pos().x();
        m_panStartY = e->pos().y();
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        e->accept();
        return;
    }
    QGraphicsView::mousePressEvent(e);
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::MiddleButton) {
        m_pan = false;
        QApplication::restoreOverrideCursor();
        e->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(e);
}

void GraphicsView::mouseMoveEvent(QMouseEvent *e)
{
    if (m_pan || m_space) {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (e->pos().x() - m_panStartX));
        verticalScrollBar()->setValue(verticalScrollBar()->value() - (e->pos().y() - m_panStartY));
        m_panStartX = e->pos().x();
        m_panStartY = e->pos().y();
        e->accept();
        return;
    }
    m_panStartX = e->pos().x();
    m_panStartY = e->pos().y();
    QGraphicsView::mouseMoveEvent(e);
}

void GraphicsView::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Space) {
        m_space = true;
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        e->accept();
    }
    QGraphicsView::keyPressEvent(e);
}

void GraphicsView::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Space) {
        m_space = false;
        QApplication::restoreOverrideCursor();
        e->accept();
    }
    QGraphicsView::keyReleaseEvent(e);
}

GraphicsViewZoom *GraphicsView::gvzoom() const
{
    return m_gvzoom;
}