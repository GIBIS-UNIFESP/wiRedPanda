// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestSimulationUnit.h"

#include <QElapsedTimer>
#include <QFile>

#include "App/Core/Application.h"
#include "App/Core/ItemWithId.h"
#include "App/Core/SimulationHost.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/QuickShell/IC/QuickTestUtils.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"

using QuickTestUtils::inputStatus;

namespace {
// Minimal SimulationHost stub giving full control over simulationItems() (including
// injecting nullptrs), so Simulation::initialize()'s defensive item-list handling can be
// tested in isolation from a real host.
class StubSimulationHost : public SimulationHost
{
public:
    QList<ItemWithId *> simulationItems() const override { return m_items; }
    void setMuted(bool muted) override { m_muted = muted; }

    QList<ItemWithId *> m_items;
    bool m_muted = false;
};
} // namespace

void TestSimulationUnit::testSimulationWithNoElements()
{
    // Test: Simulation handles empty scene gracefully
    QuickCircuitBuilder builder;
    Simulation sim(&builder);

    QVERIFY(!sim.isRunning()); // starts stopped
}

void TestSimulationUnit::testAddRemoveClockDuringSimulation()
{
    // A Clock added to (and later removed from) a RUNNING simulation must be
    // integrated into / dropped from the clock-update list without corrupting
    // the rest of the circuit — the dangling-clock-pointer crash class.
    QuickCircuitBuilder builder;
    auto *sim = builder.simulation();

    auto *sw = new InputSwitch();
    auto *led = new Led();
    builder.addOwnedElement(sw);
    builder.addOwnedElement(led);

    builder.connect(sw, 0, led, 0);

    sim->start();
    QVERIFY(sim->isRunning());
    sim->update();

    // Add a wired Clock while the simulation runs.
    auto *clock = new Clock();
    auto *clockLed = new Led();
    builder.addOwnedElement(clock);
    builder.addOwnedElement(clockLed);
    auto *clockConn = builder.connect(clock, 0, clockLed, 0);
    sim->restart();
    sim->update();

    // The pre-existing path must keep propagating correctly with the clock in place.
    sw->setOn(true);
    sim->update();
    QCOMPARE(inputStatus(led), true);

    // Remove the Clock (connection first, then the element) while the
    // simulation is still running.
    delete clockConn;
    builder.removeElement(clock);
    delete clock;
    sim->restart();
    sim->update();

    // The simulation must survive the removal and keep computing correctly.
    QVERIFY(sim->isRunning());
    sw->setOn(false);
    sim->update();
    QCOMPARE(inputStatus(led), false);
    sw->setOn(true);
    sim->update();
    QCOMPARE(inputStatus(led), true);
}

void TestSimulationUnit::testElementRemovalMidSimulation()
{
    // Removing a wired logic element from a RUNNING simulation must not leave
    // stale pointers in the sorted-element or connection lists, and the
    // simulation must keep computing afterwards.
    QuickCircuitBuilder builder;
    auto *sim = builder.simulation();

    auto *sw = new InputSwitch();
    auto *andGate = new And();
    auto *led = new Led();
    builder.addOwnedElement(sw);
    builder.addOwnedElement(andGate);
    builder.addOwnedElement(led);

    auto *connIn = builder.connect(sw, 0, andGate, 0);
    auto *connOut = builder.connect(andGate, 0, led, 0);

    sim->start();
    QVERIFY(sim->isRunning());
    sw->setOn(true);
    sim->update();
    QCOMPARE(inputStatus(andGate), true);

    // Remove the AND gate mid-run (connections first, then the element).
    delete connIn;
    delete connOut;
    builder.removeElement(andGate);
    delete andGate;
    sim->restart();
    sim->update();
    sim->update();

    // The simulation must survive and still integrate NEW topology correctly:
    // wire the switch directly to the LED and verify propagation resumes.
    QVERIFY(sim->isRunning());
    auto *directConn = builder.connect(sw, 0, led, 0);
    QVERIFY(directConn != nullptr);
    sim->restart();
    sim->update();
    QCOMPARE(inputStatus(led), true);
    sw->setOn(false);
    sim->update();
    QCOMPARE(inputStatus(led), false);
}

void TestSimulationUnit::testSimulationStartStopNoBreadcrumbsB22()
{
    // sentryBreadcrumb is a no-op without HAVE_SENTRY linked, so verify at the
    // source level: Simulation::start() and ::stop() must not contain a
    // sentryBreadcrumb call. Every SimulationBlocker scope hits both, and
    // would flood the 100-slot ring buffer.
#define QUOTE(string) _QUOTE(string)
#define _QUOTE(string) #string
    const QString sourcePath =
        QString(QUOTE(CURRENTDIR)) + "/../App/Simulation/Simulation.cpp";
#undef _QUOTE
#undef QUOTE
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
    // Computed values must be pushed onto output-port visuals even when no
    // wire is attached. A connection-based walk skips them, so a flip-flop's
    // unwired -Q (or any unwired gate output) keeps a stale color.
    QuickCircuitBuilder builder;

    auto *vcc = ElementFactory::buildElement(ElementType::InputVcc);
    auto *notGate = ElementFactory::buildElement(ElementType::Not);
    builder.addOwnedElement(vcc);
    builder.addOwnedElement(notGate);

    builder.connect(vcc, 0, notGate, 0);

    // The NOT output stays unwired: undriven (Unknown) until a tick computes it
    QCOMPARE(notGate->outputPort(0)->status(), Status::Unknown);

    Simulation sim(&builder);
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

    QVector<GraphicElement *> elements{nullptr};
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
    QuickTestUtils::ScopedInteractiveMode interactiveGuard;
    Application::interactiveMode = true;

    QuickCircuitBuilder builder;
    auto *sw = new InputSwitch();
    auto *led = new Led();
    builder.addOwnedElement(sw);
    builder.addOwnedElement(led);
    builder.connect(sw, 0, led, 0);

    Simulation sim(&builder);
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
    QuickCircuitBuilder builder;
    auto *sim = builder.simulation();

    auto *clock = qobject_cast<Clock *>(ElementFactory::buildElement(ElementType::Clock));
    QVERIFY(clock);
    // 10 Hz ⇒ 50 ms per phase: slow enough that the post-resume assertion below runs well
    // inside the LOW phase, fast enough for the first falling edge to land quickly.
    clock->setFrequency(10.0);
    builder.addOwnedElement(clock);

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
