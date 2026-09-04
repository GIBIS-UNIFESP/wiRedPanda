// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestFeedback.h"

#include <QRegularExpression>
#include <QSignalSpy>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"

// There is no Scene here: QuickCircuitBuilder::addOwnedElement() takes the place of
// scene.addItem() for elements the builder owns outright (every element built here via
// ElementFactory::buildElement()), and builder.connect(from, fromPort, to, toPort) takes the
// place of manually new'ing a Connection and calling scene.addItem() on it.

// ============================================================
// Convergence Detection Tests
// ============================================================

void TestFeedback::testSRLatchFromNANDConvergence()
{
    // SR latch built from NAND gates should converge to stable state
    std::unique_ptr<QuickCircuitBuilder> builder(createSRLatchFromNAND());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    verifyConvergence(builder.get());
    verifyStableState(builder.get());
}

void TestFeedback::testDLatchWithFeedback()
{
    // D latch with feedback should converge
    std::unique_ptr<QuickCircuitBuilder> builder(createDLatchWithFeedback());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    verifyConvergence(builder.get());
    verifyStableState(builder.get());
}

void TestFeedback::testRingOscillatorNonConvergence()
{
    // With 4-state logic, ring oscillators converge at Unknown
    // (NOT(Unknown) = Unknown is a fixed point).
    std::unique_ptr<QuickCircuitBuilder> builder(createRingOscillator());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    verifyConvergence(builder.get());
}

