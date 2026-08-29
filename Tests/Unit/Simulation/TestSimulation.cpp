// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestSimulation.h"

#include <memory>

#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>

#include "App/Core/Application.h"
#include "App/Core/SimulationHost.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/IC.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/TestUtils.h"

namespace {
// Minimal SimulationHost stub giving full control over simulationItems() (including
// injecting nullptrs), so Simulation::initialize()'s defensive item-list handling can be
// tested in isolation from a real Scene.
class StubSimulationHost : public SimulationHost
{
public:
    QList<QGraphicsItem *> simulationItems() const override { return m_items; }
    void setMuted(bool muted) override { m_muted = muted; }

    QList<QGraphicsItem *> m_items;
    bool m_muted = false;
};
} // namespace

void TestSimulationUnit::testSimulationWithNoElements()
{
    // Test: Simulation handles empty scene gracefully
    WorkSpace workspace;
    Simulation sim(workspace.scene());

    QVERIFY(!sim.isRunning()); // starts stopped
}

void TestSimulationUnit::testAddRemoveClockDuringSimulation()
{
    // A Clock added to (and later removed from) a RUNNING simulation must be
    // integrated into / dropped from the clock-update list without corrupting
    // the rest of the circuit — the dangling-clock-pointer crash class.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *sim = scene->simulation();

    auto *sw = new InputSwitch();
    auto *led = new Led();
    scene->addItem(sw);
    scene->addItem(led);

    CircuitBuilder builder(scene);
    builder.connect(sw, 0, led, 0);

    sim->start();
    QVERIFY(sim->isRunning());
    sim->update();

    // Add a wired Clock while the simulation runs.
    auto *clock = new Clock();
    auto *clockLed = new Led();
    scene->addItem(clock);
    scene->addItem(clockLed);
    auto *clockConn = new Connection();
    clockConn->setStartPort(clock->outputPort(0));
    clockConn->setEndPort(clockLed->inputPort(0));
    scene->addItem(clockConn);
    scene->setCircuitUpdateRequired();
    sim->update();

    // The pre-existing path must keep propagating correctly with the clock in place.
    sw->setOn(true);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(led), true);

    // Remove the Clock (connection first, mirroring scene deletion order)
    // while the simulation is still running.
    scene->removeItem(clockConn);
    delete clockConn;
    scene->removeItem(clock);
    delete clock;
    scene->setCircuitUpdateRequired();
    sim->update();

    // The simulation must survive the removal and keep computing correctly.
    QVERIFY(sim->isRunning());
    sw->setOn(false);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(led), false);
    sw->setOn(true);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(led), true);
}

void TestSimulationUnit::testElementRemovalMidSimulation()
{
    // Removing a wired logic element from a RUNNING simulation must not leave
    // stale pointers in the sorted-element or connection lists (the H2-class
    // use-after-free), and the simulation must keep computing afterwards.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *sim = scene->simulation();

    auto *sw = new InputSwitch();
    auto *andGate = new And();
    auto *led = new Led();
    scene->addItem(sw);
    scene->addItem(andGate);
    scene->addItem(led);

    CircuitBuilder builder(scene);
    auto *connIn = builder.connect(sw, 0, andGate, 0);
    auto *connOut = builder.connect(andGate, 0, led, 0);

    sim->start();
    QVERIFY(sim->isRunning());
    sw->setOn(true);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(andGate), true);

    // Remove the AND gate mid-run (connections first, then the element).
    scene->removeItem(connIn);
    delete connIn;
    scene->removeItem(connOut);
    delete connOut;
    scene->removeItem(andGate);
    delete andGate;
    scene->setCircuitUpdateRequired();
    sim->update();
    sim->update();

    // The simulation must survive and still integrate NEW topology correctly:
    // wire the switch directly to the LED and verify propagation resumes.
    QVERIFY(sim->isRunning());
    auto *directConn = builder.connect(sw, 0, led, 0);
    QVERIFY(directConn != nullptr);
    scene->setCircuitUpdateRequired();
    sim->update();
    QCOMPARE(TestUtils::inputStatus(led), true);
    sw->setOn(false);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(led), false);
}

