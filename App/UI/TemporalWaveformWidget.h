// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Timing diagram widgets for the temporal simulation waveform viewer.
 */

#pragma once

#include <QWidget>

#include "App/Simulation/SimTime.h"

class WaveformRecorder;

/// Shared row geometry for the two waveform columns. The label column
/// (WaveformLabelWidget) and the trace column (TemporalWaveformWidget) MUST use identical
/// values or their rows misalign, so they live here as the single source of truth rather than
/// being duplicated per class.
namespace WaveformLayout {
inline constexpr int TraceHeight = 30; ///< Pixel height of one signal row.
inline constexpr int TraceMargin = 4;  ///< Vertical gap between consecutive rows.
inline constexpr int RulerHeight = 24; ///< Pixel height of the time-ruler header.
} // namespace WaveformLayout

/**
 * \class WaveformLabelWidget
 * \brief Fixed-width widget displaying signal names for the waveform viewer.
 */
class WaveformLabelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WaveformLabelWidget(QWidget *parent = nullptr);
    void setRecorder(const WaveformRecorder *recorder) { m_recorder = recorder; }
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    static constexpr int LABEL_WIDTH = 120;

    const WaveformRecorder *m_recorder = nullptr;
};

/**
 * \class TemporalWaveformWidget
 * \brief Displays timing diagrams from recorded temporal simulation transitions.
 *
 * \details Draws one horizontal trace per watched signal with vertical edges
 * at transition points.  Supports zoom (pixels per nanosecond).  This widget
 * is placed inside a QScrollArea for horizontal scrolling; signal labels are
 * drawn by a separate WaveformLabelWidget positioned outside the scroll area.
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

    /// Returns the desired size based on trace count and time span, clamped to Qt's
    /// QWIDGETSIZE_MAX widget-dimension limit (a larger minimum size is rejected with a
    /// qWarning on every repaint and the canvas cannot grow past it anyway).
    QSize sizeHint() const override;

    /// Returns the minimum size.
    QSize minimumSizeHint() const override;

    /// Returns the simulation time mapped to canvas x = 0. Zero while the whole recording
    /// fits within QWIDGETSIZE_MAX at the current zoom; once it no longer fits, the canvas
    /// freezes at the maximum width and this origin advances so the window always covers the
    /// NEWEST data at the user's chosen zoom (older data becomes reachable again by zooming
    /// out). Public so tests can assert the sliding-window mapping.
    SimTime timeOrigin() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    // --- Drawing helpers ---
    /// \a visibleStart / \a visibleEnd bound the exposed paint region (in sim time) so a
    /// repaint costs O(viewport), not O(recorded history); \a origin is timeOrigin().
    void drawTimeRuler(QPainter &painter, int width, SimTime origin, SimTime visibleStart, SimTime visibleEnd);
    void drawTrace(QPainter &painter, int traceIndex, int y0, int width, int height,
                   SimTime origin, SimTime visibleStart, SimTime visibleEnd);

    // --- Coordinate conversion ---
    int timeToX(SimTime time, SimTime origin) const;

    // --- Constants --- (shared row geometry lives in namespace WaveformLayout, above)
    static constexpr int HIGH_Y_OFFSET = 4;
    static constexpr int LOW_Y_OFFSET = 26;

    // --- Members ---
    const WaveformRecorder *m_recorder = nullptr;
    double m_pixelsPerNs = 0.001; ///< Default: 1 pixel per 1000 ns (1 µs).
};
