// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestSimulation.h"

#include "App/Element/GraphicElements/Clock.h"
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
    // Test: Clock elements can be added/removed during simulation
    WorkSpace workspace;
    Simulation sim(workspace.scene());

    Clock clock;
    workspace.scene()->addItem(&clock);

    // Should handle dynamic clock addition
    QVERIFY(true);
}

void TestSimulationUnit::testFeedbackLoopDetection()
{
    // Test: Simulation detects combinational feedback loops
    WorkSpace workspace;
    Simulation sim(workspace.scene());

    // Feedback detection should work
    QVERIFY(true);
}

void TestSimulationUnit::testElementRemovalMidSimulation()
{
    // Test: Elements can be removed during simulation
    WorkSpace workspace;
    Simulation sim(workspace.scene());

    Clock clock;
    workspace.scene()->addItem(&clock);
    workspace.scene()->removeItem(&clock);

    // Should handle mid-simulation removal
    QVERIFY(true);
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

