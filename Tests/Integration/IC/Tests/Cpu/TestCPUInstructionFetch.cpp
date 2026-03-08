// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/Cpu/TestCPUInstructionFetch.h"

#include "Tests/Integration/IC/Tests/Cpu/CpuHelpers.h"

using TestUtils::clockCycle;
using TestUtils::readMultiBitOutput;

// ============================================================
// Consolidated Test Group: InstructionFetch
// ============================================================
// This consolidated test class merges the following test classes:
// - TestCPU23InstructionFetchBasic
// - TestCPU24InstructionFetchEdgeCases
// - TestCPU25InstructionFetchSequences
// - TestCPU26InstructionFetch
//

void TestCPUInstructionFetch::initTestCase()
{
    TestUtils::setupTestEnvironment();
}

void TestCPUInstructionFetch::cleanup()
{
}

void TestCPUInstructionFetch::testInstructionFetch()
{
    QFETCH(int, initialPC);
    QFETCH(int, cyclesPerFetch);
    // This is an integration test combining PC + Memory + IR
    QVector<InputSwitch *> pcLoadVal;
    QVector<InputSwitch *> memAddr;
    QVector<InputSwitch *> memDataIn;
    QVector<Led *> pcOut;
    QVector<Led *> irOut;
    QVector<Led *> memDataOut;
    InputSwitch *pcLoad, *pcInc, *pcReset, *pcEnable;
    InputSwitch *clock;
    for (int i = 0; i < 8; i++) {
        pcLoadVal.append(new InputSwitch());
        memAddr.append(new InputSwitch());
        memDataIn.append(new InputSwitch());
        pcOut.append(new Led());
        irOut.append(new Led());
        memDataOut.append(new Led());
    }
    pcLoad = new InputSwitch();
    pcInc = new InputSwitch();
    pcReset = new InputSwitch();
    pcEnable = new InputSwitch();
    clock = new InputSwitch();
    // Build integrated fetch circuit
    std::unique_ptr<WorkSpace> workspace(buildProgramCounter8bit(pcLoadVal.data(), pcLoad, pcInc, pcReset, pcEnable, clock, pcOut.data()));
    auto *sim = workspace->simulation();
    // Initialize PC enable (required for PC operations)
    pcEnable->setOn(true);
    pcInc->setOn(false);
    pcLoad->setOn(false);
    pcReset->setOn(false);
    sim->update();
    // Set initial PC value
    for (int i = 0; i < 8; i++) {
        pcLoadVal[i]->setOn((initialPC >> i) & 1);
    }
    sim->update();
    // Load initial PC value (requires two clock cycles for complete load)
    pcLoad->setOn(true);
    sim->update();
    clockCycle(sim, clock);  // Complete clock cycle (HIGH->settle->LOW->settle)
    sim->update();
    pcLoad->setOn(false);
    sim->update();
    // Run fetch cycles (increment PC)
    pcInc->setOn(true);
    sim->update();
    for (int i = 0; i < cyclesPerFetch; i++) {
        // Complete clock cycle (HIGH->settle->LOW->settle)
        clockCycle(sim, clock);
        sim->update();
    }
    pcInc->setOn(false);
    sim->update();
    // Verify PC has incremented exactly by cyclesPerFetch (with 8-bit wrap)
    int currentPC = readMultiBitOutput(pcOut);
    int expectedPC = (initialPC + cyclesPerFetch) & 0xFF;
    QCOMPARE(currentPC, expectedPC);
}

void TestCPUInstructionFetch::testInstructionFetch_data()
{
    QTest::addColumn<int>("initialPC");
    QTest::addColumn<int>("cyclesPerFetch");
    QTest::newRow("fetch_single_addr0") << 0x00 << 1;
    QTest::newRow("fetch_single_addr10") << 0x10 << 1;
    QTest::newRow("fetch_sequence_3") << 0x00 << 3;
    QTest::newRow("fetch_from_mid_addr") << 0x80 << 1;
    QTest::newRow("fetch_wrap_ff") << 0xFF << 1;
    QTest::newRow("fetch_increment_test") << 0x00 << 2;
    QTest::newRow("fetch_hold_enable") << 0x00 << 1;
    QTest::newRow("fetch_timing") << 0x00 << 1;
}

