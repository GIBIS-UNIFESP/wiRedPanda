// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Synchronous cycle-based simulation engine with event-driven clock support.
 */

#pragma once

#include <chrono>
#include <deque>
#include <memory>

#include <QGraphicsItem>
#include <QHash>
#include <QObject>
#include <QSet>
#include <QTimer>

#include "App/Element/ElementSimState.h"
#include "App/Simulation/SimEvent.h"
#include "App/Simulation/SimTime.h"
#include "App/Simulation/WaveformRecorder.h"

class Clock;
class GraphicElement;
class GraphicElementInput;
class InputPort;
class OutputPort;
class SimulationHost;

/**
 * \class Simulation
 * \brief Manages the digital circuit simulation loop.
 *
 * \details The simulation runs a 1 ms periodic QTimer.  On each tick it:
 * 1. Updates all GraphicElementInput outputs (including clocks).
 * 2. Settles all elements via the unified event-driven engine (processEvents()),
 *    in priority order.
 * 3. Propagates values through connections to output elements.
 *
 * Feedback loops are detected during initialization; the event-driven engine
 * handles them with the same settle path as feed-forward logic, bounded by a
 * delta-cycle cap.
 */
class Simulation : public QObject
{
    Q_OBJECT

    friend class TestDanglingPointer;

public:
    /// Maximum iteration count for post-commit combinational re-settle passes.
    static constexpr int kMaxSettleIterations = 10;

    // --- Lifecycle ---

    /**
     * \brief Constructs a Simulation bound to \a host.
     * \param host Scene (via the narrow SimulationHost interface) whose elements will be simulated.
     * \param parent QObject parent (the same Scene as \a host, passed separately since
     *               SimulationHost is not itself a QObject).
     */
    explicit Simulation(SimulationHost *host, QObject *parent = nullptr);

    /// Destructor; stops the simulation timer.
    ~Simulation() override = default;

    // --- Control ---

    /// Starts the 1 ms simulation timer.
    void start();

    /// Stops the simulation timer.
    void stop();

    /// Sets whether the user has explicitly muted audio; persists across stop/start cycles.
    void setUserMuted(bool muted);

    /// Returns \c true if the user has explicitly muted audio.
    bool isUserMuted() const;

    /// Invalidates the cached simulation topology: clears m_initialized
    /// and the hot-path element/connection vectors so no stale reference
    /// can be dereferenced on subsequent ticks. The next update() call
    /// re-runs initialize(). The QTimer's run state (running/stopped) is
    /// preserved — callers who also want to pause should use
    /// SimulationBlocker.
    void restart();

    /// Returns \c true if the simulation timer is currently running.
    bool isRunning();

    /// Returns \c true if \a element is part of a combinational feedback loop.
    bool isInFeedbackLoop(const GraphicElement *element) const;

    // --- Initialization ---

    /**
     * \brief Builds the simulation graph from the current scene elements.
     * \return \c true if initialization succeeded (all elements are valid).
     */
    bool initialize();

    // --- Step ---

    /// Executes one simulation step (used by tests to advance the simulation manually).
    void update();

    /// Enables or disables the visual refresh throttle.
    /// When disabled, phases 3–4 run on every update() call regardless of tick count.
    /// \sa SimulationThrottleDisabler
    void setVisualThrottleEnabled(bool enabled);

    /// Invalidates the event-driven engine's incremental "what changed since last tick"
    /// tracking without rebuilding the topology (unlike restart(), which also drops the
    /// cached graph). Call this after resetting elements' simulation state directly (e.g.
    /// WaveformSimulator::sweep() resetting every element to power-on defaults before a
    /// sweep) — otherwise the next update() only re-seeds from m_inputs/m_clocks and never
    /// notices that every other element's value is now stale, since processEvents() only
    /// does a full network seed on the first tick after initialize().
    /// Deliberately does NOT touch the sim clock or the waveform recorder: the timeline
    /// belongs to the live session or to an enclosing beginTimedRun()/endTimedRun() bracket.
    void resetEventTracking();