void TestSimulationUnit::testSimulationStartStopNoBreadcrumbsB22()
{
    // sentryBreadcrumb is a no-op without HAVE_SENTRY linked, so verify the
    // regression at the source level: Simulation::start() and ::stop() must
    // not contain a sentryBreadcrumb call any more. Every SimulationBlocker
    // scope used to add three breadcrumbs (sim-stop, command, sim-start)
    // and overwhelm the 100-slot ring buffer.
    const QString sourcePath =
        QString(QUOTE(CURRENTDIR)) + "/../App/Simulation/Simulation.cpp";
    QFile src(sourcePath);
    QVERIFY2(src.open(QIODevice::ReadOnly),
             qPrintable(QString("Cannot open %1").arg(src.fileName())));
    const QString source = QString::fromUtf8(src.readAll());
    src.close();

    auto bodyOf = [&source](const QString &signature) -> QString {
        const qsizetype start = source.indexOf(signature);
        if (start < 0) return {};
        const qsizetype openBrace = source.indexOf('{', start);
        if (openBrace < 0) return {};
        int depth = 0;
        for (qsizetype i = openBrace; i < source.size(); ++i) {
            if (source[i] == '{') ++depth;
            else if (source[i] == '}') {
                --depth;
                if (depth == 0) return source.mid(openBrace, i - openBrace + 1);
            }
        }
        return {};
    };

    const QString stopBody = bodyOf("Simulation::stop()");
    const QString startBody = bodyOf("Simulation::start()");
    QVERIFY2(!stopBody.isEmpty(), "Could not locate Simulation::stop body");
    QVERIFY2(!startBody.isEmpty(), "Could not locate Simulation::start body");
    QVERIFY2(!stopBody.contains("sentryBreadcrumb"),
             "Simulation::stop must not emit a Sentry breadcrumb — every "
             "SimulationBlocker scope hits this and floods the buffer.");
    QVERIFY2(!startBody.contains("sentryBreadcrumb"),
             "Simulation::start must not emit a Sentry breadcrumb — every "
             "SimulationBlocker scope hits this and floods the buffer.");
}

void TestSimulationUnit::testUnconnectedOutputPortVisualUpdates()
{
    // Regression: Phase 3 must push computed values onto output-port visuals
    // even when no wire is attached. A connection-based walk skips them, so a
    // flip-flop's unwired -Q (or any unwired gate output) keeps a stale color.
    WorkSpace workspace;
    auto *scene = workspace.scene();

    auto *vcc = ElementFactory::buildElement(ElementType::InputVcc);
    auto *notGate = ElementFactory::buildElement(ElementType::Not);
    scene->addItem(vcc);
    scene->addItem(notGate);

    auto *conn = new Connection();
    conn->setStartPort(vcc->outputPort(0));
    conn->setEndPort(notGate->inputPort(0));
    scene->addItem(conn);

    // The NOT output stays unwired: undriven (Unknown) until a tick computes it
    QCOMPARE(notGate->outputPort(0)->status(), Status::Unknown);

    Simulation sim(scene);
    sim.update();

    // NOT(1) = 0 — and the unwired output port's visual must reflect it
    QCOMPARE(notGate->outputPort(0)->status(), Status::Inactive);
    QCOMPARE(notGate->outputPort(0)->status(), notGate->outputValue(0));
}

void TestSimulationUnit::testInitializeReturnsFalseWithNoHost()
{
    Simulation sim(nullptr);
    QVERIFY(!sim.initialize());
}

void TestSimulationUnit::testInitializeSkipsNullItemsAndFailsWithNoElements()
{
    // Two null items: bypasses the "single background item" early return (size != 1), the
    // sort comparator's null-handling branch runs, the item loop's null-item continue runs
    // for both, and since nothing real was ever appended, initialize() must still fail.
    StubSimulationHost host;
    host.m_items = {nullptr, nullptr};

    Simulation sim(&host);
    QVERIFY(!sim.initialize());
}

