// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/IC/TestCPUIntegration.h"

#include "Tests/QuickShell/IC/QuickCpuHelpers.h"

using TestUtils::clockCycle;
using TestUtils::readMultiBitOutput;

// ============================================================
// Test Group: Program Counter
// ============================================================
// Tests the 8-bit Program Counter component in isolation:
// increment-by-one behavior over N clock cycles.
//

void TestCPUIntegration::testProgramCounterIncrement()
{
    QFETCH(int, programLength);
    // Build 8-bit Program Counter component
    TestUtils::OwnedElementPool pool;
    InputSwitch *pc_clock = pool.make<InputSwitch>();
    InputSwitch *reset = pool.make<InputSwitch>();
    QVector<InputSwitch *> pc_load_val;
    QVector<Led *> pc_out;
    for (int i = 0; i < 8; i++) {
        pc_load_val.append(pool.make<InputSwitch>());
        pc_out.append(pool.make<Led>());
    }
    InputSwitch *pc_load = pool.make<InputSwitch>();
    InputSwitch *pc_inc = pool.make<InputSwitch>();
    auto builder_pc = buildProgramCounter8bit(
        pc_load_val.data(), pc_load, pc_inc, reset, pc_clock, pc_out.data());
    auto *sim_pc = builder_pc->simulation();
    // Reset system to initial state
    reset->setOn(true);
    pc_inc->setOn(false);
    sim_pc->update();
    clockCycle(sim_pc, pc_clock);
    sim_pc->update();
    reset->setOn(false);
    // Increment PC for execution
    pc_inc->setOn(true);
    sim_pc->update();
    int pc_value = 0;
    // Clock the PC (programLength + 1) times and read the final value
    for (int cycle = 0; cycle <= programLength; cycle++) {
        clockCycle(sim_pc, pc_clock);  // Complete clock cycle (HIGH->settle->LOW->settle)
        sim_pc->update();
        pc_value = readMultiBitOutput(pc_out);
    }
    // Verify: PC should have incremented exactly once per cycle
    // Loop runs from 0 to programLength (inclusive), so programLength+1 increments
    int expectedPC = programLength + 1;
    QCOMPARE(pc_value, expectedPC);
}

void TestCPUIntegration::testProgramCounterIncrement_data()
{
    QTest::addColumn<int>("programLength");
    QTest::newRow("pc_increment_1_cycle")  << 1;
    QTest::newRow("pc_increment_3_cycles") << 3;
    QTest::newRow("pc_increment_4_cycles") << 4;
    QTest::newRow("pc_increment_10_cycles") << 10;
}
