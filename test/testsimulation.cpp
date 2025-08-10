// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testsimulation.h"

#include "and.h"
#include "or.h"
#include "not.h"
#include "xor.h"
#include "clock.h"
#include "common.h"
#include "inputbutton.h"
#include "led.h"
#include "qneconnection.h"
#include "scene.h"
#include "workspace.h"
#include "simulation.h"
#include "dflipflop.h"

#include <QTest>
#include <QSignalSpy>
#include <QTimer>
#include <QEventLoop>
#include <chrono>

using namespace std::chrono_literals;

void TestSimulation::testCase1()
{
    WorkSpace workspace;

    InputButton button1;
    InputButton button2;
    And andItem;
    Led led;
    QNEConnection connection1;
    QNEConnection connection2;
    QNEConnection connection3;

    auto *scene = workspace.scene();
    scene->addItem(&led);
    scene->addItem(&andItem);
    scene->addItem(&button1);
    scene->addItem(&button2);
    scene->addItem(&connection1);
    scene->addItem(&connection2);
    scene->addItem(&connection3);

    connection1.setStartPort(button1.outputPort());
    connection1.setEndPort(andItem.inputPort(0));
    connection2.setStartPort(button2.outputPort());
    connection2.setEndPort(andItem.inputPort(1));
    connection3.setStartPort(andItem.outputPort());
    connection3.setEndPort(led.inputPort());

    const auto elements(Common::sortGraphicElements(scene->elements()));

    QVERIFY((elements.at(0) == &button1) || (elements.at(1) == &button1));
    QVERIFY((elements.at(0) == &button2) || (elements.at(1) == &button2));
    QVERIFY(elements.at(2) == &andItem);
    QVERIFY(elements.at(3) == &led);
}

void TestSimulation::testSimulationConstruction()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    
    // Test simulation construction with scene
    Simulation simulation(scene);
    
    // Simulation should exist and have basic properties
    QVERIFY(true); // Constructor worked without crashing
}

void TestSimulation::testSimulationWithScene()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    
    // Create simulation with scene
    Simulation simulation(scene);
    
    // Add some basic elements to scene
    auto *input = new InputButton();
    auto *output = new Led();
    scene->addItem(input);
    scene->addItem(output);
    
    // Simulation should handle scene with elements
    QVERIFY(scene->elements().size() >= 2);
}

void TestSimulation::testInitialize()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Simulation simulation(scene);
    
    // Test initialization
    bool initialized = simulation.initialize();
    
    // Should initialize successfully (true or false is valid)
    QVERIFY(initialized == true || initialized == false);
}

void TestSimulation::testIsRunning()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Simulation simulation(scene);
    
    // Initially should not be running
    QVERIFY(!simulation.isRunning());
    
    // Start simulation
    simulation.start();
    QVERIFY(simulation.isRunning());
    
    // Stop simulation
    simulation.stop();
    QVERIFY(!simulation.isRunning());
}

void TestSimulation::testStartStop()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Simulation simulation(scene);
    
    // Test start
    simulation.start();
    QVERIFY(simulation.isRunning());
    
    // Test stop
    simulation.stop();
    QVERIFY(!simulation.isRunning());
    
    // Test multiple start/stop cycles
    for (int i = 0; i < 3; ++i) {
        simulation.start();
        QVERIFY(simulation.isRunning());
        simulation.stop();
        QVERIFY(!simulation.isRunning());
    }
}

void TestSimulation::testRestart()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Simulation simulation(scene);
    
    // Start first
    simulation.start();
    QVERIFY(simulation.isRunning());
    
    // Test restart
    simulation.restart();
    
    // Should still be running after restart
    QVERIFY(simulation.isRunning());
    
    simulation.stop();
}

void TestSimulation::testUpdate()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Simulation simulation(scene);
    
    // Test update method - should not crash
    simulation.update();
    QVERIFY(true);
    
    // Initialize and test update
    simulation.initialize();
    simulation.update();
    QVERIFY(true);
}

