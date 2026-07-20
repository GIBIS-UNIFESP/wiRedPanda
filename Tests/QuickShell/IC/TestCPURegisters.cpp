// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/IC/TestCPURegisters.h"

#include "Tests/QuickShell/IC/QuickCpuHelpers.h"

using TestUtils::clockCycle;
using TestUtils::inputStatus;
using TestUtils::readMultiBitOutput;

// ============================================================
// Consolidated Test Group: Registers
// ============================================================
// This consolidated test class merges the following test classes:
// - TestCPU1SingleGatedRegister
// - TestCPU2FlagRegister
// - TestCPU3InstructionRegister
//

void TestCPURegisters::testFlagRegister()
{
    QFETCH(bool, zeroIn);
    QFETCH(bool, negativeIn);
    QFETCH(bool, writeEnable);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);

    TestUtils::OwnedElementPool pool;
    InputSwitch *zeroInput = pool.make<InputSwitch>();
    InputSwitch *negativeInput = pool.make<InputSwitch>();
    InputSwitch *flagWrite = pool.make<InputSwitch>();
    InputSwitch *clock = pool.make<InputSwitch>();
    Led *zeroOut = pool.make<Led>();
    Led *negativeOut = pool.make<Led>();

    auto builder = buildFlagRegister(zeroInput, negativeInput, flagWrite, clock, zeroOut, negativeOut);
    auto *sim = builder->simulation();
    // Initialize - all inputs off
    zeroInput->setOn(false);
    negativeInput->setOn(false);
    flagWrite->setOn(false);
    clock->setOn(false);
    sim->update();
    // Set input values
    zeroInput->setOn(zeroIn);
    negativeInput->setOn(negativeIn);
    flagWrite->setOn(writeEnable);
    sim->update();
    // Clock cycle to latch values
    TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
    sim->update();
    // Disable write enable to test latching
    flagWrite->setOn(false);
    sim->update();
    // Verify outputs
    bool zeroResult = TestUtils::inputStatus(zeroOut, 0);
    bool negativeResult = TestUtils::inputStatus(negativeOut, 0);
    QCOMPARE(zeroResult, expectedZero);
    QCOMPARE(negativeResult, expectedNegative);
}

void TestCPURegisters::testFlagRegister_data()
{
    QTest::addColumn<bool>("zeroIn");
    QTest::addColumn<bool>("negativeIn");
    QTest::addColumn<bool>("writeEnable");
    QTest::addColumn<bool>("expectedZero");
    QTest::addColumn<bool>("expectedNegative");
    // Test 1: Store zero=1, negative=0
    QTest::newRow("store_zero_flag") << true << false << true << true << false;
    // Test 2: Store zero=0, negative=1
    QTest::newRow("store_negative_flag") << false << true << true << false << true;
    // Test 3: Store both flags
    QTest::newRow("store_both_flags") << true << true << true << true << true;
    // Test 4: Store neither flag
    QTest::newRow("store_no_flags") << false << false << true << false << false;
    // Test 5: Write disable - no change (flags should be 0 initially)
    QTest::newRow("write_disabled") << true << true << false << false << false;
    // (the former "flag_persistence" row duplicated "store_zero_flag"; real
    //  cross-cycle persistence is covered by testFlagRegisterPersistence below.)
}

void TestCPURegisters::testFlagRegisterPersistence()
{
    // Store a known non-zero flag state, then drive the OPPOSITE inputs while
    // write is disabled and clock repeatedly: the latched values must persist.
    // (A transparent latch or a missing write-gate would leak the new inputs.)
    TestUtils::OwnedElementPool pool;
    InputSwitch *zeroInput = pool.make<InputSwitch>();
    InputSwitch *negativeInput = pool.make<InputSwitch>();
    InputSwitch *flagWrite = pool.make<InputSwitch>();
    InputSwitch *clock = pool.make<InputSwitch>();
    Led *zeroOut = pool.make<Led>();
    Led *negativeOut = pool.make<Led>();

    auto builder = buildFlagRegister(zeroInput, negativeInput, flagWrite, clock, zeroOut, negativeOut);
    auto *sim = builder->simulation();

    // Store zero=1, negative=0 with write enabled.
    zeroInput->setOn(true);
    negativeInput->setOn(false);
    flagWrite->setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, clock);
    sim->update();
    QCOMPARE(TestUtils::inputStatus(zeroOut, 0), true);
    QCOMPARE(TestUtils::inputStatus(negativeOut, 0), false);

    // Disable write, present the opposite inputs, and clock several times.
    flagWrite->setOn(false);
    zeroInput->setOn(false);
    negativeInput->setOn(true);
    sim->update();
    for (int i = 0; i < 3; ++i) {
        TestUtils::clockCycle(sim, clock);
        sim->update();
    }
    // Stored state must be retained, not overwritten by the new inputs.
    QCOMPARE(TestUtils::inputStatus(zeroOut, 0), true);
    QCOMPARE(TestUtils::inputStatus(negativeOut, 0), false);
}

