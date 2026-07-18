// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickMinimap.h"

#include <QBuffer>
#include <QUndoStack>

#include "App/Core/Settings.h"
#include "App/QuickShell/Canvas/CanvasItem.h"

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
