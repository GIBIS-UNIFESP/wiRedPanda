// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// Regression tests for the Sentry cluster headed by WIREDPANDA-H2, re-derived against the
// Quick-side API -- see TestDanglingPointer.h's class doc comment for the full mapping from the
// Widgets original (Tests/Unit/Simulation/TestDanglingPointer.cpp) to QuickCircuitBuilder/
// CanvasItem.
//
// Ordering note: non-crash assertions run first so ctest still reports them even when a later
// death-test tears the process down.

#include "Tests/QuickShell/TestDanglingPointer.h"

#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QVector>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"
#include "Tests/QuickShell/QuickCpuTestUtils.h"

namespace {

// Loads jkflipflop (5 inputs, 2 outputs) into the builder, runs
// QuickCircuitBuilder::initSimulation() so IC::initializeSimulation() populates
// m_sortedInternalElements, and returns the IC. Returns nullptr if the fixture file is missing.
IC *loadInitializedIC(QuickCircuitBuilder &builder)
{
    auto *ic = static_cast<IC *>(builder.addOwnedElement(new IC()));

    const QString icFile = TestUtils::examplesDir() + "jkflipflop.panda";
    if (!QFile::exists(icFile)) {
        return nullptr;
    }
    ic->loadFile(icFile, QFileInfo(icFile).absolutePath());
    builder.initSimulation();
    return ic;
}

} // namespace

void TestDanglingPointer::initTestCase()
{
    // Shared test environment (headless mode, type registration) is set
    // up by the global runner (setupTestEnvironment()) before this slot.
}

// ==========================================================================
// Assertion-based tests — these FAIL pre-fix without crashing the process.
// ==========================================================================

// Bug 1 — IC::resetInternalState() leaves m_sortedInternalElements dangling.
// qDeleteAll(m_internalElements) frees the elements; the sorted vector that
// references those same objects is never cleared, leaving every entry as
// a freed pointer.
void TestDanglingPointer::bug1_resetInternalStateMustClearSortedVector()
{
    QuickCircuitBuilder builder;
    auto *ic = loadInitializedIC(builder);
    QVERIFY2(ic, "jkflipflop.panda not found in examples — fixture missing");
    QVERIFY(!ic->m_internalElements.isEmpty());
    QVERIFY(!ic->m_sortedInternalElements.isEmpty());

    ic->resetInternalState();

    // After the fix, every piece of state referring to m_internalElements
    // must be cleared in the same atomic step.
    QCOMPARE(ic->m_internalElements.size(), 0);
    QCOMPARE(ic->m_sortedInternalElements.size(), 0); // FAILS pre-fix
    QCOMPARE(ic->m_boundaryInputElements.size(), 0);
    QVERIFY(!ic->m_internalHasFeedback);
}