void TestSimulationUnit::testUpdatePortWithNullPortsAreNoOps()
{
    // Pure defensive guards on private static helpers -- reachable directly via the friend
    // seam; nothing to observe besides "does not crash" since a null port has no state.
    Simulation::updatePort(static_cast<OutputPort *>(nullptr));
    Simulation::updatePort(static_cast<InputPort *>(nullptr));
}

void TestSimulationUnit::testIdleTicksAreSkippedOnceAtFixedPoint()
{
    // Discriminating a skipped tick from a drained one needs care. Corrupting an element's
    // output and checking it survives does NOT work: the incremental seed only wakes successors
    // of sources whose outputChanged() is set, so with no source change a *drained* tick
    // re-evaluates nothing either and the corruption survives both ways -- such a test passes
    // with the skip disabled, so it discriminates nothing.
    //
    // What genuinely differs is the visual bookkeeping: a drained tick always sets
    // m_visualsDirty (a flush is owed), while a skipped tick leaves it alone. That is only
    // observable while the throttle is engaged, so the flush isn't performed immediately.
    TestUtils::ScopedInteractiveMode interactiveGuard;
    Application::interactiveMode = true;

    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *sw = new InputSwitch();
    auto *notGate = new Not();
    auto *led = new Led();
    scene->addItem(sw);
    scene->addItem(notGate);
    scene->addItem(led);
    CircuitBuilder builder(scene);
    builder.connect(sw, 0, notGate, 0);
    builder.connect(notGate, 0, led, 0);

    Simulation sim(scene);
    sim.setVisualThrottleEnabled(true);
    sim.m_visualTickInterval = 4; // friend seam: deterministic, not screen-derived

    sw->setOn(true);
    sim.m_visualTickCount = 3;    // next tick is visualsDue: it flushes and clears the flag
    sim.update();
    QVERIFY2(sim.m_atFixedPoint,
             "a drain that empties the event queue must be recognised as a fixed point");
    QVERIFY2(!sim.m_visualsDirty, "precondition: that tick flushed, so nothing is owed");
    QCOMPARE(notGate->outputValue(0), Status::Inactive); // NOT(1)

    // Now a tick with no source change, while a flush is NOT due. If it is skipped, nothing is
    // owed afterwards. If it drains, it owes a flush.
    sim.update();
    QVERIFY2(!sim.m_visualsDirty,
             "an idle tick at a fixed point must be skipped, not drained");

    // A real source change must leave the fixed point and do actual work.
    sw->setOn(false);
    sim.update();
    QCOMPARE(notGate->outputValue(0), Status::Active); // NOT(0)
    QVERIFY2(sim.m_visualsDirty, "a tick that really drained owes a visual flush");
}

void TestSimulationUnit::testForcedReseedClearsFixedPointFlag()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *sw = new InputSwitch();
    auto *led = new Led();
    scene->addItem(sw);
    scene->addItem(led);
    CircuitBuilder builder(scene);
    builder.connect(sw, 0, led, 0);

    Simulation sim(scene);
    sw->setOn(true);
    sim.update();
    QVERIFY(sim.m_atFixedPoint);

    // Each of these forces a full re-seed (m_eventInitDone = false) and must therefore also
    // drop the fixed-point conclusion -- otherwise the very tick that is supposed to re-settle
    // the network is skipped as idle, and m_eventInitDone stays false forever.
    sim.resetEventTracking();
    QVERIFY2(!sim.m_atFixedPoint, "resetEventTracking() must clear the fixed-point flag");

    sim.update();
    QVERIFY(sim.m_atFixedPoint);
    sim.beginTimedRun(1);
    QVERIFY2(!sim.m_atFixedPoint, "beginTimedRun() must clear the fixed-point flag");

    sim.update();
    QVERIFY2(sim.m_eventInitDone, "the first timed update() must actually seed the network");
    sim.endTimedRun(0);
    QVERIFY2(!sim.m_atFixedPoint, "endTimedRun() must clear the fixed-point flag");

    sim.restart();
    QVERIFY2(!sim.m_atFixedPoint, "restart() must clear the fixed-point flag");
}

