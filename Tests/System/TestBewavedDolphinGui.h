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

    /// BeWavedDolphin shares its scene's Simulation (and WaveformRecorder) with the main
    /// window's Temporal Waveform dock — merely opening/running a dolphin sweep must not
    /// write synthetic test-vector transitions into a trace the dock is live-recording.
    void testRunDoesNotPolluteLiveWaveformRecorder();

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
    // Regression: BewavedDolphin's Qt::WindowModal setting was a no-op because the window
    // was never given a real Qt parent (host is a non-QWidget interface) — the comment
    // claiming "the user cannot interact with the main circuit" was false as implemented.
    void testConstructorWithParentEnablesWindowModality();

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
