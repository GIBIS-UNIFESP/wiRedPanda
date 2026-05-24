// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/SignalDelegate.h"

#include <QPainter>
#include <QStyleOptionViewItem>

SignalDelegate::SignalDelegate(QObject *parent)
    : QItemDelegate(parent)
{
    loadPixmaps();
}

namespace {

QPixmap createWaveformPixmap(const bool isHigh, const bool hasEdge, const QColor &color)
{
    constexpr int kWidth  = 100;
    constexpr int kHeight = 38;
    constexpr qreal kYScale = static_cast<qreal>(kHeight) / 30.0;

    QPixmap pixmap(kWidth, kHeight);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, false);

    QColor shadow(color);
    shadow.setAlphaF(0.5);

    if (isHigh) {
        painter.fillRect(QRectF(0, 12.0 * kYScale, kWidth, 18.0 * kYScale), shadow);
    } else if (hasEdge) {
        painter.fillRect(QRectF(0, 22.0 * kYScale, kWidth, 8.0 * kYScale), shadow);
    } else {
        painter.fillRect(QRectF(0, 24.0 * kYScale, kWidth, 6.0 * kYScale), shadow);
    }

    const qreal lineY = (isHigh ? 8.0 : 20.0) * kYScale;
    painter.fillRect(QRectF(0, lineY, kWidth, 4.0 * kYScale), color);

    if (hasEdge) {
        painter.fillRect(QRectF(0, 8.0 * kYScale, 4.0, 16.0 * kYScale), color);
    }

    return pixmap;
}

} // anonymous namespace

void SignalDelegate::loadPixmaps()
{
    const QColor green(0x00, 0x80, 0x00);
    const QColor blue(0x75, 0x8E, 0xFF);

    m_lowGreen     = createWaveformPixmap(false, false, green);
    m_highGreen    = createWaveformPixmap(true,  false, green);
    m_risingGreen  = createWaveformPixmap(true,  true,  green);
    m_fallingGreen = createWaveformPixmap(false, true,  green);

    m_lowBlue      = createWaveformPixmap(false, false, blue);
    m_highBlue     = createWaveformPixmap(true,  false, blue);
    m_risingBlue   = createWaveformPixmap(true,  true,  blue);
    m_fallingBlue  = createWaveformPixmap(false, true,  blue);
}

QPixmap SignalDelegate::pixmapFor(const int value, const bool isInput, const bool hasPrev, const int prevValue) const
{
    if (value == 0) {
        const bool isFalling = hasPrev && (prevValue == 1);
        return isInput ? (isFalling ? m_fallingBlue : m_lowBlue)
                       : (isFalling ? m_fallingGreen : m_lowGreen);
    } else {
        const bool isRising = hasPrev && (prevValue == 0);
        return isInput ? (isRising ? m_risingBlue : m_highBlue)
                       : (isRising ? m_risingGreen : m_highGreen);
    }
}

void SignalDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QVariant value = index.data(Qt::DecorationRole);

    // In waveform (Line) mode, cells store a QPixmap representing the signal segment
    // (low, high, rising, falling). In Number mode there is no pixmap, so we fall through.
    if (value.canConvert<QPixmap>()) {
        const QPixmap pixmap = qvariant_cast<QPixmap>(value);
        const QPixmap scaled = pixmap.scaled(option.rect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        // Draw the selection highlight behind the waveform image rather than over it
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        }

        const int x = option.rect.x() + (option.rect.width()  - scaled.width())  / 2;
        const int y = option.rect.y() + (option.rect.height() - scaled.height()) / 2;
        painter->drawPixmap(x, y, scaled);
        return;
    }

    // Number mode: delegate to the standard item delegate to render the "0"/"1" text
    QItemDelegate::paint(painter, option, index);
}
