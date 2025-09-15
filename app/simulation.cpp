// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "simulation.h"

#include "clock.h"
#include "common.h"
#include "elementmapping.h"
#include "graphicelement.h"
#include "ic.h"
#include "logicnode.h"
#include "node.h"
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
    // static int cycleCount = 0;

    if (!m_initialized && !initialize()) {
        return;
    }

    if (m_timer.isActive()) {
        const auto globalTime = std::chrono::steady_clock::now();

        for (auto *clock : std::as_const(m_clocks)) {
            if (clock) {
                clock->updateClock(globalTime);
            }
        }
    }

    for (auto *inputElm : std::as_const(m_inputs)) {
        if (inputElm) {
            inputElm->updateOutputs();
        }
    }

    // Check if we have elements in feedback loops that need iterative settling
    bool hasFeedbackElements = false;
    if (m_elmMapping) {
        for (auto &logic : m_elmMapping->logicElms()) {
            if (logic && logic->inFeedbackLoop()) {
                hasFeedbackElements = true;
                break;
            }
        }
    }

    if (hasFeedbackElements) {
        // Use iterative settling for circuits with feedback loops
        updateWithIterativeSettling();
    } else {
        // Standard single-pass update for purely combinational circuits
        if (m_elmMapping) {
            for (auto &logic : m_elmMapping->logicElms()) {
                if (logic) {
                    logic->updateLogic();
                }
            }
        }
    }

    // Update all connections - both PhysicalConnection and WirelessConnection objects
    // are processed identically since they both inherit from QNEConnection
    updateAllConnections();

    for (auto *outputElm : std::as_const(m_outputs)) {
        if (outputElm) {
            for (auto *inputPort : outputElm->inputs()) {
                if (inputPort) {
                    updatePort(inputPort);
                }
            }
        }
    }

}

void Simulation::updatePort(QNEOutputPort *port)
{
    if (!port) {
        return;
    }

    auto *elm = port->graphicElement();
    if (!elm) {
        return;
    }
    auto *logic = elm->getOutputLogic(port->index());
    if (!logic) {
        port->setStatus(Status::Invalid);
        return;
    }

    int outputIndex = elm->getOutputIndexForPort(port->index());
    port->setStatus(logic->isValid() ? static_cast<Status>(logic->outputValue(outputIndex)) : Status::Invalid);
}

void Simulation::updatePort(QNEInputPort *port)
{
    if (!port) {
        return;
    }

    auto *elm = port->graphicElement();
    if (!elm) {
        return;
    }
    auto *logic = elm->logic();
    if (!logic) {
        port->setStatus(Status::Invalid);
        return;
    }

    port->setStatus(logic->isValid() ? static_cast<Status>(logic->inputValue(port->index())) : Status::Invalid);

    if (elm->elementGroup() == ElementGroup::Output) {
        elm->refresh();
    }
}

void Simulation::updateAllConnections()
{
    // Update all connections (both PhysicalConnection and WirelessConnection)
    // Since WirelessConnection inherits from QNEConnection, they work identically
    for (auto *connection : std::as_const(m_connections)) {
        updatePort(connection->startPort());
    }

    // No special wireless handling needed! Perfect timing equivalence achieved
    // through unified connection model
}

// updateWirelessAsPhysical() method removed - no longer needed in Option D!
// Wireless connections are now actual QNEConnection objects that get processed
// automatically in updateAllConnections() with perfect timing equivalence.

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
    if (m_scene) {
        m_scene->mute(true);
    }
}

void Simulation::start()
{
    qCDebug(zero) << "Starting simulation.";

    if (!m_initialized) {
        initialize();
    }

    m_timer.start();
    if (m_scene) {
        m_scene->mute(false);
    }
    qCDebug(zero) << "Simulation started.";
}

void Simulation::updateWithIterativeSettling()
{
    if (!m_elmMapping) {
        return;
    }

    const int maxIterations = 10; // Prevent infinite loops
    const auto &logicElements = m_elmMapping->logicElms();

    // Store previous output values for convergence detection
    QVector<QVector<bool>> previousOutputs(logicElements.size());

    for (int i = 0; i < logicElements.size(); ++i) {
        auto &logic = logicElements[i];
        if (!logic) {
            continue;
        }
        // Initialize with current outputs
        previousOutputs[i].resize(logic->outputSize());
        for (int j = 0; j < previousOutputs[i].size(); ++j) {
            previousOutputs[i][j] = logic->outputValue(j);
        }
    }

    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        // Update all logic elements
        for (auto &logic : logicElements) {
            if (logic) {
                logic->updateLogic();
            }
        }

        // Check for convergence (no output changes)
        bool converged = true;
        for (int i = 0; i < logicElements.size(); ++i) {
            auto &logic = logicElements[i];
            if (!logic) {
                continue;
            }

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
        }
    }
}

bool Simulation::initialize()
{
    if (!m_scene) {
        return false;
    }

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
        // Check for all connection types - now all share QNEConnection::Type
        if (!item) {
            continue;
        }

        if (item->type() == QNEConnection::Type) {
            auto *connection = qgraphicsitem_cast<QNEConnection *>(item);
            if (connection) {
                m_connections.append(connection);
            }
        }

        if (item->type() == GraphicElement::Type) {
            auto *element = qgraphicsitem_cast<GraphicElement *>(item);
            if (!element) {
                continue;
            }
            elements.append(element);

            if (element->elementType() == ElementType::Clock) {
                auto *clock = qobject_cast<Clock *>(element);
                if (clock) {
                    m_clocks.append(clock);
                    clock->resetClock(globalTime);
                }
            }

            if (element->elementGroup() == ElementGroup::Input) {
                auto *input = qobject_cast<GraphicElementInput *>(element);
                if (input) {
                    m_inputs.append(input);
                }
            }

            if (element->elementGroup() == ElementGroup::Output) {
                m_outputs.append(element);
            }
        }
    }

    std::sort(elements.begin(), elements.end(), [](const auto &a, const auto &b) {
        if (!a || !b) {
            return a != nullptr; // Put null elements at the end
        }
        return a->priority() > b->priority();
    });

    qCDebug(zero) << "Elements read: " << elements.size();

    if (elements.empty()) {
        return false;
    }

    qCDebug(two) << "Recreating mapping for simulation.";
    m_elmMapping = std::make_unique<ElementMapping>(elements);

    if (!m_elmMapping) {
        return false;
    }

    qCDebug(two) << "Sorting.";
    m_elmMapping->sort();

    m_initialized = true;

    qCDebug(zero) << "Finished simulation layer.";
    return true;
}
