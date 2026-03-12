// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class Scene;
class Simulation;

class TestFeedback : public QObject {
    Q_OBJECT

public:
    TestFeedback() = default;

private slots:
    // Test infrastructure

    // Convergence Detection Tests (8 tests)
    void testSRLatchFromNANDConvergence();
    void testDLatchWithFeedback();
    void testRingOscillatorNonConvergence();
    void testSetResetPriorityInSRLatch();
    void testInitialStateDependency();
    void testPureCombinationalCircuit();
    void testIterationCountVerification();
    void testConvergenceSpeedVariation();

    // Non-Converging Circuits Tests (5 tests)
    void testRingOscillatorWarningAfterMaxIterations();
    void testOddLengthInverterChainFeedback();
    void testConflictingFeedbackSignals();
    void testWarningMessageContent();
    void testSimulationContinuesAfterNonConvergence();

    // Mixed Circuits Tests (4 tests)
    void testCombinationalPlusFeedbackInSameCircuit();
    void testMultipleIndependentFeedbackLoops();
    void testNestedFeedbackLoops();
    void testPriorityCalculationWithFeedback();

    // Edge Cases Tests (4 tests)
    void testSingleElementFeedback();
    void testFeedbackThroughMultipleElementTypes();
    void testLargeFeedbackLoops();
    void testAllCycleNodesMarked();

    // Performance Tests (2 tests)
    void testDeepCircuitsWithFeedback();
    void testMultipleSimultaneousFeedbackLoops();

private:
    // Helper functions for circuit building
    Scene *createSRLatchFromNAND();
    Scene *createDLatchWithFeedback();
    Scene *createRingOscillator();
    Scene *createMixedCircuit();
    Scene *createDeepCircuit();

    // Verification helpers
    void verifyConvergence(Scene *scene, bool shouldConverge);
    void verifyFeedbackDetection(Scene *scene);
    void verifyStableState(Scene *scene);
};
