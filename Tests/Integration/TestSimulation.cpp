// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestSimulation.h"

#include "App/Core/Priorities.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/InputButton.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Nand.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/Nor.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/GraphicElements/SRLatch.h"
#include "App/Element/GraphicElements/Xor.h"
#include "App/Element/IC.h"
#include "App/Nodes/QNEConnection.h"
#include "App/Nodes/QNEPort.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "Tests/Common/TestUtils.h"

using TestUtils::getInputStatus;

void TestSimulation::testTopologicalSorting()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputButton button1, button2;
    And andItem;
    Led led;

    builder.add(&button1, &button2, &andItem, &led);
    builder.connect(&button1, 0, &andItem, 0);
    builder.connect(&button2, 0, &andItem, 1);
    builder.connect(&andItem, 0, &led, 0);

    const auto elements(Scene::sortByTopology(workspace.scene()->elements()));

    // Inputs should come first
    QVERIFY2((elements.at(0) == &button1) || (elements.at(1) == &button1),
        "Button1 should appear in first two positions of topologically sorted elements");
    QVERIFY2((elements.at(0) == &button2) || (elements.at(1) == &button2),
        "Button2 should appear in first two positions of topologically sorted elements");
    // AND gate depends on inputs
    QVERIFY2(elements.at(2) == &andItem,
        "AND gate should appear at position 2 (after inputs) in topologically sorted elements");
    // LED depends on AND gate
    QVERIFY2(elements.at(3) == &led,
        "LED should appear at position 3 (after AND gate) in topologically sorted elements");
}

void TestSimulation::testAndGatePropagation()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    And andGate;
    Led led;

    builder.add(&sw1, &sw2, &andGate, &led);
    builder.connect(&sw1, 0, &andGate, 0);
    builder.connect(&sw2, 0, &andGate, 1);
    builder.connect(&andGate, 0, &led, 0);

    auto *simulation = builder.initSimulation();

    // Test AND truth table through simulation
    sw1.setOn(false); sw2.setOn(false); simulation->update();
    QCOMPARE(getInputStatus(&led), false);

    sw1.setOn(false); sw2.setOn(true); simulation->update();
    QCOMPARE(getInputStatus(&led), false);

    sw1.setOn(true); sw2.setOn(false); simulation->update();
    QCOMPARE(getInputStatus(&led), false);

    sw1.setOn(true); sw2.setOn(true); simulation->update();
    QCOMPARE(getInputStatus(&led), true);
}

void TestSimulation::testOrGatePropagation()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    Or orGate;
    Led led;

    builder.add(&sw1, &sw2, &orGate, &led);
    builder.connect(&sw1, 0, &orGate, 0);
    builder.connect(&sw2, 0, &orGate, 1);
    builder.connect(&orGate, 0, &led, 0);

    auto *simulation = builder.initSimulation();

    sw1.setOn(false); sw2.setOn(false); simulation->update();
    QCOMPARE(getInputStatus(&led), false);

    sw1.setOn(false); sw2.setOn(true); simulation->update();
    QCOMPARE(getInputStatus(&led), true);

    sw1.setOn(true); sw2.setOn(false); simulation->update();
    QCOMPARE(getInputStatus(&led), true);

    sw1.setOn(true); sw2.setOn(true); simulation->update();
    QCOMPARE(getInputStatus(&led), true);
}