// Bug 3 — IC::loadFile() PATH 2 calls resetInternalState() BEFORE
// loadFileDirectly(), which throws on corrupt input. The throw leaves
// m_internalElements empty but m_sortedInternalElements dangling.
//
// PATH 2 is only reached when qobject_cast<Scene *>(scene()) returns nullptr -- always true
// here, since GraphicElement no longer has a Scene concept at all.
void TestDanglingPointer::bug3_failedLoadMustLeaveConsistentState()
{
    auto ic = std::make_unique<IC>();

    const QString icFile = TestUtils::examplesDir() + "jkflipflop.panda";
    QVERIFY2(QFile::exists(icFile),
             qPrintable(QString("IC fixture not found: %1").arg(icFile)));
    ic->loadFile(icFile, QFileInfo(icFile).absolutePath());
    ic->initializeSimulation();
    QVERIFY(!ic->m_internalElements.isEmpty());
    QVERIFY(!ic->m_sortedInternalElements.isEmpty());

    // Empty .panda file: exists + opens but readPandaHeader falls through
    // all three format paths (no magic, empty legacy string, null-center
    // clipboard read) and throws "Invalid file format." at
    // Serialization.cpp:59. Arbitrary garbage bytes can unfortunately
    // parse as a legacy clipboard stream when the leading bytes happen to
    // decode to a non-null QPointF — an empty file is the one guaranteed-
    // to-throw input.
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QString corruptPath = tmp.filePath("corrupt.panda");
    {
        QFile f(corruptPath);
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
        f.close();
    }

    bool threw = false;
    try {
        ic->loadFile(corruptPath, tmp.path());
    } catch (const std::exception &) {
        threw = true;
    }
    QVERIFY2(threw, "loadFile() on corrupt bytes must throw");

    // Invariant: either rollback (both populated) or clean reset (both empty).
    // Pre-fix the two diverge: internals cleared, sorted dangling.
    const bool elementsEmpty = ic->m_internalElements.isEmpty();
    const bool sortedEmpty = ic->m_sortedInternalElements.isEmpty();
    QVERIFY2(elementsEmpty == sortedEmpty,
             qPrintable(QString("Inconsistent post-throw state: "
                                "m_internalElements.isEmpty()=%1 vs "
                                "m_sortedInternalElements.isEmpty()=%2")
                            .arg(elementsEmpty)
                            .arg(sortedEmpty)));
}

// Bug 4 — Simulation::restart() must not leave a stale m_sortedElements
// behind. The pre-fix body was `m_initialized = false;` only — the hot-
// path vectors kept pointing at whatever they held before, so any tick
// that ran before the next initialize() faulted on the stale entries.
void TestDanglingPointer::bug4_restartMustClearStaleTopology()
{
    QuickCircuitBuilder builder;
    auto *ic = loadInitializedIC(builder);
    QVERIFY2(ic, "jkflipflop.panda not found in examples — fixture missing");
    auto *sim = builder.simulation();
    QVERIFY(sim->m_initialized);
    QVERIFY(!sim->m_sortedElements.isEmpty());

    sim->restart();

    // Invariant: m_initialized=true implies m_sortedElements reflects the
    // current topology. Pre-fix, restart() broke this: initialized=false
    // with a populated, now-stale m_sortedElements.
    QVERIFY2(sim->m_initialized || sim->m_sortedElements.isEmpty(),
             qPrintable(QString("Simulation::restart() left torn state: "
                                "m_initialized=%1, m_sortedElements.size()=%2")
                            .arg(sim->m_initialized)
                            .arg(sim->m_sortedElements.size())));
}

// Bug 5 — the invariant "m_initialized=true implies m_sortedElements is non-empty" must hold
// even after Simulation::initialize() early-returns on a host with nothing left to simulate.
// Re-targeted at Simulation::initialize()/update() directly: Simulation::initialize()'s real
// early-return condition is `if (elements.empty()) return false;` (confirmed by reading it),
// which is SimulationHost-generic -- it no longer needs Scene's old "border rect always counts
// as one item" workaround the original Widgets test relied on.
void TestDanglingPointer::bug5_setCircuitUpdateRequiredMustNotLieAboutInit()
{
    QuickCircuitBuilder builder;

    // Load a real circuit — initialize() succeeds, m_initialized=true,
    // m_sortedElements populated.
    auto *ic = loadInitializedIC(builder);
    QVERIFY2(ic, "jkflipflop.panda not found in examples — fixture missing");
    auto *sim = builder.simulation();
    QVERIFY(sim->m_initialized);
    QVERIFY(!sim->m_sortedElements.isEmpty());

    // Remove the only element so the host has nothing left to simulate and
    // initialize() bails at its early return.
    builder.removeElement(ic);
    delete ic;

    sim->restart();
    sim->update(); // update() lazily re-runs initialize() while m_initialized is false

    // Invariant: m_initialized must match the actual state — either it is
    // false (forcing the next update() to re-run initialize) OR
    // m_sortedElements is non-empty. The bug left them disagreeing:
    // initialized=true, sortedElements=empty.
    const bool initialized = sim->m_initialized;
    const bool hasSorted = !sim->m_sortedElements.isEmpty();
    QVERIFY2(!initialized || hasSorted,
             qPrintable(QString("Simulation left m_initialized=%1 with "
                                "m_sortedElements.isEmpty()=%2 — stale "
                                "'initialized' flag after a failed initialize().")
                            .arg(initialized)
                            .arg(!hasSorted)));
}

