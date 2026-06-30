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
#include "App/Element/ElementFactory.h"
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
    // Guarded at the CONNECTION, not inside update(): update() is also public API called
    // directly (BeWavedDolphin's sweep, MCP, tests), and those callers must keep seeing an
    // exception rather than a silently swallowed tick. Only the timer-driven invocation
    // crosses Qt's signal-slot dispatch, which is where throwing is undefined behaviour --
    // on macOS the unwinder reaches std::terminate mid-stack and Application::notify's
    // backstop never runs (see Application::guardedSlot). update() runs arbitrary
    // updateLogic() code 1000x/s, so it is exactly the kind of body that doc means.
    connect(&m_timer, &QTimer::timeout, this, [this] {
        Application::guardedSlot(this, [this] { update(); });
    });

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

void Simulation::setElementDelay(const GraphicElement *element, const SimTime ns)
{
    if (!element || !element->hasPropagationDelay()) {
        return;
    }
    m_delays[element] = ns;
}

void Simulation::resetEventTracking()
{
    m_eventInitDone = false;
    m_atFixedPoint = false;  // a forced re-seed must not be skipped as "idle"
    m_eventQueue.clear();
    m_currentTime = 0;
}

void Simulation::update()
{
    // Lazily build the simulation layer on the first tick after a restart so
    // that circuit edits made while stopped are always reflected when the
    // simulation resumes.
    if (!m_initialized && !initialize()) {
        return;
    }

    // Invariant: m_initialized == true implies the topology vectors reflect the current
    // scene. Any change that touches m_initialized without rebuilding the vectors trips here
    // in debug/asan/ubsan builds — much earlier than a tick-time crash.
    Q_ASSERT(m_initialized);

    // Snapshot the topology vectors before iterating: if restart() is invoked
    // reentrantly while this tick is mid-flight, it clears and rebuilds
    // m_clocks/m_inputs/m_sortedElements/m_outputs, which
    // would invalidate any in-flight range-for iterator over the members
    // themselves. Iterating local copies keeps this tick's iteration valid for
    // its remainder even if the members mutate underneath.
    const auto clocks = m_clocks;
    const auto inputs = m_inputs;
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

    // Advance the visual throttle on every tick (skipped or not) so the phase 3-4 cadence stays
    // time-based. Non-interactive callers (tests, BeWavedDolphin's throttle disabler) flush on
    // every tick.
    const bool visualsDue = !(m_visualThrottleEnabled && Application::interactiveMode)
                            || (++m_visualTickCount >= m_visualTickInterval);
    if (visualsDue) {
        m_visualTickCount = 0;
    }

    // Skip provably-idle ticks. A drain that left the event queue empty is a fixed point of the
    // deterministic element functions: it can only be left by a clock flip, an input-element
    // change (both flagged through setOutputValue()'s change detection — user toggles included,
    // they write the same way), or a structural edit (restart()/initialize() clear the flag).
    // Everything else recomputes bit-identical outputs 1000x/s, which on large clocked circuits
    // is almost every tick.
    //
    // Unlike the pre-event-driven engine this mirrors, the flags are NOT cleared here: the
    // incremental seed inside processEvents() reads outputChanged() to decide which sources wake
    // their successors, and clearing it first would make the tick a no-op. seedChangedSource()
    // does the clearing, exactly once, when it consumes the change.
    bool sourceChanged = false;
    for (auto *clock : clocks) {
        if (clock && clock->outputChanged()) {
            sourceChanged = true;
            break;
        }
    }
    if (!sourceChanged) {
        for (auto *inputElm : inputs) {
            if (inputElm && inputElm->outputChanged()) {
                sourceChanged = true;
                break;
            }
        }
    }

    if (!sourceChanged && m_atFixedPoint) {
        // Sim-time still advances. processEvents() is what normally moves m_currentTime to the
        // tick boundary, so returning without it would freeze the temporal clock and any caller
        // ticking to reach a future timestamp would wait forever. Advancing here is sound
        // precisely because the queue is empty: there is nothing scheduled for the time being
        // skipped over, and no source changed to schedule anything.
        m_currentTime += m_timePerTick;

        if (visualsDue && m_visualsDirty) {
            pushVisualStatuses(elements, outputs);
            m_visualsDirty = false;
        }
        return;
    }

    // Phase 2: drain the event queue over this tick's window. Publish-side delay makes this a
    // single pass rather than a loop of delta-cycle "waves": an element evaluates the instant an
    // input it reads is published, and schedules its own output publication one propagation delay
    // later, so a ripple chain advances by element delays instead of one hop per tick. Sequential
    // commits are ordinary Publish events, so nothing needs re-propagating afterwards: there
    // is no begin/commit bracket and no post-commit resettle pass.
    // Drop the fixed-point conclusion BEFORE draining, not after. update() is a QTimer slot
    // that runs arbitrary updateLogic() implementations, Application::notify() catches what
    // escapes it, and the app keeps ticking -- so an exception mid-drain would otherwise leave
    // this holding the previous tick's `true` while the queue is non-empty, and every later
    // tick with no source change would be skipped as idle WITH EVENTS PENDING. Clearing on
    // entry makes any early exit leave the safe value.
    m_atFixedPoint = false;

    processEvents(m_currentTime + m_timePerTick, elements, inputs, clocks);

    // Mirror each IC's settled internal outputs onto its external output port values. This is
    // part of settling, not of drawing: the IC container is excluded from the flat netlist, so
    // nothing else writes ic->outputValue(), and leaving it inside the throttled visual push
    // meant MCP's get_output_value could read a value up to a full throttle interval stale.
    mirrorICOutputValues();

    // An empty queue means this tick settled everything it was given: nothing is pending at any
    // future timestamp, so with no source change the next tick would recompute the same values.
    // In temporal mode a drain that stops at the tick boundary leaves events queued and is
    // therefore NOT a fixed point. A canonicalised (frozen) oscillating region is one — not
    // because Unknown is idempotent under the region's own logic (it is not: AND(0, Unknown) is
    // Inactive, so a member with a dominating input would move if re-evaluated) but because
    // nothing reschedules a frozen member. Anything that starts rescheduling them has to
    // re-establish this on its own terms.
    m_atFixedPoint = m_eventQueue.empty();

    // Phases 1-2 above just ran, so a visual push is owed the next time visualsDue holds even if
    // this tick's own push is throttled away below.
    m_visualsDirty = true;

    if (!visualsDue) {
        return;
    }

    pushVisualStatuses(elements, outputs);
    m_visualsDirty = false;
}