void TestSimulation::testNotGatePropagation()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    Not notGate;
    Led led;

    builder.add(&sw, &notGate, &led);
    builder.connect(&sw, 0, &notGate, 0);
    builder.connect(&notGate, 0, &led, 0);

    auto *simulation = builder.initSimulation();

    sw.setOn(false); simulation->update();
    QCOMPARE(getInputStatus(&led), true);

    sw.setOn(true); simulation->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testNandGatePropagation()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    Nand nandGate;
    Led led;

    builder.add(&sw1, &sw2, &nandGate, &led);
    builder.connect(&sw1, 0, &nandGate, 0);
    builder.connect(&sw2, 0, &nandGate, 1);
    builder.connect(&nandGate, 0, &led, 0);

    auto *simulation = builder.initSimulation();

    sw1.setOn(false); sw2.setOn(false); simulation->update();
    QCOMPARE(getInputStatus(&led), true);

    sw1.setOn(false); sw2.setOn(true); simulation->update();
    QCOMPARE(getInputStatus(&led), true);

    sw1.setOn(true); sw2.setOn(false); simulation->update();
    QCOMPARE(getInputStatus(&led), true);

    sw1.setOn(true); sw2.setOn(true); simulation->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testNorGatePropagation()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    Nor norGate;
    Led led;

    builder.add(&sw1, &sw2, &norGate, &led);
    builder.connect(&sw1, 0, &norGate, 0);
    builder.connect(&sw2, 0, &norGate, 1);
    builder.connect(&norGate, 0, &led, 0);

    auto *simulation = builder.initSimulation();

    sw1.setOn(false); sw2.setOn(false); simulation->update();
    QCOMPARE(getInputStatus(&led), true);

    sw1.setOn(false); sw2.setOn(true); simulation->update();
    QCOMPARE(getInputStatus(&led), false);

    sw1.setOn(true); sw2.setOn(false); simulation->update();
    QCOMPARE(getInputStatus(&led), false);

    sw1.setOn(true); sw2.setOn(true); simulation->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testXorGatePropagation()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    Xor xorGate;
    Led led;

    builder.add(&sw1, &sw2, &xorGate, &led);
    builder.connect(&sw1, 0, &xorGate, 0);
    builder.connect(&sw2, 0, &xorGate, 1);
    builder.connect(&xorGate, 0, &led, 0);

    auto *simulation = builder.initSimulation();

    sw1.setOn(false); sw2.setOn(false); simulation->update();
    QCOMPARE(getInputStatus(&led), false);

    sw1.setOn(false); sw2.setOn(true); simulation->update();
    QCOMPARE(getInputStatus(&led), true);

    sw1.setOn(true); sw2.setOn(false); simulation->update();
    QCOMPARE(getInputStatus(&led), true);

    sw1.setOn(true); sw2.setOn(true); simulation->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testCascadedGates()
{
    // Test: (A AND B) OR C
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swA, swB, swC;
    And andGate;
    Or orGate;
    Led led;

    builder.add(&swA, &swB, &swC, &andGate, &orGate, &led);
    builder.connect(&swA, 0, &andGate, 0);
    builder.connect(&swB, 0, &andGate, 1);
    builder.connect(&andGate, 0, &orGate, 0);
    builder.connect(&swC, 0, &orGate, 1);
    builder.connect(&orGate, 0, &led, 0);

    auto *simulation = builder.initSimulation();

    // (0 AND 0) OR 0 = 0
    swA.setOn(false); swB.setOn(false); swC.setOn(false); simulation->update();
    QCOMPARE(getInputStatus(&led), false);

    // (0 AND 0) OR 1 = 1
    swA.setOn(false); swB.setOn(false); swC.setOn(true); simulation->update();
    QCOMPARE(getInputStatus(&led), true);

    // (1 AND 1) OR 0 = 1
    swA.setOn(true); swB.setOn(true); swC.setOn(false); simulation->update();
    QCOMPARE(getInputStatus(&led), true);

    // (1 AND 0) OR 0 = 0
    swA.setOn(true); swB.setOn(false); swC.setOn(false); simulation->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testMultiCycleStability()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    And andGate;
    Led led;

    builder.add(&sw1, &sw2, &andGate, &led);
    builder.connect(&sw1, 0, &andGate, 0);
    builder.connect(&sw2, 0, &andGate, 1);
    builder.connect(&andGate, 0, &led, 0);

    auto *simulation = builder.initSimulation();

    // Set inputs and run multiple cycles
    sw1.setOn();
    sw2.setOn();
    simulation->update();
    QCOMPARE(getInputStatus(&led), true);

    // Change input and run multiple cycles
    sw1.setOff();
    simulation->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testSimulationInitialization()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    auto *simulation = builder.simulation();

    // Simulation should not be running initially
    QVERIFY2(!simulation->isRunning(), "Simulation should not be running initially");

    InputSwitch sw;
    Led led;

    builder.add(&sw, &led);
    builder.connect(&sw, 0, &led, 0);

    // Initialize should succeed
    QVERIFY2(simulation->initialize(), "Simulation initialization should succeed");

    // Start simulation
    simulation->start();
    QVERIFY2(simulation->isRunning(), "Simulation should be running after start");

    // Stop simulation
    simulation->stop();
    QVERIFY2(!simulation->isRunning(), "Simulation should not be running initially");

    // Restart resets initialization state
    simulation->restart();
    simulation->start();
    QVERIFY2(simulation->isRunning(), "Simulation should be running after start");

    simulation->stop();
}
void TestSimulation::testElementProcessingOrderConsistency()
{
    // Test that element processing order remains consistent across multiple runs
    // This verifies our fix for std::stable_sort in topological sorting
    // Uses stable element IDs for comparison, not pointer addresses

    constexpr int NUM_RUNS = 10;
    QVector<QVector<int>> runs;  // Store element IDs, which are stable per creation order

    for (int run = 0; run < NUM_RUNS; ++run) {
        WorkSpace workspace;
        auto *scene = workspace.scene();

        // Create a complex circuit with many elements at same position
        // to test stable sorting behavior
        InputButton button1, button2, button3;
        And and1, and2;
        Or or1;
        Not not1;
        Led led;

        // Position elements to potentially have same priorities
        button1.setPos(0, 0);
        button2.setPos(0, 10);
        button3.setPos(0, 20);
        and1.setPos(100, 5);
        and2.setPos(100, 15);
        or1.setPos(200, 10);
        not1.setPos(300, 10);
        led.setPos(400, 10);

        scene->addItem(&button1);
        scene->addItem(&button2);
        scene->addItem(&button3);
        scene->addItem(&and1);
        scene->addItem(&and2);
        scene->addItem(&or1);
        scene->addItem(&not1);
        scene->addItem(&led);

        // Connect elements to create a meaningful circuit
        QNEConnection conn1, conn2, conn3, conn4, conn5, conn6, conn7;
        scene->addItem(&conn1);
        scene->addItem(&conn2);
        scene->addItem(&conn3);
        scene->addItem(&conn4);
        scene->addItem(&conn5);
        scene->addItem(&conn6);
        scene->addItem(&conn7);

        conn1.setStartPort(button1.outputPort());
        conn1.setEndPort(and1.inputPort(0));
        conn2.setStartPort(button2.outputPort());
        conn2.setEndPort(and1.inputPort(1));
        conn3.setStartPort(button3.outputPort());
        conn3.setEndPort(and2.inputPort(0));
        conn4.setStartPort(and1.outputPort());
        conn4.setEndPort(or1.inputPort(0));
        conn5.setStartPort(and2.outputPort());
        conn5.setEndPort(or1.inputPort(1));
        conn6.setStartPort(or1.outputPort());
        conn6.setEndPort(not1.inputPort());
        conn7.setStartPort(not1.outputPort());
        conn7.setEndPort(led.inputPort());

        // Get element processing order and extract element IDs
        const auto elements = Scene::sortByTopology(scene->elements());
        QVector<int> ids;
        for (auto *elem : elements) {
            ids.append(elem->id());
        }
        runs.append(ids);
    }

    // Verify all runs produce identical element ordering
    // Compares element IDs, which are stable based on creation order
    for (int run = 1; run < NUM_RUNS; ++run) {
        QCOMPARE(runs[run].size(), runs[0].size());
        for (int i = 0; i < runs[0].size(); ++i) {
            QCOMPARE(runs[run][i], runs[0][i]);
        }
    }
}