// Bug 6 — Source-level check. A runtime reproduction requires the
// Application::notify QMessageBox nested-event-loop window, which can't
// be triggered reliably inside QtTest. Instead, assert that each
// topology-mutating command opens a SimulationBlocker scope in its
// redo() and undo().
void TestDanglingPointer::bug6_topologyCommandsMustUseSimulationBlocker()
{
    // CURRENTDIR is defined at compile time (App/QuickShell/CMakeLists.txt's own
    // target_compile_definitions), stable across how the test is invoked.
    const QString commandsCpp =
        QString(QUOTE(CURRENTDIR)) + "/../App/QuickShell/Canvas/CanvasCommands.cpp";
    QFile src(commandsCpp);
    QVERIFY2(src.open(QIODevice::ReadOnly),
             qPrintable(QString("Cannot open %1").arg(src.fileName())));
    const QString source = QString::fromUtf8(src.readAll());
    src.close();

    // Extract the body of a given `Class::method` implementation. Matches
    // the function header, then balanced braces to its closing `}`.
    auto bodyOf = [&source](const QString &qualifiedName) -> QString {
        const QString pattern =
            QStringLiteral("\\b") + QRegularExpression::escape(qualifiedName)
            + QStringLiteral("\\s*\\([^)]*\\)\\s*\\{");
        QRegularExpression rx(pattern);
        const auto match = rx.match(source);
        if (!match.hasMatch()) return {};

        const qsizetype start = match.capturedEnd() - 1; // at the `{`
        int depth = 0;
        for (qsizetype i = start; i < source.size(); ++i) {
            const QChar c = source.at(i);
            if (c == '{') ++depth;
            else if (c == '}') {
                --depth;
                if (depth == 0) return source.mid(start, i - start + 1);
            }
        }
        return {};
    };

    const QStringList unsafeCommands = {
        "CanvasUpdateCommand::redo",
        "CanvasUpdateCommand::undo",
        "CanvasMorphCommand::redo",
        "CanvasMorphCommand::undo",
        "CanvasChangePortSizeCommand::redo",
        "CanvasChangePortSizeCommand::undo",
        "CanvasSplitCommand::redo",
        "CanvasSplitCommand::undo",
    };

    QStringList missingBlocker;
    for (const QString &name : unsafeCommands) {
        const QString body = bodyOf(name);
        if (body.isEmpty()) {
            // If we can't find the function, don't silently pass — record it.
            missingBlocker << (name + " (function body not located)");
            continue;
        }
        if (!body.contains("SimulationBlocker")) {
            missingBlocker << name;
        }
    }

    QVERIFY2(missingBlocker.isEmpty(),
             qPrintable(QString("The following topology-mutating command methods "
                                "must open a SimulationBlocker scope in their "
                                "body to prevent re-entrancy from ticking on torn state:\n  - %1")
                            .arg(missingBlocker.join("\n  - "))));
}

