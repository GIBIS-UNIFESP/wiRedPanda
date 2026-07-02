// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/TemporalWaveformWidget.h"

#include <algorithm>

#include <QPainter>
#include <QWheelEvent>

#include "App/Simulation/WaveformRecorder.h"

// ============================================================================
// WaveformLabelWidget — fixed signal name column
// ============================================================================

WaveformLabelWidget::WaveformLabelWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedWidth(LABEL_WIDTH);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
}

QSize WaveformLabelWidget::sizeHint() const
{
    const int traceCount = m_recorder ? m_recorder->traceCount() : 0;
    const int h = WaveformLayout::RulerHeight + traceCount * (WaveformLayout::TraceHeight + WaveformLayout::TraceMargin);
    return {LABEL_WIDTH, qMax(h, WaveformLayout::RulerHeight + WaveformLayout::TraceHeight)};
}

void WaveformLabelWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.fillRect(rect(), palette().base());

    if (!m_recorder || m_recorder->traceCount() == 0) {
        return;
    }

    // Empty ruler area to align with the waveform ruler
    painter.fillRect(0, 0, LABEL_WIDTH, WaveformLayout::RulerHeight, palette().window());

    for (int i = 0; i < m_recorder->traceCount(); ++i) {
        const int y = WaveformLayout::RulerHeight + i * (WaveformLayout::TraceHeight + WaveformLayout::TraceMargin);

        painter.fillRect(0, y, LABEL_WIDTH - 2, WaveformLayout::TraceHeight, palette().alternateBase());
        painter.setPen(palette().color(QPalette::Text));
        painter.drawText(QRect(4, y, LABEL_WIDTH - 8, WaveformLayout::TraceHeight),
                         Qt::AlignVCenter | Qt::AlignRight,
                         m_recorder->trace(i).name);
    }
}

// ============================================================================
// TemporalWaveformWidget — scrollable waveform traces
// ============================================================================

