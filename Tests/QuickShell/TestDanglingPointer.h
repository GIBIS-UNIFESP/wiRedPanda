// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/**
 * \class TestDanglingPointer
 * \brief Regression tests for the Sentry cluster anchored by WIREDPANDA-H2, re-derived against
 * the Quick-side API (QuickCircuitBuilder / CanvasItem) rather than mechanically transliterated
 * from the Widgets original (Tests/Unit/Simulation/TestDanglingPointer.cpp).
 *
 * \details One test per bug enumerated in .claude/SENTRY_CRASH_ANALYSIS_WIREDPANDA-H2.md, minus
 * bug7_icRegistryFileChangedMustNotLeaveDanglingPointers: it regression-tested
 * ICRegistry::onFileChanged()'s QFileSystemWatcher-driven hot-reload, and CanvasICRegistry.h's
 * own doc comment confirms file watching is deliberately not ported yet ("this canvas has no
 * file-backed-IC live-reload concept yet") -- there is no Quick-side function left to check.
 * Everything else maps to a real Quick-side equivalent: Scene::setCircuitUpdateRequired() calls
 * map to Simulation::restart()/update() directly (QuickCircuitBuilder has no Scene wrapping it);
 * App/Scene/Commands.cpp's UpdateCommand/MorphCommand/ChangePortSizeCommand/SplitCommand map to
 * App/QuickShell/Canvas/CanvasCommands.cpp's CanvasUpdateCommand/CanvasMorphCommand/
 * CanvasChangePortSizeCommand/CanvasSplitCommand; ConnectionManager::deleteEditedConnection and
 * the old Scene-tracked in-progress-wire mechanism map to CanvasItem::cancelEditedWire()/
 * startWireFromOutput()/tryCompleteWire(); App/Scene/ICRegistry.cpp's reload helpers map to
 * App/QuickShell/Canvas/CanvasICRegistry.cpp's identically-named ones.
 *
 * Non-crash assertion tests run first so ctest still reports them even when a later death test
 * tears the process down.
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

    /// Bug 5 — the invariant "m_initialized=true ⇒ m_sortedElements is non-empty" must hold
    /// even after Simulation::initialize() early-returns on an empty host. Re-targeted at
    /// Simulation::initialize()/update() directly (SimulationHost-generic, no Scene involved --
    /// confirmed by reading Simulation::initialize()'s own `if (elements.empty()) return false;`
    /// early-return, which no longer depends on a Scene-specific "border rect" item count the
    /// way the pre-fix Scene::setCircuitUpdateRequired() bug did).
    void bug5_setCircuitUpdateRequiredMustNotLieAboutInit();

    /// Bug 6 — CanvasUpdateCommand / CanvasMorphCommand / CanvasChangePortSizeCommand /
    /// CanvasSplitCommand must use SimulationBlocker inside their redo() and undo() bodies.
    /// Source-level check because single-threaded QtTest does not exhibit the reentrancy the
    /// production crash relies on.
    void bug6_topologyCommandsMustUseSimulationBlocker();

    /// Hardening — CanvasItem::cancelEditedWire() deletes the in-progress wire directly, same
    /// shape as the Widgets original's ConnectionManager::deleteEditedConnection. Behavioral
    /// check (not source-level, unlike its Widgets counterpart): starting and cancelling a wire
    /// while the simulation is running must not crash or corrupt the circuit.
    void hardening_deleteEditedConnectionMustUseSimulationBlocker();

    /// WIREDPANDA-JD — historical: Simulation::initialize() used to collect every Connection
    /// unconditionally, including in-progress wires. Structurally impossible now (confirmed by
    /// reading CanvasItem's ListSimulationHost::simulationItems(), which only ever returns
    /// m_elements, never m_connections) -- this regression-tests the real production API anyway:
    /// an in-progress wire coexisting with a real connection during a re-initialize must not
    /// corrupt or crash the simulation.
    void jd_initializeMustSkipIncompleteConnections();

    // --- Crash-triggering tests (process dies pre-fix) ---------------

    /// Bug 8 — iterativeSettle() iterates without null checks.
    void bug8_iterativeSettleMustTolerateNullEntry();

    /// Hardening — Simulation::update() Phase 3 now walks m_sortedElements
    /// (rewritten to cover unconnected output ports too). Inject a null
    /// entry and call update() to verify the existing guard tolerates it.
    void hardening_phase3MustTolerateNullElement();

    /// Bug 2 — IC::updateLogic() dereferences m_sortedInternalElements
    /// entries without a null guard.
    void bug2_icUpdateLogicMustTolerateNullEntry();

    /// Hardening — CanvasICRegistry::embedICsByFile()/extractToFile() must reload live,
    /// already-in-scene ICs through the shared reloadTargetsAtomically() helper (which opens a
    /// SimulationBlocker around the whole loop), not by mutating them directly. Source-level
    /// check, same shape as bug6.
    void hardening_icRegistryReloadHelpersMustUseSimulationBlocker();

    /// Combined H2 reproduction — sits at the end so the process crash
    /// only affects this last slot. Covers the concrete path that
    /// Simulation.cpp's `element->updateLogic()` took in production.
    void integration_simulationTickAfterResetMustNotCrash();

    /// WIREDPANDA-JD — historical crash reproduction, kept as defense in depth, via CanvasItem's
    /// real wire-drag-and-cancel API this time (startWireFromOutput()/detachWire()) rather than
    /// hand-built Connection objects. The next update() must not crash.
    void jd_cancelledWireMustNotLeaveDanglingPointer();

    /// WIREDPANDA-HC — Port::drainConnections issued (and still issues) a bare `delete conn`
    /// during cascade-destruction, bypassing CanvasItem::removeItem(Connection*). Out-of-band
    /// destruction (delete an element directly, no command, no removeItem on its connections
    /// first) must not leave CanvasItem's id registry pointing at freed memory.
    void hcDrainConnectionsMustCleanRegistry();
};
