// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestSimulation : public QObject
{
    Q_OBJECT

private slots:
    void testCase1();
    
    // Core functionality tests
    void testSimulationConstruction();
    void testSimulationWithScene();
    void testInitialize();
    void testIsRunning();
    void testStartStop();
    void testRestart();
    void testUpdate();
    
    // Timer and clock tests  
    void testTimerBasicFunctionality();
    void testClockElements();
    void testTimerInterval();
    
    // Simulation state tests
    void testInitializedState();
    void testSimulationReset();
    void testMultipleStartStop();
    
    // Circuit simulation tests
    void testSimpleLogicCircuit();
    void testComplexLogicCircuit();
    void testSequentialLogicCircuit();
    
    // Element interaction tests
    void testInputElementsUpdate();
    void testOutputElementsUpdate();
    void testConnectionsUpdate();
    void testLogicElementsUpdate();
    
    // Integration tests
    void testSimulationFlow();
    void testSimulationWithClocks();
    void testSimulationEdgeCases();
    
    // Performance and stability tests
    void testSimulationPerformance();
    void testSimulationStability();
};
