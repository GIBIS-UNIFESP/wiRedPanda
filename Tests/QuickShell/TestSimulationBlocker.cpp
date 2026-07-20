// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestSimulationBlocker.h"

#include <memory>

#include <QTest>

#include "App/Element/ElementFactory.h"
#include "App/Simulation/Simulation.h"
#include "App/Simulation/SimulationBlocker.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"

// ============================================================
// RAII Pattern Tests
// ============================================================

void TestSimulationBlocker::testRAIIPatternStopsSimulation()
{
    // Test that SimulationBlocker stops simulation on construction
    std::unique_ptr<QuickCircuitBuilder> builder(createSimpleCircuit());
    QVERIFY(builder != nullptr);
    auto *sim = builder->simulation();

    // Start simulation
    sim->start();
    QVERIFY(sim->isRunning());

    {
        // Create blocker - should stop simulation
        SimulationBlocker blocker(sim);
        QVERIFY(!sim->isRunning());
    }

    // After blocker scope, simulation should be running again
    QVERIFY(sim->isRunning());

    sim->stop();
}

void TestSimulationBlocker::testRAIIPatternRestartsSimulation()
{
    // Test that SimulationBlocker restarts simulation on destruction
    std::unique_ptr<QuickCircuitBuilder> builder(createSimpleCircuit());
    QVERIFY(builder != nullptr);
    auto *sim = builder->simulation();

    // Start simulation
    sim->start();
    QVERIFY(sim->isRunning());

    // Create and destroy blocker
    {
        SimulationBlocker blocker(sim);
        QVERIFY(!sim->isRunning());
    }
    // Blocker destroyed - simulation should restart

    QVERIFY(sim->isRunning());

    // Stop simulation for cleanup
    sim->stop();
}

void TestSimulationBlocker::testNestedSimulationBlockers()
{
    // Test that nested blockers work correctly (inner blocker should not restart outer blocker)
    std::unique_ptr<QuickCircuitBuilder> builder(createSimpleCircuit());
    QVERIFY(builder != nullptr);
    auto *sim = builder->simulation();

    // Start simulation
    sim->start();
    QVERIFY(sim->isRunning());

    {
        // Outer blocker
        SimulationBlocker blocker1(sim);
        QVERIFY(!sim->isRunning());

        {
            // Inner blocker - simulation already stopped
            SimulationBlocker blocker2(sim);
            QVERIFY(!sim->isRunning());
        }
        // Inner blocker destroyed - simulation may or may not restart
        // depending on m_restart flag (it tracks if it was running before blocker2)

        QVERIFY(!sim->isRunning());
    }
    // Outer blocker destroyed - simulation should restart

    QVERIFY(sim->isRunning());

    // Stop simulation for cleanup
    sim->stop();
}

// ============================================================
// Helper Functions
// ============================================================

QuickCircuitBuilder *TestSimulationBlocker::createSimpleCircuit()
{
    auto *builder = new QuickCircuitBuilder();

    auto *and1 = builder->addOwnedElement(ElementFactory::buildElement(ElementType::And));
    auto *btn = builder->addOwnedElement(ElementFactory::buildElement(ElementType::InputButton));

    and1->setPos(100, 100);
    btn->setPos(0, 100);

    return builder;
}
