// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestSimulation.h"

#include <QElapsedTimer>
#include <QTemporaryDir>

#include "App/Core/Application.h"
#include "App/Core/SimulationHost.h"
#include "App/Element/Element.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Commands.h"
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
    QCOMPARE(TestUtils::inputStatus(led), Status::Active);

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
    QCOMPARE(TestUtils::inputStatus(led), Status::Inactive);
    sw->setOn(true);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(led), Status::Active);
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
    QCOMPARE(TestUtils::inputStatus(andGate), Status::Active);

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
    QCOMPARE(TestUtils::inputStatus(led), Status::Active);
    sw->setOn(false);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(led), Status::Inactive);
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

void TestSimulationUnit::testCollectSequentialElementsSkipsNullElements()
{
    StubSimulationHost host;
    Simulation sim(&host);

    QVector<Element *> elements{nullptr};
    sim.collectSequentialElements(elements);

    QVERIFY(sim.m_sequentialElements.isEmpty());
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

void TestSimulationUnit::testRebuildKeepsClockLevel()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *clock = new Clock();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{clock}, scene));
    auto *sim = scene->simulation();
    // Locked, the clock does not follow wall time; its first build starts it HIGH.
    clock->setLocked(true);
    sim->update();
    QCOMPARE(clock->isOn(), true);
    clock->setOn(false);

    // A structural edit elsewhere rebuilds the simulation; the clock keeps its level, and a clock the edit adds starts.
    auto *added = new Clock();
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{added}, scene));
    sim->update();
    QCOMPARE(clock->isOn(), false);
    QCOMPARE(added->isOn(), true);

    // A restart starts every clock afresh.
    sim->restart();
    sim->update();
    QCOMPARE(clock->isOn(), true);
}

void TestSimulationUnit::testResumeAfterAPausedRebuildKeepsClockPhase()
{
    // A structural edit while paused rebuilds the simulation on the resume; the pause still shifts every clock, the
    // kept one and the one the edit adds, so neither catches up on the pause one half-period per tick.
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *sim = scene->simulation();
    auto *clock = new Clock();
    clock->setFrequency(1.0);
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{clock}, scene));
    sim->start();
    sim->update();
    sim->stop();

    // Three half-periods of pause, under the clock's catch-up limit
    QTest::qWait(1600);
    auto *added = new Clock();
    added->setFrequency(1.0);
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{added}, scene));
    sim->start();
    QVERIFY(!sim->m_hasPausedAt);

    int toggles = 0;
    int addedToggles = 0;
    for (int tick = 0; tick < 3; ++tick) {
        const bool level = clock->isOn();
        const bool addedLevel = added->isOn();
        sim->update();
        toggles += (clock->isOn() != level) ? 1 : 0;
        addedToggles += (added->isOn() != addedLevel) ? 1 : 0;
    }
    sim->stop();
    QVERIFY2(toggles <= 1, qPrintable(QString("the kept clock toggled %1 times in 3 ticks").arg(toggles)));
    QVERIFY2(addedToggles <= 1, qPrintable(QString("the added clock toggled %1 times in 3 ticks").arg(addedToggles)));
}

void TestSimulationUnit::testRebuildKeepsFlipFlopState_data()
{
    QTest::addColumn<bool>("inIC");
    QTest::newRow("scene") << false;
    QTest::newRow("inside an IC") << true;
}

void TestSimulationUnit::testRebuildKeepsFlipFlopState()
{
    QFETCH(bool, inIC);

    WorkSpace workspace;
    auto *scene = workspace.scene();
    CircuitBuilder builder(scene);
    auto *data = new InputSwitch();
    auto *clock = new InputSwitch();
    auto *led = new Led();
    builder.add(data, clock, led);
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    if (inIC) {
        // A sub-circuit holding the flip-flop element itself: one built from gates settles to a level of its own.
        const QString icFile = dir.filePath("flipflop.panda");
        {
            WorkSpace definition;
            CircuitBuilder inner(definition.scene());
            auto *innerData = new InputSwitch();
            auto *innerClock = new InputSwitch();
            auto *flipFlop = new DFlipFlop();
            auto *innerQ = new Led();
            innerData->setLabel("D");
            innerClock->setLabel("Clock");
            innerQ->setLabel("Q");
            innerClock->setPos(0, 64);
            inner.add(innerData, innerClock, flipFlop, innerQ);
            inner.connect(innerData, 0, flipFlop, 0);
            inner.connect(innerClock, 0, flipFlop, 1);
            inner.connect(flipFlop, 0, innerQ, 0);
            definition.save(icFile);
        }
        auto *ic = new IC();
        ic->loadFile(icFile, dir.path());
        builder.add(ic);
        builder.connect(data, 0, ic, "D");
        builder.connect(clock, 0, ic, "Clock");
        builder.connect(ic, "Q", led, 0);
    } else {
        auto *flipFlop = new DFlipFlop();
        builder.add(flipFlop);
        builder.connect(data, 0, flipFlop, 0);
        builder.connect(clock, 0, flipFlop, 1);
        builder.connect(flipFlop, 0, led, 0);
    }
    auto *sim = builder.initSimulation();

    // A rising edge stores D = 1; then the clock falls and D goes to 0.
    data->setOn(true);
    clock->setOn(false);
    sim->update();
    clock->setOn(true);
    sim->update();
    clock->setOn(false);
    data->setOn(false);
    sim->update();
    QCOMPARE(led->inputPort(0)->status(), Status::Active);

    // A structural edit elsewhere rebuilds the simulation, with no edge: the flip-flop keeps its Q.
    scene->receiveCommand(new AddItemsCommand(QList<QGraphicsItem *>{new And()}, scene));
    sim->update();
    QCOMPARE(led->inputPort(0)->status(), Status::Active);

    // A restart starts it afresh.
    sim->restart();
    sim->update();
    QCOMPARE(led->inputPort(0)->status(), Status::Inactive);
}
