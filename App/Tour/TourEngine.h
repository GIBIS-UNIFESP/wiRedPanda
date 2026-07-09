// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

#include "App/Core/StepEngineCore.h"
#include "App/Tour/TourStep.h"

class TourEngine : public QObject
{
    Q_OBJECT

public:
    explicit TourEngine(QObject *parent = nullptr);

    /// Loads a tour from a Qt resource path (e.g. ":/Tours/ui-overview.json").
    /// Returns false and leaves the engine inactive if the resource is missing or malformed.
    bool loadFromResource(const QString &resourcePath);

    QString tourId()    const { return m_core.id(); }
    QString tourTitle() const { return m_core.title(); }

    int  currentStep() const { return m_core.currentStep(); }
    int  totalSteps()  const { return m_core.totalSteps(); }
    bool isActive()    const { return m_core.isActive(); }

    /// Returns the data for the current step.
    const TourStep &currentStepData() const;

    void start();
    void stop();
    void goToPreviousStep();
    void advanceStep();

    /// Re-translates the title and all steps' title/body text for the current language,
    /// preserving currentStep()/isActive(). No-op if never loaded or inactive. Emits
    /// retranslated() (never stepChanged()) so listeners can't replay step-entry side
    /// effects like MainWindow's click-target handling.
    void retranslate();

signals:
    void stepChanged(int step, int total, const TourStep &data);
    void tourCompleted();
    void tourStopped();
    void retranslated();

private:
    void emitCurrentStep();
    void markCompleted();

    StepEngineCore<TourStep> m_core;
};
