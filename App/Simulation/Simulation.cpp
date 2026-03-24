// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Simulation/Simulation.h"

#include <QGraphicsView>

#include "App/Core/Common.h"
#include "App/Core/Priorities.h"
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
    // 1ms tick drives the simulation at ~1000 steps/second — fast enough for
    // human perception while keeping CPU load predictable.
    m_timer.setInterval(1ms);
    connect(&m_timer, &QTimer::timeout, this, &Simulation::update);
}

void Simulation::update()
{
    // Lazily build the simulation layer on the first tick after a restart so
    // that circuit edits made while stopped are always reflected when the
    // simulation resumes.
    if (!m_initialized && !initialize()) {
        return;
    }

    // Clock elements are the only truly time-driven components; all other logic
    // is combinational and responds immediately to their values.
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

    if (m_simHasFeedbackElements) {
        // Use iterative settling for circuits with feedback loops.
        updateWithIterativeSettling();
    } else {
        // Phase 2: update all logic elements in topologically sorted order so
        // every gate sees its inputs before computing its output.
        for (auto *element : std::as_const(m_sortedElements)) {
            if (element) {
                element->updateLogic();
            }
        }
    }

    // Phase 3: push computed logic values onto the wire (QNEOutputPort) visuals
    for (auto *connection : std::as_const(m_connections)) {
        updatePort(connection->startPort());
    }

    // Phase 4: refresh output element visuals (LEDs, buzzers, etc.) using their input ports
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
        port->setStatus(Status::Invalid);
        return;
    }

    port->setStatus(element->outputValue(port->index()) ? Status::Active : Status::Inactive);
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

    // Output elements (LEDs, buzzers) need an explicit repaint to show the new state.
    auto *elm = port->graphicElement();
    if (elm && elm->elementGroup() == ElementGroup::Output) {
        elm->refresh();
    }
}

void Simulation::restart()
{
    // Clearing the flag causes the next update() call to rebuild the entire
    // simulation graph, effectively resetting all logic state to its default.
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
    } else {
        // After a pause the wall clock has advanced, so clocks must be reset
        // to "now" — otherwise they would fire many missed ticks immediately.
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
    // 10 passes is enough for any realistic SR/JK feedback depth; circuits that
    // genuinely oscillate (e.g. a ring oscillator) won't converge and we log a
    // warning on the last iteration rather than looping forever.
    const int maxIterations = 10;

    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        // Update all logic elements
        for (auto *element : std::as_const(m_sortedElements)) {
            if (element) {
                element->clearOutputChanged();
                element->updateLogic();
            }
        }

        // Check for convergence: no element changed any output this pass.
        const bool converged = std::none_of(m_sortedElements.cbegin(), m_sortedElements.cend(),
            [](const auto *element) { return element && element->outputChanged(); });

        if (converged) {
            // Circuit has stabilized, no need for more iterations
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

    // Rebuild all categorised lists from scratch so stale pointers from
    // a previous circuit state don't linger after undo/redo or file load.
    m_convergenceWarned = false;
    m_clocks.clear();
    m_outputs.clear();
    m_inputs.clear();
    m_connections.clear();
    m_sortedElements.clear();

    QVector<GraphicElement *> elements;
    auto items = m_scene->items();

    // Sort items by position coordinates for consistent ordering between runs.
    // QGraphicsScene::items() returns items in an unspecified Z/stacking order;
    // stabilising on (Y, X) gives deterministic wire-update sequences across
    // sessions and makes test results reproducible.
    std::stable_sort(items.begin(), items.end(), [](const auto &a, const auto &b) {
        if (!a || !b) {
            return a != nullptr;
        }
        // Sort by Y coordinate first, then X coordinate for consistent 2D ordering
        if (qFuzzyCompare(a->y(), b->y())) {
            return a->x() < b->x();
        }
        return a->y() < b->y();
    });

    // A scene with only one item is the scene border/background rectangle;
    // there is no circuit yet, so building a simulation graph would be pointless.
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

