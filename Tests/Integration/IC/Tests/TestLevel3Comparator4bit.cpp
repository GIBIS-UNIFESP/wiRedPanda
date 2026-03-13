// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel3Comparator4bit.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel3Comparator4Bit::initTestCase()
{
}

void TestLevel3Comparator4Bit::cleanup()
{
}

void TestLevel3Comparator4Bit::testComparator4Bit_data()
{
    QTest::addColumn<int>("valueA");
    QTest::addColumn<int>("valueB");
    QTest::addColumn<bool>("expectedGreater");
    QTest::addColumn<bool>("expectedEqual");
    QTest::addColumn<bool>("expectedLess");

    QTest::newRow("5 > 3") << 5 << 3 << true << false << false;
    QTest::newRow("3 < 5") << 3 << 5 << false << false << true;
    QTest::newRow("5 == 5") << 5 << 5 << false << true << false;
    QTest::newRow("0 == 0") << 0 << 0 << false << true << false;
    QTest::newRow("15 > 0") << 15 << 0 << true << false << false;
    QTest::newRow("0 < 15") << 0 << 15 << false << false << true;
    QTest::newRow("7 == 7") << 7 << 7 << false << true << false;
    QTest::newRow("12 > 8") << 12 << 8 << true << false << false;
    QTest::newRow("8 < 12") << 8 << 12 << false << false << true;
    QTest::newRow("15 == 15") << 15 << 15 << false << true << false;
    QTest::newRow("10 < 11") << 10 << 11 << false << false << true;
    QTest::newRow("11 > 10") << 11 << 10 << true << false << false;
}

void TestLevel3Comparator4Bit::testComparator4Bit()
{
    QFETCH(int, valueA);
    QFETCH(int, valueB);
    QFETCH(bool, expectedGreater);
    QFETCH(bool, expectedEqual);
    QFETCH(bool, expectedLess);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Load Comparator4bit IC
    auto *comparatorIC = loadBuildingBlockIC("level3_comparator_4bit.panda");
    builder.add(comparatorIC);

    QVERIFY2(comparatorIC->inputSize() == 8, "Comparator4bit IC should have 8 input ports (A[0-3], B[0-3])");
    QVERIFY2(comparatorIC->outputSize() == 3, "Comparator4bit IC should have 3 output ports (Greater, Equal, Less)");

    // Create input switches for A and B operands
    InputSwitch swA[4], swB[4];
    for (int i = 0; i < 4; ++i) {
        builder.add(&swA[i], &swB[i]);
        swA[i].setOn((valueA >> i) & 1);
        swB[i].setOn((valueB >> i) & 1);
    }

    // Create output LEDs
    Led ledGreater, ledEqual, ledLess;
    builder.add(&ledGreater, &ledEqual, &ledLess);

    // Connect inputs to IC
    for (int i = 0; i < 4; ++i) {
        builder.connect(&swA[i], 0, comparatorIC, QString("A[%1]").arg(i));
        builder.connect(&swB[i], 0, comparatorIC, QString("B[%1]").arg(i));
    }

    // Connect IC outputs to LEDs
    builder.connect(comparatorIC, "Greater", &ledGreater, 0);
    builder.connect(comparatorIC, "Equal", &ledEqual, 0);
    builder.connect(comparatorIC, "Less", &ledLess, 0);

    auto *simulation = builder.initSimulation();
    simulation->update();

    QCOMPARE(getInputStatus(&ledGreater), expectedGreater);
    QCOMPARE(getInputStatus(&ledEqual), expectedEqual);
    QCOMPARE(getInputStatus(&ledLess), expectedLess);
}
