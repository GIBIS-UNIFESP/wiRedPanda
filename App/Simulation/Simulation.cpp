// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Simulation/Simulation.h"

#include <QGraphicsView>

#include "App/Core/Common.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/IC.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/ElementMapping.h"

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

// ============================================================================
// Update dispatch
// ============================================================================

void Simulation::update()
{
    // Lazily build the simulation layer on the first tick after a restart so
    // that circuit edits made while stopped are always reflected when the
    // simulation resumes.
    if (!m_initialized && !initialize()) {
        return;
    }

    if (m_mode == SimulationMode::Temporal) {
        updateTemporal();
    } else {
        updateFunctional();
    }
}

// ============================================================================
// Functional mode (zero-delay, topological sort) — original behaviour
// ============================================================================

void Simulation::updateFunctional()
{
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

    if (m_hasFeedbackElements) {
        // Use iterative settling for circuits with feedback loops.
        // A single topological pass is insufficient when a gate's output feeds
        // back into an earlier stage; iterations continue until stable or the
        // cap is reached.
        updateWithIterativeSettling();
    } else {
        // Phase 2: update all logic elements in topologically sorted order so
        // every gate sees its inputs before computing its output.
        if (m_elmMapping) {
            for (auto &logic : m_elmMapping->logicElms()) {
                if (logic) {
                    logic->updateLogic();
                }
            }
        }
    }

    refreshVisuals();
}

// ============================================================================
// Temporal mode (event-driven with propagation delays)
// ============================================================================

void Simulation::updateTemporal()
{
    if (!m_elmMapping) {
        return;
    }

    const SimTime targetTime = m_currentTime + m_timePerTick;

    // Schedule clock edge events within this time window.
    for (auto *clock : std::as_const(m_clocks)) {
        if (clock) {
            clock->scheduleEdges(m_eventQueue, m_currentTime, targetTime);
        }
    }

    // Detect user input changes and inject immediate events.
    for (auto *inputElm : std::as_const(m_inputs)) {
        if (inputElm) {
            inputElm->scheduleIfChanged(m_eventQueue, m_currentTime);
        }
    }

    // Process events up to targetTime.
    const int maxDeltaCycles = 1000;
    int deltaCycles = 0;

    while (!m_eventQueue.empty() && m_eventQueue.nextTime() <= targetTime) {
        const SimTime eventTime = m_eventQueue.nextTime();
        m_currentTime = eventTime;

        // Process all events at this exact timestamp (one delta cycle).
        while (!m_eventQueue.empty() && m_eventQueue.nextTime() == eventTime) {
            auto event = m_eventQueue.pop();
            if (!event.target) {
                continue;
            }

            // Clock events toggle the source output directly since
            // LogicSource::updateLogic() would reset it to its initial value.
            if (event.target->propagationDelay() == 0 && event.target->outputSize() == 1
                && event.target->inputPairs().isEmpty()) {
                // This is a LogicSource (clock or constant) — toggle its output.
                const auto current = event.target->outputValue();
                event.target->setOutputValue(current == Status::Active ? Status::Inactive : Status::Active);
            } else {
                event.target->clearOutputChanged();
                event.target->updateLogic();
            }

            // Only propagate to successors if the element's output actually changed.
            if (event.target->outputChanged()) {
                scheduleSuccessors(event.target);
            }
        }

        // Record waveform transitions after each delta cycle settles.
        if (m_recorder.isRecording()) {
            m_recorder.recordAll(eventTime);
        }

        if (++deltaCycles > maxDeltaCycles) {
            if (!m_convergenceWarned) {
                m_convergenceWarned = true;
                qDebug() << "Temporal simulation: delta cycle limit exceeded at time" << m_currentTime << "ns";
                emit simulationWarning(tr("Warning: temporal simulation delta cycle limit exceeded — possible oscillation."));
            }
            break;
        }
    }

    m_currentTime = targetTime;

    // Sync clock graphic elements with their logic output so the icon updates.
    // In temporal mode the event loop toggles LogicSource directly, bypassing
    // Clock::setOn(), so m_isOn and the pixmap fall out of sync.
    for (auto *clock : std::as_const(m_clocks)) {
        if (clock && clock->logic()) {
            clock->setOn(clock->logic()->outputValue() == Status::Active);
        }
    }

    refreshVisuals();
}

void Simulation::scheduleSuccessors(LogicElement *source)
{
    for (int outIdx = 0; outIdx < source->outputSize(); ++outIdx) {
        for (const auto &succ : source->successors(outIdx)) {
            const SimTime delay = succ.logic->propagationDelay();
            m_eventQueue.schedule({m_currentTime + delay, succ.logic});
        }
    }
}

// ============================================================================
// Visual refresh (shared by both modes)
// ============================================================================

