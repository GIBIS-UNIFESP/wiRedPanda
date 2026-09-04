// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Tests QuickDolphinController, mirroring Tests/System/TestBewavedDolphinGui.cpp's intent
/// against the real controller API instead of a QMainWindow/QTableView/QAction-driven
/// BewavedDolphin.
///
/// setCellValue() (the double-click entry point) is NOT undoable: it mutates and calls run()
/// directly rather than pushing a SetCellsCommand. See testSetCellValueIsNotUndoable().
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
    void testSelectCellPlainClickReplacesSelection();
    void testSelectCellCtrlClickTogglesSelection();
    void testSelectCellShiftClickSelectsRange();
    void testSelectCellShiftClickCanShrinkAfterGrowing();
    void testSelectCellShiftClickUnionsWithPriorCtrlSelection();
    void testSelectionChangedHighlightsCorrespondingInputElement();

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
