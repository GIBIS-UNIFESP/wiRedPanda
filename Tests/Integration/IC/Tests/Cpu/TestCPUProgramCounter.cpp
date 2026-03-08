// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/Cpu/TestCPUProgramCounter.h"

#include "Tests/Integration/IC/Tests/Cpu/CpuHelpers.h"

using TestUtils::clockCycle;
using TestUtils::readMultiBitOutput;

// ============================================================
// Consolidated Test Group: ProgramCounter
// ============================================================
// This consolidated test class merges the following test classes:
// - TestCPU4ProgramCounterBasic
// - TestCPU5ProgramCounterLoad
// - TestCPU6ProgramCounterSequences
//

void TestCPUProgramCounter::initTestCase()
{
    TestUtils::setupTestEnvironment();
}

void TestCPUProgramCounter::cleanup()
{
}

void TestCPUProgramCounter::testProgramCounterBasic()
{
    QFETCH(int, initialValue);
    QFETCH(bool, doReset);
    QFETCH(bool, doLoad);
    QFETCH(int, loadValue);
    QFETCH(int, incrementCount);
    QFETCH(int, expectedResult);
    QVector<InputSwitch *> loadValueBits;
    InputSwitch *load, *inc, *reset, *enable, *clock;
    QVector<Led *> pcBits;
    for (int i = 0; i < 8; i++) {
        loadValueBits.append(new InputSwitch());
        pcBits.append(new Led());
    }
    load = new InputSwitch();
    inc = new InputSwitch();
    reset = new InputSwitch();
    enable = new InputSwitch();
    clock = new InputSwitch();
    std::unique_ptr<WorkSpace> workspace(buildProgramCounter8bit(loadValueBits.data(), load, inc, reset, enable, clock, pcBits.data()));
    auto *sim = workspace->simulation();
    // Test setup - set initial load values using setOn(bool) instead of setMultiBitInput
    for (int i = 0; i < 8; i++) {
        bool bit = (initialValue >> i) & 1;
        loadValueBits[i]->setOn(bit);
    }
    sim->update();
    // If initialValue != 0, we need to load it into the PC (flip-flops default to 0)
    if (initialValue != 0) {
        load->setOn(true);
        sim->update();
        TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
        sim->update();
        load->setOn(false);
        sim->update();
    }
    // Apply reset if needed
    if (doReset) {
        reset->setOn(true);
        sim->update();
        TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
        sim->update();
        reset->setOn(false);
        sim->update();
    }
    // Apply load if needed
    if (doLoad) {
        // Set loadValue bits directly using setOn(bool) instead of setMultiBitInput
        for (int i = 0; i < 8; i++) {
            bool bit = (loadValue >> i) & 1;
            loadValueBits[i]->setOn(bit);  // Use setOn(bool) version
        }
        sim->update();
        load->setOn(true);
        sim->update();
        TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
        sim->update();
        load->setOn(false);
        sim->update();
    }
    // Apply increments
    if (incrementCount > 0) {
        inc->setOn(true);
        sim->update();
        for (int i = 0; i < incrementCount; i++) {
            // Complete clock cycle (HIGH→settle→LOW→settle)
            // Note: clockCycle() performs a complete pulse in one call
            TestUtils::clockCycle(sim, clock);
            sim->update();
        }
        inc->setOn(false);
        sim->update();
    }
    sim->update();
    int result = readMultiBitOutput(pcBits);
    QCOMPARE(result, expectedResult);
}

void TestCPUProgramCounter::testProgramCounterBasic_data()
{
    QTest::addColumn<int>("initialValue");
    QTest::addColumn<bool>("doReset");
    QTest::addColumn<bool>("doLoad");
    QTest::addColumn<int>("loadValue");
    QTest::addColumn<int>("incrementCount");
    QTest::addColumn<int>("expectedResult");
    // Test 1: Reset
    QTest::newRow("pc_reset") << 0 << true << false << 0 << 0 << 0;
    // Test 2: Single increment
    QTest::newRow("pc_increment_0_to_1") << 0 << false << false << 0 << 1 << 1;
    // Test 3: Multiple increments
    QTest::newRow("pc_increment_sequence") << 0 << false << false << 0 << 3 << 3;
    // Test 7: Hold (no increment)
    QTest::newRow("pc_hold") << 0x10 << false << false << 0 << 0 << 0x10;
}

