// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// Regression tests for the WIREDPANDA-H2 use-after-free bug cluster in IC/Simulation
// dangling-pointer handling.
//
// Ordering note: non-crash assertions run first so ctest still reports
// them even when a later death-test tears the process down.

#include "Tests/QuickShell/TestDanglingPointer.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaObject>
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
#include "Tests/QuickShell/IC/QuickTestUtils.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"

using QuickTestUtils::inputStatus;

namespace {

// Loads jkflipflop (5 inputs, 2 outputs) into the builder, runs the outer
// Simulation::initialize() so IC::initializeSimulation() populates
// m_sortedInternalElements, and returns the IC. Returns nullptr if the
// fixture file is missing.
IC *loadInitializedIC(QuickCircuitBuilder &builder)
{
    auto *ic = new IC();
    builder.addOwnedElement(ic);

    const QString icFile = QuickTestUtils::examplesDir() + "jkflipflop.panda";
    if (!QFile::exists(icFile)) {
        return nullptr;
    }
    ic->loadFile(icFile, QFileInfo(icFile).absolutePath());
    builder.initSimulation();
    return ic;
}

// Extracts the body of a given `Class::method` implementation from a source file's full
// text. Matches the function header, then walks balanced braces to its closing `}`.
// [\s\S]*? (lazy, spans newlines) rather than [^)]* for the parameter list -- a negated-
// character-class match can't see past a nested closing paren in a parameter type like
// std::function<void(IC *)>, so it would stop at the wrong `)`.
QString bodyOfFunction(const QString &source, const QString &qualifiedName)
{
    const QString pattern =
        QStringLiteral("\\b") + QRegularExpression::escape(qualifiedName)
        + QStringLiteral("\\s*\\([\\s\\S]*?\\)\\s*\\{");
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
}

} // namespace

void TestDanglingPointer::initTestCase()
{
    // Shared test environment (headless mode, type registration) is set
    // up by the global runner (runQuickTestSuite()) before this slot.
}

// ==========================================================================
// Assertion-based tests — verify state via QVERIFY/QCOMPARE, no crash.
// ==========================================================================

// resetInternalState() must clear m_sortedInternalElements together with m_internalElements:
// qDeleteAll(m_internalElements) frees the elements, and the sorted vector holds pointers to
// those same objects.
void TestDanglingPointer::bug1_resetInternalStateMustClearSortedVector()
{
    QuickCircuitBuilder builder;
    auto *ic = loadInitializedIC(builder);
    QVERIFY2(ic, "jkflipflop.panda not found in examples — fixture missing");
    QVERIFY(!ic->m_internalElements.isEmpty());
    QVERIFY(!ic->m_sortedInternalElements.isEmpty());

    ic->resetInternalState();

    // Every piece of state referring to m_internalElements must be cleared in the same
    // atomic step.
    QCOMPARE(ic->m_internalElements.size(), 0);
    QCOMPARE(ic->m_sortedInternalElements.size(), 0);
    QCOMPARE(ic->m_boundaryInputElements.size(), 0);
    QVERIFY(!ic->m_internalHasFeedback);
}

// Bug 3 — IC::loadFile() PATH 2 calls resetInternalState() BEFORE
// loadFileDirectly(), which throws on corrupt input. The throw leaves
// m_internalElements empty but m_sortedInternalElements dangling.
//
// PATH 2 is only reached when qobject_cast<Scene *>(scene()) returns
// nullptr — so the IC must live outside any Scene for this test. There is
// no Scene at all in this port, so every IC here already takes that path.
void TestDanglingPointer::bug3_failedLoadMustLeaveConsistentState()
{
    auto ic = std::make_unique<IC>();

    const QString icFile = QuickTestUtils::examplesDir() + "jkflipflop.panda";
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
    const bool elementsEmpty = ic->m_internalElements.isEmpty();
    const bool sortedEmpty = ic->m_sortedInternalElements.isEmpty();
    QVERIFY2(elementsEmpty == sortedEmpty,
             qPrintable(QString("Inconsistent post-throw state: "
                                "m_internalElements.isEmpty()=%1 vs "
                                "m_sortedInternalElements.isEmpty()=%2")
                            .arg(elementsEmpty)
                            .arg(sortedEmpty)));
}

