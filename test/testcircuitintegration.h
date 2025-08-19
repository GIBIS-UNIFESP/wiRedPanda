// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestCircuitIntegration : public QObject
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

    // Advanced circuit tests
    void testShiftRegisterCircuit();
    void testBinaryCounterWithFlipFlops();
    void testCrossCoupledMemoryCircuits();

    // Memory circuit validation tests
    void testSRLatchBugFix();
    void testDLatchTransparency();
    void testDLatchEnableSignal();
    void testMemoryCircuitEdgeCases();

    // Large circuit and stress tests
    void testLargeCircuitSimulation();
    void testComplexConnectionPatterns();

    // Sequential timing verification tests
    void testSequentialTimingVerification();

private:
    // Helper functions for circuit construction
    void setupBasicWorkspace();
    void cleanupWorkspace();
    void runSimulationCycles(int cycles);
    void verifySimulationOutput(const QString &testName, bool expectedOutput);
    int countConnections() const;

    // Test infrastructure members
    class WorkSpace *m_workspace = nullptr;
    class Scene *m_scene = nullptr;
    class Simulation *m_simulation = nullptr;
};
