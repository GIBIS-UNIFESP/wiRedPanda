// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QVector>

#include "App/Tour/TourStep.h"

class TourEngine : public QObject
{
    Q_OBJECT

public:
    explicit TourEngine(QObject *parent = nullptr);

    /// Loads a tour from a Qt resource path (e.g. ":/Tours/ui-overview.json").
    /// Returns false and leaves the engine inactive if the resource is missing or malformed.
    bool loadFromResource(const QString &resourcePath);

    QString tourId()    const { return m_id; }
    QString tourTitle() const { return m_title; }

    int  currentStep() const { return m_currentStep; }
    int  totalSteps()  const { return static_cast<int>(m_steps.size()); }
    bool isActive()    const { return m_active; }

    /// Returns the data for the current step.
    const TourStep &currentStepData() const;

    void start();
    void stop();
    void goToPreviousStep();
    void advanceStep();

signals:
    void stepChanged(int step, int total, const TourStep &data);
    void tourCompleted();
    void tourStopped();

private:
    void emitCurrentStep();
    void markCompleted();

    QString           m_id;
    QString           m_title;
    QVector<TourStep> m_steps;
    int               m_currentStep = 0;
    bool              m_active = false;
};
