// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>

#include <QObject>
#include <QString>
#include <QTest>
#include <QVector>

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

    /// The graph-level half of the override. connectWirelessElements() repoints an Rx at its
    /// Tx, while sortSimElements() builds edges by walking output-port CONNECTIONS -- so the
    /// overridden physical wire must be dropped from m_successorGraph, or it stays as an edge
    /// the engine never reads. Route that wire from downstream and such an edge fabricates a
    /// cycle in an acyclic circuit, dropping calculatePriorities() onto the legacy walk for the
    /// WHOLE circuit and making the phantom component a canonicalisation target.
    void testWirelessOverrideDoesNotFabricateFeedbackLoop();
    void testWirelessFeedbackLoop();
    void testWirelessUnicodeLabels();

    // Negative tests — verify graceful handling of degenerate circuit conditions
    void testUnconnectedRequiredInputGraceful();
    void testPartiallyConnectedCircuitIsolation();
    void testDanglingConnectionGraceful();

private:
    // Shared by the five determinism tests above, which differ only in what circuit they
    // build and what property they extract: calls `runOnce` `numRuns` times (each call builds
    // its own fresh circuit and returns a vector of some deterministic, comparable property)
    // and verifies every run matches the first.
    template <typename T>
    void verifyDeterministicAcrossRuns(int numRuns, const QString &label, const std::function<QVector<T>()> &runOnce)
    {
        QVector<QVector<T>> runs;
        for (int run = 0; run < numRuns; ++run) {
            runs.append(runOnce());
        }
        for (int run = 1; run < numRuns; ++run) {
            QCOMPARE(runs[run].size(), runs[0].size());
            for (int i = 0; i < runs[0].size(); ++i) {
                if (runs[run][i] != runs[0][i]) {
                    QFAIL(qPrintable(QString("%1 mismatch at run %2, position %3").arg(label).arg(run).arg(i)));
                }
            }
        }
    }
};
