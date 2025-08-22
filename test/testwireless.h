// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestWireless : public QObject
{
    Q_OBJECT

private slots:
    void testWirelessConnectionManager();
    void testBasicWirelessConnection();
    void testMultipleWirelessConnections();
    void testWirelessLabelChanges();
    void testWirelessNodeRemoval();
    void testEmptyLabels();
    void testDuplicateLabels();
    void testWirelessConnectionInSimulation();
    void testWirelessSerialization();
    void testEdgeCases();
    void testStressConditions();
    void testInvalidOperations();
    void testStateConsistency();

    // New comprehensive simulation tests
    void testWirelessSignalPriority();
    void testWirelessSignalConsistency();
    void testWirelessSingleNodeGroup();
    void testWirelessMultipleGroups();
    void testWirelessNodeWithoutInput();
    void testWirelessUpdateCycles();

    // UI constraint validation tests
    void testWirelessUIConstraints();
    void testWirelessConnectionValidation();
    void testWirelessUIDisabledState();
};
