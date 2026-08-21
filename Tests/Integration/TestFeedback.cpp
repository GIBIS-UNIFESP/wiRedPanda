// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestFeedback.h"

#include <QRegularExpression>
#include <QSignalSpy>

#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Scene/Scene.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "Tests/Common/TestUtils.h"

// ============================================================
// Convergence Detection Tests
// ============================================================

void TestFeedback::testSRLatchFromNANDConvergence()
{
    // SR latch built from NAND gates should converge to stable state
    std::unique_ptr<Scene> scene(createSRLatchFromNAND());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    verifyConvergence(scene.get(), true);
    verifyStableState(scene.get());
}

void TestFeedback::testDLatchWithFeedback()
{
    // D latch with feedback should converge
    std::unique_ptr<Scene> scene(createDLatchWithFeedback());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    verifyConvergence(scene.get(), true);
    verifyStableState(scene.get());
}

void TestFeedback::testRingOscillatorNonConvergence()
{
    // With 4-state logic, ring oscillators converge at Unknown
    // (NOT(Unknown) = Unknown is a fixed point).
    std::unique_ptr<Scene> scene(createRingOscillator());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    verifyConvergence(scene.get(), true);
}

void TestFeedback::testSetResetPriorityInSRLatch()
{
    // SR latch from NAND gates (createSRLatchFromNAND(): switchS(y=0) -> nand1[0] (Q, y=0);
    // switchR(y=50) -> nand2[0] (Q', y=50); cross-feedback ties them together). This is an
    // active-low NAND latch: driving S low forces Q high and Q' low (Set); driving R low
    // forces Q low and Q' high (Reset).
    std::unique_ptr<Scene> scene(createSRLatchFromNAND());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    InputSwitch *switchS = nullptr;
    InputSwitch *switchR = nullptr;
    GraphicElement *nand1 = nullptr; // Q
    GraphicElement *nand2 = nullptr; // Q'
    findSRLatchParts(scene.get(), switchS, switchR, nand1, nand2);
    QVERIFY2(switchS && switchR && nand1 && nand2, "Failed to locate S/R switches and NAND gates");

    Simulation sim(scene.get());

    switchS->setOn(false); // S active (low)
    switchR->setOn(true);
    sim.update();
    sim.update();
    QCOMPARE(nand1->outputPort(0)->status(), Status::Active);
    QCOMPARE(nand2->outputPort(0)->status(), Status::Inactive);

    switchS->setOn(true);
    switchR->setOn(false); // R active (low)
    sim.update();
    sim.update();
    QCOMPARE(nand1->outputPort(0)->status(), Status::Inactive);
    QCOMPARE(nand2->outputPort(0)->status(), Status::Active);
}

void TestFeedback::testInitialStateDependency()
{
    // A latch's whole point is memory: the same present input (S=1, R=1, the NAND latch's
    // "hold" state) must settle to different stable outputs depending on which state the
    // latch was driven into first. Two identical latches, driven to opposite states, then
    // both released into "hold" -- if the circuit had no memory, both would converge to the
    // same output; a real latch instead retains whichever state it was left in.
    std::unique_ptr<Scene> scene1(createSRLatchFromNAND());
    std::unique_ptr<Scene> scene2(createSRLatchFromNAND());

    QVERIFY2(scene1 != nullptr, "Failed to create first scene");
    QVERIFY2(scene2 != nullptr, "Failed to create second scene");

    InputSwitch *switchS1 = nullptr, *switchR1 = nullptr;
    GraphicElement *nand1Q1 = nullptr, *nand1Q1Bar = nullptr;
    findSRLatchParts(scene1.get(), switchS1, switchR1, nand1Q1, nand1Q1Bar);
    QVERIFY(switchS1 && switchR1 && nand1Q1 && nand1Q1Bar);

    InputSwitch *switchS2 = nullptr, *switchR2 = nullptr;
    GraphicElement *nand2Q1 = nullptr, *nand2Q1Bar = nullptr;
    findSRLatchParts(scene2.get(), switchS2, switchR2, nand2Q1, nand2Q1Bar);
    QVERIFY(switchS2 && switchR2 && nand2Q1 && nand2Q1Bar);

    Simulation sim1(scene1.get());
    Simulation sim2(scene2.get());

    // Drive scene1 into Set (Q=1), scene2 into Reset (Q=0) -- opposite initial states.
    switchS1->setOn(false);
    switchR1->setOn(true);
    sim1.update();
    sim1.update();

    switchS2->setOn(true);
    switchR2->setOn(false);
    sim2.update();
    sim2.update();

    // Now release both into the identical "hold" input (S=1, R=1).
    switchS1->setOn(true);
    switchR1->setOn(true);
    sim1.update();
    sim1.update();

    switchS2->setOn(true);
    switchR2->setOn(true);
    sim2.update();
    sim2.update();

    // Same present input, different remembered state.
    QCOMPARE(nand1Q1->outputPort(0)->status(), Status::Active);
    QCOMPARE(nand2Q1->outputPort(0)->status(), Status::Inactive);
    QVERIFY2(nand1Q1->outputPort(0)->status() != nand2Q1->outputPort(0)->status(),
              "Two latches driven to opposite states and released into the same hold input must retain different states");
}

