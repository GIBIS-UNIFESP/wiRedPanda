// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/UI/CircuitRecorder.h"

#include <algorithm>

#include <QColor>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QGraphicsView>
#include <QImage>
#include <QPainter>
#include <QProcess>
#include <QStandardPaths>
#include <QTimer>

#include "App/BeWavedDolphin/DolphinFile.h"
#include "App/BeWavedDolphin/DolphinModelBuilder.h"
#include "App/BeWavedDolphin/Serializer.h"
#include "App/BeWavedDolphin/WaveformSimulator.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Scene/GraphicsView.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/Simulation/SimulationThrottleDisabler.h"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
#include "App/UI/gif.h"
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif

CircuitRecorder::CircuitRecorder(QObject *parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &CircuitRecorder::captureFrame);
}

CircuitRecorder::~CircuitRecorder()
{
    if (isRecording()) {
        stopRecording();
    }
}

bool CircuitRecorder::isFFmpegAvailable()
{
    return !QStandardPaths::findExecutable(QStringLiteral("ffmpeg")).isEmpty();
}

double CircuitRecorder::elapsedSeconds() const
{
    if (m_state == State::Idle) {
        return 0.0;
    }
    qint64 currentRun = (m_state == State::Recording) ? m_elapsedTimer.elapsed() : 0;
    return static_cast<double>(m_accumulatedMs + currentRun) / 1000.0;
}

QRectF CircuitRecorder::determineSourceRect() const
{
    if (!m_scene) {
        return QRectF(0, 0, 800, 600);
    }

    switch (m_config.region) {
    case Region::VisibleViewport:
        if (m_view && m_view->viewport()) {
            QRect viewportRect = m_view->viewport()->rect();
            QRectF sceneRect = m_view->mapToScene(viewportRect).boundingRect();
            if (!sceneRect.isEmpty()) {
                return sceneRect;
            }
        }
        [[fallthrough]];

    case Region::Selection: {
        QList<QGraphicsItem *> selected = m_scene->selectedItems();
        if (!selected.isEmpty()) {
            QRectF selRect;
            for (QGraphicsItem *item : selected) {
                selRect = selRect.united(item->sceneBoundingRect());
            }
            if (!selRect.isEmpty()) {
                return selRect.adjusted(-32, -32, 32, 32);
            }
        }
        [[fallthrough]];
    }

    case Region::FullCircuit:
    default: {
        QRectF bounds = m_scene->cachedItemsBoundingRect();
        if (bounds.isEmpty() || !bounds.isValid()) {
            bounds = QRectF(-100, -100, 800, 600);
        }
        return bounds.adjusted(-32, -32, 32, 32);
    }
    }
}

bool CircuitRecorder::startRecording(Scene *scene, GraphicsView *view, const Configuration &config)
{
    if (!scene || config.filePath.isEmpty()) {
        emit recordingError(tr("Invalid scene or empty output file path."));
        return false;
    }

    if (isRecording()) {
        stopRecording();
    }

    m_scene = scene;
    m_view = view;
    m_config = config;
    m_config.fps = std::clamp(m_config.fps, 10, 30);
    m_config.resolutionScale = std::clamp(m_config.resolutionScale, 0.25, 1.0);

    QRectF sourceRect = determineSourceRect();
    int targetW = std::max(16, static_cast<int>(sourceRect.width() * m_config.resolutionScale));
    int targetH = std::max(16, static_cast<int>(sourceRect.height() * m_config.resolutionScale));

    // FFmpeg and GIF encoders require even dimensions for yuv420p / LZW matrix packing
    m_frameWidth = (targetW & ~1);
    m_frameHeight = (targetH & ~1);

    if (m_config.simulationMode == SimulationMode::DolphinWaveform) {
        return recordDolphinWaveform();
    }

    m_frameCount = 0;
    m_accumulatedMs = 0;
    m_lastFrameMs = 0;

    bool initialized = false;
    if (m_config.format == Format::GIF) {
        initialized = initGifEncoder();
    } else {
        initialized = initFFmpegProcess();
    }

    if (!initialized) {
        return false;
    }

    m_state = State::Recording;
    if (m_scene && m_scene->simulation()) {
        m_throttleDisabler = std::make_unique<SimulationThrottleDisabler>(m_scene->simulation());
    }
    m_elapsedTimer.start();
    m_timer.start(1000 / m_config.fps);

    emit recordingStarted();
    captureFrame(); // Capture initial frame immediately
    return true;
}

