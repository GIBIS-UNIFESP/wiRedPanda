// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QPointer>
#include <QVector>

#include "App/Exercise/ExerciseStep.h"

class Scene;

class ExerciseEngine : public QObject
{
    Q_OBJECT

public:
    explicit ExerciseEngine(QObject *parent = nullptr);

    /// Loads an exercise from a Qt resource path (e.g. ":/Exercises/basic-and-gate.json").
    /// Returns false and leaves the engine inactive if the resource is missing or malformed.
    bool loadFromResource(const QString &resourcePath);

    QString exerciseId()    const { return m_id; }
    QString exerciseTitle() const { return m_title; }

    /// Binds the engine to \a scene. Disconnects the previous scene if any.
    /// Pass nullptr to detach without binding a new scene.
    void setScene(Scene *scene);

    int  currentStep() const { return m_currentStep; }
    int  totalSteps()  const { return static_cast<int>(m_steps.size()); }
    bool isActive()    const { return m_active; }

    /// Returns the data for the current step. Caller must check isActive() first.
    const ExerciseStep &currentStepData() const;

    void start();
    void stop();
    void goToPreviousStep();

    /// Manually advances one step (used by the overlay Next/Finish button for observe steps).
    void advanceStep();

    /// Re-translates the title and all steps' instruction/hint text for the current
    /// language, preserving currentStep()/isActive(). No-op if never loaded or inactive.
    /// Emits retranslated() (never stepChanged()) so listeners can't replay step-entry
    /// side effects like MainWindow's click-target handling.
    void retranslate();

signals:
    void stepChanged(int step, int total, const ExerciseStep &data);
    void stepCompleted(int step);
    void exerciseCompleted();
    void exerciseStopped();
    void retranslated();

private slots:
    void onCircuitChanged();

private:
    /// Returns true when the current step's requirements are satisfied by the current scene.
    /// Always returns false for observe steps (both requirement vectors empty).
    bool validateCurrentStep() const;

    bool validateElements(const QVector<ExerciseElementRequirement> &reqs) const;
    bool validateConnections(const QVector<ExerciseConnectionRequirement> &reqs) const;

    void emitCurrentStep();
    void markCompleted();

    QString               m_id;
    QString               m_title;
    QString               m_resourcePath;
    QVector<ExerciseStep> m_steps;
    int                   m_currentStep = 0;
    bool                  m_active = false;
    QPointer<Scene>       m_scene;
};
