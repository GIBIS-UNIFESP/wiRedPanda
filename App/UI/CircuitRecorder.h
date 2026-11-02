// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief CircuitRecorder: Captures live simulation frames into GIF or video formats.
 */

#pragma once

#include <memory>

#include <QElapsedTimer>
#include <QImage>
#include <QObject>
#include <QProcess>
#include <QRectF>
#include <QString>
#include <QTimer>

struct GifWriter;

class Scene;
class GraphicsView;

/**
 * \class CircuitRecorder
 * \brief Orchestrates live circuit simulation recording to animated GIF or MP4/WebM video.
 */
class CircuitRecorder : public QObject
{
    Q_OBJECT

public:
    enum class SimulationMode {
        Live,
        DolphinWaveform
    };

    enum class Format {
        GIF,
        MP4,
        WebM
    };

    enum class Region {
        FullCircuit,
        VisibleViewport,
        Selection
    };

    enum class State {
        Idle,
        Recording,
        Paused
    };

    struct Configuration {
        QString filePath;
        Format format = Format::GIF;
        Region region = Region::FullCircuit;
        int fps = 15;                   ///< 10..30 FPS
        double resolutionScale = 1.0;   ///< 0.25..1.0 scale factor

        SimulationMode simulationMode = SimulationMode::Live;
        QString dolphinFilePath;
        double stepDurationSeconds = 1.0; ///< Hold duration per Dolphin step (seconds)
        bool autoTrimTrailingSteps = true; ///< Auto-trim trailing steps with no input changes
    };

    explicit CircuitRecorder(QObject *parent = nullptr);
    ~CircuitRecorder() override;

    /// Returns \c true if FFmpeg binary is installed and found on the system PATH.
    static bool isFFmpegAvailable();

    /// Returns current recording state.
    State state() const { return m_state; }

    /// Returns \c true if currently recording or paused.
    bool isRecording() const { return m_state != State::Idle; }

    /// Returns current configuration.
    const Configuration &configuration() const { return m_config; }

    /// Returns total frames recorded in current session.
    int frameCount() const { return m_frameCount; }

    /// Returns total elapsed recording time in seconds.
    double elapsedSeconds() const;

    /// Starts recording \a scene / \a view using \a config.
    bool startRecording(Scene *scene, GraphicsView *view, const Configuration &config);

    /// Stops recording and finalizes output file.
    void stopRecording();

    /// Pauses frame capture.
    void pauseRecording();

    /// Resumes frame capture.
    void resumeRecording();

signals:
    void recordingStarted();
    void recordingStopped(const QString &filePath);
    void frameRecorded(int frameCount, double durationSeconds);
    void recordingError(const QString &errorMessage);

private slots:
    void captureFrame();

private:
    bool initGifEncoder();
    bool initFFmpegProcess();
    void cleanupEncoder();
    bool recordDolphinWaveform();
    QRectF determineSourceRect() const;

    Scene *m_scene = nullptr;
    GraphicsView *m_view = nullptr;
    Configuration m_config;
    State m_state = State::Idle;

    QTimer m_timer;
    QElapsedTimer m_elapsedTimer;
    qint64 m_accumulatedMs = 0;
    qint64 m_lastFrameMs = 0;

    int m_frameCount = 0;
    int m_frameWidth = 0;
    int m_frameHeight = 0;

    // GIF Encoder State
    std::unique_ptr<GifWriter> m_gifWriter;
    bool m_gifInitialized = false;
    bool m_isCapturing = false;

    // FFmpeg Process State
    QProcess m_ffmpegProcess;

    // Visual refresh throttle disabler guard while recording
    std::unique_ptr<class SimulationThrottleDisabler> m_throttleDisabler;
};
