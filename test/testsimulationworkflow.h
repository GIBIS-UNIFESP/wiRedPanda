// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestSimulationWorkflow : public QObject
{
    Q_OBJECT

private slots:
    // File-based integration tests
    void testLoadAndSimulateExampleFiles();
    void testSpecificExampleSimulation();
    void testRoundTripFileSerialization();

    // Simulation lifecycle tests
    void testSimulationLifecycle();
    void testTimingBehavior();
    void testStateTransitions();

    // Error handling and edge cases
    void testErrorHandlingInSimulation();
    void testMemoryManagement();

    // Basic simulation workflow (from TestSimulation)
    void testBasicSimulationWorkflow();

    // ElementMapping and simulation engine testing
    void testElementMappingTopologicalSort();
    void testDependencyResolution();
    void testPriorityCalculation();
    void testSimulationConvergence();
    void testCombinationalLoopDetection();

private:
    // Helper functions for simulation workflow
    void setupBasicWorkspace();
    void cleanupWorkspace();
    void runSimulationCycles(int cycles);
    void verifySimulationOutput(const QString &testName, bool expectedOutput);
    bool loadExampleFile(const QString &fileName);
    int countConnections() const;

    // Test infrastructure members
    class WorkSpace *m_workspace = nullptr;
    class Scene *m_scene = nullptr;
    class Simulation *m_simulation = nullptr;
};
