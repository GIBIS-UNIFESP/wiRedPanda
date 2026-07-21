// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestSimulationUnit.h"

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"
#include "Tests/QuickShell/QuickCpuTestUtils.h"

void TestSimulationUnit::testSimulationWithNoElements()
{
    // Test: Simulation handles empty scene gracefully
    QuickCircuitBuilder builder;
    auto *sim = builder.simulation();

    QVERIFY(!sim->isRunning()); // starts stopped
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
    builder.add(sw, led);

    builder.connect(sw, 0, led, 0);

    sim->start();
    QVERIFY(sim->isRunning());
    sim->update();

    // Add a wired Clock while the simulation runs.
    auto *clock = new Clock();
    auto *clockLed = new Led();
    builder.add(clock, clockLed);
    auto *clockConn = new Connection();
    clockConn->setStartPort(clock->outputPort(0));
    clockConn->setEndPort(clockLed->inputPort(0));
    sim->restart();
    sim->update();

    // The pre-existing path must keep propagating correctly with the clock in place.
    sw->setOn(true);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(led), true);

    // Remove the Clock (connection first, mirroring scene deletion order)
    // while the simulation is still running.
    delete clockConn;
    builder.removeElement(clock);
    delete clock;
    sim->restart();
    sim->update();

    // The simulation must survive the removal and keep computing correctly.
    QVERIFY(sim->isRunning());
    sw->setOn(false);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(led), false);
    sw->setOn(true);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(led), true);

    builder.removeElement(clockLed);
    delete clockLed;
}

void TestSimulationUnit::testElementRemovalMidSimulation()
{
    // Removing a wired logic element from a RUNNING simulation must not leave
    // stale pointers in the sorted-element or connection lists (the H2-class
    // use-after-free), and the simulation must keep computing afterwards.
    QuickCircuitBuilder builder;
    auto *sim = builder.simulation();

    auto *sw = new InputSwitch();
    auto *andGate = new And();
    auto *led = new Led();
    builder.add(sw, andGate, led);

    auto *connIn = builder.connect(sw, 0, andGate, 0);
    auto *connOut = builder.connect(andGate, 0, led, 0);

    sim->start();
    QVERIFY(sim->isRunning());
    sw->setOn(true);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(andGate), true);

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
    QuickCircuitBuilder builder;

    auto *vcc = builder.addOwnedElement(ElementFactory::buildElement(ElementType::InputVcc));
    auto *notGate = builder.addOwnedElement(ElementFactory::buildElement(ElementType::Not));

    auto *conn = new Connection();
    conn->setStartPort(vcc->outputPort(0));
    conn->setEndPort(notGate->inputPort(0));

    // The NOT output stays unwired: undriven (Unknown) until a tick computes it
    QCOMPARE(notGate->outputPort(0)->status(), Status::Unknown);

    auto *sim = builder.initSimulation();
    sim->update();

    // NOT(1) = 0 — and the unwired output port's visual must reflect it
    QCOMPARE(notGate->outputPort(0)->status(), Status::Inactive);
    QCOMPARE(notGate->outputPort(0)->status(), notGate->outputValue(0));
}

void TestSimulationUnit::testAddingClockToClocklessRunningSimulationStartsTicking()
{
    // Regression for the gap found while designing Simulation's deadline-based timer
    // scheduling: a circuit with no Clock has nothing to wait for, so start()'s timer
    // stays stopped. Adding the *first* Clock afterwards is a structural edit
    // (restart()) and must actually resume ticking on its own -- via the real QTimer and
    // event loop, with no manual update() call from this test -- not silently stay dead.
    QuickCircuitBuilder builder;
    auto *sim = builder.simulation();

    auto *sw = new InputSwitch();
    auto *led = new Led();
    builder.add(sw, led);
    builder.connect(sw, 0, led, 0);

    sim->start();
    QVERIFY(sim->isRunning());

    // A fast clock (2kHz, 250us half-period) driving a fresh LED, wired in after start().
    auto *clock = new Clock();
    clock->setFrequency(2000.0);
    auto *clockLed = new Led();
    builder.add(clock, clockLed);
    builder.connect(clock, 0, clockLed, 0);
    sim->restart();

    // No manual sim->update() anywhere below -- only the real timer, woken by restart(),
    // may drive this.
    QVERIFY(QTest::qWaitFor([&] {
        return clockLed->inputPort(0)->status() == Status::Active;
    }, 500));
}

void TestSimulationUnit::testInteractiveInputWakesStoppedTimer()
{
    // Mirrors what CanvasItem::activateOnPress() does on a real click: write the input's
    // own value, then call wakeSoon(). With no Clock in the circuit, start()'s timer is
    // stopped -- downstream propagation to the LED must still happen promptly via the
    // one prompt wake wakeSoon() schedules, not require a manual update() call.
    QuickCircuitBuilder builder;
    auto *sim = builder.simulation();

    auto *sw = new InputSwitch();
    auto *led = new Led();
    builder.add(sw, led);
    builder.connect(sw, 0, led, 0);

    sim->start();
    QVERIFY(sim->isRunning());

    sw->setOn(true);
    sim->wakeSoon();

    QVERIFY(QTest::qWaitFor([&] {
        return led->inputPort(0)->status() == Status::Active;
    }, 500));
}

void TestSimulationUnit::testRescheduleTimerAfterFrequencyIncrease()
{
    // Mirrors QuickElementEditor::applyProperty()'s/the MCP element handler's live
    // frequency-edit hook: a clock's own timing can change after start() already
    // scheduled a wake for its old (slower) deadline. rescheduleTimer() must re-derive
    // the schedule from the clock's new frequency, not leave the stale, far-future wake
    // in place.
    QuickCircuitBuilder builder;
    auto *sim = builder.simulation();

    auto *clock = new Clock();
    clock->setFrequency(1.0); // 500ms half-period -- far outside this test's timeout below
    auto *led = new Led();
    builder.add(clock, led);
    builder.connect(clock, 0, led, 0);

    sim->start();
    QVERIFY(sim->isRunning());

    clock->setFrequency(2000.0); // 250us half-period
    sim->rescheduleTimer();

    // Only reachable within this short timeout if the reschedule actually took effect --
    // the original 1Hz deadline wouldn't fire for another ~500ms.
    QVERIFY(QTest::qWaitFor([&] {
        return led->inputPort(0)->status() == Status::Active;
    }, 100));
}
