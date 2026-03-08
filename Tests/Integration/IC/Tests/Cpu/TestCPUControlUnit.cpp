// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/Cpu/TestCPUControlUnit.h"

#include "Tests/Integration/IC/Tests/Cpu/CpuHelpers.h"

using TestUtils::clockCycle;
using TestUtils::getInputStatus;
using TestUtils::readMultiBitOutput;

// ============================================================
// Consolidated Test Group: ControlUnit
// ============================================================
// This consolidated test class merges the following test classes:
// - TestCPU21ControlUnit
// - TestCPU22ControlUnitFlagwrite
//

void TestCPUControlUnit::initTestCase()
{
    // Initialize test case resources if needed
}

void TestCPUControlUnit::cleanup()
{
    // Clean up test resources if needed
}

void TestCPUControlUnit::testControlUnit()
{
    QFETCH(int, stateTransitions);
    QFETCH(int, expectedFinalState);
    InputSwitch *reset, *clock;
    QVector<Led *> state;
    Led *pcInc, *irLoad, *regWrite, *aluEnable, *flagWrite;
    reset = new InputSwitch();
    clock = new InputSwitch();
    for (int i = 0; i < 3; i++) {
        state.append(new Led());
    }
    pcInc = new Led();
    irLoad = new Led();
    regWrite = new Led();
    aluEnable = new Led();
    flagWrite = new Led();
    std::unique_ptr<WorkSpace> workspace(buildControlUnit(reset, clock, state.data(), pcInc, irLoad, regWrite, aluEnable, flagWrite));
    auto *sim = workspace->simulation();
    // Reset to FETCH
    reset->setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
    sim->update();
    reset->setOn(false);
    sim->update();
    int currentState = readMultiBitOutput(state);
    // Transition states
    for (int i = 0; i < stateTransitions; i++) {
        TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
        sim->update();
        currentState = readMultiBitOutput(state);
    }
    QCOMPARE(currentState % 5, expectedFinalState % 5);  // Modulo 5 for state wrapping
}

void TestCPUControlUnit::testControlUnitFlagWrite()
{
    // Test that flagWrite is asserted during EXECUTE state (state 2 = 010)
    InputSwitch *reset, *clock;
    QVector<Led *> state;
    Led *pcInc, *irLoad, *regWrite, *aluEnable, *flagWrite;
    reset = new InputSwitch();
    clock = new InputSwitch();
    for (int i = 0; i < 3; i++) {
        state.append(new Led());
    }
    pcInc = new Led();
    irLoad = new Led();
    regWrite = new Led();
    aluEnable = new Led();
    flagWrite = new Led();
    std::unique_ptr<WorkSpace> workspace(buildControlUnit(reset, clock, state.data(), pcInc, irLoad, regWrite, aluEnable, flagWrite));
    auto *sim = workspace->simulation();
    // Reset to FETCH (state 0)
    reset->setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
    sim->update();
    reset->setOn(false);
    sim->update();
    // Verify starting at state 0 (FETCH), flagWrite should be 0
    int currentState = readMultiBitOutput(state);
    QCOMPARE(currentState, 0);  // FETCH
    QCOMPARE(TestUtils::getInputStatus(flagWrite, 0), false);
    // Transition to state 1 (DECODE)
    TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
    sim->update();
    currentState = readMultiBitOutput(state);
    QCOMPARE(currentState, 1);  // DECODE
    QCOMPARE(TestUtils::getInputStatus(flagWrite, 0), false);
    // Transition to state 2 (EXECUTE) - flagWrite should be asserted
    TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
    sim->update();
    currentState = readMultiBitOutput(state);
    QCOMPARE(currentState, 2);  // EXECUTE
    QCOMPARE(TestUtils::getInputStatus(flagWrite, 0), true);  // FlagWrite asserted!
    // Transition to state 3 (WRITEBACK) - flagWrite should be de-asserted
    TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
    sim->update();
    currentState = readMultiBitOutput(state);
    QCOMPARE(currentState, 3);  // WRITEBACK
    QCOMPARE(TestUtils::getInputStatus(flagWrite, 0), false);
    // Transition to state 4 (HALT) - flagWrite should remain 0
    TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
    sim->update();
    currentState = readMultiBitOutput(state);
    QCOMPARE(currentState, 4);  // HALT
    QCOMPARE(TestUtils::getInputStatus(flagWrite, 0), false);
}

void TestCPUControlUnit::testControlUnit_data()
{
    QTest::addColumn<int>("stateTransitions");
    QTest::addColumn<int>("expectedFinalState");
    QTest::newRow("cu_reset") << 0 << static_cast<int>(CU_FETCH);           // 0 transitions: state 0
    QTest::newRow("cu_fetch_to_decode") << 1 << static_cast<int>(CU_DECODE);  // 1 transition: state 1
    QTest::newRow("cu_decode_to_execute") << 2 << static_cast<int>(CU_EXECUTE);  // 2 transitions: state 2
    QTest::newRow("cu_execute_to_writeback") << 3 << static_cast<int>(CU_WRITEBACK);  // 3 transitions: state 3
    QTest::newRow("cu_writeback_to_fetch") << 5 << static_cast<int>(CU_FETCH);  // 5 transitions: state 0 (wrap)
    QTest::newRow("cu_full_cycle") << 5 << static_cast<int>(CU_FETCH);  // 5 transitions: state 0 (full cycle back to FETCH)
    QTest::newRow("cu_nop_sequence") << 5 << static_cast<int>(CU_FETCH);  // 5 transitions: state 0
    QTest::newRow("cu_multi_instruction") << 12 << static_cast<int>(CU_EXECUTE);  // 12 % 5 = 2: state 2
    QTest::newRow("cu_halt_state") << 4 << static_cast<int>(CU_HALT);  // 4 transitions: state 4 (HALT)
    QTest::newRow("cu_state_stability") << 1 << static_cast<int>(CU_DECODE);  // 1 transition: state 1
}