// Hardening — CanvasItem::cancelEditedWire() deletes the in-progress wire directly, same shape
// as the Widgets original's ConnectionManager::deleteEditedConnection. Behavioral check: the
// in-progress wire is never registered via addItem()/simulationItems() (confirmed by reading
// startWireFromOutput()/cancelEditedWire()), so there is nothing for the 1 ms simulation timer to
// dangle a reference to — starting and cancelling a wire while the simulation is running must not
// crash or corrupt the circuit.
void TestDanglingPointer::hardening_deleteEditedConnectionMustUseSimulationBlocker()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    canvas.addItem(sw);
    canvas.addItem(led);

    auto *realConn = new Connection();
    realConn->setStartPort(sw->outputPort(0));
    realConn->setEndPort(led->inputPort(0));
    canvas.addItem(realConn);
    canvas.restartSimulation();

    canvas.simulation()->start();
    QVERIFY(canvas.simulation()->isRunning());

    // Start dragging a wire from the switch's (already-connected) output, then cancel it.
    canvas.startWireFromOutput(sw->outputPort(0));
    canvas.cancelEditedWire();

    // The real connection must be unaffected and the simulation must survive.
    QVERIFY(canvas.simulation()->isRunning());
    static_cast<InputSwitch *>(sw)->setOn(true);
    canvas.simulation()->update();
    QCOMPARE(TestUtils::inputStatus(led), true);

    canvas.simulation()->stop();
}

// WIREDPANDA-JD — historical: Simulation::initialize() used to collect every Connection
// unconditionally, including in-progress wires (only startPort set). Structurally impossible now
// (CanvasItem's ListSimulationHost::simulationItems() only ever returns m_elements, confirmed by
// reading it) -- this regression-tests the real production API anyway: an in-progress wire
// coexisting with a real connection during a re-initialize must not corrupt or crash the
// simulation, and the real connection must keep propagating correctly.
void TestDanglingPointer::jd_initializeMustSkipIncompleteConnections()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    canvas.addItem(sw);
    canvas.addItem(led);

    auto *conn = new Connection();
    conn->setStartPort(sw->outputPort(0));
    conn->setEndPort(led->inputPort(0));
    canvas.addItem(conn);

    canvas.simulation()->initialize();
    QVERIFY(canvas.simulation()->m_initialized);

    // Start (but don't finish) dragging a second wire off the same output port -- mirrors a user
    // mid-drag when some other structural edit forces a re-initialize.
    canvas.startWireFromOutput(sw->outputPort(0));

    canvas.simulation()->initialize();
    QVERIFY(canvas.simulation()->m_initialized);

    static_cast<InputSwitch *>(sw)->setOn(true);
    canvas.simulation()->update();
    QVERIFY(TestUtils::inputStatus(led));

    // Cancel the in-progress wire and tick again — must not crash, and the real connection must
    // still function.
    canvas.cancelEditedWire();

    static_cast<InputSwitch *>(sw)->setOn(false);
    canvas.simulation()->update();
    QVERIFY(!TestUtils::inputStatus(led));
}

// ==========================================================================
// Crash-triggering tests — these SIGSEGV pre-fix. Kept at the end so they
// don't prevent the assertion tests above from reporting.
// ==========================================================================

// Bug 8 — iterativeSettle() iterates its input without null checks.
void TestDanglingPointer::bug8_iterativeSettleMustTolerateNullEntry()
{
    QVector<GraphicElement *> entries;
    entries.append(nullptr);
    // Return value is meaningless for a null input; the point is survival.
    (void)Simulation::iterativeSettle(entries, 1);
    QVERIFY(true);
}

// Hardening — Simulation::update() Phase 3 walks m_sortedElements and reads
// each element's output ports; a null entry must not crash. Inject a null
// and tick the simulation to verify the existing `if (element)` guard holds.
void TestDanglingPointer::hardening_phase3MustTolerateNullElement()
{
    QuickCircuitBuilder builder;
    auto *ic = loadInitializedIC(builder);
    QVERIFY2(ic, "jkflipflop.panda not found in examples — fixture missing");
    auto *sim = builder.simulation();
    QVERIFY(sim->m_initialized);

    // Phase 3 only runs when the visual throttle lets through; force-disable
    // so update() actually walks m_sortedElements this tick instead of
    // skipping straight back to Phase 2.
    sim->setVisualThrottleEnabled(false);

    sim->m_sortedElements.append(nullptr);

    sim->update();
    QVERIFY(true);
}

