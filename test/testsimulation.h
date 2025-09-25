// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestSimulation : public QObject
{
    Q_OBJECT

private slots:
    void testCase1();

    // Determinism tests to prevent regressions
    void testElementProcessingOrderConsistency();
    void testSceneInitializationDeterminism();
    void testSimulationOutputReproducibility();
    void testElementMappingStability();
    void testCircuitWithFeedbackLoops();
};
