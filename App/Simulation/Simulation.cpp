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

void Simulation::resetEventTracking()
{
    // PURE tracking invalidation: the next update() reseeds the whole network instead of
    // trusting incremental change detection. The time-base and the recorder deliberately
    // stay untouched — they belong to whoever owns the timeline (the live session, or a
    // beginTimedRun()/endTimedRun() bracket around a sweep, which suspends recording and
    // restores the live clock so the recorded history survives).
    m_eventInitDone = false;
    m_eventQueue.clear();
}

void Simulation::update()
{
    // Lazily build the simulation layer on the first tick after a restart so
    // that circuit edits made while stopped are always reflected when the
    // simulation resumes.
    if (!m_initialized && !initialize()) {
        return;
    }

    // Bug 5 invariant: the H2 cluster fix established that m_initialized=true
    // implies the topology vectors reflect the current scene. Any future change
    // that touches m_initialized without rebuilding the vectors trips here in
    // debug/asan/ubsan builds — much earlier than a tick-time crash.
    Q_ASSERT(m_initialized);

    // Snapshot the topology vectors before iterating: if restart() is invoked
    // reentrantly while this tick is mid-flight, it clears and rebuilds
    // m_clocks/m_inputs/m_sequentialElements, which would invalidate any in-flight
    // range-for iterator over the members themselves. Iterating local copies keeps
    // this tick's iteration valid for its remainder even if the members mutate
    // underneath. m_sortedElements/m_outputs are no longer snapshotted here: Phase
    // 2's wave loop and Phase 3/4 (refreshVisuals()) now read them as members, via
    // primitives (beginWave()/processEvents()/finishWave()/refreshVisuals()) shared
    // with the step debugger, which drives them across separate calls where no
    // single-tick snapshot exists to hand them.
    const auto clocks = m_clocks;
    const auto inputs = m_inputs;
    const auto sequentialElements = m_sequentialElements;

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

    // Stamp input- and clock-driven edges at the tick boundary where they take effect.
    // The drain-side captures below only run at event timestamps, and a toggled input's
    // earliest downstream event is its first successor's evaluation at t + that gate's
    // delay — without this capture the input's edge and the first gate's edge share a
    // timestamp, so the first gate appears delay-free in the timing diagram. recordAll()
    // dedups, so steady ticks record nothing. Skipped on a reseed tick (m_eventInitDone
    // false): element outputs may hold unsettled values there (e.g. sweep-clobbered state
    // before the reseed corrects it) and snapshotting them would record phantom glitches.
    if (m_eventInitDone && m_recorder.isRecording()) {
        m_recorder.recordAll(m_currentTime);
    }

    // Phase 2: update all GraphicElements in topological order, in a bounded sequence of
    // "delta-cycle waves". A single begin/process/commit/resettle pass (the original design)
    // correctly gives synchronous sequential elements non-blocking semantics for the common
    // case (multiple flip-flops all clocked directly from the same source), but it silently
    // stops a flip-flop's committed change from ever reaching a SECOND flip-flop it feeds
    // directly (a ripple counter, clock divider, or any Clock/Data pin wired straight to
    // another flip-flop's Q, with no gate in between) — commitDeferredOutputs() runs once,
    // after processEvents() has already returned for the tick, and nothing afterwards
    // schedules that element's successors; the post-edge resettle below deliberately skips
    // ElementGroup::Memory elements so they are never re-clocked. Looping the whole sequence,
    // waking only sequential elements that have not yet run this tick, lets such a chain
    // "ripple" through in as many waves as it has stages, while a flip-flop already fired via
    // its own clock this tick is never re-invoked a second time (re-running one with an
    // unchanged clock is not idempotent for the value it captures for the *next* edge, so
    // re-triggering it here would corrupt ordinary same-clock-domain circuits).
    //
    // Bound: sequentialElements.size() + 1 waves. This is provable, not a heuristic — every
    // wave that makes progress permanently moves at least one not-yet-evaluated element into
    // m_evaluatedSequentialThisTick (it never shrinks), so after at most
    // sequentialElements.size() productive waves every sequential element has run at least
    // once; the following wave then finds nothing new to schedule and nothing changed, and
    // exits. Genuine oscillation cannot loop this: an oscillating feedback region is caught by
    // processEvents()'s own per-timestamp evaluation cap and canonicalized to Unknown, which
    // is idempotent from that point on.
    // Advance the visual throttle on every tick (settled or not) so the refresh cadence
    // stays time-based. Non-interactive callers (tests, BeWavedDolphin's throttle
    // disabler) flush on every tick, as before.
    const bool visualsDue = !(m_visualThrottleEnabled && Application::interactiveMode)
                            || (++m_visualTickCount >= m_visualTickInterval);
    if (visualsDue) {
        m_visualTickCount = 0;
    }

    // Seed before opening any wave bracket: seeding only schedules events (nothing
    // evaluates), which is what lets a settled tick bail out below without paying
    // beginWave()/finishWave()'s full sequential-element sweeps — on a large idle
    // circuit those two sweeps 1000x/s were the engine's entire idle cost. Later
    // waves used to re-seed via processEvents(), but that was always a no-op: the
    // wave-0 seed clears the source-change flags it keys on.
    seedTickEvents();

    const SimTime tickTargetTime = m_currentTime + m_timePerTick;

    if (m_eventQueue.empty() || m_eventQueue.nextTime() > tickTargetTime) {
        // Settled for this tick's window: nothing pending (or, in temporal mode, the
        // earliest event lies beyond the boundary — pure time advancement either way).
        // Advance the boundary exactly as finishDrain() would have, and still honour a
        // pending throttled visual flush: the last wave-running tick may have returned
        // before its refresh was due, and settling must not swallow it.
        m_currentTime = tickTargetTime;
        if (visualsDue && m_visualsDirty) {
            refreshVisuals();
            m_visualsDirty = false;
        }
        return;
    }

    m_evaluatedSequentialThisTick.clear();
    const int maxWaves = static_cast<int>(sequentialElements.size()) + 1;

    for (int wave = 0; wave < maxWaves; ++wave) {
        beginWave();

        // Evaluate logic via the unified event-driven engine — one code path for
        // combinational and feedback circuits. Reproduces the old iterative settling at zero
        // delay and supports propagation-delay (temporal) simulation when delays are set.
        // Sequential elements were just bracketed by beginWave(), so their setOutputValue()
        // calls here stage silently and don't trigger successor scheduling until the
        // post-edge resettle pass inside finishWave(), after the commit runs. Wave 0
        // seeds/drains exactly as before; later waves drain only what the previous wave's
        // finishWave() "wake not-yet-run sequential elements" step scheduled.
        processEvents(tickTargetTime);

        QSet<GraphicElement *> changedThisWave;
        if (!finishWave(changedThisWave)) {
            break;
        }
    }

    // Visual updates only need to run at display-refresh rate, not at simulation rate
    // (1000 Hz) — skipping most ticks avoids dirtying QGraphicsItems that would be
    // overwritten before the next repaint. In non-interactive (test) mode every tick
    // flushes so tests see immediate visual state after each step. The dirty flag lets
    // a later settled tick deliver a flush this tick's throttle deferred.
    m_visualsDirty = true;

    if (!visualsDue) {
        return;
    }

    refreshVisuals();
    m_visualsDirty = false;
}

