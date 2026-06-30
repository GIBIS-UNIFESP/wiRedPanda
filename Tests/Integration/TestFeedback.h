// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>
#include <QVector>

class GraphicElement;
class InputSwitch;
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
    void testNonConvergenceDiagnosticNamesTheTrippingElementAndComponent();
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

    // --- Oscillation trip-path guards ---

    /// The engine stages every element's evaluation, so a bare setOutputValue(Unknown) inside
    /// canonicalizeOscillation() would land in the staging buffer of any element mid-window and
    /// never be published. Every element of a canonicalised region must actually read Unknown,
    /// not just the subset that happened not to be staging.
    void testCanonicalizationReachesEveryElementOfTheRegion();

    /// A trip leaves the timestamp loop before the sampling region would ordinarily run, so
    /// every Memory-group element queued in pendingSamples for that timestamp must still be
    /// sampled on the way out -- otherwise a flip-flop clocked on that tick silently never
    /// captures.
    void testFlipFlopStillSamplesWhenOscillationTripsSameTick();

    // --- Non-convergence semantics ---

    /// The canonicalised Unknown must reach elements that READ the oscillating region. Clearing
    /// the queue on a trip would strand them: the steady-state seed wakes successors only for
    /// changed inputs and clocks, never an arbitrary element, so a downstream gate would keep
    /// its stale definite value permanently.
    void testOscillationUnknownReachesDownstreamReader();

    /// Canonicalisation must be scoped to the strongly connected component that is actually
    /// oscillating. Working from a flat set of every element in any cycle would drag an
    /// unrelated, settled SR latch to Unknown along with the oscillating ring.
    void testOscillationDoesNotCanonicalizeUnrelatedFeedbackRegion();

    /// Two independent oscillators must BOTH canonicalise, and the drain must still terminate --
    /// the design's argument is that each trip freezes at least one SCC and there are finitely
    /// many, which nothing else exercises.
    void testTwoIndependentOscillatorsBothCanonicalize();

    /// The cap must not false-positive on legitimate multi-wave settling. A straight chain is
    /// useless for this -- each element evaluates about once. Reconvergent fan-in is the shape
    /// that makes one element re-evaluate many times at a single timestamp.
    void testReconvergentFanInDoesNotTripCap();

    /// A feedback loop made only of Memory-group elements must terminate. Level-sensitive
    /// latches have no edge detection, so a DLatch with ~Q wired back to D and Enable high
    /// oscillates within a single timestamp. Without a cap in the sampling region the drain
    /// never returns -- this one fails as a hang, not as an assertion.
    void testLatchOnlyFeedbackLoopTerminates();
    void testCrossCoupledLatchLoopTerminates();
    void testPropertyDisconnectedElementChangesNoReading();
    void testPropertyIdenticalCircuitsSettleIdentically();
    void testPropertyRestartReproducesTheSameSettledState();
    void testInverterRingTerminatesAndCanonicalizes_data();
    void testInverterRingTerminatesAndCanonicalizes();
    void testRingCrossingTheAbsoluteCeilingStillCanonicalizes();
    void testOscillatorWithADownstreamConsumerStillCanonicalizes();
    void testOscillatorInsideAnIcCanonicalizesAndRecovers();
    void testOscillatorAcrossAWirelessPairCanonicalizesAndRecovers();
    /// The companion guard: a flip-flop caught inside someone ELSE's oscillating cone must
    /// keep its stored state, which is what the unconditional Memory skip was protecting.
    void testFlipFlopKeepsStateWhenCombinationalRegionTrips();

    /// A flip-flop that captures on the tick an unrelated region trips must also WAKE its
    /// readers. Committing the deferred samples directly -- no clearOutputChanged() before, no
    /// wakeSuccessors() after -- would publish the captured value to nothing, and if the queue
    /// then emptied the idle skip would freeze the circuit there.
    void testSampledFlipFlopReachesDownstreamWhenOscillationTrips();

    /// Time-to-detect must not grow with the square of the oscillating component's size.
    /// cap = kEvalCapSlack * (d+1) * (|scc|+1) is a PRODUCT, so every member of an N-element
    /// ring burns a cap that itself scales with N; only the absolute per-timestamp ceiling keeps
    /// the total linear. Without it, rings of 11/51/101/201 take 755 / 14,274 / 59,826 /
    /// 222,045 us -- doubling N roughly quadruples the cost.
    void testOscillationDetectionCostIsNotQuadratic();

private:
    /// Builds \a count Not gates wired into a ring inside \a scene, oldest first. An odd
    /// \a count never converges.
    QVector<GraphicElement *> addInverterRing(Scene *scene, int count, qreal y = 0);

    // Helper functions for circuit building
    Scene *createSRLatchFromNAND();
    Scene *createDLatchWithFeedback();
    Scene *createRingOscillator();
    Scene *createMixedCircuit();

    // Verification helpers
    void verifyConvergence(Scene *scene);
    void verifyFeedbackDetection(Scene *scene);
    void verifyStableState(Scene *scene);

    // Locates the S/R switches and the Q/Q' NAND gates in a createSRLatchFromNAND() scene,
    // by their construction position (S-side elements at y=0, R-side at y=50).
    void findSRLatchParts(Scene *scene, InputSwitch *&switchS, InputSwitch *&switchR,
                          GraphicElement *&nand1Q, GraphicElement *&nand2QBar);
};