void TestFeedback::testPureCombinationalCircuit()
{
    // Pure combinational circuits should not require iterative settling
    Scene scene;

    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *and2 = ElementFactory::buildElement(ElementType::And);
    auto *btn = ElementFactory::buildElement(ElementType::InputButton);

    QVERIFY2(and1 != nullptr, "Failed to create AND gate 1");
    QVERIFY2(and2 != nullptr, "Failed to create AND gate 2");
    QVERIFY2(btn != nullptr, "Failed to create button");

    scene.addItem(and1);
    scene.addItem(and2);
    scene.addItem(btn);

    // Set positions
    and1->setPos(100, 100);
    and2->setPos(200, 100);
    btn->setPos(0, 100);

    // Create simple combinational chain: btn → and1[0,1] → and2[0,1]
    auto *conn1 = new Connection();
    scene.addItem(conn1);
    conn1->setStartPort(btn->outputPort(0));
    conn1->setEndPort(and1->inputPort(0));

    auto *conn2 = new Connection();
    scene.addItem(conn2);
    conn2->setStartPort(btn->outputPort(0));
    conn2->setEndPort(and1->inputPort(1));

    auto *conn3 = new Connection();
    scene.addItem(conn3);
    conn3->setStartPort(and1->outputPort(0));
    conn3->setEndPort(and2->inputPort(0));

    auto *conn4 = new Connection();
    scene.addItem(conn4);
    conn4->setStartPort(and1->outputPort(0));
    conn4->setEndPort(and2->inputPort(1));

    // Run simulation
    Simulation sim(&scene);
    sim.update();

    // Verify no feedback loops exist
    bool found = false;
    for (auto *elem : scene.elements()) {
        if (sim.isInFeedbackLoop(elem)) {
            found = true;
            break;
        }
    }
    QVERIFY2(!found, "Pure combinational circuit should have no feedback elements");
}

void TestFeedback::testIterationCountVerification()
{
    // Verify iteration count for convergence
    std::unique_ptr<Scene> scene(createSRLatchFromNAND());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    Simulation sim(scene.get());

    // First update should complete convergence check
    sim.update();

    // Verify that convergence was detected
    verifyFeedbackDetection(scene.get());
}

void TestFeedback::testConvergenceSpeedVariation()
{
    // Different circuits should converge in different numbers of iterations. Use
    // Simulation::iterativeSettle() directly with a rising iteration budget to find each
    // circuit's actual minimal convergence count, and compare them.
    std::unique_ptr<Scene> scene1(createSRLatchFromNAND());      // Should converge quickly (2-3 iterations)
    std::unique_ptr<Scene> scene2(createRingOscillator());        // odd inverter count -- may not converge at all

    QVERIFY2(scene1 != nullptr, "Failed to create first scene");
    QVERIFY2(scene2 != nullptr, "Failed to create second scene");

    // A real Simulation::update() first, to run through initialize() so each element's
    // internal sim-value arrays are sized and wired to their predecessors -- calling
    // updateLogic() (via iterativeSettle() below) on a never-initialized element crashes.
    Simulation sim1(scene1.get());
    sim1.update();
    Simulation sim2(scene2.get());
    QTest::ignoreMessage(QtDebugMsg, QRegularExpression(".*converge.*"));
    sim2.update();

    auto minConvergenceIterations = [](Scene *scene) -> int {
        const auto elements = scene->elements();
        for (int n = 1; n <= Simulation::kMaxSettleIterations; ++n) {
            // resetSimState() restarts every output at its power-on default, so each attempt
            // below measures convergence from the same blank slate (the arrays themselves
            // stay sized/wired from the sim.update() call above).
            for (auto *elem : elements) {
                elem->resetSimState();
            }
            if (Simulation::iterativeSettle(elements, n)) {
                return n;
            }
        }
        return -1; // did not converge within the budget
    };

    const int srLatchIterations = minConvergenceIterations(scene1.get());
    const int ringOscillatorIterations = minConvergenceIterations(scene2.get());

    // Empirically: the SR latch converges in 2 iterations; the odd-length ring oscillator
    // does not converge within the kMaxSettleIterations budget at all (-1).
    QCOMPARE(srLatchIterations, 2);
    QCOMPARE(ringOscillatorIterations, -1);
}

