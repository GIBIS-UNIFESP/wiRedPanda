// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/TemporalWaveformWidget.h"

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
    const int h = RULER_HEIGHT + traceCount * (TRACE_HEIGHT + TRACE_MARGIN);
    return {LABEL_WIDTH, qMax(h, RULER_HEIGHT + TRACE_HEIGHT)};
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
    painter.fillRect(0, 0, LABEL_WIDTH, RULER_HEIGHT, palette().window());

    for (int i = 0; i < m_recorder->traceCount(); ++i) {
        const int y = RULER_HEIGHT + i * (TRACE_HEIGHT + TRACE_MARGIN);

        painter.fillRect(0, y, LABEL_WIDTH - 2, TRACE_HEIGHT, palette().alternateBase());
        painter.setPen(palette().color(QPalette::Text));
        painter.drawText(QRect(4, y, LABEL_WIDTH - 8, TRACE_HEIGHT),
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
    setMinimumHeight(RULER_HEIGHT + TRACE_HEIGHT);
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
    const int h = RULER_HEIGHT + traceCount * (TRACE_HEIGHT + TRACE_MARGIN);

    int w = 400;
    if (m_recorder && m_recorder->maxTime() > 0) {
        const double pixels = m_recorder->maxTime() * m_pixelsPerNs;
        w = static_cast<int>(qMin(pixels, static_cast<double>(INT_MAX - 20))) + 20;
    }
    return {qMax(w, 100), qMax(h, RULER_HEIGHT + TRACE_HEIGHT)};
}

QSize TemporalWaveformWidget::minimumSizeHint() const
{
    return {100, RULER_HEIGHT + TRACE_HEIGHT};
}

// ============================================================================
// Paint
// ============================================================================

void TemporalWaveformWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    // Resize to fit content so the parent QScrollArea shows scrollbars.
    const QSize hint = sizeHint();
    if (hint != size()) {
        setMinimumSize(hint);
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    painter.fillRect(rect(), palette().base());

    if (!m_recorder || m_recorder->traceCount() == 0) {
        painter.setPen(palette().color(QPalette::Disabled, QPalette::Text));
        painter.drawText(rect(), Qt::AlignCenter, tr("No signals watched"));
        return;
    }

    const int traceWidth = width();
    const SimTime visibleStart = 0;
    const SimTime visibleEnd = (traceWidth > 0 && m_pixelsPerNs > 0)
                                   ? static_cast<SimTime>(traceWidth / m_pixelsPerNs)
                                   : 0;

    drawTimeRuler(painter, traceWidth, visibleStart, visibleEnd);

    for (int i = 0; i < m_recorder->traceCount(); ++i) {
        const int y = RULER_HEIGHT + i * (TRACE_HEIGHT + TRACE_MARGIN);
        drawTrace(painter, i, y, traceWidth, TRACE_HEIGHT, visibleStart, visibleEnd);
    }
}

// ============================================================================
// Time ruler
// ============================================================================

void TemporalWaveformWidget::drawTimeRuler(QPainter &painter, int traceWidth,
                                           SimTime visibleStart, SimTime visibleEnd)
{
    painter.fillRect(0, 0, traceWidth, RULER_HEIGHT, palette().window());
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
        const int x = timeToX(t, visibleStart);
        if (x < 0 || x > traceWidth) {
            continue;
        }
        painter.drawLine(x, RULER_HEIGHT - 6, x, RULER_HEIGHT);

        const QString label = QString::number(static_cast<double>(t) / unitDiv, 'g', 4) + " " + unit;
        painter.drawText(x + 2, RULER_HEIGHT - 8, label);
    }

    painter.drawLine(0, RULER_HEIGHT, traceWidth, RULER_HEIGHT);
}

// ============================================================================
// Signal trace
// ============================================================================

void TemporalWaveformWidget::drawTrace(QPainter &painter, int traceIndex, int y0,
                                       int traceWidth, int height,
                                       SimTime visibleStart, SimTime visibleEnd)
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

    Status currentStatus = Status::Inactive;
    SimTime currentTime = visibleStart;

    if (visibleStart >= transitions.first().first) {
        currentStatus = trace.statusAt(visibleStart);
    }

    for (const auto &[tTime, tStatus] : transitions) {
        if (tTime > visibleEnd) {
            break;
        }
        if (tTime < visibleStart) {
            currentStatus = tStatus;
            currentTime = tTime;
            continue;
        }

        const int xFrom = timeToX(qMax(currentTime, visibleStart), visibleStart);
        const int xTo = timeToX(tTime, visibleStart);
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

    const int xFrom = timeToX(qMax(currentTime, visibleStart), visibleStart);
    const int xEnd = traceWidth;
    if (xEnd > xFrom) {
        painter.drawLine(xFrom, yForStatus(currentStatus), xEnd, yForStatus(currentStatus));
    }
}

// ============================================================================
// Coordinate conversion
// ============================================================================

int TemporalWaveformWidget::timeToX(SimTime time, SimTime visibleStart) const
{
    const double pixels = (time - visibleStart) * m_pixelsPerNs;
    return static_cast<int>(qBound(-1e8, pixels, 1e8));
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
