// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestSimulationUnit : public QObject
{
    Q_OBJECT

private slots:

    void testSimulationWithNoElements();
    void testAddRemoveClockDuringSimulation();
    void testElementRemovalMidSimulation();

    // Regression: B22 — Simulation::start/stop must not emit Sentry breadcrumbs
    void testSimulationStartStopNoBreadcrumbsB22();

    // Regression: Phase 3 must refresh output-port visuals even with no wire attached
    void testUnconnectedOutputPortVisualUpdates();

    void testInitializeReturnsFalseWithNoHost();
    void testInitializeSkipsNullItemsAndFailsWithNoElements();
    void testUpdatePortWithNullPortsAreNoOps();
    void testUpdateFlushesPendingVisualsOnLaterIdleTick();
    /// The provably-idle tick skip: once a drain leaves the event queue empty, a tick with no
    /// source change must not recompute. Pinned as a state-machine assertion rather than left
    /// to the flush assertions, which pass whether or not the skip happens.
    void testIdleTicksAreSkippedOnceAtFixedPoint();
    /// Anything that forces a re-seed must clear the fixed-point flag, or the re-seed itself is
    /// skipped as "idle" and the network never settles. Clearing the queue and m_eventInitDone
    /// without the flag is enough to lose the first timed tick, which collapses every temporal
    /// delay measurement built on it.
    void testForcedReseedClearsFixedPointFlag();

    /// restart() documents that "any future cached state added to Simulation must be cleared
    /// above" and asserts it. Every pointer-keyed container is checked BY NAME here --
    /// m_simFeedbackComponent and m_simFeedbackComponents hold raw GraphicElement* vectors that
    /// canonicalizeOscillation() dereferences -- so the next container added fails here rather
    /// than silently outliving a rebuild.
    void testRestartClearsEveryPointerKeyedContainer();
    /// initialize() can bail out early after already clearing the topology vectors, so it must
    /// leave exactly what restart() leaves -- never m_initialized standing beside containers
    /// describing the previous circuit.
    void testFailedInitializeLeavesNoStaleTopology();
    void testEvaluationCapGrowsAlongEveryCrossComponentEdge();
    /// The guard's destructor must restore the throttle state it found, not enable it: an inner
    /// guard would otherwise turn the throttle back on inside an outer guard's scope.
    void testNestedThrottleDisablersRestoreRatherThanEnable();

    /// An IC's externally visible value must be correct as soon as the drain settles, not when
    /// the visual throttle next fires. mirrorICOutputValues() is the only thing that writes
    /// ic->outputValue() -- the IC container is excluded from the flat netlist -- so running it
    /// inside pushVisualStatuses() would put a logic value behind a presentation gate. The rest
    /// of the suite cannot see that: Application::interactiveMode is false in tests, which makes
    /// every tick visuals-due.
    void testIcOutputValueIsFreshBeforeTheVisualPush();

    /// m_atFixedPoint must be cleared on ENTRY to the drain, not only assigned at the end of a
    /// successful one: an exception escaping updateLogic() would otherwise leave it holding the
    /// previous tick's `true` while the queue is non-empty. Application::notify() catches and
    /// the app keeps ticking, so every later tick with no source change would be skipped as idle
    /// WITH EVENTS PENDING -- the circuit stalling silently until the user touches an input.
    void testAbortedDrainDoesNotLeaveTheFixedPointFlagSet();

    /// Types whose default propagation delay is 0 are delay-free by design (sources, sinks,
    /// Nodes, ICs), and every writer refuses to set one on them -- the editor hides the field,
    /// UpdateCommand skips the push, MCP rejects it. Reading the resolved value ungated would
    /// let a hand-edited .panda inject a delay the engine honours and no UI ever shows.
    void testDelayFreeTypesIgnoreAPropagationDelayOverride();

    /// A SimulationBlocker pause/resume cycle (every UpdateCommand redo/undo, including a
    /// plain InputSwitch click) must not force clocks HIGH or restart their phase.
    void testBlockerCyclePreservesClockLevel();

    /// beginTimedRun() restarts the timeline and forces a re-seed; endTimedRun() restores the
    /// previous window and the live clock, and drops events left queued past the swept window.
    void testTimedRunBracketResetsAndRestores();
};
