// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Simulation/Simulation.h"

#include <QGraphicsView>

#include "App/Core/Common.h"
#include "App/Core/Priorities.h"
#include "App/Core/SentryHelpers.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/IC.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Scene.h"

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

    // Clock elements are the only truly time-driven components.
    if (m_timer.isActive()) {
        const auto globalTime = std::chrono::steady_clock::now();
        for (auto *clock : std::as_const(m_clocks)) {
            if (clock) {
                clock->updateClock(globalTime);
            }
        }
    }

    // Phase 1: propagate user-controlled inputs (switches, buttons, etc.)
    for (auto *inputElm : std::as_const(m_inputs)) {
        if (inputElm) {
            inputElm->updateOutputs();
        }
    }

    // Phase 2: update all GraphicElements in topological order
    if (m_simHasFeedbackElements) {
        updateWithIterativeSettling();
    } else {
        for (auto *element : std::as_const(m_sortedElements)) {
            if (element) {
                element->updateLogic();
            }
        }
    }

    // Phase 3: push computed values onto the wire (QNEOutputPort) visuals
    for (auto *connection : std::as_const(m_connections)) {
        updatePort(connection->startPort());
    }

    // Phase 4: refresh output element visuals (LEDs, buzzers, etc.)
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

    auto *element = port->graphicElement();
    if (!element) {
        port->setStatus(Status::Unknown);
        return;
    }

    port->setStatus(element->outputValue(port->index()));
}