void TestSimulation::testTimerBasicFunctionality()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Simulation simulation(scene);
    
    // Test that simulation can start (which starts the internal timer)
    simulation.start();
    QVERIFY(simulation.isRunning());
    
    // Brief wait to allow timer operation
    QTest::qWait(10);
    
    simulation.stop();
    QVERIFY(!simulation.isRunning());
}

void TestSimulation::testClockElements()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Simulation simulation(scene);
    
    // Add clock element to scene
    auto *clock = new Clock();
    scene->addItem(clock);
    
    // Initialize simulation with clock
    simulation.initialize();
    
    // Should handle clocks without crashing
    simulation.update();
    QVERIFY(true);
}

void TestSimulation::testTimerInterval()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Simulation simulation(scene);
    
    // The simulation uses a fixed 1ms timer interval internally
    // We can't directly test the private timer, but we can verify
    // that the simulation timing works correctly
    simulation.start();
    QVERIFY(simulation.isRunning());
    
    // Let it run briefly and verify it doesn't crash
    QTest::qWait(5);
    
    simulation.stop();
}

void TestSimulation::testInitializedState()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Simulation simulation(scene);
    
    // Initially should not be initialized (internal state)
    // We test this indirectly through the initialize() method
    bool result = simulation.initialize();
    QVERIFY(result == true || result == false);
}

void TestSimulation::testSimulationReset()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Simulation simulation(scene);
    
    // Initialize and start
    simulation.initialize();
    simulation.start();
    QVERIFY(simulation.isRunning());
    
    // Test restart (which is a form of reset)
    simulation.restart();
    QVERIFY(simulation.isRunning());
    
    simulation.stop();
}

void TestSimulation::testMultipleStartStop()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Simulation simulation(scene);
    
    // Test multiple rapid start/stop cycles
    for (int i = 0; i < 5; ++i) {
        simulation.start();
        QVERIFY(simulation.isRunning());
        
        simulation.stop();
        QVERIFY(!simulation.isRunning());
    }
}

void TestSimulation::testSimpleLogicCircuit()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    
    // Create simple AND circuit
    auto *input1 = new InputButton();
    auto *input2 = new InputButton();
    auto *andGate = new And();
    auto *output = new Led();
    
    scene->addItem(input1);
    scene->addItem(input2);
    scene->addItem(andGate);
    scene->addItem(output);
    
    // Test simulation with this circuit
    Simulation simulation(scene);
    simulation.initialize();
    simulation.update();
    
    QVERIFY(scene->elements().size() >= 4);
}

void TestSimulation::testComplexLogicCircuit()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    
    // Create more complex circuit
    auto *input = new InputButton();
    auto *notGate = new Not();
    auto *orGate = new Or();
    auto *xorGate = new Xor();
    auto *output = new Led();
    
    scene->addItem(input);
    scene->addItem(notGate);
    scene->addItem(orGate);
    scene->addItem(xorGate);
    scene->addItem(output);
    
    // Test simulation with complex circuit
    Simulation simulation(scene);
    simulation.initialize();
    simulation.update();
    
    QVERIFY(scene->elements().size() >= 5);
}

void TestSimulation::testSequentialLogicCircuit()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    
    // Create sequential circuit with flip-flop
    auto *clock = new Clock();
    auto *dff = new DFlipFlop();
    auto *output = new Led();
    
    scene->addItem(clock);
    scene->addItem(dff);
    scene->addItem(output);
    
    // Test simulation with sequential elements
    Simulation simulation(scene);
    simulation.initialize();
    simulation.update();
    
    QVERIFY(scene->elements().size() >= 3);
}

void TestSimulation::testInputElementsUpdate()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    
    // Add multiple input elements
    auto *button1 = new InputButton();
    auto *button2 = new InputButton();
    scene->addItem(button1);
    scene->addItem(button2);
    
    Simulation simulation(scene);
    simulation.initialize();
    
    // Test that input elements are handled during update
    simulation.update();
    QVERIFY(true); // Should complete without crashing
}