TemporalWaveformWidget::TemporalWaveformWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(WaveformLayout::RulerHeight + WaveformLayout::TraceHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void TemporalWaveformWidget::setPixelsPerNs(double ppn)
{
    m_pixelsPerNs = qBound(1e-7, ppn, 10.0);
    updateGeometry();
    update();
}

void TemporalWaveformWidget::zoomIn()
{
    setPixelsPerNs(m_pixelsPerNs * 2.0);
}

void TemporalWaveformWidget::zoomOut()
{
    setPixelsPerNs(m_pixelsPerNs / 2.0);
}

void TemporalWaveformWidget::zoomFit()
{
    if (!m_recorder || m_recorder->traceCount() == 0) {
        return;
    }
    const SimTime maxTime = m_recorder->maxTime();
    if (maxTime == 0) {
        return;
    }
    const int availableWidth = width();
    if (availableWidth <= 0) {
        return;
    }
    setPixelsPerNs(static_cast<double>(availableWidth) / static_cast<double>(maxTime));
}

QSize TemporalWaveformWidget::sizeHint() const
{
    const int traceCount = m_recorder ? m_recorder->traceCount() : 0;
    const int h = WaveformLayout::RulerHeight + traceCount * (WaveformLayout::TraceHeight + WaveformLayout::TraceMargin);

    int w = 400;
    if (m_recorder && m_recorder->maxTime() > 0) {
        // Qt hard-caps widget dimensions at QWIDGETSIZE_MAX; a minimum size beyond it is
        // rejected with a qWarning on EVERY setMinimumSize() call and re-invalidates layout,
        // so a long recording at high zoom must clamp here (timeOrigin() then slides the
        // canvas window over the newest data instead of growing the widget).
        const double pixels = static_cast<double>(m_recorder->maxTime()) * m_pixelsPerNs;
        w = static_cast<int>(qMin(pixels, static_cast<double>(QWIDGETSIZE_MAX - 20))) + 20;
    }
    return {qBound(100, w, QWIDGETSIZE_MAX),
            qBound(WaveformLayout::RulerHeight + WaveformLayout::TraceHeight, h, QWIDGETSIZE_MAX)};
}

QSize TemporalWaveformWidget::minimumSizeHint() const
{
    return {100, WaveformLayout::RulerHeight + WaveformLayout::TraceHeight};
}

SimTime TemporalWaveformWidget::timeOrigin() const
{
    if (!m_recorder || m_pixelsPerNs <= 0) {
        return 0;
    }
    const SimTime maxTime = m_recorder->maxTime();
    const double capPixels = static_cast<double>(QWIDGETSIZE_MAX - 20);
    if (static_cast<double>(maxTime) * m_pixelsPerNs <= capPixels) {
        return 0;
    }
    return maxTime - static_cast<SimTime>(capPixels / m_pixelsPerNs);
}

// ============================================================================
// Paint
// ============================================================================

void TemporalWaveformWidget::paintEvent(QPaintEvent *event)
{
    // Resize to fit content so the parent QScrollArea shows scrollbars. sizeHint() is
    // clamped to QWIDGETSIZE_MAX, so this settles instead of re-requesting an over-limit
    // minimum (which Qt rejects with a qWarning) on every repaint.
    const QSize hint = sizeHint();
    if (hint != size()) {
        setMinimumSize(hint);
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    painter.fillRect(event->rect(), palette().base());

    if (!m_recorder || m_recorder->traceCount() == 0) {
        painter.setPen(palette().color(QPalette::Disabled, QPalette::Text));
        painter.drawText(rect(), Qt::AlignCenter, tr("No signals watched"));
        return;
    }

    const int traceWidth = width();
    if (traceWidth <= 0 || m_pixelsPerNs <= 0) {
        return;
    }

    // Convert the exposed paint region to a sim-time window so drawing costs O(viewport)
    // instead of O(recorded history). The margin absorbs ruler labels and 2px trace edges
    // anchored just outside the exposed rect, keeping partial-expose repaints seamless.
    constexpr int kExposeMargin = 120;
    const SimTime origin = timeOrigin();
    const int exposedLeft = qMax(0, event->rect().left() - kExposeMargin);
    const int exposedRight = qMin(traceWidth, event->rect().right() + kExposeMargin);
    const SimTime visibleStart = origin + static_cast<SimTime>(exposedLeft / m_pixelsPerNs);
    const SimTime visibleEnd = origin + static_cast<SimTime>(exposedRight / m_pixelsPerNs);

    drawTimeRuler(painter, traceWidth, origin, visibleStart, visibleEnd);

    for (int i = 0; i < m_recorder->traceCount(); ++i) {
        const int y = WaveformLayout::RulerHeight + i * (WaveformLayout::TraceHeight + WaveformLayout::TraceMargin);
        if (y > event->rect().bottom() || y + WaveformLayout::TraceHeight < event->rect().top()) {
            continue; // row entirely outside the exposed region
        }
        drawTrace(painter, i, y, traceWidth, WaveformLayout::TraceHeight, origin, visibleStart, visibleEnd);
    }
}

// ============================================================================
// Time ruler
// ============================================================================

void TemporalWaveformWidget::drawTimeRuler(QPainter &painter, int traceWidth,
                                           SimTime origin, SimTime visibleStart, SimTime visibleEnd)
{
    painter.fillRect(0, 0, traceWidth, WaveformLayout::RulerHeight, palette().window());
    painter.setPen(palette().color(QPalette::Text));

    if (visibleEnd <= visibleStart || m_pixelsPerNs <= 0) {
        return;
    }

    const double targetPixelSpacing = 100.0;
    double rawInterval = targetPixelSpacing / m_pixelsPerNs;

    double magnitude = 1.0;
    while (magnitude * 10 < rawInterval) { magnitude *= 10; }
    double niceInterval;
    if (rawInterval <= magnitude * 2) {
        niceInterval = magnitude * 2;
    } else if (rawInterval <= magnitude * 5) {
        niceInterval = magnitude * 5;
    } else {
        niceInterval = magnitude * 10;
    }

    const auto tickInterval = static_cast<SimTime>(qMax(niceInterval, 1.0));
    const SimTime firstTick = (visibleStart / tickInterval) * tickInterval;

    const char *unit = "ns";
    double unitDiv = 1.0;
    if (tickInterval >= 1'000'000) {
        unit = "ms";
        unitDiv = 1'000'000.0;
    } else if (tickInterval >= 1'000) {
        unit = "\xC2\xB5s";
        unitDiv = 1'000.0;
    }

    for (SimTime t = firstTick; t <= visibleEnd; t += tickInterval) {
        const int x = timeToX(t, origin);
        if (x < 0 || x > traceWidth) {
            continue;
        }
        painter.drawLine(x, WaveformLayout::RulerHeight - 6, x, WaveformLayout::RulerHeight);

        const QString label = QString::number(static_cast<double>(t) / unitDiv, 'g', 4) + " " + unit;
        painter.drawText(x + 2, WaveformLayout::RulerHeight - 8, label);
    }

    painter.drawLine(0, WaveformLayout::RulerHeight, traceWidth, WaveformLayout::RulerHeight);
}

// ============================================================================
// Signal trace
// ============================================================================

void TemporalWaveformWidget::drawTrace(QPainter &painter, int traceIndex, int y0,
                                       int traceWidth, int height,
                                       SimTime origin, SimTime visibleStart, SimTime visibleEnd)
{
    const auto &trace = m_recorder->trace(traceIndex);
    const auto &transitions = trace.transitions;

    painter.setPen(QPen(palette().color(QPalette::Mid), 1, Qt::DotLine));
    painter.drawLine(0, y0 + height, traceWidth, y0 + height);

    if (transitions.isEmpty()) {
        painter.setPen(QPen(QColor(100, 100, 100), 2));
        painter.drawLine(0, y0 + LOW_Y_OFFSET, traceWidth, y0 + LOW_Y_OFFSET);
        return;
    }

    const QColor signalColor = (trace.logic && trace.logic->propagationDelay() > 0)
                                   ? QColor(0, 180, 0)
                                   : QColor(0, 100, 220);
    painter.setPen(QPen(signalColor, 2));

    auto yForStatus = [&](Status s) -> int {
        return y0 + (s == Status::Active ? HIGH_Y_OFFSET : LOW_Y_OFFSET);
    };

    // Signal level entering the window (Inactive before the first transition), then only the
    // transitions inside the window — located by binary search, NOT a scan from t = 0, so a
    // repaint touches O(log n + visible) of a history that can hold hours of transitions.
    Status currentStatus = trace.statusAt(visibleStart);
    SimTime currentTime = visibleStart;

    const auto firstVisible = std::lower_bound(
        transitions.cbegin(), transitions.cend(), visibleStart,
        [](const QPair<SimTime, Status> &tr, SimTime t) { return tr.first < t; });

    for (auto it = firstVisible; it != transitions.cend(); ++it) {
        const auto &[tTime, tStatus] = *it;
        if (tTime > visibleEnd) {
            break;
        }

        const int xFrom = timeToX(currentTime, origin);
        const int xTo = timeToX(tTime, origin);
        const int yCurrent = yForStatus(currentStatus);

        if (xTo > xFrom) {
            painter.drawLine(xFrom, yCurrent, xTo, yCurrent);
        }

        if (tStatus != currentStatus) {
            const int yNew = yForStatus(tStatus);
            painter.drawLine(xTo, yCurrent, xTo, yNew);
        }

        currentStatus = tStatus;
        currentTime = tTime;
    }

    // Extend the last known level to the window's right edge (clamped to the canvas).
    const int xFrom = timeToX(currentTime, origin);
    const int xEnd = qMin(traceWidth, timeToX(visibleEnd, origin));
    if (xEnd > xFrom) {
        painter.drawLine(xFrom, yForStatus(currentStatus), xEnd, yForStatus(currentStatus));
    }
}

// ============================================================================
// Coordinate conversion
// ============================================================================

int TemporalWaveformWidget::timeToX(SimTime time, SimTime origin) const
{
    // SimTime is unsigned: subtract in the right order so a pre-origin time (e.g. a ruler
    // tick rounded down past the window start) maps to a negative x and gets culled, instead
    // of wrapping around to a huge positive coordinate.
    const double delta = (time >= origin) ? static_cast<double>(time - origin)
                                          : -static_cast<double>(origin - time);
    return static_cast<int>(qBound(-1e8, delta * m_pixelsPerNs, 1e8));
}

// ============================================================================
// Mouse wheel zoom
// ============================================================================

void TemporalWaveformWidget::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() > 0) {
        zoomIn();
    } else if (event->angleDelta().y() < 0) {
        zoomOut();
    }
    event->accept();
}
