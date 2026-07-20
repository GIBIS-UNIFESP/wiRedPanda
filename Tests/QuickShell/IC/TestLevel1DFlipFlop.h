// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/// Phase C validation case for the qtquick-rewrite plan's Widgets-test-parity audit: the first
/// Level/CPU regression class ported onto QuickCircuitBuilder, proving the harness works before
/// trusting it for the full ~80-class suite (see .claude/WIDGETS_TEST_PARITY_AUDIT.md). Mirrors
/// the deleted Tests/Integration/IC/Tests/TestLevel1DFlipFlop.cpp verbatim in test behavior --
/// only the circuit-construction harness changed (QuickCircuitBuilder instead of
/// CircuitBuilder(Scene*), heap-allocated elements now via addOwnedElement() since
/// QuickCircuitBuilder's addElement() is non-owning, unlike the old Scene-backed one).
class TestLevel1DFlipFlop : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    // D Flip-Flop IC tests - sequential design for proper edge-triggered testing
    void testDFlipFlopSequential();
    void testPresetClearOverrideClock();
    void testAsyncPresetClearUnderClockHigh();
    void testPresetClearBothAsserted();
};
