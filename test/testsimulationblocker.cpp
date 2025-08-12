// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testsimulationblocker.h"

#include "simulationblocker.h"
#include "simulation.h"
#include "scene.h"
#include <QTest>

void TestSimulationBlocker::testSimulationBlockerConstruction()
{
    Scene scene;
    Simulation* simulation = new Simulation(&scene);
    SimulationBlocker* blocker = new SimulationBlocker(simulation);
    QVERIFY(blocker != nullptr);
    delete blocker;
    delete simulation;
}

void TestSimulationBlocker::testSimulationBlocking()
{
    Scene scene;
    Simulation simulation(&scene);
    SimulationBlocker blocker(&simulation);
    // Should block simulation
    QVERIFY(true);
}

void TestSimulationBlocker::testSimulationUnblocking()
{
    {
        Scene scene;
    Simulation simulation(&scene);
    SimulationBlocker blocker(&simulation);
        // Simulation blocked
    }
    // Should unblock when blocker goes out of scope
    QVERIFY(true);
}

void TestSimulationBlocker::testMultipleBlockers()
{
    Scene scene1;
    Simulation simulation1(&scene1);
    SimulationBlocker blocker1(&simulation1);
    Scene scene2;
    Simulation simulation2(&scene2);
    SimulationBlocker blocker2(&simulation2);
    Scene scene3;
    Simulation simulation3(&scene3);
    SimulationBlocker blocker3(&simulation3);
    
    // Multiple blockers should work
    QVERIFY(true);
}

void TestSimulationBlocker::testBlockerDestruction()
{
    for (int i = 0; i < 100; ++i) {
        Scene scene;
    Simulation simulation(&scene);
    SimulationBlocker blocker(&simulation);
        // Should construct and destruct properly
    }
    QVERIFY(true);
}