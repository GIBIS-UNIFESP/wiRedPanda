// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/Simulation/Simulation.h"

#include <algorithm>

#include <QGraphicsView>
#include <QGuiApplication>
#include <QScreen>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/Priorities.h"
#include "App/Core/SimulationHost.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/IC.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"

using namespace std::chrono_literals;

Simulation::Simulation(SimulationHost *host, QObject *parent)
    : QObject(parent)
    , m_host(host)
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
            m_visualTickInterval = (std::max)(1, static_cast<int>(1000.0 / hz));
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
    // simulation resumes. Only attempt this once per restart(): a scene with
    // nothing to simulate yet (a freshly created, still-empty tab) can never
    // succeed until a structural edit calls restart() again, so retrying on
    // every single 1 ms tick forever would just busy-loop for no reason.
    if (!m_initialized) {
        if (!m_needsInitializeAttempt) {
            return;
        }
        m_needsInitializeAttempt = false;
        if (!initialize()) {
            return;
        }
    }

    // Bug 5 invariant: the H2 cluster fix established that m_initialized=true
    // implies the topology vectors reflect the current scene. Any future change
    // that touches m_initialized without rebuilding the vectors trips here in
    // debug/asan/ubsan builds — much earlier than a tick-time crash.
    Q_ASSERT(m_initialized);

    // Snapshot the topology vectors before iterating: if restart() is invoked
    // reentrantly while this tick is mid-flight, it clears and rebuilds
    // m_clocks/m_inputs/m_sequentialElements/m_sortedElements/m_outputs, which
    // would invalidate any in-flight range-for iterator over the members
    // themselves. Iterating local copies keeps this tick's iteration valid for
    // its remainder even if the members mutate underneath.
    const auto clocks = m_clocks;
    const auto inputs = m_inputs;
    const auto sequentialElements = m_sequentialElements;
    const auto elements = m_sortedElements;
    const auto outputs = m_outputs;

    // Clock elements are the only truly time-driven components; all other logic
    // is combinational and responds immediately to their values.
    if (m_timer.isActive()) {
        const auto globalTime = std::chrono::steady_clock::now();

        for (auto *clock : clocks) {
            if (clock) {
                clock->updateClock(globalTime);
            }
        }
    }

    // Phase 1: propagate user-controlled inputs (switches, buttons, etc.)
    for (auto *inputElm : inputs) {
        if (inputElm) {
            inputElm->updateOutputs();
        }
    }

    // Advance the visual throttle on every tick (skipped or not) so the phase 3-4
    // cadence stays time-based. Non-interactive callers (tests, BeWavedDolphin's
    // throttle disabler) flush on every tick, as before.
    const bool visualsDue = !(m_visualThrottleEnabled && Application::interactiveMode)
                            || (++m_visualTickCount >= m_visualTickInterval);
    if (visualsDue) {
        m_visualTickCount = 0;
    }

    // Skip provably-idle ticks. A completed sweep whose settle passes converged is a
    // fixed point of the deterministic element functions; it can only be left by a
    // clock flip, an input-element change (both flagged through setOutputValue()'s
    // change detection -- user toggles included, they write the same way), or a
    // structural edit (restart() clears m_atFixedPoint). Everything else recomputes
    // bit-identical outputs 1000x/s, which on large clocked circuits is almost every
    // tick. The flags are cleared as they are read so one flip triggers one sweep.
    bool sourceChanged = false;
    for (auto *clock : clocks) {
        if (clock && clock->outputChanged()) {
            sourceChanged = true;
            clock->clearOutputChanged();
        }
    }
    for (auto *inputElm : inputs) {
        if (inputElm && inputElm->outputChanged()) {
            sourceChanged = true;
            inputElm->clearOutputChanged();
        }
    }

    if (!sourceChanged && m_atFixedPoint) {
        if (visualsDue && m_visualsDirty) {
            pushVisualStatuses(elements, outputs);
            m_visualsDirty = false;
        }
        return;
    }

    // Phase 2: update all GraphicElements in topological order.
    //
    // Synchronous sequential elements (flip-flops, latches — ElementGroup::Memory,
    // collected across the whole IC hierarchy) get non-blocking semantics: their
    // outputs are staged during the pass and committed together afterwards, so any
    // combinational logic between them (gated clocks especially) reads the
    // pre-tick state. This matches real synchronous hardware and the exported
    // SystemVerilog's non-blocking (<=) model. Gate-built feedback latches are
    // ElementGroup::Gate, not Memory, so they are never deferred and settle
    // through the existing path unchanged.
    for (auto *element : sequentialElements) {
        if (element) {
            element->beginDeferredCommit();
        }
    }

    // A plain topological sweep reaches its fixed point in one pass by construction;
    // only feedback settling can fail to converge (oscillating circuits).
    bool sweepConverged = true;

    if (m_simHasFeedbackElements) {
        // Use iterative settling for circuits with feedback loops.
        sweepConverged = updateWithIterativeSettling(elements);
    } else {
        // Phase 2: update all logic elements in topologically sorted order so
        // every gate sees its inputs before computing its output.
        for (auto *element : elements) {
            if (element) {
                element->updateLogic();
            }
        }
    }

    // Publish every staged sequential output simultaneously (global commit).
    bool anySequentialChanged = false;
    for (auto *element : sequentialElements) {
        if (element) {
            element->clearOutputChanged();
            element->commitDeferredOutputs();
            if (element->outputChanged()) {
                anySequentialChanged = true;
            }
        }
    }

    // Post-edge settle (the second half of a clock cycle / a bounded delta
    // cycle): once flip-flops have committed, re-propagate their new outputs
    // through combinational logic and IC output boundaries so the end-of-tick
    // state is a true fixed point — and so asynchronous overrides (preset/clear)
    // surface immediately rather than lagging a tick behind. Sequential elements
    // are skipped here so they are not re-clocked. Only needed on ticks where an
    // edge or async override actually changed sequential state; steady-state
    // ticks pay nothing. Feed-forward logic settles in one topological sweep;
    // iterate only when combinational feedback is present.
    if (anySequentialChanged) {
        const int maxPasses = m_simHasFeedbackElements ? kMaxSettleIterations : 1;
        for (int pass = 0; pass < maxPasses; ++pass) {
            bool changed = false;
            for (auto *element : elements) {
                if (element && element->elementGroup() != ElementGroup::Memory) {
                    element->clearOutputChanged();
                    element->resettleCombinational();
                    changed = changed || element->outputChanged();
                }
            }
            if (!changed) {
                break;
            }
            if (pass == maxPasses - 1) {
                // Exhausted the budget while still changing: not a fixed point.
                sweepConverged = false;
            }
        }
    }

    // This sweep's result is a fixed point unless a settle pass ran out of budget while
    // still changing (an oscillating feedback circuit, which must keep sweeping).
    m_atFixedPoint = sweepConverged;
    m_visualsDirty = true;

    // Visual updates only need to run at display-refresh rate, not at simulation
    // rate (1000 Hz) — skipping most ticks avoids dirtying QGraphicsItems that would
    // be overwritten before the next repaint. In non-interactive (test) mode every
    // tick flushes so tests see immediate visual state after each step.
    if (visualsDue) {
        pushVisualStatuses(elements, outputs);
        m_visualsDirty = false;
    }
}

