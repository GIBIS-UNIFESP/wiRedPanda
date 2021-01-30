#include "graphicsviewzoom.h"

#include <QApplication>
#include <QDebug>
#include <QMouseEvent>
#include <QScrollBar>
#include <QtMath>

const double GraphicsViewZoom::maxZoom = 1.5;
const double GraphicsViewZoom::minZoom = 0.20;

GraphicsViewZoom::GraphicsViewZoom(QGraphicsView *view)
    : QObject(view)
    , _view(view)
{
    _view->viewport()->installEventFilter(this);
    _view->setMouseTracking(true);
    _modifiers = Qt::ControlModifier;
    _zoom_factor_base = 1.0015;
}

void GraphicsViewZoom::gentle_zoom(double factor)
{
    QTransform tr = _view->transform().scale(factor, factor);
    double scfac = tr.m11();
    if ((scfac >= minZoom) && (scfac <= maxZoom)) {
        _view->setTransform(tr);
        _view->centerOn(target_scene_pos);
        QPointF delta_viewport_pos = target_viewport_pos - QPointF(_view->viewport()->width() / 2.0, _view->viewport()->height() / 2.0);
        QPointF viewport_center = _view->mapFromScene(target_scene_pos) - delta_viewport_pos;
        _view->centerOn(_view->mapToScene(viewport_center.toPoint()));
        emit zoomed();
    }
}

void GraphicsViewZoom::set_modifiers(Qt::KeyboardModifiers modifiers)
{
    _modifiers = modifiers;
}

void GraphicsViewZoom::set_zoom_factor_base(double value)
{
    _zoom_factor_base = value;
}

void GraphicsViewZoom::zoomIn()
{
    _view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
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
    return (_view->transform().m11());
}

void GraphicsViewZoom::setScaleFactor(double factor)
{
    QTransform tr = _view->transform();
    tr *= QTransform::fromScale(1.0 / tr.m11(), 1.0 / tr.m11());
    tr.scale(factor, factor);
    _view->setTransform(tr);
}

bool GraphicsViewZoom::canZoomIn()
{
    return ((scaleFactor() + ZOOMFAC * 2) <= GraphicsViewZoom::maxZoom);
}

bool GraphicsViewZoom::canZoomOut()
{
    return ((scaleFactor() - ZOOMFAC * 2) >= GraphicsViewZoom::minZoom);
}

bool GraphicsViewZoom::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseMove) {
        auto *mouse_event = static_cast<QMouseEvent *>(event);
        QPointF delta = target_viewport_pos - mouse_event->pos();
        if ((qAbs(delta.x()) > 5) || (qAbs(delta.y()) > 5)) {
            target_viewport_pos = mouse_event->pos();
            target_scene_pos = _view->mapToScene(mouse_event->pos());
        }
    } else if (event->type() == QEvent::Wheel) {
        auto *wheel_event = static_cast<QWheelEvent *>(event);
        if (QApplication::keyboardModifiers() == _modifiers) {
            // if( wheel_event->orientation( ) == Qt::Vertical ) {
            double angle = wheel_event->angleDelta().y();
            double factor = qPow(_zoom_factor_base, angle);
            gentle_zoom(factor);
            return (true);
            //}
        }
    }
    Q_UNUSED(object)
    return (false);
}
