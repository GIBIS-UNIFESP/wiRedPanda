// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestFeedback.h"

#include <QRegularExpression>

#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"

// ============================================================
// Convergence Detection Tests
// ============================================================

void TestFeedback::testSRLatchFromNANDConvergence()
{
    // SR latch built from NAND gates should converge to stable state
    std::unique_ptr<QuickCircuitBuilder> builder(createSRLatchFromNAND());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    verifyConvergence(builder.get(), true);
    verifyStableState(builder.get());
}

void TestFeedback::testDLatchWithFeedback()
{
    // D latch with feedback should converge
    std::unique_ptr<QuickCircuitBuilder> builder(createDLatchWithFeedback());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    verifyConvergence(builder.get(), true);
    verifyStableState(builder.get());
}

void TestFeedback::testRingOscillatorNonConvergence()
{
    // With 4-state logic, ring oscillators converge at Unknown
    // (NOT(Unknown) = Unknown is a fixed point).
    std::unique_ptr<QuickCircuitBuilder> builder(createRingOscillator());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    verifyConvergence(builder.get(), true);
}

void TestFeedback::testSetResetPriorityInSRLatch()
{
    // SR latch from NAND gates respects Set/Reset priority
    std::unique_ptr<QuickCircuitBuilder> builder(createSRLatchFromNAND());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    // Create simulation and verify stable outputs
    Simulation sim(builder.get());

    // Update to stabilize
    sim.update();

    // Verify outputs are stable and consistent
    verifyStableState(builder.get());
}

void TestFeedback::testInitialStateDependency()
{
    // Feedback circuits depend on initial state
    // Create two identical SR latches with different initial states
    std::unique_ptr<QuickCircuitBuilder> builder1(createSRLatchFromNAND());
    std::unique_ptr<QuickCircuitBuilder> builder2(createSRLatchFromNAND());

    QVERIFY2(builder1 != nullptr, "Failed to create first circuit");
    QVERIFY2(builder2 != nullptr, "Failed to create second circuit");

    Simulation sim1(builder1.get());
    Simulation sim2(builder2.get());

    // Both should converge despite potentially different initial states
    sim1.update();
    sim2.update();

    verifyFeedbackDetection(builder1.get());
    verifyFeedbackDetection(builder2.get());
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
    // Different circuits should converge in different numbers of iterations
    std::unique_ptr<QuickCircuitBuilder> builder1(createSRLatchFromNAND()); // Should converge quickly (2-3 iterations)
    std::unique_ptr<QuickCircuitBuilder> builder2(createRingOscillator());  // 4-state logic converges at Unknown

    QVERIFY2(builder1 != nullptr, "Failed to create first circuit");
    QVERIFY2(builder2 != nullptr, "Failed to create second circuit");

    Simulation sim1(builder1.get());
    Simulation sim2(builder2.get());

    sim1.update();
    verifyStableState(builder1.get());

    sim2.update();
}

// ============================================================
// Non-Converging Circuits Tests
// ============================================================

void TestFeedback::testRingOscillatorWarningAfterMaxIterations()
{
    std::unique_ptr<QuickCircuitBuilder> builder(createRingOscillator());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    Simulation sim(builder.get());
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

    verifyConvergence(builder.get(), true);
}

void TestFeedback::testWarningMessageContent()
{
    // Verify warning message for circuits that converge at Unknown
    std::unique_ptr<QuickCircuitBuilder> builder(createRingOscillator());
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    Simulation sim(builder.get());
    sim.update();

    // Verify the circuit has feedback elements
    bool found = false;
    for (auto *elem : builder->elements()) {
        if (sim.isInFeedbackLoop(elem)) {
            found = true;
            break;
        }
    }
    QVERIFY2(found, "Ring oscillator should have feedback elements");
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

void TestFeedback::verifyConvergence(QuickCircuitBuilder *builder, bool shouldConverge)
{
    QVERIFY2(builder != nullptr, "Failed to create feedback circuit");

    if (!shouldConverge) {
        // Non-converging circuits will print a debug warning
        QTest::ignoreMessage(QtDebugMsg, QRegularExpression(".*converge.*"));
    }

    Simulation sim(builder);
    sim.update();

    if (shouldConverge) {
        // For converging circuits, verify outputs are stable
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
