// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "simulation.h"

#include "clock.h"
#include "common.h"
#include "elementmapping.h"
#include "graphicelement.h"
#include "ic.h"

// Include flip-flop classes for sequential element detection
#include "logicelement/logicdflipflop.h"
#include "logicelement/logicjkflipflop.h"
#include "logicelement/logictflipflop.h"
#include "logicelement/logicsrflipflop.h"
#include "qneconnection.h"
#include "scene.h"

#include <QGraphicsView>

using namespace std::chrono_literals;

Simulation::Simulation(Scene *scene)
    : QObject(scene)
    , m_scene(scene)
{
    m_timer.setInterval(1ms);
    connect(&m_timer, &QTimer::timeout, this, &Simulation::update);
}

void Simulation::update()
{
    if (!m_initialized && !initialize()) {
        return;
    }

    if (m_timer.isActive()) {
        const auto globalTime = std::chrono::steady_clock::now();

        for (auto *clock : std::as_const(m_clocks)) {
            clock->updateClock(globalTime);
        }
    }

    // Update input elements
    for (auto *inputElm : std::as_const(m_inputs)) {
        inputElm->updateOutputs();
    }

    // Update combinational logic (single pass)
    updateCombinationalLogic();
    
    // Update feedback circuits with convergence
    updateFeedbackCircuitsWithConvergence();

    // Update connections and outputs
    for (auto *connection : std::as_const(m_connections)) {
        updatePort(connection->startPort());
    }

    for (auto *outputElm : std::as_const(m_outputs)) {
        for (auto *inputPort : outputElm->inputs()) {
            updatePort(inputPort);
        }
    }
}

void Simulation::updatePort(QNEOutputPort *port)
{
    if (!port) {
        return;
    }

    auto *elm = port->graphicElement();

    if (elm->elementType() == ElementType::IC) {
        auto *logic = qobject_cast<IC *>(elm)->outputLogic(port->index());
        port->setStatus(logic->isValid() ? static_cast<Status>(logic->outputValue(0)) : Status::Invalid);
    } else {
        auto *logic = elm->logic();
        port->setStatus(logic->isValid() ? static_cast<Status>(logic->outputValue(port->index())) : Status::Invalid);
    }
}

void Simulation::updatePort(QNEInputPort *port)
{
    auto *elm = port->graphicElement();
    auto *logic = elm->logic();

    port->setStatus(logic->isValid() ? static_cast<Status>(logic->inputValue(port->index())) : Status::Invalid);

    if (elm->elementGroup() == ElementGroup::Output) {
        elm->refresh();
    }
}

void Simulation::restart()
{
    m_initialized = false;
}

bool Simulation::isRunning()
{
    return m_timer.isActive();
}

void Simulation::stop()
{
    m_timer.stop();
    m_scene->mute(true);
}

void Simulation::start()
{
    qCDebug(zero) << "Starting simulation.";

    if (!m_initialized) {
        initialize();
    }

    m_timer.start();
    m_scene->mute(false);
    qCDebug(zero) << "Simulation started.";
}

bool Simulation::initialize()
{
    m_clocks.clear();
    m_outputs.clear();
    m_inputs.clear();
    m_connections.clear();

    QVector<GraphicElement *> elements;
    const auto items = m_scene->items();

    if (items.size() == 1) {
        return false;
    }

    qCDebug(two) << "GENERATING SIMULATION LAYER.";

    const auto globalTime = std::chrono::steady_clock::now();

    for (auto *item : items) {
        if (item->type() == QNEConnection::Type) {
            m_connections.append(qgraphicsitem_cast<QNEConnection *>(item));
        }

        if (item->type() == GraphicElement::Type) {
            auto *element = qgraphicsitem_cast<GraphicElement *>(item);
            elements.append(element);

            if (element->elementType() == ElementType::Clock) {
                m_clocks.append(qobject_cast<Clock *>(element));
                m_clocks.constLast()->resetClock(globalTime);
            }

            if (element->elementGroup() == ElementGroup::Input) {
                m_inputs.append(qobject_cast<GraphicElementInput *>(element));
            }

            if (element->elementGroup() == ElementGroup::Output) {
                m_outputs.append(element);
            }
        }
    }

    std::sort(elements.begin(), elements.end(), [](const auto &a, const auto &b) {
        return a->priority() > b->priority();
    });

    qCDebug(zero) << "Elements read: " << elements.size();

    if (elements.empty()) {
        return false;
    }

    qCDebug(two) << "Recreating mapping for simulation.";
    m_elmMapping = std::make_unique<ElementMapping>(elements);

    qCDebug(two) << "Sorting.";
    m_elmMapping->sort();

    m_initialized = true;

    qCDebug(zero) << "Finished simulation layer.";
    return true;
}

