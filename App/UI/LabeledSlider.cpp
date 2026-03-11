// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/LabeledSlider.h"

#include <algorithm>

#include <QPainter>
#include <QSlider>
#include <QStyle>

LabeledSlider::LabeledSlider(QWidget *parent)
    : QSlider(parent)
{
    // Reserve 35px below the slider track for the fraction labels drawn in paintEvent;
    // without this the labels are clipped by the widget boundary
    setContentsMargins(0, 0, 0, 35);
}

void LabeledSlider::paintEvent(QPaintEvent *event)
{
    // Call parent to draw the slider
    QSlider::paintEvent(event);

    // Draw fraction labels under each tick mark.  Only runs when tick marks are
    // enabled so ordinary QSlider users aren't affected.
    if (tickPosition() != QSlider::NoTicks) {
        QPainter painter(this);
        painter.setFont(font());

        const int min = minimum();
        const int max = maximum();
        const int interval = tickInterval();
        // Number of gaps between ticks; labels at both ends means tickCount+1 labels total.
        const int tickCount = (max - min) / interval;

        if (tickCount <= 0) {
            return;
        }

        // PM_SliderLength is the groove handle length; halving it gives the inset where
        // the track actually starts and ends, matching how Qt positions tick marks
        const int trackStart = style()->pixelMetric(QStyle::PM_SliderLength) / 2;
        const int trackEnd = width() - trackStart;
        const int trackWidth = trackEnd - trackStart;

        for (int i = 0; i <= tickCount; ++i) {
            const int value = min + (i * interval);
            if (value > max) {
                break;
            }

            // Normalise value to [0,1] then map onto the actual pixel track width.
            const double pos = static_cast<double>(value - min) / (max - min);
            const int xPos = trackStart + static_cast<int>(pos * trackWidth);

            // The slider represents a clock phase offset in eighths of a period.
            // Dividing by 8 converts the integer tick position to a readable fraction.
            const float periodFraction = static_cast<float>(value) / 8.0f;

            // Use human-readable vulgar fraction strings for the canonical eighths so
            // labels are compact and informative; fall back to a 3-decimal float for
            // any unexpected values outside the designed ±4 step range
            QString labelText;
            if (periodFraction == 0.0f) {
                labelText = "0";
            } else if (periodFraction == 0.125f) {
                labelText = "1/8";
            } else if (periodFraction == 0.25f) {
                labelText = "1/4";
            } else if (periodFraction == 0.375f) {
                labelText = "3/8";
            } else if (periodFraction == 0.5f) {
                labelText = "1/2";
            } else if (periodFraction == -0.125f) {
                labelText = "-1/8";
            } else if (periodFraction == -0.25f) {
                labelText = "-1/4";
            } else if (periodFraction == -0.375f) {
                labelText = "-3/8";
            } else if (periodFraction == -0.5f) {
                labelText = "-1/2";
            } else {
                labelText = QString::number(periodFraction, 'f', 3);
            }

            const QFontMetrics metrics(font());
            const int textWidth = metrics.horizontalAdvance(labelText);
            const int textHeight = metrics.height();

            // Centre the label under its tick but clamp to a 2px inset so the
            // first and last labels don't overflow the widget rectangle
            int textX = xPos - textWidth / 2;
            textX = qMax(textX, 2);
            textX = qMin(textX, width() - textWidth - 2);

            painter.drawText(textX, height() - textHeight - 2, textWidth, textHeight, Qt::AlignLeft, labelText);
        }
    }
}
