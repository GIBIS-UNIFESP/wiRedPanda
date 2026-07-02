// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/Cpu/TestCPUControlUnit.h"

#include "Tests/Integration/IC/Tests/Cpu/CpuHelpers.h"

using TestUtils::clockCycle;
using TestUtils::inputStatus;
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
    // Exact comparison: the 5-state machine wraps 4->0, so after N transitions
    // the state is N % 5 (already encoded in expectedFinalState). The previous
    // "currentState % 5" masked a state that wrongly ran past 4 (e.g. 5,6,7).
    QCOMPARE(currentState, expectedFinalState);
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
    QCOMPARE(TestUtils::inputStatus(flagWrite, 0), false);
    // Transition to state 1 (DECODE)
    TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
    sim->update();
    currentState = readMultiBitOutput(state);
    QCOMPARE(currentState, 1);  // DECODE
    QCOMPARE(TestUtils::inputStatus(flagWrite, 0), false);
    // Transition to state 2 (EXECUTE) - flagWrite should be asserted
    TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
    sim->update();
    currentState = readMultiBitOutput(state);
    QCOMPARE(currentState, 2);  // EXECUTE
    QCOMPARE(TestUtils::inputStatus(flagWrite, 0), true);  // FlagWrite asserted!
    // Transition to state 3 (WRITEBACK) - flagWrite should be de-asserted
    TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
    sim->update();
    currentState = readMultiBitOutput(state);
    QCOMPARE(currentState, 3);  // WRITEBACK
    QCOMPARE(TestUtils::inputStatus(flagWrite, 0), false);
    // Transition to state 4 (HALT) - flagWrite should remain 0
    TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
    sim->update();
    currentState = readMultiBitOutput(state);
    QCOMPARE(currentState, 4);  // HALT
    QCOMPARE(TestUtils::inputStatus(flagWrite, 0), false);
}

void TestCPUControlUnit::testControlUnitSignals()
{
    // Each control output must assert in exactly one state (it was wired to a raw
    // state bit before, so e.g. aluEnable asserted in HALT instead of EXECUTE):
    //   FETCH(0): pcInc, irLoad     EXECUTE(2): aluEnable, flagWrite
    //   WRITEBACK(3): regWrite      DECODE(1) and HALT(4): none
    InputSwitch *reset = new InputSwitch();
    InputSwitch *clock = new InputSwitch();
    QVector<Led *> state;
    for (int i = 0; i < 3; i++) state.append(new Led());
    Led *pcInc = new Led(), *irLoad = new Led(), *regWrite = new Led();
    Led *aluEnable = new Led(), *flagWrite = new Led();
    std::unique_ptr<WorkSpace> workspace(buildControlUnit(reset, clock, state.data(), pcInc, irLoad, regWrite, aluEnable, flagWrite));
    auto *sim = workspace->simulation();

    reset->setOn(true);
    sim->update();
    clockCycle(sim, clock);
    reset->setOn(false);
    sim->update();

    struct Expect { int pcInc, irLoad, aluEnable, regWrite, flagWrite; };
    const Expect expected[5] = {
        /* FETCH     */ {1, 1, 0, 0, 0},
        /* DECODE    */ {0, 0, 0, 0, 0},
        /* EXECUTE   */ {0, 0, 1, 0, 1},
        /* WRITEBACK */ {0, 0, 0, 1, 0},
        /* HALT      */ {0, 0, 0, 0, 0},
    };

    for (int s = 0; s < 5; ++s) {
        QCOMPARE(readMultiBitOutput(state), s);
        const Expect &e = expected[s];
        QCOMPARE(inputStatus(pcInc, 0), e.pcInc != 0);
        QCOMPARE(inputStatus(irLoad, 0), e.irLoad != 0);
        QCOMPARE(inputStatus(aluEnable, 0), e.aluEnable != 0);
        QCOMPARE(inputStatus(regWrite, 0), e.regWrite != 0);
        QCOMPARE(inputStatus(flagWrite, 0), e.flagWrite != 0);
        clockCycle(sim, clock);
        sim->update();
    }
}

void TestCPUControlUnit::testControlUnitResetClears()
{
    // Reset must clear the state from a non-zero value, not merely hold it. The
    // earlier mux tied port 0 to Q, so reset held the current state and only
    // looked correct because the register powers up at 0.
    InputSwitch *reset = new InputSwitch();
    InputSwitch *clock = new InputSwitch();
    QVector<Led *> state;
    for (int i = 0; i < 3; i++) state.append(new Led());
    Led *pcInc = new Led(), *irLoad = new Led(), *regWrite = new Led();
    Led *aluEnable = new Led(), *flagWrite = new Led();
    std::unique_ptr<WorkSpace> workspace(buildControlUnit(reset, clock, state.data(), pcInc, irLoad, regWrite, aluEnable, flagWrite));
    auto *sim = workspace->simulation();

    // Advance to a non-zero state (EXECUTE = 2).
    reset->setOn(false);
    sim->update();
    clockCycle(sim, clock); // -> 1
    clockCycle(sim, clock); // -> 2
    sim->update();
    QCOMPARE(readMultiBitOutput(state), 2);

    // Assert reset and clock: state must return to FETCH (0).
    reset->setOn(true);
    sim->update();
    clockCycle(sim, clock);
    sim->update();
    QCOMPARE(readMultiBitOutput(state), 0);
}

void TestCPUControlUnit::testControlUnit_data()
{
    QTest::addColumn<int>("stateTransitions");
    QTest::addColumn<int>("expectedFinalState");
    QTest::newRow("cu_reset") << 0 << static_cast<int>(CU_FETCH);           // 0 transitions: state 0
    QTest::newRow("cu_fetch_to_decode") << 1 << static_cast<int>(CU_DECODE);  // 1 transition: state 1
    QTest::newRow("cu_decode_to_execute") << 2 << static_cast<int>(CU_EXECUTE);  // 2 transitions: state 2
    QTest::newRow("cu_execute_to_writeback") << 3 << static_cast<int>(CU_WRITEBACK);  // 3 transitions: state 3
    QTest::newRow("cu_full_cycle_wrap") << 5 << static_cast<int>(CU_FETCH);  // 5 transitions: wraps 4->0
    QTest::newRow("cu_multi_instruction") << 12 << static_cast<int>(CU_EXECUTE);  // 12 % 5 = 2: state 2
    QTest::newRow("cu_halt_state") << 4 << static_cast<int>(CU_HALT);  // 4 transitions: state 4 (HALT)
    // (removed cu_full_cycle and cu_nop_sequence: identical to cu_full_cycle_wrap.)
}
