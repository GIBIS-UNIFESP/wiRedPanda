// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestLevel5LoadableCounter4Bit : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void testLoadableCounter_data();
    void testLoadableCounter();

    // Boundary transition tests (all 16 transitions 0->1, 1->2, ..., 15->0)
    void testBinaryCounterBoundaryTransitions_data();
    void testBinaryCounterBoundaryTransitions();

    // Timing edge case tests (rapid consecutive transitions)
    void testBinaryCounterTimingEdgeCases_data();
    void testBinaryCounterTimingEdgeCases();

    // Hold behavior extended tests (multi-cycle stability)
    void testBinaryCounterHoldBehavior_data();
    void testBinaryCounterHoldBehavior();
};