void TestSimulationUnit::testUpdateFlushesPendingVisualsOnLaterIdleTick()
{
    // The visual throttle only engages when Application::interactiveMode is true (forced
    // false globally for the rest of the test suite) -- toggle it locally via a scope guard
    // that restores it even if a QVERIFY/QCOMPARE below fails and returns early, since
    // Application::interactiveMode is process-wide and would otherwise leak into every later
    // test in the run.
    TestUtils::ScopedInteractiveMode interactiveGuard;
    Application::interactiveMode = true;

    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *sw = new InputSwitch();
    auto *led = new Led();
    scene->addItem(sw);
    scene->addItem(led);
    CircuitBuilder builder(scene);
    builder.connect(sw, 0, led, 0);

    Simulation sim(scene);
    sim.setVisualThrottleEnabled(true);
    // Friend-seam access: a short, deterministic interval instead of the real screen-derived
    // one (usually ~16), so this test doesn't depend on the host's refresh rate.
    sim.m_visualTickInterval = 5;
    sim.m_visualTickCount = 0;

    sw->setOn(true); // a real change -- the sweep runs
    sim.update(); // tick 1/5: visualsDue is false, so the sweep's visual flush is skipped
    QVERIFY2(sim.m_visualsDirty, "Precondition: the sweep must leave a pending visual flush");

    // No further change: every subsequent tick hits the fixed-point early return until the
    // throttle interval elapses. Ticks 2-4/5 must still have the flush pending -- only the
    // 5th tick should clear it -- otherwise the throttle isn't actually gating the flush.
    for (int i = 0; i < 3; ++i) {
        sim.update();
        QVERIFY2(sim.m_visualsDirty,
                  "The pending visual flush must still be pending before the throttle interval elapses");
    }
    sim.update(); // tick 5/5: interval elapses, flush happens
    QVERIFY2(!sim.m_visualsDirty,
              "A pending visual flush must happen on an idle tick once the throttle interval elapses");
}

void TestSimulationUnit::testBlockerCyclePreservesClockLevel()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *sim = scene->simulation();

    auto *clock = qobject_cast<Clock *>(ElementFactory::buildElement(ElementType::Clock));
    QVERIFY(clock);
    // 10 Hz ⇒ 50 ms per phase: slow enough that the post-resume assertion below runs well
    // inside the LOW phase, fast enough for the first falling edge to land quickly.
    clock->setFrequency(10.0);
    scene->addItem(clock);

    sim->start();
    QVERIFY(sim->isRunning());

    // Clocks start HIGH (resetClock in initialize()); spin the event loop until the first
    // real falling edge lands so the clock is observably mid-LOW-phase. A plain qWait loop
    // instead of QTRY_VERIFY_WITH_TIMEOUT: Qt 6.8/6.9's QTRY macros expand with a
    // long→int chrono conversion that trips -Werror=conversion on the CI compilers.
    QElapsedTimer waitForLow;
    waitForLow.start();
    while (clock->isOn() && waitForLow.elapsed() < 2000) {
        QTest::qWait(10);
    }
    QVERIFY2(!clock->isOn(), "clock never fell within 2 s of starting");

    // A SimulationBlocker cycle brackets every UpdateCommand redo/undo — including a plain
    // InputSwitch click on a running circuit. The resume must preserve the level: resetClock()
    // forces the output HIGH, which is an out-of-thin-air rising edge for every clock-driven
    // element, advancing counters without a real edge.
    {
        SimulationBlocker blocker(sim);
    }
    QVERIFY2(!clock->isOn(),
             "resume after a SimulationBlocker cycle forced the clock HIGH mid-LOW-phase");

    sim->stop();
}

