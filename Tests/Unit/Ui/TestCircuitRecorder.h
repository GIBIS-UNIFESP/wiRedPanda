// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestCircuitRecorder : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testInitialState();
    void testConfigurationClamping();
    void testFFmpegAvailability();
    void testGifRecordingStartStop();
    void testDialogConfiguration();
    void testDolphinWaveformRecording();

    // --- Regression tests for the deep-review findings in
    // .claude/PR533_VIVID_FROG_REVIEW.md. Each test names the finding it covers.
    // Findings #1, #15, #18 crash the process itself (use-after-free), so those are
    // QSKIP-guarded -- remove the QSKIP once the underlying fix lands so the assertions
    // that follow it actually run. The others use QEXPECT_FAIL (Qt Test's XFAIL) rather
    // than a bare failing assertion: this executable also produces the ctest
    // "non_gui_complete" fixture every GUI test requires (CMakeLists.txt), so a hard
    // failure here would silently prevent unrelated GUI tests from running at all.
    // QEXPECT_FAIL keeps the exit code clean while still reporting XFAIL until fixed --
    // once fixed, the assertion XPASSes, which Qt Test *does* flag, as the signal to
    // remove that QEXPECT_FAIL line.
    void regressionDanglingSceneCrashOnCaptureFrame(); // finding #1
    void regressionDolphinRecordingCorruptsFlipFlopState(); // finding #2
    void regressionDialogWipesDefaultPathOnFirstUse(); // finding #8
    void regressionBlinkTimerNeverStopsAfterHide(); // finding #10
    void regressionNoOverwriteConfirmation(); // finding #14
    void regressionDeleteInputElementMidSweepCrash(); // finding #15
    void regressionFfmpegDashPrefixedFilenameFailsSilently(); // findings #16 + #17
    void regressionOversizedCanvasCrash(); // finding #18
    void regressionRecordingOverlayPositionUnclampedForNarrowTab(); // finding #20

private:
    bool m_prevRendering = false;
};
