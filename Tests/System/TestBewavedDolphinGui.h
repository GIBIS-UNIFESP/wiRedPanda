// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>

/// GUI-level tests for the BeWavedDolphin waveform editor.
///
/// Exercises waveform creation, cell editing, simulation run,
/// length control, clipboard, export, and view operations.
class TestBewavedDolphinGui : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    // --- Waveform creation ---

    void testCreateBlankWaveform();
    void testCreateWaveformInputOutputCounts();

    // --- Cell editing ---

    void testSetCellValue();
    void testRunSimulationFillsOutputs();
    void testSetLengthChangesColumns();
    void testCombinationalMode();

    // --- Temporal (propagation-delay) mode ---

    /// In temporal mode a gate's output transition lags its cause by the gate delay, rendered as
    /// column-lag; functional mode shows the transition in the same column (zero delay).
    void testTemporalModeShowsPropagationLag();
    /// Cumulative delay: a NOT→AND chain lags more than a single NOT, proving delays compose.
    void testTemporalModeCumulativeChainLag();

    /// A NON-temporal sweep stays genuinely functional (same-column transitions) even when the
    /// main window left the shared Simulation in temporal mode — run() must not inherit the
    /// live tick window — and the live window is restored afterwards.
    void testNonTemporalSweepIgnoresLiveTemporalMode();

    /// BeWavedDolphin's sweep drives the same Simulation (and WaveformRecorder) its own
    /// Live Analyzer page may be watching — a sweep must neither write synthetic
    /// test-vector transitions into a live trace NOR destroy it: the recorded history and
    /// the live time-base survive the sweep intact and keep appending monotonically after
    /// (previously every run() — including the one at tool-open — wiped the recording and
    /// rewound the clock to 0, silently collapsing the analyzer to flat seed lines).
    void testRunPreservesLiveAnalyzerTimeline();

    /// The sweep's pre-run reset must reach sequential elements nested inside ICs (the flat
    /// netlist simulates them directly) — a flip-flop's live-run state must not leak into a
    /// sweep that promises power-on reproducibility.
    void testRunResetsICInternalSequentialState();

    // --- Live Analyzer page ---

    /// BD hosts two pages (Stimulus Editor | Live Analyzer); switching to the analyzer gates
    /// the grid-document actions off and restores their exact prior state on return — the
    /// permanently-disabled Merge/Split must not be resurrected by the round trip.
    void testLiveAnalyzerTabGatesGridActions();
    /// Watch All registers every non-VCC/GND output port with the scene's recorder and
    /// starts recording; Clear stops and empties it.
    void testLiveAnalyzerWatchAllAndClear();
    /// watchICInternals() reveals the analyzer page and registers path-prefixed traces for
    /// the IC's internal primitives.
    void testLiveAnalyzerWatchICInternals();
    /// The frozen label column follows the vertical scrollbar (rows stay aligned with
    /// traces) and its preferred height stays capped for long watch lists.
    void testLiveAnalyzerVerticalScrollSyncsLabels();
    /// "Latest Edge" zooms to a ns-resolving scale with the newest recorded transition in
    /// view (gate delays are sub-pixel at any live-following zoom); empty recorder = no-op.
    void testLiveAnalyzerZoomToLatestEdge();
    /// Zoom must anchor the viewed time: +/- keep the sim-time at the viewport center fixed
    /// (the scroll area preserves the raw PIXEL value across the canvas rescale, which slid
    /// the view exponentially toward t = 0 on every zoom-in — traces then read all-low).
    void testLiveAnalyzerZoomKeepsAnchor();
    /// The wheel (no modifier — stimulus-editor parity) zooms around the sim-time under
    /// the cursor.
    void testLiveAnalyzerWheelZoomsAtCursor();
    /// While the view tracks the newest data (sticky tail), the +/- buttons zoom around
    /// the tracked tail — scope-style — instead of centering (and dropping) it.
    void testLiveAnalyzerButtonZoomFollowsTail();
    /// Fit spans the whole recording across the VIEWPORT — not the canvas's own width,
    /// which no longer fits anything once the canvas has grown past the viewport.
    void testLiveAnalyzerFitSpansViewport();
    /// exportImage() writes a loadable, non-blank image of the panel exactly as displayed
    /// (the autonomous-debugging capability: tests export snapshots and the images are
    /// inspected directly).
    void testLiveAnalyzerExportImage();
    /// End-to-end through the REAL pipeline (real Simulation ticks at temporal 1x, real
    /// InputSwitch toggles): a 4-NOT chain's 5/10/15/20 ns staircase is recorded with
    /// sub-tick timestamps AND is on-screen, pixels apart, after tail-pinned zoom to
    /// saturation. Exports PNG snapshots at each phase for visual inspection.
    void testGateDelayStaircaseReachableByZoom();

    // --- File I/O ---

    void testExportToPng();

    // --- Editing actions ---

    void testClearAction();
    void testAutoCropAction();

    // --- Selection-based editing ---

    void testSetTo0WithSelection();
    void testSetTo1WithSelection();
    void testInvertWithSelection();
    void testCopyPasteWithSelection();
    void testSetClockWaveWithSelection();

    // --- View operations ---

    void testZoomInOut();
    void testShowNumbers();
    void testShowWaveforms();
    void testFitScreen();
    void testLongWaveformScrolls();

    // --- Double-click cell ---

    void testDoubleClickToggle();

    // --- Text output ---

    void testSaveToTxtOutput();

    // --- Missing coverage ---

    void testCutWithSelection();
    void testSaveAndLoadWaveform();
    void testExportToPdf();
    void testOutputRowsIgnoreSetValue();
    void testPasteAtBoundary();
    void testAboutDialog();
    void testAboutQtDialog();

    // --- Additional operations ---

    void testSaveAsAction();
    void testExitAction();
    void testMergeSplitDisabled();
    void testShortcutsReconciledWithMainWindow();

    // Regression: C9 — actionSetClockWave gated on a non-empty selection
    void testSetClockWaveDisabledWithoutSelectionC9();

    // Regression: A26 — Zoom In/Out scales column width only (m_zoomLevel); Fit Screen
    // scales all metrics uniformly (m_fitScale) and must not crash on degenerate geometry
    // or produce extreme scale factors.
    void testZoomScaleTrackingA26();
    void testFitScreenClampsAndGuardsA26();

    // Regression: a linked waveform file living outside the project directory must still
    // resolve by its absolute path, not just by basename inside the project's own folder.
    void testCreateWaveformResolvesAbsolutePathOutsideProjectDir();

    // Regression: run()/saveToTxt() dereferenced m_inputs/m_outputs via sweep() with no
    // check that those elements were still in the live scene — deleting one (main canvas,
    // or an MCP client neither path's modality blocks) left them dangling. Source-level
    // check, same shape as TestDanglingPointer.cpp's hardening_* tests: a plain behavioral
    // repro isn't reliable here since reading recently-freed memory doesn't crash on every
    // allocator/run.
    void hardeningRunAndSaveToTxtMustCheckElementsStillLive();
    // Behavioral companion: deleting a tracked element and then calling run()/saveToTxt()
    // must not crash and must fail cleanly, not silently export garbage.
    void testRunAndSaveToTxtHandleDeletedTrackedElement();

    // Regression: saveToTxt()'s column count had no cap, unlike its sibling
    // on_actionCombinational_triggered — 12 input ports (2^12 = 4096 combinations) must
    // clamp to SignalModel::kMaxColumns (2048), the same bound the sibling already uses.
    void testSaveToTxtClampsColumnCountForManyInputPorts();

    // --- Undo/Redo (#19) ---

    void testUndoStackStartsCleanAfterCreateWaveform();
    void testUndoRedoSetTo0();
    void testUndoRedoSetTo1();
    void testUndoRedoInvert();
    void testUndoRedoClear();
    void testUndoRedoSetClockWave();
    void testUndoRedoCombinational();
    void testUndoRedoDoubleClickToggle();
    void testUndoRedoMultipleOperationsRestoresOriginalState();

private:
    QTemporaryDir m_tempDir;
};
