// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/TemporalWaveformWidget.h"

#include <QPainter>
#include <QWheelEvent>

#include "App/Simulation/WaveformRecorder.h"

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
    const int availableWidth = width() - LABEL_WIDTH;
    if (availableWidth <= 0) {
        return;
    }
    setPixelsPerNs(static_cast<double>(availableWidth) / static_cast<double>(maxTime));
}

QSize TemporalWaveformWidget::sizeHint() const
{
    const int traceCount = m_recorder ? m_recorder->traceCount() : 0;
    const int h = RULER_HEIGHT + traceCount * (TRACE_HEIGHT + TRACE_MARGIN);

    int w = LABEL_WIDTH + 400; // default width
    if (m_recorder && m_recorder->maxTime() > 0) {
        w = LABEL_WIDTH + static_cast<int>(m_recorder->maxTime() * m_pixelsPerNs) + 20;
    }
    return {qMax(w, LABEL_WIDTH + 100), qMax(h, RULER_HEIGHT + TRACE_HEIGHT)};
}

QSize TemporalWaveformWidget::minimumSizeHint() const
{
    return {LABEL_WIDTH + 100, RULER_HEIGHT + TRACE_HEIGHT};
}

// ============================================================================
// Paint
// ============================================================================

void TemporalWaveformWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // Background
    painter.fillRect(rect(), palette().base());

    if (!m_recorder || m_recorder->traceCount() == 0) {
        painter.setPen(palette().color(QPalette::Disabled, QPalette::Text));
        painter.drawText(rect(), Qt::AlignCenter, tr("No signals watched"));
        return;
    }

    const int x0 = LABEL_WIDTH;
    const int traceWidth = width() - LABEL_WIDTH;
    const SimTime maxTime = m_recorder->maxTime();
    const SimTime visibleStart = 0; // TODO: scroll offset
    const SimTime visibleEnd = (traceWidth > 0 && m_pixelsPerNs > 0)
                                   ? static_cast<SimTime>(traceWidth / m_pixelsPerNs)
                                   : maxTime;

    // Time ruler
    drawTimeRuler(painter, x0, traceWidth, visibleStart, visibleEnd);

    // Signal traces
    for (int i = 0; i < m_recorder->traceCount(); ++i) {
        const int y = RULER_HEIGHT + i * (TRACE_HEIGHT + TRACE_MARGIN);

        // Label background
        painter.fillRect(0, y, LABEL_WIDTH - 2, TRACE_HEIGHT, palette().alternateBase());

        // Label text
        painter.setPen(palette().color(QPalette::Text));
        painter.drawText(QRect(4, y, LABEL_WIDTH - 8, TRACE_HEIGHT),
                         Qt::AlignVCenter | Qt::AlignRight,
                         m_recorder->trace(i).name);

        // Trace
        drawTrace(painter, i, x0, y, traceWidth, TRACE_HEIGHT, visibleStart, visibleEnd);
    }
}

// ============================================================================
// Time ruler
// ============================================================================

void TemporalWaveformWidget::drawTimeRuler(QPainter &painter, int x0, int traceWidth,
                                           SimTime visibleStart, SimTime visibleEnd)
{
    painter.fillRect(x0, 0, traceWidth, RULER_HEIGHT, palette().window());
    painter.setPen(palette().color(QPalette::Text));

    if (visibleEnd <= visibleStart || m_pixelsPerNs <= 0) {
        return;
    }

    // Choose a tick interval that gives ~80-150 pixel spacing
    const double targetPixelSpacing = 100.0;
    double rawInterval = targetPixelSpacing / m_pixelsPerNs;

    // Round to a "nice" number: 1, 2, 5, 10, 20, 50, 100, ...
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

    // Choose unit label
    const char *unit = "ns";
    double unitDiv = 1.0;
    if (tickInterval >= 1'000'000'000) {
        unit = "ms";
        unitDiv = 1'000'000.0;
    } else if (tickInterval >= 1'000'000) {
        unit = "µs";
        unitDiv = 1'000.0;
    }

    for (SimTime t = firstTick; t <= visibleEnd; t += tickInterval) {
        const int x = timeToX(t, visibleStart, x0);
        if (x < x0 || x > x0 + traceWidth) {
            continue;
        }
        painter.drawLine(x, RULER_HEIGHT - 6, x, RULER_HEIGHT);

        const QString label = QString::number(static_cast<double>(t) / unitDiv, 'g', 4) + " " + unit;
        painter.drawText(x + 2, RULER_HEIGHT - 8, label);
    }

    // Bottom line
    painter.drawLine(x0, RULER_HEIGHT, x0 + traceWidth, RULER_HEIGHT);
}

// ============================================================================
// Signal trace
// ============================================================================

void TemporalWaveformWidget::drawTrace(QPainter &painter, int traceIndex, int x0, int y0,
                                       int traceWidth, int height, SimTime visibleStart, SimTime visibleEnd)
{
    const auto &trace = m_recorder->trace(traceIndex);
    const auto &transitions = trace.transitions;

    // Trace background with subtle grid line
    painter.setPen(QPen(palette().color(QPalette::Mid), 1, Qt::DotLine));
    painter.drawLine(x0, y0 + height, x0 + traceWidth, y0 + height);

    if (transitions.isEmpty()) {
        // No data — draw a flat inactive line
        painter.setPen(QPen(QColor(100, 100, 100), 2));
        painter.drawLine(x0, y0 + LOW_Y_OFFSET, x0 + traceWidth, y0 + LOW_Y_OFFSET);
        return;
    }

    // Choose color: green for output elements, blue for inputs
    const QColor signalColor = (trace.logic && trace.logic->propagationDelay() > 0)
                                   ? QColor(0, 180, 0)   // green (logic gates / outputs)
                                   : QColor(0, 100, 220); // blue (inputs / sources)
    painter.setPen(QPen(signalColor, 2));

    // Draw each segment between transitions
    auto yForStatus = [&](Status s) -> int {
        return y0 + (s == Status::Active ? HIGH_Y_OFFSET : LOW_Y_OFFSET);
    };

    // Initial segment: from visibleStart to first transition
    Status currentStatus = Status::Inactive;
    SimTime currentTime = visibleStart;

    // Find the status at visibleStart
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

        // Horizontal line from current to transition
        const int xFrom = timeToX(qMax(currentTime, visibleStart), visibleStart, x0);
        const int xTo = timeToX(tTime, visibleStart, x0);
        const int yCurrent = yForStatus(currentStatus);

        if (xTo > xFrom) {
            painter.drawLine(xFrom, yCurrent, xTo, yCurrent);
        }

        // Vertical edge
        if (tStatus != currentStatus) {
            const int yNew = yForStatus(tStatus);
            painter.drawLine(xTo, yCurrent, xTo, yNew);
        }

        currentStatus = tStatus;
        currentTime = tTime;
    }

    // Final segment: from last transition to visibleEnd
    const int xFrom = timeToX(qMax(currentTime, visibleStart), visibleStart, x0);
    const int xEnd = x0 + traceWidth;
    if (xEnd > xFrom) {
        painter.drawLine(xFrom, yForStatus(currentStatus), xEnd, yForStatus(currentStatus));
    }
}

// ============================================================================
// Coordinate conversion
// ============================================================================

int TemporalWaveformWidget::timeToX(SimTime time, SimTime visibleStart, int x0) const
{
    return x0 + static_cast<int>((time - visibleStart) * m_pixelsPerNs);
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
