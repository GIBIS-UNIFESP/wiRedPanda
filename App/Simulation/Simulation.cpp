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

    auto *logic = port->logic();
    if (!logic) {
        port->setStatus(Status::Invalid);
        return;
    }

    int outputIndex = port->logicIndex();
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

    // Reflect the logic input value on the port so connection colors update correctly.
    port->setStatus(logic->isValid() ? static_cast<Status>(logic->inputValue(port->index())) : Status::Invalid);

    // Output elements (LEDs, buzzers) need an explicit repaint to show the new state.
    if (elm->elementGroup() == ElementGroup::Output) {
        elm->refresh();
    }
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
    if (!m_elmMapping) {
        return;
    }

    // 10 passes is enough for any realistic SR/JK feedback depth; circuits that
    // genuinely oscillate (e.g. a ring oscillator) won't converge and we log a
    // warning on the last iteration rather than looping forever.
    const int maxIterations = 10;
    const auto &logicElements = m_elmMapping->logicElms();

    // Snapshot outputs before the first pass so the convergence check on
    // iteration 0 has valid "previous" values to compare against.
    for (int i = 0; i < logicElements.size(); ++i) {
        auto &logic = logicElements[i];
        if (!logic) {
            continue;
        }
        for (int j = 0; j < m_previousOutputs[i].size(); ++j) {
            m_previousOutputs[i][j] = logic->outputValue(j);
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
                if (m_previousOutputs[i][j] != currentOutput) {
                    converged = false;
                    m_previousOutputs[i][j] = currentOutput;
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
    if (!m_scene) {
        return false;
    }

    // Rebuild all four categorised lists from scratch so stale pointers from
    // a previous circuit state don't linger after undo/redo or file load.
    m_clocks.clear();
    m_outputs.clear();
    m_inputs.clear();
    m_connections.clear();

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

    qCDebug(two) << "Recreating mapping for simulation.";
    // ElementMapping builds logic nodes and wires their predecessors; sort()
    // then assigns priorities and validates each node in topological order.
    m_elmMapping = std::make_unique<ElementMapping>(elements);

    if (!m_elmMapping) {
        return false;
    }

    qCDebug(two) << "Sorting.";
    m_elmMapping->sort();

    // Cache feedback flag and pre-allocate previousOutputs for iterative settling
    const auto &logicElms = m_elmMapping->logicElms();
    m_hasFeedbackElements = std::any_of(logicElms.begin(), logicElms.end(), [](const auto &logic) {
        return logic && logic->inFeedbackLoop();
    });

    m_previousOutputs.resize(logicElms.size());
    for (int i = 0; i < logicElms.size(); ++i) {
        if (logicElms[i]) {
            m_previousOutputs[i].resize(logicElms[i]->outputSize());
        }
    }

    m_initialized = true;

    qCDebug(zero) << "Finished simulation layer.";
    return true;
}

