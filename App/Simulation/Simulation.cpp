// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Simulation/Simulation.h"

#include <QGraphicsView>
#include <QGuiApplication>
#include <QScreen>

#include "App/Core/Application.h"
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
    // 1ms tick drives the simulation at ~1000 steps/second — fast enough for
    // human perception while keeping CPU load predictable.
    m_timer.setInterval(1ms);
    connect(&m_timer, &QTimer::timeout, this, &Simulation::update);

    // Derive the visual refresh interval from the monitor's refresh rate so
    // we match the display without wasting repaints.  Falls back to 60 Hz.
    if (auto *screen = QGuiApplication::primaryScreen()) {
        const qreal hz = screen->refreshRate();
        if (hz > 0) {
            m_visualTickInterval = qMax(1, static_cast<int>(1000.0 / hz));
        }
    }
}

void Simulation::setVisualThrottleEnabled(bool enabled)
{
    m_visualThrottleEnabled = enabled;
    if (enabled) {
        m_visualTickCount = 0; // start fresh so throttle resumes cleanly
    }
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

    // Phase 2: update all GraphicElements in topological order
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

    // Visual updates only need to run at display-refresh rate (~5 fps),
    // not at simulation rate (1000 Hz).  Skip phases 3-4 on most ticks
    // to avoid dirtying QGraphicsItems that will be overwritten before
    // the next repaint.  In non-interactive (test) mode, always update
    // so that tests see immediate visual state after each step.
    if (m_visualThrottleEnabled && Application::interactiveMode && ++m_visualTickCount < m_visualTickInterval) {
        return;
    }
    m_visualTickCount = 0;

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
        m_scene->mute(m_userMuted);
    }
    qCDebug(zero) << "Simulation started.";
}

void Simulation::setUserMuted(const bool muted)
{
    m_userMuted = muted;
    if (m_scene) {
        m_scene->mute(muted);
    }
}

bool Simulation::isUserMuted() const
{
    return m_userMuted;
}

void Simulation::updateWithIterativeSettling()
{
    if (!iterativeSettle(m_sortedElements) && !m_convergenceWarned) {
        m_convergenceWarned = true;
        qDebug() << "Feedback circuit did not converge after 10 iterations";
        emit simulationWarning(tr("Warning: feedback circuit did not converge — the circuit may be oscillating."));
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
    const auto txMap = buildTxMap(elements);

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
    }
}

QHash<QString, GraphicElement *> Simulation::buildTxMap(const QVector<GraphicElement *> &elements)
{
    QHash<QString, GraphicElement *> txMap;
    for (auto *elm : std::as_const(elements)) {
        if (elm->wirelessMode() == WirelessMode::Tx && !elm->label().isEmpty()) {
            if (!txMap.contains(elm->label())) {
                txMap.insert(elm->label(), elm);
            }
        }
    }
    return txMap;
}

QHash<GraphicElement *, QVector<GraphicElement *>> Simulation::buildSuccessorGraph(
    const QVector<GraphicElement *> &elements,
    const QHash<QString, GraphicElement *> &txMap)
{
    QHash<GraphicElement *, QVector<GraphicElement *>> successors;

    // Build successor edges from physical connections
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

    // Add wireless Tx→Rx edges.
    // connectWirelessElements() already set predecessors for simulation input routing,
    // but those don't create QNEConnection objects, so the connection-walking loop above
    // doesn't see wireless dependencies.  We must add them explicitly here for correct
    // topological ordering.
    for (auto *elm : std::as_const(elements)) {
        if (elm->wirelessMode() == WirelessMode::Rx && !elm->label().isEmpty()) {
            if (auto *tx = txMap.value(elm->label(), nullptr)) {
                if (!successors[tx].contains(elm)) {
                    successors[tx].append(elm);
                }
            }
        }
    }

    return successors;
}

Simulation::SortResult Simulation::topologicalSort(
    const QVector<GraphicElement *> &elements,
    const QHash<GraphicElement *, QVector<GraphicElement *>> &successors)
{
    SortResult result;

    QVector<GraphicElement *> rawPtrs(elements);
    calculatePriorities(rawPtrs, successors, result.priorities);
    result.feedbackNodes = findFeedbackNodes(rawPtrs, successors);

    result.sorted = elements;
    std::stable_sort(result.sorted.begin(), result.sorted.end(),
        [&result](const auto *a, const auto *b) {
            return result.priorities.value(const_cast<GraphicElement *>(a), -1)
                 > result.priorities.value(const_cast<GraphicElement *>(b), -1);
        });

    return result;
}

bool Simulation::iterativeSettle(const QVector<GraphicElement *> &elements, const int maxIterations)
{
    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        for (auto *element : std::as_const(elements)) {
            element->clearOutputChanged();
            element->updateLogic();
        }

        const bool converged = std::none_of(elements.cbegin(), elements.cend(),
            [](const auto *element) { return element->outputChanged(); });

        if (converged) {
            return true;
        }
    }
    return false;
}

void Simulation::sortSimElements(const QVector<GraphicElement *> &elements)
{
    const auto txMap = buildTxMap(elements);
    const auto successors = buildSuccessorGraph(elements, txMap);
    const auto result = topologicalSort(elements, successors);

    m_simPriorities.clear();
    m_simFeedbackNodes.clear();
    for (auto *elm : std::as_const(elements)) {
        m_simPriorities[elm] = result.priorities.value(elm, -1);
        if (result.feedbackNodes.contains(elm)) {
            m_simFeedbackNodes.insert(elm);
        }
    }
    m_simHasFeedbackElements = !m_simFeedbackNodes.isEmpty();
    m_sortedElements = result.sorted;
}