void TestSimulationUnit::testTimedRunBracketResetsAndRestores()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *sw = new InputSwitch();
    auto *notGate = new Not();
    scene->addItem(sw);
    scene->addItem(notGate);
    CircuitBuilder builder(scene);
    builder.connect(sw, 0, notGate, 0);
    Simulation *sim = builder.initSimulation();

    // Put the "live" session on its own temporal timeline so the restore below is observable:
    // with a 3-unit window the clock advances 3 per update().
    sim->setTimePerTick(3);
    sim->update();
    sim->update();
    const SimTime liveTime = sim->currentTime();
    QCOMPARE(liveTime, SimTime{6});

    // --- begin: fresh timeline, empty queue, full re-seed pending ---
    sim->beginTimedRun(1);
    QCOMPARE(sim->m_timePerTick, SimTime{1});
    QCOMPARE(sim->currentTime(), SimTime{0});
    QVERIFY2(sim->m_eventQueue.empty(), "beginTimedRun() must drop events from the live run");
    QVERIFY2(!sim->m_eventInitDone, "beginTimedRun() must force a whole-network re-seed");

    sim->update();
    QVERIFY2(sim->m_eventInitDone, "the first timed update() must seed the network");
    QCOMPARE(sim->currentTime(), SimTime{1});

    // A delay far beyond the tick window leaves an event pending past the swept timeline —
    // exactly what endTimedRun() has to clear.
    sim->setElementDelay(notGate, 500);
    sw->setOn(true);
    sim->update();
    QVERIFY2(!sim->m_eventQueue.empty(), "Precondition: an event must still be queued past the window");

    // --- end: previous window and live clock restored, nothing left queued ---
    sim->endTimedRun(3);
    QCOMPARE(sim->m_timePerTick, SimTime{3});
    QCOMPARE(sim->currentTime(), liveTime);
    QVERIFY2(sim->m_eventQueue.empty(),
             "endTimedRun() must drop events queued past the swept window — with the window "
             "restored they could never come due again, and would pin element pointers");
    QVERIFY2(!sim->m_eventInitDone, "endTimedRun() must force the live run to re-seed");
}

void TestSimulationUnit::testRestartClearsEveryPointerKeyedContainer()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *n1 = ElementFactory::buildElement(ElementType::Nand);
    auto *n2 = ElementFactory::buildElement(ElementType::Nand);
    scene->addItem(sw);
    scene->addItem(n1);
    scene->addItem(n2);
    sw->setPos(0, 0);
    n1->setPos(100, 0);
    n2->setPos(200, 0);

    const auto wire = [scene](GraphicElement *a, int ap, GraphicElement *b, int bp) {
        auto *c = new Connection();
        scene->addItem(c);
        c->setStartPort(a->outputPort(ap));
        c->setEndPort(b->inputPort(bp));
    };
    wire(sw, 0, n1, 0);
    wire(n1, 0, n2, 0);   // cross-coupled latch, so the component containers get populated
    wire(n2, 0, n1, 1);

    auto &sim = *scene->simulation();
    QVERIFY(sim.initialize());
    QVERIFY2(!sim.m_simFeedbackComponents.isEmpty(), "precondition: a cycle must be detected");
    QVERIFY(!sim.m_simFeedbackComponent.isEmpty());
    QVERIFY(!sim.m_simEvalCaps.isEmpty());
    QVERIFY(!sim.m_simPriorities.isEmpty());
    QVERIFY(!sim.m_delays.isEmpty());

    sim.restart();

    QVERIFY2(sim.m_simFeedbackComponents.isEmpty(), "m_simFeedbackComponents must not survive restart()");
    QVERIFY2(sim.m_simFeedbackComponent.isEmpty(), "m_simFeedbackComponent must not survive restart()");
    QVERIFY2(sim.m_simEvalCaps.isEmpty(), "m_simEvalCaps must not survive restart()");
    QVERIFY(sim.m_simPriorities.isEmpty());
    QVERIFY(sim.m_simFeedbackNodes.isEmpty());
    QVERIFY(sim.m_delays.isEmpty());
    QVERIFY(sim.m_publishGeneration.isEmpty());
}