void Simulation::refreshVisuals()
{
    // Push computed logic values onto the wire (QNEOutputPort) visuals.
    for (auto *connection : std::as_const(m_connections)) {
        updatePort(connection->startPort());
    }

    // Refresh output element visuals (LEDs, buzzers, etc.) using their input ports.
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

// ============================================================================
// Port update helpers
// ============================================================================

void Simulation::updatePort(QNEOutputPort *port)
{
    if (!port) {
        return;
    }

    auto *logic = port->logic();
    if (!logic) {
        port->setStatus(Status::Invalid);
        return;
    }

    int outputIndex = port->logicIndex();
    port->setStatus(logic->outputValue(outputIndex));
}

void Simulation::updatePort(QNEInputPort *port)
{
    if (!port) {
        return;
    }

    // Phase 3 already set every QNEOutputPort's status from the logic values.
    // Propagate that already-computed status forward rather than re-traversing
    // the logic graph via logic->inputValue().
    // For unconnected optional ports, fall back to the port's declared default
    // (mirrors the globalVCC/globalGND predecessor set in ElementMapping::applyConnection).
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

// ============================================================================
// Iterative settling (functional mode, feedback loops only)
// ============================================================================

void Simulation::updateWithIterativeSettling()
{
    if (!m_elmMapping) {
        return;
    }

    // 10 passes is enough for any realistic SR/JK feedback depth; circuits that
    // genuinely oscillate (e.g. a ring oscillator) won't converge and we log a
    // warning on the last iteration rather than looping forever.
    const int maxIterations = 10;
    const auto &logicElements = m_elmMapping->logicElms();

    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        for (auto &logic : logicElements) {
            if (logic) {
                logic->clearOutputChanged();
                logic->updateLogic();
            }
        }

        // Check for convergence: no element changed any output this pass.
        const bool converged = std::none_of(logicElements.cbegin(), logicElements.cend(),
            [](const auto &logic) { return logic && logic->outputChanged(); });

        if (converged) {
            break;
        }

        if (iteration == maxIterations - 1 && !m_convergenceWarned) {
            m_convergenceWarned = true;
            qDebug() << "Feedback circuit did not converge after" << maxIterations << "iterations";
            emit simulationWarning(tr("Warning: feedback circuit did not converge — the circuit may be oscillating."));
        }
    }
}

// ============================================================================
// Mode control
// ============================================================================

void Simulation::setMode(SimulationMode mode)
{
    if (m_mode == mode) {
        return;
    }
    m_mode = mode;
    m_currentTime = 0;
    m_eventQueue.clear();
    restart();
}

void Simulation::restart()
{
    // Clearing the flag causes the next update() call to rebuild the entire
    // element mapping, effectively resetting all logic state to its default.
    m_initialized = false;
}

bool Simulation::isRunning()
{
    return m_timer.isActive();
}

bool Simulation::isInFeedbackLoop(const LogicElement *logic) const
{
    return m_elmMapping && m_elmMapping->isInFeedbackLoop(logic);
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
    } else if (m_mode == SimulationMode::Functional) {
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

// ============================================================================
// Initialization
// ============================================================================

bool Simulation::initialize()
{
    if (!m_scene) {
        return false;
    }

    // Rebuild all four categorised lists from scratch so stale pointers from
    // a previous circuit state don't linger after undo/redo or file load.
    m_convergenceWarned = false;
    m_clocks.clear();
    m_outputs.clear();
    m_inputs.clear();
    m_connections.clear();
    m_eventQueue.clear();

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
    // there is no circuit yet, so building an element mapping would be pointless.
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
                    if (m_mode == SimulationMode::Functional) {
                        clock->resetClock(globalTime);
                    } else {
                        clock->resetTemporalClock(m_currentTime);
                    }
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

    qCDebug(two) << "Recreating mapping for simulation.";
    // ElementMapping builds logic nodes and wires their predecessors; sort()
    // then assigns priorities and validates each node in topological order.
    m_elmMapping = std::make_unique<ElementMapping>(elements);

    if (!m_elmMapping) {
        return false;
    }

    qCDebug(two) << "Sorting.";
    m_elmMapping->sort();

    m_hasFeedbackElements = m_elmMapping->hasFeedbackElements();

    // In temporal mode, seed the event queue with initial input values so
    // the circuit settles to its initial state.
    if (m_mode == SimulationMode::Temporal) {
        for (auto *inputElm : std::as_const(m_inputs)) {
            if (inputElm) {
                inputElm->updateOutputs();
                if (inputElm->logic()) {
                    scheduleSuccessors(inputElm->logic());
                }
            }
        }
    }

    m_initialized = true;

    qCDebug(zero) << "Finished simulation layer.";
    return true;
}
