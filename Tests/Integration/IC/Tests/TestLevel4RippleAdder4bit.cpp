// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4RippleAdder4bit.h"

#include <QFileInfo>

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel4RippleAdder4Bit::initTestCase()
{
}

void TestLevel4RippleAdder4Bit::cleanup()
{
}

void TestLevel4RippleAdder4Bit::testAdder4Bit_data()
{
    QTest::addColumn<int>("valueA");
    QTest::addColumn<int>("valueB");
    QTest::addColumn<int>("expectedSum");
    QTest::addColumn<bool>("expectedCarryOut");

    // Test cases: Real 4-bit addition with carry propagation
    // Sum is masked to 4 bits; carry-out flag indicates overflow
    QTest::newRow("0 + 0 = 0, no carry") << 0 << 0 << 0 << false;
    QTest::newRow("1 + 1 = 2, no carry") << 1 << 1 << 2 << false;
    QTest::newRow("3 + 5 = 8, no carry") << 3 << 5 << 8 << false;         // 0011 + 0101 = 1000
    QTest::newRow("1 + 7 = 8, no carry") << 1 << 7 << 8 << false;         // 0001 + 0111 = 1000
    QTest::newRow("7 + 8 = 15, no carry") << 7 << 8 << 15 << false;       // 0111 + 1000 = 1111
    QTest::newRow("4 + 4 = 8, no carry") << 4 << 4 << 8 << false;         // 0100 + 0100 = 1000
    QTest::newRow("15 + 1 = 0, CARRY OUT") << 15 << 1 << 0 << true;       // 1111 + 0001 = 10000 → overflow!
    QTest::newRow("8 + 8 = 0, CARRY OUT") << 8 << 8 << 0 << true;         // 1000 + 1000 = 10000 → overflow!
    QTest::newRow("8 + 7 = 15, no carry") << 8 << 7 << 15 << false;       // 1000 + 0111 = 1111
    QTest::newRow("15 + 15 = 14, CARRY OUT") << 15 << 15 << 14 << true;   // 1111 + 1111 = 11110 → overflow!
}

void TestLevel4RippleAdder4Bit::testAdder4Bit()
{
    QFETCH(int, valueA);
    QFETCH(int, valueB);
    QFETCH(int, expectedSum);
    QFETCH(bool, expectedCarryOut);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Load Level4RippleAdder4bit IC
    auto *adderIC = loadBuildingBlockIC("level4_ripple_adder_4bit.panda");
    builder.add(adderIC);

    // Verify IC has correct port counts
    QVERIFY2(adderIC->inputSize() == 9, "Level4RippleAdder4bit IC should have 9 input ports (A[0-3], B[0-3], CarryIn)");
    QVERIFY2(adderIC->outputSize() == 5, "Level4RippleAdder4bit IC should have 5 output ports (Sum[0-3], CarryOut)");

    // Create input switches for A and B operands
    InputSwitch swA[4], swB[4];
    for (int i = 0; i < 4; ++i) {
        builder.add(&swA[i], &swB[i]);
        swA[i].setOn((valueA >> i) & 1);
        swB[i].setOn((valueB >> i) & 1);
    }

    // Create output LEDs for Sum and CarryOut
    Led ledSum[4], ledCarryOut;
    for (int i = 0; i < 4; ++i) {
        builder.add(&ledSum[i]);
    }
    builder.add(&ledCarryOut);

    // Create CarryIn input (always 0 for basic addition)
    InputSwitch carryIn;
    carryIn.setOn(false);
    builder.add(&carryIn);

    // Connect inputs to IC using semantic port labels
    // Level4RippleAdder4bit IC input ports: A[0-3], B[0-3], CarryIn
    for (int i = 0; i < 4; ++i) {
        builder.connect(&swA[i], 0, adderIC, QString("A[%1]").arg(i));
        builder.connect(&swB[i], 0, adderIC, QString("B[%1]").arg(i));
    }
    builder.connect(&carryIn, 0, adderIC, "CarryIn");

    // Connect IC outputs to LEDs using semantic port labels
    // Level4RippleAdder4bit IC output ports: Sum[0-3], CarryOut
    for (int i = 0; i < 4; ++i) {
        builder.connect(adderIC, QString("Sum[%1]").arg(i), &ledSum[i], 0);
    }
    builder.connect(adderIC, "CarryOut", &ledCarryOut, 0);

    auto *simulation = builder.initSimulation();
    simulation->update();

    // Read 4-bit result (sum is 4 bits)
    int actualSum = 0;
    for (int i = 0; i < 4; ++i) {
        actualSum |= (getInputStatus(&ledSum[i]) ? 1 : 0) << i;
    }

    // Read carry-out flag (overflow indicator)
    bool actualCarryOut = getInputStatus(&ledCarryOut);

    // Verify both sum and carry-out
    QCOMPARE(actualSum, expectedSum);
    QCOMPARE(actualCarryOut, expectedCarryOut);
}