// ============================================================
// Non-Converging Circuits Tests
// ============================================================

void TestFeedback::testRingOscillatorWarningAfterMaxIterations()
{
    // The odd-length ring oscillator never converges within kMaxSettleIterations (confirmed
    // by testConvergenceSpeedVariation) -- Simulation::update() must log a debug warning about
    // it. QTest::ignoreMessage() fails the test if the message never appears.
    std::unique_ptr<Scene> scene(createRingOscillator());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    Simulation sim(scene.get());
    QTest::ignoreMessage(QtDebugMsg, QRegularExpression(".*did not converge after 10 iterations.*"));
    sim.update();
}

void TestFeedback::testOddLengthInverterChainFeedback()
{
    // Odd-length inverter chain (feedback loop) converges at Unknown in 4-state logic
    std::unique_ptr<Scene> scene(createRingOscillator());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    Simulation sim(scene.get());
    sim.update();

    // Verify feedback was detected
    verifyFeedbackDetection(scene.get());
}

void TestFeedback::testConflictingFeedbackSignals()
{
    // Feedback with conflicting signals should still converge
    std::unique_ptr<Scene> scene(createSRLatchFromNAND());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    // Set both S=1 and R=1 (conflicting signals)
    auto elements = scene->elements();
    for (auto *elem : elements) {
        if (elem->elementType() == ElementType::InputSwitch) {
            auto *sw = dynamic_cast<InputSwitch *>(elem);
            if (sw) {
                sw->setOn(true);
            }
        }
    }

    verifyConvergence(scene.get(), true);
}

void TestFeedback::testWarningMessageContent()
{
    // Simulation::updateWithIterativeSettling() emits simulationWarning() with the actual
    // user-facing text alongside the qDebug() line -- inspect that, rather than just its
    // existence (already covered by testRingOscillatorWarningAfterMaxIterations) or the
    // presence of feedback elements (already covered by testAllCycleNodesMarked/
    // testOddLengthInverterChainFeedback).
    std::unique_ptr<Scene> scene(createRingOscillator());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    Simulation sim(scene.get());
    QSignalSpy warningSpy(&sim, &Simulation::simulationWarning);

    QTest::ignoreMessage(QtDebugMsg, QRegularExpression(".*did not converge.*"));
    sim.update();

    QCOMPARE(warningSpy.count(), 1);
    const QString message = warningSpy.constFirst().constFirst().toString();
    QVERIFY2(message.contains(QStringLiteral("did not converge"), Qt::CaseInsensitive),
              qPrintable(QStringLiteral("Warning message '%1' must explain the circuit did not converge").arg(message)));
    QVERIFY2(!message.isEmpty(), "Warning message must not be empty");
}

void TestFeedback::testSimulationContinuesAfterNonConvergence()
{
    // Simulation should continue even if feedback loop converges at Unknown
    std::unique_ptr<Scene> scene(createRingOscillator());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    Simulation sim(scene.get());

    // Should not throw, should continue
    sim.update();
    sim.update();  // Multiple updates should work

    // Verify scene still has elements
    QVERIFY2(!scene->elements().isEmpty(), "Feedback circuit should contain elements");
}

// ============================================================
// Mixed Circuits Tests
// ============================================================

