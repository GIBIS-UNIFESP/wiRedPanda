// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickPortHoverOverlay.h"

#include "App/QuickShell/Canvas/CanvasItem.h"

QuickPortHoverOverlay::QuickPortHoverOverlay(QObject *parent)
    : QObject(parent)
{
}

void QuickPortHoverOverlay::setCanvas(CanvasItem *canvas)
{
    if (m_canvas) {
        disconnect(m_canvas, nullptr, this, nullptr);
    }
    m_canvas = canvas;
    if (!m_chips.isEmpty()) {
        m_chips.clear();
        emit refreshed();
    }
    if (m_canvas) {
        connect(m_canvas, &CanvasItem::portHoverChanged, this, &QuickPortHoverOverlay::onPortHoverChanged);
    }
}

void QuickPortHoverOverlay::onPortHoverChanged(const QVariantList &chips)
{
    m_chips = chips;
    emit refreshed();
}