// Bug 2 — IC::updateLogic() dereferences m_sortedInternalElements without
// a null guard. Inject a null directly and call updateLogic.
void TestDanglingPointer::bug2_icUpdateLogicMustTolerateNullEntry()
{
    QuickCircuitBuilder builder;
    auto *ic = loadInitializedIC(builder);
    QVERIFY2(ic, "jkflipflop.panda not found in examples — fixture missing");
    QVERIFY(!ic->m_sortedInternalElements.isEmpty());

    // Inject a nullptr — mirrors the dangling state left by Bug 1/Bug 3
    // but deterministically, without relying on heap poisoning patterns.
    const qsizetype mid = ic->m_sortedInternalElements.size() / 2;
    ic->m_sortedInternalElements.insert(mid, nullptr);

    ic->updateLogic();  // pre-fix: null deref
    QVERIFY(true);      // reaching here is the pass condition
}

// Hardening — CanvasICRegistry::embedICsByFile()/extractToFile() reload live, already-in-scene
// ICs via ic->loadFromBlob()/loadFile(), the same H2 use-after-free shape as bug1/bug2/bug3: an
// IC's internal flip-flop elements freed by resetInternalState() while Simulation still points at
// them, unless the 1 ms timer is stopped for the duration. Fixed by routing both functions
// through the shared CanvasICRegistry::reloadTargetsAtomically() helper. Source-level check, same
// shape as bug6, for the same reason: the real reentrancy window can't be triggered
// deterministically inside single-threaded QtTest.
void TestDanglingPointer::hardening_icRegistryReloadHelpersMustUseSimulationBlocker()
{
    const QString path =
        QString(QUOTE(CURRENTDIR)) + "/../App/QuickShell/Canvas/CanvasICRegistry.cpp";
    QFile src(path);
    QVERIFY2(src.open(QIODevice::ReadOnly),
             qPrintable(QString("Cannot open %1").arg(src.fileName())));
    const QString source = QString::fromUtf8(src.readAll());
    src.close();

    auto bodyOf = [&source](const QString &qualifiedName) -> QString {
        // [\s\S]*? (lazy, spans newlines) rather than [^)]* — reloadTargetsAtomically's own
        // parameter list contains std::function<void(IC *)>, a nested closing paren that a
        // negated-character-class match can't see past to find the parameter list's real end.
        const QString pattern =
            QStringLiteral("\\b") + QRegularExpression::escape(qualifiedName)
            + QStringLiteral("\\s*\\([\\s\\S]*?\\)\\s*\\{");
        QRegularExpression rx(pattern);
        const auto match = rx.match(source);
        if (!match.hasMatch()) return {};

        const qsizetype start = match.capturedEnd() - 1;
        int depth = 0;
        for (qsizetype i = start; i < source.size(); ++i) {
            const QChar c = source.at(i);
            if (c == '{') ++depth;
            else if (c == '}') {
                --depth;
                if (depth == 0) return source.mid(start, i - start + 1);
            }
        }
        return {};
    };

    const QString helperBody = bodyOf("CanvasICRegistry::reloadTargetsAtomically");
    QVERIFY2(!helperBody.isEmpty(), "Could not locate CanvasICRegistry::reloadTargetsAtomically definition.");
    QVERIFY2(helperBody.contains("SimulationBlocker"),
             "CanvasICRegistry::reloadTargetsAtomically must open a SimulationBlocker "
             "scope around its whole loop over targets, not just each individual "
             "mutation.");

    const QStringList reloadCallers = {
        "CanvasICRegistry::embedICsByFile",
        "CanvasICRegistry::extractToFile",
    };
    QStringList missingCall;
    for (const QString &name : reloadCallers) {
        const QString body = bodyOf(name);
        if (body.isEmpty()) {
            missingCall << (name + " (function body not located)");
            continue;
        }
        if (!body.contains("reloadTargetsAtomically")) {
            missingCall << name;
        }
    }

    QVERIFY2(missingCall.isEmpty(),
             qPrintable(QString("The following functions mutate live, already-in-scene "
                                "IC elements and must go through reloadTargetsAtomically() "
                                "rather than mutating them directly:\n  - %1")
                            .arg(missingCall.join("\n  - "))));
}

