// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QColor>
#include <QWidget>

#include "App/Exercise/ExerciseStep.h"

class QLabel;
class QPushButton;
class ExerciseEngine;

/// Semi-transparent overlay displayed at the bottom of the canvas during a circuit exercise.
class ExerciseOverlay : public QWidget
{
    Q_OBJECT
    friend class TestExerciseEngine;

public:
    explicit ExerciseOverlay(ExerciseEngine *engine, QWidget *parent = nullptr);

    /// Repositions the overlay to the bottom-centre of its parent widget.
    /// Called by WorkSpace::resizeEvent and after reparenting.
    void repositionToParent();

public slots:
    void onStepChanged(int step, int total, const ExerciseStep &stepData);
    void onExerciseCompleted();
    void onRetranslated();

signals:
    void closeRequested();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUi();
    void updateNextButton(bool isLastStep);
    void applyTheme();

    /// Scales a \a basePx literal (tuned against a 13px baseline) by the application's current
    /// font size, so this overlay's text respects an OS/Qt font-scale accessibility setting
    /// instead of staying visually frozen at a fixed pixel size.
    static int scaledFontPx(int basePx);

    ExerciseEngine *m_engine;

    QColor m_bgColor;
    QColor m_textColor;
    QColor m_hintColor;
    QColor m_counterColor;

    QLabel       *m_stepCounter      = nullptr;
    QLabel       *m_instructionLabel = nullptr;
    QLabel       *m_hintLabel        = nullptr;
    QPushButton  *m_hintButton       = nullptr;
    QPushButton  *m_prevButton       = nullptr;
    QPushButton  *m_nextButton       = nullptr;
    QPushButton  *m_closeButton      = nullptr;

    bool m_hintVisible = false;
};