    // --- Temporal (propagation-delay) simulation ---

    /// Sim-time advanced per update() tick. 0 ⇒ functional (zero-delay): every event
    /// lands at the current instant and the tick is a full settle. >0 ⇒ temporal:
    /// events spread across future timestamps by per-element propagation delay.
    /// Emits timePerTickChanged() so observers (e.g. the Live Analyzer's zoom) can adapt.
    /// Ends any open step-debugger session first: a stepped tick mid-flight was opened
    /// under the old tick window, and rewinding across a mode/speed change is undefined.
    void setTimePerTick(SimTime ns)
    {
        if (m_timePerTick == ns) {
            return;
        }
        finishStepSession();
        m_timePerTick = ns;
        emit timePerTickChanged(ns);
    }

    /// Sim-time advanced per update() tick (0 ⇒ functional). Lets a caller snapshot the
    /// current mode before a timed sweep and restore it afterwards.
    SimTime timePerTick() const { return m_timePerTick; }

    /// Begins a deterministic timed (temporal) run that advances \a nsPerTick of sim-time per
    /// update(): sets the per-tick window, resets sim-time to 0, drops any queued events, and
    /// forces the next update() to re-seed the whole network from the current element state.
    /// The bracket is invisible to a live recording: it captures the live clock and suspends
    /// the waveform recorder itself (restored by endTimedRun()), so a sweep can neither
    /// pollute nor destroy the Live Analyzer's recorded history.
    /// Pair with endTimedRun() so the live simulation resumes cleanly. Used by callers that drive
    /// update() manually over a fixed timeline (e.g. the BeWavedDolphin column sweep).
    void beginTimedRun(SimTime nsPerTick);

    /// Ends a timed run started by beginTimedRun(): restores the per-tick window to \a restoreTo,
    /// drops any events still queued past the swept window, and restores the live sim clock and
    /// recording flag captured by beginTimedRun() — the live timeline (and any recorded
    /// history) resumes exactly where it left off.
    void endTimedRun(SimTime restoreTo);

    // --- Step debugger ---

    /// Outcome of a stepPropagation()/stepBack() call.
    enum class StepStatus {
        Stepped,  ///< At least one element's output visibly changed (see StepResult::changed).
        Settled,  ///< Nothing to step: no pending input change and no reachable queued event.
        NotReady, ///< Stepping unavailable: the timer is running, or no valid topology exists.
    };

    /// Result of one debugger step: the status plus the elements whose outputs changed
    /// (highlight targets), most-upstream first.
    struct StepResult {
        StepStatus status = StepStatus::NotReady;
        QVector<GraphicElement *> changed;
    };

    /// Advances the PAUSED simulation to the next signal change — the circuit debugger's
    /// "step". Drives the same engine primitives as update(), suspended one visible change
    /// per call: combinational changes step one element at a time in causal order, while a
    /// synchronous commit (flip-flops sharing a clock edge, plus its resettled combinational
    /// cone) publishes as ONE step, exactly like hardware. Clocks stay frozen (they are
    /// wall-clock driven and the timer is stopped); pending manual input toggles are picked
    /// up by the stepped tick's phase-1 preamble. Each visible step pushes a rewind snapshot
    /// for stepBack(). Requires the timer to be paused (returns NotReady otherwise).
    StepResult stepPropagation();

    /// Rewinds the last forward step: restores every element's runtime + internal state, the
    /// event queue, the sim clock, and the step bookkeeping from the snapshot, and rewinds
    /// the waveform recorder to its matching mark. Returns the elements whose visible
    /// outputs changed by the rewind. History is session-bounded: resuming Play, sweeps,
    /// mode changes, and topology rebuilds all clear it (you can't rewind past a "continue").
    StepResult stepBack();

    /// True when step history is available for stepBack().
    bool canStepBack() const { return !m_stepHistory.empty(); }

    /// Sets the propagation delay (sim-time units) for \a element. 0 ⇒ zero-delay.
    void setElementDelay(const GraphicElement *element, SimTime ns) { m_delays[element] = ns; }

