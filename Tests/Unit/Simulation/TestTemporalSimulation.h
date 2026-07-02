// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestTemporalSimulation : public QObject
{
    Q_OBJECT

public:
    TestTemporalSimulation() = default;

private slots:
    // --- EventQueue unit tests ---
    void testEventQueueOrdering();
    void testEventQueueSameTime();
    void testEventQueueClear();

    // --- Per-element propagation delay ---
    void testDefaultPropagationDelays();
    void testZeroDelayElements();

    // --- Successor graph ---
    void testSuccessorGraphBuilt();

    // --- Functional vs temporal mode ---
    void testFunctionalModeDefault();
    void testTemporalModeAdvancesTime();

    // --- Temporal engine: zero delay equivalence ---
    void testZeroDelayAndGate();
    void testZeroDelayCascadedGates();
    void testZeroDelayFeedbackConverges();

    // --- Temporal engine: propagation delays ---
    void testNotGateDelayPropagation();
    void testChainedGatesCumulativeDelay();

    // --- Temporal engine: delays apply inside ICs (flattened netlist) ---
    void testICInternalDelayPropagation();
    void testWatchedICOutputPinNotOneTickStale();
    void testNestedICDelayAccumulation();
    void testNestedICDelayAccumulationDeepNesting();

    // --- Temporal engine: input change detection ---
    void testInputSwitchSchedulesEvent();

    // --- Waveform recorder ---
    /// beginTimedRun()/endTimedRun() (the dolphin sweep bracket) must be invisible to a
    /// live recording: the bracket itself suspends recording, and endTimedRun restores
    /// the live clock — not rewind it to 0 — so the preserved history keeps ascending.
    void testTimedRunPreservesRecorderTimeline();
    void testRecorderWatchAndRecord();
    void testRecorderWatchSignalDedups();
    void testRecorderDeduplication();
    void testRecorderStatusAt();
    void testRecorderIntegration();
    void testRecorderSubTickResolution();
    void testRecorderStatusAtEmptyTrace();
    void testRecorderRecordAllSkipsNullAndCollapsesSameTime();

    // --- Waveform widget ---
    void testWidgetSizeHint();
    void testWaveformWidgetZoom();
    /// Regression: a long recording at high zoom demanded a canvas wider than Qt's
    /// QWIDGETSIZE_MAX — every repaint re-called setMinimumSize() with the over-limit hint,
    /// spamming a qWarning per frame and re-invalidating layout until the UI froze. The hint
    /// must clamp at the Qt limit and timeOrigin() must slide the window to the newest data.
    void testWidgetCanvasClampedAtQtLimit();
    /// Per-channel palette: first 8 channels pairwise distinct, cycling at 8; label names
    /// share their trace's color.
    void testAnalyzerTraceColorPalette();
    /// Any wheel (no modifier needed — stimulus-editor parity) is consumed as a
    /// cursor-anchored zoom request for the hosting panel; the canvas never rescales
    /// itself and the wheel never scrolls the trace list (the scrollbars pan).
    void testCanvasWheelZoomAlwaysRequests();
    /// Nothing is known before a trace's first recorded sample (watching typically starts
    /// mid-run): that region must render blank, not as a definite LOW.
    void testCanvasPreRecordRegionBlank();
    /// A watched-but-never-advancing timeline (maxTime == 0) explains itself persistently
    /// on the canvas: functional mode vs no-edges-yet; cleared once real data lands.
    void testCanvasDegenerateStateHint();

    // --- Temporal example circuits (Examples/temporal_*.panda) ---
    void testExampleRingOscillator();
    void testExampleStaticHazard();
    void testExampleGateDelayChain();
};
