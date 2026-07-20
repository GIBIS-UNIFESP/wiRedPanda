// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/IC/TestCPUInstructionFetch.h"

#include "Tests/QuickShell/IC/QuickCpuHelpers.h"

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

void TestCPUInstructionFetch::testInstructionFetch()
{
    QFETCH(int, initialPC);
    QFETCH(int, cyclesPerFetch);
    // This is an integration test combining PC + Memory + IR
    TestUtils::OwnedElementPool pool;
    QVector<InputSwitch *> pcLoadVal;
    QVector<Led *> pcOut;
    for (int i = 0; i < 8; i++) {
        pcLoadVal.append(pool.make<InputSwitch>());
        pcOut.append(pool.make<Led>());
    }
    InputSwitch *pcLoad = pool.make<InputSwitch>();
    InputSwitch *pcInc = pool.make<InputSwitch>();
    InputSwitch *pcReset = pool.make<InputSwitch>();
    InputSwitch *clock = pool.make<InputSwitch>();
    // Build integrated fetch circuit
    auto builder = buildProgramCounter8bit(pcLoadVal.data(), pcLoad, pcInc, pcReset, clock, pcOut.data());
    auto *sim = builder->simulation();
    pcInc->setOn(false);
    pcLoad->setOn(false);
    pcReset->setOn(false);
    sim->update();
    // Set initial PC value
    for (int i = 0; i < 8; i++) {
        pcLoadVal[i]->setOn((initialPC >> i) & 1);
    }
    sim->update();
    // Load initial PC value (mux->register path; completes on a single clock edge)
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
    QTest::newRow("fetch_mid_multi_cycle") << 0x40 << 4;  // mid-range start, multi-cycle
    QTest::newRow("fetch_wrap_multi") << 0xFE << 3;       // 0xFE + 3 wraps to 0x01
}
