// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// `simulation_control` operates on CanvasItem::simulation(). `create_waveform`/
/// `export_waveform` maintain an `m_persistentDolphin` against `QuickDolphinController`;
/// QuickSimulationHandler.h declares this test class a friend purely for
/// `testCreateWaveformReplacesExistingDolphin`'s identity check.
class TestQuickSimulationHandler : public QObject
{
    Q_OBJECT

private slots:
    void testSimulationControlRejectsMissingAction();
    void testSimulationControlRejectsEmptyAction();
    void testSimulationControlRejectsNoScene();
    void testSimulationControlStartStopRestartUpdate();
    void testSimulationControlRejectsInvalidAction();

    void testCreateWaveformRejectsNoScene();
    void testCreateWaveformRejectsInvalidDuration();
    void testCreateWaveformAcceptsMaxDuration();
    void testCreateWaveformCreatesRealWaveform();
    void testCreateWaveformCreatesRealWaveformWithInputPatterns();
    void testCreateWaveformRejectsUnknownInputLabel();
    void testCreateWaveformRejectsPatternLengthMismatch();
    void testCreateWaveformRejectsInvalidPatternValue();
    void testCreateWaveformReplacesExistingDolphin();
    void testCreateWaveformResponseValidatesAgainstTheSchema();

    void testExportWaveformRejectsMissingParams();
    void testExportWaveformRejectsInvalidFormat();
    void testExportWaveformRejectsNoWaveformYet();
    void testExportWaveformExportsRealTxtFile();
    void testExportWaveformRejectsFileOpenFailure();
    void testExportWaveformExportsRealPngFile();
    void testExportWaveformRejectsPngExportFailure();

    void testHandleCommandRejectsUnknownCommand();
    void testHeapAllocationDeletesCleanly();
};