// restart() must clear m_sortedElements together with m_initialized -- leaving the hot-path
// vectors pointing at stale entries would fault any tick that runs before the next
// initialize().
void TestDanglingPointer::bug4_restartMustClearStaleTopology()
{
    QuickCircuitBuilder builder;
    auto *sim = builder.simulation();
    auto *ic = loadInitializedIC(builder);
    QVERIFY(ic != nullptr);
    QVERIFY(sim->m_initialized);
    QVERIFY(!sim->m_sortedElements.isEmpty());

    sim->restart();

    // Invariant: m_initialized=true implies m_sortedElements reflects the
    // current topology.
    QVERIFY2(sim->m_initialized || sim->m_sortedElements.isEmpty(),
             qPrintable(QString("Simulation::restart() left torn state: "
                                "m_initialized=%1, m_sortedElements.size()=%2")
                            .arg(sim->m_initialized)
                            .arg(sim->m_sortedElements.size())));
}

// Bug 5 — when Simulation::initialize() early-returns (e.g., the host has no elements left),
// m_sortedElements is cleared but m_initialized can be left at its previous value — a stale
// lie. This test exercises that "drop to zero elements, then re-derive" path directly against
// Simulation::restart(), which must clear both together.
void TestDanglingPointer::bug5_setCircuitUpdateRequiredMustNotLieAboutInit()
{
    QuickCircuitBuilder builder;
    auto *sim = builder.simulation();

    // Load a real circuit — initialize() succeeds, m_initialized=true,
    // m_sortedElements populated.
    auto *ic = loadInitializedIC(builder);
    QVERIFY(ic != nullptr);
    QVERIFY(sim->m_initialized);
    QVERIFY(!sim->m_sortedElements.isEmpty());

    // Remove every GraphicElement so the host has zero elements left and
    // initialize() bails at its early return.
    builder.removeElement(ic);
    delete ic;

    sim->restart();

    // Invariant: m_initialized must match the actual state — either it is
    // false (forcing the next update() to re-run initialize) OR
    // m_sortedElements is non-empty.
    const bool initialized = sim->m_initialized;
    const bool hasSorted = !sim->m_sortedElements.isEmpty();
    QVERIFY2(!initialized || hasSorted,
             qPrintable(QString("restart() left m_initialized=%1 "
                                "with m_sortedElements.isEmpty()=%2 — stale "
                                "'initialized' flag after a failed initialize().")
                            .arg(initialized)
                            .arg(!hasSorted)));
}

// Bug 6 — Source-level check. A runtime reproduction requires the
// Application::notify QMessageBox nested-event-loop window, which can't
// be triggered reliably inside QtTest. Instead, assert that each
// topology-mutating command opens a SimulationBlocker scope in its
// redo() and undo(). Checks App/QuickShell/Canvas/CanvasCommands.cpp.
void TestDanglingPointer::bug6_topologyCommandsMustUseSimulationBlocker()
{
    // CURRENTDIR is defined at compile time as ${CMAKE_CURRENT_SOURCE_DIR}/Tests
    // (see CMakeLists.txt). That's stable across how the test is invoked --
    // from the build dir, via ctest, or from the source tree.
#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string
    const QString commandsCpp =
        QString(QUOTE(CURRENTDIR)) + "/../App/QuickShell/Canvas/CanvasCommands.cpp";
#undef _QUOTE
#undef QUOTE
    QFile src(commandsCpp);
    QVERIFY2(src.open(QIODevice::ReadOnly),
             qPrintable(QString("Cannot open %1").arg(src.fileName())));
    const QString source = QString::fromUtf8(src.readAll());
    src.close();

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
        const QString body = bodyOfFunction(source, name);
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
                                "body to prevent Application::notify QMessageBox "
                                "re-entrancy from ticking on torn state:\n  - %1")
                            .arg(missingBlocker.join("\n  - "))));
}

// Hardening — CanvasItem::cancelEditedWire() deletes the in-progress wire directly. The
// in-progress wire is never registered via addItem()/simulationItems(), so there is nothing
// for the simulation timer to dangle a reference to — starting and cancelling a wire while
// the simulation is running must not crash or corrupt the circuit.
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
    QVERIFY(inputStatus(led));

    canvas.simulation()->stop();
}