void TestSimulation::testSceneInitializationDeterminism()
{
    // Test that scene element initialization produces consistent ordering
    // This verifies our fix for scene->elements() deterministic ordering

    constexpr int NUM_RUNS = 10;
    QVector<QVector<GraphicElement *>> elementOrderings;

    for (int run = 0; run < NUM_RUNS; ++run) {
        WorkSpace workspace;
        auto *scene = workspace.scene();

        // Add items in same order to test deterministic element ordering
        InputButton button1, button2;
        And andGate;
        Led led;
        QNEConnection conn1, conn2, conn3;

        // Set positions to ensure deterministic topological sorting
        button1.setPos(0, 0);
        button2.setPos(0, 50);
        andGate.setPos(100, 25);
        led.setPos(200, 25);

        // Add items to scene
        scene->addItem(&button1);
        scene->addItem(&andGate);
        scene->addItem(&button2);
        scene->addItem(&led);
        scene->addItem(&conn1);
        scene->addItem(&conn2);
        scene->addItem(&conn3);

        // Connect the circuit for topological ordering
        conn1.setStartPort(button1.outputPort());
        conn1.setEndPort(andGate.inputPort(0));
        conn2.setStartPort(button2.outputPort());
        conn2.setEndPort(andGate.inputPort(1));
        conn3.setStartPort(andGate.outputPort());
        conn3.setEndPort(led.inputPort());

        // Get elements using our fixed deterministic method
        auto elements = scene->elements();
        elementOrderings.append(elements);
    }

    // Verify consistent ordering across runs
    for (int run = 1; run < NUM_RUNS; ++run) {
        QCOMPARE(elementOrderings[run].size(), elementOrderings[0].size());

        for (int i = 0; i < elementOrderings[0].size(); ++i) {
            if (elementOrderings[run][i] != elementOrderings[0][i]) {
                QString msg = QString("Scene elements order mismatch at run %1, position %2")
                                  .arg(run).arg(i);
                QFAIL(qPrintable(msg));
            }
        }
    }
}

void TestSimulation::testSimulationOutputReproducibility()
{
    // Test that identical circuits produce identical simulation outputs
    // This is the ultimate test of determinism

    constexpr int NUM_RUNS = 5;
    constexpr int SIMULATION_STEPS = 20;
    QVector<QVector<bool>> simulationResults;

    for (int run = 0; run < NUM_RUNS; ++run) {
        WorkSpace workspace;
        auto *scene = workspace.scene();
        Simulation simulation(scene);

        // Create deterministic test circuit: AND gate with two inputs
        InputButton button1, button2;
        And andGate;
        Led led;

        button1.setPos(0, 0);
        button2.setPos(0, 50);
        andGate.setPos(100, 25);
        led.setPos(200, 25);

        scene->addItem(&button1);
        scene->addItem(&button2);
        scene->addItem(&andGate);
        scene->addItem(&led);

        // Connect circuit
        QNEConnection conn1, conn2, conn3;
        scene->addItem(&conn1);
        scene->addItem(&conn2);
        scene->addItem(&conn3);

        conn1.setStartPort(button1.outputPort());
        conn1.setEndPort(andGate.inputPort(0));
        conn2.setStartPort(button2.outputPort());
        conn2.setEndPort(andGate.inputPort(1));
        conn3.setStartPort(andGate.outputPort());
        conn3.setEndPort(led.inputPort());

        simulation.initialize();

        // Run simulation with deterministic input pattern
        QVector<bool> outputs;

        // Test pattern: 00, 01, 10, 11 repeatedly
        for (int step = 0; step < SIMULATION_STEPS; ++step) {
            bool input1 = (step / 2) % 2;
            bool input2 = step % 2;

            button1.setOn(input1);
            button2.setOn(input2);

            simulation.update();

            // Record LED output (check if input port is active)
            bool ledOutput = (led.inputPort()->status() == Status::Active);
            outputs.append(ledOutput);

            // Verify AND gate logic
            bool expectedOutput = input1 && input2;
            QCOMPARE(ledOutput, expectedOutput);
        }

        simulationResults.append(outputs);
    }

    // Verify all runs produce identical outputs
    for (int run = 1; run < NUM_RUNS; ++run) {
        QCOMPARE(simulationResults[run].size(), simulationResults[0].size());

        for (int step = 0; step < SIMULATION_STEPS; ++step) {
            if (simulationResults[run][step] != simulationResults[0][step]) {
                QString msg = QString("Output mismatch at run %1, step %2: "
                                     "expected %3, got %4")
                                  .arg(run).arg(step)
                                  .arg(simulationResults[0][step])
                                  .arg(simulationResults[run][step]);
                QFAIL(qPrintable(msg));
            }
        }
    }
}

void TestSimulation::testSimulationGraphStability()
{
    // Test simulation graph for consistent behavior across multiple initializations
    // This verifies our QHash -> QMap fix for deterministic iteration

    constexpr int NUM_RUNS = 10;
    QVector<QVector<QPair<int, int>>> mappingResults;

    for (int run = 0; run < NUM_RUNS; ++run) {
        WorkSpace workspace;
        auto *scene = workspace.scene();

        // Create circuit with complex dependencies
        InputButton input1, input2, input3;
        And and1, and2;
        Or or1;
        Not not1, not2;
        Led led1, led2;

        // Position elements to create complex priority relationships
        input1.setPos(0, 0);
        input2.setPos(0, 50);
        input3.setPos(0, 100);
        and1.setPos(100, 25);
        and2.setPos(100, 75);
        or1.setPos(200, 25);
        not1.setPos(200, 75);
        not2.setPos(300, 50);
        led1.setPos(400, 25);
        led2.setPos(400, 75);

        scene->addItem(&input1);
        scene->addItem(&input2);
        scene->addItem(&input3);
        scene->addItem(&and1);
        scene->addItem(&and2);
        scene->addItem(&or1);
        scene->addItem(&not1);
        scene->addItem(&not2);
        scene->addItem(&led1);
        scene->addItem(&led2);

        // Create connections to form complex dependency graph
        QVector<QNEConnection *> connections;
        for (int i = 0; i < 10; ++i) {
            connections.append(new QNEConnection);
            scene->addItem(connections[i]);
        }

        connections[0]->setStartPort(input1.outputPort());
        connections[0]->setEndPort(and1.inputPort(0));
        connections[1]->setStartPort(input2.outputPort());
        connections[1]->setEndPort(and1.inputPort(1));
        connections[2]->setStartPort(input2.outputPort());
        connections[2]->setEndPort(and2.inputPort(0));
        connections[3]->setStartPort(input3.outputPort());
        connections[3]->setEndPort(and2.inputPort(1));
        connections[4]->setStartPort(and1.outputPort());
        connections[4]->setEndPort(or1.inputPort(0));
        connections[5]->setStartPort(and2.outputPort());
        connections[5]->setEndPort(not1.inputPort());
        connections[6]->setStartPort(or1.outputPort());
        connections[6]->setEndPort(not2.inputPort());
        connections[7]->setStartPort(not1.outputPort());
        connections[7]->setEndPort(or1.inputPort(1));
        connections[8]->setStartPort(not2.outputPort());
        connections[8]->setEndPort(led1.inputPort());
        connections[9]->setStartPort(not1.outputPort());
        connections[9]->setEndPort(led2.inputPort());

        // Build simulation graph and verify consistent ordering
        auto elements = scene->elements();
        for (auto *elm : elements) {
            elm->initSimulationVectors(elm->inputSize(), elm->outputSize());
        }
        Simulation::buildConnectionGraph(elements);

        QHash<GraphicElement *, QVector<GraphicElement *>> successors;
        for (auto *elm : elements) {
            for (auto *outputPort : elm->outputs()) {
                for (auto *conn : outputPort->connections()) {
                    if (auto *endPort = conn->endPort()) {
                        auto *successor = endPort->graphicElement();
                        if (successor && !successors[elm].contains(successor)) {
                            successors[elm].append(successor);
                        }
                    }
                }
            }
        }
        QHash<GraphicElement *, int> priorities;
        calculatePriorities(elements, successors, priorities);

        QVector<QPair<int, int>> elementProperties;
        for (auto *elm : elements) {
            int priority = priorities.value(elm, -1);
            int outputSize = elm->outputSize();
            elementProperties.append(QPair<int, int>(priority, outputSize));
        }

        mappingResults.append(elementProperties);

        // Clean up connections
        qDeleteAll(connections);
    }

    // Verify consistent ordering across all runs
    for (int run = 1; run < NUM_RUNS; ++run) {
        QCOMPARE(mappingResults[run].size(), mappingResults[0].size());

        for (int i = 0; i < mappingResults[0].size(); ++i) {
            if (mappingResults[run][i] != mappingResults[0][i]) {
                QString msg = QString("Simulation graph mismatch at run %1, position %2: expected (%3,%4), got (%5,%6)")
                                  .arg(run).arg(i)
                                  .arg(mappingResults[0][i].first).arg(mappingResults[0][i].second)
                                  .arg(mappingResults[run][i].first).arg(mappingResults[run][i].second);
                QFAIL(qPrintable(msg));
            }
        }
    }
}

