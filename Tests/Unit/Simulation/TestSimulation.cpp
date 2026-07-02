// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestSimulation.h"

#include <QElapsedTimer>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationBlocker.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/TestUtils.h"

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
    // InputSwitch click on a running circuit. Resuming used to call resetClock(), which
    // forces the output HIGH: an out-of-thin-air rising edge fed to every clock-driven
    // element (counters advanced without a real edge). The resume must preserve the level.
    {
        SimulationBlocker blocker(sim);
    }
    QVERIFY2(!clock->isOn(),
             "resume after a SimulationBlocker cycle forced the clock HIGH mid-LOW-phase");

    sim->stop();
}