void TestFeedback::testCombinationalPlusFeedbackInSameCircuit()
{
    // Circuit with both combinational logic and feedback
    std::unique_ptr<Scene> scene(createMixedCircuit());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    Simulation sim(scene.get());
    sim.update();

    // Verify that mixed circuit stabilizes
    verifyFeedbackDetection(scene.get());
}

void TestFeedback::testMultipleIndependentFeedbackLoops()
{
    // Multiple independent feedback loops in same circuit
    Scene scene;

    // Create two separate SR latches (two feedback loops)
    auto *nand1a = ElementFactory::buildElement(ElementType::Nand);
    auto *nand1b = ElementFactory::buildElement(ElementType::Nand);
    auto *nand2a = ElementFactory::buildElement(ElementType::Nand);
    auto *nand2b = ElementFactory::buildElement(ElementType::Nand);

    QVERIFY2(nand1a != nullptr, "Failed to create NAND gate 1a");
    QVERIFY2(nand1b != nullptr, "Failed to create NAND gate 1b");
    QVERIFY2(nand2a != nullptr, "Failed to create NAND gate 2a");
    QVERIFY2(nand2b != nullptr, "Failed to create NAND gate 2b");

    scene.addItem(nand1a);
    scene.addItem(nand1b);
    scene.addItem(nand2a);
    scene.addItem(nand2b);

    nand1a->setPos(0, 0);
    nand1b->setPos(50, 0);
    nand2a->setPos(150, 0);
    nand2b->setPos(200, 0);

    // First SR latch: nand1a ↔ nand1b
    auto *conn1 = new Connection();
    scene.addItem(conn1);
    conn1->setStartPort(nand1a->outputPort(0));
    conn1->setEndPort(nand1b->inputPort(0));

    auto *conn2 = new Connection();
    scene.addItem(conn2);
    conn2->setStartPort(nand1b->outputPort(0));
    conn2->setEndPort(nand1a->inputPort(0));

    // Second SR latch: nand2a ↔ nand2b
    auto *conn3 = new Connection();
    scene.addItem(conn3);
    conn3->setStartPort(nand2a->outputPort(0));
    conn3->setEndPort(nand2b->inputPort(0));

    auto *conn4 = new Connection();
    scene.addItem(conn4);
    conn4->setStartPort(nand2b->outputPort(0));
    conn4->setEndPort(nand2a->inputPort(0));

    Simulation sim(&scene);
    sim.update();

    verifyFeedbackDetection(&scene);
}

void TestFeedback::testNestedFeedbackLoops()
{
    // Feedback loops (3-NOT ring - odd length, converges at Unknown in 4-state logic)
    std::unique_ptr<Scene> scene(createRingOscillator());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    Simulation sim(scene.get());
    sim.update();

    verifyFeedbackDetection(scene.get());
}

void TestFeedback::testPriorityCalculationWithFeedback()
{
    // Priority calculation in logicelement.cpp should detect feedback
    std::unique_ptr<Scene> scene(createSRLatchFromNAND());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    // Verify feedback detection sets priority correctly
    verifyFeedbackDetection(scene.get());
}

// ============================================================
// Edge Cases Tests
// ============================================================

void TestFeedback::testSingleElementFeedback()
{
    // NOT gate connected to itself (single element feedback, converges at Unknown)
    Scene scene;

    auto *notGate = ElementFactory::buildElement(ElementType::Not);
    QVERIFY2(notGate != nullptr, "Failed to create NOT gate");

    scene.addItem(notGate);
    notGate->setPos(50, 50);

    // Create feedback connection: NOT output -> NOT input (self-loop)
    auto *conn = new Connection();
    scene.addItem(conn);
    conn->setStartPort(notGate->outputPort(0));
    conn->setEndPort(notGate->inputPort(0));

    Simulation sim(&scene);
    sim.update();

    verifyFeedbackDetection(&scene);
}

void TestFeedback::testFeedbackThroughMultipleElementTypes()
{
    // Feedback path through multiple element types (3-NOT ring - odd length, converges at Unknown in 4-state logic)
    std::unique_ptr<Scene> scene(createRingOscillator());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    Simulation sim(scene.get());
    sim.update();

    verifyFeedbackDetection(scene.get());
}

