// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickMinimap.h"

#include <QBuffer>
#include <QUndoStack>

#include "App/Core/Settings.h"
#include "App/Core/ThemeManager.h"
#include "App/QuickShell/Canvas/CanvasItem.h"

namespace {
// Mirrors MinimapWidget::MinimapWidget()'s setMinimumSize(160, 120) and
// WorkSpace::applyMinimapGeometry()'s local margin=12 -- also duplicated as Minimap.qml's own
// margin/minW/minH readonly properties, which this method's result feeds into.
constexpr qreal kMinimapMargin = 12.0;
constexpr qreal kMinimapMinWidth = 160.0;
constexpr qreal kMinimapMinHeight = 120.0;
} // namespace

QuickMinimap::QuickMinimap(QObject *parent)
    : QObject(parent)
    , m_visible(Settings::minimapVisible())
{
    // ~5fps, matching MinimapWidget::m_throttle's own interval -- see this class's header doc
    // comment for why both content and view changes share this one throttle here, unlike
    // MinimapWidget's split (content throttled, view unthrottled).
    m_regenTimer.setSingleShot(true);
    m_regenTimer.setInterval(200);
    connect(&m_regenTimer, &QTimer::timeout, this, &QuickMinimap::regenerate);

    // Mirrors MinimapWidget.cpp's identical ThemeManager::instance().themeChanged ->
    // invalidateCache() connection: renderMinimapImage() paints real circuit-element colors
    // that follow the current theme, so a switch must invalidate the cached thumbnail --
    // otherwise it keeps showing the old theme's colors until the next real circuit edit or
    // pan/zoom. Connected once here (not per-setCanvas()) since the theme is process-wide, not
    // per-canvas; requestRegen() -> regenerate() already no-ops gracefully with no bound canvas.
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &QuickMinimap::requestRegen);
}

void QuickMinimap::setCanvas(CanvasItem *canvas)
{
    if (m_canvas) {
        disconnect(m_canvas, nullptr, this, nullptr);
    }
    m_canvas = canvas;
    if (m_canvas) {
        connect(m_canvas->undoStack(), &QUndoStack::indexChanged, this, &QuickMinimap::requestRegen);
        connect(m_canvas, &CanvasItem::zoomChanged, this, &QuickMinimap::requestRegen);
        requestRegen();
    } else {
        m_imageUrl.clear();
        m_viewportRect = QRectF();
        emit refreshed();
    }
}

void QuickMinimap::setVisible(bool visible)
{
    if (m_visible == visible) {
        return;
    }
    m_visible = visible;
    Settings::setMinimapVisible(visible);
    emit visibleChanged();
    requestRegen();
}

void QuickMinimap::setMinimapWidth(qreal width)
{
    if (qFuzzyCompare(m_minimapWidth, width)) {
        return;
    }
    m_minimapWidth = width;
    emit minimapSizeChanged();
    requestRegen();
}

void QuickMinimap::setMinimapHeight(qreal height)
{
    if (qFuzzyCompare(m_minimapHeight, height)) {
        return;
    }
    m_minimapHeight = height;
    emit minimapSizeChanged();
    requestRegen();
}

qreal QuickMinimap::contentAspectRatio() const
{
    if (!m_canvas) {
        return 1.0;
    }
    const QRectF bounds = m_canvas->elementsBoundingRect();
    if (!bounds.isValid() || bounds.width() <= 0.0 || bounds.height() <= 0.0) {
        return 1.0;
    }
    return bounds.width() / bounds.height();
}

void QuickMinimap::navigateTo(qreal localX, qreal localY) const
{
    if (!m_canvas || m_minimapWidth <= 0.0 || m_minimapHeight <= 0.0) {
        return;
    }
    const QRectF content = m_canvas->minimapContentRect(m_minimapWidth, m_minimapHeight);
    if (content.isEmpty()) {
        return;
    }
    const qreal scale = m_minimapWidth / content.width();
    const QPointF worldPoint(
        qBound(content.left(), content.left() + localX / scale, content.right()),
        qBound(content.top(), content.top() + localY / scale, content.bottom()));
    m_canvas->centerOn(worldPoint);
}

QRectF QuickMinimap::clampGeometry(const QRectF &candidate, qreal parentWidth, qreal parentHeight) const
{
    const qreal maxWidth = qMax(kMinimapMinWidth, parentWidth - 2 * kMinimapMargin);
    const qreal maxHeight = qMax(kMinimapMinHeight, parentHeight - 2 * kMinimapMargin);
    const qreal width = qBound(kMinimapMinWidth, candidate.width(), maxWidth);
    const qreal height = qBound(kMinimapMinHeight, candidate.height(), maxHeight);
    const qreal x = qBound(kMinimapMargin, candidate.x(), qMax(kMinimapMargin, parentWidth - width - kMinimapMargin));
    const qreal y = qBound(kMinimapMargin, candidate.y(), qMax(kMinimapMargin, parentHeight - height - kMinimapMargin));
    return QRectF(x, y, width, height);
}

void QuickMinimap::requestRegen()
{
    m_regenTimer.start();
}

void QuickMinimap::regenerate()
{
    if (!m_visible || !m_canvas || m_minimapWidth <= 0.0 || m_minimapHeight <= 0.0) {
        return;
    }

    const QImage image = m_canvas->renderMinimapImage(m_minimapWidth, m_minimapHeight);
    m_imageUrl.clear();
    if (!image.isNull()) {
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        m_imageUrl = QStringLiteral("data:image/png;base64,") + QString::fromLatin1(bytes.toBase64());
    }

    const QRectF content = m_canvas->minimapContentRect(m_minimapWidth, m_minimapHeight);
    if (content.isEmpty()) {
        m_viewportRect = QRectF();
    } else {
        const QRectF visible = m_canvas->visibleWorldRect();
        const qreal scale = m_minimapWidth / content.width();
        m_viewportRect = QRectF(
            (visible.left() - content.left()) * scale,
            (visible.top() - content.top()) * scale,
            visible.width() * scale,
            visible.height() * scale);
    }

    emit refreshed();
}
