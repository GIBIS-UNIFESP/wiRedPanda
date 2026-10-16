// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/ElementWarningLabel.h"

#include <QFontMetricsF>
#include <QPainter>
#include <QToolTip>

#include "App/Core/ThemeManager.h"

namespace {
constexpr qreal kEW_PaddingX = 6.0;
constexpr qreal kEW_PaddingY = 4.0;
constexpr qreal kEW_ElementGap = 8.0;
constexpr qreal kEW_HoverLabelZ = 1000.0;
} // namespace

ElementWarningLabel::ElementWarningLabel(const QStringList &lines, QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , m_lines(lines)
{
    m_font = QToolTip::font();

    QFontMetricsF metrics(m_font);
    qreal maxw = 0;
    for (const auto &l : m_lines) {
        maxw = std::max(maxw, metrics.horizontalAdvance(l));
    }
    const qreal width = maxw + (2.0 * kPaddingX);
    const qreal height = (metrics.height() * qreal(m_lines.size())) + (2.0 * kEW_PaddingY);

    // Anchor the chip above the element by default; the caller will position the item.
    m_bounds = QRectF(-width / 2.0, -(kEW_ElementGap + height), width, height);

    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setZValue(kEW_HoverLabelZ);
    setAcceptedMouseButtons(Qt::NoButton);
}

QRectF ElementWarningLabel::boundingRect() const
{
    return m_bounds.adjusted(-1.0, -1.0, 1.0, 1.0);
}

QPainterPath ElementWarningLabel::shape() const
{
    return {};
}

void ElementWarningLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    const auto attributes = ThemeManager::attributes();

    painter->setPen(QPen(attributes.m_portHoverLabelText, 1.0));
    painter->setBrush(attributes.m_portHoverLabelBg);
    painter->drawRect(m_bounds);

    painter->setFont(m_font);
    painter->setPen(attributes.m_portHoverLabelText);

    const qreal lineH = QFontMetricsF(m_font).height();
    qreal y = m_bounds.top() + kPaddingY;
    for (const auto &l : m_lines) {
        painter->drawText(QRectF(m_bounds.left() + kPaddingX, y, m_bounds.width() - (2.0 * kPaddingX), lineH), Qt::AlignLeft | Qt::AlignVCenter, l);
        y += lineH;
    }
}