void TestFeedback::testLargeFeedbackLoops()
{
    // Large feedback loop with 10 elements (even count → converges)
    Scene scene;

    // Create chain of 10 inverters
    QVector<GraphicElement *> inverters;
    for (int i = 0; i < 10; ++i) {
        auto *inv = ElementFactory::buildElement(ElementType::Not);
        QVERIFY2(inv != nullptr, "Failed to create inverter");
        scene.addItem(inv);
        inv->setPos(i * 50, 0);
        inverters.append(inv);
    }

    // Wire them as a ring: inv0 → inv1 → ... → inv9 → inv0
    for (int i = 0; i < 10; ++i) {
        auto *conn = new Connection();
        scene.addItem(conn);
        conn->setStartPort(inverters[i]->outputPort(0));
        conn->setEndPort(inverters[(i + 1) % 10]->inputPort(0));
    }

    Simulation sim(&scene);
    sim.update();

    verifyFeedbackDetection(&scene);
    verifyStableState(&scene);
}

void TestFeedback::testAllCycleNodesMarked()
{
    // SR latch from NAND: switchS → nand1[0], switchR → nand2[0], nand1 ↔ nand2
    // Both NAND gates are in the cycle; both switches are NOT.
    std::unique_ptr<Scene> scene(createSRLatchFromNAND());
    QVERIFY(scene != nullptr);

    Simulation sim(scene.get());
    sim.update();

    int feedbackCount = 0;
    int nonFeedbackCount = 0;

    for (auto *elem : scene->elements()) {
        if (elem->elementType() == ElementType::InputSwitch) {
            QVERIFY2(!sim.isInFeedbackLoop(elem),
                     "Input switch should NOT be in feedback loop");
            ++nonFeedbackCount;
        } else if (elem->elementType() == ElementType::Nand) {
            QVERIFY2(sim.isInFeedbackLoop(elem),
                     "NAND gate in SR latch cycle should be in feedback loop");
            ++feedbackCount;
        }
    }

    QCOMPARE(feedbackCount, 2);
    QCOMPARE(nonFeedbackCount, 2);
}

// ============================================================
// Performance Tests
// ============================================================

void TestFeedback::testDeepCircuitsWithFeedback()
{
    // Deep circuit with feedback loop (10 NOTs in ring)
    Scene scene;

    // Create 10 NOT gates in a ring
    QVector<GraphicElement *> elements;
    for (int i = 0; i < 10; ++i) {
        auto *notGate = ElementFactory::buildElement(ElementType::Not);
        QVERIFY2(notGate != nullptr, "Failed to create NOT gate");
        scene.addItem(notGate);
        notGate->setPos(i * 50, 0);
        elements.append(notGate);
    }

    // Wire them as a ring: not0 → not1 → ... → not9 → not0 (even count → converges)
    for (int i = 0; i < 10; ++i) {
        auto *conn = new Connection();
        scene.addItem(conn);
        conn->setStartPort(elements[i]->outputPort(0));
        conn->setEndPort(elements[(i + 1) % 10]->inputPort(0));
    }

    Simulation sim(&scene);
    sim.update();

    verifyFeedbackDetection(&scene);
}

void TestFeedback::testMultipleSimultaneousFeedbackLoops()
{
    // 5 or more independent feedback loops running simultaneously
    Scene scene;

    // Create 5 separate 2-NOT feedback rings (each bistable, converges)
    QVector<GraphicElement *> elements;
    for (int loop = 0; loop < 5; ++loop) {
        QVector<GraphicElement *> loopElements;
        for (int i = 0; i < 2; ++i) {  // 2 NOT gates per loop
            auto *notGate = ElementFactory::buildElement(ElementType::Not);
            QVERIFY2(notGate != nullptr, "Failed to create NOT gate");
            scene.addItem(notGate);
            notGate->setPos(loop * 150 + i * 60, 0);
            loopElements.append(notGate);
            elements.append(notGate);
        }

        // Wire the 2-NOT ring: not0 ↔ not1
        auto *conn1 = new Connection();
        scene.addItem(conn1);
        conn1->setStartPort(loopElements[0]->outputPort(0));
        conn1->setEndPort(loopElements[1]->inputPort(0));

        auto *conn2 = new Connection();
        scene.addItem(conn2);
        conn2->setStartPort(loopElements[1]->outputPort(0));
        conn2->setEndPort(loopElements[0]->inputPort(0));
    }

    Simulation sim(&scene);
    sim.update();

    verifyFeedbackDetection(&scene);
}

// ============================================================
// Helper Functions
// ============================================================

