// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestIntegration : public QObject
{
    Q_OBJECT

private slots:
    // Basic circuit construction tests
    void testBasicAndGateCircuit();
    void testBasicOrGateCircuit();
    void testNotGateChain();
    void testCombinationalLogicCircuit();

    // Sequential circuit tests
    void testDFlipFlopWithClock();
    void testJKFlipFlopSequence();
    void testSRFlipFlopBehavior();
    void testLatchCircuit();

    // Complex circuit tests
    void testBinaryCounter();
    void testMultiplexerCircuit();
    void testDecoderCircuit();
    void testAdderCircuit();

    // File-based integration tests
    void testLoadAndSimulateExampleFiles();
    void testSpecificExampleSimulation();
    void testRoundTripFileSerialization();

    // Simulation workflow tests
    void testSimulationLifecycle();
    void testTimingBehavior();
    void testStateTransitions();

    // Edge case and stress tests
    void testLargeCircuitSimulation();
    void testComplexConnectionPatterns();
    void testErrorHandlingInSimulation();
    void testMemoryManagement();

    // Memory circuit bug validation tests
    void testSRLatchBugFix();
    void testDLatchTransparency();
    void testDLatchEnableSignal();
    void testShiftRegisterCircuit();
    void testBinaryCounterWithFlipFlops();
    void testCrossCoupledMemoryCircuits();
    void testMemoryCircuitEdgeCases();

private:
    // Helper functions for common test patterns
    void setupBasicWorkspace();
    void verifySimulationOutput(const QString &testName, bool expectedOutput);
    void runSimulationCycles(int cycles);
    bool loadExampleFile(const QString &fileName);
    void cleanupWorkspace();
    int countConnections() const;

    // Test infrastructure members
    class WorkSpace *m_workspace = nullptr;
    class Scene *m_scene = nullptr;
    class Simulation *m_simulation = nullptr;
};
