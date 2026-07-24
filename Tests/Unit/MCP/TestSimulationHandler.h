// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestSimulationHandler : public QObject
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
    void testCreateWaveformCreatesRealWaveform();
    void testCreateWaveformRejectsUnknownInputLabel();
    void testCreateWaveformRejectsPatternLengthMismatch();
    void testCreateWaveformRejectsInvalidPatternValue();
    void testCreateWaveformReplacesExistingDolphin();

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
