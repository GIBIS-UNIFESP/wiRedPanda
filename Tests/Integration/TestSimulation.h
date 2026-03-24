// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestSimulation : public QObject
{
    Q_OBJECT

private slots:
    // Topological sorting
    void testTopologicalSorting();

    // Signal propagation - combinational logic
    void testAndGatePropagation();
    void testOrGatePropagation();
    void testNotGatePropagation();
    void testNandGatePropagation();
    void testNorGatePropagation();
    void testXorGatePropagation();

    // Cascaded gates
    void testCascadedGates();

    // Multi-cycle stability
    void testMultiCycleStability();

    // Initialization
    void testSimulationInitialization();

    // Determinism tests
    void testElementProcessingOrderConsistency();
    void testSceneInitializationDeterminism();
    void testSimulationOutputReproducibility();
    void testSimulationGraphStability();
    void testCircuitWithFeedbackLoops();

    // Wireless signal propagation
    void testWirelessTxRxPropagation();
    void testWirelessMultiRxFanOut();
    void testWirelessOrphanedRx();
    void testWirelessInsideIC();

    // Wireless edge cases
    void testWirelessDuplicateTxIgnored();
    void testWirelessEmptyLabelIgnored();
    void testWirelessLabelMismatchIsolation();
    void testWirelessMultipleIndependentChannels();
    void testWirelessTxUnconnectedInput();
    void testWirelessLabelCaseSensitive();
    void testWirelessNoneModeNodeDoesNotInterfere();
    void testWirelessLabelSetAfterMode();
    void testWirelessCascadeThroughWire();
    void testWirelessRxFeedsCombinationalLogic();
    void testWirelessOrphanedTxDoesNotCrash();
    void testWirelessOrphanedRxWithPhysicalWire();
    void testWirelessOverridesPhysicalWire();
    void testWirelessFeedbackLoop();
    void testWirelessUnicodeLabels();

    // Negative tests — verify graceful handling of degenerate circuit conditions
    void testUnconnectedRequiredInputGraceful();
    void testPartiallyConnectedCircuitIsolation();
    void testDanglingConnectionGraceful();
};

