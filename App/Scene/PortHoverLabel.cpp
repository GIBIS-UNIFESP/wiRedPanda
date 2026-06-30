// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Scene/PortHoverLabel.h"

#include "App/Core/ThemeManager.h"

#include <QFontMetricsF>
#include <QPainter>
#include <QToolTip>

namespace {
constexpr qreal kPaddingX = 5.0;   ///< Horizontal text padding inside the chip.
constexpr qreal kPaddingY = 2.0;   ///< Vertical text padding inside the chip.
/// Gap (in device pixels) between the anchored port and the chip.
constexpr qreal kPortGap = 8.0;
/// Drawn above every element and wire so the label is never occluded.
constexpr qreal kHoverLabelZ = 1000.0;
} // namespace

PortHoverLabel::PortHoverLabel(const QString &text, const Side side, QGraphicsItem *parent)
    : QGraphicsObject(parent)
    , m_text(text)
{
    // Match the native tooltip font so the chip is indistinguishable from the hovered
    // port's own QToolTip.
    m_font = QToolTip::font();

    const QFontMetricsF metrics(m_font);
    const qreal width = metrics.horizontalAdvance(m_text) + (2.0 * kPaddingX);
    const qreal height = metrics.height() + (2.0 * kPaddingY);

    // The item is anchored at the port (origin); offset the drawn rect to the side away
    // from the element body, centred on the port along the perpendicular axis.
    switch (side) {
    case Side::Left:
        m_bounds = QRectF(-(kPortGap + width), -height / 2.0, width, height);
        break;
    case Side::Right:
        m_bounds = QRectF(kPortGap, -height / 2.0, width, height);
        break;
    case Side::Top:
        m_bounds = QRectF(-width / 2.0, -(kPortGap + height), width, height);
        break;
    case Side::Bottom:
        m_bounds = QRectF(-width / 2.0, kPortGap, width, height);
        break;
    }

    // Keep a constant on-screen size regardless of zoom, exactly like a tooltip widget.
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    setZValue(kHoverLabelZ);
    setAcceptedMouseButtons(Qt::NoButton);
}

QRectF PortHoverLabel::boundingRect() const
{
    // Inflate by the 1px pen so the border is never clipped.
    return m_bounds.adjusted(-1.0, -1.0, 1.0, 1.0);
}

QPainterPath PortHoverLabel::shape() const
{
    // Purely decorative: stay out of scene hit-testing (Scene::itemAt(), rubber-band
    // selection, etc.) so the chip can never shadow a port, wire, or element underneath it.
    return {};
}

void PortHoverLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    const auto attributes = ThemeManager::attributes();

    // Square 1px border + flat fill, mirroring the application's QToolTip stylesheet.
    painter->setPen(QPen(attributes.m_portHoverLabelText, 1.0));
    painter->setBrush(attributes.m_portHoverLabelBg);
    painter->drawRect(m_bounds);

    painter->setFont(m_font);
    painter->setPen(attributes.m_portHoverLabelText);
    painter->drawText(m_bounds, Qt::AlignCenter, m_text);
}