void TestSimulation::testCircuitWithFeedbackLoops()
{
    // Test circuits with feedback loops for deterministic behavior
    // This exercises our feedback loop detection and priority handling fixes
    // Uses a proper NAND-based SR latch circuit

    constexpr int NUM_RUNS = 5;
    QVector<QVector<bool>> feedbackResults;

    for (int run = 0; run < NUM_RUNS; ++run) {
        WorkSpace workspace;
        auto *scene = workspace.scene();
        Simulation simulation(scene);

        // Create NAND-based SR latch circuit (classic feedback example)
        InputButton setBtn, resetBtn;
        Nand nand1, nand2;
        Led qLed, notQLed;

        setBtn.setPos(0, 0);
        resetBtn.setPos(0, 100);
        nand1.setPos(150, 30);
        nand2.setPos(150, 70);
        qLed.setPos(300, 30);
        notQLed.setPos(300, 70);

        scene->addItem(&setBtn);
        scene->addItem(&resetBtn);
        scene->addItem(&nand1);
        scene->addItem(&nand2);
        scene->addItem(&qLed);
        scene->addItem(&notQLed);

        // Create feedback connections for proper NAND SR latch
        QNEConnection conn1, conn2, conn3, conn4, conn5, conn6;
        scene->addItem(&conn1);
        scene->addItem(&conn2);
        scene->addItem(&conn3);
        scene->addItem(&conn4);
        scene->addItem(&conn5);
        scene->addItem(&conn6);

        // NAND1 inputs: SET and feedback from NAND2
        conn1.setStartPort(setBtn.outputPort());
        conn1.setEndPort(nand1.inputPort(0));
        conn3.setStartPort(nand2.outputPort());
        conn3.setEndPort(nand1.inputPort(1));

        // NAND2 inputs: RESET and feedback from NAND1
        conn2.setStartPort(resetBtn.outputPort());
        conn2.setEndPort(nand2.inputPort(0));
        conn4.setStartPort(nand1.outputPort());
        conn4.setEndPort(nand2.inputPort(1));

        // Outputs (Q and ~Q)
        conn5.setStartPort(nand1.outputPort());
        conn5.setEndPort(qLed.inputPort());
        conn6.setStartPort(nand2.outputPort());
        conn6.setEndPort(notQLed.inputPort());

        simulation.initialize();

        // Test deterministic feedback behavior
        QVector<bool> outputs;

        // Test sequence for NAND SR latch (active-low inputs)
        // Initial state: both inputs high (inactive)
        setBtn.setOn(true);
        resetBtn.setOn(true);
        simulation.update();

        // Reset: pull reset low (active), keep set high
        setBtn.setOn(true);
        resetBtn.setOn(false);  // Active reset
        simulation.update();
        outputs.append(qLed.inputPort()->status() == Status::Active);  // Should be false after reset

        // Set: pull set low (active), release reset
        setBtn.setOn(false);  // Active set
        resetBtn.setOn(true);
        simulation.update();
        outputs.append(qLed.inputPort()->status() == Status::Active);  // Should be true after set

        // Hold: both inputs high (inactive) - latch should hold state
        setBtn.setOn(true);
        resetBtn.setOn(true);
        simulation.update();
        outputs.append(qLed.inputPort()->status() == Status::Active);  // Should still be true

        // Reset again: pull reset low
        setBtn.setOn(true);
        resetBtn.setOn(false);  // Active reset
        simulation.update();
        outputs.append(qLed.inputPort()->status() == Status::Active);  // Should be false again

        feedbackResults.append(outputs);
    }

    // Verify consistent behavior across all runs
    for (int run = 1; run < NUM_RUNS; ++run) {
        QCOMPARE(feedbackResults[run].size(), feedbackResults[0].size());

        for (int step = 0; step < feedbackResults[0].size(); ++step) {
            if (feedbackResults[run][step] != feedbackResults[0][step]) {
                QString msg = QString("Feedback circuit mismatch at run %1, step %2: "
                                     "expected %3, got %4")
                                  .arg(run).arg(step)
                                  .arg(feedbackResults[0][step])
                                  .arg(feedbackResults[run][step]);
                QFAIL(qPrintable(msg));
            }
        }
    }

    // Verify expected NAND SR latch behavior in first run
    QVERIFY2(!feedbackResults[0][0], "SR Latch should be reset at state 0");
    QVERIFY2(feedbackResults[0][1], "SR Latch should be set at state 1");
    QVERIFY2(feedbackResults[0][2], "SR Latch should hold at state 2");
    QVERIFY2(!feedbackResults[0][3], "SR Latch should be reset again at state 3");
}