    /// Current simulation time (advances only in temporal mode).
    SimTime currentTime() const { return m_currentTime; }

    /// Waveform recorder for the temporal timing-diagram viewer. Watched signals are
    /// captured at each temporal tick boundary while recording is enabled.
    WaveformRecorder &waveformRecorder() { return m_recorder; }
    const WaveformRecorder &waveformRecorder() const { return m_recorder; }

    // --- Static graph building (used by IC::initializeSimulation too) ---

    static void buildConnectionGraph(const QVector<GraphicElement *> &elements);
    /// Overrides physical predecessors on Rx nodes with their matching Tx node.
    /// Must be called after buildConnectionGraph() so wireless always wins, and before sort().
    static void connectWirelessElements(const QVector<GraphicElement *> &elements);

    /// Builds a label→element map for wireless Tx nodes. First Tx per label wins.
    static QHash<QString, GraphicElement *> buildTxMap(const QVector<GraphicElement *> &elements);

    /// Builds a successor adjacency list from connection graph + wireless Tx→Rx edges.
    static QHash<GraphicElement *, QVector<GraphicElement *>> buildSuccessorGraph(
        const QVector<GraphicElement *> &elements,
        const QHash<QString, GraphicElement *> &txMap);

    /// Result of topological sort with feedback detection.
    struct SortResult {
        QVector<GraphicElement *> sorted;                ///< Elements in priority order (highest first).
        QHash<GraphicElement *, int> priorities;          ///< Priority per element.
        QSet<GraphicElement *> feedbackNodes;             ///< Elements in feedback loops.
    };

    /// Topologically sorts elements using the successor graph, detects feedback loops.
    static SortResult topologicalSort(const QVector<GraphicElement *> &elements,
                                      const QHash<GraphicElement *, QVector<GraphicElement *>> &successors);

signals:
    /// Emitted (at most once per initialize()) when a feedback circuit fails to converge.
    void simulationWarning(const QString &message);

    /// Emitted when setTimePerTick() changes the tick window (mode/speed selection). NOT
    /// emitted by beginTimedRun()/endTimedRun() — their temporary swap is a sweep-internal
    /// detail that observers (e.g. the Live Analyzer's zoom) must not react to.
    void timePerTickChanged(SimTime nsPerTick);

private:
    Q_DISABLE_COPY(Simulation)

    // --- Helpers ---

    static void updatePort(InputPort *port);
    static void updatePort(OutputPort *port);

    /// Recursively collects every primitive (non-IC) element, descending through ICs, into \a out.
    /// IC boundary Nodes are primitives and are included; IC container nodes are not.
    static void collectFlatElements(const QVector<GraphicElement *> &elements,
                                    QVector<GraphicElement *> &out);

    /// Splices every IC's boundary into the flat netlist: routes each IC input port's external
    /// driver to the matching boundary input Node, repoints external consumers of each IC output
    /// port to the matching boundary output Node, and records (IC, port, boundary Node) for
    /// output mirroring. Recurses into nested ICs.
    void spliceICBoundaries(const QVector<GraphicElement *> &elements);

    /// Copies each boundary output Node's value back onto its IC's external output port so wire
    /// rendering and ic->outputValue() stay correct (the IC no longer settles itself). Also
    /// pushes the value onto the IC's own output port visual directly, since the IC container
    /// node is excluded from m_sortedElements (Phase 3's element-output loop never visits it).
    void mirrorICOutputs();

    /// Builds m_successorGraph by inverting the flat elements' predecessor links, then topo-sorts.
    void sortSimElements(const QVector<GraphicElement *> &elements);

    /// Recursively appends every ElementGroup::Memory element in \a elements
    /// (descending into ICs) to m_sequentialElements. These get non-blocking
    /// (deferred) output commits so synchronous logic matches SystemVerilog.
    void collectSequentialElements(const QVector<GraphicElement *> &elements);

