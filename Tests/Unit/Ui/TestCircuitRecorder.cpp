// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestCircuitRecorder.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>

#include "App/Core/Application.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/UI/CircuitRecorder.h"
#include "App/UI/CircuitRecorderDialog.h"
#include "App/UI/RecordingOverlay.h"

void TestCircuitRecorder::initTestCase()
{
    m_prevRendering = Application::renderingEnabled;
    Application::renderingEnabled = true;
}

void TestCircuitRecorder::cleanupTestCase()
{
    Application::renderingEnabled = m_prevRendering;
}

void TestCircuitRecorder::testInitialState()
{
    CircuitRecorder recorder;
    QCOMPARE(recorder.state(), CircuitRecorder::State::Idle);
    QVERIFY(!recorder.isRecording());
    QCOMPARE(recorder.frameCount(), 0);
    QCOMPARE(recorder.elapsedSeconds(), 0.0);
}

void TestCircuitRecorder::testConfigurationClamping()
{
    Scene scene;
    CircuitRecorder recorder;

    CircuitRecorder::Configuration config;
    config.filePath = QDir::tempPath() + QStringLiteral("/clamp_test.gif");
    config.fps = 50; // Should clamp to 30
    config.resolutionScale = 2.5; // Should clamp to 1.0

    bool started = recorder.startRecording(&scene, nullptr, config);
    QVERIFY(started);

    QCOMPARE(recorder.configuration().fps, 30);
    QCOMPARE(recorder.configuration().resolutionScale, 1.0);

    recorder.stopRecording();
    QFile::remove(config.filePath);
}

void TestCircuitRecorder::testFFmpegAvailability()
{
    bool available = CircuitRecorder::isFFmpegAvailable();
    // Just verify call completes without error
    Q_UNUSED(available);
}

void TestCircuitRecorder::testGifRecordingStartStop()
{
    Scene scene;
    CircuitRecorder recorder;

    QString tempPath = QDir::tempPath() + QStringLiteral("/test_sim_record.gif");
    if (QFile::exists(tempPath)) {
        QFile::remove(tempPath);
    }

    QSignalSpy spyStarted(&recorder, &CircuitRecorder::recordingStarted);
    QSignalSpy spyStopped(&recorder, &CircuitRecorder::recordingStopped);
    QSignalSpy spyFrame(&recorder, &CircuitRecorder::frameRecorded);

    CircuitRecorder::Configuration config;
    config.filePath = tempPath;
    config.format = CircuitRecorder::Format::GIF;
    config.fps = 15;

    bool started = recorder.startRecording(&scene, nullptr, config);
    QVERIFY(started);
    QCOMPARE(spyStarted.count(), 1);

    // Allow timer to trigger a couple frame captures
    QTest::qWait(250);

    QVERIFY(recorder.frameCount() >= 1);
    QVERIFY(spyFrame.count() >= 1);

    recorder.stopRecording();
    QCOMPARE(spyStopped.count(), 1);
    QCOMPARE(recorder.state(), CircuitRecorder::State::Idle);

    QVERIFY(QFile::exists(tempPath));
    QVERIFY(QFile(tempPath).size() > 0);

    QFile::remove(tempPath);
}

void TestCircuitRecorder::testDialogConfiguration()
{
    CircuitRecorderDialog dialog;
    CircuitRecorder::Configuration config;
    config.filePath = QStringLiteral("/tmp/custom_recording.gif");
    config.fps = 20;
    config.region = CircuitRecorder::Region::VisibleViewport;
    config.resolutionScale = 0.75;
    config.simulationMode = CircuitRecorder::SimulationMode::DolphinWaveform;
    config.dolphinFilePath = QStringLiteral("/tmp/sample.dolphin");
    config.stepDurationSeconds = 1.5;

    dialog.setConfiguration(config);

    auto result = dialog.configuration();
    QCOMPARE(result.filePath, config.filePath);
    QCOMPARE(result.fps, config.fps);
    QCOMPARE(result.region, config.region);
    QCOMPARE(result.resolutionScale, config.resolutionScale);
    QCOMPARE(result.simulationMode, config.simulationMode);
    QCOMPARE(result.dolphinFilePath, config.dolphinFilePath);
    QCOMPARE(result.stepDurationSeconds, config.stepDurationSeconds);
}

