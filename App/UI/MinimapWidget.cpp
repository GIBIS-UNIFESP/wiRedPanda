// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/MinimapWidget.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPaintDevice>
#include <QPainter>
#include <QScrollBar>
#include <QTimer>

#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"

MinimapWidget::MinimapWidget(Scene *scene, GraphicsView *view, QWidget *parent)
    : QWidget(parent)
    , m_scene(scene)
    , m_view(view)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground, false);
    setFocusPolicy(Qt::NoFocus);
    // Default minimap size; can be tuned later or made a setting.
    setFixedSize(220, 160);
    setToolTip(tr("Mini-map: click or drag to navigate"));

    if (m_scene) {
        connect(m_scene, &Scene::circuitHasChanged, this, &MinimapWidget::invalidateCache);
    }

    if (m_view) {
        connect(m_view, &GraphicsView::zoomChanged, this, &MinimapWidget::updateOverlay);
        if (m_view->horizontalScrollBar())
            connect(m_view->horizontalScrollBar(), &QScrollBar::valueChanged, this, &MinimapWidget::updateOverlay);
        if (m_view->verticalScrollBar())
            connect(m_view->verticalScrollBar(), &QScrollBar::valueChanged, this, &MinimapWidget::updateOverlay);
        if (m_view->viewport())
            m_view->viewport()->installEventFilter(this);
    }

    m_throttle.setInterval(200);
    m_throttle.setSingleShot(true);
    connect(&m_throttle, &QTimer::timeout, this, &MinimapWidget::onThrottleTimeout);
}

QSize MinimapWidget::sizeHint() const
{
    return QSize(200, 150);
}

void MinimapWidget::paintEvent(QPaintEvent * /*event*/)
{
    if (!m_scene || !m_view) {
        return;
    }
    QPainter painter(this);
    painter.fillRect(rect(), palette().window().color());
    painter.setRenderHint(QPainter::Antialiasing, false);

    QRectF srcUsed;
    double scale = 1.0, dx = 0.0, dy = 0.0, usedW = 0.0, usedH = 0.0;
    if (!computeTransform(srcUsed, scale, dx, dy, usedW, usedH))
        return;

    const qreal dpr = this->devicePixelRatioF();
    const QSize pixSize = QSize(qRound(usedW * dpr), qRound(usedH * dpr));
    if (m_cacheDirty || m_cache.size() != pixSize) {
        m_cache = QPixmap(pixSize);
        m_cache.setDevicePixelRatio(dpr);
        m_cache.fill(palette().window().color());
        QPainter pixmapPainter(&m_cache);
        pixmapPainter.setRenderHint(QPainter::Antialiasing, false);
        m_scene->render(&pixmapPainter, QRectF(0, 0, usedW, usedH), srcUsed, Qt::KeepAspectRatio);
        m_cacheDirty = false;
    }

    // Draw the cached pixmap at the computed offset so the minimap content
    // aligns with the widget's mapping and avoids letterbox misalignment.
    painter.drawPixmap(QRectF(dx, dy, usedW, usedH), m_cache, QRectF(0, 0, usedW, usedH));

    // Draw viewport rectangle in minimap coordinates
    const QRectF visible = m_view->mapToScene(m_view->viewport()->rect()).boundingRect();

    QRectF vr(
        dx + (visible.left() - srcUsed.left()) * scale,
        dy + (visible.top() - srcUsed.top()) * scale,
        visible.width() * scale,
        visible.height() * scale);

    QPen pen(palette().highlight().color());
    pen.setWidthF(1.6);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(vr);
}

void MinimapWidget::mousePressEvent(QMouseEvent *event)
{
    if (!m_scene || !m_view) {
        return;
    }

    m_view->centerOn(widgetToScene(event->pos()));
    updateOverlay();
    m_dragging = true;
    event->accept();
}

void MinimapWidget::wheelEvent(QWheelEvent *event)
{
    // Swallow wheel events to prevent zooming the main view when the cursor
    // is over the minimap. Do not call base implementation.
    event->accept();
}

void MinimapWidget::invalidateCache()
{
    // Start or restart the throttle timer. The actual cache invalidation
    // and repaint will happen when the timer fires (max ~5 fps).
    m_throttle.start();
}

void MinimapWidget::onThrottleTimeout()
{
    m_cacheDirty = true;
    update();
}

void MinimapWidget::updateOverlay()
{
    update();
}

bool MinimapWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == (m_view ? m_view->viewport() : nullptr) && event->type() == QEvent::Resize) {
        updateOverlay();
        return QWidget::eventFilter(watched, event);
    }
    return QWidget::eventFilter(watched, event);
}

bool MinimapWidget::computeTransform(QRectF &srcOut, double &scaleOut, double &dxOut, double &dyOut, double &usedWOut, double &usedHOut) const
{
    if (!m_scene)
        return false;

    // Scene always contains a permanent (near-zero) rubber-band selection-rect item, so
    // itemsBoundingRect() never actually reports invalid/empty here in practice -- this
    // fallback and early return exist only as a defensive guard for a Scene that isn't
    // fully constructed the usual way.
    QRectF src = m_scene->itemsBoundingRect();
    if (!src.isValid() || src.isEmpty())
        src = m_scene->sceneRect();
    if (!src.isValid() || src.isEmpty())
        return false;

    const double w = static_cast<double>(width());
    const double h = static_cast<double>(height());
    const double sx = src.width() > 0 ? w / src.width() : 1.0;
    const double sy = src.height() > 0 ? h / src.height() : 1.0;
    const double scale = qMin(sx, sy);
    const double usedW = src.width() * scale;
    const double usedH = src.height() * scale;
    const double dx = (w - usedW) * 0.5;
    const double dy = (h - usedH) * 0.5;

    srcOut = src;
    scaleOut = scale;
    dxOut = dx;
    dyOut = dy;
    usedWOut = usedW;
    usedHOut = usedH;
    return true;
}

QPointF MinimapWidget::widgetToScene(const QPointF &p) const
{
    QRectF src;
    double scale = 1.0, dx = 0.0, dy = 0.0, usedW = 0.0, usedH = 0.0;
    if (!computeTransform(src, scale, dx, dy, usedW, usedH))
        return QPointF();

    double x = (p.x() - dx) / scale + src.left();
    double y = (p.y() - dy) / scale + src.top();

    // Clamp to source rect
    x = qBound(src.left(), x, src.right());
    y = qBound(src.top(), y, src.bottom());
    return QPointF(x, y);
}

void MinimapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging || !m_scene || !m_view) {
        QWidget::mouseMoveEvent(event);
        return;
    }

    m_view->centerOn(widgetToScene(event->pos()));
    updateOverlay();
    event->accept();
}

void MinimapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_dragging) {
        m_dragging = false;
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}
