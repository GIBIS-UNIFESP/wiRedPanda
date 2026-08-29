// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Synchronous cycle-based simulation engine with event-driven clock support.
 */

#pragma once

#include <chrono>
#include <memory>

#include <QGraphicsItem>
#include <QHash>
#include <QObject>
#include <QSet>
#include <QTimer>

#include "App/Simulation/SimEvent.h"
#include "App/Simulation/SimTime.h"

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
    friend class TestSimulationUnit;

public:
    // --- Lifecycle ---

    /**
     * \brief Constructs a Simulation bound to \a host.
     * \param host Scene (via the narrow SimulationHost interface) whose elements will be simulated.
     * \param parent QObject parent (the same Scene as \a host, passed separately since
     *               SimulationHost is not itself a QObject).
     */
    explicit Simulation(SimulationHost *host, QObject *parent = nullptr);

    /// Destructor; stops the simulation timer.
    ~Simulation() override = default; // LCOV_EXCL_LINE -- destructor-ABI-variant gcov gap (Itanium ABI's separate deleting/complete-object destructors), same class as other `= default` destructors across this sweep.

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

    /// Returns \c true when the last drain left nothing pending, i.e. a further tick with no
    /// source change would recompute identical values. Lets a caller that must present a
    /// SETTLED circuit (rather than advance it by one tick) know when to stop stepping.
    bool isAtFixedPoint() const { return m_atFixedPoint; }

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
    void resetEventTracking();

    // --- Temporal (propagation-delay) simulation ---

    /// Sim-time advanced per update() tick. 0 ⇒ functional (zero-delay): every event
    /// lands at the current instant and the tick is a full settle. >0 ⇒ temporal:
    /// events spread across future timestamps by per-element propagation delay.
    void setTimePerTick(SimTime ns) { m_timePerTick = ns; }

    /// Sim-time advanced per update() tick (0 ⇒ functional). Lets a caller snapshot the
    /// current mode before a timed sweep and restore it afterwards.
    SimTime timePerTick() const { return m_timePerTick; }

    /**
     * \brief Begins a deterministic timed run advancing \a nsPerTick of sim-time per update().
     * \details Sets the per-tick window, restarts sim-time at 0, drops any queued events, and
     * forces the next update() to re-seed the whole network from the current element state
     * rather than relying on incremental change tracking carried over from the live run.
     * Pair with endTimedRun() so the live simulation resumes cleanly. Used by callers that
     * drive update() manually over a fixed timeline (e.g. the BeWavedDolphin column sweep).
     */
    void beginTimedRun(SimTime nsPerTick);

    /**
     * \brief Ends a timed run started by beginTimedRun(), restoring the window to \a restoreTo.
     * \details Drops any events still queued past the swept window and restores the live sim
     * clock. The queue clear is not bookkeeping: those events carry future timestamps, and
     * once the window is back to 0 the drain's `nextTime() <= targetTime` can never hold
     * again, so they would linger — holding element pointers — until the next restart().
     */
    void endTimedRun(SimTime restoreTo);

    /// Sets the propagation delay (sim-time units) for \a element. 0 ⇒ zero-delay.
    /// Ignored for element types that are delay-free by design (default 0: sources, sinks,
    /// Nodes, ICs), so the map cannot hold a delay the UI and MCP both refuse to set --
    /// see initialize().
    void setElementDelay(const GraphicElement *element, SimTime ns);

    /// Current simulation time (advances only in temporal mode).
    SimTime currentTime() const { return m_currentTime; }

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
        /// The cyclic components as a PARTITION -- every element of every inner vector is in a
        /// feedback loop. Carried out of the sort so the single Tarjan pass it already runs
        /// serves every consumer, rather than each recomputing it over the same graph.
        QVector<QVector<GraphicElement *>> feedbackComponents;
    };

    /// Topologically sorts elements using the successor graph, detects feedback loops.
    static SortResult topologicalSort(const QVector<GraphicElement *> &elements,
                                      const QHash<GraphicElement *, QVector<GraphicElement *>> &successors);

signals:
    /// Emitted (at most once per initialize()) when a feedback circuit fails to converge.
    void simulationWarning(const QString &message);

