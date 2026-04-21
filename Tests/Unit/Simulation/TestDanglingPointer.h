// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/**
 * \class TestDanglingPointer
 * \brief Regression tests for the Sentry cluster anchored by WIREDPANDA-H2.
 *
 * \details One test per bug enumerated in
 * \c .claude/SENTRY_CRASH_ANALYSIS_WIREDPANDA-H2.md. Each test fails
 * (assertion or SIGSEGV) on the current tree and passes once the matching
 * fix lands. Non-crash assertion tests run first so ctest still reports
 * them even when a later death test tears the process down.
 */
class TestDanglingPointer : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    // --- Assertion-based tests (safe: no SIGSEGV pre-fix) -------------

    /// Bug 1 — IC::resetInternalState() must clear every vector it owns,
    /// not just m_internalElements.
    void bug1_resetInternalStateMustClearSortedVector();

    /// Bug 3 — IC::loadFile() PATH 2 resets internals before loadFileDirectly()
    /// can throw. After a failed load, m_sortedInternalElements must stay
    /// in sync with m_internalElements.
    void bug3_failedLoadMustLeaveConsistentState();

    /// Bug 4 — Simulation::restart() currently only flips m_initialized,
    /// leaving m_sortedElements populated with pre-restart pointers. The
    /// invariant "m_initialized=true ⇒ m_sortedElements is the current
    /// topology" must hold after restart().
    void bug4_restartMustClearStaleTopology();

    /// Bug 5 — Scene::setCircuitUpdateRequired() ignores initialize()'s
    /// return value, leaving m_initialized=true when the sorted vector is
    /// empty after an early-return.
    void bug5_setCircuitUpdateRequiredMustNotLieAboutInit();

    /// Bug 6 — UpdateCommand / MorphCommand / ChangePortSizeCommand /
    /// SplitWireCommand / SpliceNodeCommand must use SimulationBlocker
    /// inside their redo() and undo() bodies. Implemented as a source-
    /// level check because single-threaded QtTest does not exhibit the
    /// reentrancy the production crash relies on.
    void bug6_topologyCommandsMustUseSimulationBlocker();

    // --- Crash-triggering tests (process dies pre-fix) ---------------

    /// Bug 8 — iterativeSettle() iterates without null checks.
    void bug8_iterativeSettleMustTolerateNullEntry();

    /// Bug 2 — IC::updateLogic() dereferences m_sortedInternalElements
    /// entries without a null guard.
    void bug2_icUpdateLogicMustTolerateNullEntry();

    /// Bug 7 — ICRegistry::onFileChanged() reloads without a
    /// SimulationBlocker and uses restart() (Bug 4) instead of a full
    /// setCircuitUpdateRequired(). Drives the full H2 crash shape via
    /// the file-watcher path.
    void bug7_icRegistryFileChangedMustNotLeaveDanglingPointers();

    /// Combined H2 reproduction — sits at the end so the process crash
    /// only affects this last slot. Covers the concrete path that
    /// Simulation.cpp:87's `element->updateLogic()` took in production.
    void integration_simulationTickAfterResetMustNotCrash();
};
