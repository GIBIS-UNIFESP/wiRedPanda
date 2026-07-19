// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Exercise/ExerciseEngine.h"

#include <QDebug>
#include <QPointer>

#include "App/Core/Settings.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Scene/Scene.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

namespace {

void fillExerciseStepFields(const QString &id, const QJsonObject &stepObj, ExerciseStep &step)
{
    const QString stepKey        = stepObj.value("key").toString();
    const QString rawInstruction = stepObj.value("instruction").toString();
    const QString rawHint        = stepObj.value("hint").toString();

    if (stepKey.isEmpty()) {
        step.instruction = rawInstruction;
        step.hint        = rawHint;
    } else {
        const QString keyPrefix = id + QLatin1Char('.') + stepKey + QLatin1Char('.');
        step.instruction = ExerciseTourResources::translate(keyPrefix + QStringLiteral("instruction"), rawInstruction);
        step.hint        = ExerciseTourResources::translate(keyPrefix + QStringLiteral("hint"), rawHint);
    }

    for (const QJsonValue &elemVal : stepObj.value("requiredElements").toArray()) {
        const QJsonObject elemObj = elemVal.toObject();
        ExerciseElementRequirement req;
        req.typeName = elemObj.value("type").toString();
        req.minCount = qMax(0, elemObj.value("minCount").toInt(1));
        step.requiredElements.append(req);
    }

    for (const QJsonValue &connVal : stepObj.value("requiredConnections").toArray()) {
        const QJsonObject connObj = connVal.toObject();
        ExerciseConnectionRequirement req;
        req.fromTypeName = connObj.value("fromType").toString();
        req.toTypeName   = connObj.value("toType").toString();
        req.minCount     = qMax(0, connObj.value("minCount").toInt(1));
        step.requiredConnections.append(req);
    }

    step.click   = stepObj.value("click").toVariant().toStringList();
    step.context = stepObj.value("context").toString();
}

} // namespace

ExerciseEngine::ExerciseEngine(QObject *parent)
    : QObject(parent)
    , m_core(&fillExerciseStepFields,
             {&Settings::exerciseProgress, &Settings::setExerciseProgress,
              &Settings::completedExercises, &Settings::setCompletedExercises})
{
}

bool ExerciseEngine::loadFromResource(const QString &resourcePath)
{
    return m_core.loadFromResource(resourcePath);
}

void ExerciseEngine::setScene(Scene *scene)
{
    if (m_disconnectFn) {
        m_disconnectFn();
    }
    if (scene) {
        // A local QPointer, captured by value into each closure below -- not a class member --
        // for the usual destroyed-out-from-under-us safety net without this class ever storing
        // Scene* itself (see m_connectFn's doc comment for why that matters).
        const QPointer<Scene> safeScene(scene);
        m_connectFn = [this, safeScene] {
            if (safeScene) {
                connect(safeScene, &Scene::circuitHasChanged, this, &ExerciseEngine::onCircuitChanged);
            }
        };
        m_disconnectFn = [this, safeScene] {
            if (safeScene) {
                disconnect(safeScene, &Scene::circuitHasChanged, this, &ExerciseEngine::onCircuitChanged);
            }
        };
        m_elementsFn = [safeScene] {
            return safeScene ? safeScene->elements() : QVector<GraphicElement *>{};
        };
    } else {
        m_connectFn = nullptr;
        m_disconnectFn = nullptr;
        m_elementsFn = nullptr;
    }
    if (scene && m_core.isActive()) {
        m_connectFn();
    }
}

QVector<GraphicElement *> ExerciseEngine::currentElements() const
{
    return m_elementsFn ? m_elementsFn() : QVector<GraphicElement *>{};
}

const ExerciseStep &ExerciseEngine::currentStepData() const
{
    return m_core.currentStepData();
}

void ExerciseEngine::start()
{
    if (!m_core.start()) {
        return;
    }
    if (m_connectFn) {
        m_connectFn();
    }
    emitCurrentStep();
}