void Simulation::updatePort(QNEInputPort *port)
{
    if (!port) {
        return;
    }

    const auto &conns = port->connections();
    const Status status = (!conns.isEmpty() && conns.first()->startPort())
                              ? conns.first()->startPort()->status()
                              : port->defaultValue();
    port->setStatus(status);

    auto *elm = port->graphicElement();
    if (elm && elm->elementGroup() == ElementGroup::Output) {
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

bool Simulation::isInFeedbackLoop(const GraphicElement *element) const
{
    return m_simFeedbackNodes.contains(element);
}

void Simulation::stop()
{
    sentryBreadcrumb("simulation", QStringLiteral("Simulation stopped"));
    m_timer.stop();
    if (m_scene) {
        m_scene->mute(true);
    }
}

void Simulation::start()
{
    sentryBreadcrumb("simulation", QStringLiteral("Simulation started"));
    qCDebug(zero) << "Starting simulation.";

    if (!m_initialized) {
        initialize();
    } else {
        const auto globalTime = std::chrono::steady_clock::now();
        for (auto *clock : std::as_const(m_clocks)) {
            if (clock) {
                clock->resetClock(globalTime);
            }
        }
    }

    m_timer.start();
    if (m_scene) {
        m_scene->mute(false);
    }
    qCDebug(zero) << "Simulation started.";
}

void Simulation::updateWithIterativeSettling()
{
    const int maxIterations = 10;

    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        for (auto *element : std::as_const(m_sortedElements)) {
            if (element) {
                element->clearOutputChanged();
                element->updateLogic();
            }
        }

        const bool converged = std::none_of(m_sortedElements.cbegin(), m_sortedElements.cend(),
            [](const auto *element) { return element && element->outputChanged(); });

        if (converged) {
            break;
        }

        // If we're on the last iteration without convergence, warn the user once.
        if (iteration == maxIterations - 1 && !m_convergenceWarned) {
            m_convergenceWarned = true;
            qDebug() << "Feedback circuit did not converge after" << maxIterations << "iterations";
            emit simulationWarning(tr("Warning: feedback circuit did not converge — the circuit may be oscillating."));
        }
    }
}

bool Simulation::initialize()
{
    if (!m_scene) {
        return false;
    }

    m_convergenceWarned = false;
    m_clocks.clear();
    m_outputs.clear();
    m_inputs.clear();
    m_connections.clear();
    m_sortedElements.clear();

    QVector<GraphicElement *> elements;
    auto items = m_scene->items();

    std::stable_sort(items.begin(), items.end(), [](const auto &a, const auto &b) {
        if (!a || !b) {
            return a != nullptr;
        }
        if (qFuzzyCompare(a->y(), b->y())) {
            return a->x() < b->x();
        }
        return a->y() < b->y();
    });

    if (items.size() == 1) {
        return false;
    }

    qCDebug(two) << "GENERATING SIMULATION LAYER.";

    const auto globalTime = std::chrono::steady_clock::now();

    for (auto *item : std::as_const(items)) {
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

    qCDebug(zero) << "Elements read: " << elements.size();

    if (elements.empty()) {
        return false;
    }

    // Initialize simulation vectors on all scene-level elements
    for (auto *elm : std::as_const(elements)) {
        elm->initSimulationVectors(elm->inputSize(), elm->outputSize());
    }

    // Build connection graph
    buildConnectionGraph(elements);
    connectWirelessElements(elements);

    // Initialize IC internal simulation graphs
    for (auto *elm : std::as_const(elements)) {
        if (elm->elementType() == ElementType::IC) {
            static_cast<IC *>(elm)->initializeSimulation();
        }
    }

    // Topological sort with feedback detection
    sortSimElements(elements);

    m_initialized = true;

    qCDebug(zero) << "Finished simulation layer.";
    return true;
}

// --- Simulation graph building ---

void Simulation::buildConnectionGraph(const QVector<GraphicElement *> &elements)
{
    for (auto *elm : std::as_const(elements)) {
        for (int i = 0; i < elm->inputSize(); ++i) {
            auto *inputPort = elm->inputPort(i);
            const auto &connections = inputPort->connections();

            if (connections.size() == 1) {
                auto *connection = connections.constFirst();
                if (!connection) {
                    continue;
                }
                if (auto *outputPort = connection->startPort()) {
                    auto *sourceElement = outputPort->graphicElement();
                    if (sourceElement) {
                        elm->connectPredecessor(i, sourceElement, outputPort->index());
                    }
                }
            }
        }
    }
}

void Simulation::connectWirelessElements(const QVector<GraphicElement *> &elements)
{
    // Build a map from channel label to the Tx node's GraphicElement.
    // If two Tx nodes share the same label the first registered wins and a warning is emitted.
    QHash<QString, GraphicElement *> txMap;
    for (auto *elm : std::as_const(elements)) {
        if (elm->wirelessMode() != WirelessMode::Tx || elm->label().isEmpty()) {
            continue;
        }
        if (txMap.contains(elm->label())) {
            qCWarning(zero) << "Duplicate wireless Tx label:" << elm->label() << "— second transmitter ignored.";
        } else {
            txMap.insert(elm->label(), elm);
        }
    }

    // Wire each Rx node's input to the matching Tx node's output.
    // connectPredecessor() overwrites whatever buildConnectionGraph() set,
    // so the topological sort will see the true wireless dependency.
    for (auto *elm : std::as_const(elements)) {
        if (elm->wirelessMode() != WirelessMode::Rx || elm->label().isEmpty()) {
            continue;
        }
        if (auto *txElement = txMap.value(elm->label(), nullptr)) {
            elm->connectPredecessor(0, txElement, 0);
        }
        // No matching Tx: buildConnectionGraph() already wired this port to its default,
        // so the element is valid and outputs its default status.
    }
}

void Simulation::sortSimElements(const QVector<GraphicElement *> &elements)
{
    QHash<GraphicElement *, QVector<GraphicElement *>> successors;

    for (auto *elm : std::as_const(elements)) {
        for (auto *outputPort : elm->outputs()) {
            for (auto *conn : outputPort->connections()) {
                if (auto *endPort = conn->endPort()) {
                    auto *successor = endPort->graphicElement();
                    if (successor && !successors[elm].contains(successor)) {
                        successors[elm].append(successor);
                    }
                }
            }
        }
    }

    // Add wireless Tx→Rx edges to the successor graph.
    // connectWirelessElements() already set predecessors for simulation input routing,
    // but those don't create QNEConnection objects, so the connection-walking loop above
    // doesn't see wireless dependencies.  We must add them explicitly here for correct
    // topological ordering.
    // NOTE: must match connectWirelessElements() — first Tx per label wins.
    QHash<QString, GraphicElement *> txMap;
    for (auto *elm : std::as_const(elements)) {
        if (elm->wirelessMode() == WirelessMode::Tx && !elm->label().isEmpty()) {
            if (!txMap.contains(elm->label())) {
                txMap.insert(elm->label(), elm);
            }
        }
    }
    for (auto *elm : std::as_const(elements)) {
        if (elm->wirelessMode() == WirelessMode::Rx && !elm->label().isEmpty()) {
            if (auto *tx = txMap.value(elm->label(), nullptr)) {
                if (!successors[tx].contains(elm)) {
                    successors[tx].append(elm);
                }
            }
        }
    }

    QVector<GraphicElement *> rawPtrs(elements);
    QHash<GraphicElement *, int> priorities;
    calculatePriorities(rawPtrs, successors, priorities);
    const auto feedbackElements = findFeedbackNodes(rawPtrs, successors);

    m_simPriorities.clear();
    m_simFeedbackNodes.clear();
    for (auto *elm : std::as_const(elements)) {
        m_simPriorities[elm] = priorities.value(elm, -1);
        if (feedbackElements.contains(elm)) {
            m_simFeedbackNodes.insert(elm);
        }
    }
    m_simHasFeedbackElements = !m_simFeedbackNodes.isEmpty();

    m_sortedElements = elements;
    std::stable_sort(m_sortedElements.begin(), m_sortedElements.end(),
        [this](const auto *a, const auto *b) {
            return m_simPriorities.value(a, -1) > m_simPriorities.value(b, -1);
        });
}

