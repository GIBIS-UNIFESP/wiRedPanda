// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickExerciseController.h"

#include <QDebug>

#include "App/Exercise/ExerciseEngine.h"
#include "App/Exercise/ExerciseStep.h"
#include "App/QuickShell/Canvas/CanvasItem.h"

QuickExerciseController::QuickExerciseController(ExerciseEngine *engine, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
{
    connect(m_engine, &ExerciseEngine::stepChanged, this, &QuickExerciseController::onStepChanged);
    connect(m_engine, &ExerciseEngine::exerciseCompleted, this, &QuickExerciseController::onExerciseCompleted);
    connect(m_engine, &ExerciseEngine::exerciseStopped, this, &QuickExerciseController::onExerciseStopped);
    connect(m_engine, &ExerciseEngine::retranslated, this, &QuickExerciseController::onRetranslated);
}

void QuickExerciseController::setCanvas(CanvasItem *canvas)
{
    m_canvas = canvas;
    if (m_engine->isActive()) {
        m_engine->setCanvas(canvas);
    }
}

bool QuickExerciseController::isActive() const
{
    return m_engine->isActive();
}

QString QuickExerciseController::stepCounterText() const
{
    if (m_completed) {
        return {};
    }
    return tr("Step %1 of %2").arg(m_step + 1).arg(m_total);
}

QString QuickExerciseController::instruction() const
{
    return m_completed ? tr("Exercise complete! Well done.") : m_instruction;
}

QString QuickExerciseController::hintButtonText() const
{
    return m_hintVisible ? tr("Hide hint") : tr("Hint");
}

bool QuickExerciseController::start(const QString &resourcePath)
{
    if (resourcePath.isEmpty()) {
        return false;
    }
    if (!m_engine->loadFromResource(resourcePath)) {
        qWarning() << "ExerciseEngine: failed to load" << resourcePath;
        return false;
    }
    m_engine->setCanvas(m_canvas);
    m_engine->start();
    return true;
}

void QuickExerciseController::toggleHint()
{
    m_hintVisible = !m_hintVisible;
    emit hintVisibleChanged();
}

void QuickExerciseController::goToPreviousStep()
{
    m_engine->goToPreviousStep();
}

void QuickExerciseController::advanceStep()
{
    if (m_completed) {
        // Next has been relabeled Close -- mirrors ExerciseOverlay::onExerciseCompleted()'s
        // rewiring of the real button's own click handler.
        close();
        return;
    }
    m_engine->advanceStep();
}

void QuickExerciseController::close()
{
    m_engine->stop();
}

void QuickExerciseController::onStepChanged(int step, int total, const ExerciseStep &data)
{
    m_step = step;
    m_total = total;
    m_instruction = data.instruction;
    m_hint = data.hint;
    m_hintVisible = false;

    const bool isObserveStep = data.requiredElements.isEmpty() && data.requiredConnections.isEmpty();
    const bool isLastStep = (step == total - 1);
    m_nextButtonText = isLastStep ? tr("Finish") : tr("Next →");
    m_nextEnabled = isObserveStep;
    m_prevEnabled = step > 0;
    m_completed = false;

    emit activeChanged();
    emit completedChanged();
    emit stepChanged();
    emit hintVisibleChanged();
}

void QuickExerciseController::onExerciseCompleted()
{
    m_completed = true;
    m_hintVisible = false;
    m_prevEnabled = false;
    m_nextButtonText = tr("Close");
    m_nextEnabled = true;

    emit completedChanged();
    emit stepChanged();
    emit hintVisibleChanged();
}

void QuickExerciseController::onExerciseStopped()
{
    m_completed = false;
    emit activeChanged();
}

void QuickExerciseController::onRetranslated()
{
    const ExerciseStep &data = m_engine->currentStepData();
    m_instruction = data.instruction;
    m_hint = data.hint;
    const bool isLastStep = (m_engine->currentStep() == m_engine->totalSteps() - 1);
    m_nextButtonText = isLastStep ? tr("Finish") : tr("Next →");
    emit stepChanged();
}
