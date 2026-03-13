// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel5BarrelShifter4bit.h"

#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel5BarrelShifter4Bit::initTestCase()
{
    // Initialize test environment
}

void TestLevel5BarrelShifter4Bit::cleanup()
{
    // Clean up after each test
}

// Barrel Shifter: 4-bit arithmetic shifter (SHL/SHR)
// ============================================================================
// INPUT/OUTPUT BIT-WIDTH
// ============================================================================
// Inputs:  inputData[3:0] (data to shift), shiftAmount[1:0], shiftRight (direction)
// Output:  result[3:0] (shifted result)
// Total bit-width: 4 data bits + 2 control bits + 1 direction bit = 7 bits
// ============================================================================

void TestLevel5BarrelShifter4Bit::testBarrelShifter_data()
{
    QTest::addColumn<int>("inputData");
    QTest::addColumn<int>("shiftAmount");
    QTest::addColumn<bool>("shiftRight");  // false=left, true=right
    QTest::addColumn<int>("expectedResult");

    // Left shift operations (shiftRight = false)
    QTest::newRow("SHL: 0001 << 0 = 0001") << 1 << 0 << false << 1;
    QTest::newRow("SHL: 0001 << 1 = 0010") << 1 << 1 << false << 2;
    QTest::newRow("SHL: 0001 << 2 = 0100") << 1 << 2 << false << 4;
    QTest::newRow("SHL: 0001 << 3 = 1000") << 1 << 3 << false << 8;
    QTest::newRow("SHL: 0011 << 1 = 0110") << 3 << 1 << false << 6;
    QTest::newRow("SHL: 1111 << 1 = 1110") << 15 << 1 << false << 14;

    // Right shift operations (shiftRight = true)
    QTest::newRow("SHR: 1000 >> 0 = 1000") << 8 << 0 << true << 8;
    QTest::newRow("SHR: 1000 >> 1 = 0100") << 8 << 1 << true << 4;
    QTest::newRow("SHR: 1000 >> 2 = 0010") << 8 << 2 << true << 2;
    QTest::newRow("SHR: 1000 >> 3 = 0001") << 8 << 3 << true << 1;
    QTest::newRow("SHR: 1111 >> 1 = 0111") << 15 << 1 << true << 7;
    QTest::newRow("SHR: 1100 >> 2 = 0011") << 12 << 2 << true << 3;
}

void TestLevel5BarrelShifter4Bit::testBarrelShifter()
{
    QFETCH(int, inputData);
    QFETCH(int, shiftAmount);
    QFETCH(bool, shiftRight);
    QFETCH(int, expectedResult);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Load BarrelShifter4bit IC
    auto *shifterIC = loadBuildingBlockIC("level5_barrel_shifter_4bit.panda");
    builder.add(shifterIC);

    // Verify IC has correct port counts
    // BarrelShifter4bit: Data[0-3], ShiftAmount[0-1], ShiftDirection (7 inputs), ShiftedData[0-3] (4 outputs)
    // Note: Ground is internal (InputGnd), not exposed as an input port
    QVERIFY2(shifterIC->inputSize() == 7, qPrintable(QString("BarrelShifter4bit IC should have 7 input ports, got %1").arg(shifterIC->inputSize())));
    QVERIFY2(shifterIC->outputSize() == 4, qPrintable(QString("BarrelShifter4bit IC should have 4 output ports, got %1").arg(shifterIC->outputSize())));

    // Create input switches for data
    InputSwitch swData[4];
    for (int i = 0; i < 4; ++i) {
        builder.add(&swData[i]);
        swData[i].setOn((inputData >> i) & 1);
    }

    // Create input switches for shift amount
    InputSwitch swShift[2];
    builder.add(&swShift[0], &swShift[1]);
    swShift[0].setOn((shiftAmount >> 0) & 1);
    swShift[1].setOn((shiftAmount >> 1) & 1);

    // Create input switch for direction (0=left, 1=right)
    InputSwitch swDir;
    builder.add(&swDir);
    swDir.setOn(shiftRight);

    // Create output LEDs
    Led ledOut[4];
    for (int i = 0; i < 4; ++i) {
        builder.add(&ledOut[i]);
    }

    // Connect inputs to IC using semantic port labels
    for (int i = 0; i < 4; ++i) {
        builder.connect(&swData[i], 0, shifterIC, QString("Data[%1]").arg(i));
    }
    builder.connect(&swShift[0], 0, shifterIC, "ShiftAmount[0]");
    builder.connect(&swShift[1], 0, shifterIC, "ShiftAmount[1]");
    builder.connect(&swDir, 0, shifterIC, "ShiftDirection");

    // Connect IC outputs to LEDs
    for (int i = 0; i < 4; ++i) {
        builder.connect(shifterIC, QString("ShiftedData[%1]").arg(i), &ledOut[i], 0);
    }

    auto *simulation = builder.initSimulation();
    simulation->update();

    // Read result bits from circuit
    int result = 0;
    for (int i = 0; i < 4; ++i) {
        if (getInputStatus(&ledOut[i])) {
            result |= (1 << i);
        }
    }

    // Compare circuit output with expected result
    int maskedResult = result & 0x0F;
    int maskedExpected = expectedResult & 0x0F;

    QCOMPARE(maskedResult, maskedExpected);
}
