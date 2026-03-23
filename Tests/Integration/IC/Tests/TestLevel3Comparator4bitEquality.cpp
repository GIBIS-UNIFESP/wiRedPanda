// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel3Comparator4bitEquality.h"

#include <QFile>

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/Cpu/CpuCommon.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel3Comparator4BitEquality::initTestCase()
{
}

void TestLevel3Comparator4BitEquality::cleanup()
{
}

void TestLevel3Comparator4BitEquality::testComparator4BitEquality_data()
{
    QTest::addColumn<int>("valueA");
    QTest::addColumn<int>("valueB");
    QTest::addColumn<bool>("expectedEqual");

    // Equal cases
    QTest::newRow("eq_0000_0000") << 0 << 0 << true;
    QTest::newRow("eq_1111_1111") << 15 << 15 << true;
    QTest::newRow("eq_0101_0101") << 5 << 5 << true;
    QTest::newRow("eq_1010_1010") << 10 << 10 << true;

    // Not-equal cases
    QTest::newRow("neq_0000_0001") << 0 << 1 << false;
    QTest::newRow("neq_1000_0000") << 8 << 0 << false;
    QTest::newRow("neq_0101_1010") << 5 << 10 << false;
    QTest::newRow("neq_1111_0000") << 15 << 0 << false;
    QTest::newRow("neq_0011_0101") << 3 << 5 << false;
    QTest::newRow("neq_0111_0001") << 7 << 1 << false;
    QTest::newRow("neq_1111_1110") << 15 << 14 << false;
}

void TestLevel3Comparator4BitEquality::testComparator4BitEquality()
{
    QFETCH(int, valueA);
    QFETCH(int, valueB);
    QFETCH(bool, expectedEqual);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // 4-bit equality comparator
    InputSwitch inputA[4], inputB[4];
    Led ledEqual;

    // Load Comparator4BitEquality IC
    IC *comparatorIC = loadBuildingBlockIC("level3_comparator_4bit_equality.panda");

    // Add all elements to scene
    builder.add(comparatorIC, &ledEqual);
    for (int i = 0; i < 4; i++) {
        builder.add(&inputA[i], &inputB[i]);
    }

    // Connect inputs to IC using semantic port labels
    for (int i = 0; i < 4; i++) {
        builder.connect(&inputA[i], 0, comparatorIC, QString("A[%1]").arg(i));
        builder.connect(&inputB[i], 0, comparatorIC, QString("B[%1]").arg(i));
    }

    // Connect IC output to LED
    builder.connect(comparatorIC, "Equal", &ledEqual, 0);

    auto *simulation = builder.initSimulation();

    // Set input bits
    for (int i = 0; i < 4; ++i) {
        inputA[i].setOn((valueA >> i) & 1);
        inputB[i].setOn((valueB >> i) & 1);
    }

    // Initialize and settle
    simulation->update();

    // Read output
    bool actualEqual = getInputStatus(&ledEqual);

    // Verify
    QCOMPARE(actualEqual, expectedEqual);
}

