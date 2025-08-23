// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testsimulation.h"

#include "and.h"
#include "common.h"
#include "elementmapping.h"
#include "inputbutton.h"
#include "led.h"
#include "nor.h"
#include "qneconnection.h"
#include "scene.h"
#include "simulation.h"
#include "workspace.h"

#include <QTest>
#include <QSignalSpy>

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

void TestSimulation::testCrossCoupledConvergence()
{
    // Test basic cross-coupled NOR gates for convergence behavior
    WorkSpace workspace;
    auto *scene = workspace.scene();
    
    // Create two cross-coupled NOR gates with inputs to bias initial state
    InputButton input1;  // Add input to bias the initial state
    Nor nor1, nor2;
    Led led1, led2;
    QNEConnection connInput, conn1, conn2, conn3, conn4;
    
    scene->addItem(&input1);
    scene->addItem(&nor1);
    scene->addItem(&nor2);
    scene->addItem(&led1);
    scene->addItem(&led2);
    scene->addItem(&connInput);
    scene->addItem(&conn1);
    scene->addItem(&conn2);
    scene->addItem(&conn3);
    scene->addItem(&conn4);
    
    // Connect one input to bias initial state
    connInput.setStartPort(input1.outputPort());
    connInput.setEndPort(nor1.inputPort(0));  // Input to NOR1
    
    // Cross-couple the NOR gates
    conn1.setStartPort(nor1.outputPort());
    conn1.setEndPort(nor2.inputPort(1));  // Q → NOR2 input
    
    conn2.setStartPort(nor2.outputPort());
    conn2.setEndPort(nor1.inputPort(1));  // Q̄ → NOR1 input
    
    // Connect outputs to LEDs
    conn3.setStartPort(nor1.outputPort());
    conn3.setEndPort(led1.inputPort());
    
    conn4.setStartPort(nor2.outputPort());
    conn4.setEndPort(led2.inputPort());
    
    // Create element mapping and detect feedback loops
    QVector<GraphicElement *> elements = {&input1, &nor1, &nor2, &led1, &led2};
    ElementMapping mapping(elements);
    mapping.sort();
    
    // Verify feedback groups were detected
    const auto &feedbackGroups = mapping.feedbackGroups();
    QVERIFY(!feedbackGroups.isEmpty());
    QVERIFY(feedbackGroups.size() >= 1);
    
    // Verify that NOR gates are marked as feedback dependent
    QVERIFY(nor1.logic()->isFeedbackDependent());
    QVERIFY(nor2.logic()->isFeedbackDependent());
    
    // Initialize simulation 
    Simulation simulation(scene);
    QVERIFY(simulation.initialize());
    
    // Test with input LOW (should result in complementary outputs)
    input1.setOn(false);
    simulation.update();
    
    bool q1_low = (led1.inputPort()->status() == Status::Active);
    bool q2_low = (led2.inputPort()->status() == Status::Active);
    
    // Test with input HIGH (should result in different complementary outputs)
    input1.setOn(true);
    simulation.update();
    
    bool q1_high = (led1.inputPort()->status() == Status::Active);
    bool q2_high = (led2.inputPort()->status() == Status::Active);
    
    // Verify that outputs are complementary in both cases
    QVERIFY(q1_low != q2_low);    // Complementary when input is low
    QVERIFY(q1_high != q2_high);  // Complementary when input is high
    
    // Verify that the circuit produces different states for different inputs
    QVERIFY((q1_low != q1_high) || (q2_low != q2_high));
}