void TestCPUProgramCounter::testProgramCounterLoad()
{
    QFETCH(int, initialValue);
    QFETCH(bool, doReset);
    QFETCH(bool, doLoad);
    QFETCH(int, loadValue);
    QFETCH(int, incrementCount);
    QFETCH(int, expectedResult);
    QVector<InputSwitch *> loadValueBits;
    InputSwitch *load, *inc, *reset, *enable, *clock;
    QVector<Led *> pcBits;
    for (int i = 0; i < 8; i++) {
        loadValueBits.append(new InputSwitch());
        pcBits.append(new Led());
    }
    load = new InputSwitch();
    inc = new InputSwitch();
    reset = new InputSwitch();
    enable = new InputSwitch();
    clock = new InputSwitch();
    std::unique_ptr<WorkSpace> workspace(buildProgramCounter8bit(loadValueBits.data(), load, inc, reset, enable, clock, pcBits.data()));
    auto *sim = workspace->simulation();
    // Test setup - set initial load values using setOn(bool) instead of setMultiBitInput
    for (int i = 0; i < 8; i++) {
        bool bit = (initialValue >> i) & 1;
        loadValueBits[i]->setOn(bit);
    }
    sim->update();
    // If initialValue != 0, we need to load it into the PC (flip-flops default to 0)
    if (initialValue != 0) {
        load->setOn(true);
        sim->update();
        TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
        sim->update();
        load->setOn(false);
        sim->update();
    }
    // Apply reset if needed
    if (doReset) {
        reset->setOn(true);
        sim->update();
        TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
        sim->update();
        reset->setOn(false);
        sim->update();
    }
    // Apply load if needed
    if (doLoad) {
        // Set loadValue bits directly using setOn(bool) instead of setMultiBitInput
        for (int i = 0; i < 8; i++) {
            bool bit = (loadValue >> i) & 1;
            loadValueBits[i]->setOn(bit);  // Use setOn(bool) version
        }
        sim->update();
        load->setOn(true);
        sim->update();
        TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
        sim->update();
        load->setOn(false);
        sim->update();
    }
    // Apply increments
    if (incrementCount > 0) {
        inc->setOn(true);
        sim->update();
        for (int i = 0; i < incrementCount; i++) {
            // Complete clock cycle (HIGH→settle→LOW→settle)
            // Note: clockCycle() performs a complete pulse in one call
            TestUtils::clockCycle(sim, clock);
            sim->update();
        }
        inc->setOn(false);
        sim->update();
    }
    sim->update();
    int result = readMultiBitOutput(pcBits);
    QCOMPARE(result, expectedResult);
}

void TestCPUProgramCounter::testProgramCounterLoad_data()
{
    QTest::addColumn<int>("initialValue");
    QTest::addColumn<bool>("doReset");
    QTest::addColumn<bool>("doLoad");
    QTest::addColumn<int>("loadValue");
    QTest::addColumn<int>("incrementCount");
    QTest::addColumn<int>("expectedResult");
    // Test 4: Load value
    QTest::newRow("pc_load_0x42") << 0 << false << true << 0x42 << 0 << 0x42;
    // Test 5: Load then increment
    QTest::newRow("pc_load_then_inc") << 0 << false << true << 0x10 << 1 << 0x11;
}

void TestCPUProgramCounter::testProgramCounterPriority()
{
    QFETCH(bool, reset);
    QFETCH(bool, load);
    QFETCH(bool, inc);
    QFETCH(int, loadValue);
    QFETCH(int, expectedResult);
    QVector<InputSwitch *> loadValueBits;
    InputSwitch *loadSwitch, *incSwitch, *resetSwitch, *enable, *clock;
    QVector<Led *> pcBits;
    for (int i = 0; i < 8; i++) {
        loadValueBits.append(new InputSwitch());
        pcBits.append(new Led());
    }
    loadSwitch = new InputSwitch();
    incSwitch = new InputSwitch();
    resetSwitch = new InputSwitch();
    enable = new InputSwitch();
    clock = new InputSwitch();
    std::unique_ptr<WorkSpace> workspace(buildProgramCounter8bit(loadValueBits.data(), loadSwitch, incSwitch, resetSwitch, enable, clock, pcBits.data()));
    auto *sim = workspace->simulation();
    // Set up load value
    for (int i = 0; i < 8; i++) {
        bool bit = (loadValue >> i) & 1;
        loadValueBits[i]->setOn(bit);
    }
    sim->update();
    // Test simultaneous control activation (NOT sequential)
    // Set all controls that should be active
    resetSwitch->setOn(reset);
    loadSwitch->setOn(load);
    incSwitch->setOn(inc);
    sim->update();
    // Execute one clock cycle with all controls active
    TestUtils::clockCycle(sim, clock);
    sim->update();
    // Read result and verify priority
    int result = readMultiBitOutput(pcBits);
    QCOMPARE(result, expectedResult);
}

