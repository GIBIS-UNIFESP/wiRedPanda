// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Live Analyzer: real-time timing-diagram page hosted by BeWavedDolphin.
 *
 * \details Passive probe of the running interactive simulation — the counterpart to the
 * stimulus-editor grid. Watches signals (including IC internals) through the Simulation's
 * WaveformRecorder and renders their timestamped transitions as scrollable traces: in
 * functional mode a live view of current levels, in temporal mode a nanosecond-resolution
 * timing diagram.
 */

#pragma once

#include <QPointer>
#include <QTimer>
#include <QWidget>

#include "App/Simulation/SimTime.h"

class IC;
class QScrollArea;
class Scene;
class WaveformRecorder;

/// Shared geometry for the analyzer's frozen-pane grid (time ruler row on top, label column
/// on the left, trace canvas in the scrollable cell). All four widgets MUST agree on these
/// values or rows/ticks misalign, so they live here as the single source of truth.
namespace AnalyzerLayout {
inline constexpr int TraceHeight = 30; ///< Pixel height of one signal row.
inline constexpr int TraceMargin = 4;  ///< Vertical gap between consecutive rows.
inline constexpr int RulerHeight = 24; ///< Pixel height of the time-ruler row.
inline constexpr int LabelWidth = 120; ///< Pixel width of the signal-name column.
} // namespace AnalyzerLayout

/**
 * \class AnalyzerCanvas
 * \brief Scrollable canvas drawing one horizontal trace per watched signal.
 *
 * \details Lives inside the panel's QScrollArea. The canvas width follows the recorded
 * timeline at the current zoom, clamped to Qt's QWIDGETSIZE_MAX; past the clamp,
 * timeOrigin() slides a window over the newest data so live following keeps working at any
 * zoom. Painting is restricted to the exposed region, so a repaint costs O(viewport)
 * regardless of how much history has accumulated.
 */
class AnalyzerCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit AnalyzerCanvas(QWidget *parent = nullptr);

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
    /// out). Public so the ruler and tests share the sliding-window mapping.
    SimTime timeOrigin() const;

signals:
    /// Emitted whenever the zoom level changes, so the ruler can repaint in sync.
    void zoomChanged();

protected:
    void paintEvent(QPaintEvent *event) override;

    /// Ctrl+wheel zooms; a plain wheel is ignored so the surrounding scroll area scrolls
    /// (the standard waveform-tool convention once vertical scrolling exists).
    void wheelEvent(QWheelEvent *event) override;

private:
    /// \a visibleStart / \a visibleEnd bound the exposed paint region (in sim time) so a
    /// repaint costs O(viewport), not O(recorded history); \a origin is timeOrigin().
    void drawTrace(QPainter &painter, int traceIndex, int y0, int width, int height,
                   SimTime origin, SimTime visibleStart, SimTime visibleEnd);

    int timeToX(SimTime time, SimTime origin) const;

    static constexpr int HIGH_Y_OFFSET = 4;
    static constexpr int LOW_Y_OFFSET = 26;

    const WaveformRecorder *m_recorder = nullptr;
    double m_pixelsPerNs = 0.001; ///< Default: 1 pixel per 1000 ns (1 µs).
};

/**
 * \class AnalyzerTimeRuler
 * \brief Fixed ruler row above the scroll area, horizontally synced to the canvas.
 *
 * \details Frozen-pane header: it does not scroll vertically with the traces, and follows
 * horizontal scrolling via setHorizontalOffset() (connected to the scroll bar) instead of
 * being part of the canvas — so it stays visible however the view is scrolled.
 */
class AnalyzerTimeRuler : public QWidget
{
    Q_OBJECT

public:
    explicit AnalyzerTimeRuler(const AnalyzerCanvas *canvas, QWidget *parent = nullptr);

    /// Sets the horizontal scroll offset (the canvas x-coordinate at the ruler's left edge).
    void setHorizontalOffset(int offset);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    const AnalyzerCanvas *m_canvas;
    int m_horizontalOffset = 0;
};

/**
 * \class AnalyzerLabelColumn
 * \brief Fixed-width signal-name column, vertically synced to the canvas.
 *
 * \details Frozen-pane column: follows vertical scrolling via setVerticalOffset()
 * (connected to the scroll bar) so its rows stay aligned with the canvas traces.
 */
class AnalyzerLabelColumn : public QWidget
{
    Q_OBJECT

public:
    explicit AnalyzerLabelColumn(QWidget *parent = nullptr);

    void setRecorder(const WaveformRecorder *recorder) { m_recorder = recorder; }

    /// Sets the vertical scroll offset (the canvas y-coordinate at the column's top edge).
    void setVerticalOffset(int offset);

    /// Returns the current vertical scroll offset.
    int verticalOffset() const { return m_verticalOffset; }

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    const WaveformRecorder *m_recorder = nullptr;
    int m_verticalOffset = 0;
};

/**
 * \class LiveAnalyzerPanel
 * \brief Self-contained Live Analyzer page: watch controls + frozen-pane trace view.
 *
 * \details Owns everything the analyzer needs for one circuit scene: the Watch All / Clear /
 * zoom toolbar, the ruler/label/canvas grid, the periodic refresh (active only while
 * visible), the sticky-tail follow of newest data, and the watch actions themselves.
 * BeWavedDolphin hosts it as a tab; it has no dependency on the host beyond a Scene.
 *
 * The scene is held through a QPointer and its destruction detaches the recorder, so a
 * panel outliving its circuit tab degrades safely instead of dangling.
 */
class LiveAnalyzerPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LiveAnalyzerPanel(Scene *scene, QWidget *parent = nullptr);

    /// Watches every output port of every element in the scene (skipping VCC/GND).
    void watchAllSignals();

    /// Removes all watched signals and recorded data.
    void clearWatchedSignals();

    /// Recursively watches every internal primitive of \a ic with path-prefixed names
    /// ("<IC>/<sub-IC>/<element>"). Augments the current watch set rather than clearing it,
    /// so several ICs can be watched side by side.
    void watchICInternals(IC *ic);

    /// The trace canvas (exposed for the host's tests and zoom bookkeeping).
    AnalyzerCanvas *canvas() const { return m_canvas; }

    /// The signal-name column (exposed for tests).
    AnalyzerLabelColumn *labelColumn() const { return m_labels; }

    /// The scroll area holding the canvas (exposed for tests).
    QScrollArea *scrollArea() const { return m_scrollArea; }

signals:
    /// User-facing hints (e.g. "switch to Temporal mode"); the host routes this to its
    /// status bar.
    void statusMessage(const QString &message, int timeoutMs);

protected:
    /// The periodic refresh only runs while the panel is visible.
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    /// Re-derives the zoom so ~5 seconds of wall-clock time fits the visible width
    /// (at 1000 ticks/sec, 5 s = 5000 ticks of sim time). Called when the simulation
    /// speed changes and when watching starts.
    void adaptZoomToTickWindow(SimTime nsPerTick);

    /// Points the user at Temporal mode when recording cannot advance a timeline.
    void hintIfFunctionalMode();

    WaveformRecorder *recorder() const;

    QPointer<Scene> m_scene;
    AnalyzerCanvas *m_canvas = nullptr;
    AnalyzerTimeRuler *m_ruler = nullptr;
    AnalyzerLabelColumn *m_labels = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QTimer m_refreshTimer;
    bool m_followTail = true; ///< View pinned to the newest data (sticky tail).
};
