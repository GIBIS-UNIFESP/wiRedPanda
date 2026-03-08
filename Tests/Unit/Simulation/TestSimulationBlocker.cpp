// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Simulation/TestSimulationBlocker.h"

#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationBlocker.h"
#include "Tests/Common/TestUtils.h"

// ============================================================
// RAII Pattern Tests
// ============================================================

void TestSimulationBlocker::testRAIIPatternStopsSimulation()
{
    // Test that SimulationBlocker stops simulation on construction
    auto *scene = createSimpleCircuit();
    QVERIFY(scene != nullptr);

    Simulation sim(scene);  // Simulation takes ownership of scene

    // Start simulation
    sim.start();
    QVERIFY(sim.isRunning());

    {
        // Create blocker - should stop simulation
        SimulationBlocker blocker(&sim);
        QVERIFY(!sim.isRunning());
    }

    // After blocker scope, simulation should be running again
    QVERIFY(sim.isRunning());

    // Note: Do NOT delete scene here - Simulation owns it and will delete it
}

void TestSimulationBlocker::testRAIIPatternRestartsSimulation()
{
    // Test that SimulationBlocker restarts simulation on destruction
    auto *scene = createSimpleCircuit();
    QVERIFY(scene != nullptr);

    Simulation sim(scene);  // Simulation takes ownership of scene

    // Start simulation
    sim.start();
    QVERIFY(sim.isRunning());

    // Create and destroy blocker
    {
        SimulationBlocker blocker(&sim);
        QVERIFY(!sim.isRunning());
    }
    // Blocker destroyed - simulation should restart

    QVERIFY(sim.isRunning());

    // Stop simulation for cleanup
    sim.stop();

    // Note: Do NOT delete scene here - Simulation owns it and will delete it
}

void TestSimulationBlocker::testNestedSimulationBlockers()
{
    // Test that nested blockers work correctly (inner blocker should not restart outer blocker)
    auto *scene = createSimpleCircuit();
    QVERIFY(scene != nullptr);

    Simulation sim(scene);  // Simulation takes ownership of scene

    // Start simulation
    sim.start();
    QVERIFY(sim.isRunning());

    {
        // Outer blocker
        SimulationBlocker blocker1(&sim);
        QVERIFY(!sim.isRunning());

        {
            // Inner blocker - simulation already stopped
            SimulationBlocker blocker2(&sim);
            QVERIFY(!sim.isRunning());
        }
        // Inner blocker destroyed - simulation may or may not restart
        // depending on m_restart flag (it tracks if it was running before blocker2)

        QVERIFY(!sim.isRunning());
    }
    // Outer blocker destroyed - simulation should restart

    QVERIFY(sim.isRunning());

    // Stop simulation for cleanup
    sim.stop();

    // Note: Do NOT delete scene here - Simulation owns it and will delete it
}

// ============================================================
// Helper Functions
// ============================================================

Scene *TestSimulationBlocker::createSimpleCircuit()
{
    auto *scene = new Scene();

    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *btn = ElementFactory::buildElement(ElementType::InputButton);

    scene->addItem(and1);
    scene->addItem(btn);

    and1->setPos(100, 100);
    btn->setPos(0, 100);

    return scene;
}