private:
    Q_DISABLE_COPY(Simulation)

    // --- Helpers ---

    static void updatePort(InputPort *port);
    static void updatePort(OutputPort *port);

    /// Drops every topology-derived container -- the categorised element vectors, the successor
    /// graph, the IC output mirror, the event queue and every pointer-keyed cache -- and clears
    /// m_initialized. Shared by restart() and initialize()'s prologue so a rebuild that bails
    /// out early cannot leave the previous circuit's state standing beside a set flag.
    void invalidateTopology();

    /// Recursively collects every primitive (non-IC) element, descending through ICs, into \a out.
    /// IC boundary Nodes are primitives and are included; IC container nodes are not.
    static void collectFlatElements(const QVector<GraphicElement *> &elements,
                                    QVector<GraphicElement *> &out);

    /// Splices every IC's boundary into the flat netlist: routes each IC input port's external
    /// driver to the matching boundary input Node, repoints external consumers of each IC output
    /// port to the matching boundary output Node, and records (IC, port, boundary Node) for
    /// output mirroring. Recurses into nested ICs.
    void spliceICBoundaries(const QVector<GraphicElement *> &elements);

    /// Copies each boundary output Node's settled value onto its IC's external output port
    /// VALUE. Logic, not presentation: the IC container is excluded from m_sortedElements, so
    /// this is the only thing that makes ic->outputValue() correct -- which MCP's
    /// get_output_value and every other reader depend on. Runs at the end of every drain.
    void mirrorICOutputValues();

    /// Pushes each mirrored IC output onto its port VISUAL. Split from the value mirror above
    /// so a throttled tick can skip the repaint without leaving the value stale; the IC
    /// container node is excluded from m_sortedElements, so Phase 3's loop never visits it.
    void pushICOutputVisuals();

    /// Builds m_successorGraph by inverting the flat elements' predecessor links, then topo-sorts.
    void sortSimElements(const QVector<GraphicElement *> &elements);

    /// Derives each element's per-timestamp evaluation cap from the SCC condensation, using the
    /// component partition built by sortSimElements(). An element can legitimately re-evaluate
    /// once per distinct arrival wave reaching it (bounded by its depth in the condensation) and,
    /// inside a feedback region, once more per settling round (bounded by the component's size),
    /// so the product with a slack factor bounds honest settling without bounding oscillation.
    /// Deliberately generous: over-generosity costs only detection latency, which is cheap
    /// because the frozen value is Unknown regardless of when the trip happens, whereas a tight
    /// bound false-positives on legitimate multi-wave settling.
    void computeEvaluationCaps(const QVector<GraphicElement *> &elements);

    /// Slack factor in the per-element evaluation cap. See computeEvaluationCaps().
    static constexpr int kEvalCapSlack = 4;

    /// Absolute backstop on TOTAL evaluations at one timestamp, per element in the netlist.
    /// The per-element cap bounds any single element but is a product -- kEvalCapSlack *
    /// (depth+1) * (|scc|+1) -- so an N-element oscillating component has every member burning
    /// a cap that itself scales with N, making time-to-detect quadratic in component size.
    /// (Measured: inverter rings of 11/51/101/201 took 755/14,274/59,826/222,045 us.) A global
    /// ceiling keeps total detection cost linear in the netlist size, without giving up the
    /// per-element cap's locality, which is what stops unrelated circuitry changing a reading.
    /// Generous on purpose: honest settling needs at most one evaluation per arrival wave per
    /// element, so this leaves well over an order of magnitude of headroom.
    static constexpr int kMaxEvalsPerTimestampPerElement = 256;

    /// Budget on events drained in ONE tick, in TEMPORAL mode only. The drain is otherwise
    /// bounded only by targetTime, so its work grows linearly with the sim-time window -- and
    /// the speed selector defaults to 1x (1,000,000 ns/tick). Measured on the shipped
    /// Examples/temporal_ring_oscillator.panda: 9 us at 1 ns/tick, 182 us at 100, 5,099 us at
    /// 10,000 and 541,461 us at 1,000,000 -- half a second of blocking work inside a 1 ms timer
    /// callback, repeating forever.
    ///
    /// Deliberately NOT applied in functional mode: there every event lands at the current
    /// instant and targetTime never advances, so a tick that stopped early would strand the
    /// circuit half-evaluated with no later tick able to resume it. Functional mode is bounded
    /// by the evaluation caps instead.
    ///
    /// Exceeding the budget is graceful degradation, not an error: the drain stops on a
    /// timestamp boundary, sim-time advances only as far as it actually got, and the next tick
    /// continues. Sim-time then runs slower than the nominal window under load, which is what
    /// every event-driven simulator does and what the on-screen readout already reports.
    static constexpr int kMaxEventsPerTemporalTick = 8192;

    /// The delay \a element pays when PUBLISHING its own newly staged outputs: 0 in functional
    /// mode (every event lands at the current instant), else its own propagation-delay
    /// override/default. Publish-side, not wake-side -- an element is evaluated the instant an
    /// input it reads changes, and pays this delay before that result becomes visible. That is
    /// what lets it sample its inputs at the moment they actually changed.
    SimTime delayTo(const GraphicElement *element) const
    {
        return (m_timePerTick == 0) ? SimTime{0} : m_delays.value(element, 0);
    }

    /// Schedules \a target to re-read its live inputs and STAGE new outputs at \a time,
    /// at its topological priority. Staging only -- nothing becomes visible until the matching
    /// Publish fires (see schedulePublish()).
    void scheduleEvaluate(SimTime time, GraphicElement *target)
    {
        m_eventQueue.schedule({time, m_simPriorities.value(target, -1), target,
                               SimEventKind::Evaluate, 0});
    }

    /// Schedules \a target's staged outputs to be published at \a time, SUPERSEDING any publish
    /// still pending for it. Supersession is what makes the delay model inertial: an input that
    /// reverts within the delay re-stages the already-published value, and this later publish
    /// then changes nothing.
    void schedulePublish(SimTime time, GraphicElement *target)
    {
        const quint64 generation = ++m_publishGeneration[target];
        m_eventQueue.schedule({time, m_simPriorities.value(target, -1), target,
                               SimEventKind::Publish, generation});
    }

    /// Pushes settled logic values onto the wire/port visuals (mirrorICOutputValues() plus phases
    /// 3-4). Split out of update() so the provably-idle path can refresh visuals WITHOUT
    /// draining: a skipped tick still owes a flush when the throttle next comes due.
    void pushVisualStatuses(const QVector<GraphicElement *> &elements,
                            const QVector<GraphicElement *> &outputs);

    /// The unified event-driven engine: a blocking, time-bucketed settle over the event
    /// queue, draining up to \a targetTime. First tick seeds the whole network (\a elements);
    /// subsequent ticks incrementally seed only the sources that changed (\a inputs, \a clocks),
    /// over three regions per timestamp: ACTIVE (combinational settles to a fixed point via
    /// delta cycles), SAMPLE (Memory-group elements read their now-settled inputs and stage),
    /// then PUBLISH (staged values apply and wake their readers). Holding sequential elements
    /// out of the active region is what stops a flip-flop sampling a still-rippling cone;
    /// sampling them all before any publishes is what makes the commit non-blocking. Handles
    /// zero-delay (functional) and propagation-delay (temporal) simulation in one code path.
    /// \a elements/\a inputs/\a clocks are update()'s per-tick snapshots — this engine must
    /// not reach for
    /// m_sortedElements/m_inputs/m_clocks directly, since a reentrant restart() mid-tick
    /// (e.g. triggered by a QMessageBox nested loop from an updateLogic() exception) clears
    /// and rebuilds those members out from under a live range-for.
    void processEvents(SimTime targetTime,
                        const QVector<GraphicElement *> &elements,
                        const QVector<GraphicElementInput *> &inputs,
                        const QVector<Clock *> &clocks);

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
    /// Set whenever phases 1-2 run; cleared once phases 3-4 actually push the result. Lets a
    /// throttled tick that skipped the push know a flush is still owed on the next visualsDue tick.
    bool m_visualsDirty = true;

    // --- Members: Direct simulation graph ---

    QVector<GraphicElement *> m_sortedElements;
    QHash<const GraphicElement *, int> m_simPriorities;
    /// Which cyclic component each feedback element belongs to (-1 / absent = none). Lets an
    /// oscillating region be canonicalised on its own instead of dragging every other cycle in
    /// the circuit down with it.
    QHash<const GraphicElement *, int> m_simFeedbackComponent;
    /// The members of each component, indexed by the id above.
    QVector<QVector<GraphicElement *>> m_simFeedbackComponents;
    /// Per-element bound on evaluations at a single timestamp, derived from the SCC condensation
    /// (see computeEvaluationCaps()). Local by construction: unrelated circuitry cannot move it.
    QHash<const GraphicElement *, int> m_simEvalCaps;

    /// Persisted successor adjacency (element → its successors), built in sortSimElements() by
    /// inverting each flat element's predecessor links (physical, wireless, and spliced IC
    /// boundaries), so it matches exactly the data flow the engine reads in updateLogic().
    QHash<GraphicElement *, QVector<GraphicElement *>> m_successorGraph;

    /// (IC, output port, boundary output Node) triples for mirrorICOutputValues(). Built in
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
    SimTime m_timePerTick = 0;                        ///< 0 ⇒ functional; >0 ⇒ temporal window per tick.
    SimTime m_liveTimeBeforeTimedRun = 0;             ///< Live sim clock captured by beginTimedRun().
    QHash<const GraphicElement *, SimTime> m_delays;  ///< Per-element propagation delay (default 0).
    /// Supersession stamps for pending Publish events, keyed by element. Cleared by restart():
    /// a freed element's address can be reused, and a stale stamp would silently drop the new
    /// element's first publish (same hazard restart() already guards for m_simPriorities).
    QHash<const GraphicElement *, quint64> m_publishGeneration;
    bool m_eventInitDone = false;                     ///< False until the first seed-all baseline settle.
    /// True when the last drain left the event queue empty, i.e. the netlist is at a fixed point
    /// and a tick with no source change would recompute identical values. Cleared by
    /// restart()/initialize() because a structural edit invalidates the conclusion.
    bool m_atFixedPoint = false;
};
