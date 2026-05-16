// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestSequential : public QObject
{
    Q_OBJECT

private slots:

    // ============================================================
    // Async Reset Tests (Issue 4.3)
    // ============================================================

    void testRegisterAsyncClear_data();
    void testRegisterAsyncClear();

    void testCounterAsyncReset_data();
    void testCounterAsyncReset();

    void testShiftRegisterAsyncClear_data();
    void testShiftRegisterAsyncClear();

    // ============================================================
    // FSM Tests
    // ============================================================

    void testSimpleStateMachine_data();
    void testSimpleStateMachine();

    void testFsmStateTransitions_data();
    void testFsmStateTransitions();

    void testFsmTimingEdgeCases_data();
    void testFsmTimingEdgeCases();

    void testFsmExtendedSequences_data();
    void testFsmExtendedSequences();

    void testFsmQnotComplementarity_data();
    void testFsmQnotComplementarity();

    void testFsmLongSequenceStability_data();
    void testFsmLongSequenceStability();

    void testFsmStateLock_data();
    void testFsmStateLock();

    void testFsmRapidClockNoSettle_data();
    void testFsmRapidClockNoSettle();

    void testFsmTriggerDuringClock_data();
    void testFsmTriggerDuringClock();

    void testFsmLongTermStability_data();
    void testFsmLongTermStability();

    // ============================================================
    // T Flip-Flop Tests (Issue #3)
    // ============================================================

    void testTFlipFlopToggle_data();
    void testTFlipFlopToggle();

    // ============================================================
    // SR Flip-Flop Tests (Issue #3)
    // ============================================================

    void testSRFlipFlopSetReset_data();
    void testSRFlipFlopSetReset();
};
