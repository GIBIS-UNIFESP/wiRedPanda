// Copyright 2015 - 2022, GIBIS-Unifesp and the WiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "graphicsviewzoom.h"

#include <QApplication>
#include <QDebug>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QScrollBar>
#include <QtMath>

#define ZOOMFAC 0.1

GraphicsViewZoom::GraphicsViewZoom(QGraphicsView *view)
    : QObject(view)
    , m_view(view)
{
    m_view->viewport()->installEventFilter(this);
    m_view->setMouseTracking(true);
    m_modifiers = Qt::ControlModifier;
    m_zoomFactorBase = 1.0015;
}

void GraphicsViewZoom::gentleZoom(double factor)
{
    QTransform tr = m_view->transform().scale(factor, factor);
    double scfac = tr.m11();
    if ((scfac >= minZoom) && (scfac <= maxZoom)) {
        m_view->setTransform(tr);
        m_view->centerOn(m_targetScenePos);
        QPointF delta_viewport_pos = m_targetViewportPos - QPointF(m_view->viewport()->width() / 2.0, m_view->viewport()->height() / 2.0);
        QPointF viewport_center = m_view->mapFromScene(m_targetScenePos) - delta_viewport_pos;
        m_view->centerOn(m_view->mapToScene(viewport_center.toPoint()));
        emit zoomed();
    }
}

void GraphicsViewZoom::setModifiers(Qt::KeyboardModifiers modifiers)
{
    m_modifiers = modifiers;
}

void GraphicsViewZoom::setZoomFactorBase(double value)
{
    m_zoomFactorBase = value;
}

void GraphicsViewZoom::zoomIn()
{
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    double newScale = std::round(scaleFactor() * 10) / 10.0 + ZOOMFAC;
    if (newScale <= GraphicsViewZoom::maxZoom) {
        setScaleFactor(newScale);
    }
    emit zoomed();
}

void GraphicsViewZoom::zoomOut()
{
    double newScale = std::round(scaleFactor() * 10) / 10.0 - ZOOMFAC;
    if (newScale >= GraphicsViewZoom::minZoom) {
        setScaleFactor(newScale);
    }
    emit zoomed();
}

void GraphicsViewZoom::resetZoom()
{
    setScaleFactor(1.0);
    emit zoomed();
}

double GraphicsViewZoom::scaleFactor()
{
    return m_view->transform().m11();
}

void GraphicsViewZoom::setScaleFactor(double factor)
{
    QTransform tr = m_view->transform();
    tr *= QTransform::fromScale(1.0 / tr.m11(), 1.0 / tr.m11());
    tr.scale(factor, factor);
    m_view->setTransform(tr);
}

bool GraphicsViewZoom::canZoomIn()
{
    return (scaleFactor() + ZOOMFAC * 2) <= GraphicsViewZoom::maxZoom;
}

bool GraphicsViewZoom::canZoomOut()
{
    return (scaleFactor() - ZOOMFAC * 2) >= GraphicsViewZoom::minZoom;
}

bool GraphicsViewZoom::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseMove) {
        auto *mouse_event = static_cast<QMouseEvent *>(event);
        QPointF delta = m_targetViewportPos - mouse_event->pos();
        if ((qAbs(delta.x()) > 5) || (qAbs(delta.y()) > 5)) {
            m_targetViewportPos = mouse_event->pos();
            m_targetScenePos = m_view->mapToScene(mouse_event->pos());
        }
    } else if (event->type() == QEvent::Wheel) {
        auto *wheel_event = static_cast<QWheelEvent *>(event);
        if (QApplication::keyboardModifiers() == m_modifiers) {
            // if( wheel_event->orientation( ) == Qt::Vertical ) {
            double angle = wheel_event->angleDelta().y();
            double factor = qPow(m_zoomFactorBase, angle);
            gentleZoom(factor);
            return true;
            //}
        }
    }
    Q_UNUSED(object)
    return false;
}
