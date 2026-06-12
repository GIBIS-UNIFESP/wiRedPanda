// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestSimulation.h"

#include "App/Element/GraphicElements/Clock.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Nor.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
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
    // Clock elements can be added and removed across re-initializations
    WorkSpace workspace;
    auto *sim = workspace.scene()->simulation();

    auto *clock = new Clock;
    auto *led = new Led;
    workspace.scene()->addItem(clock);
    workspace.scene()->addItem(led);
    QVERIFY(sim->initialize());
    sim->update();

    workspace.scene()->removeItem(clock);
    delete clock;

    // The simulation must re-initialize and tick cleanly without the clock
    QVERIFY(sim->initialize());
    sim->update();
}

void TestSimulationUnit::testFeedbackLoopDetection()
{
    // A cross-coupled NOR pair (SR latch) is a feedback loop; a free-standing
    // input is not
    WorkSpace workspace;
    auto *sim = workspace.scene()->simulation();

    auto *nor1 = new Nor;
    auto *nor2 = new Nor;
    auto *input = new InputSwitch;
    workspace.scene()->addItem(nor1);
    workspace.scene()->addItem(nor2);
    workspace.scene()->addItem(input);

    auto connect = [&workspace](GraphicElement *from, GraphicElement *to, int toPort) {
        auto *conn = new QNEConnection;
        conn->setStartPort(from->outputPort());
        conn->setEndPort(to->inputPort(toPort));
        workspace.scene()->addItem(conn);
    };
    connect(nor1, nor2, 0);
    connect(nor2, nor1, 0);
    connect(input, nor1, 1);

    QVERIFY(sim->initialize());

    QVERIFY(sim->isInFeedbackLoop(nor1));
    QVERIFY(sim->isInFeedbackLoop(nor2));
    QVERIFY(!sim->isInFeedbackLoop(input));
}

void TestSimulationUnit::testElementRemovalMidSimulation()
{
    // Elements can be removed mid-run; the simulation keeps ticking
    WorkSpace workspace;
    auto *sim = workspace.scene()->simulation();

    auto *input = new InputSwitch;
    auto *led = new Led;
    workspace.scene()->addItem(input);
    workspace.scene()->addItem(led);

    auto *conn = new QNEConnection;
    conn->setStartPort(input->outputPort());
    conn->setEndPort(led->inputPort());
    workspace.scene()->addItem(conn);

    QVERIFY(sim->initialize());
    input->setOn(true);
    sim->update();
    QVERIFY(TestUtils::getInputStatus(led));

    // Remove the source mid-simulation (deleting it also detaches the wire)
    workspace.scene()->removeItem(input);
    delete input;

    QVERIFY(sim->initialize());
    sim->update();
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