// Combined integration reproduction of WIREDPANDA-H2. Same tick path the
// production crash took: outer Simulation::update() → element->updateLogic()
// on the IC → IC::updateLogic() iterates m_sortedInternalElements → reads
// the vtable of a freed entry.
void TestDanglingPointer::integration_simulationTickAfterResetMustNotCrash()
{
    QuickCircuitBuilder builder;
    auto *ic = loadInitializedIC(builder);
    QVERIFY2(ic, "jkflipflop.panda not found in examples — fixture missing");
    QVERIFY(!ic->m_sortedInternalElements.isEmpty());

    // Snap-freeze the crash state: one null sentinel in the sorted vector,
    // mirroring the dangling state Bug 1 / Bug 3 produce, deterministically.
    const qsizetype mid = ic->m_sortedInternalElements.size() / 2;
    ic->m_sortedInternalElements.insert(mid, nullptr);

    builder.simulation()->update();
    QVERIFY(true);
}

// WIREDPANDA-JD — historical crash reproduction, kept as defense in depth, via CanvasItem's real
// wire-drag-and-cancel API this time. An in-progress wire (startPort only, no endPort) coexists
// with a real connection, a re-initialize runs while it's present, then it's cancelled. The next
// full update() must not crash.
void TestDanglingPointer::jd_cancelledWireMustNotLeaveDanglingPointer()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);

    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    canvas.addItem(sw);
    canvas.addItem(led);

    auto *conn = new Connection();
    conn->setStartPort(sw->outputPort(0));
    conn->setEndPort(led->inputPort(0));
    canvas.addItem(conn);

    canvas.simulation()->initialize();
    QVERIFY(canvas.simulation()->m_initialized);
    canvas.simulation()->setVisualThrottleEnabled(false);

    // Start an in-progress wire (startPort only) off the same output.
    canvas.startWireFromOutput(sw->outputPort(0));

    // Re-initialize while the in-progress wire is present.
    canvas.simulation()->initialize();

    // Cancel the in-progress wire (mirrors deleteEditedConnection's bare delete).
    canvas.cancelEditedWire();

    // Must not crash.
    canvas.simulation()->update();
    QVERIFY(true);
}

// WIREDPANDA-HC — when an element is destroyed out-of-band (no command,
// no removeItem on attached wires first), Qt's ~GraphicElement cascade fires
// drainConnections on each child port. Pre-fix, drainConnections issued a
// bare `delete conn` that bypassed CanvasItem::removeItem(Connection*), leaving a stale
// id-registry entry pointing at freed memory — the upstream necessary condition for the
// FH/EW/G1/GP/HC paint-time _purecall family.
void TestDanglingPointer::hcDrainConnectionsMustCleanRegistry()
{
    CanvasItem canvas(nullptr, /*buildDemo=*/false);
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *led = ElementFactory::buildElement(ElementType::Led);
    canvas.addItem(sw);
    canvas.addItem(led);

    auto *conn = new Connection();
    conn->setStartPort(sw->outputPort(0));
    conn->setEndPort(led->inputPort(0));
    canvas.addItem(conn);

    const int connId = conn->id();
    QCOMPARE(canvas.itemById(connId), conn);

    // Out-of-band destruction: removeItem on the element only, then delete.
    // Cascade reaches ~OutputPort → drainConnections, which deletes the
    // wire still registered under connId.
    canvas.removeItem(sw);
    delete sw;

    // Post-fix: drainConnections routed the delete through CanvasItem::removeItem(),
    // so the registry entry is gone.
    QVERIFY(canvas.itemById(connId) == nullptr);
}
