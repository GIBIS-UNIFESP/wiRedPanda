// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/LabeledSlider.h"

#include <algorithm>
#include <limits>
#include <tuple>

#include <QPainter>
#include <QSlider>
#include <QStyle>

LabeledSlider::LabeledSlider(QWidget *parent)
    : QSlider(parent)
{
    // No contentsMargins() call here — see sizeHint() for why that approach doesn't
    // work (it doesn't grow the size the owning layout actually allocates).
}

QSize LabeledSlider::sizeHint() const
{
    QSize hint = QSlider::sizeHint();
    if (tickPosition() != QSlider::NoTicks) {
        hint.setHeight(hint.height() + fontMetrics().height() + 4);
    }
    return hint;
}

QSize LabeledSlider::minimumSizeHint() const
{
    return sizeHint();
}

// Converts a slider tick \a value (eighths of a clock period) to a compact fraction string.
// Uses human-readable vulgar fractions for the canonical eighths; falls back to a 3-decimal
// float for any unexpected value outside the designed range.
static QString periodFractionLabel(const int value)
{
    const float periodFraction = static_cast<float>(value) / 8.0f;

    if (periodFraction == 0.0f)     { return "0";    }
    if (periodFraction == 0.125f)   { return "1/8";  }
    if (periodFraction == 0.25f)    { return "1/4";  }
    if (periodFraction == 0.375f)   { return "3/8";  }
    if (periodFraction == 0.5f)     { return "1/2";  }
    if (periodFraction == -0.125f)  { return "-1/8"; }
    if (periodFraction == -0.25f)   { return "-1/4"; }
    if (periodFraction == -0.375f)  { return "-3/8"; }
    if (periodFraction == -0.5f)    { return "-1/2"; }
    return QString::number(static_cast<double>(periodFraction), 'f', 3);
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

        // tickInterval() defaults to 0 (QSlider) until explicitly set; guard before the
        // divide below rather than relying on every future caller pairing setTickPosition()
        // with a non-zero setTickInterval().
        if (interval <= 0) {
            return;
        }

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
        const QFontMetrics metrics(font());
        const int textHeight = metrics.height();

        // Computes the clamped draw position for tick index \a i's label.
        auto labelRectFor = [&](const int i) {
            const int value = min + (i * interval);
            const double pos = static_cast<double>(value - min) / (max - min);
            const int xPos = trackStart + static_cast<int>(pos * trackWidth);
            const QString text = periodFractionLabel(value);
            const int textWidth = metrics.horizontalAdvance(text);
            // Centre the label under its tick but clamp to a 2px inset so the
            // first and last labels don't overflow the widget rectangle.
            int textX = xPos - textWidth / 2;
            textX = (std::max)(textX, 2);
            textX = (std::min)(textX, width() - textWidth - 2);
            return std::tuple(text, textX, textWidth);
        };

        constexpr int minLabelGap = 4;

        // The endpoints (min/max) are the most useful reference points, so their space
        // is reserved up front; interior labels are only drawn where they fit between
        // whatever was drawn immediately before them and the reserved right endpoint —
        // never overlapping either. Adapts to any widget width/font instead of assuming
        // a fixed "label every Nth tick" stride.
        const auto [maxText, maxTextX, maxTextWidth] = labelRectFor(tickCount);

        int lastLabelRight = std::numeric_limits<int>::min();
        for (int i = 0; i < tickCount; ++i) {
            const auto [text, textX, textWidth] = labelRectFor(i);
            const bool isEndpoint = (i == 0);
            const bool collidesWithPrevious = !isEndpoint && textX < lastLabelRight + minLabelGap;
            const bool collidesWithMaxEndpoint = textX + textWidth + minLabelGap > maxTextX;
            if (collidesWithPrevious || (!isEndpoint && collidesWithMaxEndpoint)) {
                continue;
            }
            lastLabelRight = textX + textWidth;
            painter.drawText(textX, height() - textHeight - 2, textWidth, textHeight, Qt::AlignLeft, text);
        }

        painter.drawText(maxTextX, height() - textHeight - 2, maxTextWidth, textHeight, Qt::AlignLeft, maxText);
    }
}