void TestSimulationUnit::testFailedInitializeLeavesNoStaleTopology()
{
    // initialize() clears the topology vectors as its first act and can then bail out early
    // (a scene holding only the border rectangle, or no elements at all). Clearing only some of
    // them would leave m_initialized true beside containers still populated from the PREVIOUS
    // circuit, while update()'s Q_ASSERT(m_initialized) documents the opposite invariant: the
    // flag standing means the vectors describe the current scene. A failed rebuild must
    // therefore leave exactly what restart() leaves.
    //
    // Driven through the stub host rather than by emptying a real Scene, so the second
    // initialize() sees a different item list without any element actually being destroyed --
    // the containers under test are keyed by those very pointers, and reading a freed one to
    // decide whether it "survived" would be undefined behaviour.
    auto n1 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Nand));
    auto n2 = std::unique_ptr<GraphicElement>(ElementFactory::buildElement(ElementType::Nand));
    auto feed1 = std::make_unique<Connection>();
    auto feed2 = std::make_unique<Connection>();
    feed1->setStartPort(n1->outputPort(0));
    feed1->setEndPort(n2->inputPort(0));
    feed2->setStartPort(n2->outputPort(0));
    feed2->setEndPort(n1->inputPort(1));   // cross-coupled latch: the component containers fill

    StubSimulationHost host;
    host.m_items = {n1.get(), n2.get()};

    Simulation sim(&host);
    QVERIFY(sim.initialize());
    QVERIFY2(!sim.m_simFeedbackComponents.isEmpty(), "precondition: a cycle must be detected");
    QVERIFY(!sim.m_simPriorities.isEmpty());
    QVERIFY(!sim.m_delays.isEmpty());
    QVERIFY(!sim.m_successorGraph.isEmpty());

    host.m_items = {};
    QVERIFY2(!sim.initialize(), "precondition: a host with no elements must fail to initialize");

    QVERIFY2(!sim.m_initialized, "a failed initialize() must not leave the initialized flag standing");
    QVERIFY2(sim.m_simFeedbackComponents.isEmpty(), "m_simFeedbackComponents must not survive a failed initialize()");
    QVERIFY2(sim.m_simFeedbackComponent.isEmpty(), "m_simFeedbackComponent must not survive a failed initialize()");
    QVERIFY2(sim.m_simEvalCaps.isEmpty(), "m_simEvalCaps must not survive a failed initialize()");
    QVERIFY2(sim.m_simPriorities.isEmpty(), "m_simPriorities must not survive a failed initialize()");
    QVERIFY2(sim.m_delays.isEmpty(), "m_delays must not survive a failed initialize()");
    QVERIFY(sim.m_publishGeneration.isEmpty());
    QVERIFY2(sim.m_successorGraph.isEmpty(), "m_successorGraph must not survive a failed initialize()");
    QVERIFY(sim.m_icOutputMirror.isEmpty());

    // Detach the ports before the connections and elements go out of scope.
    feed1->setStartPort(nullptr);
    feed1->setEndPort(nullptr);
    feed2->setStartPort(nullptr);
    feed2->setEndPort(nullptr);
}

