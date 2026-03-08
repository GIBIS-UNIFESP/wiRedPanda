// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestSimulation.h"

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/InputButton.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Nand.h"
#include "App/Element/GraphicElements/Nor.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/GraphicElements/SRLatch.h"
#include "App/Element/GraphicElements/Xor.h"
#include "App/Nodes/QNEConnection.h"
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

    const auto elements(Common::sortGraphicElements(workspace.scene()->elements()));

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
    // This verifies our fix for std::stable_sort in ElementMapping::sortLogicElements
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
        const auto elements = Common::sortGraphicElements(scene->elements());
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

void TestSimulation::testElementMappingStability()
{
    // Test ElementMapping for consistent behavior across multiple initializations
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

        // Create ElementMapping and test its consistent behavior
        auto elements = scene->elements();
        ElementMapping mapping(elements);

        // The constructor triggers priority calculation and sorting
        mapping.sort();
        const auto &logicElements = mapping.logicElms();

        // Store structural properties to verify consistent ordering
        QVector<QPair<int, int>> elementProperties;
        for (const auto &logicPtr : logicElements) {
            // Store priority and output size as stable comparison
            int priority = logicPtr->priority();
            int outputSize = logicPtr->outputSize();
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
                QString msg = QString("ElementMapping structure mismatch at run %1, position %2: expected (%3,%4), got (%5,%6)")
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
// Negative Tests — Graceful handling of degenerate circuit conditions
// ============================================================================

void TestSimulation::testUnconnectedRequiredInputGraceful()
{
    // An AND gate with no inputs connected is invalid in the logic layer.
    // The simulation must not crash, and the downstream LED must stay inactive
    // (the invalid element and its successors receive Status::Invalid).
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    And andGate;
    Led led;

    builder.add(&andGate, &led);
    builder.connect(&andGate, 0, &led, 0);  // AND output → LED (no AND inputs connected)

    auto *sim = builder.initSimulation();
    sim->update();

    // AND gate logic is invalid (required inputs unconnected); cascade invalidates LED.
    // LED input port receives Status::Invalid → getInputStatus returns false.
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
    And andIsolated;  // no connections — will be invalid

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
    // ElementMapping skips it (startPort() == nullptr), so the AND gate's input
    // remains unconnected → invalid logic → simulation must not crash.
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
    // logic is invalid → LED input stays inactive.
    QCOMPARE(getInputStatus(&led), false);

    // A second update must also be stable.
    sim->update();
    QCOMPARE(getInputStatus(&led), false);
}
