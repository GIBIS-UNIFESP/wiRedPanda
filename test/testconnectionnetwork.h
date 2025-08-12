// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestConnectionNetwork : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Complex connection network tests
    void testSimpleCircuitConnections();
    void testComplexNetworkTopology();
    void testConnectionValidation();
    void testConnectionCycles();
    void testConnectionBranching();
    void testConnectionSignalPropagation();
    void testConnectionRemovalEffects();
    void testConnectionRecreation();
    void testConnectionInterference();
    void testConnectionPerformance();
    void testConnectionMemoryManagement();
    void testConnectionStateConsistency();
};