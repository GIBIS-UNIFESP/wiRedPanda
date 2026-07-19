// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Phase 7d of the qtquick-rewrite plan: real coverage for QuickDolphinController, mirroring
/// Tests/System/TestBewavedDolphinGui.cpp's intent against the real controller API instead of
/// a QMainWindow/QTableView/QAction-driven BewavedDolphin -- the freshest gap Phase 7 has found
/// (Phase 6 landed with only ad-hoc, reverted diagnostic verification).
///
/// Deliberately out of scope for this pass, tracked here rather than silently dropped:
/// - testAboutDialog/testAboutQtDialog: DolphinWindow.qml has no About/AboutQt menu entries yet
///   (matches the main Quick app's own current gap, per Phase 5's own note).
/// - testSaveAsAction/testExitAction: real QuickDialogProvider-dialog-driven / QMainWindow
///   close mechanics -- testSaveAndLoadWaveform below covers the underlying DolphinFile::save()
///   round trip without the dialog layer; a dedicated dialog-driven follow-up is real future work.
/// - testMergeSplitDisabled: the Merge/Split multi-bit-bus actions don't exist in either the
///   Widgets menu (disabled placeholders) or DolphinWindow.qml (not built at all) -- nothing to
///   test on either side.
/// - testShortcutsReconciledWithMainWindow: Quick's shortcuts live in QML (Shortcut/
///   MenuItem.shortcut items), a fundamentally different mechanism from QAction::shortcut() --
///   needs its own dedicated check, not a direct port.
/// - testLongWaveformScrolls: QHeaderView/QScrollBar-specific Widgets mechanics; SignalTable.qml's
///   TableView virtualizes differently (Phase 6a's own 2048-column relayout-timing check already
///   exercised something adjacent, informally).
/// - testConstructorWithParentEnablesWindowModality: Widgets QWidget-parent/modality mechanic;
///   DolphinWindow is a genuinely separate top-level QQuickWindow with no equivalent constructor
///   shape to test.
///
/// One real, confirmed behavioral difference from the original, locked in rather than silently
/// assumed equivalent: setCellValue() (the double-click entry point) is NOT undoable in the Quick
/// port -- BewavedDolphin::on_tableView_cellDoubleClicked() pushed a SetCellsCommand, but
/// QuickDolphinController::setCellValue() just mutates + run()s directly. See
/// testSetCellValueIsNotUndoable().
class TestQuickDolphinController : public QObject
{
    Q_OBJECT

private slots:
    // --- Waveform creation ---
    void testCreateBlankWaveform();
    void testCreateWaveformInputOutputCounts();

    // --- Cell editing ---
    void testSetCellValue();
    void testRunSimulationFillsOutputs();
    void testSetLengthChangesColumns();
    void testCombinationalMode();
    void testOutputRowsIgnoreSetValue();

    // --- Export ---
    void testExportToPng();
    void testExportToPdf();

    // --- Editing actions ---
    void testClearAction();
    void testAutoCropAction();

    // --- Selection-based editing ---
    void testSetTo0WithSelection();
    void testSetTo1WithSelection();
    void testInvertWithSelection();
    void testCopyPasteWithSelection();
    void testCutWithSelection();
    void testPasteAtBoundary();
    void testSetClockWaveWithSelection();
    void testHasSelectionTracksSelectionState();

    // --- View operations ---
    void testZoomInOut();
    void testShowNumbers();
    void testShowWaveforms();
    void testFitScreenIsIdempotent();
    void testZoomScaleTrackingA26();
    void testFitScreenClampsAndGuardsA26();

    // --- Text output ---
    void testSaveToTxtOutput();
    void testSaveToTxtClampsColumnCountForManyInputPorts();

    // --- File round trip ---
    void testSaveAndLoadWaveform();
    void testCreateWaveformResolvesAbsolutePathOutsideProjectDir();

    // --- Hardening ---
    void hardeningRunAndSaveToTxtMustCheckElementsStillLive();
    void testRunAndSaveToTxtHandleDeletedTrackedElement();

    // --- Undo/redo ---
    void testUndoStackStartsCleanAfterCreateWaveform();
    void testUndoRedoSetTo0();
    void testUndoRedoSetTo1();
    void testUndoRedoInvert();
    void testUndoRedoClear();
    void testUndoRedoSetClockWave();
    void testUndoRedoCombinational();
    void testUndoRedoMultipleOperationsRestoresOriginalState();
    void testSetCellValueIsNotUndoable();
};
