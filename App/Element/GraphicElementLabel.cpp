// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElementLabel.h"

#include <QFontMetricsF>
#include <QPainter>
#include <QPen>

QRectF GraphicElementLabel::boundingRect() const
{
    if (m_text.isEmpty()) {
        return {};
    }
    const QFontMetricsF metrics(m_font);
    return QRectF(0, 0, metrics.horizontalAdvance(m_text), metrics.height());
}

void GraphicElementLabel::paint(QPainter *painter) const
{
    if (!m_visible || m_text.isEmpty()) {
        return;
    }

    painter->save();
    painter->setFont(m_font);
    painter->setPen(QPen(m_brush.color()));
    // QGraphicsSimpleTextItem draws with its bounding rect's top-left at local (0,0); drawText()
    // positions the baseline, so offset down by the font's ascent to match.
    painter->drawText(QPointF(0, QFontMetricsF(m_font).ascent()), m_text);
    painter->restore();
}
