// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Tour/TourEngine.h"

#include "App/Core/ExerciseTourResources.h"
#include "App/Core/Settings.h"

namespace {

void fillTourStepFields(const QString &id, const QJsonObject &stepObj, TourStep &step)
{
    const QString stepKey = stepObj.value("key").toString();
    const QString rawStepTitle = stepObj.value("title").toString();
    const QString rawBody      = stepObj.value("body").toString();

    if (stepKey.isEmpty()) {
        step.title = rawStepTitle;
        step.body  = rawBody;
    } else {
        const QString keyPrefix = id + QLatin1Char('.') + stepKey + QLatin1Char('.');
        step.title = ExerciseTourResources::translate(keyPrefix + QStringLiteral("title"), rawStepTitle);
        step.body  = ExerciseTourResources::translate(keyPrefix + QStringLiteral("body"), rawBody);
    }

    step.target = stepObj.value("target").toString();
    step.click  = stepObj.value("click").toVariant().toStringList();
}

} // namespace

TourEngine::TourEngine(QObject *parent)
    : QObject(parent)
    , m_core(&fillTourStepFields,
             {&Settings::tourProgress, &Settings::setTourProgress,
              &Settings::completedTours, &Settings::setCompletedTours})
{
}

bool TourEngine::loadFromResource(const QString &resourcePath)
{
    return m_core.loadFromResource(resourcePath);
}

const TourStep &TourEngine::currentStepData() const
{
    return m_core.currentStepData();
}

void TourEngine::start()
{
    if (m_core.start()) {
        emitCurrentStep();
    }
}

void TourEngine::stop()
{
    if (m_core.stop()) {
        emit tourStopped();
    }
}

void TourEngine::goToPreviousStep()
{
    if (m_core.goToPreviousStep()) {
        emitCurrentStep();
    }
}

void TourEngine::advanceStep()
{
    const auto result = m_core.advance();
    if (result.reachedEnd) {
        markCompleted();
        return;
    }
    if (result.advanced) {
        emitCurrentStep();
    }
}

void TourEngine::retranslate()
{
    if (m_core.retranslate()) {
        emit retranslated();
    }
}

void TourEngine::emitCurrentStep()
{
    emit stepChanged(m_core.currentStep(), m_core.totalSteps(), currentStepData());
}

void TourEngine::markCompleted()
{
    m_core.markCompleted();
    emit tourCompleted();
}