bool CircuitRecorder::recordDolphinWaveform()
{
    if (!m_scene) {
        emit recordingError(tr("No active scene to record."));
        return false;
    }

    if (m_config.dolphinFilePath.isEmpty() || !QFile::exists(m_config.dolphinFilePath)) {
        emit recordingError(tr("Dolphin waveform file not found: %1").arg(m_config.dolphinFilePath));
        return false;
    }

    DolphinModelBuilder::Signals dolphinSignals;
    try {
        dolphinSignals = DolphinModelBuilder::collect(m_scene);
    } catch (const std::exception &e) {
        emit recordingError(tr("Failed to collect circuit elements for Dolphin recording: %1").arg(e.what()));
        return false;
    }
    auto inputs = dolphinSignals.inputs;
    int totalInputPorts = dolphinSignals.inputPorts;

    if (totalInputPorts == 0 || inputs.isEmpty()) {
        emit recordingError(tr("The current circuit has no input elements."));
        return false;
    }

    DolphinSerializer::WaveformData waveformData;
    try {
        waveformData = DolphinFile::load(m_config.dolphinFilePath, totalInputPorts);
    } catch (const std::exception &e) {
        emit recordingError(tr("Failed to load Dolphin waveform file: %1").arg(e.what()));
        return false;
    }

    if (waveformData.columns <= 0 || waveformData.values.isEmpty()) {
        emit recordingError(tr("Dolphin file contains no time columns or waveform data."));
        return false;
    }

    bool initialized = false;
    if (m_config.format == Format::GIF) {
        initialized = initGifEncoder();
    } else {
        initialized = initFFmpegProcess();
    }

    if (!initialized) {
        return false;
    }

    // Snapshot live inputs to restore after recording
    QVector<Status> savedInputs = WaveformSimulator::captureInputs(inputs, totalInputPorts);

    m_state = State::Recording;
    emit recordingStarted();

    // Disable simulation visual throttle and block live timer during sweep
    SimulationThrottleDisabler throttleDisabler(m_scene->simulation());
    SimulationBlocker simulationBlocker(m_scene->simulation());

    // Reset sequential element state (flip-flops, counters) before sweep
    for (auto *elm : m_scene->elements()) {
        if (elm) {
            elm->resetSimState();
        }
    }

    int effectiveColumns = waveformData.columns;
    if (m_config.autoTrimTrailingSteps && waveformData.columns > 1) {
        int lastActiveCol = waveformData.columns - 1;
        while (lastActiveCol > 0) {
            bool differsFromPrev = false;
            for (int r = 0; r < waveformData.inputPorts; ++r) {
                int currentVal = waveformData.values[r * waveformData.columns + lastActiveCol];
                int prevVal    = waveformData.values[r * waveformData.columns + (lastActiveCol - 1)];
                if (currentVal != prevVal) {
                    differsFromPrev = true;
                    break;
                }
            }
            if (differsFromPrev) {
                break;
            }
            lastActiveCol--;
        }
        effectiveColumns = lastActiveCol + 1;
    }

    int framesPerStep = std::max(1, static_cast<int>(std::round(m_config.fps * m_config.stepDurationSeconds)));
    m_frameCount = 0;
    m_elapsedTimer.start();

    // Iterate through all columns in the waveform
    for (int col = 0; col < effectiveColumns && m_state == State::Recording; ++col) {
        int row = 0;
        for (auto *input : std::as_const(inputs)) {
            if (!input) continue;
            for (int port = 0; port < input->outputSize(); ++port) {
                if (row < waveformData.inputPorts) {
                    int val = waveformData.values[row * waveformData.columns + col];
                    input->setWaveformValue(val, port);
                }
                row++;
            }
        }

        // Settle simulation logic and update visual element states
        m_scene->simulation()->update();

        // Process pending GUI / repaint events
        QCoreApplication::processEvents();

        // Render frame for this waveform step
        QRectF sourceRect = determineSourceRect();
        QImage image(m_frameWidth, m_frameHeight, QImage::Format_RGBA8888);
        image.fill(Qt::white);

        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        m_scene->render(&painter, QRectF(0, 0, m_frameWidth, m_frameHeight), sourceRect);
        painter.end();

        if (m_config.format == Format::GIF && m_gifInitialized && m_gifWriter) {
            uint32_t stepDelayCentiseconds = static_cast<uint32_t>(std::max(1, static_cast<int>(std::round(m_config.stepDurationSeconds * 100.0))));
            GifWriteFrame(m_gifWriter.get(), image.constBits(),
                          static_cast<uint32_t>(m_frameWidth), static_cast<uint32_t>(m_frameHeight),
                          stepDelayCentiseconds, 8, false);

            m_frameCount++;
            double currentElapsed = (col + 1) * m_config.stepDurationSeconds;
            emit frameRecorded(m_frameCount, currentElapsed);
            QCoreApplication::processEvents();
        } else if (m_ffmpegProcess.isOpen()) {
            for (int f = 0; f < framesPerStep && m_state == State::Recording; ++f) {
                m_ffmpegProcess.write(reinterpret_cast<const char *>(image.constBits()), image.sizeInBytes());
                m_frameCount++;
                double currentElapsed = static_cast<double>(m_frameCount) / m_config.fps;
                emit frameRecorded(m_frameCount, currentElapsed);
                QCoreApplication::processEvents();
            }
        }
    }

    // Restore initial circuit input states
    WaveformSimulator::restoreInputs(inputs, savedInputs);

    stopRecording();
    return true;
}

