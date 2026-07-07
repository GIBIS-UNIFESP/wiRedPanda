// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Tour/TourEngine.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "App/Core/Settings.h"
#include "App/Core/ExerciseTourResources.h"

TourEngine::TourEngine(QObject *parent)
    : QObject(parent)
{
}

bool TourEngine::loadFromResource(const QString &resourcePath)
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
        const QJsonObject stepObj = stepVal.toObject();
        TourStep step;

        const QString stepKey = stepObj.value("key").toString();
        const QString rawStepTitle = stepObj.value("title").toString();
        const QString rawBody      = stepObj.value("body").toString();

        if (stepKey.isEmpty()) {
            step.title = rawStepTitle;
            step.body  = rawBody;
        } else {
            const QString keyPrefix = m_id + QLatin1Char('.') + stepKey + QLatin1Char('.');
            step.title = ExerciseTourResources::translate(keyPrefix + QStringLiteral("title"), rawStepTitle);
            step.body  = ExerciseTourResources::translate(keyPrefix + QStringLiteral("body"), rawBody);
        }

        step.target = stepObj.value("target").toString();
        step.click  = stepObj.value("click").toVariant().toStringList();
        m_steps.append(step);
    }

    return true;
}

const TourStep &TourEngine::currentStepData() const
{
    static TourStep empty;
    if (m_steps.isEmpty() || m_currentStep < 0 || m_currentStep >= m_steps.size()) {
        return empty;
    }
    return m_steps.at(m_currentStep);
}

void TourEngine::start()
{
    if (m_steps.isEmpty()) {
        return;
    }
    m_currentStep = Settings::tourProgress(m_id);
    if (m_currentStep < 0 || m_currentStep >= m_steps.size()) {
        m_currentStep = 0;
    }
    m_active = true;
    emitCurrentStep();
}

void TourEngine::stop()
{
    if (!m_active) {
        return;
    }
    m_active = false;
    emit tourStopped();
}

void TourEngine::goToPreviousStep()
{
    if (!m_active || m_currentStep <= 0) {
        return;
    }
    --m_currentStep;
    Settings::setTourProgress(m_id, m_currentStep);
    emitCurrentStep();
}

void TourEngine::advanceStep()
{
    if (!m_active || m_steps.isEmpty()) {
        return;
    }
    if (m_currentStep >= m_steps.size() - 1) {
        markCompleted();
        return;
    }
    ++m_currentStep;
    Settings::setTourProgress(m_id, m_currentStep);
    emitCurrentStep();
}

void TourEngine::retranslate()
{
    if (m_resourcePath.isEmpty() || !m_active) {
        return;
    }
    const int savedStep = m_currentStep;

    if (!loadFromResource(m_resourcePath)) {
        return;
    }

    m_currentStep = qBound(0, savedStep, static_cast<int>(m_steps.size()) - 1);
    m_active = true;
    emit retranslated();
}

void TourEngine::emitCurrentStep()
{
    emit stepChanged(m_currentStep, static_cast<int>(m_steps.size()), currentStepData());
}

void TourEngine::markCompleted()
{
    m_active = false;

    QStringList completed = Settings::completedTours();
    if (!completed.contains(m_id)) {
        completed.append(m_id);
        Settings::setCompletedTours(completed);
    }
    Settings::setTourProgress(m_id, 0);

    emit tourCompleted();
}