void Simulation::beginWave()
{
    // Synchronous sequential elements (flip-flops, latches — ElementGroup::Memory,
    // collected across the whole IC hierarchy) get non-blocking semantics: their
    // outputs are staged during the pass and committed together afterwards, so any
    // combinational logic between them (gated clocks especially) reads the
    // pre-tick state. This matches real synchronous hardware and the exported
    // SystemVerilog's non-blocking (<=) model. Gate-built feedback latches are
    // ElementGroup::Gate, not Memory, so they are never deferred and settle
    // through the existing path unchanged. Re-arming an element that already
    // committed in an earlier wave this tick is harmless: beginDeferredCommit()
    // seeds the staging buffer from the CURRENT (already-published) output.
    for (auto *element : std::as_const(m_sequentialElements)) {
        if (element) {
            element->beginDeferredCommit();
        }
    }
}

bool Simulation::finishWave(QSet<GraphicElement *> &changedThisWave)
{
    // Publish every staged sequential output simultaneously (global commit). Track exactly
    // which elements changed value this wave (sequential here, combinational below) — the
    // wake step needs the real changed set, not "every sequential element", so a steady-state
    // tick where nothing actually changed schedules nothing (see the wake step's comment).
    for (auto *element : std::as_const(m_sequentialElements)) {
        if (element) {
            element->clearOutputChanged();
            element->commitDeferredOutputs();
            if (element->outputChanged()) {
                changedThisWave.insert(element);
            }
        }
    }
    const bool anySequentialChanged = !changedThisWave.isEmpty();

    // Post-edge settle (the second half of a clock cycle / a bounded delta
    // cycle): once flip-flops have committed, re-propagate their new outputs
    // through combinational logic and IC output boundaries so the end-of-tick
    // state is a true fixed point — and so asynchronous overrides (preset/clear)
    // surface immediately rather than lagging a tick behind. Sequential elements
    // are skipped here so they are not re-clocked. Only needed on ticks where an
    // edge or async override actually changed sequential state; steady-state
    // ticks pay nothing.
    bool resettleChanged = false;
    if (anySequentialChanged) {
        resettleChanged = resettleCombinationalOnce(changedThisWave);
    }

    // Capture the just-published sequential outputs (and their re-settled combinational
    // cone) at the tick boundary. The drain-side captures all run BEFORE
    // commitDeferredOutputs() publishes the staged values, and a steady tick drains no
    // events at all — without this capture, a flip-flop's Q transition would be recorded
    // only at the NEXT event-bearing timestamp (for a slow clock, half a period late).
    // m_currentTime is when every observer (wires, LEDs, downstream logic) actually sees
    // the new values, so that is the truthful timestamp; recordAll()'s same-time collapse
    // merges multi-wave commits at one boundary. mirrorICOutputs() first, for the same
    // reason as the drain-side captures: a directly-watched IC output pin is read via the
    // IC's own outputValue(), which only mirrorICOutputs() keeps in sync with the
    // flattened boundary Node that actually carries the committed value.
    if (anySequentialChanged && m_recorder.isRecording()) {
        mirrorICOutputs();
        m_recorder.recordAll(m_currentTime);
    }

    // Wake every Memory-group successor of something that GENUINELY changed this wave
    // (via commit or resettle) and that has not yet been scheduled at all this tick. This
    // is what lets a flip-flop driven by another flip-flop's Q — directly, or through
    // intervening combinational gates already re-propagated by resettleCombinationalOnce()
    // above — get its own evaluate/stage/commit wave, in as many hops as the chain has
    // stages. Targeting only real successors of real changes (not "every sequential element,
    // unconditionally") matters: an earlier draft woke every not-yet-evaluated sequential
    // element every wave regardless of relevance, which on a steady-state tick (nothing
    // changed) still scheduled each one at t+delay — a spurious event that, once its delay
    // finally elapsed ticks later, caused a bogus re-evaluation with no real edge behind it,
    // corrupting edge-detection state. Marking a woken successor into
    // m_evaluatedSequentialThisTick the moment it's scheduled here (not only once
    // the drain actually pops and evaluates it) matters in temporal mode: if its delay
    // pushes the scheduled time past this tick's target, the drain correctly leaves the
    // event pending in the persistent queue for a later tick (exactly like any other
    // delayed successor) rather than evaluating it prematurely — without this early marking,
    // a later wave this same tick would see it as still "not yet run" and schedule a
    // duplicate event for it on every remaining wave.
    bool newlyScheduled = false;
    for (auto *changed : std::as_const(changedThisWave)) {
        const auto it = m_successorGraph.constFind(changed);
        if (it == m_successorGraph.constEnd()) {
            continue;
        }
        for (auto *successor : *it) {
            if (successor && successor->elementGroup() == ElementGroup::Memory
                && !m_evaluatedSequentialThisTick.contains(successor)) {
                schedule(m_currentTime + delayTo(successor), successor);
                m_evaluatedSequentialThisTick.insert(successor);
                newlyScheduled = true;
            }
        }
    }

    return anySequentialChanged || resettleChanged || newlyScheduled;
}