// ============================================================================
// Wireless signal propagation
// ============================================================================

void TestSimulation::testWirelessTxRxPropagation()
{
    // A Tx/Rx pair on the same channel must propagate the signal wirelessly.
    // Circuit: InputSwitch → Tx("SIG") ... Rx("SIG") → LED
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    auto *tx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    Led led;

    tx->setLabel("SIG");
    tx->setWirelessMode(WirelessMode::Tx);
    rx->setLabel("SIG");
    rx->setWirelessMode(WirelessMode::Rx);

    builder.add(&sw, tx, rx, &led);
    builder.connect(&sw, 0, tx, 0);   // physical wire: switch → Tx input
    builder.connect(rx, 0, &led, 0);  // physical wire: Rx output → LED

    auto *sim = builder.initSimulation();

    sw.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led), false);

    sw.setOn(true); sim->update();
    QCOMPARE(getInputStatus(&led), true);

    sw.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testWirelessMultiRxFanOut()
{
    // 1 Tx broadcasting to 2 Rx nodes on the same channel.
    // Circuit: InputSwitch → Tx("BUS") ... Rx1("BUS") → LED1
    //                                  ... Rx2("BUS") → LED2
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    auto *tx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rx1 = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rx2 = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    Led led1, led2;

    tx->setLabel("BUS");
    tx->setWirelessMode(WirelessMode::Tx);
    rx1->setLabel("BUS");
    rx1->setWirelessMode(WirelessMode::Rx);
    rx2->setLabel("BUS");
    rx2->setWirelessMode(WirelessMode::Rx);

    builder.add(&sw, tx, rx1, rx2, &led1, &led2);
    builder.connect(&sw, 0, tx, 0);
    builder.connect(rx1, 0, &led1, 0);
    builder.connect(rx2, 0, &led2, 0);

    auto *sim = builder.initSimulation();

    sw.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led1), false);
    QCOMPARE(getInputStatus(&led2), false);

    sw.setOn(true); sim->update();
    QCOMPARE(getInputStatus(&led1), true);
    QCOMPARE(getInputStatus(&led2), true);

    sw.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led1), false);
    QCOMPARE(getInputStatus(&led2), false);
}

void TestSimulation::testWirelessOrphanedRx()
{
    // An Rx node with no matching Tx should output its default (Inactive) status.
    // Circuit: Rx("ORPHAN") → LED  (no Tx with label "ORPHAN" exists)
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *rx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    Led led;

    rx->setLabel("ORPHAN");
    rx->setWirelessMode(WirelessMode::Rx);

    builder.add(rx, &led);
    builder.connect(rx, 0, &led, 0);

    auto *sim = builder.initSimulation();

    sim->update();
    QCOMPARE(getInputStatus(&led), false);

    // Multiple updates must remain stable.
    sim->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testWirelessInsideIC()
{
    // Wireless Tx/Rx nodes inside an IC sub-circuit must propagate signals.
    // The IC file (wireless_passthrough.panda) contains:
    //   InputSwitch → Tx("W") ... Rx("W") → LED
    // We load it as a file-based IC and verify the truth table through the IC ports.
    const QString icFile = TestUtils::cpuComponentsDir() + "wireless_passthrough.panda";
    QVERIFY2(QFile::exists(icFile), qPrintable("IC file not found: " + icFile));

    WorkSpace ws;
    auto *ic = new IC();

    CircuitBuilder builder(ws.scene());
    InputSwitch outerSw;
    Led outerLed;

    builder.add(&outerSw, ic, &outerLed);

    try {
        ic->loadFile(icFile, QFileInfo(icFile).absolutePath());
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Failed to load IC file: %1").arg(e.what())));
    }

    QVERIFY2(ic->inputSize() > 0, qPrintable(QString("IC inputs=%1 outputs=%2")
        .arg(ic->inputSize()).arg(ic->outputSize())));
    QVERIFY2(ic->outputSize() > 0, "IC must expose at least one output port");

    // Verify the IC internals contain wireless nodes
    int txCount = 0, rxCount = 0;
    for (auto *elm : ic->internalElements()) {
        if (elm->wirelessMode() == WirelessMode::Tx) ++txCount;
        if (elm->wirelessMode() == WirelessMode::Rx) ++rxCount;
    }
    QCOMPARE(txCount, 1);
    QCOMPARE(rxCount, 1);

    builder.connect(&outerSw, 0, ic, 0);
    builder.connect(ic, 0, &outerLed, 0);

    auto *sim = builder.initSimulation();

    outerSw.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&outerLed), false);

    outerSw.setOn(true); sim->update();
    QCOMPARE(getInputStatus(&outerLed), true);

    outerSw.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&outerLed), false);
}

// ============================================================================
// Wireless edge cases
// ============================================================================

