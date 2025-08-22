// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "simulation.h"

#include "clock.h"
#include "common.h"
#include "elementmapping.h"
#include "graphicelement.h"
#include "ic.h"
#include "node.h"
#include "qneconnection.h"
#include "scene.h"
#include "wirelessconnectionmanager.h"

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

    for (auto *inputElm : std::as_const(m_inputs)) {
        inputElm->updateOutputs();
    }

    // Check if we have elements in feedback loops that need iterative settling
    bool hasFeedbackElements = false;
    for (auto &logic : m_elmMapping->logicElms()) {
        if (logic->inFeedbackLoop()) {
            hasFeedbackElements = true;
            break;
        }
    }

    if (hasFeedbackElements) {
        // Use iterative settling for circuits with feedback loops
        updateWithIterativeSettling();
    } else {
        // Standard single-pass update for purely combinational circuits
        for (auto &logic : m_elmMapping->logicElms()) {
            logic->updateLogic();
        }
    }

    // Update physical connections
    for (auto *connection : std::as_const(m_connections)) {
        updatePort(connection->startPort());
    }
    
    // Update wireless connections
    updateWirelessConnections();

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

void Simulation::updateWirelessConnections()
{
    if (!m_scene->wirelessManager()) {
        return;
    }

    // Get all active wireless groups
    const QStringList activeLabels = m_scene->wirelessManager()->getActiveLabels();
    
    for (const QString &label : activeLabels) {
        // Get all nodes in this wireless group
        const QSet<Node *> nodes = m_scene->wirelessManager()->getWirelessGroup(label);
        
        if (nodes.size() < 2) {
            continue; // Need at least 2 nodes for a connection
        }
        
        // Find signals from input nodes and detect contention (like physical connections)
        Status groupSignal = Status::Invalid;
        bool foundValidSignal = false;
        QSet<Status> uniqueSignals;
        
        for (auto *node : nodes) {
            if (!node || !node->logic()) {
                continue;
            }
            
            // Check if this node has an input signal
            const Status nodeSignal = static_cast<Status>(node->logic()->outputValue(0));
            
            if (node->logic()->isValid() && nodeSignal != Status::Invalid) {
                uniqueSignals.insert(nodeSignal);
                if (!foundValidSignal) {
                    groupSignal = nodeSignal;
                    foundValidSignal = true;
                }
            }
        }
        
        // Detect contention: if we have conflicting signals (both Active and Inactive)
        if (uniqueSignals.contains(Status::Active) && uniqueSignals.contains(Status::Inactive)) {
            groupSignal = Status::Invalid; // Contention detected - behave like physical connections
            foundValidSignal = true;
        }
        
        // Propagate the group signal to all nodes in the group
        if (foundValidSignal) {
            for (auto *node : nodes) {
                if (!node || !node->logic()) {
                    continue;
                }
                
                // Set the output of this node to match the group signal
                node->outputPort()->setStatus(groupSignal);
                // Also update the node's internal logic value for wireless signals
                if (node->logic()) {
                    if (groupSignal != Status::Invalid) {
                        node->logic()->setOutputValue(0, static_cast<bool>(groupSignal));
                    }
                    // Note: For Invalid status, we let the port status handle the invalid state
                    // The logic element's validity is managed by its normal validation process
                }
                
                // Update connected logic elements
                const auto connections = node->outputPort()->connections();
                for (auto *connection : connections) {
                    if (auto *inputPort = connection->endPort()) {
                        updatePort(inputPort);
                    }
                }
            }
        } else {
        }
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

void Simulation::updateWithIterativeSettling()
{
    const int maxIterations = 10; // Prevent infinite loops
    const auto &logicElements = m_elmMapping->logicElms();

    // Store previous output values for convergence detection
    QVector<QVector<bool>> previousOutputs(logicElements.size());

    for (int i = 0; i < logicElements.size(); ++i) {
        auto &logic = logicElements[i];
        // Initialize with current outputs
        previousOutputs[i].resize(logic->outputSize());
        for (int j = 0; j < previousOutputs[i].size(); ++j) {
            previousOutputs[i][j] = logic->outputValue(j);
        }
    }

    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        // Update all logic elements
        for (auto &logic : logicElements) {
            logic->updateLogic();
        }

        // Check for convergence (no output changes)
        bool converged = true;
        for (int i = 0; i < logicElements.size(); ++i) {
            auto &logic = logicElements[i];

            // Check all outputs for convergence
            for (int j = 0; j < logic->outputSize(); ++j) {
                bool currentOutput = logic->outputValue(j);
                if (previousOutputs[i][j] != currentOutput) {
                    converged = false;
                    previousOutputs[i][j] = currentOutput;
                }
            }
        }

        if (converged) {
            // Circuit has stabilized, no need for more iterations
            break;
        }

        // If we're on the last iteration without convergence, log a warning
        if (iteration == maxIterations - 1) {
            qDebug() << "Warning: Feedback circuit did not converge after" << maxIterations << "iterations";
        }
    }
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
