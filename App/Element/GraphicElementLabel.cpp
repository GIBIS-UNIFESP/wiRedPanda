// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Element/GraphicElementLabel.h"

#include <QPainter>

#include "App/Element/GraphicElement.h"

QRectF GraphicElementLabel::boundingRectInOwnerLocal() const
{
    const QRectF local = boundingRect();
    QPolygonF corners;
    corners << local.topLeft() << local.topRight() << local.bottomRight() << local.bottomLeft();

    QPolygonF mapped;
    for (const QPointF &corner : std::as_const(corners)) {
        mapped << mapToOwnerLocal(corner);
    }

    return mapped.boundingRect();
}

QRectF GraphicElementLabel::sceneBoundingRect() const
{
    const QRectF ownerLocal = boundingRectInOwnerLocal();
    QPolygonF corners;
    corners << ownerLocal.topLeft() << ownerLocal.topRight()
            << ownerLocal.bottomRight() << ownerLocal.bottomLeft();

    QPolygonF scenePoly;
    for (const QPointF &corner : std::as_const(corners)) {
        scenePoly << m_owner->pointToScene(corner);
    }

    return scenePoly.boundingRect();
}

void GraphicElementLabel::paint(QPainter *painter) const
{
    if (!m_visible || m_text.isEmpty()) {
        return;
    }

    painter->save();
    painter->setFont(m_font);
    painter->setPen(QPen(m_brush, 0));
    painter->drawText(boundingRect(), Qt::AlignLeft | Qt::AlignTop, m_text);
    painter->restore();
}
