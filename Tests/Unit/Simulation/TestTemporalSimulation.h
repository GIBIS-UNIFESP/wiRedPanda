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
    /// Adjacent ruler ticks always get distinct labels — 'g'-rounded labels collapsed
    /// every ns-scale tick into the same "4e+06 ns" at delay-resolving zoom.
    void testRulerLabelsDistinctAtNsScale();

    // --- Step debugger (Simulation::stepPropagation()/stepBack()) ---
    /// Functional mode, switch→NOT→NOT→LED: each step yields exactly one changed element,
    /// walking the toggled input's fallout through the netlist in causal order; once the
    /// propagation is exhausted the next step reports Settled.
    void testStepWalksChangeInCausalOrder();
    /// Temporal mode: each step lands on the changed gate's event timestamp, so the sim
    /// clock hops by the per-element propagation delays (5/10/15/20 ns staircase).
    void testStepTemporalAdvancesSimTime();
    /// Engine-preservation contract: stepping a circuit to Settled must end in exactly the
    /// state a plain update() run reaches on an identical circuit.
    void testStepEquivalentToUpdate();
    /// Two D flip-flops on one manual clock: their synchronous commit publishes as ONE step
    /// (both elements together), and the downstream one captures the pre-edge value —
    /// stepping must not break non-blocking semantics.
    void testStepSequentialCommitIsOneStep();
    /// N steps forward + N steps back restore outputs, sim time, and history emptiness
    /// exactly; stepping forward again reproduces the identical sequence.
    void testStepBackRestoresState();
    /// Temporal + recording: rewinding a step also rewinds the recorded timeline to the
    /// step's mark (transitions recorded by the rolled-back step disappear).
    void testStepBackTruncatesRecorder();
    /// start(), beginTimedRun()/endTimedRun(), setTimePerTick(), and restart() are all
    /// "continue" boundaries: they clear the rewind history and leave no open
    /// deferred-commit bracket behind (a follow-up update() behaves normally).
    void testStepHistoryLifecycle();
    /// Stepping while the timer runs is refused (NotReady) and mutates nothing.
    void testStepWhileRunningNotReady();
    /// Stepping a settled circuit reports Settled without advancing the sim clock —
    /// repeated clicks must not creep time forward.
    void testStepSettledNoTimeCreep();
    /// Temporal mode with a tick window smaller than the gate delay: one step fast-forwards
    /// whole tick windows to the pending event instead of needing one click per empty tick.
    void testStepFastForwardsToPendingEvent();

    // --- Temporal example circuits (Examples/temporal_*.panda) ---
    void testExampleRingOscillator();
    void testExampleStaticHazard();
    void testExampleGateDelayChain();
};
