// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/TestFeedback.h"

#include <ctime>

#include <QElapsedTimer>
#include <QRegularExpression>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "App/Core/Common.h"
#include "App/Core/StatusOps.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/GraphicElementInput.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/IC.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/TestUtils.h"

// ============================================================
// Convergence Detection Tests
// ============================================================

void TestFeedback::testSRLatchFromNANDConvergence()
{
    // SR latch built from NAND gates should converge to stable state
    std::unique_ptr<Scene> scene(createSRLatchFromNAND());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    verifyConvergence(scene.get());
    verifyStableState(scene.get());
}

void TestFeedback::testDLatchWithFeedback()
{
    // D latch with feedback should converge
    std::unique_ptr<Scene> scene(createDLatchWithFeedback());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    verifyConvergence(scene.get());
    verifyStableState(scene.get());
}

void TestFeedback::testRingOscillatorNonConvergence()
{
    // With 4-state logic, ring oscillators converge at Unknown
    // (NOT(Unknown) = Unknown is a fixed point).
    std::unique_ptr<Scene> scene(createRingOscillator());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    verifyConvergence(scene.get());
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
    // Different circuit shapes show different convergence characteristics under the unified
    // event-driven engine: the SR latch settles to a definite state silently, while the
    // odd-length ring oscillator can never reach a stable non-oscillating value and
    // canonicalizes to Unknown (the per-timestamp evaluation cap's documented fixed point).
    // Asserted on the converged VALUE rather than on an iteration count: the engine settles by
    // draining events per timestamp, not by fixed full-topological passes, so a raw iteration
    // count is not a meaningful quantity here.
    std::unique_ptr<Scene> scene1(createSRLatchFromNAND());
    std::unique_ptr<Scene> scene2(createRingOscillator());

    QVERIFY2(scene1 != nullptr, "Failed to create first scene");
    QVERIFY2(scene2 != nullptr, "Failed to create second scene");

    Simulation sim1(scene1.get());
    sim1.update();
    Simulation sim2(scene2.get());
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*converge.*"));
    sim2.update();

    bool srLatchHasDefiniteOutput = false;
    for (auto *elem : scene1->elements()) {
        if (elem->simOutputSize() > 0 && elem->outputValue(0) != Status::Unknown) {
            srLatchHasDefiniteOutput = true;
            break;
        }
    }
    QVERIFY2(srLatchHasDefiniteOutput, "SR latch must converge to a definite (non-Unknown) state");

    bool ringOscillatorHasUnknownOutput = false;
    for (auto *elem : scene2->elements()) {
        if (elem->simOutputSize() > 0 && elem->outputValue(0) == Status::Unknown) {
            ringOscillatorHasUnknownOutput = true;
            break;
        }
    }
    QVERIFY2(ringOscillatorHasUnknownOutput, "Odd-length ring oscillator must canonicalize to Unknown");
}

// ============================================================
// Non-Converging Circuits Tests
// ============================================================

void TestFeedback::testRingOscillatorWarningAfterMaxIterations()
{
    // The odd-length ring oscillator never converges within the per-timestamp evaluation cap
    // (confirmed by testConvergenceSpeedVariation) -- Simulation::update() must log a warning
    // about it. QTest::ignoreMessage() fails the test if the message never appears.
    std::unique_ptr<Scene> scene(createRingOscillator());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    Simulation sim(scene.get());
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*did not converge.*"));
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

    verifyConvergence(scene.get());
}

void TestFeedback::testWarningMessageContent()
{
    // The engine emits simulationWarning() once per initialize() with the
    // user-facing text alongside the qDebug() line -- inspect that, rather than just its
    // existence (already covered by testRingOscillatorWarningAfterMaxIterations) or the
    // presence of feedback elements (already covered by testAllCycleNodesMarked/
    // testOddLengthInverterChainFeedback).
    std::unique_ptr<Scene> scene(createRingOscillator());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    Simulation sim(scene.get());
    QSignalSpy warningSpy(&sim, &Simulation::simulationWarning);

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*did not converge.*"));
    sim.update();

    QCOMPARE(warningSpy.count(), 1);
    const QString message = warningSpy.constFirst().constFirst().toString();
    QVERIFY2(message.contains(QStringLiteral("did not converge"), Qt::CaseInsensitive),
              qPrintable(QStringLiteral("Warning message '%1' must explain the circuit did not converge").arg(message)));
    QVERIFY2(!message.isEmpty(), "Warning message must not be empty");
}