void Simulation::pushVisualStatuses(const QVector<GraphicElement *> &elements,
                                    const QVector<GraphicElement *> &outputs)
{
    // The VALUES were already mirrored at the end of the drain (mirrorICOutputValues); this is
    // only the repaint, which is what the throttle is entitled to skip.
    pushICOutputVisuals();

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
    if (!element) { // LCOV_EXCL_LINE — every Port has a non-null owner set unconditionally by ElementPorts::addPort() at construction (the "port always has an owner" invariant established throughout this sweep).
        port->setStatus(Status::Unknown); // LCOV_EXCL_LINE — see above.
        return; // LCOV_EXCL_LINE — see above.
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
    m_sortedElements.clear();
    m_clocks.clear();
    m_inputs.clear();
    m_outputs.clear();
    m_successorGraph.clear(); // holds element pointers; must not outlive a rebuild
    m_icOutputMirror.clear(); // holds IC + boundary-node pointers; must not outlive a rebuild
    m_eventQueue.clear();
    m_atFixedPoint = false;   // a structural edit invalidates "nothing can change"
    m_delays.clear();         // keyed by element pointers; must not outlive a rebuild
    m_publishGeneration.clear();           // ditto: a reused address must not inherit a stamp
    // Keyed by element pointers too. sortSimElements() re-clears them on the next successful
    // initialize(), but if that never runs (e.g. the whole circuit was deleted, so initialize()
    // bails early), a freed element's reused address could inherit a stale priority or
    // feedback-node marking through schedule()/isInFeedbackLoop().
    m_simPriorities.clear();
    m_simFeedbackNodes.clear();
    // Same hazard, and the two component containers must go TOGETHER: canonicalizeOscillation()
    // indexes m_simFeedbackComponents with an id read out of m_simFeedbackComponent, so clearing
    // one alone turns a graceful "not in any cycle" decline into an out-of-bounds .at().
    m_simFeedbackComponent.clear();
    m_simFeedbackComponents.clear();
    m_simEvalCaps.clear();
    m_currentTime = 0;
    // Postcondition: any cached state added to Simulation must be cleared above. This assert
    // documents the invariant and trips immediately if a new container is forgotten.
    Q_ASSERT(!m_initialized);
    Q_ASSERT(m_sortedElements.isEmpty()
          && m_clocks.isEmpty() && m_inputs.isEmpty() && m_outputs.isEmpty()
          && m_successorGraph.isEmpty() && m_icOutputMirror.isEmpty()
          && m_simPriorities.isEmpty() && m_simFeedbackNodes.isEmpty()
          && m_simFeedbackComponent.isEmpty() && m_simFeedbackComponents.isEmpty()
          && m_simEvalCaps.isEmpty());
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
    // Record when the pause began (only when actually running) so start() can shift the
    // clocks' phase reference by the pause duration instead of resetting them.
    if (m_timer.isActive()) {
        m_pausedAt = std::chrono::steady_clock::now();
        m_hasPausedAt = true;
    }
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
    } else if (m_hasPausedAt) {
        // Resuming after a stop(): the wall clock advanced while paused, so shift each
        // clock's phase reference by the pause duration. This prevents the burst of missed
        // toggles a stale reference would cause, WITHOUT resetting the clocks — resetClock()
        // forces the output HIGH and restarts the phase, which would feed a spurious rising
        // edge into every clock-driven circuit on each SimulationBlocker cycle (every
        // UpdateCommand redo/undo, including a plain InputSwitch click). Level and phase
        // survive any pause/resume; only initialize() (Restart, rebuilds) gives clocks a
        // fresh HIGH start via resetClock().
        const auto pause = std::chrono::steady_clock::now() - m_pausedAt;
        for (auto *clock : std::as_const(m_clocks)) {
            if (clock) {
                clock->shiftClock(pause);
            }
        }
        m_hasPausedAt = false; // consumed; a repeated start() must not double-shift
    } else {
        // Initialized but with no recorded pause (e.g. initialize() ran directly via a
        // structural edit while stopped): the clocks' references are stale by an unknown
        // amount, so reset them to now.
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

void Simulation::processEvents(const SimTime targetTime,
                                const QVector<GraphicElement *> &elements,
                                const QVector<GraphicElementInput *> &inputs,
                                const QVector<Clock *> &clocks)
{
    // Event-driven, BLOCKING settle — one path for both modes, with PUBLISH-SIDE delay: an
    // element evaluates the instant an input it reads is published, stages the result, and
    // schedules its own publication one propagation delay later. Because it runs AT the
    // instant its input changed (rather than being woken late), it samples the value that
    // actually caused it; because only the newest pending publication survives (generation
    // supersession), a pulse narrower than the delay is absorbed. Functional mode is this same
    // drain with every delay forced to 0: all events land at the current instant (m_timePerTick
    // == 0 ⇒ targetTime == m_currentTime, no future timestamps) and it degenerates to the
    // classic zero-delay combinational/feedback settle.
    //
    // Each timestamp runs three regions — ACTIVE (combinational settles to a fixed point via
    // delta cycles), SAMPLE (Memory-group elements read their now-settled inputs and stage),
    // PUBLISH (staged values apply and wake readers). Sequential elements are held out of the
    // active region so a flip-flop never samples a cone that is still rippling, and they all
    // sample before any of them publishes, which is what makes the commit non-blocking.
    // Within the active region every evaluation reads only published values, so the outcome is
    // independent of evaluation order; the topological priority in the queue ordering is a
    // deterministic tie-break, not the correctness mechanism.
    //
    // The network is seeded whole once (first tick), then incrementally — only the sources that
    // changed (schedule-on-change). \a targetTime is supplied by the caller rather than derived
    // here, so a sweep can drain a window that is not the live one (BeWavedDolphin brackets its
    // sweep with begin/endTimedRun()).

    // Wakes every element that reads \a element, at \a at. The delay is NOT applied here:
    // under publish-side delay a successor evaluates immediately and pays its own delay when it
    // publishes, which is what lets it sample its inputs at the instant they actually changed.
    const auto wakeSuccessors = [this](GraphicElement *element, const SimTime at) {
        const auto it = m_successorGraph.constFind(element);
        if (it == m_successorGraph.constEnd()) {
            return;
        }
        for (auto *successor : *it) {
            if (successor) {
                scheduleEvaluate(at, successor);
            }
        }
    };

    // --- Seed ---
    if (!m_eventInitDone) {
        // Power-on baseline: settle the whole network at the current instant rather than
        // spreading it over max-path-delay, so t=0 presents a fully settled circuit.
        //
        // Same THREE REGIONS as the drain below, for the same reason. A single in-order pass
        // that commits each element as it goes gives blocking (`=`) semantics: stage 2 of a
        // shift register reads stage 1's just-committed Q instead of its pre-edge value, and
        // the input ripples through the whole register in one tick. This bites on every rebuild
        // because initialize() resets clocks HIGH while m_simLastClk starts Inactive -- so the
        // first tick after one presents a rising edge to every flip-flop.
        //
        // A forced re-seed (resetEventTracking(), beginTimedRun()) can arrive while elements
        // are mid-window, so drop any pending staged result first: re-seeding means recomputing
        // from live state, not resuming a half-finished publication.
        for (auto *element : elements) {
            if (element) {
                element->discardDeferredCommit();
            }
        }

        // Region 1 -- combinational, in topological order, committing as it goes.
        for (auto *element : elements) {
            if (!element || element->elementGroup() == ElementGroup::Memory) {
                continue;
            }
            element->beginDeferredCommit();
            element->clearOutputChanged();
            element->updateLogic();
            element->commitDeferredOutputs();
        }

        // Region 2 -- every Memory element samples the now-settled cone, staging only.
        for (auto *element : elements) {
            if (!element || element->elementGroup() != ElementGroup::Memory) {
                continue;
            }
            element->beginDeferredCommit();
            element->clearOutputChanged();
            element->updateLogic();
        }

        // Region 3 -- they all publish together, which is what makes the commit non-blocking.
        for (auto *element : elements) {
            if (!element || element->elementGroup() != ElementGroup::Memory) {
                continue;
            }
            element->commitDeferredOutputs();
        }

        for (auto *element : elements) {
            if (element && element->outputChanged()) {
                wakeSuccessors(element, m_currentTime);
            }
        }
        m_eventInitDone = true;
    } else {
        // Steady state: only sources that actually changed wake anything, and they wake their
        // successors NOW — the delay is paid on the successor's publication.
        const auto seedChangedSource = [&](GraphicElement *source) {
            if (!source || !source->outputChanged()) {
                return;
            }
            wakeSuccessors(source, m_currentTime);
            source->clearOutputChanged();
        };
        for (auto *inputElm : inputs) {
            seedChangedSource(inputElm);
        }
        for (auto *clock : clocks) {
            seedChangedSource(clock);
        }
    }

    // --- Drain ---
    // Two event kinds. Evaluate re-reads live inputs and stages the result; Publish applies a
    // staged result and wakes the readers. Publish is ordered after Evaluate at the same instant
    // (see SimEvent::operator>), so every element that reads a signal has already staged its
    // response before any staged value becomes visible — non-blocking semantics without a
    // separate commit bracket, and correct even when the delay is zero.
    // Per element, per timestamp — NOT one global budget. A global budget would make the frozen
    // value depend on the total element count, so adding a fully disconnected gate would change
    // a reading elsewhere. The bound comes from the element's own place in the SCC condensation
    // (computeEvaluationCaps()), so nothing outside its cone or component can move it.
    // Locals on purpose. Hoisting these to members to "avoid the per-tick construction" would
    // save nothing: QHash's default constructor is `= default` over a null d-pointer, so an
    // empty one costs no allocation, and QHash::clear() DELETES its data -- so a member cleared
    // each drain frees and reallocates exactly what a local does (checked against Qt 6.9.3's
    // qhash.h). QSet is QHash-backed and behaves identically. The only version that would
    // actually help is never clearing at all and letting entries expire by timestamp -- which
    // evalsAtTime's stamp already supports -- but that is an unmeasured change to the drain's
    // correctness loop, so it stays a local until something measures it.
    QHash<const GraphicElement *, QPair<SimTime, int>> evalsAtTime; // element -> (stamp, count)
    QSet<const GraphicElement *> frozen; // regions canonicalised at the current timestamp
    // Absolute backstop across ALL elements at one timestamp (see kMaxEvalsPerTimestampPerElement).
    // The per-element cap is local but multiplicative; this keeps total detection cost linear in
    // the netlist size.
    const qint64 evalCeiling =
        static_cast<qint64>(kMaxEvalsPerTimestampPerElement) * (std::max)(qsizetype{1}, elements.size());
    qint64 totalEvalsAtTime = 0;
    // Per-tick event budget; temporal mode only (see kMaxEventsPerTemporalTick).
    const bool budgeted = (m_timePerTick > 0);
    qint64 eventsThisTick = 0;
    bool budgetExhausted = false;
    SimTime lastCompletedTime = SIM_TIME_UNSET;
    SimTime lastTime = SIM_TIME_UNSET;
    QVector<GraphicElement *> pendingSamples; // Memory-group elements waiting to sample at `t`

    const auto canonicalizeOscillation = [this, &frozen](GraphicElement *tripped, const SimTime t) {
        // Only the component that is actually oscillating. The element whose cap tripped IS a
        // member of it, so no search is needed -- and an unrelated, settled latch elsewhere in
        // the circuit keeps its definite value. If another region is also oscillating it trips
        // on its own later; each trip freezes at least one component and there are finitely
        // many, so the drain still terminates.
        const int componentId = m_simFeedbackComponent.value(tripped, -1);
        const bool inCycle = (componentId >= 0 && componentId < m_simFeedbackComponents.size());

        if (!m_convergenceWarned) {
            m_convergenceWarned = true;
            // Name WHAT tripped, not just when. A timestamp alone cannot be acted on: it does
            // not say which of a few hundred elements hit its cap, nor which of possibly
            // several cyclic regions is the oscillating one, nor -- the case worth telling
            // apart -- that the cap misfired on acyclic logic and the drain is bailing out
            // rather than canonicalising. The user-facing string stays as it is; translating
            // an element name into it would invalidate every catalog for no gain a student
            // can use.
            const QString what = tripped
                ? QStringLiteral("%1 (%2)").arg(tripped->label().isEmpty()
                                                    ? QStringLiteral("<unlabelled>")
                                                    : tripped->label(),
                                                ElementFactory::typeToText(tripped->elementType()))
                : QStringLiteral("<null>");
            const QString where = inCycle
                ? QStringLiteral("cyclic component %1 (%2 members)")
                      .arg(componentId).arg(m_simFeedbackComponents.at(componentId).size())
                : QStringLiteral("no cyclic component -- the cap misfired on acyclic logic, "
                                 "so this timestamp is abandoned rather than canonicalised");
            // Deliberately the DEFAULT category, not qCWarning(zero): Comment::setVerbosity()
            // disables categories 0-5 entirely at the production default, warnings included, so
            // a categorised line would be invisible exactly where a bug report needs it.
            qWarning().noquote() << "Feedback circuit did not converge at time" << t
                            << "- evaluation cap tripped on" << what << "in" << where;
            emit simulationWarning(tr("Warning: feedback circuit did not converge — the circuit may be oscillating."));
        }

        if (!inCycle) {
            // Not in any cycle: the cap fired on acyclic logic, which can only be a false
            // positive. Canonicalising would be wrong, so stop draining this timestamp instead.
            return false;
        }

        // Whether the oscillation is IN the sequential elements themselves. A level-sensitive
        // DLatch/SRLatch has no edge detection, so a cycle made only of them really does
        // oscillate within a single timestamp, so skipping Memory unconditionally would leave
        // that case undetectable. Skip Memory only when a COMBINATIONAL member tripped, which is
        // the case worth protecting: an edge-triggered flip-flop caught inside someone else's
        // oscillating cone must not lose its stored state.
        const bool trippedIsMemory = (tripped->elementGroup() == ElementGroup::Memory);
        for (auto *member : std::as_const(m_simFeedbackComponents.at(componentId))) {
            if (!member || (member->elementGroup() == ElementGroup::Memory && !trippedIsMemory)) {
                continue;
            }
            // Write THROUGH a deferred-commit window rather than calling setOutputValue() bare.
            // Every element the drain has evaluated is still inside such a window awaiting its
            // Publish, and setOutputValue() routes into the staging buffer while one is open
            // (ElementSimState::setOutputValue) -- so a bare write would land in m_staged and be
            // dropped with the superseded Publish. begin/commit closes the window, publishes
            // Unknown for real, and sets outputChanged() so successors can be woken.
            member->beginDeferredCommit();
            for (int i = 0; i < member->outputSize(); ++i) {
                member->setOutputValue(i, Status::Unknown);
            }
            member->commitDeferredOutputs();
            ++m_publishGeneration[member]; // supersede any pending Publish
            frozen.insert(member);
        }

        // Propagate. The queue is NOT cleared: readers of the region have to learn that it went
        // undefined, and clearing it would leave a downstream gate holding a stale definite value
        // forever (the steady-state seed only wakes successors of changed inputs and clocks). The
        // freeze above is what makes this safe -- frozen members are skipped when popped, so the
        // region cannot regenerate same-timestamp events and re-trip.
        for (auto *member : std::as_const(m_simFeedbackComponents.at(componentId))) {
            if (!member || frozen.contains(member) == false) {
                continue;
            }
            const auto it = m_successorGraph.constFind(member);
            if (it == m_successorGraph.constEnd()) {
                continue;
            }
            for (auto *successor : *it) {
                if (successor && !frozen.contains(successor)) {
                    scheduleEvaluate(t, successor);
                }
            }
        }
        return true;
    };

    while (!m_eventQueue.empty() && m_eventQueue.nextTime() <= targetTime) {
        // Checked on the TIMESTAMP boundary, never inside one: a half-drained timestamp would
        // break the active/sample/publish split that makes the commit non-blocking.
        if (budgeted && eventsThisTick >= kMaxEventsPerTemporalTick
            && lastCompletedTime != SIM_TIME_UNSET) {
            budgetExhausted = true;
            break;
        }
        const SimTime t = m_eventQueue.nextTime();
        m_currentTime = t;
        if (t != lastTime) {
            // A new timestamp is a fresh budget and a fresh freeze. evalsAtTime is STAMPED
            // rather than cleared -- temporal mode visits many timestamps per tick, and clearing
            // a hash each time would put allocations in the drain's hot loop -- while the freeze
            // set and the running total, which are cheap, are simply reset here.
            frozen.clear();
            totalEvalsAtTime = 0;
            lastTime = t;
        }

        // --- Active region: settle combinational logic at `t` to a fixed point ---
        // Sequential elements are held back rather than evaluated here. A flip-flop must sample
        // its data input only once the cone feeding it has finished rippling at this instant;
        // sampling mid-ripple is what made an 8-bit program counter latch a half-propagated carry
        // (0xFF + 1 read as 0x1C). This is the active/NBA split proper, not just
        // Evaluate-before-Publish.
        bool overrun = false;
        GraphicElement *trippedElement = nullptr;
        while (!m_eventQueue.empty() && m_eventQueue.nextTime() == t) {
            const SimEvent event = m_eventQueue.pop();
            ++eventsThisTick;
            auto *element = event.target;
            if (!element) {
                continue;
            }

            if (event.kind == SimEventKind::Publish) {
                // A stale generation means a later evaluation superseded this publication: the
                // input moved again within the delay. Dropping it is inertial absorption.
                if (m_publishGeneration.value(element, 0) != event.generation) {
                    continue;
                }
                element->clearOutputChanged();
                element->commitDeferredOutputs();
                if (element->outputChanged()) {
                    wakeSuccessors(element, t);
                }
                continue;
            }

            if (element->elementGroup() == ElementGroup::Memory) {
                if (frozen.contains(element)) {
                    // Reachable since Memory-only regions can be canonicalised: a frozen latch
                    // must not re-enter pendingSamples and regenerate events at this timestamp.
                    continue;
                }
                if (!pendingSamples.contains(element)) {
                    pendingSamples.append(element); // sample after the active region settles
                }
                continue;
            }

            if (frozen.contains(element)) {
                continue; // canonicalised at this timestamp: it must not regenerate events
            }

            // Only reschedule when this evaluation actually changed what will be published.
            // Re-evaluating inside an unelapsed delay is routine (any input of the element
            // moving wakes it), and superseding the pending publish each time would drag the
            // transition out to `last-input-change + delay` instead of `cause + delay`.
            const bool windowWasOpen = element->isDeferCommitOpen();
            element->beginDeferredCommit();
            element->clearStagedChanged();
            element->updateLogic();
            if (element->stagedChanged() || !windowWasOpen) {
                schedulePublish(t + delayTo(element), element);
            }

            auto &counter = evalsAtTime[element];
            if (counter.first != t) {
                counter = {t, 0};
            }
            ++counter.second;
            ++totalEvalsAtTime;
            if (counter.second >= m_simEvalCaps.value(element, kEvalCapSlack)
                || totalEvalsAtTime >= evalCeiling) {
                trippedElement = element;
                overrun = true;
                break;
            }
        }

        if (overrun) {
            const bool canonicalised = canonicalizeOscillation(trippedElement, t);
            // Sample before leaving. Memory-group elements deferred into pendingSamples for this
            // timestamp have not run yet, and breaking straight out would drop their edge
            // entirely -- a flip-flop clocked on the same tick that an unrelated region trips the
            // cap would silently never capture. They sample the canonicalised (Unknown) values,
            // which is the correct reading of "the circuit did not settle".
            //
            // Published, not committed. Committing straight here would skip clearOutputChanged()
            // and wakeSuccessors(), so a flip-flop that DID capture would publish to nothing --
            // and if the queue then emptied, m_atFixedPoint would freeze the circuit with the new
            // value stranded. Going through the ordinary Publish event gets the wake, the delay
            // and the generation stamp for free; the drain continues below, so it fires.
            for (auto *element : std::as_const(pendingSamples)) {
                if (frozen.contains(element)) {
                    continue;
                }
                const bool windowWasOpen = element->isDeferCommitOpen();
                element->beginDeferredCommit();
                element->clearStagedChanged();
                element->updateLogic();
                if (element->stagedChanged() || !windowWasOpen) {
                    schedulePublish(t + delayTo(element), element);
                }
            }
            pendingSamples.clear();

            if (!canonicalised) {
                break; // false positive on acyclic logic: stop rather than spin
            }
            continue; // the region is frozen; keep draining so Unknown reaches its readers
        }

        // --- Sampling region: every sequential element reads its now-settled inputs, stages the
        // captured value, and schedules its own publication one propagation delay out. They all
        // sample before any of them publishes, which is what makes the commit non-blocking.
        //
        // Sampling is capped exactly like evaluation. Without this a cycle made only of
        // level-sensitive latches (a DLatch with ~Q wired to D and Enable high is enough) loops
        // between this region and the active region forever at one timestamp: with no counter
        // here, and with canonicalizeOscillation() skipping Memory, the drain would have no way
        // to stop.
        bool sampleOverrun = false;
        GraphicElement *sampleTripped = nullptr;
        for (auto *element : std::as_const(pendingSamples)) {
            auto &counter = evalsAtTime[element];
            if (counter.first != t) {
                counter = {t, 0};
            }
            ++counter.second;
            ++totalEvalsAtTime;
            if (!sampleOverrun
                && (counter.second >= m_simEvalCaps.value(element, kEvalCapSlack)
                    || totalEvalsAtTime >= evalCeiling)) {
                sampleTripped = element;
                sampleOverrun = true;
            }
            if (frozen.contains(element)) {
                continue; // canonicalised at this timestamp; it must not regenerate events
            }
            const bool windowWasOpen = element->isDeferCommitOpen();
            element->beginDeferredCommit();
            element->clearStagedChanged();
            element->updateLogic();
            if (element->stagedChanged() || !windowWasOpen) {
                schedulePublish(t + delayTo(element), element);
            }
        }
        pendingSamples.clear();
        lastCompletedTime = t;

        if (sampleOverrun) {
            // Every deferred sample above has already run, so no edge is dropped by tripping
            // here -- unlike the active region's break, this comes after the whole batch.
            if (!canonicalizeOscillation(sampleTripped, t)) {
                break; // false positive on acyclic logic: stop rather than spin
            }
            continue;
        }
    }

    // Time advances to the tick boundary (temporal) or stays 0 (functional) -- unless the tick
    // ran out of budget, in which case it advances only as far as the drain actually got, so the
    // next tick resumes from there rather than skipping over undrained events.
    m_currentTime = budgetExhausted ? lastCompletedTime : targetTime;
}

bool Simulation::initialize()
{
    if (!m_host) {
        return false;
    }

    // Rebuild all categorised lists from scratch so stale pointers from
    // a previous circuit state don't linger after undo/redo or file load.
    m_convergenceWarned = false;
    m_atFixedPoint = false;  // the rebuilt netlist must settle once before any tick is skipped
    m_eventInitDone = false; // first processEvents() after (re)init seeds the whole network
    // Same invariant restart() documents: topology-derived state must not outlive a
    // rebuild. Pending events are stale twice over — SimEvent::target raw pointers (a
    // structural command may have freed the element: delete, morph, split-undo) and
    // priorities baked from the OLD m_simPriorities at schedule time. In temporal mode
    // pending cross-tick events are routine, so without this clear the next drain pops
    // an event for a freed element (use-after-free). The full-network seed that
    // m_eventInitDone=false forces makes the dropped events redundant anyway.
    m_eventQueue.clear();
    m_clocks.clear();
    m_outputs.clear();
    m_inputs.clear();
    m_sortedElements.clear();

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
            if (!element) { // LCOV_EXCL_LINE — item->type() == GraphicElement::Type is GraphicElement's own hardcoded type() override, so the cast can never fail for an item reporting exactly that type.
                continue; // LCOV_EXCL_LINE — see above.
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

    // Every clock was just reset to "now" above, so any pause recorded by stop() is stale —
    // a later start() must not additionally shift the fresh references by the old pause.
    m_hasPausedAt = false;

    if (elements.empty()) {
        return false;
    }

    // Initialize simulation vectors on all scene-level elements.
    for (auto *elm : std::as_const(elements)) {
        elm->initSimulationVectors(elm->inputSize(), elm->outputSize());
    }

    // Build connection graph
    buildConnectionGraph(elements);
    connectWirelessElements(elements);

    // Wire each IC's internal primitives (connections + nested ICs). The IC itself does not
    // simulate; its internals join the top-level flat netlist below.
    for (auto *elm : std::as_const(elements)) {
        if (elm->elementType() == ElementType::IC) {
            static_cast<IC *>(elm)->initializeSimulation();
        }
    }

    // Flatten the IC hierarchy into one netlist of primitive elements: collect every primitive
    // (descending through ICs), splice each IC boundary so internal gates read external drivers
    // and external consumers read internal outputs, then seed per-element propagation delays so
    // temporal mode applies them uniformly — including inside (nested) ICs.
    QVector<GraphicElement *> flatElements;
    collectFlatElements(elements, flatElements);

    m_icOutputMirror.clear();
    spliceICBoundaries(elements);

    // Seed each flat element's propagation delay for the temporal engine. This is the baseline;
    // setElementDelay() may still override an entry at runtime.
    //
    // Types whose default is 0 are delay-free BY DESIGN -- sources, sinks, Nodes and ICs (an
    // IC's delay lives in the internals it was flattened into). Every writer already refuses to
    // set one on them: ElementEditor hides the field, UpdateCommand::refreshRuntimeState() skips
    // the push, and MCP set_property rejects it with a documented validation error. Reading the
    // resolved value ungated would let a hand-edited .panda inject a delay the engine honours
    // and no UI reveals. Read through the same predicate the writers use, so there is one rule
    // rather than a split.
    m_delays.clear();
    for (auto *elm : std::as_const(flatElements)) {
        m_delays[elm] = elm->hasPropagationDelay() ? elm->propagationDelay() : SimTime{0};
    }

    // Topological sort with feedback detection over the flat netlist
    sortSimElements(flatElements);

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
                if (!connection) { // LCOV_EXCL_LINE — Port::attachConnection() guards against a null argument (returns immediately without inserting), so m_connections can never contain a null entry.
                    continue; // LCOV_EXCL_LINE — see above.
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
} // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned QHash<QString, GraphicElement *>, never reached after the return above.

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
} // LCOV_EXCL_LINE — recurring pattern 1: compiler-generated cleanup for the returned QHash<GraphicElement *, QVector<GraphicElement *>>, never reached after the return above.

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

void Simulation::computeEvaluationCaps(const QVector<GraphicElement *> &elements)
{
    m_simEvalCaps.clear();

    // Longest path INTO each component of the condensation. Upstream, not downstream: what
    // bounds an element's re-evaluations is how many distinct arrival waves can reach it.
    // (m_simPriorities is the longest DOWNSTREAM path, and once feedback exists it is not even
    // that -- calculatePriorities() falls back to the legacy walk, which assigns feedback nodes
    // early. So it cannot be reused here.)
    const auto componentOf = [this](const GraphicElement *elm) {
        return m_simFeedbackComponent.value(elm, -1);
    };

    // Condensation node id: a component id, or a unique id per acyclic element.
    QHash<const GraphicElement *, int> nodeId;
    QVector<int> nodeSize;
    for (auto *elm : elements) {
        const int comp = componentOf(elm);
        if (comp >= 0) {
            nodeId[elm] = comp;
        } else {
            nodeId[elm] = static_cast<int>(m_simFeedbackComponents.size())
                        + static_cast<int>(nodeSize.size());
            nodeSize.append(1);
        }
    }
    const int nodeCount = static_cast<int>(m_simFeedbackComponents.size())
                        + static_cast<int>(nodeSize.size());

    // Longest-path-to-node over the condensation DAG, relaxed in the flat netlist's topological
    // order (m_sortedElements is sorted by priority, which is a valid topological order for the
    // acyclic part; edges inside a component are self-edges here and contribute nothing).
    //
    // The largest component reaching each node is propagated along the same edges. An element
    // DOWNSTREAM of an oscillating region is re-evaluated once per rotation of that region, so
    // its honest bound has to include the region's settling rounds too -- exactly as a member's
    // does. Crediting only members would make a consumer's cap SMALLER than the cap of the ring
    // feeding it (24 against 32 for a 3-gate ring with one driver and one sink, because the
    // driver adds a depth step to the ring but two to the sink). The consumer would then trip
    // first, canonicalizeOscillation() would find it in no cyclic component and correctly refuse
    // to freeze it, and the timestamp would be abandoned with NOTHING canonicalised -- leaving
    // the ring in a definite but self-inconsistent state (an inverter reading 0 and holding 0)
    // for the rest of the session. With the size credited, every consumer's cap strictly exceeds
    // that of the region driving it, so a member always trips first.
    // Relaxed in the condensation's OWN topological order (Kahn), not in m_sortedElements order.
    // A single pass over the priority order is only correct if that order happens to be
    // topological, and with a cycle present calculatePriorities() falls back to the legacy walk,
    // which is documented to assign feedback nodes early and can collapse a producer and its
    // consumer to the same priority. A node relaxed before one of its predecessors gets an
    // UNDER-estimated depth -- and since the cap is derived from it, that is precisely the
    // condition that lets a consumer's cap fall to or below the cap of the region driving it.
    // Deriving the order from the graph makes the property hold by construction rather than by
    // whatever the priority walk happens to produce.
    QVector<QVector<int>> condensation(nodeCount);
    QVector<int> indegree(nodeCount, 0);
    for (auto *elm : elements) {
        const auto it = m_successorGraph.constFind(elm);
        if (it == m_successorGraph.constEnd()) {
            continue;
        }
        const int from = nodeId.value(elm, -1);
        if (from < 0) {
            continue;
        }
        for (auto *successor : *it) {
            const int to = nodeId.value(successor, -1);
            if (to < 0 || to == from) {
                continue; // intra-component edge: not a depth step
            }
            // Parallel edges are kept rather than deduplicated: the indegree counts edge
            // INSTANCES and each instance is decremented exactly once, so the two stay
            // consistent, and relaxing the same edge twice is idempotent.
            condensation[from].append(to);
            ++indegree[to];
        }
    }

    QVector<int> depth(nodeCount, 0);
    QVector<int> maxComponent(nodeCount, 1);
    for (int c = 0; c < m_simFeedbackComponents.size(); ++c) {
        maxComponent[c] = static_cast<int>(m_simFeedbackComponents.at(c).size());
    }

    // Contracting the SCCs leaves a DAG, so every node reaches indegree 0 and is relaxed after
    // all of its predecessors. (Were that ever untrue, the unreached nodes would keep depth 0 --
    // an under-estimate, which is the safe direction: a cap too small trips early rather than
    // letting an oscillation run.)
    QVector<int> order;
    order.reserve(nodeCount);
    for (int node = 0; node < nodeCount; ++node) {
        if (indegree.at(node) == 0) {
            order.append(node);
        }
    }
    for (int head = 0; head < order.size(); ++head) {
        const int from = order.at(head);
        for (const int to : std::as_const(condensation.at(from))) {
            depth[to] = (std::max)(depth[to], depth[from] + 1);
            maxComponent[to] = (std::max)(maxComponent[to], maxComponent[from]);
            if (--indegree[to] == 0) {
                order.append(to);
            }
        }
    }

    for (auto *elm : elements) {
        const int node = nodeId.value(elm, 0);
        const int d = depth.value(node, 0);
        const int rounds = maxComponent.value(node, 1);
        m_simEvalCaps[elm] = kEvalCapSlack * (d + 1) * (rounds + 1);
    }
}

void Simulation::sortSimElements(const QVector<GraphicElement *> &elements)
{
    // Build the flat successor adjacency the same way buildSuccessorGraph() does — walking each
    // element's output connections in port/connection order — so the adjacency ORDER matches
    // buildSuccessorGraph()'s. That order matters: calculatePriorities() assigns a feedback
    // node's priority from its already-processed successors, so a different successor order would
    // change which feedback symmetry is broken first (e.g. gated-clock races inside ICs). IC
    // boundaries are mapped to their boundary Nodes so the graph is flat and edge-complete.
    QHash<GraphicElement *, QVector<GraphicElement *>> successors;
    const auto addEdge = [&successors](GraphicElement *src, GraphicElement *dst) {
        if (src && dst) {
            auto &succ = successors[src];
            if (!succ.contains(dst)) {
                succ.append(dst);
            }
        }
    };
    // Maps an input port to the primitive that actually consumes the signal: an IC input port
    // resolves to that IC's matching boundary input Node.
    const auto mapConsumer = [](InputPort *endPort) -> GraphicElement * {
        auto *consumer = endPort->graphicElement();
        if (consumer && consumer->elementType() == ElementType::IC) {
            auto *ic = static_cast<IC *>(consumer);
            const int idx = endPort->index();
            if (idx >= 0 && idx < ic->internalInputs().size()) {
                return ic->internalInputs().at(idx)->graphicElement();
            }
        }
        return consumer;
    };

    // A physical wire into an Rx node is DEAD once wireless has overridden it:
    // connectWirelessElements() repoints the Rx's predecessor at its Tx, so updateLogic() never
    // reads that wire again. Keeping the edge would leave m_successorGraph disagreeing with the
    // data flow it claims to mirror, and the disagreement is not cosmetic -- route the wire from
    // downstream of the Rx and the stale edge fabricates a cycle in an acyclic circuit, which
    // pushes calculatePriorities() onto the legacy walk for the entire circuit and makes the
    // phantom component a legitimate canonicalisation target.
    const auto edgeIsOverriddenByWireless = [](GraphicElement *src, GraphicElement *consumer) {
        return consumer && consumer->wirelessMode() == WirelessMode::Rx
            && consumer->simInputCount() > 0 && consumer->simPredecessor(0) != src;
    };

    // 1. Physical edges, in output-port/connection order (driver → consumer, IC inputs mapped to
    //    their boundary Nodes). Boundary input Nodes' internal fan-out is covered here too.
    for (auto *elm : std::as_const(elements)) {
        for (auto *outputPort : elm->outputs()) {
            for (auto *conn : outputPort->connections()) {
                if (auto *endPort = conn ? conn->endPort() : nullptr) {
                    auto *consumer = mapConsumer(endPort);
                    if (edgeIsOverriddenByWireless(elm, consumer)) {
                        continue;
                    }
                    addEdge(elm, consumer);
                }
            }
        }
    }
    // 2. Boundary output edges: a boundary output Node feeds the external consumers wired to its
    //    IC's output port (those connections live on the IC port, not the Node).
    for (const auto &m : std::as_const(m_icOutputMirror)) {
        for (auto *conn : m.ic->outputPort(m.outIndex)->connections()) {
            if (auto *endPort = conn ? conn->endPort() : nullptr) {
                addEdge(m.boundaryNode, mapConsumer(endPort));
            }
        }
    }
    // 3. Wireless Tx→Rx edges (label-matched, no Connection object), appended after physical edges to
    //    match buildSuccessorGraph()'s ordering. connectWirelessElements() set each Rx's
    //    predecessor to its Tx.
    for (auto *elm : std::as_const(elements)) {
        if (elm->wirelessMode() == WirelessMode::Rx && elm->simInputCount() > 0) {
            addEdge(elm->simPredecessor(0), elm);
        }
    }
    m_successorGraph = successors; // persist for the event-driven engine (processEvents)
    QVector<GraphicElement *> rawElements(elements);
    const auto result = topologicalSort(elements, successors);

    m_simPriorities.clear();
    m_simFeedbackNodes.clear();
    for (auto *elm : std::as_const(elements)) {
        m_simPriorities[elm] = result.priorities.value(elm, -1);
        if (result.feedbackNodes.contains(elm)) {
            m_simFeedbackNodes.insert(elm);
        }
    }

    // The cyclic components as a partition, not just a membership set: canonicalising an
    // oscillating region must leave every other cycle in the circuit alone.
    m_simFeedbackComponents = findFeedbackComponents(rawElements, successors);
    m_simFeedbackComponent.clear();
    for (int id = 0; id < m_simFeedbackComponents.size(); ++id) {
        for (auto *member : std::as_const(m_simFeedbackComponents.at(id))) {
            m_simFeedbackComponent[member] = id;
        }
    }

    m_sortedElements = result.sorted;
    computeEvaluationCaps(elements);
}

void Simulation::collectFlatElements(const QVector<GraphicElement *> &elements,
                                     QVector<GraphicElement *> &out)
{
    for (auto *elm : std::as_const(elements)) {
        if (!elm) {
            continue;
        }
        if (elm->elementType() == ElementType::IC) {
            // Descend into the IC's primitives (including its boundary Nodes); the IC container
            // node itself does not participate in simulation.
            collectFlatElements(static_cast<IC *>(elm)->internalElements(), out);
        } else {
            out.append(elm);
        }
    }
}

namespace {

/// Resolves \a startPort to its driving primitive (element, output index). If the driver is an
/// IC output port, maps it to that IC's matching boundary output Node — the real primitive that
/// carries the value in the flat netlist.
std::pair<GraphicElement *, int> resolveDriver(OutputPort *startPort)
{
    auto *element = startPort->graphicElement();
    if (element && element->elementType() == ElementType::IC) {
        auto *ic = static_cast<IC *>(element);
        const int idx = startPort->index();
        if (idx >= 0 && idx < ic->internalOutputs().size()) {
            auto *boundaryOut = ic->internalOutputs().at(idx);
            return {boundaryOut->graphicElement(), boundaryOut->index()};
        }
    }
    return {element, startPort->index()};
}

} // namespace

void Simulation::spliceICBoundaries(const QVector<GraphicElement *> &elements)
{
    for (auto *elm : std::as_const(elements)) {
        if (!elm || elm->elementType() != ElementType::IC) {
            continue;
        }
        auto *ic = static_cast<IC *>(elm);
        const auto &boundaryInputs = ic->internalInputs();
        const auto &boundaryOutputs = ic->internalOutputs();

        // Inputs: each boundary input Node reads the IC input port's external driver.
        for (int i = 0; i < ic->inputSize() && i < boundaryInputs.size(); ++i) {
            const auto &conns = ic->inputPort(i)->connections();
            if (conns.size() != 1 || !conns.constFirst()) {
                continue; // unconnected (or ambiguous) IC input — boundary Node keeps its default
            }
            auto *startPort = conns.constFirst()->startPort();
            if (!startPort) {
                continue;
            }
            const auto [srcElm, srcPort] = resolveDriver(startPort);
            auto *boundaryPort = boundaryInputs.at(i);
            if (srcElm && boundaryPort && boundaryPort->graphicElement()) {
                boundaryPort->graphicElement()->connectPredecessor(boundaryPort->index(), srcElm, srcPort);
            }
        }

        // Outputs: external consumers read the boundary output Node; record it for mirroring.
        for (int j = 0; j < ic->outputSize() && j < boundaryOutputs.size(); ++j) {
            auto *boundaryPort = boundaryOutputs.at(j);
            auto *boundaryNode = boundaryPort ? boundaryPort->graphicElement() : nullptr;
            if (!boundaryNode) {
                continue;
            }
            for (auto *conn : ic->outputPort(j)->connections()) {
                if (!conn) {
                    continue;
                }
                auto *endPort = conn->endPort();
                auto *consumer = endPort ? endPort->graphicElement() : nullptr;
                // IC consumers are repointed by their own input splice (resolveDriver maps this
                // IC output to its boundary Node), so skip them here.
                if (!consumer || consumer->elementType() == ElementType::IC) {
                    continue;
                }
                consumer->connectPredecessor(endPort->index(), boundaryNode, boundaryPort->index());
            }
            m_icOutputMirror.append({ic, j, boundaryNode});
        }

        // Recurse into nested ICs.
        spliceICBoundaries(ic->internalElements());
    }
}

void Simulation::mirrorICOutputValues()
{
    for (const auto &m : std::as_const(m_icOutputMirror)) {
        if (m.ic && m.boundaryNode) {
            // A bare setOutputValue() on the IC container, which is safe only because IC
            // containers are never evaluated and so never hold an open deferred-commit window.
            // If that ever changes this has to write through begin/commit like every other
            // out-of-drain writer.
            m.ic->setOutputValue(m.outIndex, m.boundaryNode->outputValue(0));
        }
    }
}

void Simulation::pushICOutputVisuals()
{
    for (const auto &m : std::as_const(m_icOutputMirror)) {
        if (m.ic) {
            updatePort(m.ic->outputPort(m.outIndex));
        }
    }
}