void TestCPURegisters::testInstructionRegister()
{
    QFETCH(int, instructionValue);
    QFETCH(bool, shouldLoad);
    QFETCH(int, expectedIR);

    TestUtils::OwnedElementPool pool;
    QVector<InputSwitch *> instruction;
    QVector<Led *> ir;
    for (int i = 0; i < 8; i++) {
        instruction.append(pool.make<InputSwitch>());
        ir.append(pool.make<Led>());
    }
    InputSwitch *load = pool.make<InputSwitch>();
    InputSwitch *clock = pool.make<InputSwitch>();

    auto builder = buildInstructionRegister8bit(instruction.data(), load, clock, ir.data());
    auto *sim = builder->simulation();
    // Load initial value using setOn(bool)
    for (int i = 0; i < 8; i++) {
        instruction[i]->setOn((0x55 >> i) & 1);
    }
    load->setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
    sim->update();
    load->setOn(false);
    sim->update();
    if (shouldLoad) {
        // Set instruction using setOn(bool)
        for (int i = 0; i < 8; i++) {
            instruction[i]->setOn((instructionValue >> i) & 1);
        }
        sim->update();
        load->setOn(true);
        sim->update();
        TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
        sim->update();
        load->setOn(false);
        sim->update();
    } else {
        // Test hold: first load the expected value, then verify hold works
        for (int i = 0; i < 8; i++) {
            instruction[i]->setOn((instructionValue >> i) & 1);
        }
        sim->update();
        load->setOn(true);
        sim->update();
        TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
        sim->update();
        // Now test hold: disable load and verify value doesn't change
        load->setOn(false);
        sim->update();
        TestUtils::clockCycle(sim, clock);  // Rising edge (with load disabled)
        sim->update();
        TestUtils::clockCycle(sim, clock);  // Falling edge
        sim->update();
    }
    sim->update();
    int result = readMultiBitOutput(ir);
    QCOMPARE(result, expectedIR);
}

void TestCPURegisters::testInstructionRegister_data()
{
    QTest::addColumn<int>("instructionValue");
    QTest::addColumn<bool>("shouldLoad");
    QTest::addColumn<int>("expectedIR");
    // Test 1: Load 0xA5
    QTest::newRow("ir_load_0xA5") << 0xA5 << true << 0xA5;
    // Test 2: Load 0x11
    QTest::newRow("ir_load_0x11") << 0x11 << true << 0x11;
    // Test 3: Load 0xFF
    QTest::newRow("ir_load_0xFF") << 0xFF << true << 0xFF;
    // Test 4: Load 0x00
    QTest::newRow("ir_load_0x00") << 0x00 << true << 0x00;
    // Test 5: Sequential loads
    QTest::newRow("ir_sequential") << 0x22 << true << 0x22;
    // Test 6: Hold (no load)
    QTest::newRow("ir_hold") << 0x42 << false << 0x42;
}

void TestCPURegisters::testSingleGatedRegister()
{
    TestUtils::OwnedElementPool pool;
    InputSwitch *writeData[8];
    Led *readOut[8];
    for (int i = 0; i < 8; i++) {
        writeData[i] = pool.make<InputSwitch>();
        readOut[i] = pool.make<Led>();
    }
    InputSwitch *decoderOut = pool.make<InputSwitch>();
    InputSwitch *writeEnable = pool.make<InputSwitch>();
    InputSwitch *clock = pool.make<InputSwitch>();

    auto builder = buildSingleGatedRegister(writeData, decoderOut, writeEnable, clock, readOut);
    auto *sim = builder->simulation();
    // Test 1: Write 0x42 when decoderOut is HIGH
    for (int i = 0; i < 8; i++) {
        writeData[i]->setOn((0x42 >> i) & 1);
    }
    decoderOut->setOn(true);
    writeEnable->setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, clock);
    writeEnable->setOn(false);
    sim->update();
    QVector<GraphicElement *> readVec1;
    for (int i = 0; i < 8; i++) readVec1.append(readOut[i]);
    int readValue = readMultiBitOutput(readVec1);
    // Test 2: Write different value when decoderOut is LOW (should NOT write)
    for (int i = 0; i < 8; i++) {
        writeData[i]->setOn((0x55 >> i) & 1);
    }
    decoderOut->setOn(false);  // Decoder NOT selecting this register
    writeEnable->setOn(true);
    sim->update();
    TestUtils::clockCycle(sim, clock);
    writeEnable->setOn(false);
    sim->update();
    QVector<GraphicElement *> readVec2;
    for (int i = 0; i < 8; i++) readVec2.append(readOut[i]);
    int readValue2 = readMultiBitOutput(readVec2);
    // Verify results
    QCOMPARE(readValue, 0x42);   // First write should succeed
    QCOMPARE(readValue2, 0x42);  // Second write should NOT happen
}
