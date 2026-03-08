// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/Cpu/TestCPUIntegration.h"

#include "Tests/Integration/IC/Tests/Cpu/CpuHelpers.h"

using TestUtils::clockCycle;
using TestUtils::readMultiBitOutput;

// ============================================================
// Test Group: Program Counter
// ============================================================
// Tests the 8-bit Program Counter component in isolation:
// increment-by-one behavior over N clock cycles.
//

void TestCPUIntegration::initTestCase()
{
    TestUtils::setupTestEnvironment();
}

void TestCPUIntegration::cleanup()
{
}

void TestCPUIntegration::testProgramCounterIncrement()
{
    QFETCH(int, programLength);
    // Build 8-bit Program Counter component
    InputSwitch *pc_clock = new InputSwitch();
    InputSwitch *reset = new InputSwitch();
    QVector<InputSwitch *> pc_load_val;
    QVector<Led *> pc_out;
    for (int i = 0; i < 8; i++) {
        pc_load_val.append(new InputSwitch());
        pc_out.append(new Led());
    }
    InputSwitch *pc_load = new InputSwitch();
    InputSwitch *pc_inc = new InputSwitch();
    InputSwitch *pc_enable = new InputSwitch();
    std::unique_ptr<WorkSpace> workspace_pc(buildProgramCounter8bit(
        pc_load_val.data(), pc_load, pc_inc, reset, pc_enable, pc_clock, pc_out.data()));
    auto *sim_pc = workspace_pc->simulation();
    // Reset system to initial state
    reset->setOn(true);
    pc_enable->setOn(false);
    pc_inc->setOn(false);
    sim_pc->update();
    clockCycle(sim_pc, pc_clock);
    sim_pc->update();
    reset->setOn(false);
    // Enable PC for execution
    pc_enable->setOn(true);
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