void TestSimulation::testWirelessDuplicateTxIgnored()
{
    // Two Tx nodes with the same label — only one is registered, the other is
    // silently ignored.  The test verifies: no crash, exactly one Tx drives the
    // Rx, and the result is deterministic across multiple updates.
    // Circuit: sw1 → Tx1("DUP")  sw2 → Tx2("DUP")  Rx("DUP") → LED
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    auto *tx1 = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *tx2 = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rx  = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    Led led;

    tx1->setLabel("DUP");
    tx1->setWirelessMode(WirelessMode::Tx);
    tx2->setLabel("DUP");
    tx2->setWirelessMode(WirelessMode::Tx);
    rx->setLabel("DUP");
    rx->setWirelessMode(WirelessMode::Rx);

    builder.add(&sw1, &sw2, tx1, tx2, rx, &led);
    builder.connect(&sw1, 0, tx1, 0);
    builder.connect(&sw2, 0, tx2, 0);
    builder.connect(rx, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // Both ON → LED must be ON regardless of which Tx won.
    sw1.setOn(true); sw2.setOn(true); sim->update();
    QCOMPARE(getInputStatus(&led), true);

    // Both OFF → LED must be OFF.
    sw1.setOn(false); sw2.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led), false);

    // Exactly one Tx controls the Rx. Determine which one won and verify
    // the other is indeed ignored.
    sw1.setOn(true); sw2.setOn(false); sim->update();
    const bool sw1Controls = getInputStatus(&led);

    sw1.setOn(false); sw2.setOn(true); sim->update();
    const bool sw2Controls = getInputStatus(&led);

    // Exactly one switch must control the LED, not both.
    QVERIFY2(sw1Controls != sw2Controls,
             "Exactly one Tx must win — both or neither controlling the LED indicates a bug");

    // Verify determinism: repeat and check the same result.
    sw1.setOn(true); sw2.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led), sw1Controls);
}

void TestSimulation::testWirelessEmptyLabelIgnored()
{
    // Tx and Rx nodes with empty labels must not connect to each other.
    // Circuit: sw → Tx("")  Rx("") → LED
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    auto *tx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    Led led;

    tx->setLabel("");
    tx->setWirelessMode(WirelessMode::Tx);
    rx->setLabel("");
    rx->setWirelessMode(WirelessMode::Rx);

    builder.add(&sw, tx, rx, &led);
    builder.connect(&sw, 0, tx, 0);
    builder.connect(rx, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // Empty labels are skipped — Rx has no Tx match, LED stays inactive.
    sw.setOn(true); sim->update();
    QCOMPARE(getInputStatus(&led), false);

    sw.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testWirelessLabelMismatchIsolation()
{
    // Tx("A") must NOT drive Rx("B") — different labels are different channels.
    // Circuit: sw → Tx("A")  Rx("B") → LED
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    auto *tx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    Led led;

    tx->setLabel("A");
    tx->setWirelessMode(WirelessMode::Tx);
    rx->setLabel("B");
    rx->setWirelessMode(WirelessMode::Rx);

    builder.add(&sw, tx, rx, &led);
    builder.connect(&sw, 0, tx, 0);
    builder.connect(rx, 0, &led, 0);

    auto *sim = builder.initSimulation();

    sw.setOn(true); sim->update();
    QCOMPARE(getInputStatus(&led), false);

    sw.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testWirelessMultipleIndependentChannels()
{
    // Two independent wireless channels must not cross-talk.
    // Circuit: sw1 → Tx("CH_A") ... Rx("CH_A") → led1
    //          sw2 → Tx("CH_B") ... Rx("CH_B") → led2
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    auto *txA = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rxA = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *txB = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rxB = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    Led led1, led2;

    txA->setLabel("CH_A"); txA->setWirelessMode(WirelessMode::Tx);
    rxA->setLabel("CH_A"); rxA->setWirelessMode(WirelessMode::Rx);
    txB->setLabel("CH_B"); txB->setWirelessMode(WirelessMode::Tx);
    rxB->setLabel("CH_B"); rxB->setWirelessMode(WirelessMode::Rx);

    builder.add(&sw1, &sw2, txA, rxA, txB, rxB, &led1, &led2);
    builder.connect(&sw1, 0, txA, 0);
    builder.connect(rxA, 0, &led1, 0);
    builder.connect(&sw2, 0, txB, 0);
    builder.connect(rxB, 0, &led2, 0);

    auto *sim = builder.initSimulation();

    // Both off
    sw1.setOn(false); sw2.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led1), false);
    QCOMPARE(getInputStatus(&led2), false);

    // CH_A on, CH_B off — only led1 should light up
    sw1.setOn(true); sw2.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led1), true);
    QCOMPARE(getInputStatus(&led2), false);

    // CH_A off, CH_B on — only led2 should light up
    sw1.setOn(false); sw2.setOn(true); sim->update();
    QCOMPARE(getInputStatus(&led1), false);
    QCOMPARE(getInputStatus(&led2), true);

    // Both on
    sw1.setOn(true); sw2.setOn(true); sim->update();
    QCOMPARE(getInputStatus(&led1), true);
    QCOMPARE(getInputStatus(&led2), true);
}