void TestLevel4RippleAdder4Bit::testRippleCarry_data()
{
    QTest::addColumn<int>("valueA");
    QTest::addColumn<int>("valueB");
    QTest::addColumn<int>("expectedSum");
    QTest::addColumn<bool>("expectedCarryOut");

    // Test cases: Real 4-bit addition with carry propagation
    // Sum is masked to 4 bits; carry-out flag indicates overflow
    QTest::newRow("0 + 0 = 0, no carry") << 0 << 0 << 0 << false;
    QTest::newRow("1 + 1 = 2, no carry") << 1 << 1 << 2 << false;
    QTest::newRow("3 + 5 = 8, no carry") << 3 << 5 << 8 << false;         // 0011 + 0101 = 1000
    QTest::newRow("1 + 7 = 8, no carry") << 1 << 7 << 8 << false;         // 0001 + 0111 = 1000
    QTest::newRow("7 + 8 = 15, no carry") << 7 << 8 << 15 << false;       // 0111 + 1000 = 1111
    QTest::newRow("4 + 4 = 8, no carry") << 4 << 4 << 8 << false;         // 0100 + 0100 = 1000
    QTest::newRow("15 + 1 = 0, CARRY OUT") << 15 << 1 << 0 << true;       // 1111 + 0001 = 10000 → overflow!
    QTest::newRow("8 + 8 = 0, CARRY OUT") << 8 << 8 << 0 << true;         // 1000 + 1000 = 10000 → overflow!
    QTest::newRow("8 + 7 = 15, no carry") << 8 << 7 << 15 << false;       // 1000 + 0111 = 1111
    QTest::newRow("15 + 15 = 14, CARRY OUT") << 15 << 15 << 14 << true;   // 1111 + 1111 = 11110 → overflow!
}

void TestLevel4RippleAdder4Bit::testRippleCarry()
{
    QFETCH(int, valueA);
    QFETCH(int, valueB);
    QFETCH(int, expectedSum);
    QFETCH(bool, expectedCarryOut);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Load Level4RippleAdder4bit IC
    auto *adderIC = loadBuildingBlockIC("level4_ripple_adder_4bit.panda");
    builder.add(adderIC);

    // Create input switches for A and B operands
    InputSwitch swA[4], swB[4];
    for (int i = 0; i < 4; ++i) {
        builder.add(&swA[i], &swB[i]);
        swA[i].setOn((valueA >> i) & 1);
        swB[i].setOn((valueB >> i) & 1);
    }

    // Create output LEDs for Sum and CarryOut
    Led ledSum[4], ledCarryOut;
    for (int i = 0; i < 4; ++i) {
        builder.add(&ledSum[i]);
    }
    builder.add(&ledCarryOut);

    // Create CarryIn input (always 0 for basic addition)
    InputSwitch carryIn;
    carryIn.setOn(false);
    builder.add(&carryIn);

    // Connect inputs to IC using semantic port labels
    // Level4RippleAdder4bit IC input ports: A[0-3], B[0-3], CarryIn
    for (int i = 0; i < 4; ++i) {
        builder.connect(&swA[i], 0, adderIC, QString("A[%1]").arg(i));
        builder.connect(&swB[i], 0, adderIC, QString("B[%1]").arg(i));
    }
    builder.connect(&carryIn, 0, adderIC, "CarryIn");

    // Connect IC outputs to LEDs using semantic port labels
    // Level4RippleAdder4bit IC output ports: Sum[0-3], CarryOut
    for (int i = 0; i < 4; ++i) {
        builder.connect(adderIC, QString("Sum[%1]").arg(i), &ledSum[i], 0);
    }
    builder.connect(adderIC, "CarryOut", &ledCarryOut, 0);

    auto *simulation = builder.initSimulation();
    simulation->update();

    // Read 4-bit result (sum is 4 bits)
    int actualSum = 0;
    for (int i = 0; i < 4; ++i) {
        actualSum |= (getInputStatus(&ledSum[i]) ? 1 : 0) << i;
    }

    // Read carry-out flag (overflow indicator)
    bool actualCarryOut = getInputStatus(&ledCarryOut);

    // Verify both sum and carry-out
    QCOMPARE(actualSum, expectedSum);
    QCOMPARE(actualCarryOut, expectedCarryOut);
}