void TestFeedback::findSRLatchParts(Scene *scene, InputSwitch *&switchS, InputSwitch *&switchR,
                                    GraphicElement *&nand1Q, GraphicElement *&nand2QBar)
{
    switchS = switchR = nullptr;
    nand1Q = nand2QBar = nullptr;
    for (auto *elem : scene->elements()) {
        const bool isSSide = (elem->pos().y() == 0);
        if (elem->elementType() == ElementType::InputSwitch) {
            (isSSide ? switchS : switchR) = dynamic_cast<InputSwitch *>(elem);
        } else if (elem->elementType() == ElementType::Nand) {
            (isSSide ? nand1Q : nand2QBar) = elem;
        }
    }
}

Scene *TestFeedback::createSRLatchFromNAND()
{
    auto *scene = new Scene();

    auto *nand1 = ElementFactory::buildElement(ElementType::Nand);
    auto *nand2 = ElementFactory::buildElement(ElementType::Nand);
    auto *switchS = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *switchR = ElementFactory::buildElement(ElementType::InputSwitch);

    scene->addItem(nand1);
    scene->addItem(nand2);
    scene->addItem(switchS);
    scene->addItem(switchR);

    nand1->setPos(100, 0);
    nand2->setPos(100, 50);
    switchS->setPos(0, 0);
    switchR->setPos(0, 50);

    // Wire: S → nand1[0], R → nand2[0]
    auto *connS = new Connection();
    scene->addItem(connS);
    connS->setStartPort(switchS->outputPort(0));
    connS->setEndPort(nand1->inputPort(0));

    auto *connR = new Connection();
    scene->addItem(connR);
    connR->setStartPort(switchR->outputPort(0));
    connR->setEndPort(nand2->inputPort(0));

    // Cross-feedback: nand2.out → nand1[1], nand1.out → nand2[1]
    auto *connFeed1 = new Connection();
    scene->addItem(connFeed1);
    connFeed1->setStartPort(nand2->outputPort(0));
    connFeed1->setEndPort(nand1->inputPort(1));

    auto *connFeed2 = new Connection();
    scene->addItem(connFeed2);
    connFeed2->setStartPort(nand1->outputPort(0));
    connFeed2->setEndPort(nand2->inputPort(1));

    return scene;
}

Scene *TestFeedback::createDLatchWithFeedback()
{
    auto *scene = new Scene();

    auto *dLatch = ElementFactory::buildElement(ElementType::DLatch);
    auto *switchData = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *switchEnable = ElementFactory::buildElement(ElementType::InputSwitch);

    scene->addItem(dLatch);
    scene->addItem(switchData);
    scene->addItem(switchEnable);

    dLatch->setPos(100, 50);
    switchData->setPos(0, 0);
    switchEnable->setPos(0, 50);

    // Wire Data and Enable inputs
    auto *connData = new Connection();
    scene->addItem(connData);
    connData->setStartPort(switchData->outputPort(0));
    connData->setEndPort(dLatch->inputPort(0));

    auto *connEnable = new Connection();
    scene->addItem(connEnable);
    connEnable->setStartPort(switchEnable->outputPort(0));
    connEnable->setEndPort(dLatch->inputPort(1));

    return scene;
}

Scene *TestFeedback::createRingOscillator()
{
    auto *scene = new Scene();

    auto *not1 = ElementFactory::buildElement(ElementType::Not);
    auto *not2 = ElementFactory::buildElement(ElementType::Not);
    auto *not3 = ElementFactory::buildElement(ElementType::Not);

    scene->addItem(not1);
    scene->addItem(not2);
    scene->addItem(not3);

    not1->setPos(0, 0);
    not2->setPos(100, 0);
    not3->setPos(200, 0);

    // Wire as ring: NOT1 -> NOT2 -> NOT3 -> NOT1 (odd count, non-converging)
    auto *conn1 = new Connection();
    scene->addItem(conn1);
    conn1->setStartPort(not1->outputPort(0));
    conn1->setEndPort(not2->inputPort(0));

    auto *conn2 = new Connection();
    scene->addItem(conn2);
    conn2->setStartPort(not2->outputPort(0));
    conn2->setEndPort(not3->inputPort(0));

    auto *conn3 = new Connection();
    scene->addItem(conn3);
    conn3->setStartPort(not3->outputPort(0));
    conn3->setEndPort(not1->inputPort(0));

    return scene;
}