// WIREDPANDA-JD — Simulation has no m_connections member; Phase 3 walks m_sortedElements
// instead, so there is no separate connection list for an in-progress (uncommitted) wire to
// dangle in. Regression test: an in-progress wire coexisting with a real connection during a
// re-initialize must not corrupt or crash the simulation, and the real connection must keep
// propagating correctly.
void TestDanglingPointer::jd_initializeMustSkipIncompleteConnections()
{
    QuickCircuitBuilder builder;
    auto *sim = builder.simulation();

    auto *sw = new InputSwitch();
    auto *led = new Led();
    builder.addOwnedElement(sw);
    builder.addOwnedElement(led);

    builder.connect(sw, 0, led, 0);

    sim->initialize();
    QVERIFY(sim->m_initialized);

    // An in-progress wire has only startPort set (no endPort — the user is still dragging
    // it). Not tracked by the builder: the caller manages its lifetime directly.
    auto *inProgressWire = new Connection();
    inProgressWire->setStartPort(sw->outputPort(0));

    // Re-initialize (simulates any command that triggers a topology rebuild while the user
    // is drawing a wire — e.g. undo/redo, IC hot-reload).
    sim->initialize();
    QVERIFY(sim->m_initialized);

    sw->setOn(true);
    sim->update();
    QVERIFY(inputStatus(led));

    // Cancel the in-progress wire and tick again — nothing in Simulation held onto the freed
    // wire, so this must not crash, and the real connection must still function.
    delete inProgressWire;

    sw->setOn(false);
    sim->update();
    QVERIFY(!inputStatus(led));
}

// ==========================================================================
// Crash-triggering tests. Kept at the end so they don't prevent the
// assertion tests above from reporting.
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
    auto *sim = builder.simulation();
    auto *ic = loadInitializedIC(builder);
    QVERIFY2(ic, "jkflipflop.panda not found in examples — fixture missing");
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

    ic->updateLogic();
    QVERIFY(true);      // reaching here is the pass condition
}

// Bug 7 — CanvasICRegistry::onFileChanged() must reload IC instances under a
// SimulationBlocker scope and drive a full re-initialize at the end. Checks
// App/QuickShell/Canvas/CanvasICRegistry.cpp. The full-reinitialize contract lives on
// Simulation::restart() itself, reached here via CanvasItem::restartSimulation().
void TestDanglingPointer::bug7_icRegistryFileChangedMustNotLeaveDanglingPointers()
{
#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string
    const QString path =
        QString(QUOTE(CURRENTDIR)) + "/../App/QuickShell/Canvas/CanvasICRegistry.cpp";
#undef _QUOTE
#undef QUOTE
    QFile src(path);
    QVERIFY2(src.open(QIODevice::ReadOnly),
             qPrintable(QString("Cannot open %1").arg(src.fileName())));
    const QString source = QString::fromUtf8(src.readAll());
    src.close();

    const QString body = bodyOfFunction(source, "CanvasICRegistry::onFileChanged");
    QVERIFY2(!body.isEmpty(), "Could not locate CanvasICRegistry::onFileChanged definition.");

    QVERIFY2(body.contains("reloadTargetsAtomically"),
             "CanvasICRegistry::onFileChanged must reload targets through "
             "reloadTargetsAtomically(), which opens a SimulationBlocker "
             "scope around the whole reload loop so the simulation timer "
             "is quiescent while each IC's m_internalElements is "
             "freed and replaced.");

    QVERIFY2(body.contains("restartSimulation"),
             "CanvasICRegistry::onFileChanged must call restartSimulation() "
             "to drive a full re-initialize after the reload -- a bare "
             "simulation()->restart() call alone only invalidated the outer "
             "Simulation::m_sortedElements, leaving each IC's "
             "m_sortedInternalElements untouched.");
}

