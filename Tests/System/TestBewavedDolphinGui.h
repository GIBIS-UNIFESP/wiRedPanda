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

    // Regression: C9 — actionSetClockWave gated on a non-empty selection
    void testSetClockWaveDisabledWithoutSelectionC9();

private:
    QTemporaryDir m_tempDir;
};