void TestSimulation::testWirelessTxUnconnectedInput()
{
    // A Tx node with no physical wire on its input port propagates Unknown.
    // The Rx must stay inactive (Unknown → false).
    // Circuit: Tx("OPEN") (unconnected input) ... Rx("OPEN") → LED
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    auto *tx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    Led led;

    tx->setLabel("OPEN");
    tx->setWirelessMode(WirelessMode::Tx);
    rx->setLabel("OPEN");
    rx->setWirelessMode(WirelessMode::Rx);

    builder.add(tx, rx, &led);
    // No wire driving Tx input — only connect Rx output → LED
    builder.connect(rx, 0, &led, 0);

    auto *sim = builder.initSimulation();

    sim->update();
    QCOMPARE(getInputStatus(&led), false);

    // Must remain stable across updates.
    sim->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testWirelessLabelCaseSensitive()
{
    // Labels are case-sensitive: Tx("CLK") must NOT drive Rx("clk").
    // Circuit: sw → Tx("CLK") ... Rx("clk") → LED
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    auto *tx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    Led led;

    tx->setLabel("CLK");
    tx->setWirelessMode(WirelessMode::Tx);
    rx->setLabel("clk");
    rx->setWirelessMode(WirelessMode::Rx);

    builder.add(&sw, tx, rx, &led);
    builder.connect(&sw, 0, tx, 0);
    builder.connect(rx, 0, &led, 0);

    auto *sim = builder.initSimulation();

    sw.setOn(true); sim->update();
    QCOMPARE(getInputStatus(&led), false);

    sw.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testWirelessNoneModeNodeDoesNotInterfere()
{
    // A plain Node (None mode) with label "SIG" must not participate in wireless
    // routing, even when Tx("SIG") and Rx("SIG") exist alongside it.
    // Circuit: sw → Tx("SIG") ... Rx("SIG") → LED
    //          plainNode("SIG") with None mode — should be invisible to wireless.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    auto *tx   = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rx   = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *none = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    Led led;

    tx->setLabel("SIG");   tx->setWirelessMode(WirelessMode::Tx);
    rx->setLabel("SIG");   rx->setWirelessMode(WirelessMode::Rx);
    none->setLabel("SIG"); // None mode — must not interfere

    builder.add(&sw, tx, none, rx, &led);
    builder.connect(&sw, 0, tx, 0);
    builder.connect(rx, 0, &led, 0);
    // none is added but not wired to anything relevant

    auto *sim = builder.initSimulation();

    sw.setOn(true); sim->update();
    QCOMPARE(getInputStatus(&led), true);

    sw.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testWirelessLabelSetAfterMode()
{
    // Setting the label AFTER the wireless mode must still create a valid channel.
    // Circuit: sw → Tx("LATE") ... Rx("LATE") → LED
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    auto *tx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    Led led;

    // Set mode first, label second
    tx->setWirelessMode(WirelessMode::Tx);
    tx->setLabel("LATE");
    rx->setWirelessMode(WirelessMode::Rx);
    rx->setLabel("LATE");

    builder.add(&sw, tx, rx, &led);
    builder.connect(&sw, 0, tx, 0);
    builder.connect(rx, 0, &led, 0);

    auto *sim = builder.initSimulation();

    sw.setOn(true); sim->update();
    QCOMPARE(getInputStatus(&led), true);

    sw.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testWirelessCascadeThroughWire()
{
    // Multi-hop: wireless → physical wire → wireless.
    // Circuit: sw → Tx("A") ... Rx("A") → wire → Tx("B") ... Rx("B") → LED
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    auto *txA = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rxA = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *txB = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rxB = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    Led led;

    txA->setLabel("A"); txA->setWirelessMode(WirelessMode::Tx);
    rxA->setLabel("A"); rxA->setWirelessMode(WirelessMode::Rx);
    txB->setLabel("B"); txB->setWirelessMode(WirelessMode::Tx);
    rxB->setLabel("B"); rxB->setWirelessMode(WirelessMode::Rx);

    builder.add(&sw, txA, rxA, txB, rxB, &led);
    builder.connect(&sw, 0, txA, 0);     // physical: sw → TxA
    builder.connect(rxA, 0, txB, 0);     // physical: RxA → TxB (bridge between channels)
    builder.connect(rxB, 0, &led, 0);    // physical: RxB → LED

    auto *sim = builder.initSimulation();

    sw.setOn(true); sim->update();
    QCOMPARE(getInputStatus(&led), true);

    sw.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testWirelessRxFeedsCombinationalLogic()
{
    // Wireless signal feeds into an AND gate alongside a physical switch.
    // Circuit: sw1 → Tx("X") ... Rx("X") → AND ← sw2
    //                                       AND → LED
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    auto *tx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    And andGate;
    Led led;

    tx->setLabel("X"); tx->setWirelessMode(WirelessMode::Tx);
    rx->setLabel("X"); rx->setWirelessMode(WirelessMode::Rx);

    builder.add(&sw1, &sw2, tx, rx, &andGate, &led);
    builder.connect(&sw1, 0, tx, 0);       // sw1 → Tx (wireless source)
    builder.connect(rx, 0, &andGate, 0);   // Rx → AND input 0
    builder.connect(&sw2, 0, &andGate, 1); // sw2 → AND input 1
    builder.connect(&andGate, 0, &led, 0); // AND → LED

    auto *sim = builder.initSimulation();

    // 0 AND 0 = 0
    sw1.setOn(false); sw2.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led), false);

    // 1 AND 0 = 0
    sw1.setOn(true); sw2.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led), false);

    // 0 AND 1 = 0
    sw1.setOn(false); sw2.setOn(true); sim->update();
    QCOMPARE(getInputStatus(&led), false);

    // 1 AND 1 = 1
    sw1.setOn(true); sw2.setOn(true); sim->update();
    QCOMPARE(getInputStatus(&led), true);
}

void TestSimulation::testWirelessOrphanedTxDoesNotCrash()
{
    // A Tx node with no matching Rx must not crash and must not interfere with
    // other circuits sharing the scene.
    // Circuit: sw1 → Tx("DEAD_END")   (no Rx for "DEAD_END")
    //          sw2 → AND ← sw3, AND → LED   (independent physical circuit)
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2, sw3;
    auto *tx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    And andGate;
    Led led;

    tx->setLabel("DEAD_END");
    tx->setWirelessMode(WirelessMode::Tx);

    builder.add(&sw1, &sw2, &sw3, tx, &andGate, &led);
    builder.connect(&sw1, 0, tx, 0);         // sw1 → Tx (signal goes nowhere)
    builder.connect(&sw2, 0, &andGate, 0);
    builder.connect(&sw3, 0, &andGate, 1);
    builder.connect(&andGate, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // Independent circuit must still work.
    sw2.setOn(true); sw3.setOn(true); sim->update();
    QCOMPARE(getInputStatus(&led), true);

    sw2.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testWirelessOrphanedRxWithPhysicalWire()
{
    // An Rx node with a physical input wire (bypassing ConnectionManager via
    // programmatic CircuitBuilder) but no matching Tx: the physical wire should
    // work because connectWirelessElements() doesn't overwrite when no Tx matches.
    // Circuit: sw → Rx("NO_TX") → LED   (physical wire only, no wireless Tx)
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    auto *rx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    Led led;

    rx->setLabel("NO_TX");
    rx->setWirelessMode(WirelessMode::Rx);

    builder.add(&sw, rx, &led);
    builder.connect(&sw, 0, rx, 0);   // physical wire to Rx input (bypasses ConnectionManager)
    builder.connect(rx, 0, &led, 0);

    auto *sim = builder.initSimulation();

    sw.setOn(true); sim->update();
    QCOMPARE(getInputStatus(&led), true);

    sw.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testWirelessOverridesPhysicalWire()
{
    // When Rx has both a physical wire AND a matching Tx, connectPredecessor()
    // overwrites the physical wire with the wireless source.  The wireless
    // signal must take precedence.
    // Circuit: swPhys → Rx("CH") → LED     (physical wire on Rx input)
    //          swWireless → Tx("CH")         (wireless source)
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch swPhys, swWireless;
    auto *tx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    Led led;

    tx->setLabel("CH"); tx->setWirelessMode(WirelessMode::Tx);
    rx->setLabel("CH"); rx->setWirelessMode(WirelessMode::Rx);

    builder.add(&swPhys, &swWireless, tx, rx, &led);
    builder.connect(&swPhys, 0, rx, 0);      // physical wire to Rx (bypasses ConnectionManager)
    builder.connect(&swWireless, 0, tx, 0);  // wireless source
    builder.connect(rx, 0, &led, 0);

    auto *sim = builder.initSimulation();

    // Wireless ON, physical OFF → LED should be ON (wireless takes precedence).
    swWireless.setOn(true); swPhys.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led), true);

    // Wireless OFF, physical ON → LED should be OFF (wireless takes precedence).
    swWireless.setOn(false); swPhys.setOn(true); sim->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testWirelessFeedbackLoop()
{
    // A wireless feedback loop: Rx("FB") → NOT → Tx("FB").
    // The successor graph has Tx→Rx (wireless) and Rx→NOT→Tx (physical),
    // forming a cycle.  findFeedbackNodes() must detect it and the simulation
    // must converge via iterative settling (NOT inverts, so the loop oscillates
    // but settling stops after maxIterations).
    // Circuit: sw → AND → Tx("FB") ... Rx("FB") → NOT → AND(input1)
    //                                               NOT → LED
    // The AND gate breaks the direct feedback: sw=OFF clamps it to 0.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    And andGate;
    auto *tx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    Not notGate;
    Led led;

    tx->setLabel("FB"); tx->setWirelessMode(WirelessMode::Tx);
    rx->setLabel("FB"); rx->setWirelessMode(WirelessMode::Rx);

    builder.add(&sw, &andGate, tx, rx, &notGate, &led);
    builder.connect(&sw, 0, &andGate, 0);       // sw → AND input 0
    builder.connect(&notGate, 0, &andGate, 1);  // NOT → AND input 1 (feedback)
    builder.connect(&andGate, 0, tx, 0);         // AND → Tx
    builder.connect(rx, 0, &notGate, 0);         // Rx → NOT
    builder.connect(&notGate, 0, &led, 0);       // NOT → LED

    auto *sim = builder.initSimulation();

    // sw=OFF: AND output is 0 regardless of feedback → Tx broadcasts 0 →
    // Rx receives 0 → NOT outputs 1 → LED=ON, AND(0,1)=0 → stable.
    sw.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led), true);

    // Must remain stable across updates.
    sim->update();
    QCOMPARE(getInputStatus(&led), true);
}

void TestSimulation::testWirelessUnicodeLabels()
{
    // Unicode labels must work as wireless channel names — QHash<QString>
    // performs exact comparison, so non-ASCII labels should match correctly.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw;
    auto *tx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    Led led;

    tx->setLabel(QStringLiteral("信号")); tx->setWirelessMode(WirelessMode::Tx);
    rx->setLabel(QStringLiteral("信号")); rx->setWirelessMode(WirelessMode::Rx);

    builder.add(&sw, tx, rx, &led);
    builder.connect(&sw, 0, tx, 0);
    builder.connect(rx, 0, &led, 0);

    auto *sim = builder.initSimulation();

    sw.setOn(true); sim->update();
    QCOMPARE(getInputStatus(&led), true);

    sw.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led), false);
}

