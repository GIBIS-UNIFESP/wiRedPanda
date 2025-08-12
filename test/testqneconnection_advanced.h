// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestQNEConnectionAdvanced : public QObject
{
    Q_OBJECT

private slots:
    void testLoadMethodEmptyPortMap();
    void testLoadMethodWithPortMap();
    void testLoadMethodMissingPortsInMap();
    void testLoadMethodPortTypeCombinations();
    // Note: sceneEvent() tests removed as method is protected
    void testSetPortsWithNullValues();
    void testSetPortsReplacingExistingPorts();
    void testStatusTransitionsWithEdgeCases();
    void testItemChangeSelectionHandling();
    void testAngleCalculationEdgeCases();
    void testOtherPortEdgeCases();
    void testPaintMethodWithDifferentStates();
};