void TestSimulationUnit::testEvaluationCapGrowsAlongEveryCrossComponentEdge()
{
    // THE invariant the per-element evaluation cap has to satisfy, stated directly instead of
    // being inferred from one circuit's numbers: along every edge that leaves a condensation
    // node, the consumer's cap must STRICTLY exceed the producer's. That is what guarantees a
    // member of an oscillating region trips before anything reading it. A trip landing on a
    // reader instead makes the drain abandon the timestamp with nothing canonicalised, leaving
    // a self-inconsistent circuit.
    //
    // The cap is kEvalCapSlack * (depth + 1) * (rounds + 1), so the invariant holds as long as
    // the longest-path depth is computed correctly: a cross-node edge means depth[to] >=
    // depth[from] + 1 and rounds[to] >= rounds[from]. It fails as soon as the relaxation visits
    // a node before one of its predecessors and under-estimates its depth.
    //
    // Driven over a REAL circuit of a few hundred elements with several cyclic components,
    // because only that scale discriminates: relaxing in priority order rather than the
    // condensation's own topological order under-estimates depth by up to 41 levels on the CPU
    // fixtures, while every hand-built circuit small enough to reason about comes out right.
    const QString icPath = TestUtils::cpuComponentsDir() + "level6_program_counter_8bit_arithmetic.panda";
    if (!QFileInfo::exists(icPath)) {
        QSKIP("CPU component fixture missing");
    }

    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *ic = new IC();
    scene->addItem(ic);
    ic->loadFile(QFileInfo(icPath).absoluteFilePath(), QFileInfo(icPath).absolutePath());
    ic->setPos(0, 0);

    auto &sim = *scene->simulation();
    QVERIFY(sim.initialize());
    QVERIFY2(sim.m_simFeedbackComponents.size() > 1,
             "precondition: the fixture must contain several cyclic components, which is the "
             "shape that made the relaxation order matter");
    QVERIFY2(sim.m_sortedElements.size() > 100,
             "precondition: a circuit large enough for the condensation to be deep");

    int checked = 0;
    for (auto it = sim.m_successorGraph.cbegin(); it != sim.m_successorGraph.cend(); ++it) {
        auto *from = it.key();
        const int fromComponent = sim.m_simFeedbackComponent.value(from, -1);
        const int fromCap = sim.m_simEvalCaps.value(from, -1);
        QVERIFY2(fromCap > 0, "every element in the successor graph must have a cap");

        for (auto *to : it.value()) {
            const int toComponent = sim.m_simFeedbackComponent.value(to, -1);
            // Inside one cyclic component every member shares a cap by construction; the
            // invariant is about edges that leave it.
            if (to == from || (fromComponent >= 0 && fromComponent == toComponent)) {
                continue;
            }
            const int toCap = sim.m_simEvalCaps.value(to, -1);
            QVERIFY2(toCap > 0, "every successor must have a cap too");
            QVERIFY2(toCap > fromCap,
                     qPrintable(QStringLiteral("cap did not grow across a condensation edge: "
                                               "%1 (cap %2, component %3) -> %4 (cap %5, component %6)")
                                    .arg(from->label().isEmpty() ? QStringLiteral("<unlabelled>") : from->label())
                                    .arg(fromCap).arg(fromComponent)
                                    .arg(to->label().isEmpty() ? QStringLiteral("<unlabelled>") : to->label())
                                    .arg(toCap).arg(toComponent)));
            ++checked;
        }
    }

    QVERIFY2(checked > 100,
             qPrintable(QStringLiteral("precondition: too few cross-component edges to be "
                                       "meaningful (%1)").arg(checked)));
}

void TestSimulationUnit::testIcOutputValueIsFreshBeforeTheVisualPush()
{
    TestUtils::ScopedInteractiveMode interactiveGuard;
    Application::interactiveMode = true;   // the throttle only applies interactively

    WorkSpace ws;
    auto *scene = ws.scene();
    const QString icFile = QDir::current().filePath("Tests/Fixtures/simple_and.panda");
    if (!QFileInfo::exists(icFile)) {
        QSKIP("simple_and.panda fixture missing");
    }

    auto *ic = new IC();
    scene->addItem(ic);
    ic->loadFile(icFile, QFileInfo(icFile).absolutePath());
    ic->setPos(200, 0);
    QVERIFY2(ic->inputSize() >= 2, "fixture should be a 2-input AND");

    QVector<GraphicElementInput *> switches;
    for (int i = 0; i < ic->inputSize(); ++i) {
        auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
        scene->addItem(sw);
        sw->setPos(0, i * 60);
        auto *c = new Connection();
        scene->addItem(c);
        c->setStartPort(sw->outputPort(0));
        c->setEndPort(ic->inputPort(i));
        switches.append(qobject_cast<GraphicElementInput *>(sw));
    }

    auto &sim = *scene->simulation();
    sim.setVisualThrottleEnabled(true);
    sim.m_visualTickInterval = 50;         // friend seam: make the throttle actually bite
    QVERIFY(sim.initialize());

    for (auto *sw : switches) { sw->setOn(false, 0); }
    sim.m_visualTickCount = 49;            // consume the pending flush
    sim.update();
    QCOMPARE(ic->outputValue(0), Status::Inactive);

    // Drive the AND high. The drain settles the internal boundary node on this tick; the
    // visual push is throttled away. The AND's settled value (Active) deliberately DIFFERS
    // from its power-on default, or the assertion could not tell stale from fresh.
    for (auto *sw : switches) { sw->setOn(true, 0); }
    sim.update();

    QVERIFY2(ic->outputValue(0) == Status::Active,
             "ic->outputValue() must reflect the settled drain immediately: it is what MCP "
             "get_output_value reads, and mirroring is logic, not presentation");
}