    /// Delay applied when scheduling \a successor: 0 in functional mode (every event lands
    /// at the current instant), else the successor's own propagation-delay override/default.
    SimTime delayTo(const GraphicElement *successor) const
    {
        return (m_timePerTick == 0) ? SimTime{0} : m_delays.value(successor, 0);
    }

    /// Schedules \a target into the event queue at \a time, at its topological priority.
    void schedule(SimTime time, GraphicElement *target)
    {
        m_eventQueue.schedule({time, m_simPriorities.value(target, -1), target});
    }

    /// The unified event-driven engine: a blocking, time-bucketed settle over the event
    /// queue, draining up to \a targetTime. First tick seeds the whole network (\a elements);
    /// subsequent ticks incrementally seed only the sources that changed (\a inputs, \a clocks),
    /// with per-(delta-cycle) dedup and immediate output commit. Handles both zero-delay
    /// (functional) and propagation-delay (temporal) simulation in one code path. Sequential
    /// (Memory-group) elements are bracketed by the caller's
    /// beginDeferredCommit()/commitDeferredOutputs(), so their setOutputValue() calls here
    /// stage silently (outputChanged() stays false) until the caller's explicit commit —
    /// this engine doesn't need to know about that mechanism to honor it. Every Memory-group
    /// element evaluated during the drain is recorded into m_evaluatedSequentialThisTick, so
    /// update()'s wave loop knows which sequential elements have already run this tick.
    /// Composed entirely from the resumable primitives below (seedTickEvents(),
    /// drainOneEvent(), finishDrain()) so the step debugger can drive the SAME engine one
    /// event at a time.
    void processEvents(SimTime targetTime);

    // --- Engine primitives (shared by update()/processEvents() and the step debugger) ---

    /// Outcome of a single drainOneEvent() call.
    struct DrainResult {
        enum class Status {
            Exhausted,         ///< No event at or before the target time — the drain is done.
            Evaluated,         ///< One event was popped (element may be null for a dead event).
            OscillationCapped, ///< The per-timestamp cap tripped; feedback canonicalized, queue cleared.
        };
        Status status = Status::Exhausted;
        GraphicElement *element = nullptr; ///< The evaluated element (null for a dead event).
        bool changed = false;              ///< Whether the evaluation changed the element's output.
    };

    /// Seeds the event queue for this tick: whole-network on the first tick after (re)init,
    /// incrementally (changed inputs/clocks only) afterwards. Idempotent within a tick —
    /// wave ≥ 1 calls find the sources' change flags already cleared and seed nothing.
    void seedTickEvents();

    /// Pops and evaluates ONE event at or before \a targetTime (the drain-loop body of the
    /// unified engine). Advances m_currentTime to the event's timestamp, captures the
    /// previous timestamp into the waveform recorder when time moves, schedules changed
    /// elements' successors, and trips the per-timestamp oscillation cap. The drain cursor
    /// (m_drainEvalsAtTime/m_drainLastTime) persists across calls so the step debugger can
    /// resume a drain one event per user click; reset it via resetDrainCursor() at the start
    /// of each wave's drain.
    DrainResult drainOneEvent(SimTime targetTime);

    /// Ends a wave's drain: captures the final settled timestamp into the recorder and
    /// advances m_currentTime to \a targetTime (the tick boundary; stays put in functional
    /// mode where the target equals the current instant).
    void finishDrain(SimTime targetTime);

    /// Resets the drain cursor for a fresh wave drain (matches the per-processEvents()
    /// initialization the cursor had when it was a pair of locals).
    void resetDrainCursor()
    {
        m_drainEvalsAtTime = 0;
        m_drainLastTime = SIM_TIME_UNSET;
    }

    /// Opens a wave: brackets every synchronous sequential element with
    /// beginDeferredCommit() so their outputs stage silently until finishWave() commits.
    void beginWave();

    /// Closes a wave: commits all staged sequential outputs simultaneously (non-blocking
    /// semantics), re-settles combinational logic once when anything committed, captures the
    /// tick boundary into the recorder, and wakes not-yet-run Memory successors of the
    /// changed set. \a changedThisWave must be empty on entry; on return it holds every
    /// element whose output changed in this wave (sequential commits and resettled
    /// combinational elements alike). Returns \c true when another wave is needed.
    bool finishWave(QSet<GraphicElement *> &changedThisWave);

