// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestSimulationBlocker.h"

#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationBlocker.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"

// ============================================================
// RAII Pattern Tests
// ============================================================

void TestSimulationBlocker::testRAIIPatternStopsSimulation()
{
    // Test that SimulationBlocker stops simulation on construction
    QuickCircuitBuilder builder;
    populateSimpleCircuit(builder);

    Simulation sim(&builder);

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
}

void TestSimulationBlocker::testRAIIPatternRestartsSimulation()
{
    // Test that SimulationBlocker restarts simulation on destruction
    QuickCircuitBuilder builder;
    populateSimpleCircuit(builder);

    Simulation sim(&builder);

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
}

void TestSimulationBlocker::testNestedSimulationBlockers()
{
    // Test that nested blockers work correctly (inner blocker should not restart outer blocker)
    QuickCircuitBuilder builder;
    populateSimpleCircuit(builder);

    Simulation sim(&builder);

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
}

void TestSimulationBlocker::testBlockerOnAlreadyStoppedSimulationDoesNotStartIt()
{
    // The branch testRAIIPatternStopsSimulation/RestartsSimulation never exercise standalone:
    // SimulationBlocker's constructor only flags for restart if the simulation was actually
    // running. A simulation that was never started (or intentionally left stopped) must stay
    // stopped once the blocker goes out of scope, not be started as a side effect.
    QuickCircuitBuilder builder;
    populateSimpleCircuit(builder);

    Simulation sim(&builder);
    QVERIFY(!sim.isRunning());

    {
        SimulationBlocker blocker(&sim);
        QVERIFY(!sim.isRunning());
    }

    QVERIFY2(!sim.isRunning(),
             "A blocker constructed on an already-stopped simulation must not start it on destruction");
}

// ============================================================
// Helper Functions
// ============================================================

void TestSimulationBlocker::populateSimpleCircuit(QuickCircuitBuilder &builder)
{
    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *btn = ElementFactory::buildElement(ElementType::InputButton);

    builder.addOwnedElement(and1);
    builder.addOwnedElement(btn);

    and1->setPos(100, 100);
    btn->setPos(0, 100);
}
