// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Timing diagram widget for the temporal simulation waveform viewer.
 */

#pragma once

#include <QWidget>

class WaveformRecorder;

/**
 * \class TemporalWaveformWidget
 * \brief Displays timing diagrams from recorded temporal simulation transitions.
 *
 * \details Draws one horizontal trace per watched signal with vertical edges
 * at transition points.  Supports zoom (pixels per nanosecond) and scroll.
 * The widget reads data from a WaveformRecorder and repaints on demand.
 */
class TemporalWaveformWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TemporalWaveformWidget(QWidget *parent = nullptr);

    /// Sets the recorder to read trace data from.
    void setRecorder(const WaveformRecorder *recorder) { m_recorder = recorder; }

    /// Returns the current zoom level (pixels per nanosecond).
    double pixelsPerNs() const { return m_pixelsPerNs; }

    /// Sets the zoom level.
    void setPixelsPerNs(double ppn);

    /// Zooms in by a factor of 2.
    void zoomIn();

    /// Zooms out by a factor of 2.
    void zoomOut();

    /// Resets zoom to fit all recorded data in the current width.
    void zoomFit();

    /// Returns the desired size based on trace count and time span.
    QSize sizeHint() const override;

    /// Returns the minimum size (enough for the label column).
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    // --- Drawing helpers ---
    void drawTimeRuler(QPainter &painter, int x0, int width, SimTime visibleStart, SimTime visibleEnd);
    void drawTrace(QPainter &painter, int traceIndex, int x0, int y0, int width, int height,
                   SimTime visibleStart, SimTime visibleEnd);

    // --- Coordinate conversion ---
    int timeToX(SimTime time, SimTime visibleStart, int x0) const;

    // --- Constants ---
    static constexpr int LABEL_WIDTH = 120;  ///< Width of the signal name column.
    static constexpr int TRACE_HEIGHT = 30;  ///< Height of each signal trace.
    static constexpr int TRACE_MARGIN = 4;   ///< Vertical margin between traces.
    static constexpr int RULER_HEIGHT = 24;  ///< Height of the time ruler.
    static constexpr int HIGH_Y_OFFSET = 4;  ///< Inset from top of trace for HIGH level.
    static constexpr int LOW_Y_OFFSET = 26;  ///< Inset from top of trace for LOW level.

    // --- Members ---
    const WaveformRecorder *m_recorder = nullptr;
    double m_pixelsPerNs = 0.001; ///< Default: 1 pixel per 1000 ns (1 µs).
};