void TestSimulation::testSRLatchHoldBehavior()
{
    // Test the critical SR latch hold behavior that was failing
    WorkSpace workspace;
    auto *scene = workspace.scene();
    
    // Create SR Latch components
    InputButton s, r;
    Nor nor1, nor2;  // nor1 = Q, nor2 = Q̄
    Led qLed, qNotLed;
    QNEConnection connS, connR, connCross1, connCross2, connQ, connQNot;
    
    // Add all items to scene
    scene->addItem(&s);
    scene->addItem(&r);
    scene->addItem(&nor1);
    scene->addItem(&nor2);
    scene->addItem(&qLed);
    scene->addItem(&qNotLed);
    scene->addItem(&connS);
    scene->addItem(&connR);
    scene->addItem(&connCross1);
    scene->addItem(&connCross2);
    scene->addItem(&connQ);
    scene->addItem(&connQNot);
    
    // Connect SR Latch with cross-coupling
    connR.setStartPort(r.outputPort());
    connR.setEndPort(nor1.inputPort(0));  // R → NOR1
    
    connS.setStartPort(s.outputPort());
    connS.setEndPort(nor2.inputPort(0));  // S → NOR2
    
    connCross1.setStartPort(nor2.outputPort());
    connCross1.setEndPort(nor1.inputPort(1));  // Cross-couple Q̄ → NOR1
    
    connCross2.setStartPort(nor1.outputPort());
    connCross2.setEndPort(nor2.inputPort(1));  // Cross-couple Q → NOR2
    
    connQ.setStartPort(nor1.outputPort());
    connQ.setEndPort(qLed.inputPort());  // Q output
    
    connQNot.setStartPort(nor2.outputPort());
    connQNot.setEndPort(qNotLed.inputPort());  // Q̄ output
    
    // Initialize simulation
    Simulation simulation(scene);
    QVERIFY(simulation.initialize());
    
    // Test sequence
    
    // 1. Reset: R=1, S=0 → Q should be 0, Q̄ should be 1
    s.setOn(false);
    r.setOn(true);
    simulation.update();
    
    bool qReset = (qLed.inputPort()->status() == Status::Active);
    bool qNotReset = (qNotLed.inputPort()->status() == Status::Active);
    
    QVERIFY(!qReset);  // Q should be low
    QVERIFY(qNotReset); // Q̄ should be high
    
    // 2. Set: S=1, R=0 → Q should be 1, Q̄ should be 0
    s.setOn(true);
    r.setOn(false);
    simulation.update();
    
    bool qSet = (qLed.inputPort()->status() == Status::Active);
    bool qNotSet = (qNotLed.inputPort()->status() == Status::Active);
    
    QVERIFY(qSet);     // Q should be high
    QVERIFY(!qNotSet); // Q̄ should be low
    
    // 3. CRITICAL TEST - Hold: S=0, R=0 → Should maintain previous state
    s.setOn(false);
    r.setOn(false);
    simulation.update();
    
    bool qHold = (qLed.inputPort()->status() == Status::Active);
    bool qNotHold = (qNotLed.inputPort()->status() == Status::Active);
    
    // This is the test that should pass with our convergence implementation
    QVERIFY(qHold);      // Q should remain high (hold set state)
    QVERIFY(!qNotHold);  // Q̄ should remain low
    QVERIFY(qHold != qNotHold);  // Outputs should be complementary
}

void TestSimulation::testFeedbackDetection()
{
    // Test that our cycle detection algorithm correctly identifies feedback loops
    WorkSpace workspace;
    auto *scene = workspace.scene();
    
    // Create a simple combinational circuit (should have no feedback)
    InputButton input;
    And andGate;
    Led output;
    QNEConnection conn1, conn2;
    
    scene->addItem(&input);
    scene->addItem(&andGate);
    scene->addItem(&output);
    scene->addItem(&conn1);
    scene->addItem(&conn2);
    
    conn1.setStartPort(input.outputPort());
    conn1.setEndPort(andGate.inputPort(0));
    conn2.setStartPort(andGate.outputPort());
    conn2.setEndPort(output.inputPort());
    
    // Test combinational circuit - should have no feedback groups
    QVector<GraphicElement *> combElements = {&input, &andGate, &output};
    ElementMapping combMapping(combElements);
    combMapping.sort();
    
    const auto &combFeedbackGroups = combMapping.feedbackGroups();
    QVERIFY(combFeedbackGroups.isEmpty());  // No feedback in combinational circuit
    QVERIFY(!andGate.logic()->isFeedbackDependent());
    
    // Now test a feedback circuit
    Nor nor1, nor2;
    QNEConnection feedbackConn1, feedbackConn2;
    
    scene->addItem(&nor1);
    scene->addItem(&nor2);
    scene->addItem(&feedbackConn1);
    scene->addItem(&feedbackConn2);
    
    // Create cross-coupled feedback
    feedbackConn1.setStartPort(nor1.outputPort());
    feedbackConn1.setEndPort(nor2.inputPort(1));
    feedbackConn2.setStartPort(nor2.outputPort());
    feedbackConn2.setEndPort(nor1.inputPort(1));
    
    QVector<GraphicElement *> feedbackElements = {&nor1, &nor2};
    ElementMapping feedbackMapping(feedbackElements);
    feedbackMapping.sort();
    
    const auto &feedbackGroups = feedbackMapping.feedbackGroups();
    QVERIFY(!feedbackGroups.isEmpty());  // Should detect feedback
    QVERIFY(feedbackGroups.size() >= 1);
    
    // Verify elements are marked as feedback dependent
    QVERIFY(nor1.logic()->isFeedbackDependent());
    QVERIFY(nor2.logic()->isFeedbackDependent());
    
    // Verify the feedback group contains both elements
    bool nor1Found = false, nor2Found = false;
    for (const auto &group : feedbackGroups) {
        for (const auto &element : group) {
            if (element.get() == nor1.logic()) nor1Found = true;
            if (element.get() == nor2.logic()) nor2Found = true;
        }
    }
    QVERIFY(nor1Found && nor2Found);
}
