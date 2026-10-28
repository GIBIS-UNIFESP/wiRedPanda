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
    void testCollectSequentialElementsSkipsNullElements();
    void testUpdateFlushesPendingVisualsOnLaterIdleTick();

    /// A SimulationBlocker pause/resume cycle (every UpdateCommand redo/undo, including a
    /// plain InputSwitch click) must not force clocks HIGH or restart their phase.
    void testBlockerCyclePreservesClockLevel();

    /// A rebuild after a structural edit keeps each clock's level and phase; a restart starts them afresh.
    void testRebuildKeepsClockLevel();

    /// A resume after a structural edit made while paused shifts the clocks by the pause, as a plain resume does.
    void testResumeAfterAPausedRebuildKeepsClockPhase();

    /// A rebuild keeps the outputs of the elements it keeps, so a flip-flop keeps its Q; a restart starts it afresh.
    void testRebuildKeepsFlipFlopState_data();
    void testRebuildKeepsFlipFlopState();
};