void ExerciseEngine::stop()
{
    if (!m_core.stop()) {
        return;
    }
    if (m_disconnectFn) {
        m_disconnectFn();
    }
    emit exerciseStopped();
}

void ExerciseEngine::goToPreviousStep()
{
    if (m_core.goToPreviousStep()) {
        emitCurrentStep();
    }
}

void ExerciseEngine::performAdvance()
{
    const auto result = m_core.advance();
    if (result.reachedEnd) {
        markCompleted();
        return;
    }
    if (result.advanced) {
        emit stepCompleted(result.completedStep);
        emitCurrentStep();
    }
}

void ExerciseEngine::advanceStep()
{
    performAdvance();
}

void ExerciseEngine::retranslate()
{
    if (m_core.retranslate()) {
        emit retranslated();
    }
}

void ExerciseEngine::onCircuitChanged()
{
    if (!validateCurrentStep()) {
        return;
    }
    performAdvance();
}

bool ExerciseEngine::validateCurrentStep() const
{
    if (!m_core.isActive() || m_core.totalSteps() == 0 || !m_elementsFn) {
        return false;
    }
    const ExerciseStep &step = m_core.currentStepData();

    // Observe steps (both vectors empty) never auto-advance.
    if (step.requiredElements.isEmpty() && step.requiredConnections.isEmpty()) {
        return false;
    }

    return validateElements(step.requiredElements)
        && validateConnections(step.requiredConnections);
}

bool ExerciseEngine::validateElements(const QVector<ExerciseElementRequirement> &reqs) const
{
    if (reqs.isEmpty()) {
        return true;
    }

    const QVector<GraphicElement *> elements = currentElements();

    for (const ExerciseElementRequirement &req : reqs) {
        const ElementType required = ElementFactory::textToType(req.typeName);
        if (required == ElementType::Unknown) {
            qWarning() << "ExerciseEngine::validateElements: unknown element type" << req.typeName
                       << "in exercise" << m_core.id() << "-- this step can never be satisfied";
            return false;
        }
        int count = 0;
        for (const GraphicElement *elm : elements) {
            if (elm->elementType() == required) {
                ++count;
            }
        }
        if (count < req.minCount) {
            return false;
        }
    }
    return true;
}

bool ExerciseEngine::validateConnections(const QVector<ExerciseConnectionRequirement> &reqs) const
{
    if (reqs.isEmpty()) {
        return true;
    }

    const QVector<GraphicElement *> elements = currentElements();

    for (const ExerciseConnectionRequirement &req : reqs) {
        const ElementType toType   = ElementFactory::textToType(req.toTypeName);
        const ElementType fromType = ElementFactory::textToType(req.fromTypeName);
        if (toType == ElementType::Unknown || fromType == ElementType::Unknown) {
            qWarning() << "ExerciseEngine::validateConnections: unknown element type in"
                       << (toType == ElementType::Unknown ? req.toTypeName : req.fromTypeName)
                       << "in exercise" << m_core.id() << "-- this step can never be satisfied";
            return false;
        }

        int count = 0;
        for (const GraphicElement *elm : elements) {
            if (elm->elementType() != toType) {
                continue;
            }
            for (const InputPort *port : elm->inputs()) {
                for (const Connection *conn : port->connections()) {
                    if (conn->startPort()
                            && conn->startPort()->graphicElement()
                            && conn->startPort()->graphicElement()->elementType() == fromType) {
                        ++count;
                    }
                }
            }
        }
        if (count < req.minCount) {
            return false;
        }
    }
    return true;
}

void ExerciseEngine::emitCurrentStep()
{
    emit stepChanged(m_core.currentStep(), m_core.totalSteps(), currentStepData());
}

void ExerciseEngine::markCompleted()
{
    m_core.markCompleted();
    if (m_disconnectFn) {
        m_disconnectFn();
    }
    emit exerciseCompleted();
}
