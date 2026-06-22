// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/MinimapWidget.h"

#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <QScrollBar>
#include <QEvent>

#include "App/Scene/Scene.h"
#include "App/Scene/GraphicsView.h"

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
    setToolTip(tr("Mini-map: click to navigate. Wheel ignored."));

    if (m_scene) {
        connect(m_scene, &Scene::circuitHasChanged, this, &MinimapWidget::invalidateCache);
    }

    if (m_view) {
        connect(m_view, &GraphicsView::zoomChanged, this, &MinimapWidget::invalidateCache);
        if (m_view->horizontalScrollBar())
            connect(m_view->horizontalScrollBar(), &QScrollBar::valueChanged, this, &MinimapWidget::invalidateCache);
        if (m_view->verticalScrollBar())
            connect(m_view->verticalScrollBar(), &QScrollBar::valueChanged, this, &MinimapWidget::invalidateCache);
        if (m_view->viewport())
            m_view->viewport()->installEventFilter(this);
    }
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
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.fillRect(rect(), palette().window().color());

    // Source rect: prefer items bounding rect, fallback to sceneRect
    QRectF src = m_scene->itemsBoundingRect();
    if (!src.isValid() || src.isEmpty())
        src = m_scene->sceneRect();
    if (!src.isValid() || src.isEmpty())
        return;

    // Regenerate cache if needed
    if (m_cacheDirty || m_cache.size() != size()) {
        m_cache = QPixmap(size());
        m_cache.fill(palette().window().color());
        QPainter pixmapPainter(&m_cache);
        pixmapPainter.setRenderHint(QPainter::Antialiasing, false);
        m_scene->render(&pixmapPainter, QRectF(0, 0, width(), height()), src, Qt::KeepAspectRatio);
        m_cacheDirty = false;
    }

    painter.drawPixmap(0, 0, m_cache);

    // Draw viewport rectangle in minimap coordinates
    const QRectF visible = m_view->mapToScene(m_view->viewport()->rect()).boundingRect();

    // Map visible rect to widget coordinates (account for KeepAspectRatio letterboxing)
    const double sx = src.width() > 0 ? static_cast<double>(width()) / src.width() : 1.0;
    const double sy = src.height() > 0 ? static_cast<double>(height()) / src.height() : 1.0;
    const double scale = qMin(sx, sy);
    const double ox = src.left();
    const double oy = src.top();
    const double usedW = src.width() * scale;
    const double usedH = src.height() * scale;
    const double dx = (width() - usedW) * 0.5;
    const double dy = (height() - usedH) * 0.5;

    QRectF vr(
        dx + (visible.left() - ox) * scale,
        dy + (visible.top() - oy) * scale,
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

    // Convert widget position to scene coordinates using src mapping used in paintEvent
    QRectF src = m_scene->itemsBoundingRect();
    if (!src.isValid() || src.isEmpty()) {
        src = m_scene->sceneRect();
    }
    if (!src.isValid() || src.isEmpty()) {
        return;
    }

    const double sx = src.width() > 0 ? static_cast<double>(width()) / src.width() : 1.0;
    const double sy = src.height() > 0 ? static_cast<double>(height()) / src.height() : 1.0;
    const double scale = qMin(sx, sy);
    const double dx = (width() - src.width() * scale) * 0.5;
    const double dy = (height() - src.height() * scale) * 0.5;

    const QPointF pos = event->pos();
    const double sceneX = src.left() + (pos.x() - dx) / scale;
    const double sceneY = src.top() + (pos.y() - dy) / scale;

    m_view->centerOn(QPointF(sceneX, sceneY));
    invalidateCache();
    // start drag
    m_dragging = true;
    m_lastPos = event->pos();
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
    m_cacheDirty = true;
    update();
}

bool MinimapWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == (m_view ? m_view->viewport() : nullptr) && event->type() == QEvent::Resize) {
        m_cacheDirty = true;
        update();
        return QWidget::eventFilter(watched, event);
    }
    return QWidget::eventFilter(watched, event);
}

void MinimapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging || !m_scene || !m_view) {
        return QWidget::mouseMoveEvent(event), void();
    }

    // Compute scene position for last and current minimap positions, then shift center
    QRectF src = m_scene->itemsBoundingRect();
    if (!src.isValid() || src.isEmpty())
        src = m_scene->sceneRect();
    if (!src.isValid() || src.isEmpty())
        return;

    const double sx = src.width() > 0 ? static_cast<double>(width()) / src.width() : 1.0;
    const double sy = src.height() > 0 ? static_cast<double>(height()) / src.height() : 1.0;
    const double scale = qMin(sx, sy);
    const double usedW = src.width() * scale;
    const double usedH = src.height() * scale;
    const double dx = (width() - usedW) * 0.5;
    const double dy = (height() - usedH) * 0.5;

    const QPointF pcur = event->pos();
    const QPointF pprev = m_lastPos;

    const QPointF sceneCur(src.left() + (pcur.x() - dx) / scale,
                          src.top()  + (pcur.y() - dy) / scale);
    const QPointF scenePrev(src.left() + (pprev.x() - dx) / scale,
                           src.top()  + (pprev.y() - dy) / scale);

    const QPointF delta = scenePrev - sceneCur; // move center by delta to follow drag

    const QPointF currentCenter = m_view->mapToScene(m_view->viewport()->rect().center());
    m_view->centerOn(currentCenter + delta);

    m_lastPos = event->pos();
    invalidateCache();
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
