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

    /// Hardening — CanvasItem::cancelEditedWire() deletes the in-progress wire
    /// directly, mirroring ConnectionManager::deleteEditedConnection's shape.
    /// Behavioral check, not a source-level one: the in-progress wire is never
    /// registered via addItem()/simulationItems(), so there is nothing for the
    /// simulation timer to dangle a reference to -- starting and cancelling a
    /// wire while the simulation is running must not crash or corrupt the circuit.
    void hardening_deleteEditedConnectionMustUseSimulationBlocker();

    /// WIREDPANDA-JD — an in-progress wire (only startPort set) must be able to
    /// coexist with a real connection across a re-initialize without corrupting
    /// or crashing the simulation.
    void jd_initializeMustSkipIncompleteConnections();

    // --- Crash-triggering tests (process dies pre-fix) ---------------

    /// Bug 8 — iterativeSettle() iterates without null checks.
    void bug8_iterativeSettleMustTolerateNullEntry();

    /// Hardening — Simulation::update() Phase 3 walks m_sortedElements, which
    /// can include unconnected output ports. Inject a null entry and call
    /// update() to verify the existing guard tolerates it.
    void hardening_phase3MustTolerateNullElement();

    /// Bug 2 — IC::updateLogic() dereferences m_sortedInternalElements
    /// entries without a null guard.
    void bug2_icUpdateLogicMustTolerateNullEntry();

    /// Bug 7 — ICRegistry::onFileChanged() reloads without a
    /// SimulationBlocker and uses restart() (Bug 4) instead of a full
    /// setCircuitUpdateRequired(). Drives the full H2 crash shape via
    /// the file-watcher path.
    void bug7_icRegistryFileChangedMustNotLeaveDanglingPointers();

    /// Hardening — embedICsByFile()/extractToFile() had no SimulationBlocker
    /// at all (unlike their sibling onFileChanged()/bug7), despite mutating
    /// live, already-in-scene ICs the same way. Source-level check, same
    /// shape as bug6/bug7.
    void hardening_icRegistryReloadHelpersMustUseSimulationBlocker();

    /// WIREDPANDA-HC — Port::drainConnections issued a bare
    /// `delete conn` during cascade-destruction. Structurally fixed at the
    /// ItemWithId base-class level (self-unregisters from whichever
    /// SceneItemRegistry it's mapped in, on ANY destruction path) rather
    /// than at each individual delete call site.
    void hcDrainConnectionsMustCleanRegistry();

    /// WIREDPANDA-JD — defense in depth. An in-progress wire (startPort only)
    /// survives a re-initialize and is then deleted without rebuilding the
    /// simulation; the next update() must not crash or dereference the freed wire.
    void jd_cancelledWireMustNotLeaveDanglingPointer();

    /// Combined H2 reproduction — sits at the end so the process crash
    /// only affects this last slot. Covers the concrete path that
    /// Simulation.cpp's `element->updateLogic()` took in production.
    void integration_simulationTickAfterResetMustNotCrash();
};
