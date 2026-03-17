// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Simulation/Simulation.h"

#include <QGraphicsView>
#include <QSet>

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
    m_timer.setInterval(1ms);
    connect(&m_timer, &QTimer::timeout, this, &Simulation::update);
}

// ============================================================================
// Update: generate events, process delta cycles, refresh visuals
// ============================================================================

void Simulation::update()
{
    if (!m_initialized && !initialize()) {
        return;
    }

    // --- Generate events from clocks ---
    if (m_timer.isActive()) {
        const auto globalTime = std::chrono::steady_clock::now();
        for (auto *clock : std::as_const(m_clocks)) {
            if (clock) {
                clock->updateClock(globalTime);
                // If the clock's logic output changed, schedule its successors.
                if (clock->logic() && clock->logic()->outputChanged()) {
                    clock->logic()->clearOutputChanged();
                    m_pendingSchedule.append(clock->logic());
                }
            }
        }
    }

    // --- Generate events from user inputs ---
    for (auto *inputElm : std::as_const(m_inputs)) {
        if (inputElm) {
            inputElm->updateOutputs();
            if (inputElm->logic() && inputElm->logic()->outputChanged()) {
                inputElm->logic()->clearOutputChanged();
                m_pendingSchedule.append(inputElm->logic());
            }
        }
    }

    // --- Process all pending events (blocking FIFO) ---
    processEvents();

    // --- Refresh visuals ---
    refreshVisuals();
}

// ============================================================================
// Event-driven engine: blocking FIFO evaluation
// ============================================================================

void Simulation::processEvents()
{
    if (!m_elmMapping) {
        return;
    }

    // Seed the work queue: collect successors of all elements whose outputs
    // changed (from clock toggles and input updates).
    QVector<LogicElement *> queue;
    for (auto *source : std::as_const(m_pendingSchedule)) {
        for (int outIdx = 0; outIdx < source->outputSize(); ++outIdx) {
            for (const auto &succ : source->successors(outIdx)) {
                queue.append(succ.logic);
            }
        }
    }
    m_pendingSchedule.clear();

    // Blocking event-driven: process elements one at a time in FIFO order.
    // Each element evaluates and immediately updates its outputs (blocking).
    // If an output changed, its successors are appended to the back of the
    // queue.  FIFO ordering naturally breaks symmetry in feedback circuits —
    // whichever gate is scheduled first evaluates first and "wins."
    // Evaluation order breaks symmetry in cross-coupled feedback circuits.
    const int maxEvents = 100000;
    int processed = 0;

    while (!queue.isEmpty() && processed < maxEvents) {
        auto *elm = queue.takeFirst();
        if (!elm) {
            continue;
        }

        elm->clearOutputChanged();
        elm->updateLogic();
        ++processed;

        if (elm->outputChanged()) {
            for (int outIdx = 0; outIdx < elm->outputSize(); ++outIdx) {
                for (const auto &succ : elm->successors(outIdx)) {
                    queue.append(succ.logic);
                }
            }
        }
    }

    if (processed >= maxEvents && !m_convergenceWarned) {
        m_convergenceWarned = true;
        emit simulationWarning(tr("Warning: simulation event limit exceeded — possible oscillation."));
    }
}

// ============================================================================
// Visual refresh
// ============================================================================

void Simulation::refreshVisuals()
{
    for (auto *connection : std::as_const(m_connections)) {
        updatePort(connection->startPort());
    }

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
        port->setStatus(Status::Unknown);
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

// ============================================================================
// Control
// ============================================================================

void Simulation::restart()
{
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

// ============================================================================
// Initialization
// ============================================================================

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
    m_pendingSchedule.clear();

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

    qCDebug(two) << "Recreating mapping for simulation.";
    m_elmMapping = std::make_unique<ElementMapping>(elements);

    if (!m_elmMapping) {
        return false;
    }

    qCDebug(two) << "Sorting.";
    m_elmMapping->sort();

    m_initialized = true;

    // Schedule any logic elements whose outputs already differ from Unknown
    // (e.g. GND/VCC sources at all IC nesting levels).  They are normal graph
    // elements in m_logicElms, so processEvents() will propagate to their
    // successors on the first update() call.
    for (const auto &elm : m_elmMapping->logicElms()) {
        if (elm && elm->outputChanged()) {
            elm->clearOutputChanged();
            m_pendingSchedule.append(elm.get());
        }
    }

    qCDebug(zero) << "Finished simulation layer.";
    return true;
}
