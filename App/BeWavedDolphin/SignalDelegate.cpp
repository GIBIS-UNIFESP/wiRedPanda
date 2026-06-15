// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/SignalDelegate.h"

#include <QPainter>
#include <QStyleOptionViewItem>

namespace
{
// Waveform colors: green for output rows, blue for input rows.
const QColor kOutputColor(0, 128, 0);        ///< #008000
const QColor kInputColor(0x75, 0x8e, 0xff);  ///< #758eff

// Geometry, as fractions of the cell, decoded from the original 100x30 SVGs.
constexpr double kLineThickness = 4.0 / 30.0;   ///< Horizontal plateau line thickness.
constexpr double kBarWidth      = 4.0 / 100.0;  ///< Leading edge (vertical bar) width.
constexpr double kHighTop       = 8.0 / 30.0;   ///< Top of the high plateau line.
constexpr double kLowTop        = 20.0 / 30.0;  ///< Top of the low plateau line.
constexpr double kHighBottom    = 12.0 / 30.0;  ///< Bottom of the high plateau line.
constexpr double kLowBottom     = 24.0 / 30.0;  ///< Bottom of the low plateau line.
} // namespace

SignalDelegate::SignalDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

WaveSegment SignalDelegate::segmentFor(const int value, const bool hasPrev, const int prevValue)
{
    if (value == 0) {
        return (hasPrev && (prevValue == 1)) ? WaveSegment::Falling : WaveSegment::Low;
    }

    return (hasPrev && (prevValue == 0)) ? WaveSegment::Rising : WaveSegment::High;
}

void SignalDelegate::drawWaveform(QPainter *painter, const QRectF &cell, const WaveSegment seg, const bool isInput) const
{
    const QColor color = isInput ? kInputColor : kOutputColor;
    QColor bandColor = color;
    bandColor.setAlpha(128); // 50% opacity translucent fill

    const double x = cell.x();
    const double y = cell.y();
    const double w = cell.width();
    const double h = cell.height();

    // High plateau for High/Rising, low plateau for Low/Falling.
    const bool high = (seg == WaveSegment::High) || (seg == WaveSegment::Rising);
    const double lineTop    = (high ? kHighTop : kLowTop) * h;
    const double lineBottom = (high ? kHighBottom : kLowBottom) * h;
    const double thickness  = kLineThickness * h;

    // Draw order matches the original SVGs: band, then plateau line, then edge bar.
    painter->fillRect(QRectF(x, y + lineBottom, w, h - lineBottom), bandColor);
    painter->fillRect(QRectF(x, y + lineTop, w, thickness), color);

    if ((seg == WaveSegment::Rising) || (seg == WaveSegment::Falling)) {
        // Vertical connector joining the high and low plateau levels at the cell's left edge.
        const double barTop    = kHighTop * h;
        const double barBottom = kLowBottom * h;
        painter->fillRect(QRectF(x, y + barTop, kBarWidth * w, barBottom - barTop), color);
    }
}

void SignalDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QVariant segment = index.data(SegmentRole);

    // In waveform (Line) mode each cell carries a WaveSegment. In Number mode the
    // role is unset, so we fall through to the default text rendering.
    if (segment.isValid()) {
        // Draw the selection highlight behind the waveform rather than over it
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        }

        drawWaveform(painter, option.rect, static_cast<WaveSegment>(segment.toInt()), index.data(InputRole).toBool());
        return;
    }

    // Number mode: delegate to the standard item delegate to render the "0"/"1" text
    QItemDelegate::paint(painter, option, index);
}