void TestFeedback::testNonConvergenceDiagnosticNamesTheTrippingElementAndComponent()
{
    // "did not converge at time 0" cannot be acted on: it names neither which of the circuit's
    // elements hit its evaluation cap nor which of possibly several cyclic regions is the
    // oscillating one -- and it reads identically in the case worth telling apart, where the
    // cap misfired on ACYCLIC logic and the drain abandons the timestamp instead of
    // canonicalising anything. ignoreMessage() fails the test unless a message matching this
    // shape is actually emitted.
    std::unique_ptr<Scene> scene(createRingOscillator());
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    Simulation sim(scene.get());
    QTest::ignoreMessage(QtWarningMsg,
        QRegularExpression(R"(did not converge at time .* evaluation cap tripped on .+\(.+\) in cyclic component \d+ \(\d+ members\))"));
    sim.update();
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

void TestFeedback::verifyConvergence(Scene *scene)
{
    QVERIFY2(scene != nullptr, "Failed to create feedback circuit");

    Simulation sim(scene);
    sim.update();

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

// ============================================================
// Oscillation trip-path guards
// ============================================================

QVector<GraphicElement *> TestFeedback::addInverterRing(Scene *scene, const int count, const qreal y)
{
    QVector<GraphicElement *> ring;
    for (int i = 0; i < count; ++i) {
        auto *inverter = ElementFactory::buildElement(ElementType::Not);
        scene->addItem(inverter);
        inverter->setPos(i * 100, y);
        ring.append(inverter);
    }
    for (int i = 0; i < count; ++i) {
        auto *conn = new Connection();
        scene->addItem(conn);
        conn->setStartPort(ring.at(i)->outputPort(0));
        conn->setEndPort(ring.at((i + 1) % count)->inputPort(0));
    }
    return ring;
}

void TestFeedback::testCanonicalizationReachesEveryElementOfTheRegion()
{
    // The engine stages EVERY element's evaluation, so for an element mid-window when the cap
    // trips, a bare setOutputValue() inside canonicalizeOscillation() would be routed into its
    // staging buffer and never published. A longer ring makes it likely that some members are
    // mid-window at the trip. Whichever members those are, all of them must read Unknown.
    std::unique_ptr<Scene> scene(new Scene());
    const auto ring = addInverterRing(scene.get(), 5);

    Simulation sim(scene.get());
    sim.update();

    for (int i = 0; i < ring.size(); ++i) {
        QVERIFY2(ring.at(i)->outputValue(0) == Status::Unknown,
                 qPrintable(QString("ring inverter %1 of %2 was not canonicalised (got %3) -- the "
                                    "write was swallowed by its deferred-commit staging buffer")
                                .arg(i).arg(ring.size())
                                .arg(static_cast<int>(ring.at(i)->outputValue(0)))));
    }
}

void TestFeedback::testFlipFlopStillSamplesWhenOscillationTripsSameTick()
{
    // On a trip the drain does `canonicalizeOscillation(t); break;`, and that break leaves the
    // timestamp loop BEFORE the sampling region -- so every Memory-group element queued in
    // pendingSamples for that timestamp never samples at all.
    //
    // Reaching that requires care. A plain always-on ring oscillates on the very first tick,
    // where the flip-flop is evaluated by the SEED pass (which calls updateLogic() directly,
    // outside the queue) and therefore captures its edge before any drain happens -- such a
    // circuit passes without discriminating anything. So the ring is
    // GATED: it is stable while the enable is low, and only starts oscillating on the later tick
    // that also clocks the flip-flop, which is the tick where both go through the drain together.
    std::unique_ptr<Scene> scene(new Scene());

    // Gated ring: NAND(feedback, enable) -> NOT -> NOT -> back to NAND. With enable low the NAND
    // output is pinned high and the loop is stable; with enable high the NAND acts as a third
    // inverter and the loop oscillates.
    auto *nandGate = ElementFactory::buildElement(ElementType::Nand);
    auto *inv1 = ElementFactory::buildElement(ElementType::Not);
    auto *inv2 = ElementFactory::buildElement(ElementType::Not);
    auto *enableSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
    scene->addItem(nandGate);
    scene->addItem(inv1);
    scene->addItem(inv2);
    scene->addItem(enableSwitch);
    nandGate->setPos(100, 0);
    inv1->setPos(200, 0);
    inv2->setPos(300, 0);
    enableSwitch->setPos(0, 0);

    const auto wire = [&scene](GraphicElement *from, int fromPort, GraphicElement *to, int toPort) {
        auto *conn = new Connection();
        scene->addItem(conn);
        conn->setStartPort(from->outputPort(fromPort));
        conn->setEndPort(to->inputPort(toPort));
    };
    wire(enableSwitch, 0, nandGate, 1);
    wire(nandGate, 0, inv1, 0);
    wire(inv1, 0, inv2, 0);
    wire(inv2, 0, nandGate, 0);

    auto *flipFlop = ElementFactory::buildElement(ElementType::DFlipFlop);
    auto *dataSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *clockSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
    scene->addItem(flipFlop);
    scene->addItem(dataSwitch);
    scene->addItem(clockSwitch);
    flipFlop->setPos(200, 300);
    dataSwitch->setPos(0, 300);
    clockSwitch->setPos(0, 360);
    wire(dataSwitch, 0, flipFlop, 0);  // Data
    wire(clockSwitch, 0, flipFlop, 1); // Clock

    qobject_cast<InputSwitch *>(dataSwitch)->setOn(true);
    qobject_cast<InputSwitch *>(clockSwitch)->setOn(false);
    qobject_cast<InputSwitch *>(enableSwitch)->setOn(false);

    Simulation sim(scene.get());
    sim.update(); // settles: ring gated off, clock low, Q low
    QCOMPARE(flipFlop->outputValue(0), Status::Inactive);

    // Now start the oscillation and clock the flip-flop on the SAME tick. Both reach the engine
    // through the drain's queue, so the flip-flop's evaluation lands in pendingSamples for the
    // timestamp on which the ring trips the cap.
    qobject_cast<InputSwitch *>(enableSwitch)->setOn(true);
    qobject_cast<InputSwitch *>(clockSwitch)->setOn(true);
    sim.update();

    QVERIFY2(flipFlop->outputValue(0) == Status::Active,
             qPrintable(QString("the flip-flop must still capture its rising edge on a tick where "
                                "an unrelated region trips the oscillation cap (Q = %1)")
                            .arg(static_cast<int>(flipFlop->outputValue(0)))));
}

// ============================================================
// Non-convergence semantics
// ============================================================

void TestFeedback::testOscillationUnknownReachesDownstreamReader()
{
    // A 3-inverter ring never settles, so the engine canonicalises it to Unknown. An element
    // that merely READS the ring is not part of the cycle, so it is not canonicalised -- it must
    // receive the Unknown by ordinary propagation. Clearing the event queue on the trip would
    // strand it: on every later tick the steady-state seed wakes successors only for changed
    // inputs and clocks, never for an arbitrary element.
    std::unique_ptr<Scene> scene(new Scene());
    const auto ring = addInverterRing(scene.get(), 3);

    auto *reader = ElementFactory::buildElement(ElementType::Not);
    scene->addItem(reader);
    reader->setPos(0, 200);
    auto *tap = new Connection();
    scene->addItem(tap);
    tap->setStartPort(ring.at(0)->outputPort(0));
    tap->setEndPort(reader->inputPort(0));

    Simulation sim(scene.get());
    sim.update();
    sim.update(); // a second tick, to show the reader is stuck rather than merely late

    // ORDER IS LOAD-BEARING: this must precede the reader assertion. If ring[0] were NOT
    // canonicalised, the reader holding a definite value would be correct behaviour, and
    // swapping these two lines would silently turn this into a staging test.
    QCOMPARE(ring.at(0)->outputValue(0), Status::Unknown);
    QVERIFY2(reader->outputValue(0) == Status::Unknown,
             "a reader of an oscillating node must read Unknown, not a stale definite value");
}

void TestFeedback::testOscillationDoesNotCanonicalizeUnrelatedFeedbackRegion()
{
    // Two independent feedback regions: a 3-inverter ring that oscillates, and a NAND SR latch
    // that settles perfectly well on its own (testSRLatchFromNANDConvergence is the control that
    // proves it settles WITHOUT the ring present). Canonicalising from a flat set of every
    // element in ANY cycle would drag the latch along with the ring.
    std::unique_ptr<Scene> scene(createSRLatchFromNAND());
    InputSwitch *switchS = nullptr;
    InputSwitch *switchR = nullptr;
    GraphicElement *nand1 = nullptr;
    GraphicElement *nand2 = nullptr;
    findSRLatchParts(scene.get(), switchS, switchR, nand1, nand2);
    QVERIFY(switchS && switchR && nand1 && nand2);

    // Both inputs low on an active-low NAND latch is stable and definite (both outputs high),
    // chosen so the latch needs nothing from the ring to settle.
    switchS->setOn(false);
    switchR->setOn(false);

    const auto ring = addInverterRing(scene.get(), 3, 400);

    Simulation sim(scene.get());
    sim.update();
    sim.update();

    const bool ringCanonicalised = std::any_of(ring.cbegin(), ring.cend(), [](const auto *elm) {
        return elm->outputValue(0) == Status::Unknown;
    });
    QVERIFY2(ringCanonicalised, "precondition: the inverter ring must have tripped the cap");

    QVERIFY2(nand1->outputValue(0) != Status::Unknown,
             "an unrelated, settled SR latch must keep its definite value");
    QVERIFY2(nand2->outputValue(0) != Status::Unknown,
             "an unrelated, settled SR latch must keep its definite value");
}

void TestFeedback::testTwoIndependentOscillatorsBothCanonicalize()
{
    // Termination and completeness together: canonicalising one region freezes it, the drain
    // continues, and the second region trips on its own. If the freeze were missing the first
    // region would regenerate same-timestamp events forever; if the drain stopped at the first
    // trip the second ring would never be canonicalised.
    std::unique_ptr<Scene> scene(new Scene());
    const auto ringA = addInverterRing(scene.get(), 3, 0);
    const auto ringB = addInverterRing(scene.get(), 5, 300);

    Simulation sim(scene.get());
    sim.update();
    sim.update();

    for (int i = 0; i < ringA.size(); ++i) {
        QVERIFY2(ringA.at(i)->outputValue(0) == Status::Unknown,
                 qPrintable(QString("ring A inverter %1 was not canonicalised").arg(i)));
    }
    for (int i = 0; i < ringB.size(); ++i) {
        QVERIFY2(ringB.at(i)->outputValue(0) == Status::Unknown,
                 qPrintable(QString("ring B inverter %1 was not canonicalised").arg(i)));
    }
}

void TestFeedback::testReconvergentFanInDoesNotTripCap()
{
    // The false-positive guard. A straight chain is useless here: each element evaluates about
    // once, so it never approaches a per-element cap however low the cap is set. Reconvergent
    // fan-in is the shape that matters -- every stage of a chain also feeds an accumulating
    // cascade, so the cascade's tail sees one arrival wave per stage and legitimately
    // re-evaluates many times within a single timestamp.
    std::unique_ptr<Scene> scene(new Scene());

    auto *source = ElementFactory::buildElement(ElementType::InputSwitch);
    scene->addItem(source);
    qobject_cast<InputSwitch *>(source)->setOn(true);

    const auto wire = [&scene](GraphicElement *from, int fromPort, GraphicElement *to, int toPort) {
        auto *conn = new Connection();
        scene->addItem(conn);
        conn->setStartPort(from->outputPort(fromPort));
        conn->setEndPort(to->inputPort(toPort));
    };

    constexpr int kStages = 40;
    GraphicElement *chain = source;
    GraphicElement *cascade = nullptr;
    for (int i = 0; i < kStages; ++i) {
        auto *inverter = ElementFactory::buildElement(ElementType::Not);
        scene->addItem(inverter);
        inverter->setPos((i + 1) * 40, 0);
        wire(chain, 0, inverter, 0);
        chain = inverter;

        auto *andGate = ElementFactory::buildElement(ElementType::And);
        scene->addItem(andGate);
        andGate->setPos((i + 1) * 40, 120);
        if (cascade) {
            wire(cascade, 0, andGate, 0);
        }
        wire(inverter, 0, andGate, 1); // arrives at depth i -- one wave per stage
        cascade = andGate;
    }
    QVERIFY(cascade);

    Simulation sim(scene.get());
    QSignalSpy warnings(&sim, &Simulation::simulationWarning);
    sim.update();

    QVERIFY2(warnings.isEmpty(),
             "reconvergent fan-in settles legitimately and must not be reported as non-convergent");
    QVERIFY2(cascade->outputValue(0) != Status::Unknown,
             "the cascade must settle to a definite value");
}

void TestFeedback::testOscillationDetectionCostIsNotQuadratic()
{
    // The property is a SHAPE, not a duration: with only the per-element cap, detecting a
    // non-convergent ring costs O(n^2) in the component size; with the absolute per-timestamp
    // ceiling it is linear. Asserted as the ratio between two ring sizes rather than as a
    // wall-clock bound on one of them.
    //
    // The absolute bound this replaces (1500 ms for the 801-ring) was calibrated on an
    // unloaded native build and failed under ASan at 3133 ms -- a sanitizer slows everything by
    // a constant factor, which a ratio cancels and a fixed bound does not. CI runs the
    // sanitizer presets, so that was a real gate failure rather than a flake.
    //
    // Measured as PROCESS CPU time, not wall time. ctest runs this suite in parallel, so a
    // wall-clock reading also counts the time this process spends descheduled while its
    // neighbours run -- and the two ring sizes are measured at different points in that
    // schedule, so the contention does NOT cancel in the ratio the way a sanitizer's constant
    // factor does. Measured against wall time the ratio reached 9.1x under load while passing
    // at 4.3x in isolation. CPU time accrues only while this process is actually running, which
    // is the quantity the property is about.
    const auto costOfRing = [this](const int inverters) -> qint64 {
        Scene scene;
        addInverterRing(&scene, inverters);
        auto *sim = scene.simulation();
        [&] { QVERIFY(sim->initialize()); }();
        QSignalSpy warnings(sim, &Simulation::simulationWarning);

        const std::clock_t started = std::clock();
        sim->update();
        const qint64 ms = static_cast<qint64>((std::clock() - started) * 1000 / CLOCKS_PER_SEC);

        [&] { QVERIFY2(warnings.count() >= 1, "an odd inverter ring must be detected as non-convergent"); }();
        return ms;
    };

    // Repeated so the baseline is large enough to divide by on a fast machine, where one
    // 201-ring can round to a millisecond or less.
    constexpr int kRepeats = 4;
    qint64 small = 0;
    for (int i = 0; i < kRepeats; ++i) {
        small += costOfRing(201);
    }
    qint64 large = 0;
    for (int i = 0; i < kRepeats; ++i) {
        large += costOfRing(801);
    }

    QVERIFY2(small >= kRepeats,
             qPrintable(QStringLiteral("precondition: the baseline (%1 ms over %2 runs) is too "
                                       "small to form a meaningful ratio").arg(small).arg(kRepeats)));

    // 4x the elements: linear predicts ~4x the cost, quadratic ~16x. 8x sits between them with
    // room on both sides for scheduling noise.
    QVERIFY2(large < small * 8,
             qPrintable(QStringLiteral("detection cost grew %1x for 4x the elements (%2 ms -> %3 ms "
                                       "over %4 runs each); linear predicts ~4x and quadratic ~16x, "
                                       "so the per-timestamp ceiling is not bounding the work")
                            .arg(static_cast<double>(large) / static_cast<double>(small), 0, 'f', 1)
                            .arg(small).arg(large).arg(kRepeats)));
}

void TestFeedback::testLatchOnlyFeedbackLoopTerminates()
{
    // DLatch with ~Q (output 1) wired back to D (input 0), Enable HIGH: a cycle whose only
    // member is a Memory element. In functional mode targetTime == m_currentTime, so without a
    // cap in the sampling region the drain could never leave this timestamp.
    Scene scene;
    auto *latch = ElementFactory::buildElement(ElementType::DLatch);
    auto *en = ElementFactory::buildElement(ElementType::InputSwitch);
    scene.addItem(latch);
    scene.addItem(en);
    latch->setPos(100, 0);
    en->setPos(0, 0);

    auto *feedback = new Connection();
    scene.addItem(feedback);
    feedback->setStartPort(latch->outputPort(1));
    feedback->setEndPort(latch->inputPort(0));

    auto *enable = new Connection();
    scene.addItem(enable);
    enable->setStartPort(en->outputPort(0));
    enable->setEndPort(latch->inputPort(1));

    qobject_cast<GraphicElementInput *>(en)->setOn(true, 0);

    auto *sim = scene.simulation();
    QVERIFY(sim->initialize());
    QSignalSpy warnings(sim, &Simulation::simulationWarning);

    QElapsedTimer timer;
    timer.start();
    sim->update();
    QVERIFY2(timer.elapsed() < 5000, "the drain must terminate on a latch-only feedback loop");

    QCOMPARE(warnings.count(), 1);
    QCOMPARE(latch->outputValue(0), Status::Unknown);
    QCOMPARE(latch->outputValue(1), Status::Unknown);
}

void TestFeedback::testCrossCoupledLatchLoopTerminates()
{
    // Two latches feeding each other, so the fix cannot be special-cased to self-edges.
    Scene scene;
    auto *l1 = ElementFactory::buildElement(ElementType::DLatch);
    auto *l2 = ElementFactory::buildElement(ElementType::DLatch);
    auto *en = ElementFactory::buildElement(ElementType::InputSwitch);
    scene.addItem(l1); scene.addItem(l2); scene.addItem(en);
    en->setPos(0, 0); l1->setPos(100, 0); l2->setPos(220, 0);

    const auto wire = [&scene](GraphicElement *a, int ap, GraphicElement *b, int bp) {
        auto *c = new Connection();
        scene.addItem(c);
        c->setStartPort(a->outputPort(ap));
        c->setEndPort(b->inputPort(bp));
    };
    wire(l1, 1, l2, 0);   // ~Q1 -> D2  (inverting, so the pair cannot settle)
    wire(l2, 0, l1, 0);   //  Q2 -> D1
    wire(en, 0, l1, 1);
    wire(en, 0, l2, 1);
    qobject_cast<GraphicElementInput *>(en)->setOn(true, 0);

    auto *sim = scene.simulation();
    QVERIFY(sim->initialize());
    QSignalSpy warnings(sim, &Simulation::simulationWarning);

    QElapsedTimer timer;
    timer.start();
    sim->update();
    QVERIFY2(timer.elapsed() < 5000, "the drain must terminate on a cross-coupled latch loop");
    QCOMPARE(warnings.count(), 1);
    QCOMPARE(l1->outputValue(0), Status::Unknown);
    QCOMPARE(l2->outputValue(0), Status::Unknown);
}

namespace {

/// Builds an EN-gated 3-gate oscillating ring feeding a downstream reader, plus a settled
/// combinational path that shares nothing with it. Returns the elements worth reading. The
/// oscillating half is what makes these properties bite: a settled circuit would agree with
/// itself under almost any bug in the evaluation caps.
struct RingFixture {
    InputSwitch *enable = nullptr;
    GraphicElement *nand = nullptr;
    GraphicElement *ringReader = nullptr;
    InputSwitch *independentIn = nullptr;
    GraphicElement *independentOut = nullptr;
};

RingFixture buildRingFixture(CircuitBuilder &builder)
{
    RingFixture f;
    f.enable = new InputSwitch();
    f.nand = ElementFactory::buildElement(ElementType::Nand);
    auto *inv1 = ElementFactory::buildElement(ElementType::Not);
    auto *inv2 = ElementFactory::buildElement(ElementType::Not);
    f.ringReader = ElementFactory::buildElement(ElementType::Not);
    f.independentIn = new InputSwitch();
    f.independentOut = ElementFactory::buildElement(ElementType::Not);

    builder.add(f.enable, f.nand, inv1, inv2, f.ringReader, f.independentIn, f.independentOut);
    builder.connect(f.enable, 0, f.nand, 0);
    builder.connect(f.nand, 0, inv1, 0);
    builder.connect(inv1, 0, inv2, 0);
    builder.connect(inv2, 0, f.nand, 1);
    builder.connect(f.nand, 0, f.ringReader, 0);
    builder.connect(f.independentIn, 0, f.independentOut, 0);
    return f;
}

/// The four readings these properties compare.
QVector<Status> readings(const RingFixture &f)
{
    return {f.nand->outputValue(0), f.ringReader->outputValue(0),
            f.independentOut->outputValue(0),
            static_cast<GraphicElement *>(f.enable)->outputValue(0)};
}

} // namespace

void TestFeedback::testPropertyDisconnectedElementChangesNoReading()
{
    // LOCALITY, which is the entire justification for the evaluation cap being per-element
    // rather than one global budget: an element outside another's cone must not be able to move
    // its reading. The cap is derived from the SCC condensation precisely so that adding a gate
    // nobody is wired to cannot change when a region is declared non-convergent.
    //
    // Its discriminating power is narrower than the name suggests, and that bound is measured
    // rather than assumed: replacing the cap with one proportional to the total element count
    // leaves this test green, because an oscillating region reads Unknown whether it trips early
    // or late. What it does catch is a locality break that changes a VALUE. No circuit whose
    // settled value depends on the cap has been found -- a reconvergent chain nine stages deep
    // still settles well inside it -- so this guards the value, not the trip instant.
    WorkSpace bare;
    CircuitBuilder bareBuilder(bare.scene());
    const RingFixture a = buildRingFixture(bareBuilder);
    Simulation *simA = bareBuilder.initSimulation();
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*did not converge.*"));
    a.enable->setOn(true);
    a.independentIn->setOn(true);
    simA->update();

    WorkSpace padded;
    CircuitBuilder paddedBuilder(padded.scene());
    const RingFixture b = buildRingFixture(paddedBuilder);
    // Wired to nothing at all, in either direction.
    for (int i = 0; i < 20; ++i) {
        auto *stray = ElementFactory::buildElement(ElementType::And);
        QVERIFY(stray);
        paddedBuilder.add(stray);
    }
    Simulation *simB = paddedBuilder.initSimulation();
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*did not converge.*"));
    b.enable->setOn(true);
    b.independentIn->setOn(true);
    simB->update();

    QCOMPARE(readings(b), readings(a));
}

void TestFeedback::testPropertyIdenticalCircuitsSettleIdentically()
{
    // DETERMINISM across instances: two separately-built copies of the same circuit, driven the
    // same way, must read the same. The drain pops events in (time, kind, priority) order and
    // resolves ties by a topological priority, so a comparator that depended on anything
    // incidental -- allocation addresses, insertion order -- would show up here and nowhere
    // else, since every other test reads a single simulation.
    WorkSpace one;
    CircuitBuilder oneBuilder(one.scene());
    const RingFixture a = buildRingFixture(oneBuilder);
    Simulation *simA = oneBuilder.initSimulation();

    WorkSpace two;
    CircuitBuilder twoBuilder(two.scene());
    const RingFixture b = buildRingFixture(twoBuilder);
    Simulation *simB = twoBuilder.initSimulation();

    // One expectation per simulation, not per iteration: the warning is emitted at most once
    // per initialize() (m_convergenceWarned), so a per-iteration expectation would go unmet and
    // fail the test for the wrong reason.
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*did not converge.*"));
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*did not converge.*"));

    for (const bool level : {true, false, true}) {
        a.enable->setOn(level);
        a.independentIn->setOn(level);
        simA->update();

        b.enable->setOn(level);
        b.independentIn->setOn(level);
        simB->update();

        QCOMPARE(readings(b), readings(a));
    }
}

void TestFeedback::testPropertyRestartReproducesTheSameSettledState()
{
    // DETERMINISM across a rebuild: restart() drops the whole cached topology, and the next
    // tick rebuilds priorities, feedback components and evaluation caps from scratch. Those
    // derived values decide which element trips first in an oscillating region, so a rebuild
    // that produced a different order would settle the same circuit differently -- with the
    // same inputs, in the same session.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    const RingFixture f = buildRingFixture(builder);
    Simulation *sim = builder.initSimulation();

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*did not converge.*"));
    f.enable->setOn(true);
    f.independentIn->setOn(true);
    sim->update();
    const QVector<Status> before = readings(f);

    sim->restart();
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*did not converge.*"));
    sim->update();

    QCOMPARE(readings(f), before);
}

void TestFeedback::testInverterRingTerminatesAndCanonicalizes_data()
{
    // Ring length is the parameter, and odd lengths are the oscillating ones. Length 1 is the
    // self-loop, whose SCC is a single node -- a different branch of findFeedbackComponents()
    // from the multi-node case, and the one where "component size" is smallest and so the
    // evaluation cap tightest.
    QTest::addColumn<int>("inverters");
    QTest::newRow("1 (self-loop)") << 1;
    QTest::newRow("3")             << 3;
    QTest::newRow("5")             << 5;
    QTest::newRow("7")             << 7;
    QTest::newRow("51")            << 51;
}

void TestFeedback::testInverterRingTerminatesAndCanonicalizes()
{
    // Termination is asserted against the wall clock rather than inferred from the test
    // returning: a drain that fails to terminate hangs the whole run, and a bound turns that
    // into a named failure. The value is asserted too -- terminating with a wrong answer is not
    // what this is meant to pin.
    QFETCH(int, inverters);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    QVector<GraphicElement *> ring;
    for (int i = 0; i < inverters; ++i) {
        auto *inv = ElementFactory::buildElement(ElementType::Not);
        QVERIFY(inv);
        ring.append(inv);
        builder.add(inv);
    }
    for (int i = 0; i < inverters; ++i) {
        builder.connect(ring.at(i), 0, ring.at((i + 1) % inverters), 0);
    }

    Simulation *sim = builder.initSimulation();
    for (auto *inv : std::as_const(ring)) {
        QVERIFY2(sim->isInFeedbackLoop(inv), "precondition: every ring member must be in the cycle");
    }

    QElapsedTimer timer;
    timer.start();
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*did not converge.*"));
    sim->update();
    QVERIFY2(timer.elapsed() < 5000,
             qPrintable(QString("a %1-inverter ring did not terminate: %2 ms").arg(inverters).arg(timer.elapsed())));

    for (auto *inv : std::as_const(ring)) {
        QCOMPARE(inv->outputValue(0), Status::Unknown);
    }

    // A second tick must not undo it: Unknown is a fixed point of NOT, so a ring left this way
    // stays this way, and a tick that re-derived definite values would mean the freeze had not
    // actually taken.
    sim->update();
    for (auto *inv : std::as_const(ring)) {
        QCOMPARE(inv->outputValue(0), Status::Unknown);
    }
}

void TestFeedback::testRingCrossingTheAbsoluteCeilingStillCanonicalizes()
{
    // The ABSOLUTE per-timestamp ceiling (kMaxEvalsPerTimestampPerElement * elementCount) is a
    // different trip path from the per-element cap, and nothing exercised it. A ring of R
    // inverters costs about 4R^2 evaluations before any member reaches its own cap of ~4R,
    // while the ceiling is 256 * elementCount -- so the ceiling is crossed FIRST once R passes
    // roughly 64. At 101 it is crossed comfortably.
    //
    // Two things have to hold. The region must still be canonicalised (the ceiling trip feeds
    // canonicalizeOscillation() whichever element was mid-evaluation, and that element being a
    // ring member rather than the reader is what makes it work). And because the trip abandons
    // the timestamp with events still queued, the drain must NOT claim a fixed point -- the
    // reader downstream is still stale at that instant and only converges on the next tick.
    // This is the circuit where that invariant is load-bearing rather than defensive.
    constexpr int kRingSize = 101; // odd, so it genuinely oscillates
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    QVector<GraphicElement *> ring;
    for (int i = 0; i < kRingSize; ++i) {
        auto *inv = ElementFactory::buildElement(ElementType::Not);
        QVERIFY(inv);
        ring.append(inv);
        builder.add(inv);
    }
    for (int i = 0; i < kRingSize; ++i) {
        builder.connect(ring.at(i), 0, ring.at((i + 1) % kRingSize), 0);
    }
    auto *reader = ElementFactory::buildElement(ElementType::Not);
    QVERIFY(reader);
    builder.add(reader);
    builder.connect(ring.at(0), 0, reader, 0);

    Simulation *sim = builder.initSimulation();

    QElapsedTimer timer;
    timer.start();
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*did not converge.*"));
    sim->update();
    QVERIFY2(timer.elapsed() < 5000,
             qPrintable(QString("a %1-inverter ring did not terminate: %2 ms").arg(kRingSize).arg(timer.elapsed())));

    for (auto *inv : std::as_const(ring)) {
        QCOMPARE(inv->outputValue(0), Status::Unknown);
    }
    QVERIFY2(!sim->isAtFixedPoint(),
             "precondition: the ceiling trip abandons the timestamp with events still queued, so "
             "the drain must not report a fixed point -- if it did, the idle skip would strand "
             "the reader below at its stale value forever");

    // The next tick drains what the abandoned timestamp left behind.
    sim->update();
    QCOMPARE(reader->outputValue(0), Status::Unknown);
    QVERIFY(sim->isAtFixedPoint());
}

void TestFeedback::testOscillatorWithADownstreamConsumerStillCanonicalizes()
{
    // An oscillating region that BOTH has an upstream driver and feeds a downstream consumer.
    // The consumer is re-evaluated once per rotation of the ring, so it exhausts its own
    // evaluation cap. If computeEvaluationCaps() credited an acyclic element nothing for the
    // settling rounds of the component upstream of it, that cap would be SMALLER than the ring
    // members' despite sitting further downstream (24 against 32). The trip would then arrive on
    // an element in no cyclic component, which canonicalizeOscillation() correctly refuses to
    // freeze, abandoning the timestamp with nothing canonicalised -- leaving the region holding
    // a definite-looking but INCONSISTENT state (NAND=0 feeding INV1=0, i.e. NOT(0) = 0) that
    // every later tick reproduces.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch enable;
    enable.setLabel("EN");
    auto *nand = ElementFactory::buildElement(ElementType::Nand);
    auto *inv1 = ElementFactory::buildElement(ElementType::Not);
    auto *inv2 = ElementFactory::buildElement(ElementType::Not);
    QVERIFY(nand && inv1 && inv2);
    Led sink;

    builder.add(&enable, nand, inv1, inv2, &sink);
    builder.connect(&enable, 0, nand, 0);
    builder.connect(nand, 0, inv1, 0);
    builder.connect(inv1, 0, inv2, 0);
    builder.connect(inv2, 0, nand, 1);   // closes the ring: NAND is its only inversion, plus two
    builder.connect(nand, 0, &sink, 0);  // the downstream consumer

    Simulation *sim = builder.initSimulation();
    QVERIFY2(sim->isInFeedbackLoop(nand) && sim->isInFeedbackLoop(inv1) && sim->isInFeedbackLoop(inv2),
             "precondition: all three ring members must be seen as one cyclic component");

    QElapsedTimer timer;
    timer.start();
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*did not converge.*"));
    enable.setOn(true);
    sim->update();
    QVERIFY2(timer.elapsed() < 5000,
             qPrintable(QString("the drain did not terminate: %1 ms").arg(timer.elapsed())));

    // Every member of the oscillating region must read Unknown...
    QCOMPARE(nand->outputValue(0), Status::Unknown);
    QCOMPARE(inv1->outputValue(0), Status::Unknown);
    QCOMPARE(inv2->outputValue(0), Status::Unknown);

    // ...and the state left behind must at least be SELF-CONSISTENT, which is the property the
    // defect actually broke. Stated as an invariant rather than as three expected values,
    // because "an inverter whose output is not the negation of its input" is the wrong thing
    // regardless of which values it happens to hold.
    QVERIFY2(inv1->outputValue(0) == StatusOps::statusNot(nand->outputValue(0)),
             "the first inverter's output must be the negation of what it reads");
    QVERIFY2(inv2->outputValue(0) == StatusOps::statusNot(inv1->outputValue(0)),
             "the second inverter's output must be the negation of what it reads");
}

void TestFeedback::testOscillatorInsideAnIcCanonicalizesAndRecovers()
{
    // Canonicalisation inside an IC is an untested path. ICs are FLATTENED into the top-level
    // netlist (Simulation::initialize()), so an oscillating ring built from an IC's internal
    // primitives has to be found, frozen and recovered exactly like one drawn on the canvas.
    // The flattening is what makes that non-obvious: the region that trips is made of elements
    // the top-level scene does not contain.
    QTemporaryDir subDir;
    QVERIFY(subDir.isValid());

    // Sub-circuit: EN -> NAND -> NOT -> NOT -> back into the NAND, with the NAND's output also
    // driving the IC's output. EN high makes the NAND an inverter, so the ring has three
    // inversions and oscillates; EN low pins the NAND's output to 1 and it settles.
    WorkSpace subWorkspace;
    CircuitBuilder subBuilder(subWorkspace.scene());
    InputSwitch enable;
    enable.setLabel("EN");
    auto *nand = ElementFactory::buildElement(ElementType::Nand);
    auto *inv1 = ElementFactory::buildElement(ElementType::Not);
    auto *inv2 = ElementFactory::buildElement(ElementType::Not);
    Led out;
    out.setLabel("OUT");
    QVERIFY(nand && inv1 && inv2);
    nand->setLabel("NAND"); inv1->setLabel("INV1"); inv2->setLabel("INV2");
    subBuilder.add(&enable, nand, inv1, inv2, &out);
    subBuilder.connect(&enable, 0, nand, 0);
    subBuilder.connect(nand, 0, inv1, 0);
    subBuilder.connect(inv1, 0, inv2, 0);
    subBuilder.connect(inv2, 0, nand, 1);   // closes the ring
    subBuilder.connect(nand, 0, &out, 0);

    const QString subPath = subDir.path() + "/ic_ring_oscillator.panda";
    QCOMPARE(subWorkspace.save(subPath), WorkSpace::SaveOutcome::Saved);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    InputSwitch topEnable;
    auto *ic = new IC();
    Led topOut;
    builder.add(&topEnable, ic, &topOut);
    ic->loadFile(subPath, subDir.path());
    QVERIFY2(ic->inputSize() == 1 && ic->outputSize() == 1, "the sub-circuit must expose EN and OUT");
    builder.connect(&topEnable, 0, ic, 0);
    builder.connect(ic, 0, &topOut, 0);

    Simulation *sim = builder.initSimulation();

    QElapsedTimer timer;
    timer.start();
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*did not converge.*"));
    topEnable.setOn(true);
    sim->update();
    QVERIFY2(timer.elapsed() < 5000,
             qPrintable(QString("an oscillator inside an IC did not terminate: %1 ms").arg(timer.elapsed())));
    QCOMPARE(ic->outputValue(0), Status::Unknown);

    // Recovery: disabling the ring must give a definite value again. NOT(Unknown) is itself a
    // fixed point, so a region frozen at Unknown only recovers because a DOMINANT input --
    // NAND(0, x) = 1 -- re-enters it; asserting this is what distinguishes a canonicalisation
    // that can be left from one that wedges the circuit for the rest of the session.
    topEnable.setOn(false);
    sim->update();
    QVERIFY2(ic->outputValue(0) != Status::Unknown,
             "a canonicalised region inside an IC must recover once its oscillation is disabled");
    QCOMPARE(ic->outputValue(0), Status::Active); // NAND(0, x) = 1
}

void TestFeedback::testOscillatorAcrossAWirelessPairCanonicalizesAndRecovers()
{
    // The other untested canonicalisation path: a cycle that closes THROUGH THE AIR. The
    // wireless edge exists only in the successor graph sortSimElements() builds (step 3) --
    // there is no Connection object joining the pair -- so an oscillating region spanning it
    // has to be assembled from an edge no wire carries. testWirelessOverrideDoesNotFabricate-
    // FeedbackLoop covers the opposite case, where a physical wire must NOT create a loop.
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch enable;
    auto *nand = ElementFactory::buildElement(ElementType::Nand);
    auto *tx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    auto *rx = qobject_cast<Node *>(ElementFactory::buildElement(ElementType::Node));
    QVERIFY(nand && tx && rx);

    tx->setLabel("CH");
    tx->setWirelessMode(WirelessMode::Tx);
    rx->setLabel("CH");
    rx->setWirelessMode(WirelessMode::Rx);

    builder.add(&enable, nand, tx, rx);
    builder.connect(&enable, 0, nand, 0);
    builder.connect(nand, 0, tx, 0);   // into the transmitter
    builder.connect(rx, 0, nand, 1);   // out of the receiver, closing the ring over the air
    // Nodes do not invert, so the NAND is the ring's only inversion -- an odd count, which is
    // what makes it oscillate rather than settle. (An inverter between rx and the NAND would
    // make it two, and the ring would sit happily at a fixed point.)

    Simulation *sim = builder.initSimulation();
    QVERIFY2(sim->isInFeedbackLoop(rx),
             "precondition: the wireless edge must put the receiver in the cycle -- without it "
             "this circuit is acyclic and the rest of the test proves nothing");
    QVERIFY(sim->isInFeedbackLoop(tx));

    QElapsedTimer timer;
    timer.start();
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(".*did not converge.*"));
    enable.setOn(true);
    sim->update();
    QVERIFY2(timer.elapsed() < 5000,
             qPrintable(QString("an oscillator across a wireless pair did not terminate: %1 ms").arg(timer.elapsed())));
    QCOMPARE(nand->outputValue(0), Status::Unknown);
    QCOMPARE(rx->outputValue(0), Status::Unknown);

    enable.setOn(false);
    sim->update();
    QVERIFY2(nand->outputValue(0) != Status::Unknown,
             "a canonicalised region spanning a wireless pair must recover once disabled");
    QCOMPARE(nand->outputValue(0), Status::Active); // NAND(0, x) = 1
}

void TestFeedback::testFlipFlopKeepsStateWhenCombinationalRegionTrips()
{
    // The other half of the rule: when a COMBINATIONAL member trips, Memory members of that
    // component keep their stored state. A gated ring is enabled on the same tick as a
    // flip-flop clock edge, so the trip and the capture land in one drain.
    Scene scene;
    auto *en = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *gate = ElementFactory::buildElement(ElementType::Nand);
    auto *i1 = ElementFactory::buildElement(ElementType::Not);
    auto *i2 = ElementFactory::buildElement(ElementType::Not);
    auto *din = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *clk = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *ff = ElementFactory::buildElement(ElementType::DFlipFlop);
    for (auto *e : QVector<GraphicElement *>{en, gate, i1, i2, din, clk, ff}) { scene.addItem(e); }
    en->setPos(0, 0); gate->setPos(100, 0); i1->setPos(200, 0); i2->setPos(300, 0);
    din->setPos(0, 200); clk->setPos(0, 260); ff->setPos(150, 200);

    const auto wire = [&scene](GraphicElement *a, int ap, GraphicElement *b, int bp) {
        auto *c = new Connection();
        scene.addItem(c);
        c->setStartPort(a->outputPort(ap));
        c->setEndPort(b->inputPort(bp));
    };
    wire(en, 0, gate, 0);
    wire(gate, 0, i1, 0);
    wire(i1, 0, i2, 0);
    wire(i2, 0, gate, 1);
    wire(din, 0, ff, 0);
    wire(clk, 0, ff, 1);

    auto *enIn = qobject_cast<GraphicElementInput *>(en);
    auto *dinIn = qobject_cast<GraphicElementInput *>(din);
    auto *clkIn = qobject_cast<GraphicElementInput *>(clk);

    auto *sim = scene.simulation();
    QVERIFY(sim->initialize());
    enIn->setOn(false, 0);
    dinIn->setOn(true, 0);
    clkIn->setOn(false, 0);
    sim->update();

    enIn->setOn(true, 0);     // start the ring oscillating
    clkIn->setOn(true, 0);    // and clock the flip-flop, same tick
    sim->update();

    QVERIFY2(ff->outputValue(0) == Status::Active,
             "a flip-flop clocked on the tick an unrelated region trips must still capture, "
             "not be canonicalised to Unknown along with it");
}

void TestFeedback::testSampledFlipFlopReachesDownstreamWhenOscillationTrips()
{
    // Gated ring enabled on the same tick as a flip-flop clock edge, so a cap trip and a
    // deferred sample land in one drain, plus a NOT gate reading Q.
    Scene scene;
    auto *en = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *gate = ElementFactory::buildElement(ElementType::Nand);
    auto *i1 = ElementFactory::buildElement(ElementType::Not);
    auto *i2 = ElementFactory::buildElement(ElementType::Not);
    auto *din = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *clk = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *ff = ElementFactory::buildElement(ElementType::DFlipFlop);
    auto *reader = ElementFactory::buildElement(ElementType::Not);
    for (auto *e : QVector<GraphicElement *>{en, gate, i1, i2, din, clk, ff, reader}) {
        scene.addItem(e);
    }
    en->setPos(0, 0); gate->setPos(100, 0); i1->setPos(200, 0); i2->setPos(300, 0);
    din->setPos(0, 200); clk->setPos(0, 260); ff->setPos(150, 200); reader->setPos(320, 200);

    const auto wire = [&scene](GraphicElement *a, int ap, GraphicElement *b, int bp) {
        auto *c = new Connection();
        scene.addItem(c);
        c->setStartPort(a->outputPort(ap));
        c->setEndPort(b->inputPort(bp));
    };
    wire(en, 0, gate, 0);
    wire(gate, 0, i1, 0);
    wire(i1, 0, i2, 0);
    wire(i2, 0, gate, 1);
    wire(din, 0, ff, 0);
    wire(clk, 0, ff, 1);
    wire(ff, 0, reader, 0);

    auto *enIn = qobject_cast<GraphicElementInput *>(en);
    auto *dinIn = qobject_cast<GraphicElementInput *>(din);
    auto *clkIn = qobject_cast<GraphicElementInput *>(clk);

    auto *sim = scene.simulation();
    QVERIFY(sim->initialize());
    QSignalSpy warnings(sim, &Simulation::simulationWarning);

    enIn->setOn(false, 0);
    dinIn->setOn(true, 0);
    clkIn->setOn(false, 0);
    sim->update();
    QCOMPARE(ff->outputValue(0), Status::Inactive);
    QCOMPARE(reader->outputValue(0), Status::Active);   // NOT(0)

    enIn->setOn(true, 0);    // start the ring -> trips the cap
    clkIn->setOn(true, 0);   // and clock the flip-flop, same tick
    sim->update();

    QVERIFY2(warnings.count() >= 1, "precondition: the ring must actually trip this tick");
    QCOMPARE(ff->outputValue(0), Status::Active);
    QVERIFY2(reader->outputValue(0) == Status::Inactive,
             "the flip-flop captured, so its reader must see the new value: the trip path has to "
             "publish deferred samples, not commit them silently");
}