void Simulation::pushVisualStatuses(const QVector<GraphicElement *> &elements, const QVector<GraphicElement *> &outputs)
{
    // Phase 3: push computed logic values onto all output port visuals.
    // Iterating elements (not connections) ensures unconnected output ports
    // (e.g. -Q of a flip-flop with no wire attached) are also updated.
    // setStatus() fans out through any attached connections automatically.
    for (auto *element : elements) {
        if (element) {
            for (auto *outputPort : element->outputs()) {
                updatePort(outputPort);
            }
        }
    }

    // Phase 4: refresh output element visuals (LEDs, buzzers, etc.) using their input ports
    for (auto *outputElm : outputs) {
        if (outputElm) {
            for (auto *inputPort : outputElm->inputs()) {
                if (inputPort) {
                    updatePort(inputPort);
                }
            }
        }
    }
}

void Simulation::updatePort(OutputPort *port)
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

void Simulation::updatePort(InputPort *port)
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
    // Invalidate the cached topology. Clearing the flag alone is not
    // enough: update() iterates m_sortedElements/m_clocks/m_inputs/m_outputs
    // before a re-initialize can run (for instance when Application::notify()
    // spins a QMessageBox nested event loop), and any entry that refers to an
    // element we've already freed faults on its vtable read. Drop every
    // reference so the next tick's initialize() can rebuild them cleanly.
    m_initialized = false;
    // A structural edit means there may be something new to simulate -- allow
    // update() one more initialize() attempt even if a prior attempt (on an
    // empty scene) already gave up and stopped retrying.
    m_needsInitializeAttempt = true;
    // A structural edit invalidates the fixed point: the next tick must sweep, and its
    // visuals must flush even if the throttle boundary lands on a later skipped tick.
    m_atFixedPoint = false;
    m_visualsDirty = true;
    m_sortedElements.clear();
    m_sequentialElements.clear();
    m_clocks.clear();
    m_inputs.clear();
    m_outputs.clear();
    // Bug 4 postcondition: any future cached state added to Simulation must be
    // cleared above. This assert documents the invariant for future maintainers
    // and trips immediately if a new vector is forgotten.
    Q_ASSERT(!m_initialized);
    Q_ASSERT(m_sortedElements.isEmpty() && m_sequentialElements.isEmpty()
          && m_clocks.isEmpty() && m_inputs.isEmpty() && m_outputs.isEmpty());
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
    if (m_host) {
        m_host->setMuted(true);
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
    if (m_host) {
        m_host->setMuted(m_userMuted);
    }
    qCDebug(zero) << "Simulation started.";
}