void TestCPUProgramCounter::testProgramCounterPriority_data()
{
    QTest::addColumn<bool>("reset");
    QTest::addColumn<bool>("load");
    QTest::addColumn<bool>("inc");
    QTest::addColumn<int>("loadValue");
    QTest::addColumn<int>("expectedResult");
    // Test simultaneous control activation to verify priority: Reset > Load > Inc
    // Test 1: Reset=1, Load=0, Inc=0
    QTest::newRow("priority_reset_alone") << true << false << false << 0x00 << 0x00;
    // Test 2: Reset=1, Load=1, Inc=0 (Reset > Load)
    QTest::newRow("priority_reset_over_load") << true << true << false << 0x42 << 0x00;
    // Test 3: Reset=1, Load=0, Inc=1 (Reset > Inc)
    QTest::newRow("priority_reset_over_inc") << true << false << true << 0x00 << 0x00;
    // Test 4: Reset=1, Load=1, Inc=1 (Reset > Load > Inc)
    QTest::newRow("priority_reset_over_all") << true << true << true << 0x42 << 0x00;
    // Test 5: Reset=0, Load=1, Inc=0
    QTest::newRow("priority_load_alone") << false << true << false << 0x42 << 0x42;
    // Test 6: Reset=0, Load=1, Inc=1 (Load > Inc)
    QTest::newRow("priority_load_over_inc") << false << true << true << 0x42 << 0x42;
    // Test 7: Reset=0, Load=0, Inc=1
    QTest::newRow("priority_inc_alone") << false << false << true << 0x00 << 0x01;
    // Test 8: Reset=0, Load=0, Inc=0
    QTest::newRow("priority_hold") << false << false << false << 0x00 << 0x00;
}

void TestCPUProgramCounter::testProgramCounterSequences()
{
    QFETCH(int, initialValue);
    QFETCH(bool, doReset);
    QFETCH(bool, doLoad);
    QFETCH(int, loadValue);
    QFETCH(int, incrementCount);
    QFETCH(int, expectedResult);
    QVector<InputSwitch *> loadValueBits;
    InputSwitch *load, *inc, *reset, *enable, *clock;
    QVector<Led *> pcBits;
    for (int i = 0; i < 8; i++) {
        loadValueBits.append(new InputSwitch());
        pcBits.append(new Led());
    }
    load = new InputSwitch();
    inc = new InputSwitch();
    reset = new InputSwitch();
    enable = new InputSwitch();
    clock = new InputSwitch();
    std::unique_ptr<WorkSpace> workspace(buildProgramCounter8bit(loadValueBits.data(), load, inc, reset, enable, clock, pcBits.data()));
    auto *sim = workspace->simulation();
    // Test setup - set initial load values using setOn(bool) instead of setMultiBitInput
    for (int i = 0; i < 8; i++) {
        bool bit = (initialValue >> i) & 1;
        loadValueBits[i]->setOn(bit);
    }
    sim->update();
    // If initialValue != 0, we need to load it into the PC (flip-flops default to 0)
    if (initialValue != 0) {
        load->setOn(true);
        sim->update();
        TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
        sim->update();
        load->setOn(false);
        sim->update();
    }
    // Apply reset if needed
    if (doReset) {
        reset->setOn(true);
        sim->update();
        TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
        sim->update();
        reset->setOn(false);
        sim->update();
    }
    // Apply load if needed
    if (doLoad) {
        // Set loadValue bits directly using setOn(bool) instead of setMultiBitInput
        for (int i = 0; i < 8; i++) {
            bool bit = (loadValue >> i) & 1;
            loadValueBits[i]->setOn(bit);  // Use setOn(bool) version
        }
        sim->update();
        load->setOn(true);
        sim->update();
        TestUtils::clockCycle(sim, clock);  // Complete clock cycle (HIGH→settle→LOW→settle)
        sim->update();
        load->setOn(false);
        sim->update();
    }
    // Apply increments
    if (incrementCount > 0) {
        inc->setOn(true);
        sim->update();
        for (int i = 0; i < incrementCount; i++) {
            // Complete clock cycle (HIGH→settle→LOW→settle)
            // Note: clockCycle() performs a complete pulse in one call
            TestUtils::clockCycle(sim, clock);
            sim->update();
        }
        inc->setOn(false);
        sim->update();
    }
    sim->update();
    int result = readMultiBitOutput(pcBits);
    QCOMPARE(result, expectedResult);
}

void TestCPUProgramCounter::testProgramCounterSequences_data()
{
    QTest::addColumn<int>("initialValue");
    QTest::addColumn<bool>("doReset");
    QTest::addColumn<bool>("doLoad");
    QTest::addColumn<int>("loadValue");
    QTest::addColumn<int>("incrementCount");
    QTest::addColumn<int>("expectedResult");
    // Test 6: Overflow (0xFF → 0x00)
    QTest::newRow("pc_overflow") << 0xFF << false << false << 0 << 1 << 0x00;
    // Test 8: Complex sequence
    QTest::newRow("pc_sequence") << 0 << false << true << 0x10 << 5 << 0x15;
}
