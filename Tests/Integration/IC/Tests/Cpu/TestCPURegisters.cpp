// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/Cpu/TestCPURegisters.h"

#include "Tests/Integration/IC/Tests/Cpu/CpuHelpers.h"

using TestUtils::clockCycle;
using TestUtils::getInputStatus;
using TestUtils::readMultiBitOutput;

// ============================================================
// Consolidated Test Group: Registers
// ============================================================
// This consolidated test class merges the following test classes:
// - TestCPU1SingleGatedRegister
// - TestCPU2FlagRegister
// - TestCPU3InstructionRegister
//

void TestCPURegisters::initTestCase()
{
    // Initialize test case resources if needed
}

void TestCPURegisters::cleanup()
{
    // Clean up test resources if needed
}

void TestCPURegisters::testFlagRegister()
{
    QFETCH(bool, zeroIn);
    QFETCH(bool, negativeIn);
    QFETCH(bool, writeEnable);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);
    InputSwitch *zeroInput = new InputSwitch();
    InputSwitch *negativeInput = new InputSwitch();
    InputSwitch *flagWrite = new InputSwitch();
    InputSwitch *clock = new InputSwitch();
    Led *zeroOut = new Led();
    Led *negativeOut = new Led();
    std::unique_ptr<WorkSpace> workspace(buildFlagRegister(zeroInput, negativeInput, flagWrite, clock, zeroOut, negativeOut));
    auto *sim = workspace->simulation();
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
    bool zeroResult = TestUtils::getInputStatus(zeroOut, 0);
    bool negativeResult = TestUtils::getInputStatus(negativeOut, 0);
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
    // Test 6: Flag persistence (store then verify hold)
    QTest::newRow("flag_persistence") << true << false << true << true << false;
}

void TestCPURegisters::testInstructionRegister()
{
    QFETCH(int, instructionValue);
    QFETCH(bool, shouldLoad);
    QFETCH(int, expectedIR);
    QVector<InputSwitch *> instruction;
    InputSwitch *load, *enable, *clock;
    QVector<Led *> ir;
    for (int i = 0; i < 8; i++) {
        instruction.append(new InputSwitch());
        ir.append(new Led());
    }
    load = new InputSwitch();
    enable = new InputSwitch();
    clock = new InputSwitch();
    std::unique_ptr<WorkSpace> workspace(buildInstructionRegister8bit(instruction.data(), load, enable, clock, ir.data()));
    auto *sim = workspace->simulation();
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
    InputSwitch *writeData[8];
    InputSwitch *decoderOut, *writeEnable, *clock;
    Led *readOut[8];
    for (int i = 0; i < 8; i++) {
        writeData[i] = new InputSwitch();
        readOut[i] = new Led();
    }
    decoderOut = new InputSwitch();
    writeEnable = new InputSwitch();
    clock = new InputSwitch();
    std::unique_ptr<WorkSpace> workspace(buildSingleGatedRegister(writeData, decoderOut, writeEnable, clock, readOut));
    auto *sim = workspace->simulation();
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