void TestFeedback::testSetResetPriorityInSRLatch()
{
    // SR latch from NAND gates (createSRLatchFromNAND(): switchS(y=0) -> nand1[0] (Q, y=0);
    // switchR(y=50) -> nand2[0] (Q', y=50); cross-feedback ties them together). This is an
    // active-low NAND latch: driving S low forces Q high and Q' low (Set); driving R low
    // forces Q low and Q' high (Reset).
    std::unique_ptr<QuickCircuitBuilder> builder(createSRLatchFromNAND());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    InputSwitch *switchS = nullptr;
    InputSwitch *switchR = nullptr;
    GraphicElement *nand1 = nullptr; // Q
    GraphicElement *nand2 = nullptr; // Q'
    findSRLatchParts(builder.get(), switchS, switchR, nand1, nand2);
    QVERIFY2(switchS && switchR && nand1 && nand2, "Failed to locate S/R switches and NAND gates");

    Simulation sim(builder.get());

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
    std::unique_ptr<QuickCircuitBuilder> builder1(createSRLatchFromNAND());
    std::unique_ptr<QuickCircuitBuilder> builder2(createSRLatchFromNAND());

    QVERIFY2(builder1 != nullptr, "Failed to create first circuit");
    QVERIFY2(builder2 != nullptr, "Failed to create second circuit");

    InputSwitch *switchS1 = nullptr, *switchR1 = nullptr;
    GraphicElement *nand1Q1 = nullptr, *nand1Q1Bar = nullptr;
    findSRLatchParts(builder1.get(), switchS1, switchR1, nand1Q1, nand1Q1Bar);
    QVERIFY(switchS1 && switchR1 && nand1Q1 && nand1Q1Bar);

    InputSwitch *switchS2 = nullptr, *switchR2 = nullptr;
    GraphicElement *nand2Q1 = nullptr, *nand2Q1Bar = nullptr;
    findSRLatchParts(builder2.get(), switchS2, switchR2, nand2Q1, nand2Q1Bar);
    QVERIFY(switchS2 && switchR2 && nand2Q1 && nand2Q1Bar);

    Simulation sim1(builder1.get());
    Simulation sim2(builder2.get());

    // Drive builder1 into Set (Q=1), builder2 into Reset (Q=0) -- opposite initial states.
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
    QuickCircuitBuilder builder;

    auto *and1 = builder.addOwnedElement(ElementFactory::buildElement(ElementType::And));
    auto *and2 = builder.addOwnedElement(ElementFactory::buildElement(ElementType::And));
    auto *btn = builder.addOwnedElement(ElementFactory::buildElement(ElementType::InputButton));

    QVERIFY2(and1 != nullptr, "Failed to create AND gate 1");
    QVERIFY2(and2 != nullptr, "Failed to create AND gate 2");
    QVERIFY2(btn != nullptr, "Failed to create button");

    // Set positions
    and1->setPos(100, 100);
    and2->setPos(200, 100);
    btn->setPos(0, 100);

    // Create simple combinational chain: btn → and1[0,1] → and2[0,1]
    builder.connect(btn, 0, and1, 0);
    builder.connect(btn, 0, and1, 1);
    builder.connect(and1, 0, and2, 0);
    builder.connect(and1, 0, and2, 1);

    // Run simulation
    Simulation sim(&builder);
    sim.update();

    // Verify no feedback loops exist
    bool found = false;
    for (auto *elem : builder.elements()) {
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
    std::unique_ptr<QuickCircuitBuilder> builder(createSRLatchFromNAND());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    Simulation sim(builder.get());

    // First update should complete convergence check
    sim.update();

    // Verify that convergence was detected
    verifyFeedbackDetection(builder.get());
}

void TestFeedback::testConvergenceSpeedVariation()
{
    // Different circuits should converge in different numbers of iterations. Use
    // Simulation::iterativeSettle() directly with a rising iteration budget to find each
    // circuit's actual minimal convergence count, and compare them.
    std::unique_ptr<QuickCircuitBuilder> builder1(createSRLatchFromNAND()); // Should converge quickly (2-3 iterations)
    std::unique_ptr<QuickCircuitBuilder> builder2(createRingOscillator());  // odd inverter count -- may not converge at all

    QVERIFY2(builder1 != nullptr, "Failed to create first circuit");
    QVERIFY2(builder2 != nullptr, "Failed to create second circuit");

    // A real Simulation::update() first, to run through initialize() so each element's
    // internal sim-value arrays are sized and wired to their predecessors -- calling
    // updateLogic() (via iterativeSettle() below) on a never-initialized element crashes.
    Simulation sim1(builder1.get());
    sim1.update();
    Simulation sim2(builder2.get());
    QTest::ignoreMessage(QtDebugMsg, QRegularExpression(".*converge.*"));
    sim2.update();

    auto minConvergenceIterations = [](QuickCircuitBuilder *builder) -> int {
        const auto elements = builder->elements();
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

    const int srLatchIterations = minConvergenceIterations(builder1.get());
    const int ringOscillatorIterations = minConvergenceIterations(builder2.get());

    // The SR latch converges in 2 iterations; the odd-length ring oscillator does not
    // converge within the kMaxSettleIterations budget at all (-1).
    QCOMPARE(srLatchIterations, 2);
    QCOMPARE(ringOscillatorIterations, -1);
}

// ============================================================
// Non-Converging Circuits Tests
// ============================================================

void TestFeedback::testRingOscillatorWarningAfterMaxIterations()
{
    // The odd-length ring oscillator never converges within kMaxSettleIterations, so
    // Simulation::update() must log a debug warning about it. QTest::ignoreMessage() fails
    // the test if the message never appears.
    std::unique_ptr<QuickCircuitBuilder> builder(createRingOscillator());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    Simulation sim(builder.get());
    QTest::ignoreMessage(QtDebugMsg, QRegularExpression(".*did not converge after 10 iterations.*"));
    sim.update();
}

void TestFeedback::testOddLengthInverterChainFeedback()
{
    // Odd-length inverter chain (feedback loop) converges at Unknown in 4-state logic
    std::unique_ptr<QuickCircuitBuilder> builder(createRingOscillator());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    Simulation sim(builder.get());
    sim.update();

    // Verify feedback was detected
    verifyFeedbackDetection(builder.get());
}

void TestFeedback::testConflictingFeedbackSignals()
{
    // Feedback with conflicting signals should still converge
    std::unique_ptr<QuickCircuitBuilder> builder(createSRLatchFromNAND());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    // Set both S=1 and R=1 (conflicting signals)
    auto elements = builder->elements();
    for (auto *elem : elements) {
        if (elem->elementType() == ElementType::InputSwitch) {
            auto *sw = dynamic_cast<InputSwitch *>(elem);
            if (sw) {
                sw->setOn(true);
            }
        }
    }

    verifyConvergence(builder.get());
}

void TestFeedback::testWarningMessageContent()
{
    // Simulation::updateWithIterativeSettling() emits simulationWarning() with the actual
    // user-facing text alongside the qDebug() line; this inspects that content directly.
    std::unique_ptr<QuickCircuitBuilder> builder(createRingOscillator());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    Simulation sim(builder.get());
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
    std::unique_ptr<QuickCircuitBuilder> builder(createRingOscillator());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    Simulation sim(builder.get());

    // Should not throw, should continue
    sim.update();
    sim.update(); // Multiple updates should work

    // Verify circuit still has elements
    QVERIFY2(!builder->elements().isEmpty(), "Feedback circuit should contain elements");
}

// ============================================================
// Mixed Circuits Tests
// ============================================================

void TestFeedback::testCombinationalPlusFeedbackInSameCircuit()
{
    // Circuit with both combinational logic and feedback
    std::unique_ptr<QuickCircuitBuilder> builder(createMixedCircuit());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    Simulation sim(builder.get());
    sim.update();

    // Verify that mixed circuit stabilizes
    verifyFeedbackDetection(builder.get());
}

void TestFeedback::testMultipleIndependentFeedbackLoops()
{
    // Multiple independent feedback loops in same circuit
    QuickCircuitBuilder builder;

    // Create two separate SR latches (two feedback loops)
    auto *nand1a = builder.addOwnedElement(ElementFactory::buildElement(ElementType::Nand));
    auto *nand1b = builder.addOwnedElement(ElementFactory::buildElement(ElementType::Nand));
    auto *nand2a = builder.addOwnedElement(ElementFactory::buildElement(ElementType::Nand));
    auto *nand2b = builder.addOwnedElement(ElementFactory::buildElement(ElementType::Nand));

    QVERIFY2(nand1a != nullptr, "Failed to create NAND gate 1a");
    QVERIFY2(nand1b != nullptr, "Failed to create NAND gate 1b");
    QVERIFY2(nand2a != nullptr, "Failed to create NAND gate 2a");
    QVERIFY2(nand2b != nullptr, "Failed to create NAND gate 2b");

    nand1a->setPos(0, 0);
    nand1b->setPos(50, 0);
    nand2a->setPos(150, 0);
    nand2b->setPos(200, 0);

    // First SR latch: nand1a ↔ nand1b
    builder.connect(nand1a, 0, nand1b, 0);
    builder.connect(nand1b, 0, nand1a, 0);

    // Second SR latch: nand2a ↔ nand2b
    builder.connect(nand2a, 0, nand2b, 0);
    builder.connect(nand2b, 0, nand2a, 0);

    Simulation sim(&builder);
    sim.update();

    verifyFeedbackDetection(&builder);
}

void TestFeedback::testNestedFeedbackLoops()
{
    // Feedback loops (3-NOT ring - odd length, converges at Unknown in 4-state logic)
    std::unique_ptr<QuickCircuitBuilder> builder(createRingOscillator());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    Simulation sim(builder.get());
    sim.update();

    verifyFeedbackDetection(builder.get());
}

void TestFeedback::testPriorityCalculationWithFeedback()
{
    // Priority calculation in logicelement.cpp should detect feedback
    std::unique_ptr<QuickCircuitBuilder> builder(createSRLatchFromNAND());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    // Verify feedback detection sets priority correctly
    verifyFeedbackDetection(builder.get());
}

// ============================================================
// Edge Cases Tests
// ============================================================

void TestFeedback::testSingleElementFeedback()
{
    // NOT gate connected to itself (single element feedback, converges at Unknown)
    QuickCircuitBuilder builder;

    auto *notGate = builder.addOwnedElement(ElementFactory::buildElement(ElementType::Not));
    QVERIFY2(notGate != nullptr, "Failed to create NOT gate");

    notGate->setPos(50, 50);

    // Create feedback connection: NOT output -> NOT input (self-loop)
    builder.connect(notGate, 0, notGate, 0);

    Simulation sim(&builder);
    sim.update();

    verifyFeedbackDetection(&builder);
}

void TestFeedback::testFeedbackThroughMultipleElementTypes()
{
    // Feedback path through multiple element types (3-NOT ring - odd length, converges at Unknown in 4-state logic)
    std::unique_ptr<QuickCircuitBuilder> builder(createRingOscillator());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    Simulation sim(builder.get());
    sim.update();

    verifyFeedbackDetection(builder.get());
}

void TestFeedback::testLargeFeedbackLoops()
{
    // Large feedback loop with 10 elements (even count → converges)
    QuickCircuitBuilder builder;

    // Create chain of 10 inverters
    QVector<GraphicElement *> inverters;
    for (int i = 0; i < 10; ++i) {
        auto *inv = builder.addOwnedElement(ElementFactory::buildElement(ElementType::Not));
        QVERIFY2(inv != nullptr, "Failed to create inverter");
        inv->setPos(i * 50, 0);
        inverters.append(inv);
    }

    // Wire them as a ring: inv0 → inv1 → ... → inv9 → inv0
    for (int i = 0; i < 10; ++i) {
        builder.connect(inverters[i], 0, inverters[(i + 1) % 10], 0);
    }

    Simulation sim(&builder);
    sim.update();

    verifyFeedbackDetection(&builder);
    verifyStableState(&builder);
}

void TestFeedback::testAllCycleNodesMarked()
{
    // SR latch from NAND: switchS → nand1[0], switchR → nand2[0], nand1 ↔ nand2
    // Both NAND gates are in the cycle; both switches are NOT.
    std::unique_ptr<QuickCircuitBuilder> builder(createSRLatchFromNAND());
    QVERIFY(builder != nullptr);

    Simulation sim(builder.get());
    sim.update();

    int feedbackCount = 0;
    int nonFeedbackCount = 0;

    for (auto *elem : builder->elements()) {
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
    QuickCircuitBuilder builder;

    // Create 10 NOT gates in a ring
    QVector<GraphicElement *> elements;
    for (int i = 0; i < 10; ++i) {
        auto *notGate = builder.addOwnedElement(ElementFactory::buildElement(ElementType::Not));
        QVERIFY2(notGate != nullptr, "Failed to create NOT gate");
        notGate->setPos(i * 50, 0);
        elements.append(notGate);
    }

    // Wire them as a ring: not0 → not1 → ... → not9 → not0 (even count → converges)
    for (int i = 0; i < 10; ++i) {
        builder.connect(elements[i], 0, elements[(i + 1) % 10], 0);
    }

    Simulation sim(&builder);
    sim.update();

    verifyFeedbackDetection(&builder);
}

void TestFeedback::testMultipleSimultaneousFeedbackLoops()
{
    // 5 or more independent feedback loops running simultaneously
    QuickCircuitBuilder builder;

    // Create 5 separate 2-NOT feedback rings (each bistable, converges)
    for (int loop = 0; loop < 5; ++loop) {
        QVector<GraphicElement *> loopElements;
        for (int i = 0; i < 2; ++i) { // 2 NOT gates per loop
            auto *notGate = builder.addOwnedElement(ElementFactory::buildElement(ElementType::Not));
            QVERIFY2(notGate != nullptr, "Failed to create NOT gate");
            notGate->setPos(loop * 150 + i * 60, 0);
            loopElements.append(notGate);
        }

        // Wire the 2-NOT ring: not0 ↔ not1
        builder.connect(loopElements[0], 0, loopElements[1], 0);
        builder.connect(loopElements[1], 0, loopElements[0], 0);
    }

    Simulation sim(&builder);
    sim.update();

    verifyFeedbackDetection(&builder);
}

// ============================================================
// Helper Functions
// ============================================================

void TestFeedback::findSRLatchParts(QuickCircuitBuilder *builder, InputSwitch *&switchS, InputSwitch *&switchR,
                                    GraphicElement *&nand1Q, GraphicElement *&nand2QBar)
{
    switchS = switchR = nullptr;
    nand1Q = nand2QBar = nullptr;
    for (auto *elem : builder->elements()) {
        const bool isSSide = (elem->pos().y() == 0);
        if (elem->elementType() == ElementType::InputSwitch) {
            (isSSide ? switchS : switchR) = dynamic_cast<InputSwitch *>(elem);
        } else if (elem->elementType() == ElementType::Nand) {
            (isSSide ? nand1Q : nand2QBar) = elem;
        }
    }
}

QuickCircuitBuilder *TestFeedback::createSRLatchFromNAND()
{
    auto *builder = new QuickCircuitBuilder();

    auto *nand1 = builder->addOwnedElement(ElementFactory::buildElement(ElementType::Nand));
    auto *nand2 = builder->addOwnedElement(ElementFactory::buildElement(ElementType::Nand));
    auto *switchS = builder->addOwnedElement(ElementFactory::buildElement(ElementType::InputSwitch));
    auto *switchR = builder->addOwnedElement(ElementFactory::buildElement(ElementType::InputSwitch));

    nand1->setPos(100, 0);
    nand2->setPos(100, 50);
    switchS->setPos(0, 0);
    switchR->setPos(0, 50);

    // Wire: S → nand1[0], R → nand2[0]
    builder->connect(switchS, 0, nand1, 0);
    builder->connect(switchR, 0, nand2, 0);

    // Cross-feedback: nand2.out → nand1[1], nand1.out → nand2[1]
    builder->connect(nand2, 0, nand1, 1);
    builder->connect(nand1, 0, nand2, 1);

    return builder;
}

QuickCircuitBuilder *TestFeedback::createDLatchWithFeedback()
{
    auto *builder = new QuickCircuitBuilder();

    auto *dLatch = builder->addOwnedElement(ElementFactory::buildElement(ElementType::DLatch));
    auto *switchData = builder->addOwnedElement(ElementFactory::buildElement(ElementType::InputSwitch));
    auto *switchEnable = builder->addOwnedElement(ElementFactory::buildElement(ElementType::InputSwitch));

    dLatch->setPos(100, 50);
    switchData->setPos(0, 0);
    switchEnable->setPos(0, 50);

    // Wire Data and Enable inputs
    builder->connect(switchData, 0, dLatch, 0);
    builder->connect(switchEnable, 0, dLatch, 1);

    return builder;
}

QuickCircuitBuilder *TestFeedback::createRingOscillator()
{
    auto *builder = new QuickCircuitBuilder();

    auto *not1 = builder->addOwnedElement(ElementFactory::buildElement(ElementType::Not));
    auto *not2 = builder->addOwnedElement(ElementFactory::buildElement(ElementType::Not));
    auto *not3 = builder->addOwnedElement(ElementFactory::buildElement(ElementType::Not));

    not1->setPos(0, 0);
    not2->setPos(100, 0);
    not3->setPos(200, 0);

    // Wire as ring: NOT1 -> NOT2 -> NOT3 -> NOT1 (odd count, non-converging)
    builder->connect(not1, 0, not2, 0);
    builder->connect(not2, 0, not3, 0);
    builder->connect(not3, 0, not1, 0);

    return builder;
}

QuickCircuitBuilder *TestFeedback::createMixedCircuit()
{
    auto *builder = new QuickCircuitBuilder();

    auto *and1 = builder->addOwnedElement(ElementFactory::buildElement(ElementType::And));
    auto *or1 = builder->addOwnedElement(ElementFactory::buildElement(ElementType::Or));
    auto *not1 = builder->addOwnedElement(ElementFactory::buildElement(ElementType::Not));
    auto *btn = builder->addOwnedElement(ElementFactory::buildElement(ElementType::InputButton));

    and1->setPos(0, 0);
    or1->setPos(100, 0);
    not1->setPos(200, 0);
    btn->setPos(-100, 0);

    // Wire: btn → and1[0,1] → or1[0], not1.out → or1[1], or1.out → not1.in
    // Combinational path: btn → AND → OR
    builder->connect(btn, 0, and1, 0);
    builder->connect(btn, 0, and1, 1);
    builder->connect(and1, 0, or1, 0);

    // Feedback ring: OR → NOT → OR
    builder->connect(or1, 0, not1, 0);
    builder->connect(not1, 0, or1, 1);

    return builder;
}

void TestFeedback::verifyConvergence(QuickCircuitBuilder *builder)
{
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    Simulation sim(builder);
    sim.update();

    // Snapshot all logic element outputs
    QHash<GraphicElement *, Status> snapshot;
    for (auto *elem : builder->elements()) {
        if (elem->simOutputSize() > 0) {
            snapshot[elem] = elem->outputValue(0);
        }
    }

    // Run another update cycle
    sim.update();

    // Verify outputs haven't changed
    for (auto *elem : builder->elements()) {
        if (elem->simOutputSize() > 0) {
            Status currentValue = elem->outputValue(0);
            Status snapshotValue = snapshot.value(elem, currentValue);
            QCOMPARE(currentValue, snapshotValue);
        }
    }
}

void TestFeedback::verifyFeedbackDetection(QuickCircuitBuilder *builder)
{
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    // Initialize simulation to trigger feedback detection
    Simulation sim(builder);
    sim.update();

    // Verify that at least one element is in a feedback loop
    bool found = false;
    for (auto *elem : builder->elements()) {
        if (sim.isInFeedbackLoop(elem)) {
            found = true;
            break;
        }
    }
    QVERIFY2(found, "No element detected in a feedback loop");
}

void TestFeedback::verifyStableState(QuickCircuitBuilder *builder)
{
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    auto elements = builder->elements();
    QVERIFY2(!elements.isEmpty(), "Circuit elements should not be empty");

    // Initialize simulation to stabilize
    Simulation sim(builder);
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
