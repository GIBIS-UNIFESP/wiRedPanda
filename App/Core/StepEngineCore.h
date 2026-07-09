// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Shared step-navigation state machine for ExerciseEngine/TourEngine.
 */

#pragma once

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QVector>

#include "App/Core/ExerciseTourResources.h"

/**
 * \class StepEngineCore
 * \brief Owns the id/title/step-list/progress state machine shared by \c ExerciseEngine and
 * \c TourEngine, and the JSON resource-loading logic that builds it.
 *
 * \details Not a \c QObject: Qt's moc cannot process a templated \c QObject that declares
 * signals, so \c ExerciseEngine/\c TourEngine each own a \c StepEngineCore<StepType> member
 * and translate its return values into their own class-specific \c emit calls (different
 * \c stepChanged payload types, and \c ExerciseEngine's extra \c stepCompleted signal /
 * Scene-validation subsystem, which this core knows nothing about). \a StepType is
 * \c ExerciseStep or \c TourStep; \a fillStep and \a persistence are supplied once at
 * construction (see each engine's constructor) since they're fixed per concrete engine type,
 * not per call.
 */
template <typename StepType>
class StepEngineCore
{
public:
    /// Fills \a step's type-specific fields from \a stepObj (one entry of the JSON "steps"
    /// array); \a id is the resource's id, for building translation catalog keys.
    using FillStep = void (*)(const QString &id, const QJsonObject &stepObj, StepType &step);

    /// The four \c Settings::* functions each engine persists progress/completion through.
    struct Persistence {
        int (*getProgress)(const QString &);
        void (*setProgress)(const QString &, int);
        QStringList (*getCompleted)();
        void (*setCompleted)(const QStringList &);
    };

    /// Result of advance(): tells the owning engine what to emit next.
    struct AdvanceResult {
        bool advanced = false;   ///< emit stepChanged() (and stepCompleted(), if the caller has one).
        bool reachedEnd = false; ///< call the caller's markCompleted().
        int completedStep = -1;  ///< the step index that just finished; valid when advanced is true.
    };

    StepEngineCore(FillStep fillStep, Persistence persistence)
        : m_fillStep(fillStep)
        , m_persistence(persistence)
    {
    }

    /// Loads \a resourcePath (a Qt resource path) and rebuilds id/title/step list from it.
    /// Resets active/current-step state; the caller decides whether to (re)activate.
    bool loadFromResource(const QString &resourcePath)
    {
        m_active = false;
        m_steps.clear();
        m_currentStep = 0;
        m_id.clear();
        m_title.clear();
        m_resourcePath = resourcePath;

        QFile file(resourcePath);
        if (!file.open(QIODevice::ReadOnly)) {
            return false;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isNull() || !doc.isObject()) {
            return false;
        }

        const QJsonObject root = doc.object();
        m_id = root.value("id").toString();
        const QString rawTitle = root.value("title").toString();

        if (m_id.isEmpty() || rawTitle.isEmpty()) {
            return false;
        }

        m_title = ExerciseTourResources::translate(m_id + QStringLiteral(".title"), rawTitle);

        const QJsonArray stepsArray = root.value("steps").toArray();
        if (stepsArray.isEmpty()) {
            return false;
        }

        for (const QJsonValue &stepVal : stepsArray) {
            StepType step;
            m_fillStep(m_id, stepVal.toObject(), step);
            m_steps.append(step);
        }

        return true;
    }

    /// Activates the engine at its persisted progress (clamped to a valid step). No-op if
    /// there are no steps. Returns true if activation happened (caller should emit its
    /// stepChanged-equivalent signal).
    bool start()
    {
        if (m_steps.isEmpty()) {
            return false;
        }
        m_currentStep = m_persistence.getProgress(m_id);
        if (m_currentStep < 0 || m_currentStep >= m_steps.size()) {
            m_currentStep = 0;
        }
        m_active = true;
        return true;
    }

    /// Deactivates the engine. Returns true if it was active (caller should emit its stopped
    /// signal); false if it was already inactive.
    bool stop()
    {
        if (!m_active) {
            return false;
        }
        m_active = false;
        return true;
    }

    /// Steps back one, persisting the new position. Returns true if it moved (caller should
    /// emit its stepChanged-equivalent signal).
    bool goToPreviousStep()
    {
        if (!m_active || m_currentStep <= 0) {
            return false;
        }
        --m_currentStep;
        m_persistence.setProgress(m_id, m_currentStep);
        return true;
    }

    /// Advances one step, persisting the new position; see AdvanceResult for how the caller
    /// should react.
    AdvanceResult advance()
    {
        if (!m_active || m_steps.isEmpty()) {
            return {};
        }
        if (m_currentStep >= m_steps.size() - 1) {
            return {.reachedEnd = true};
        }
        const int completed = m_currentStep;
        ++m_currentStep;
        m_persistence.setProgress(m_id, m_currentStep);
        return {.advanced = true, .completedStep = completed};
    }

    /// Reloads the current resource (picking up any language change) while preserving the
    /// current step position and active state. No-op if never loaded or inactive. Returns
    /// true if it retranslated (caller should emit its retranslated signal).
    bool retranslate()
    {
        if (m_resourcePath.isEmpty() || !m_active) {
            return false;
        }
        const int savedStep = m_currentStep;

        if (!loadFromResource(m_resourcePath)) {
            return false;
        }

        m_currentStep = qBound(0, savedStep, static_cast<int>(m_steps.size()) - 1);
        m_active = true;
        return true;
    }

    /// Deactivates, records completion, and resets persisted progress. Caller still owns any
    /// Scene disconnection and its own completed signal.
    void markCompleted()
    {
        m_active = false;

        QStringList completed = m_persistence.getCompleted();
        if (!completed.contains(m_id)) {
            completed.append(m_id);
            m_persistence.setCompleted(completed);
        }
        m_persistence.setProgress(m_id, 0);
    }

    const StepType &currentStepData() const
    {
        static StepType empty;
        if (m_steps.isEmpty() || m_currentStep < 0 || m_currentStep >= m_steps.size()) {
            return empty;
        }
        return m_steps.at(m_currentStep);
    }

    QString id() const { return m_id; }
    QString title() const { return m_title; }
    int currentStep() const { return m_currentStep; }
    int totalSteps() const { return static_cast<int>(m_steps.size()); }
    bool isActive() const { return m_active; }

private:
    FillStep m_fillStep;
    Persistence m_persistence;

    QString m_id;
    QString m_title;
    QString m_resourcePath;
    QVector<StepType> m_steps;
    int m_currentStep = 0;
    bool m_active = false;
};