// ============================================================================
// Negative Tests — Graceful handling of degenerate circuit conditions
// ============================================================================

void TestSimulation::testUnconnectedRequiredInputGraceful()
{
    // An AND gate with no inputs connected is invalid in the logic layer.
    // The simulation must not crash, and the downstream LED must stay inactive
    // (the unknown element and its successors receive Status::Unknown).
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    And andGate;
    Led led;

    builder.add(&andGate, &led);
    builder.connect(&andGate, 0, &led, 0);  // AND output → LED (no AND inputs connected)

    auto *sim = builder.initSimulation();
    sim->update();

    // AND gate logic is unknown (required inputs unconnected); cascade propagates Unknown to LED.
    // LED input port receives Status::Unknown → getInputStatus returns false.
    QCOMPARE(getInputStatus(&led), false);

    // A second update must also be stable (no crash, no state change).
    sim->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testPartiallyConnectedCircuitIsolation()
{
    // A fully-connected sub-circuit must work correctly even when an isolated
    // element (no connections) is present in the same scene.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sw1, sw2;
    And andWorking;
    Led led;
    And andIsolated;  // no connections — will be unknown

    builder.add(&sw1, &sw2, &andWorking, &led, &andIsolated);
    builder.connect(&sw1, 0, &andWorking, 0);
    builder.connect(&sw2, 0, &andWorking, 1);
    builder.connect(&andWorking, 0, &led, 0);
    // andIsolated is intentionally left unconnected

    auto *sim = builder.initSimulation();

    // Working circuit: 0 AND 0 = 0
    sw1.setOn(false); sw2.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led), false);

    // Working circuit: 1 AND 1 = 1 — isolated AND gate must not interfere
    sw1.setOn(true); sw2.setOn(true); sim->update();
    QCOMPARE(getInputStatus(&led), true);

    // Working circuit: 1 AND 0 = 0
    sw2.setOn(false); sim->update();
    QCOMPARE(getInputStatus(&led), false);
}

void TestSimulation::testDanglingConnectionGraceful()
{
    // A QNEConnection with an end port but no start port is "dangling".
    // buildConnectionGraph() skips it (startPort() == nullptr), so the AND gate's input
    // remains unconnected → unknown logic → simulation must not crash.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    And andGate;
    Led led;

    builder.add(&andGate, &led);
    builder.connect(&andGate, 0, &led, 0);  // AND output → LED

    // Create a dangling connection: end port set, start port intentionally absent.
    QNEConnection danglingConn;
    workspace.scene()->addItem(&danglingConn);
    danglingConn.setEndPort(andGate.inputPort(0));  // registers with AND input 0

    auto *sim = builder.initSimulation();
    sim->update();

    // AND gate has one dangling connection (no predecessor) and one unconnected input:
    // logic is unknown → LED input stays inactive.
    QCOMPARE(getInputStatus(&led), false);

    // A second update must also be stable.
    sim->update();
    QCOMPARE(getInputStatus(&led), false);
}