bool CircuitRecorder::initGifEncoder()
{
    m_gifWriter = std::make_unique<GifWriter>();
    uint32_t delayCentiseconds = static_cast<uint32_t>((100 + m_config.fps / 2) / m_config.fps);
    if (!GifBegin(m_gifWriter.get(), m_config.filePath.toUtf8().constData(),
                  static_cast<uint32_t>(m_frameWidth), static_cast<uint32_t>(m_frameHeight),
                  delayCentiseconds, 8, false)) {
        m_gifWriter.reset();
        emit recordingError(tr("Could not create GIF output file: %1").arg(m_config.filePath));
        return false;
    }
    m_gifInitialized = true;
    return true;
}

bool CircuitRecorder::initFFmpegProcess()
{
    if (!isFFmpegAvailable()) {
        emit recordingError(tr("FFmpeg executable was not found on system PATH."));
        return false;
    }

    QStringList args;
    args << QStringLiteral("-y")
         << QStringLiteral("-f") << QStringLiteral("rawvideo")
         << QStringLiteral("-vcodec") << QStringLiteral("rawvideo")
         << QStringLiteral("-s") << QString("%1x%2").arg(m_frameWidth).arg(m_frameHeight)
         << QStringLiteral("-pix_fmt") << QStringLiteral("rgba")
         << QStringLiteral("-r") << QString::number(m_config.fps)
         << QStringLiteral("-i") << QStringLiteral("-");

    if (m_config.format == Format::MP4) {
        args << QStringLiteral("-c:v") << QStringLiteral("libx264")
             << QStringLiteral("-pix_fmt") << QStringLiteral("yuv420p")
             << QStringLiteral("-preset") << QStringLiteral("fast");
    } else { // WebM
        args << QStringLiteral("-c:v") << QStringLiteral("libvpx-vp9")
             << QStringLiteral("-b:v") << QStringLiteral("1M");
    }

    args << m_config.filePath;

    m_ffmpegProcess.start(QStringLiteral("ffmpeg"), args);
    if (!m_ffmpegProcess.waitForStarted(3000)) {
        emit recordingError(tr("Failed to start FFmpeg process: %1").arg(m_ffmpegProcess.errorString()));
        return false;
    }
    return true;
}

void CircuitRecorder::captureFrame()
{
    if (m_state != State::Recording || !m_scene || m_isCapturing) {
        return;
    }

    m_isCapturing = true;

    QRectF sourceRect = determineSourceRect();
    QImage image(m_frameWidth, m_frameHeight, QImage::Format_RGBA8888);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    m_scene->render(&painter, QRectF(0, 0, m_frameWidth, m_frameHeight), sourceRect);
    painter.end();

    qint64 currentMs = m_elapsedTimer.elapsed();
    qint64 deltaMs = (m_frameCount == 0) ? (1000 / m_config.fps) : (currentMs - m_lastFrameMs);
    m_lastFrameMs = currentMs;

    uint32_t delayCentiseconds = static_cast<uint32_t>(std::max<qint64>(1, (deltaMs + 5) / 10));

    if (m_config.format == Format::GIF && m_gifInitialized && m_gifWriter) {
        GifWriteFrame(m_gifWriter.get(), image.constBits(),
                      static_cast<uint32_t>(m_frameWidth), static_cast<uint32_t>(m_frameHeight),
                      delayCentiseconds, 8, false);
    } else if (m_ffmpegProcess.isOpen()) {
        m_ffmpegProcess.write(reinterpret_cast<const char *>(image.constBits()), image.sizeInBytes());
    }

    m_frameCount++;
    m_isCapturing = false;
    emit frameRecorded(m_frameCount, elapsedSeconds());
}

void CircuitRecorder::pauseRecording()
{
    if (m_state == State::Recording) {
        m_timer.stop();
        m_accumulatedMs += m_elapsedTimer.elapsed();
        m_state = State::Paused;
    }
}

void CircuitRecorder::resumeRecording()
{
    if (m_state == State::Paused) {
        m_state = State::Recording;
        m_elapsedTimer.start();
        m_timer.start(1000 / m_config.fps);
    }
}

void CircuitRecorder::stopRecording()
{
    if (m_state == State::Idle) {
        return;
    }

    m_timer.stop();
    cleanupEncoder();
    m_state = State::Idle;

    emit recordingStopped(m_config.filePath);
}

void CircuitRecorder::cleanupEncoder()
{
    m_throttleDisabler.reset();

    if (m_gifInitialized && m_gifWriter) {
        GifEnd(m_gifWriter.get());
        m_gifWriter.reset();
        m_gifInitialized = false;
    }

    if (m_ffmpegProcess.state() != QProcess::NotRunning) {
        m_ffmpegProcess.closeWriteChannel();
        if (!m_ffmpegProcess.waitForFinished(5000)) {
            m_ffmpegProcess.kill();
        }
    }
}