    /// Pushes settled values onto the visuals: IC output mirroring, wire (phase 3) and
    /// output-element (phase 4) port statuses. Called by update() at display rate and by
    /// the step debugger after every step (stepping is inherently low-rate).
    void refreshVisuals();

    // --- Step debugger internals ---

    /// Everything needed to rewind one forward step. Element pointers inside (predecessor
    /// links in ElementSimState, the evaluated-sequential set) are topology, valid for the
    /// snapshot's whole lifetime: every rebuild path (restart()/initialize()) clears the
    /// step history before elements can be freed.
    struct StepSnapshot {
        QVector<ElementSimState> elementStates;   ///< Parallel to m_sortedElements.
        QVector<QVector<Status>> internalStates;  ///< Parallel to m_sortedElements (flip-flop edge state).
        EventQueue eventQueue;
        SimTime currentTime = 0;
        bool eventInitDone = false;
        QSet<GraphicElement *> evaluatedSequentialThisTick;
        int drainEvalsAtTime = 0;
        SimTime drainLastTime = SIM_TIME_UNSET;
        bool stepTickActive = false;
        int stepWave = 0;
        SimTime stepTickTarget = 0;
        WaveformRecorder::TimelineMark timelineMark;
    };

    /// Captures the full engine state (cheap value copies) for one rewind level.
    StepSnapshot captureStepSnapshot() const;

    /// Restores engine state from \a snapshot (elements, queue, clock, step bookkeeping).
    /// The recorder is rewound separately by stepBack() — it needs the snapshot's mark.
    void restoreStepSnapshot(const StepSnapshot &snapshot);

    /// Pushes one rewind level, dropping the oldest beyond kMaxStepHistory.
    void pushStepHistory(StepSnapshot &&snapshot);

    /// Drops the step history and tick bookkeeping WITHOUT draining — only for topology
    /// rebuild paths (restart()/initialize()), where the netlist the open tick referred to
    /// is being discarded anyway (initialize() re-runs initSimulationVectors(), which also
    /// resets every element's deferred-commit staging).
    void clearStepState();

    /// Ends a step session at a "continue" boundary: completes any open stepped tick with
    /// the same primitives update() uses (never leaks an open deferred-commit bracket into
    /// a live run or a sweep), then clears the rewind history.
    void finishStepSession();

    /// Re-propagates freshly committed sequential outputs through combinational logic to a
    /// fixed point (bounded by kMaxSettleIterations when feedback is present). Skips
    /// ElementGroup::Memory elements so they are never re-clocked by this pass. Every element
    /// that changes in any pass is inserted into \a changedOut, so update()'s wave loop can walk
    /// successors of the true changed set (combinational elements included) rather than just the
    /// sequential elements it committed directly.
    /// \return true if any element's output changed.
    bool resettleCombinationalOnce(QSet<GraphicElement *> &changedOut);

    // --- Members: Timer & element lists ---

    QTimer m_timer;
    /// When stop() paused a running simulation. start() shifts each clock's phase
    /// reference by the elapsed pause instead of resetting it, so a pause/resume cycle
    /// (SimulationBlocker around every UpdateCommand, or the user's Play toggle) neither
    /// injects a spurious clock edge nor loses the phase. Invalidated by initialize().
    std::chrono::steady_clock::time_point m_pausedAt;
    bool m_hasPausedAt = false;
    QVector<Clock *> m_clocks;
    QVector<GraphicElement *> m_outputs;
    QVector<GraphicElementInput *> m_inputs;

    // --- Members: Host & state ---

    SimulationHost *m_host;

    // --- Members: State flags ---

    bool m_initialized = false;
    bool m_convergenceWarned = false;
    bool m_userMuted = false;

    // --- Members: Visual refresh throttle ---

