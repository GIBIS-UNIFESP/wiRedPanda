// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

// Test Suite Organization based on logical functionality groups
// This consolidates 42 test methods into 6 focused test suites

// Core wireless functionality tests (15 tests)
class TestWirelessCore : public QObject
{
    Q_OBJECT

private slots:
    void testWirelessManager();
    void testBasicWirelessConnection();
    void testMultipleWirelessConnections();
    void testWirelessLabelChanges();
    void testWirelessNodeRemoval();
    void testEmptyLabels();
    void testDuplicateLabels();
    void testWirelessSingleNodeGroup();
    void testWirelessMultipleGroups();
    void testWirelessNodeWithoutInput();
    void testWirelessChains();
    void testWireless1NConstraint();
    void testWirelessDuplicationConstraint();
    void testWirelessFeedbackPrevention();
    void testStateConsistency();
};

// UI integration and constraint validation tests (7 tests)
class TestWirelessUI : public QObject
{
    Q_OBJECT

private slots:
    void testWirelessUIConstraints();
    void testWirelessConnectionValidation();
    void testWirelessUIDisabledState();
    void testWirelessUIValidation();
    void testWirelessCopyPaste();
    void testExtremeLabels();
    void testWirelessVisibilityAfterSaveLoad();
};

// Simulation and timing behavior tests (8 tests)
class TestWirelessSimulation : public QObject
{
    Q_OBJECT

private slots:
    void testWirelessConnectionInSimulation();
    void testWirelessSignalPriority();
    void testWirelessSignalConsistency();
    void testWirelessUpdateCycles();
    void testWirelessPhysicalTimingEquivalence();
    void testMixedConnectionChains();
    void testComplexTimingScenarios();
    void testWirelessWithSequentialElements();
};

// Edge cases and robustness tests (6 tests)
class TestWirelessEdgeCases : public QObject
{
    Q_OBJECT

private slots:
    void testEdgeCases();
    void testInvalidOperations();
    void testMemoryLeakPrevention();
    void testCorruptionRecovery();
    void testWirelessWithICs();
    void testSerializationCorruption();
};

// Serialization and persistence tests (2 tests)
class TestWirelessSerialization : public QObject
{
    Q_OBJECT

private slots:
    void testWirelessSerialization();
    void testLargeWirelessSerialization();
};