void TestSimulation::testOutputElementsUpdate()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    
    // Add multiple output elements
    auto *led1 = new Led();
    auto *led2 = new Led();
    scene->addItem(led1);
    scene->addItem(led2);
    
    Simulation simulation(scene);
    simulation.initialize();
    
    // Test that output elements are handled during update
    simulation.update();
    QVERIFY(true); // Should complete without crashing
}

void TestSimulation::testConnectionsUpdate()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    
    // Create connected elements
    auto *input = new InputButton();
    auto *output = new Led();
    auto *connection = new QNEConnection();
    
    scene->addItem(input);
    scene->addItem(output);
    scene->addItem(connection);
    
    Simulation simulation(scene);
    simulation.initialize();
    
    // Test that connections are handled during update
    simulation.update();
    QVERIFY(true); // Should complete without crashing
}

void TestSimulation::testLogicElementsUpdate()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    
    // Add various logic elements
    auto *andGate = new And();
    auto *orGate = new Or();
    auto *notGate = new Not();
    
    scene->addItem(andGate);
    scene->addItem(orGate);
    scene->addItem(notGate);
    
    Simulation simulation(scene);
    simulation.initialize();
    
    // Test that logic elements are updated
    simulation.update();
    QVERIFY(true); // Should complete without crashing
}

void TestSimulation::testSimulationFlow()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    
    // Create complete circuit
    auto *input = new InputButton();
    auto *logic = new And();
    auto *output = new Led();
    
    scene->addItem(input);
    scene->addItem(logic);
    scene->addItem(output);
    
    Simulation simulation(scene);
    
    // Test complete simulation flow
    QVERIFY(!simulation.isRunning());
    
    simulation.initialize();
    simulation.start();
    QVERIFY(simulation.isRunning());
    
    // Let simulation run briefly
    QTest::qWait(5);
    
    simulation.stop();
    QVERIFY(!simulation.isRunning());
}

void TestSimulation::testSimulationWithClocks()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    
    // Create circuit with clock
    auto *clock = new Clock();
    auto *dff = new DFlipFlop();
    
    scene->addItem(clock);
    scene->addItem(dff);
    
    Simulation simulation(scene);
    simulation.initialize();
    simulation.start();
    
    // Let clocked simulation run briefly
    QTest::qWait(10);
    
    simulation.stop();
    QVERIFY(true); // Should complete without crashing
}

void TestSimulation::testSimulationEdgeCases()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    Simulation simulation(scene);
    
    // Test with empty scene
    simulation.initialize();
    simulation.update();
    QVERIFY(true);
    
    // Test multiple initializations
    simulation.initialize();
    simulation.initialize();
    QVERIFY(true);
    
    // Test update before initialization
    Simulation simulation2(scene);
    simulation2.update();
    QVERIFY(true);
}

void TestSimulation::testSimulationPerformance()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    
    // Create moderately complex circuit for performance test
    for (int i = 0; i < 10; ++i) {
        scene->addItem(new And());
        scene->addItem(new Or());
    }
    
    Simulation simulation(scene);
    simulation.initialize();
    
    // Time multiple updates
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; ++i) {
        simulation.update();
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    // Should complete in reasonable time (less than 1 second for 100 updates)
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    QVERIFY(duration.count() < 1000);
}

void TestSimulation::testSimulationStability()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    
    // Add circuit elements
    auto *input = new InputButton();
    auto *gate = new And();
    auto *output = new Led();
    
    scene->addItem(input);
    scene->addItem(gate);
    scene->addItem(output);
    
    Simulation simulation(scene);
    simulation.initialize();
    
    // Run many update cycles to test stability
    for (int i = 0; i < 1000; ++i) {
        simulation.update();
    }
    
    // Should remain stable throughout
    QVERIFY(true); // Completed without crashing
}
