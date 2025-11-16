// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "labeledslider.h"

#include <QPainter>
#include <QSlider>
#include <algorithm>

LabeledSlider::LabeledSlider(QWidget *parent)
    : QSlider(parent)
{
    // Increase bottom margin to accommodate labels
    setContentsMargins(0, 0, 0, 35);
}

void LabeledSlider::paintEvent(QPaintEvent *event)
{
    // Call parent to draw the slider
    QSlider::paintEvent(event);

    // Draw labels under tick marks
    if (tickPosition() != QSlider::NoTicks) {
        QPainter painter(this);
        painter.setFont(font());

        const int min = minimum();
        const int max = maximum();
        const int interval = tickInterval();
        const int tickCount = (max - min) / interval;

        // Only draw labels if we have a reasonable number of ticks
        if (tickCount <= 0) {
            return;
        }

        const int trackStart = style()->pixelMetric(QStyle::PM_SliderLength) / 2;
        const int trackEnd = width() - trackStart;
        const int trackWidth = trackEnd - trackStart;

        // Draw each label
        for (int i = 0; i <= tickCount; ++i) {
            const int value = min + (i * interval);
            if (value > max) {
                break;
            }

            // Calculate position of this tick
            const double pos = static_cast<double>(value - min) / (max - min);
            const int xPos = trackStart + static_cast<int>(pos * trackWidth);

            // Convert slider value to period fraction (slider / 8)
            const float periodFraction = static_cast<float>(value) / 8.0f;

            // Format the label as a fraction string
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

            // Clamp the text position to ensure it doesn't go outside the widget bounds
            int textX = xPos - textWidth / 2;
            textX = qMax(textX, 2);  // Minimum 2 pixels from left edge
            textX = qMin(textX, width() - textWidth - 2);  // Maximum 2 pixels from right edge

            painter.drawText(textX, height() - textHeight - 2, textWidth, textHeight, Qt::AlignLeft, labelText);
        }
    }
}