void Simulation::refreshVisuals()
{
    // Mirror each IC's settled internal outputs onto its external output ports so the wire
    // visuals below (and ic->outputValue()) reflect the flat-netlist result — the IC element
    // itself is not simulated (excluded from m_sortedElements below), so mirrorICOutputs()
    // also pushes the value onto the IC's own output port visuals directly.
    mirrorICOutputs();

    // Phase 3: push computed logic values onto all output port visuals.
    // Iterating elements (not connections) ensures unconnected output ports
    // (e.g. -Q of a flip-flop with no wire attached) are also updated.
    // setStatus() fans out through any attached connections automatically.
    for (auto *element : std::as_const(m_sortedElements)) {
        if (element) {
            for (auto *outputPort : element->outputs()) {
                updatePort(outputPort);
            }
        }
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
    m_sortedElements.clear();
    m_sequentialElements.clear();
    m_clocks.clear();
    m_inputs.clear();
    m_outputs.clear();
    m_successorGraph.clear(); // holds element pointers; must not outlive a rebuild
    m_icOutputMirror.clear(); // holds IC + boundary-node pointers; must not outlive a rebuild
    m_eventQueue.clear();
    m_delays.clear();         // keyed by element pointers; must not outlive a rebuild
    m_evaluatedSequentialThisTick.clear(); // holds element pointers; must not outlive a rebuild
    // A structural edit invalidates whatever the last flush showed; the first tick after
    // the rebuild is a seed-all settle, and its visuals must flush even if the throttle
    // boundary lands on a following settled tick.
    m_visualsDirty = true;
    // Keyed by element pointers too. sortSimElements() re-clears them on the next successful
    // initialize(), but if that never runs (e.g. the whole circuit was deleted, so initialize()
    // bails early), a freed element's reused address could inherit a stale priority or
    // feedback-node marking through schedule()/isInFeedbackLoop().
    m_simPriorities.clear();
    m_simFeedbackNodes.clear();
    m_simHasFeedbackElements = false;
    // Step snapshots hold element pointers (predecessor links, evaluated-sequential sets)
    // and must not outlive the topology; the open-tick bookkeeping refers to a netlist
    // that is being discarded, so there is nothing left to drain — clear, don't finish.
    clearStepState();
    m_currentTime = 0;
    // m_recorder's watch LIST is deliberately NOT cleared here: its traces hold QPointers, so a
    // freed element auto-nulls (crash-safe) and the dead trace is reaped by m_recorder.pruneStale()
    // on the next initialize() — restart() fires on the explicit Restart action, an IC reload, or a
    // fallback path when initialize() can't rebuild a valid topology, and discarding the user's
    // whole watch list on those would be surprising. Its recorded DATA is a different matter:
    // resetTimeline() clears just the transition history, since the clock is about to restart at 0
    // and appending new transitions to the old timeline would violate the ascending-time order
    // statusAt() and the waveform viewer both assume.
    m_recorder.resetTimeline();
    // Bug 4 postcondition: any future cached state added to Simulation must be
    // cleared above. This assert documents the invariant for future maintainers
    // and trips immediately if a new vector is forgotten.
    Q_ASSERT(!m_initialized);
    Q_ASSERT(m_sortedElements.isEmpty() && m_sequentialElements.isEmpty()
          && m_clocks.isEmpty() && m_inputs.isEmpty() && m_outputs.isEmpty()
          && m_successorGraph.isEmpty() && m_icOutputMirror.isEmpty()
          && m_evaluatedSequentialThisTick.isEmpty() && m_recorder.timelineEmpty()
          && m_simPriorities.isEmpty() && m_simFeedbackNodes.isEmpty() && !m_simHasFeedbackElements
          && m_stepHistory.empty() && !m_stepTickActive);
}

void Simulation::beginTimedRun(const SimTime nsPerTick)
{
    // A dolphin sweep is a "continue" boundary for the step debugger: complete any open
    // stepped tick (the sweep is about to clobber element state anyway) and drop the
    // rewind history before the timed window replaces the live one.
    finishStepSession();

    // Start a clean timed timeline: each update() now advances nsPerTick of sim-time. Reset the
    // clock and drop any pending events so column 0 starts at t=0, and force a full re-seed so the
    // first update() settles the whole network from the current input state rather than relying on
    // incremental schedule-on-change carried over from the live run.
    //
    // The bracket must be INVISIBLE to the live session sharing this Simulation: the Live
    // Analyzer may be recording, and a sweep runs on every stimulus-grid interaction —
    // including merely opening the waveform tool. So the bracket itself captures the live
    // clock and suspends recording (owning this here means preservation cannot depend on
    // the caller remembering a suspender); endTimedRun() restores both, keeping the
    // recorded history valid and its timestamps ascending.
    m_liveTimeBeforeTimedRun = m_currentTime;
    m_wasRecordingBeforeTimedRun = m_recorder.isRecording();
    m_recorder.setRecording(false);
    m_timePerTick = nsPerTick;
    m_currentTime = 0;
    m_eventQueue.clear();
    m_eventInitDone = false;
}

void Simulation::endTimedRun(const SimTime restoreTo)
{
    // Leave no timed state behind: events scheduled past the swept window would otherwise fire at
    // the wrong (collapsed) instant once the live run resumes. Restore the previous mode and let
    // the next live update() re-seed from scratch.
    m_timePerTick = restoreTo;
    m_eventQueue.clear();
    m_eventInitDone = false;
    // Resume the LIVE timeline where it left off. Rewinding to 0 here (the old behavior)
    // forced wiping the recorder's history (the ascending-timestamp invariant statusAt()
    // relies on) — which silently destroyed the Live Analyzer session on every sweep.
    // With the clock restored, the history stays valid as-is and recording (suspended by
    // beginTimedRun()) simply picks up where it left off; the reseed on the next live
    // tick re-settles to the restored input state and the recorder's dedup keeps the
    // sweep invisible.
    m_currentTime = m_liveTimeBeforeTimedRun;
    m_recorder.setRecording(m_wasRecordingBeforeTimedRun);
}

Simulation::StepResult Simulation::stepPropagation()
{
    // The step debugger drives the PAUSED engine one visible change per call. While the
    // timer runs, update() owns the tick pipeline — a concurrent partial drain would corrupt
    // the open wave — so stepping requires an explicit pause first.
    if (m_timer.isActive()) {
        return {};
    }
    if (!m_initialized && !initialize()) {
        return {};
    }

    StepSnapshot snapshot = captureStepSnapshot();

    if (!m_stepTickActive) {
        // Open a stepped tick: the same preamble update() runs, minus clocks — the timer is
        // stopped, so wall-clock time is frozen and stepping propagates MANUAL input changes
        // (the debugger workflow: pause, toggle a switch, step through the fallout).
        for (auto *inputElm : std::as_const(m_inputs)) {
            if (inputElm) {
                inputElm->updateOutputs();
            }
        }
        // Same boundary stamping (and the same reseed-tick guard) as update()'s phase 1.
        if (m_eventInitDone && m_recorder.isRecording()) {
            m_recorder.recordAll(m_currentTime);
        }
        m_evaluatedSequentialThisTick.clear();
        m_stepTickTarget = m_currentTime + m_timePerTick;
        m_stepWave = 0;
        // Seeding only schedules events (nothing evaluates), so it may safely run before
        // beginWave() — that lets a settled circuit bail out below with no bracket opened.
        seedTickEvents();
        resetDrainCursor();

        if (m_eventQueue.empty()
            || (m_timePerTick == 0 && m_eventQueue.nextTime() > m_stepTickTarget)) {
            // Fully settled: nothing pending, nothing reachable. (In functional mode an
            // event beyond the current instant — leftovers from a temporal session driven
            // through the raw API — is unreachable for update() too.) Deliberately no
            // history push and no m_currentTime advance: clicking Step on a settled
            // circuit is a no-op, not time creeping forward.
            return {StepStatus::Settled, {}};
        }

        // Temporal fast-forward: when every queued event lies beyond this tick's window,
        // update() would burn whole ticks pushing the boundary forward until the earliest
        // event becomes reachable. Those ticks are pure time advancement (unchanged inputs
        // seed nothing), so jump the equivalent number of windows arithmetically.
        if (m_timePerTick > 0 && m_eventQueue.nextTime() > m_stepTickTarget) {
            const SimTime deficit = m_eventQueue.nextTime() - m_stepTickTarget;
            const SimTime ticksToSkip = (deficit + m_timePerTick - 1) / m_timePerTick;
            m_currentTime += ticksToSkip * m_timePerTick;
            m_stepTickTarget += ticksToSkip * m_timePerTick;
        }

        beginWave();
        m_stepTickActive = true;
    }

    const int maxWaves = static_cast<int>(m_sequentialElements.size()) + 1;

    while (true) {
        const DrainResult drained = drainOneEvent(m_stepTickTarget);

        if (drained.status == DrainResult::Status::Evaluated) {
            if (drained.changed && drained.element) {
                pushStepHistory(std::move(snapshot));
                refreshVisuals();
                // Capture the change NOW instead of waiting for the drain to move past this
                // timestamp (which may only happen on a later click) — the Live Analyzer must
                // extend on every step. Same-time captures collapse, so if later events at
                // this timestamp refine the value, the trace converges to the settled one.
                if (m_recorder.isRecording()) {
                    m_recorder.recordAll(m_currentTime);
                }
                return {StepStatus::Stepped, {drained.element}};
            }
            continue; // a no-op evaluation is not a step
        }

        // Exhausted or OscillationCapped: this wave's drain is done — close it exactly as
        // update() does. (After the cap the queue was cleared, so Exhausted follows anyway.)
        finishDrain(m_stepTickTarget);
        QSet<GraphicElement *> changedThisWave;
        const bool moreWaves = finishWave(changedThisWave);
        ++m_stepWave;

        const bool tickDone = !moreWaves || m_stepWave >= maxWaves;
        if (tickDone) {
            m_stepTickActive = false;
        } else {
            beginWave();
            seedTickEvents(); // no-op parity with update()'s per-wave processEvents() call
            resetDrainCursor();
        }

        if (!changedThisWave.isEmpty()) {
            // The synchronous commit (plus its resettled combinational cone) is ONE step:
            // flip-flops sharing a clock edge publish together, exactly like hardware.
            pushStepHistory(std::move(snapshot));
            refreshVisuals();
            QVector<GraphicElement *> changed(changedThisWave.cbegin(), changedThisWave.cend());
            std::sort(changed.begin(), changed.end(),
                      [this](const GraphicElement *a, const GraphicElement *b) {
                          return m_simPriorities.value(a, -1) > m_simPriorities.value(b, -1);
                      });
            return {StepStatus::Stepped, changed};
        }

        if (tickDone) {
            // The tick closed without any visible change in this call (its earlier steps,
            // if any, already returned from earlier calls). Rather than bothering the user
            // with an empty "tick boundary" click, roll straight into the next tick: either
            // something is reachable there (pending delayed events after fast-forward) and
            // the recursion steps it, or nothing is and it reports Settled. Terminates
            // because every cycle drains at least one queued event or ends Settled. The
            // unused snapshot is simply dropped — only visible steps enter the history.
            // Refresh visuals first: the oscillation cap canonicalizes feedback nodes to
            // Unknown outside the changed-set tracking, and that must not linger stale.
            refreshVisuals();
            return stepPropagation();
        }
        // The wave closed silently but scheduled more work (e.g. a woken flip-flop) — keep
        // draining inside this same call until something visible happens.
    }
}

Simulation::StepResult Simulation::stepBack()
{
    if (m_timer.isActive() || m_stepHistory.empty()) {
        return {};
    }

    const StepSnapshot snapshot = std::move(m_stepHistory.back());
    m_stepHistory.pop_back();

    // Diff the visible outputs across the restore so the caller can highlight what rewound.
    QVector<QVector<Status>> before;
    before.reserve(m_sortedElements.size());
    for (auto *element : std::as_const(m_sortedElements)) {
        QVector<Status> outs;
        if (element) {
            const auto count = element->simOutputSize();
            outs.reserve(count);
            for (int p = 0; p < count; ++p) {
                outs.append(element->outputValue(p));
            }
        }
        before.append(outs);
    }

    restoreStepSnapshot(snapshot);

    QVector<GraphicElement *> changed;
    for (int i = 0; i < m_sortedElements.size(); ++i) {
        auto *element = m_sortedElements.at(i);
        if (!element) {
            continue;
        }
        const auto &prev = before.at(i);
        for (int p = 0; p < prev.size(); ++p) {
            if (element->outputValue(p) != prev.at(p)) {
                changed.append(element);
                break;
            }
        }
    }

    // Rewind the recorded timeline to the snapshot's mark — everything captured by the
    // rolled-back step (and any same-time collapse it caused) disappears with it.
    m_recorder.rewindTimeline(snapshot.timelineMark, m_currentTime);
    refreshVisuals();
    return {StepStatus::Stepped, changed};
}

Simulation::StepSnapshot Simulation::captureStepSnapshot() const
{
    StepSnapshot snapshot;
    snapshot.elementStates.reserve(m_sortedElements.size());
    snapshot.internalStates.reserve(m_sortedElements.size());
    for (auto *element : std::as_const(m_sortedElements)) {
        snapshot.elementStates.append(element ? element->simRuntimeState() : ElementSimState{});
        snapshot.internalStates.append(element ? element->simInternalState() : QVector<Status>{});
    }
    snapshot.eventQueue = m_eventQueue;
    snapshot.currentTime = m_currentTime;
    snapshot.eventInitDone = m_eventInitDone;
    snapshot.evaluatedSequentialThisTick = m_evaluatedSequentialThisTick;
    snapshot.drainEvalsAtTime = m_drainEvalsAtTime;
    snapshot.drainLastTime = m_drainLastTime;
    snapshot.stepTickActive = m_stepTickActive;
    snapshot.stepWave = m_stepWave;
    snapshot.stepTickTarget = m_stepTickTarget;
    snapshot.timelineMark = m_recorder.markTimeline();
    return snapshot;
}

void Simulation::restoreStepSnapshot(const StepSnapshot &snapshot)
{
    for (int i = 0; i < m_sortedElements.size() && i < snapshot.elementStates.size(); ++i) {
        auto *element = m_sortedElements.at(i);
        if (element) {
            element->setSimRuntimeState(snapshot.elementStates.at(i));
            element->setSimInternalState(snapshot.internalStates.at(i));
        }
    }
    m_eventQueue = snapshot.eventQueue;
    m_currentTime = snapshot.currentTime;
    m_eventInitDone = snapshot.eventInitDone;
    m_evaluatedSequentialThisTick = snapshot.evaluatedSequentialThisTick;
    m_drainEvalsAtTime = snapshot.drainEvalsAtTime;
    m_drainLastTime = snapshot.drainLastTime;
    m_stepTickActive = snapshot.stepTickActive;
    m_stepWave = snapshot.stepWave;
    m_stepTickTarget = snapshot.stepTickTarget;
}

void Simulation::pushStepHistory(StepSnapshot &&snapshot)
{
    m_stepHistory.push_back(std::move(snapshot));
    while (m_stepHistory.size() > kMaxStepHistory) {
        m_stepHistory.pop_front();
    }
}

void Simulation::clearStepState()
{
    m_stepHistory.clear();
    m_stepTickActive = false;
    m_stepWave = 0;
    m_stepTickTarget = 0;
}

void Simulation::finishStepSession()
{
    // Complete any mid-flight stepped tick with the exact primitives update() uses, bounded
    // the same way, so the deferred-commit bracket always closes before a live run or a
    // sweep touches the elements. Doing nothing here would leave flip-flops with staged
    // outputs that the next beginWave() would silently re-seed — consistent but surprising;
    // finishing the tick lands on the same state a Play-through would have reached.
    if (m_stepTickActive) {
        const int maxWaves = static_cast<int>(m_sequentialElements.size()) + 1;
        while (m_stepTickActive) {
            while (drainOneEvent(m_stepTickTarget).status == DrainResult::Status::Evaluated) {
                // Drain to exhaustion — same blocking settle as processEvents().
            }
            finishDrain(m_stepTickTarget);
            QSet<GraphicElement *> changedThisWave;
            const bool moreWaves = finishWave(changedThisWave);
            ++m_stepWave;
            if (!moreWaves || m_stepWave >= maxWaves) {
                m_stepTickActive = false;
            } else {
                beginWave();
                seedTickEvents();
                resetDrainCursor();
            }
        }
        refreshVisuals();
    }
    clearStepState();
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

    // Resuming Play is the step debugger's "continue": complete any open stepped tick so
    // the live run starts from a closed, settled state, and drop the rewind history — you
    // cannot step back past a continue, exactly like a code debugger.
    finishStepSession();

    if (!m_initialized) {
        initialize();
    } else if (m_hasPausedAt) {
        // Resuming after a stop(): the wall clock advanced while paused, so shift each
        // clock's phase reference by the pause duration. This prevents the burst of missed
        // toggles a stale reference would cause, WITHOUT resetting the clocks — resetClock()
        // forces the output HIGH and restarts the phase, which injected a spurious rising
        // edge into every clock-driven circuit on each SimulationBlocker cycle (every
        // UpdateCommand redo/undo, including a plain InputSwitch click). Level and phase
        // now survive any pause/resume; only initialize() (Restart, rebuilds) still gives
        // clocks a fresh HIGH start via resetClock().
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
        // amount, so fall back to the historical reset-to-now behavior.
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

void Simulation::processEvents(const SimTime targetTime)
{
    // Event-driven, BLOCKING settle — one path for both modes. A time-bucketed drain over the
    // event queue: each element re-evaluates at t + its propagation delay, so events spread
    // across future timestamps (real timing; inertial glitch absorption, since a refired
    // element re-reads its now-settled live inputs). Functional mode is just this same drain
    // with every delay forced to 0: all events then land at the current instant (m_timePerTick
    // == 0 ⇒ targetTime == m_currentTime, no future timestamps), so the drain degenerates to a
    // single zero-delay wave settle — the classic combinational/feedback behaviour.
    // Seeding happens in update(), once per tick BEFORE any wave bracket opens (seeding only
    // schedules, nothing evaluates) — that is what lets a settled tick bail out without paying
    // the wave sweeps. Within a timestamp, events are evaluated in topological-priority order
    // with immediate (blocking) commit so feedback symmetry is broken deterministically; delta
    // cycles repeat until the timestamp settles. \a targetTime is supplied by the caller
    // (update()'s wave loop) rather than derived here, since a single tick may drain this more
    // than once (each wave passes the SAME tick-wide target).
    resetDrainCursor();
    while (drainOneEvent(targetTime).status == DrainResult::Status::Evaluated) {
        // Drain to exhaustion (or the oscillation cap) — the blocking settle.
    }
    finishDrain(targetTime);
}

void Simulation::seedTickEvents()
{
    if (!m_eventInitDone) {
        // First tick after (re)init: seed the whole network so it settles to a baseline —
        // every element evaluates once and values propagate from the sources.
        for (auto *element : std::as_const(m_sortedElements)) {
            if (element) {
                schedule(m_currentTime, element);
            }
        }
        m_eventInitDone = true;
    } else {
        // Steady state (incremental): seed only the sources whose output changed this tick
        // (inputs via Phase 1 updateOutputs, clocks via updateClock+updateOutputs); their
        // successors re-evaluate after their propagation delay. Re-evaluating an element with
        // unchanged inputs is idempotent (gates; and flip-flops, since clk == m_simLastClk ⇒ no
        // edge), so skipping unchanged elements yields the same fixed point as a full scan.
        const auto seedChangedSource = [this](GraphicElement *source) {
            if (!source || !source->outputChanged()) {
                return;
            }
            const auto it = m_successorGraph.constFind(source);
            if (it != m_successorGraph.constEnd()) {
                for (auto *successor : *it) {
                    if (successor) {
                        schedule(m_currentTime + delayTo(successor), successor);
                    }
                }
            }
            source->clearOutputChanged();
        };
        for (auto *inputElm : std::as_const(m_inputs)) {
            seedChangedSource(inputElm);
        }
        for (auto *clock : std::as_const(m_clocks)) {
            seedChangedSource(clock);
        }
    }
}

Simulation::DrainResult Simulation::drainOneEvent(const SimTime targetTime)
{
    // Process one event at a time in (time, priority-desc) order: within a timestamp the highest-
    // priority (most upstream) element always runs first, so an element is never evaluated until
    // its upstream inputs have settled. This makes internally-generated clocks (e.g. gated clocks
    // built from a counter) reach their final value before any downstream flip-flop samples them —
    // no zero-delay glitches. The per-timestamp evaluation cap detects genuine oscillation.
    if (m_eventQueue.empty() || m_eventQueue.nextTime() > targetTime) {
        return {};
    }

    const SimEvent event = m_eventQueue.pop();
    const SimTime t = event.time;
    m_currentTime = t;
    if (t != m_drainLastTime) {
        // The previous timestamp has fully settled — capture watched signals at it before
        // advancing. Recording per distinct timestamp gives the waveform ns-resolution
        // transitions, so per-element propagation delays are visible regardless of tick size.
        // mirrorICOutputs() must run first: it's normally only called once, at the very end
        // of update(), but a directly-watched IC output pin is read via the IC's own
        // outputValue() (its actual simulation happens on the flattened internal boundary
        // node) — without refreshing the mirror here too, recordAll() would sample the
        // PREVIOUS tick's mirrored value, one tick stale. The copy is cheap and idempotent,
        // so refreshing it more often than strictly necessary for rendering is harmless.
        if (m_drainLastTime != SIM_TIME_UNSET && m_recorder.isRecording()) {
            mirrorICOutputs();
            m_recorder.recordAll(m_drainLastTime);
        }
        m_drainEvalsAtTime = 0; // the cap detects oscillation within a single timestamp
        m_drainLastTime = t;
    }

    auto *element = event.target;
    if (!element) {
        // A dead event (target freed and auto-nulled): nothing evaluated, keep draining.
        return {DrainResult::Status::Evaluated, nullptr, false};
    }
    element->clearOutputChanged();
    element->updateLogic();
    if (element->elementGroup() == ElementGroup::Memory) {
        // Record that this sequential element actually ran this tick, so update()'s wave
        // loop doesn't also "wake" it again as if it were still unreached — its own change
        // (visible once the caller's commitDeferredOutputs() publishes it) is picked up by
        // that same wave loop's next pass instead of by the scheduling below, since a
        // deferred element's outputChanged() stays false here regardless of what it wrote.
        m_evaluatedSequentialThisTick.insert(element);
    }
    if (element->outputChanged()) {
        const auto it = m_successorGraph.constFind(element);
        if (it != m_successorGraph.constEnd()) {
            for (auto *successor : *it) {
                if (successor) {
                    schedule(t + delayTo(successor), successor);
                }
            }
        }
    }

    const int maxEvalsPerTime = 1000 * qMax(1, static_cast<int>(m_sortedElements.size()));
    if (++m_drainEvalsAtTime >= maxEvalsPerTime) {
        if (!m_convergenceWarned) {
            m_convergenceWarned = true;
            qDebug() << "Feedback circuit did not converge at time" << t;
            emit simulationWarning(tr("Warning: feedback circuit did not converge — the circuit may be oscillating."));
        }
        // Zero-delay oscillation: the feedback region has no stable value. Freezing an
        // arbitrary phase is meaningless and not idempotent across ticks, so
        // canonicalize oscillating feedback nodes to Unknown (NOT(Unknown)=Unknown is a
        // fixed point) — deterministic and stable next tick. (Timed oscillation is real
        // and resolves over time, so it never reaches this per-timestamp cap.)
        for (auto *feedbackElm : std::as_const(m_sortedElements)) {
            if (feedbackElm && m_simFeedbackNodes.contains(feedbackElm)) {
                for (int i = 0; i < feedbackElm->outputSize(); ++i) {
                    feedbackElm->setOutputValue(i, Status::Unknown);
                }
            }
        }
        m_eventQueue.clear();
        return {DrainResult::Status::OscillationCapped, element, element->outputChanged()};
    }

    return {DrainResult::Status::Evaluated, element, element->outputChanged()};
}

void Simulation::finishDrain(const SimTime targetTime)
{
    // Capture the final settled timestamp for the waveform viewer. recordAll() dedups
    // unchanged values, so steady-state ticks add nothing and the trace simply extends.
    // mirrorICOutputs() first, for the same reason as the per-timestamp capture above.
    if (m_drainLastTime != SIM_TIME_UNSET && m_recorder.isRecording()) {
        mirrorICOutputs();
        m_recorder.recordAll(m_drainLastTime);
    }

    m_currentTime = targetTime; // time advances to the tick boundary (temporal); stays 0 (functional)
}

bool Simulation::resettleCombinationalOnce(QSet<GraphicElement *> &changedOut)
{
    // Re-propagate just-committed sequential outputs through combinational logic to a true
    // fixed point. Sequential elements are skipped so this pass never re-clocks them — that's
    // update()'s wave loop's job (waking a not-yet-run sequential element via the event queue,
    // which correctly re-invokes its full edge-detection logic instead of a blind resettle).
    // Feed-forward logic settles in one topological sweep; iterate only when combinational
    // feedback is present. Every element that changes in ANY pass is added to \a changedOut (not
    // just the last pass), so the caller's successor walk sees the true overall changed set
    // across this whole multi-pass convergence, including a combinational element that changed
    // in an early pass but happened to be stable by the final one.
    const int maxPasses = m_simHasFeedbackElements ? kMaxSettleIterations : 1;
    bool anyChanged = false;
    for (int pass = 0; pass < maxPasses; ++pass) {
        bool changed = false;
        for (auto *element : std::as_const(m_sortedElements)) {
            if (element && element->elementGroup() != ElementGroup::Memory) {
                element->clearOutputChanged();
                element->resettleCombinational();
                if (element->outputChanged()) {
                    changed = true;
                    changedOut.insert(element);
                }
            }
        }
        if (changed) {
            anyChanged = true;
        } else {
            break;
        }
    }
    return anyChanged;
}

bool Simulation::initialize()
{
    if (!m_host) {
        return false;
    }

    // Rebuild all categorised lists from scratch so stale pointers from
    // a previous circuit state don't linger after undo/redo or file load.
    m_convergenceWarned = false;
    m_eventInitDone = false; // first processEvents() after (re)init seeds the whole network
    // Same invariant restart() documents: topology-derived state must not outlive a
    // rebuild. Pending events are stale twice over — SimEvent::target raw pointers (a
    // structural command may have freed the element: delete, morph, split-undo) and
    // priorities baked from the OLD m_simPriorities at schedule time. In temporal mode
    // pending cross-tick events are routine, so without this clear the next drain pops
    // an event for a freed element (use-after-free). The full-network seed that
    // m_eventInitDone=false forces makes the dropped events redundant anyway.
    m_eventQueue.clear();
    m_evaluatedSequentialThisTick.clear();
    // Same pointer-lifetime invariant as restart(): step snapshots reference the old
    // netlist and cannot survive a rebuild.
    clearStepState();
    m_clocks.clear();
    m_outputs.clear();
    m_inputs.clear();
    m_sortedElements.clear();
    m_sequentialElements.clear();

    // The recorder holds QPointers to watched elements, so freed elements have already auto-nulled;
    // reap those dead traces here so the watch list tracks the rebuilt netlist (live watches, whose
    // element survives the rebuild, keep their pointer and continue recording).
    m_recorder.pruneStale();

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

    // Wire each IC's internal primitives (connections + nested ICs). The IC itself no longer
    // simulates; its internals join the top-level flat netlist below.
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
    m_delays.clear();
    for (auto *elm : std::as_const(flatElements)) {
        m_delays[elm] = elm->propagationDelay();
    }

    // Topological sort with feedback detection over the flat netlist
    sortSimElements(flatElements);

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

void Simulation::sortSimElements(const QVector<GraphicElement *> &elements)
{
    // Build the flat successor adjacency the same way buildSuccessorGraph() does — walking each
    // element's output connections in port/connection order — so the adjacency ORDER matches the
    // legacy per-scope graphs. That order matters: calculatePriorities() assigns a feedback
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

    // 1. Physical edges, in output-port/connection order (driver → consumer, IC inputs mapped to
    //    their boundary Nodes). Boundary input Nodes' internal fan-out is covered here too.
    for (auto *elm : std::as_const(elements)) {
        for (auto *outputPort : elm->outputs()) {
            for (auto *conn : outputPort->connections()) {
                if (auto *endPort = conn ? conn->endPort() : nullptr) {
                    addEdge(elm, mapConsumer(endPort));
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

void Simulation::mirrorICOutputs()
{
    for (const auto &m : std::as_const(m_icOutputMirror)) {
        if (m.ic && m.boundaryNode) {
            m.ic->setOutputValue(m.outIndex, m.boundaryNode->outputValue(0));
            // The IC container node is excluded from the flat m_sortedElements (it is pure
            // structure, not simulated), so Phase 3's element-output loop never visits its
            // ports. Push the mirrored value onto the wire visual here instead, at the source.
            updatePort(m.ic->outputPort(m.outIndex));
        }
    }
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