    int m_visualTickCount = 0;
    int m_visualTickInterval = 16;    ///< Visual update every N simulation ticks (derived from monitor refresh rate, default ~60 fps).
    bool m_visualThrottleEnabled = true; ///< When false, phases 3–4 always run (used by SimulationThrottleDisabler).

    // --- Members: Direct simulation graph ---

    QVector<GraphicElement *> m_sortedElements;
    /// All synchronous sequential elements (flip-flops, latches) across the whole
    /// hierarchy, including those nested inside ICs. Bracketed each tick with
    /// beginDeferredCommit()/commitDeferredOutputs() for non-blocking semantics.
    QVector<GraphicElement *> m_sequentialElements;
    QHash<const GraphicElement *, int> m_simPriorities;
    QSet<const GraphicElement *> m_simFeedbackNodes;
    bool m_simHasFeedbackElements = false;

    /// Persisted successor adjacency (element → its successors), built in sortSimElements() by
    /// inverting each flat element's predecessor links (physical, wireless, and spliced IC
    /// boundaries), so it matches exactly the data flow the engine reads in updateLogic().
    QHash<GraphicElement *, QVector<GraphicElement *>> m_successorGraph;

    /// Sequential (Memory-group) elements already evaluated at least once during the current
    /// update() tick's wave loop. Cleared at the start of every tick. A ripple/derived-clock
    /// chain (one flip-flop's Q feeding another's Clock or Data with no gate in between) needs
    /// more than one wave to settle: the second flip-flop is woken only once the first commits,
    /// and this set is what stops an already-fired flip-flop from being re-invoked by a later
    /// wave (which would corrupt its edge-detection/data-capture state — re-running a flip-flop
    /// with an unchanged clock is not idempotent for the value it captures for the *next* edge).
    QSet<GraphicElement *> m_evaluatedSequentialThisTick;

    /// (IC, output port, boundary output Node) triples for mirrorICOutputs(). Built in
    /// initialize() while splicing IC boundaries; the IC element itself is not simulated.
    struct ICOutputMirror {
        GraphicElement *ic = nullptr;
        int outIndex = 0;
        GraphicElement *boundaryNode = nullptr;
    };
    QVector<ICOutputMirror> m_icOutputMirror;

    // --- Members: Temporal (propagation-delay) simulation ---

    EventQueue m_eventQueue;                          ///< Time-ordered pending re-evaluations.
    SimTime m_currentTime = 0;                        ///< Current sim time (advances in temporal mode).
    /// Drain cursor: evaluations at the current timestamp (oscillation-cap counter) and the
    /// last drained timestamp (recorder-capture edge detector). Members rather than locals so
    /// a drain can be suspended and resumed across step-debugger calls.
    int m_drainEvalsAtTime = 0;
    SimTime m_drainLastTime = SIM_TIME_UNSET;
    SimTime m_timePerTick = 0;                        ///< 0 ⇒ functional; >0 ⇒ temporal window per tick.
    QHash<const GraphicElement *, SimTime> m_delays;  ///< Per-element propagation delay (default 0).
    bool m_eventInitDone = false;                     ///< False until the first seed-all baseline settle.
    WaveformRecorder m_recorder;                       ///< Timing-diagram trace recorder (temporal mode).
    SimTime m_liveTimeBeforeTimedRun = 0;             ///< Live clock captured by beginTimedRun(), restored by endTimedRun().
    bool m_wasRecordingBeforeTimedRun = false;        ///< Recording flag captured/restored across a timed run.

    // --- Members: Step debugger ---

    /// Rewind stack: one snapshot per visible forward step, oldest dropped past the cap.
    static constexpr int kMaxStepHistory = 512;
    std::deque<StepSnapshot> m_stepHistory;
    bool m_stepTickActive = false; ///< A stepped tick is mid-flight (deferred-commit bracket open).
    int m_stepWave = 0;            ///< Waves completed within the open stepped tick (update()'s bound applies).
    SimTime m_stepTickTarget = 0;  ///< The open stepped tick's drain boundary.
};
