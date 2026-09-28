// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Ui/TestCircuitRecorder.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTest>

#include "App/Core/Application.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"
#include "App/UI/CircuitRecorder.h"
#include "App/UI/CircuitRecorderDialog.h"

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