// Hardening — embedICsByFile()/extractToFile() reload live, already-in-scene ICs exactly like
// onFileChanged() (bug7) does, via ic->loadFromBlob()/loadFile(). Same H2 use-after-free
// shape: an IC's internal flip-flop elements are freed by resetInternalState() while
// Simulation::m_sequentialElements still points at them if the simulation timer isn't stopped
// for the duration. All three functions must go through the shared reloadTargetsAtomically()
// helper. Checks CanvasICRegistry.cpp.
void TestDanglingPointer::hardening_icRegistryReloadHelpersMustUseSimulationBlocker()
{
#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string
    const QString path =
        QString(QUOTE(CURRENTDIR)) + "/../App/QuickShell/Canvas/CanvasICRegistry.cpp";
#undef _QUOTE
#undef QUOTE
    QFile src(path);
    QVERIFY2(src.open(QIODevice::ReadOnly),
             qPrintable(QString("Cannot open %1").arg(src.fileName())));
    const QString source = QString::fromUtf8(src.readAll());
    src.close();

    const QString helperBody = bodyOfFunction(source, "CanvasICRegistry::reloadTargetsAtomically");
    QVERIFY2(!helperBody.isEmpty(), "Could not locate CanvasICRegistry::reloadTargetsAtomically definition.");
    QVERIFY2(helperBody.contains("SimulationBlocker"),
             "CanvasICRegistry::reloadTargetsAtomically must open a SimulationBlocker "
             "scope around its whole loop over targets, not just each individual "
             "mutation — the timer must stay stopped between iterations too, since "
             "Simulation::m_sequentialElements stays stale until the caller's "
             "restartSimulation() runs after the entire loop completes.");

    const QStringList reloadCallers = {
        "CanvasICRegistry::embedICsByFile",
        "CanvasICRegistry::extractToFile",
    };
    QStringList missingCall;
    for (const QString &name : reloadCallers) {
        const QString body = bodyOfFunction(source, name);
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
                                "(which opens a SimulationBlocker for the whole loop) rather "
                                "than mutating them directly:\n  - %1")
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

// WIREDPANDA-JD — defense in depth. An in-progress wire (startPort only, no endPort)
// coexists with a real connection, a re-initialize runs while it's present, and the wire is
// then deleted (simulating a cancelled wire drag). The next update() must not crash.
// Simulation has no m_connections member (Phase 3 walks m_sortedElements instead), so
// Simulation has no reference left to dangle; this test guards against any future code path
// reintroducing one.
void TestDanglingPointer::jd_cancelledWireMustNotLeaveDanglingPointer()
{
    QuickCircuitBuilder builder;
    auto *sim = builder.simulation();

    auto *sw = new InputSwitch();
    auto *led = new Led();
    builder.addOwnedElement(sw);
    builder.addOwnedElement(led);

    builder.connect(sw, 0, led, 0);

    sim->initialize();
    QVERIFY(sim->m_initialized);
    sim->setVisualThrottleEnabled(false);

    // Add an in-progress wire (startPort only), not tracked by the builder.
    auto *inProgressWire = new Connection();
    inProgressWire->setStartPort(sw->outputPort(0));

    // Re-initialize while the in-progress wire is present.
    sim->initialize();

    // Delete the in-progress wire (a bare delete, same as a cancelled wire drag).
    delete inProgressWire;

    // Must not crash.
    sim->update();
    QVERIFY(true);
}

// WIREDPANDA-HC — when an element is destroyed out-of-band (no command, no explicit teardown
// of attached wires first), the destruction cascade (~ElementPorts -> ~InputPort/~OutputPort
// draining their connections) must not leave a stale registry entry pointing at freed memory.
// ItemWithId self-unregisters from whichever SceneItemRegistry it's mapped in on any
// destruction path (see App/Core/ItemWithId.h), so this doesn't depend on going through a
// specific removeItem() call first.
void TestDanglingPointer::hcDrainConnectionsMustCleanRegistry()
{
    QuickCircuitBuilder builder;
    auto *sw = new InputSwitch();
    auto *led = new Led();
    builder.addOwnedElement(sw);
    builder.addOwnedElement(led);

    auto *conn = builder.connect(sw, 0, led, 0);

    const int connId = conn->id();
    QCOMPARE(builder.itemById(connId), conn);

    // Out-of-band destruction: untrack the element only, then delete.
    // Cascade reaches ~OutputPort -> drainConnections, which deletes the
    // wire still attached to it.
    builder.removeElement(sw);
    delete sw;

    // The wire's own destructor self-unregisters from the registry, so the registry entry
    // is gone.
    QVERIFY(builder.itemById(connId) == nullptr);
}
