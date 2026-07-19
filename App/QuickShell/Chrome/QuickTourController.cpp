// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/QuickShell/Chrome/QuickTourController.h"

#include <QDebug>

QuickTourController::QuickTourController(QObject *parent)
    : QObject(parent)
{
    connect(&m_engine, &TourEngine::stepChanged, this, &QuickTourController::onEngineStepChanged);
    connect(&m_engine, &TourEngine::tourCompleted, this, &QuickTourController::onTourFinished);
    connect(&m_engine, &TourEngine::tourStopped, this, &QuickTourController::onTourFinished);
    // retranslate() only ever fires while active, on the same step -- current/prevEnabled/
    // nextButtonText all recompute to the values they already had; only title/body/
    // stepCounterText's translated text actually changes. Reusing applyStepData() rather than a
    // narrower text-only path is simpler and correct either way.
    connect(&m_engine, &TourEngine::retranslated, this, [this] {
        applyStepData(m_engine.currentStep(), m_engine.totalSteps(), m_engine.currentStepData());
    });
}

QString QuickTourController::stepCounterText() const
{
    return tr("Step %1 of %2").arg(m_step + 1).arg(m_total);
}

bool QuickTourController::start(const QString &resourcePath)
{
    if (!m_engine.loadFromResource(resourcePath)) {
        qWarning() << "TourEngine: failed to load" << resourcePath;
        return false;
    }
    // loadFromResource() already rejected any resource with zero steps, so start() is
    // guaranteed to activate here -- mirrors StepEngineCore::start()'s own contract.
    m_engine.start();
    emit activeChanged();
    return true;
}

void QuickTourController::goToPreviousStep()
{
    m_engine.goToPreviousStep();
}

void QuickTourController::advanceStep()
{
    m_engine.advanceStep();
}

void QuickTourController::close()
{
    m_engine.stop();
}

void QuickTourController::applyStepData(int step, int total, const TourStep &data)
{
    m_step = step;
    m_total = total;
    m_title = data.title;
    m_body = data.body;
    m_currentTarget = data.target;
    m_prevEnabled = step > 0;
    m_nextButtonText = (step == total - 1) ? tr("Finish") : tr("Next →");
    emit stepChanged();
}

void QuickTourController::onEngineStepChanged(int step, int total, const TourStep &data)
{
    // Must emit before applyStepData()'s stepChanged() -- see clicksRequested()'s doc comment.
    emit clicksRequested(data.click);
    applyStepData(step, total, data);
}

void QuickTourController::onTourFinished()
{
    emit activeChanged();
}