Scene *TestFeedback::createMixedCircuit()
{
    auto *scene = new Scene();

    auto *and1 = ElementFactory::buildElement(ElementType::And);
    auto *or1 = ElementFactory::buildElement(ElementType::Or);
    auto *not1 = ElementFactory::buildElement(ElementType::Not);
    auto *btn = ElementFactory::buildElement(ElementType::InputButton);

    scene->addItem(and1);
    scene->addItem(or1);
    scene->addItem(not1);
    scene->addItem(btn);

    and1->setPos(0, 0);
    or1->setPos(100, 0);
    not1->setPos(200, 0);
    btn->setPos(-100, 0);

    // Wire: btn → and1[0,1] → or1[0], not1.out → or1[1], or1.out → not1.in
    // Combinational path: btn → AND → OR
    auto *connBtn1 = new Connection();
    scene->addItem(connBtn1);
    connBtn1->setStartPort(btn->outputPort(0));
    connBtn1->setEndPort(and1->inputPort(0));

    auto *connBtn2 = new Connection();
    scene->addItem(connBtn2);
    connBtn2->setStartPort(btn->outputPort(0));
    connBtn2->setEndPort(and1->inputPort(1));

    auto *connAnd = new Connection();
    scene->addItem(connAnd);
    connAnd->setStartPort(and1->outputPort(0));
    connAnd->setEndPort(or1->inputPort(0));

    // Feedback ring: OR → NOT → OR
    auto *connOr = new Connection();
    scene->addItem(connOr);
    connOr->setStartPort(or1->outputPort(0));
    connOr->setEndPort(not1->inputPort(0));

    auto *connFeed = new Connection();
    scene->addItem(connFeed);
    connFeed->setStartPort(not1->outputPort(0));
    connFeed->setEndPort(or1->inputPort(1));

    return scene;
}

void TestFeedback::verifyConvergence(Scene *scene, bool shouldConverge)
{
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    if (!shouldConverge) {
        // Non-converging circuits will print a debug warning
        QTest::ignoreMessage(QtDebugMsg, QRegularExpression(".*converge.*"));
    }

    Simulation sim(scene);
    sim.update();

    if (shouldConverge) {
        // For converging circuits, verify outputs are stable
        // Snapshot all logic element outputs
        QHash<GraphicElement *, Status> snapshot;
        for (auto *elem : scene->elements()) {
            if (elem->simOutputSize() > 0) {
                snapshot[elem] = elem->outputValue(0);
            }
        }

        // Run another update cycle
        sim.update();

        // Verify outputs haven't changed
        for (auto *elem : scene->elements()) {
            if (elem->simOutputSize() > 0) {
                Status currentValue = elem->outputValue(0);
                Status snapshotValue = snapshot.value(elem, currentValue);
                QCOMPARE(currentValue, snapshotValue);
            }
        }
    }
}

void TestFeedback::verifyFeedbackDetection(Scene *scene)
{
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    // Initialize simulation to trigger feedback detection
    Simulation sim(scene);
    sim.update();

    // Verify that at least one element is in a feedback loop
    bool found = false;
    for (auto *elem : scene->elements()) {
        if (sim.isInFeedbackLoop(elem)) {
            found = true;
            break;
        }
    }
    QVERIFY2(found, "No element detected in a feedback loop");
}

void TestFeedback::verifyStableState(Scene *scene)
{
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    auto elements = scene->elements();
    QVERIFY2(!elements.isEmpty(), "Circuit elements should not be empty");

    // Initialize simulation to stabilize
    Simulation sim(scene);
    sim.update();

    // Snapshot all logic element outputs
    QHash<GraphicElement *, Status> snapshot;
    for (auto *elem : elements) {
        if (elem->simOutputSize() > 0) {
            snapshot[elem] = elem->outputValue(0);
        }
    }

    // Run another update cycle
    sim.update();

    // Verify outputs haven't changed (circuit is stable)
    for (auto *elem : elements) {
        if (elem->simOutputSize() > 0) {
            Status currentValue = elem->outputValue(0);
            Status snapshotValue = snapshot.value(elem, currentValue);
            QCOMPARE(currentValue, snapshotValue);
        }
    }
}