void TestCircuitRecorder::testDolphinWaveformRecording()
{
    Scene scene;
    auto *inputSwitch = new InputSwitch();
    scene.addItem(inputSwitch);
    auto *led = new Led();
    scene.addItem(led);

    QString tempDolphinPath = QDir::tempPath() + QStringLiteral("/test_wave.csv");
    if (QFile::exists(tempDolphinPath)) {
        QFile::remove(tempDolphinPath);
    }

    // Create a 5-column .csv waveform file with trailing duplicate steps
    QFile file(tempDolphinPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("1,5,\n0,1,1,1,1,\n");
    file.close();

    QString tempGifPath = QDir::tempPath() + QStringLiteral("/test_dolphin_record.gif");
    if (QFile::exists(tempGifPath)) {
        QFile::remove(tempGifPath);
    }

    CircuitRecorder recorder;
    CircuitRecorder::Configuration config;
    config.filePath = tempGifPath;
    config.format = CircuitRecorder::Format::GIF;
    config.simulationMode = CircuitRecorder::SimulationMode::DolphinWaveform;
    config.dolphinFilePath = tempDolphinPath;
    config.fps = 10;
    config.stepDurationSeconds = 0.2;
    config.autoTrimTrailingSteps = true;

    bool started = recorder.startRecording(&scene, nullptr, config);
    QVERIFY(started);
    QCOMPARE(recorder.frameCount(), 2); // 5 columns auto-trimmed to 2 active columns
    QVERIFY(QFile::exists(tempGifPath));
    QVERIFY(QFile(tempGifPath).size() > 0);

    QFile::remove(tempDolphinPath);
    QFile::remove(tempGifPath);
}

// ============================================================================================
// Regression tests for .claude/PR533_VIVID_FROG_REVIEW.md's deep-review findings.
// ============================================================================================

void TestCircuitRecorder::regressionDanglingSceneCrashOnCaptureFrame()
{
    QSKIP("Finding #1: use-after-free, reproduced as a real segfault during review (GDB "
          "backtrace lands at CircuitRecorder.cpp:390, m_scene->render(...)). Remove this "
          "QSKIP once the fix lands so the assertions below actually run.");

    // Simulates what WorkspaceManager::closeTab()'s deleteLater() eventually does: destroy
    // the Scene a Live-mode recording is still pointing at, then let the still-armed QTimer
    // (here invoked directly and deterministically instead of racing a real one) fire
    // captureFrame() against the now-dangling CircuitRecorder::m_scene.
    auto *scene = new Scene();
    auto *inputSwitch = new InputSwitch();
    scene->addItem(inputSwitch);

    CircuitRecorder recorder;
    CircuitRecorder::Configuration config;
    config.filePath = QDir::tempPath() + QStringLiteral("/regression_dangling_scene.gif");
    config.format = CircuitRecorder::Format::GIF;
    config.fps = 15;

    bool started = recorder.startRecording(scene, nullptr, config);
    QVERIFY(started); // captures frame 0 synchronously while the scene is still alive

    delete scene; // CircuitRecorder::m_scene is now dangling, non-null

    // The next QTimer tick would call this same slot on the real, freed Scene.
    QMetaObject::invokeMethod(&recorder, "captureFrame");

    recorder.stopRecording();
    QFile::remove(config.filePath);
}

void TestCircuitRecorder::regressionDolphinRecordingCorruptsFlipFlopState()
{
    // Finding #2: recordDolphinWaveform() calls resetSimState() on every element to make the
    // sweep deterministic, but only WaveformSimulator::captureInputs()/restoreInputs()
    // snapshot and restore afterward -- sequential elements like flip-flops are left in
    // whatever state the sweep produced, not the state the live circuit was in before the
    // user clicked Record.
    Scene scene;
    auto *inputSwitch = new InputSwitch();
    scene.addItem(inputSwitch);
    // DolphinModelBuilder::collect() requires at least one ElementGroup::Output element
    // present (unrelated to this finding); the DFlipFlop itself is neither Input nor Output
    // group, so it needs a real Led alongside it to reach recordDolphinWaveform() at all.
    auto *led = new Led();
    scene.addItem(led);
    auto *dff = new DFlipFlop();
    scene.addItem(dff);
    // Size the simulation output vector -- required before outputValue()/setOutputValue()
    // are meaningful (outputValue() returns Unknown for any index >= this size), same as
    // TestElementLogic.cpp's initElm() helper does for direct-logic element tests.
    dff->initSimulationVectors(dff->inputSize(), dff->outputSize());

    // Force Q active, simulating an in-progress live circuit the user is demonstrating --
    // resetSimState()'s power-on default is Inactive, so this is a real state change the
    // sweep must not clobber.
    dff->setOutputValue(0, true);
    QCOMPARE(dff->outputValue(0), Status::Active);

    QString tempDolphinPath = QDir::tempPath() + QStringLiteral("/regression_dff_state.csv");
    QFile::remove(tempDolphinPath);
    QFile file(tempDolphinPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("1,2,\n0,1,\n");
    file.close();

    QString tempGifPath = QDir::tempPath() + QStringLiteral("/regression_dff_state.gif");
    QFile::remove(tempGifPath);

    CircuitRecorder recorder;
    CircuitRecorder::Configuration config;
    config.filePath = tempGifPath;
    config.format = CircuitRecorder::Format::GIF;
    config.simulationMode = CircuitRecorder::SimulationMode::DolphinWaveform;
    config.dolphinFilePath = tempDolphinPath;
    config.fps = 10;
    config.stepDurationSeconds = 0.05;
    config.autoTrimTrailingSteps = false;

    bool started = recorder.startRecording(&scene, nullptr, config);
    QVERIFY(started);

    // XFAIL, not a hard failure: TestCircuitRecorder feeds the ctest "non_gui_complete"
    // fixture that every GUI test requires (CMakeLists.txt:1145/1158/1165) -- a bare failing
    // assertion here would silently skip 7 unrelated GUI tests, not just fail this one.
    // Remove this QEXPECT_FAIL once finding #2 is fixed (it'll then XPASS, which Qt Test
    // does treat as a failure, as your signal to remove it).
    QEXPECT_FAIL("", "Finding #2: recordDolphinWaveform() resets every element's sim state "
                      "but never restores sequential elements' state afterward.", Continue);
    QVERIFY2(dff->outputValue(0) == Status::Active,
             "Dolphin-waveform recording reset the DFlipFlop's Q output instead of restoring "
             "it to its pre-recording state (finding #2)");

    QFile::remove(tempDolphinPath);
    QFile::remove(tempGifPath);
}

void TestCircuitRecorder::regressionDialogWipesDefaultPathOnFirstUse()
{
    // Finding #8: CircuitRecorderDialog::setConfiguration() unconditionally does
    // m_filePathEdit->setText(config.filePath). setupUI() (run by the constructor) already
    // populated a good non-empty default; ExportController::recordSimulationDialog() then
    // calls setConfiguration(m_recorder->configuration()) with a freshly-constructed
    // CircuitRecorder's config, whose filePath is still "" on a session's first use --
    // wiping the good default back to blank before the dialog is ever shown.
    CircuitRecorderDialog dialog;

    CircuitRecorder freshRecorder;
    QVERIFY(freshRecorder.configuration().filePath.isEmpty()); // sanity: this is what triggers it

    dialog.setConfiguration(freshRecorder.configuration());

    QEXPECT_FAIL("", "Finding #8: setConfiguration() unconditionally overwrites the Output "
                      "File field, clobbering the good default on first use.", Continue);
    QVERIFY2(!dialog.configuration().filePath.isEmpty(),
             "setConfiguration() wiped the dialog's good default Output File path with an "
             "empty string (finding #8)");
}

void TestCircuitRecorder::regressionBlinkTimerNeverStopsAfterHide()
{
    // Finding #10: RecordingOverlay's blink QTimer is started (guarded by isActive()) the
    // first time updateStatus() runs, but nothing ever stops it -- including when the
    // overlay is hidden. After the first recording session on a tab, it keeps firing every
    // 500ms for the rest of that WorkSpace's lifetime.
    RecordingOverlay overlay;
    overlay.updateStatus(1, 1.0);
    QVERIFY(overlay.m_blinkTimer->isActive());

    overlay.hide();

    QEXPECT_FAIL("", "Finding #10: nothing ever stops the blink timer, including hide().", Continue);
    QVERIFY2(!overlay.m_blinkTimer->isActive(),
             "RecordingOverlay's blink timer is still running after hide() (finding #10)");
}

void TestCircuitRecorder::regressionNoOverwriteConfirmation()
{
    // Finding #14: every other export flow (exportPdfDialog/exportImageDialog) forces a
    // native save dialog on every use, which has OS-level overwrite protection.
    // CircuitRecorderDialog::accept() only validates a non-empty path (and, for Dolphin
    // mode, that the waveform file exists) -- it never checks whether the target output
    // file already exists, so accepting the pre-filled last-used path silently overwrites a
    // previous recording with zero warning.
    CircuitRecorderDialog dialog;
    QString path = QDir::tempPath() + QStringLiteral("/regression_overwrite_test.gif");
    QFile::remove(path);
    {
        QFile existing(path);
        QVERIFY(existing.open(QIODevice::WriteOnly));
        existing.write("previous recording contents");
    }

    CircuitRecorder::Configuration config;
    config.filePath = path;
    dialog.setConfiguration(config);

    // Defensive: if a future fix adds a blocking overwrite-confirmation QMessageBox inside
    // accept(), auto-dismiss it (accepting the overwrite) so this test can't hang -- its only
    // job is to prove today's accept() raises *no* such prompt at all.
    bool modalSeen = false;
    QTimer::singleShot(0, &dialog, [&modalSeen]() {
        if (auto *mb = qobject_cast<QMessageBox *>(QApplication::activeModalWidget())) {
            modalSeen = true;
            mb->accept();
        }
    });

    dialog.accept();
    QCoreApplication::processEvents();

    QEXPECT_FAIL("", "Finding #14: accept() never checks whether the output file already "
                      "exists, so it never raises an overwrite prompt.", Continue);
    QVERIFY2(modalSeen,
             "CircuitRecorderDialog::accept() raised no overwrite-confirmation prompt for an "
             "already-existing output file (finding #14)");

    QFile::remove(path);
}

void TestCircuitRecorder::regressionDeleteInputElementMidSweepCrash()
{
    QSKIP("Finding #15: use-after-free, reproduced as a real segfault during review (GDB "
          "backtrace lands at CircuitRecorder.cpp:271, recordDolphinWaveform()'s "
          "input->outputSize() call). Remove this QSKIP once the fix lands.");

    // Reproduces pressing Delete on the swept input element while a Dolphin sweep is
    // running: recordDolphinWaveform() collects dolphinSignals.inputs once, then reuses it
    // across every column of a loop that calls unfiltered QCoreApplication::processEvents()
    // -- exactly the reentrancy window a user's Delete keypress (processed through
    // DeleteItemsCommand, an immediate synchronous `delete`, not deleteLater()) exploits.
    Scene scene;
    auto *inputSwitch = new InputSwitch();
    scene.addItem(inputSwitch);
    auto *led = new Led();
    scene.addItem(led);

    QString tempDolphinPath = QDir::tempPath() + QStringLiteral("/regression_delete_mid_sweep.csv");
    QFile::remove(tempDolphinPath);
    QFile file(tempDolphinPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QByteArray csv = "1,40,\n0";
    for (int i = 1; i < 40; ++i) {
        csv += (i % 2 == 0) ? ",1" : ",0";
    }
    csv += ",\n";
    file.write(csv);
    file.close();

    // Same effect a user pressing Delete produces via DeleteItemsCommand::redo().
    QTimer::singleShot(0, &scene, [&scene, inputSwitch]() {
        scene.removeItem(inputSwitch);
        delete inputSwitch;
    });

    CircuitRecorder recorder;
    CircuitRecorder::Configuration config;
    config.filePath = QDir::tempPath() + QStringLiteral("/regression_delete_mid_sweep.gif");
    config.format = CircuitRecorder::Format::GIF;
    config.simulationMode = CircuitRecorder::SimulationMode::DolphinWaveform;
    config.dolphinFilePath = tempDolphinPath;
    config.fps = 10;
    config.stepDurationSeconds = 0.01;
    config.autoTrimTrailingSteps = false;

    recorder.startRecording(&scene, nullptr, config);

    QFile::remove(tempDolphinPath);
    QFile::remove(config.filePath);
}

void TestCircuitRecorder::regressionFfmpegDashPrefixedFilenameFailsSilently()
{
    // Findings #16 (initFFmpegProcess() appends the output path as a bare positional ffmpeg
    // argument with no `--` separator; a filename the user types directly into the dialog's
    // free-text field -- not routed through Browse -- that starts with `-` gets parsed as an
    // unrecognized ffmpeg option and rejected outright, empirically verified against the real
    // ffmpeg 8.0.1 binary during the review) and #17 (nothing detects that failure, so
    // recordingError is never emitted).
    if (!CircuitRecorder::isFFmpegAvailable()) {
        QSKIP("ffmpeg not available on this machine -- cannot exercise the real subprocess.");
    }

    Scene scene;
    auto *inputSwitch = new InputSwitch();
    scene.addItem(inputSwitch);

    CircuitRecorder recorder;
    QSignalSpy spyError(&recorder, &CircuitRecorder::recordingError);

    // Deliberately relative (no directory) and dash-prefixed -- this is what actually
    // triggers ffmpeg's argv parser rejecting it as an unrecognized option; an absolute path
    // (e.g. under QDir::tempPath()) starts with '/' and would NOT reproduce this.
    CircuitRecorder::Configuration config;
    config.filePath = QStringLiteral("-regression_dash_prefixed.mp4");
    config.format = CircuitRecorder::Format::MP4;
    config.fps = 10;

    bool started = recorder.startRecording(&scene, nullptr, config);
    QVERIFY(started); // waitForStarted() succeeds -- ffmpeg forks fine, then exits on the bad arg

    QTest::qWait(500); // give ffmpeg time to parse argv and die, and the recorder time to notice (or not)
    recorder.stopRecording();

    QEXPECT_FAIL("", "Finding #17: no errorOccurred/finished handling for m_ffmpegProcess, "
                      "so a subprocess that dies after starting (here: finding #16's rejected "
                      "dash-prefixed argument) is never reported.", Continue);
    QVERIFY2(spyError.count() > 0,
             "CircuitRecorder never emitted recordingError despite ffmpeg dying immediately "
             "on a rejected dash-prefixed filename argument (finding #17)");

    QFile::remove(config.filePath);
}

void TestCircuitRecorder::regressionOversizedCanvasCrash()
{
    QSKIP("Finding #18: unbounded frame dimensions, reproduced as a real segfault during "
          "review (GDB backtrace lands at gif.h:120, GifMakePalette dereferencing a null "
          "QImage buffer). Remove this QSKIP once the fix lands.");

    // startRecording() clamps resolutionScale down but never caps the resulting frame
    // dimensions; Scene's own bounding rect has no maximum-size cap either. A sufficiently
    // large/sprawling circuit makes the per-frame QImage allocation fail (Qt returns a null
    // image rather than throwing), and captureFrame() never checks image.isNull() before
    // handing constBits() (now nullptr) to the GIF encoder.
    Scene scene;
    auto *inputSwitch = new InputSwitch();
    scene.addItem(inputSwitch);
    inputSwitch->setPos(0, 0);
    auto *led = new Led();
    scene.addItem(led);
    led->setPos(150000, 150000); // inflates cachedItemsBoundingRect() to ~150000x150000

    CircuitRecorder recorder;
    CircuitRecorder::Configuration config;
    config.filePath = QDir::tempPath() + QStringLiteral("/regression_oversized_canvas.gif");
    config.format = CircuitRecorder::Format::GIF;
    config.region = CircuitRecorder::Region::FullCircuit;
    config.resolutionScale = 1.0;

    recorder.startRecording(&scene, nullptr, config);

    recorder.stopRecording();
    QFile::remove(config.filePath);
}

void TestCircuitRecorder::regressionRecordingOverlayPositionUnclampedForNarrowTab()
{
    // Finding #20: both ExportController.cpp:42 and Workspace.cpp:161 position the overlay
    // via `move(width() - overlay->width() - 20, 16)` with no lower-bound clamp. A tab
    // narrower than overlay->width() + 20 pushes the overlay -- including its Stop button --
    // partly or fully off the left edge.
    WorkSpace workspace;
    workspace.show();
    QVERIFY(QTest::qWaitForWindowExposed(&workspace));

    auto *overlay = workspace.recordingOverlay();
    QVERIFY(overlay);
    overlay->show();
    const int overlayWidth = overlay->width();
    QVERIFY(overlayWidth > 0);

    // Narrower than the overlay itself -- resizeEvent() repositions it, but the unclamped
    // formula pushes x negative instead of keeping it on-screen.
    workspace.resize(overlayWidth / 2, 200);
    QCoreApplication::processEvents();

    QEXPECT_FAIL("", "Finding #20: the overlay's move() formula has no lower-bound clamp.", Continue);
    QVERIFY2(overlay->x() >= 0,
             "RecordingOverlay was pushed off the left edge of a narrow tab (finding #20)");
}
