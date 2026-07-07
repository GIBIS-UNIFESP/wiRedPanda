// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Exercise/ExerciseEngine.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "App/Core/ExerciseTourResources.h"
#include "App/Core/Settings.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Scene/Scene.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

ExerciseEngine::ExerciseEngine(QObject *parent)
    : QObject(parent)
{
}

bool ExerciseEngine::loadFromResource(const QString &resourcePath)
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
        ExerciseStep step;

        const QString stepKey       = stepObj.value("key").toString();
        const QString rawInstruction = stepObj.value("instruction").toString();
        const QString rawHint        = stepObj.value("hint").toString();

        if (stepKey.isEmpty()) {
            step.instruction = rawInstruction;
            step.hint        = rawHint;
        } else {
            const QString keyPrefix = m_id + QLatin1Char('.') + stepKey + QLatin1Char('.');
            step.instruction = ExerciseTourResources::translate(keyPrefix + QStringLiteral("instruction"), rawInstruction);
            step.hint        = ExerciseTourResources::translate(keyPrefix + QStringLiteral("hint"), rawHint);
        }

        for (const QJsonValue &elemVal : stepObj.value("requiredElements").toArray()) {
            const QJsonObject elemObj = elemVal.toObject();
            ExerciseElementRequirement req;
            req.typeName  = elemObj.value("type").toString();
            req.minCount  = elemObj.value("minCount").toInt(1);
            step.requiredElements.append(req);
        }

        for (const QJsonValue &connVal : stepObj.value("requiredConnections").toArray()) {
            const QJsonObject connObj = connVal.toObject();
            ExerciseConnectionRequirement req;
            req.fromTypeName = connObj.value("fromType").toString();
            req.toTypeName   = connObj.value("toType").toString();
            req.minCount     = connObj.value("minCount").toInt(1);
            step.requiredConnections.append(req);
        }

        step.click   = stepObj.value("click").toVariant().toStringList();
        step.context = stepObj.value("context").toString();
        m_steps.append(step);
    }

    return true;
}

void ExerciseEngine::setScene(Scene *scene)
{
    if (m_scene) {
        disconnect(m_scene, &Scene::circuitHasChanged, this, &ExerciseEngine::onCircuitChanged);
    }
    m_scene = scene;
    if (m_scene && m_active) {
        connect(m_scene, &Scene::circuitHasChanged, this, &ExerciseEngine::onCircuitChanged);
    }
}

const ExerciseStep &ExerciseEngine::currentStepData() const
{
    static ExerciseStep empty;
    if (m_steps.isEmpty() || m_currentStep < 0 || m_currentStep >= m_steps.size()) {
        return empty;
    }
    return m_steps.at(m_currentStep);
}

void ExerciseEngine::start()
{
    if (m_steps.isEmpty()) {
        return;
    }
    m_currentStep = Settings::exerciseProgress(m_id);
    if (m_currentStep < 0 || m_currentStep >= m_steps.size()) {
        m_currentStep = 0;
    }
    m_active = true;

    if (m_scene) {
        connect(m_scene, &Scene::circuitHasChanged, this, &ExerciseEngine::onCircuitChanged);
    }

    emitCurrentStep();
}

void ExerciseEngine::stop()
{
    if (!m_active) {
        return;
    }
    m_active = false;
    if (m_scene) {
        disconnect(m_scene, &Scene::circuitHasChanged, this, &ExerciseEngine::onCircuitChanged);
    }
    emit exerciseStopped();
}

void ExerciseEngine::goToPreviousStep()
{
    if (!m_active || m_currentStep <= 0) {
        return;
    }
    --m_currentStep;
    Settings::setExerciseProgress(m_id, m_currentStep);
    emitCurrentStep();
}

void ExerciseEngine::advanceStep()
{
    if (!m_active || m_steps.isEmpty()) {
        return;
    }
    if (m_currentStep >= m_steps.size() - 1) {
        markCompleted();
        return;
    }
    emit stepCompleted(m_currentStep);
    ++m_currentStep;
    Settings::setExerciseProgress(m_id, m_currentStep);
    emitCurrentStep();
}

void ExerciseEngine::retranslate()
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

void ExerciseEngine::onCircuitChanged()
{
    if (!m_active || !validateCurrentStep()) {
        return;
    }

    if (m_currentStep >= m_steps.size() - 1) {
        markCompleted();
        return;
    }

    emit stepCompleted(m_currentStep);
    ++m_currentStep;
    Settings::setExerciseProgress(m_id, m_currentStep);
    emitCurrentStep();
}

bool ExerciseEngine::validateCurrentStep() const
{
    if (!m_active || m_steps.isEmpty() || !m_scene) {
        return false;
    }
    const ExerciseStep &step = m_steps.at(m_currentStep);

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

    const QVector<GraphicElement *> elements = m_scene->elements();

    for (const ExerciseElementRequirement &req : reqs) {
        const ElementType required = ElementFactory::textToType(req.typeName);
        if (required == ElementType::Unknown) {
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

    const QVector<GraphicElement *> elements = m_scene->elements();

    for (const ExerciseConnectionRequirement &req : reqs) {
        const ElementType toType   = ElementFactory::textToType(req.toTypeName);
        const ElementType fromType = ElementFactory::textToType(req.fromTypeName);
        if (toType == ElementType::Unknown || fromType == ElementType::Unknown) {
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
    emit stepChanged(m_currentStep, static_cast<int>(m_steps.size()), currentStepData());
}

void ExerciseEngine::markCompleted()
{
    m_active = false;
    if (m_scene) {
        disconnect(m_scene, &Scene::circuitHasChanged, this, &ExerciseEngine::onCircuitChanged);
    }

    QStringList completed = Settings::completedExercises();
    if (!completed.contains(m_id)) {
        completed.append(m_id);
        Settings::setCompletedExercises(completed);
    }
    Settings::setExerciseProgress(m_id, 0);

    emit exerciseCompleted();
}