namespace {

/// A NOT gate that throws from updateLogic(). No shipped element does, and the exception path
/// is documented behaviour -- Application::notify() catches it and the app keeps running -- so
/// the drain's failure mode needs one to be observable at all.
class ThrowingNot : public Not
{
public:
    void updateLogic() override { throw std::runtime_error("probe: updateLogic failed"); }
};

} // namespace

void TestSimulationUnit::testAbortedDrainDoesNotLeaveTheFixedPointFlagSet()
{
    WorkSpace ws;
    auto *scene = ws.scene();
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *bad = new ThrowingNot();
    scene->addItem(sw);
    scene->addItem(bad);
    sw->setPos(0, 0);
    bad->setPos(100, 0);
    auto *c = new Connection();
    scene->addItem(c);
    c->setStartPort(sw->outputPort(0));
    c->setEndPort(bad->inputPort(0));

    auto &sim = *scene->simulation();
    QVERIFY(sim.initialize());

    // Pretend the previous tick concluded a fixed point, which is what makes this a
    // discriminator: assigning the flag only at the end of the drain would leave it true.
    // A source change is required as well, or the idle skip returns before the drain and
    // nothing is evaluated at all.
    sim.m_atFixedPoint = true;
    qobject_cast<GraphicElementInput *>(sw)->setOn(true, 0);

    bool threw = false;
    try {
        sim.update();
    } catch (const std::exception &) {
        threw = true;
    }
    QVERIFY2(threw, "precondition: the drain must actually have been aborted");

    QVERIFY2(!sim.m_atFixedPoint,
             "an aborted drain must not leave the fixed-point conclusion standing: later ticks "
             "would be skipped as idle while events are still pending");
}

void TestSimulationUnit::testDelayFreeTypesIgnoreAPropagationDelayOverride()
{
    WorkSpace ws;
    auto *scene = ws.scene();
    auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *node = ElementFactory::buildElement(ElementType::Node);
    auto *gate = ElementFactory::buildElement(ElementType::Not);
    scene->addItem(sw);
    scene->addItem(node);
    scene->addItem(gate);
    sw->setPos(0, 0);
    node->setPos(100, 0);
    gate->setPos(200, 0);

    const auto wire = [scene](GraphicElement *a, int ap, GraphicElement *b, int bp) {
        auto *c = new Connection();
        scene->addItem(c);
        c->setStartPort(a->outputPort(ap));
        c->setEndPort(b->inputPort(bp));
    };
    wire(sw, 0, node, 0);
    wire(node, 0, gate, 0);

    // Exactly what GraphicElementSerializer::load() does for a file carrying the key.
    node->setPropagationDelay(500);
    gate->setPropagationDelay(42);
    QVERIFY(node->hasPropagationDelayOverride());

    auto &sim = *scene->simulation();
    QVERIFY(sim.initialize());

    QVERIFY2(sim.m_delays.value(node, 999) == SimTime{0},
             "a Node is delay-free by design: the engine must not honour an override no UI or "
             "API is willing to create");
    QCOMPARE(sim.m_delays.value(gate, 0), SimTime{42});   // a real gate still honours its override

    // The runtime channel obeys the same rule, so the map cannot acquire one later either.
    sim.setElementDelay(node, 500);
    QCOMPARE(sim.m_delays.value(node, 999), SimTime{0});
    sim.setElementDelay(gate, 7);
    QCOMPARE(sim.m_delays.value(gate, 0), SimTime{7});
}
