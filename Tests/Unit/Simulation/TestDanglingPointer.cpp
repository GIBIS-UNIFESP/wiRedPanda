// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// Regression tests for the Sentry cluster headed by WIREDPANDA-H2. Each
// test is derived from the analysis in
// .claude/SENTRY_CRASH_ANALYSIS_WIREDPANDA-H2.md and is expected to fail
// on the current tree — the tests ARE the specification of the bugs.
//
// Ordering note: non-crash assertions run first so ctest still reports
// them even when a later death-test tears the process down.

#include "Tests/Unit/Simulation/TestDanglingPointer.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMetaObject>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QVector>

#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/Element/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "Tests/Common/TestUtils.h"

namespace {

// Loads jkflipflop (5 inputs, 2 outputs) onto the scene, runs the outer
// Simulation::initialize() so IC::initializeSimulation() populates
// m_sortedInternalElements, and returns the IC. Returns nullptr if the
// fixture file is missing.
IC *loadInitializedIC(WorkSpace &ws)
{
    auto *ic = new IC();
    ws.scene()->addItem(ic);

    const QString icFile = TestUtils::examplesDir() + "jkflipflop.panda";
    if (!QFile::exists(icFile)) {
        return nullptr;
    }
    ic->loadFile(icFile, QFileInfo(icFile).absolutePath());
    ws.scene()->simulation()->initialize();
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
    WorkSpace ws;
    auto *ic = loadInitializedIC(ws);
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
// PATH 2 is only reached when qobject_cast<Scene *>(scene()) returns
// nullptr — so the IC must live outside any Scene for this test.
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
    WorkSpace ws;
    auto *sim = ws.scene()->simulation();
    auto *ic = loadInitializedIC(ws);
    QVERIFY(ic);
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

// Bug 5 — Scene::setCircuitUpdateRequired() calls m_simulation.initialize()
// and discards the return value. When initialize() early-returns (e.g.,
// scene has only the border rect), m_sortedElements is cleared but
// m_initialized is left at its previous value — a stale lie.
void TestDanglingPointer::bug5_setCircuitUpdateRequiredMustNotLieAboutInit()
{
    WorkSpace ws;
    auto *sim = ws.scene()->simulation();

    // Load a real circuit — initialize() succeeds, m_initialized=true,
    // m_sortedElements populated.
    auto *ic = loadInitializedIC(ws);
    QVERIFY(ic);
    QVERIFY(sim->m_initialized);
    QVERIFY(!sim->m_sortedElements.isEmpty());

    // Remove every GraphicElement so items.size() drops to 1 (border rect)
    // and initialize() bails at the early return.
    ws.scene()->removeItem(ic);
    delete ic;

    ws.scene()->setCircuitUpdateRequired();

    // Invariant: m_initialized must match the actual state — either it is
    // false (forcing the next update() to re-run initialize) OR
    // m_sortedElements is non-empty. The current code leaves them
    // disagreeing: initialized=true, sortedElements=empty.
    const bool initialized = sim->m_initialized;
    const bool hasSorted = !sim->m_sortedElements.isEmpty();
    QVERIFY2(!initialized || hasSorted,
             qPrintable(QString("setCircuitUpdateRequired left m_initialized=%1 "
                                "with m_sortedElements.isEmpty()=%2 — stale "
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
    // CURRENTDIR is defined at compile time as ${CMAKE_CURRENT_SOURCE_DIR}/Tests
    // (see CMakeLists.txt:509,522). That's stable across how the test is
    // invoked — from the build dir, via ctest, or from the source tree.
    const QString commandsCpp =
        QString(QUOTE(CURRENTDIR)) + "/../App/Scene/Commands.cpp";
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
        "UpdateCommand::redo",
        "UpdateCommand::undo",
        "MorphCommand::redo",
        "MorphCommand::undo",
        "ChangePortSizeCommand::redo",
        "ChangePortSizeCommand::undo",
        "SplitCommand::redo",
        "SplitCommand::undo",
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
                                "body to prevent Application::notify QMessageBox "
                                "re-entrancy from ticking on torn state:\n  - %1")
                            .arg(missingBlocker.join("\n  - "))));
}

// Hardening — ConnectionManager::deleteEditedConnection deletes the
// in-progress wire without a SimulationBlocker. Source-level check
// mirroring bug6's approach.
void TestDanglingPointer::hardening_deleteEditedConnectionMustUseSimulationBlocker()
{
    const QString path =
        QString(QUOTE(CURRENTDIR)) + "/../App/Scene/ConnectionManager.cpp";
    QFile src(path);
    QVERIFY2(src.open(QIODevice::ReadOnly),
             qPrintable(QString("Cannot open %1").arg(src.fileName())));
    const QString source = QString::fromUtf8(src.readAll());
    src.close();

    QRegularExpression rx(
        QStringLiteral("\\bConnectionManager::deleteEditedConnection\\s*\\([^)]*\\)\\s*\\{"));
    const auto match = rx.match(source);
    QVERIFY2(match.hasMatch(),
             "Could not locate ConnectionManager::deleteEditedConnection definition.");

    const qsizetype bodyStart = match.capturedEnd() - 1;
    int depth = 0;
    qsizetype bodyEnd = -1;
    for (qsizetype i = bodyStart; i < source.size(); ++i) {
        const QChar c = source.at(i);
        if (c == '{') ++depth;
        else if (c == '}') {
            --depth;
            if (depth == 0) { bodyEnd = i; break; }
        }
    }
    QVERIFY2(bodyEnd > bodyStart,
             "Could not find end of ConnectionManager::deleteEditedConnection body.");
    const QString body = source.mid(bodyStart, bodyEnd - bodyStart + 1);

    QVERIFY2(body.contains("SimulationBlocker"),
             "ConnectionManager::deleteEditedConnection must open a "
             "SimulationBlocker around the removeItem/delete pair so the "
             "1 ms simulation timer cannot tick on a freed connection if "
             "the in-progress wire ever ended up in Simulation::m_connections.");
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

// Hardening — Simulation::update() Phase 3 walks m_connections and reads
// connection->startPort() with no null guard, unlike Phases 1, 2, and 4.
// Inject a null and tick the simulation; without the guard this dereferences
// nullptr and SIGSEGVs.
void TestDanglingPointer::hardening_phase3MustTolerateNullConnection()
{
    WorkSpace ws;
    auto *sim = ws.scene()->simulation();
    auto *ic = loadInitializedIC(ws);
    QVERIFY2(ic, "jkflipflop.panda not found in examples — fixture missing");
    QVERIFY(sim->m_initialized);

    // Phase 3 only runs when the visual throttle lets through; force-disable
    // so update() actually walks m_connections this tick instead of skipping
    // straight back to Phase 2.
    sim->setVisualThrottleEnabled(false);

    sim->m_connections.append(nullptr);

    sim->update();
    QVERIFY(true);
}

// Bug 2 — IC::updateLogic() dereferences m_sortedInternalElements without
// a null guard. Inject a null directly and call updateLogic.
void TestDanglingPointer::bug2_icUpdateLogicMustTolerateNullEntry()
{
    WorkSpace ws;
    auto *ic = loadInitializedIC(ws);
    QVERIFY2(ic, "jkflipflop.panda not found in examples — fixture missing");
    QVERIFY(!ic->m_sortedInternalElements.isEmpty());

    // Inject a nullptr — mirrors the dangling state left by Bug 1/Bug 3
    // but deterministically, without relying on heap poisoning patterns.
    const qsizetype mid = ic->m_sortedInternalElements.size() / 2;
    ic->m_sortedInternalElements.insert(mid, nullptr);

    ic->updateLogic();  // pre-fix: null deref
    QVERIFY(true);      // reaching here is the pass condition
}

// Bug 7 — ICRegistry::onFileChanged() must reload IC instances under a
// SimulationBlocker scope and drive a full setCircuitUpdateRequired()
// at the end. A runtime repro via QMetaObject::invokeMethod + corrupt
// file proved flaky on MSVC: the PANDACEPTION thrown by the slot
// propagates through Qt's meta-call machinery in a way that escapes
// both `catch(const std::exception&)` and `catch(...)` in the test
// harness, even though Application::notify() catches it correctly in
// production. Source-level check (same shape as bug6) directly
// verifies Fix D is in place and is robust across compilers.
void TestDanglingPointer::bug7_icRegistryFileChangedMustNotLeaveDanglingPointers()
{
    const QString path =
        QString(QUOTE(CURRENTDIR)) + "/../App/Element/ICRegistry.cpp";
    QFile src(path);
    QVERIFY2(src.open(QIODevice::ReadOnly),
             qPrintable(QString("Cannot open %1").arg(src.fileName())));
    const QString source = QString::fromUtf8(src.readAll());
    src.close();

    // Locate ICRegistry::onFileChanged's body.
    QRegularExpression rx(
        QStringLiteral("\\bICRegistry::onFileChanged\\s*\\([^)]*\\)\\s*\\{"));
    const auto match = rx.match(source);
    QVERIFY2(match.hasMatch(),
             "Could not locate ICRegistry::onFileChanged definition.");

    // Walk balanced braces to the end of the function body.
    const qsizetype bodyStart = match.capturedEnd() - 1;
    int depth = 0;
    qsizetype bodyEnd = -1;
    for (qsizetype i = bodyStart; i < source.size(); ++i) {
        const QChar c = source.at(i);
        if (c == '{') ++depth;
        else if (c == '}') {
            --depth;
            if (depth == 0) { bodyEnd = i; break; }
        }
    }
    QVERIFY2(bodyEnd > bodyStart,
             "Could not find end of ICRegistry::onFileChanged body.");
    const QString body = source.mid(bodyStart, bodyEnd - bodyStart + 1);

    QVERIFY2(body.contains("SimulationBlocker"),
             "ICRegistry::onFileChanged must open a SimulationBlocker "
             "scope around the reload loop so the 1 ms simulation QTimer "
             "is quiescent while each IC's m_internalElements is freed "
             "and replaced.");

    QVERIFY2(body.contains("setCircuitUpdateRequired"),
             "ICRegistry::onFileChanged must call setCircuitUpdateRequired() "
             "to drive a full Simulation::initialize() after the reload. "
             "The previous simulation()->restart() was too weak — it only "
             "invalidated the outer Simulation::m_sortedElements, leaving "
             "each IC's m_sortedInternalElements untouched.");
}

// Combined integration reproduction of WIREDPANDA-H2. Same tick path the
// production crash took: outer Simulation::update() → element->updateLogic()
// on the IC → IC::updateLogic() iterates m_sortedInternalElements → reads
// the vtable of a freed entry.
void TestDanglingPointer::integration_simulationTickAfterResetMustNotCrash()
{
    WorkSpace ws;
    auto *ic = loadInitializedIC(ws);
    QVERIFY2(ic, "jkflipflop.panda not found in examples — fixture missing");
    QVERIFY(!ic->m_sortedInternalElements.isEmpty());

    // Snap-freeze the crash state: one null sentinel in the sorted vector,
    // mirroring the dangling state Bug 1 / Bug 3 produce, deterministically.
    const qsizetype mid = ic->m_sortedInternalElements.size() / 2;
    ic->m_sortedInternalElements.insert(mid, nullptr);

    ws.scene()->simulation()->update();
    QVERIFY(true);
}

