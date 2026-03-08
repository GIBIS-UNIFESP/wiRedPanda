// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel2ParityChecker.h"

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel2ParityChecker::initTestCase()
{
}

void TestLevel2ParityChecker::cleanup()
{
}

void TestLevel2ParityChecker::testOddParityChecker_data()
{
    QTest::addColumn<int>("dataInput");    // 4 data bits (D0-D3)
    QTest::addColumn<int>("parityBit");    // 1 parity bit
    QTest::addColumn<int>("expectedResult"); // 1 = valid odd parity, 0 = error detected

    // Valid odd parity cases: total 1's should be ODD
    // Data: 0000, Parity: 1 (total = 1 one = ODD) ✓
    QTest::newRow("0000 + P1 = valid") << 0x0 << 1 << 1;
    // Data: 0001, Parity: 0 (total = 1 one = ODD) ✓
    QTest::newRow("0001 + P0 = valid") << 0x1 << 0 << 1;
    // Data: 0011, Parity: 0 (total = 2 ones = EVEN) ✗ (but with parity → 2 = error)
    QTest::newRow("0011 + P0 = error") << 0x3 << 0 << 0;
    // Data: 0011, Parity: 1 (total = 3 ones = ODD) ✓
    QTest::newRow("0011 + P1 = valid") << 0x3 << 1 << 1;
    // Data: 0101, Parity: 1 (total = 3 ones = ODD) ✓
    QTest::newRow("0101 + P1 = valid") << 0x5 << 1 << 1;
    // Data: 0111, Parity: 0 (total = 3 ones = ODD) ✓
    QTest::newRow("0111 + P0 = valid") << 0x7 << 0 << 1;
    // Data: 1111, Parity: 1 (total = 5 ones = ODD) ✓
    QTest::newRow("1111 + P1 = valid") << 0xF << 1 << 1;
    // Data: 1111, Parity: 0 (total = 4 ones = EVEN) ✗
    QTest::newRow("1111 + P0 = error") << 0xF << 0 << 0;
    // Data: 0000, Parity: 0 (total = 0 ones = EVEN) ✗
    QTest::newRow("0000 + P0 = error") << 0x0 << 0 << 0;
    // Data: 1001, Parity: 0 (total = 2 ones = EVEN) ✗
    QTest::newRow("1001 + P0 = error") << 0x9 << 0 << 0;
    // Data: 1010, Parity: 1 (total = 3 ones = ODD) ✓
    QTest::newRow("1010 + P1 = valid") << 0xA << 1 << 1;
}

void TestLevel2ParityChecker::testOddParityChecker()
{
    QFETCH(int, dataInput);
    QFETCH(int, parityBit);
    QFETCH(int, expectedResult);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Input switches: D0, D1, D2, D3 (data), P (parity)
    InputSwitch swD0, swD1, swD2, swD3, swP;

    // Output LED
    Led ledResult;

    // Add all input elements
    builder.add(&swD0, &swD1, &swD2, &swD3, &swP);
    builder.add(&ledResult);

    // Set input values
    swD0.setOn(static_cast<bool>(dataInput & 0x1));
    swD1.setOn(static_cast<bool>(dataInput & 0x2));
    swD2.setOn(static_cast<bool>(dataInput & 0x4));
    swD3.setOn(static_cast<bool>(dataInput & 0x8));
    swP.setOn(static_cast<bool>(parityBit));

    // Load ParityChecker IC
    IC *parityIC = loadBuildingBlockIC("level2_parity_checker.panda");
    builder.add(parityIC);

    // Connect data inputs to IC (ParityChecker has 8 inputs: data[0..7])
    // We only use the first 5 inputs for our 5-input parity check (D0-D3 + P)
    builder.connect(&swD0, 0, parityIC, "data[0]");
    builder.connect(&swD1, 0, parityIC, "data[1]");
    builder.connect(&swD2, 0, parityIC, "data[2]");
    builder.connect(&swD3, 0, parityIC, "data[3]");
    builder.connect(&swP, 0, parityIC, "data[4]");
    // data[5], data[6], data[7] are left unconnected (treated as 0)

    // Connect parity output to LED
    builder.connect(parityIC, "parity", &ledResult, 0);

    auto *simulation = builder.initSimulation();
    simulation->update();

    // Test parity check result
    QCOMPARE(getInputStatus(&ledResult) ? 1 : 0, expectedResult);
}