void Simulation::updateCombinationalLogic()
{
    // Update ALL logic elements (both combinational and sequential)
    // Sequential elements (flip-flops, latches) have proper edge detection to prevent multiple triggers
    for (auto &logic : m_elmMapping->logicElms()) {
        logic->updateLogic();
    }
}

void Simulation::updateFeedbackCircuitsWithConvergence()
{
    const auto &feedbackGroups = m_elmMapping->feedbackGroups();
    
    if (feedbackGroups.isEmpty()) {
        qDebug() << "CONVERGENCE DEBUG: No feedback groups detected";
        return;
    }
    
    qDebug() << "CONVERGENCE DEBUG: Processing" << feedbackGroups.size() << "feedback groups";
    
    for (int groupIdx = 0; groupIdx < feedbackGroups.size(); ++groupIdx) {
        const auto &feedbackGroup = feedbackGroups[groupIdx];
        if (feedbackGroup.isEmpty()) continue;
        
        qDebug() << "CONVERGENCE DEBUG: Group" << groupIdx << "has" << feedbackGroup.size() << "elements";
        
        // CRITICAL FIX: Filter out sequential elements from convergence
        // Sequential elements (flip-flops, latches) should only change on clock edges,
        // not during convergence iterations. Only combinational logic needs convergence.
        QVector<std::shared_ptr<LogicElement>> combinationalElements;
        QVector<std::shared_ptr<LogicElement>> sequentialElements;
        
        for (const auto &element : feedbackGroup) {
            // Check if element is sequential by attempting to cast to known sequential types
            bool isSequential = false;
            
            // Use dynamic_cast to identify sequential element types
            if (dynamic_cast<LogicDFlipFlop*>(element.get()) ||
                dynamic_cast<LogicJKFlipFlop*>(element.get()) ||
                dynamic_cast<LogicTFlipFlop*>(element.get()) ||
                dynamic_cast<LogicSRFlipFlop*>(element.get())) {
                isSequential = true;
            }
            
            if (isSequential) {
                sequentialElements.append(element);
            } else {
                combinationalElements.append(element);
            }
        }
        
        qDebug() << "CONVERGENCE DEBUG: Group" << groupIdx << "has" << combinationalElements.size() << "combinational and" << sequentialElements.size() << "sequential elements";
        
        // If the feedback group contains only sequential elements, skip convergence
        // Sequential elements in feedback (like Johnson Counter) are intentional and don't need convergence
        if (combinationalElements.isEmpty()) {
            qDebug() << "CONVERGENCE DEBUG: Group" << groupIdx << "contains only sequential elements - skipping convergence (intentional feedback)";
            continue;
        }
        
        bool converged = false;
        auto startTime = std::chrono::steady_clock::now();
        
        // Store initial outputs for combinational elements only
        QVector<QVector<bool>> previousOutputs;
        for (const auto &element : combinationalElements) {
            previousOutputs.append(element->getAllOutputValues());
        }
        
        for (int iteration = 0; iteration < MAX_CONVERGENCE_ITERATIONS && !converged; ++iteration) {
            qDebug() << "CONVERGENCE DEBUG: Group" << groupIdx << "iteration" << iteration + 1 << "(combinational only)";
            
            // Update only combinational elements during convergence
            for (const auto &element : combinationalElements) {
                element->updateLogic();
            }
            
            // Check if any combinational outputs changed
            converged = true;
            for (int i = 0; i < combinationalElements.size(); ++i) {
                if (combinationalElements[i]->hasOutputChanged(previousOutputs[i])) {
                    qDebug() << "CONVERGENCE DEBUG: Combinational element" << i << "output changed";
                    // Update previous outputs for next iteration
                    previousOutputs[i] = combinationalElements[i]->getAllOutputValues();
                    converged = false;
                }
            }
            
            if (converged) {
                qDebug() << "CONVERGENCE DEBUG: Group" << groupIdx << "converged in" << iteration + 1 << "iterations";
            }
            
            // Timeout check to prevent hanging
            auto elapsed = std::chrono::steady_clock::now() - startTime;
            if (elapsed > std::chrono::milliseconds(CONVERGENCE_TIMEOUT_MS)) {
                qDebug() << "CONVERGENCE DEBUG: Group" << groupIdx << "timeout after" << iteration + 1 << "iterations";
                break;
            }
        }
        
        if (!converged) {
            qDebug() << "CONVERGENCE DEBUG: Group" << groupIdx << "FAILED to converge in" << MAX_CONVERGENCE_ITERATIONS << "iterations";
        } else {
            qDebug() << "CONVERGENCE DEBUG: Group" << groupIdx << "converged successfully (combinational only)";
        }
    }
}

void Simulation::redetectFeedbackLoops()
{
    if (m_elmMapping) {
        qDebug() << "CONVERGENCE DEBUG: Manually re-detecting feedback loops";
        m_elmMapping->detectFeedbackLoops();
    }
}

ElementMapping* Simulation::elementMapping() const
{
    return m_elmMapping.get();
}
