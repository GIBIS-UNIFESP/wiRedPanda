// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4Comparator4bit.h"

#include <QFile>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel4Comparator4Bit::initTestCase()
{
}

void TestLevel4Comparator4Bit::cleanup()
{
}

void TestLevel4Comparator4Bit::testComparator4Bit_data()
{
    QTest::addColumn<int>("aValue");
    QTest::addColumn<int>("bValue");
    QTest::addColumn<bool>("expectedGreater");
    QTest::addColumn<bool>("expectedEqual");
    QTest::addColumn<bool>("expectedLess");

    // Test equal values
    QTest::newRow("equal_0000") << 0x0 << 0x0 << false << true << false;
    QTest::newRow("equal_1111") << 0xF << 0xF << false << true << false;
    QTest::newRow("equal_0101") << 0x5 << 0x5 << false << true << false;
    QTest::newRow("equal_1010") << 0xA << 0xA << false << true << false;

    // Test A > B (Greater output)
    QTest::newRow("5_greater_3") << 0x5 << 0x3 << true << false << false;
    QTest::newRow("15_greater_0") << 0xF << 0x0 << true << false << false;
    QTest::newRow("12_greater_8") << 0xC << 0x8 << true << false << false;
    QTest::newRow("11_greater_10") << 0xB << 0xA << true << false << false;  // LSB only differs

    // Test A < B (Less output)
    QTest::newRow("3_less_5") << 0x3 << 0x5 << false << false << true;
    QTest::newRow("0_less_15") << 0x0 << 0xF << false << false << true;
    QTest::newRow("8_less_12") << 0x8 << 0xC << false << false << true;
    QTest::newRow("10_less_11") << 0xA << 0xB << false << false << true;  // LSB only differs
}

void TestLevel4Comparator4Bit::testComparator4Bit()
{
    QFETCH(int, aValue);
    QFETCH(int, bValue);
    QFETCH(bool, expectedGreater);
    QFETCH(bool, expectedEqual);
    QFETCH(bool, expectedLess);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Load Comparator 4-bit IC
    auto *compIC = loadBuildingBlockIC("level4_comparator_4bit.panda");
    builder.add(compIC);

    QVERIFY2(compIC->inputSize() == 8, "Comparator4bit IC should have 8 input ports (A[0-3], B[0-3])");
    QVERIFY2(compIC->outputSize() == 3, "Comparator4bit IC should have 3 output ports (Greater, Equal, Less)");

    // Create input switches for A and B operands
    InputSwitch swA[4], swB[4];
    for (int i = 0; i < 4; ++i) {
        builder.add(&swA[i], &swB[i]);
        swA[i].setOn((aValue >> i) & 1);
        swB[i].setOn((bValue >> i) & 1);
    }

    // Create output LEDs
    Led ledGreater, ledEqual, ledLess;
    builder.add(&ledGreater, &ledEqual, &ledLess);

    // Connect inputs to IC
    for (int i = 0; i < 4; ++i) {
        builder.connect(&swA[i], 0, compIC, QString("A[%1]").arg(i));
        builder.connect(&swB[i], 0, compIC, QString("B[%1]").arg(i));
    }

    // Connect IC outputs to LEDs
    builder.connect(compIC, "Greater", &ledGreater, 0);
    builder.connect(compIC, "Equal", &ledEqual, 0);
    builder.connect(compIC, "Less", &ledLess, 0);

    auto *simulation = builder.initSimulation();
    simulation->update();

    QCOMPARE(getInputStatus(&ledGreater), expectedGreater);
    QCOMPARE(getInputStatus(&ledEqual), expectedEqual);
    QCOMPARE(getInputStatus(&ledLess), expectedLess);
}

