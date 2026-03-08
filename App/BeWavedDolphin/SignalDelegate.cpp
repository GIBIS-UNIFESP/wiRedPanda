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

void SignalDelegate::loadPixmaps()
{
    // Pre-render waveform segment SVGs to pixmaps once at startup.
    // Green = output rows, Blue = input rows. 100x38 is the baseline cell size;
    // paint() stretches these pixmaps to fill the actual cell at render time.
    m_lowGreen     = QPixmap(":/Interface/Dolphin/low_green.svg").scaled(100, 38);
    m_highGreen    = QPixmap(":/Interface/Dolphin/high_green.svg").scaled(100, 38);
    m_fallingGreen = QPixmap(":/Interface/Dolphin/falling_green.svg").scaled(100, 38);
    m_risingGreen  = QPixmap(":/Interface/Dolphin/rising_green.svg").scaled(100, 38);

    m_lowBlue     = QPixmap(":/Interface/Dolphin/low_blue.svg").scaled(100, 38);
    m_highBlue    = QPixmap(":/Interface/Dolphin/high_blue.svg").scaled(100, 38);
    m_fallingBlue = QPixmap(":/Interface/Dolphin/falling_blue.svg").scaled(100, 38);
    m_risingBlue  = QPixmap(":/Interface/Dolphin/rising_blue.svg").scaled(100, 38);
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
