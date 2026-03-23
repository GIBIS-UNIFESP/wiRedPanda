// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel7Alu16bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using CPUTestUtils::loadBuildingBlockIC;

void TestLevel7ALU16Bit::initTestCase()
{
}

void TestLevel7ALU16Bit::cleanup()
{
}

void TestLevel7ALU16Bit::testALU16Bit_data()
{
    QTest::addColumn<int>("opA");
    QTest::addColumn<int>("opB");
    QTest::addColumn<int>("aluOp");
    QTest::addColumn<int>("expectedResult");

    // Operation 0 (ADD)
    QTest::newRow("add_0x1000_0x2000") << 0x1000 << 0x2000 << 0 << 0x3000;
    QTest::newRow("add_0x0100_0x0200") << 0x0100 << 0x0200 << 0 << 0x0300;

    // Operation 2 (AND)
    QTest::newRow("and_0xFFFF_0x00FF") << 0xFFFF << 0x00FF << 2 << 0x00FF;
    QTest::newRow("and_0x1234_0x5678") << 0x1234 << 0x5678 << 2 << 0x1230;

    // Operation 3 (OR)
    QTest::newRow("or_0x1000_0x0100") << 0x1000 << 0x0100 << 3 << 0x1100;
    QTest::newRow("or_0x00FF_0xFF00") << 0x00FF << 0xFF00 << 3 << 0xFFFF;

    // Operation 4 (XOR)
    QTest::newRow("xor_0xFFFF_0x00FF") << 0xFFFF << 0x00FF << 4 << 0xFF00;
    QTest::newRow("xor_0x1234_0x5678") << 0x1234 << 0x5678 << 4 << 0x444C;
}

void TestLevel7ALU16Bit::testALU16Bit()
{
    QFETCH(int, opA);
    QFETCH(int, opB);
    QFETCH(int, aluOp);
    QFETCH(int, expectedResult);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch opAIn[16], opBIn[16], opIn[3];
    Led resultOut[16], zeroFlag, signFlag, carryFlag;

    IC *aluIC = loadBuildingBlockIC("level7_alu_16bit.panda");

    builder.add(aluIC, &zeroFlag, &signFlag, &carryFlag);
    for (int i = 0; i < 16; ++i) {
        builder.add(&opAIn[i], &opBIn[i], &resultOut[i]);
    }
    for (int i = 0; i < 3; ++i) {
        builder.add(&opIn[i]);
    }

    for (int i = 0; i < 16; ++i) {
        builder.connect(&opAIn[i], 0, aluIC, QString("OperandA[%1]").arg(i));
        builder.connect(&opBIn[i], 0, aluIC, QString("OperandB[%1]").arg(i));
        builder.connect(aluIC, QString("Result[%1]").arg(i), &resultOut[i], 0);
    }

    for (int i = 0; i < 3; ++i) {
        builder.connect(&opIn[i], 0, aluIC, QString("ALUOp[%1]").arg(i));
    }

    builder.connect(aluIC, "Zero", &zeroFlag, 0);
    builder.connect(aluIC, "Sign", &signFlag, 0);
    builder.connect(aluIC, "Carry", &carryFlag, 0);

    auto *simulation = builder.initSimulation();

    // Set operands and operation (combinational - no clock needed)
    for (int i = 0; i < 16; ++i) {
        opAIn[i].setOn((opA >> i) & 1);
        opBIn[i].setOn((opB >> i) & 1);
    }
    for (int i = 0; i < 3; ++i) {
        opIn[i].setOn((aluOp >> i) & 1);
    }

    // Update to propagate combinational logic
    simulation->update();

    // Read result
    int actualResult = 0;
    for (int i = 0; i < 16; ++i) {
        if (TestUtils::getInputStatus(&resultOut[i])) {
            actualResult |= (1 << i);
        }
    }

    // Verify result
    QCOMPARE(actualResult & 0xFFFF, expectedResult & 0xFFFF);
}

