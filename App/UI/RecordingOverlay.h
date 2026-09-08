// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief RecordingOverlay: On-screen visual badge indicating active simulation recording.
 */

#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class QTimer;

/**
 * \class RecordingOverlay
 * \brief Floating visual badge overlay displayed over the circuit view while recording.
 */
class RecordingOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit RecordingOverlay(QWidget *parent = nullptr);
    ~RecordingOverlay() override = default;

    /// Updates elapsed time and frame count displayed on the badge.
    void updateStatus(int frameCount, double durationSeconds);

signals:
    /// Emitted when the user clicks the "Stop" button on the overlay badge.
    void stopRequested();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void toggleBlink();

private:
    void setupUI();

    /// Grants TestCircuitRecorder read-only access to m_blinkTimer for
    /// regressionBlinkTimerNeverStopsAfterHide() -- same pattern as
    /// Workspace.h's `friend class TestWorkspaceUnit`.
    friend class TestCircuitRecorder;

    QLabel *m_redDotLabel = nullptr;
    QLabel *m_timeLabel = nullptr;
    QPushButton *m_stopButton = nullptr;
    QTimer *m_blinkTimer = nullptr;
    bool m_dotVisible = true;
};