void Simulation::setUserMuted(const bool muted)
{
    m_userMuted = muted;
    if (m_host) {
        m_host->setMuted(muted);
    }
}

bool Simulation::isUserMuted() const
{
    return m_userMuted;
}

bool Simulation::updateWithIterativeSettling(const QVector<GraphicElement *> &elements)
{
    const bool converged = iterativeSettle(elements);
    if (!converged && !m_convergenceWarned) {
        m_convergenceWarned = true;
        qDebug() << "Feedback circuit did not converge after 10 iterations";
        emit simulationWarning(tr("Warning: feedback circuit did not converge — the circuit may be oscillating."));
    }
    return converged;
}

bool Simulation::initialize()
{
    if (!m_host) {
        return false;
    }

    // Rebuild all categorised lists from scratch so stale pointers from
    // a previous circuit state don't linger after undo/redo or file load.
    m_convergenceWarned = false;
    m_clocks.clear();
    m_outputs.clear();
    m_inputs.clear();
    m_sortedElements.clear();
    m_sequentialElements.clear();

    QVector<GraphicElement *> elements;
    auto items = m_host->simulationItems();

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

    // Collect every synchronous sequential element (across the IC hierarchy) so
    // Phase 2 can give them non-blocking commit semantics.
    collectSequentialElements(elements);

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
                    if (successor) {
                        auto &vec = successors[elm];
                        if (!vec.contains(successor)) {
                            vec.append(successor);
                        }
                    }
                }
            }
        }
    }

    // Add wireless Tx→Rx edges.
    // connectWirelessElements() already set predecessors for simulation input routing,
    // but those don't create Connection objects, so the connection-walking loop above
    // doesn't see wireless dependencies.  We must add them explicitly here for correct
    // topological ordering.
    for (auto *elm : std::as_const(elements)) {
        if (elm->wirelessMode() == WirelessMode::Rx && !elm->label().isEmpty()) {
            if (auto *tx = txMap.value(elm->label(), nullptr)) {
                auto &txVec = successors[tx];
                if (!txVec.contains(elm)) {
                    txVec.append(elm);
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
            if (!element) {
                continue;
            }
            element->clearOutputChanged();
            element->updateLogic();
        }

        const bool converged = std::none_of(elements.cbegin(), elements.cend(),
            [](const auto *element) { return element && element->outputChanged(); });

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

void Simulation::collectSequentialElements(const QVector<GraphicElement *> &elements)
{
    for (auto *elm : std::as_const(elements)) {
        if (!elm) {
            continue;
        }
        if (elm->elementGroup() == ElementGroup::Memory) {
            m_sequentialElements.append(elm);
        }
        if (elm->elementType() == ElementType::IC) {
            collectSequentialElements(static_cast<IC *>(elm)->internalElements());
        }
    }
}
