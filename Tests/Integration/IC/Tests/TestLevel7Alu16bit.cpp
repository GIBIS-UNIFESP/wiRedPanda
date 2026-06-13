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

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct Alu16bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *opAIn[16] = {};
    InputSwitch *opBIn[16] = {};
    InputSwitch *opIn[3] = {};
    Led *resultOut[16] = {};
    Led *zeroFlag = nullptr;
    Led *signFlag = nullptr;
    Led *carryFlag = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level7_alu_16bit.panda");

        zeroFlag = new Led();
        signFlag = new Led();
        carryFlag = new Led();
        builder.add(ic, zeroFlag, signFlag, carryFlag);

        for (int i = 0; i < 16; ++i) {
            opAIn[i] = new InputSwitch();
            opBIn[i] = new InputSwitch();
            resultOut[i] = new Led();
            builder.add(opAIn[i], opBIn[i], resultOut[i]);
        }
        for (int i = 0; i < 3; ++i) {
            opIn[i] = new InputSwitch();
            builder.add(opIn[i]);
        }

        for (int i = 0; i < 16; ++i) {
            builder.connect(opAIn[i], 0, ic, QString("OperandA[%1]").arg(i));
            builder.connect(opBIn[i], 0, ic, QString("OperandB[%1]").arg(i));
            builder.connect(ic, QString("Result[%1]").arg(i), resultOut[i], 0);
        }

        for (int i = 0; i < 3; ++i) {
            builder.connect(opIn[i], 0, ic, QString("ALUOp[%1]").arg(i));
        }

        builder.connect(ic, "Zero", zeroFlag, 0);
        builder.connect(ic, "Sign", signFlag, 0);
        builder.connect(ic, "Carry", carryFlag, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<Alu16bitFixture> s_level7Alu16bit;

void TestLevel7ALU16Bit::initTestCase()
{
    s_level7Alu16bit = std::make_unique<Alu16bitFixture>();
    QVERIFY(s_level7Alu16bit->build());
}

void TestLevel7ALU16Bit::cleanupTestCase()
{
    s_level7Alu16bit.reset();
}

void TestLevel7ALU16Bit::cleanup()
{
    if (s_level7Alu16bit && s_level7Alu16bit->sim) {
        s_level7Alu16bit->sim->restart();
        s_level7Alu16bit->sim->update();
    }
}

void TestLevel7ALU16Bit::testALU16Bit_data()
{
    QTest::addColumn<int>("opA");
    QTest::addColumn<int>("opB");
    QTest::addColumn<int>("aluOp");
    QTest::addColumn<int>("expectedResult");

    QTest::newRow("add_0x1000_0x2000") << 0x1000 << 0x2000 << 0 << 0x3000;
    QTest::newRow("add_0x0100_0x0200") << 0x0100 << 0x0200 << 0 << 0x0300;

    // Cross-byte carry propagation (F26: the byte halves' carry chains are
    // now connected, so a low-byte carry reaches the high byte)
    QTest::newRow("add_carry_0x00FF_0x0001") << 0x00FF << 0x0001 << 0 << 0x0100;
    QTest::newRow("add_carry_0x80FF_0x0101") << 0x80FF << 0x0101 << 0 << 0x8200;

    // Cross-byte borrow propagation in SUB (A - B, two's complement)
    QTest::newRow("sub_0x0300_0x0001") << 0x0300 << 0x0001 << 1 << 0x02FF;
    QTest::newRow("sub_0x1234_0x0234") << 0x1234 << 0x0234 << 1 << 0x1000;
    QTest::newRow("sub_same_is_zero") << 0x4242 << 0x4242 << 1 << 0x0000;

    QTest::newRow("and_0xFFFF_0x00FF") << 0xFFFF << 0x00FF << 2 << 0x00FF;
    QTest::newRow("and_0x1234_0x5678") << 0x1234 << 0x5678 << 2 << 0x1230;

    QTest::newRow("or_0x1000_0x0100") << 0x1000 << 0x0100 << 3 << 0x1100;
    QTest::newRow("or_0x00FF_0xFF00") << 0x00FF << 0xFF00 << 3 << 0xFFFF;

    QTest::newRow("xor_0xFFFF_0x00FF") << 0xFFFF << 0x00FF << 4 << 0xFF00;
    QTest::newRow("xor_0x1234_0x5678") << 0x1234 << 0x5678 << 4 << 0x444C;

    // NOT (op 5): ~A, B ignored. Each half NOTs its own byte independently, so
    // there is no cross-byte interaction — but the op was untested at 16-bit.
    QTest::newRow("not_0xAAAA") << 0xAAAA << 0x0000 << 5 << 0x5555;
    QTest::newRow("not_0x0000") << 0x0000 << 0x0000 << 5 << 0xFFFF;

    // Cross-byte shift propagation (F61: the byte halves used to shift
    // independently with zero fill, losing A[7] -> Result[8] on SHL and
    // A[8] -> Result[7] on SHR)
    QTest::newRow("shl_cross_byte_0x0080") << 0x0080 << 0x0000 << 6 << 0x0100;
    QTest::newRow("shl_0x4321") << 0x4321 << 0x0000 << 6 << 0x8642;
    QTest::newRow("shr_cross_byte_0x0100") << 0x0100 << 0x0000 << 7 << 0x0080;
    QTest::newRow("shr_0x8642") << 0x8642 << 0x0000 << 7 << 0x4321;
}

void TestLevel7ALU16Bit::testALU16Bit()
{
    QFETCH(int, opA);
    QFETCH(int, opB);
    QFETCH(int, aluOp);
    QFETCH(int, expectedResult);

    auto &f = *s_level7Alu16bit;

    for (int i = 0; i < 16; ++i) {
        f.opAIn[i]->setOn((opA >> i) & 1);
        f.opBIn[i]->setOn((opB >> i) & 1);
    }
    for (int i = 0; i < 3; ++i) {
        f.opIn[i]->setOn((aluOp >> i) & 1);
    }

    f.sim->update();

    int actualResult = 0;
    for (int i = 0; i < 16; ++i) {
        if (getInputStatus(f.resultOut[i])) {
            actualResult |= (1 << i);
        }
    }

    QCOMPARE(actualResult & 0xFFFF, expectedResult & 0xFFFF);
}

void TestLevel7ALU16Bit::testFlags16Bit_data()
{
    QTest::addColumn<int>("opA");
    QTest::addColumn<int>("opB");
    QTest::addColumn<int>("aluOp");
    QTest::addColumn<bool>("expectedZero");
    QTest::addColumn<bool>("expectedSign");
    QTest::addColumn<bool>("expectedCarry");

    // Zero asserts only when BOTH byte halves are zero (F26: AND of the per-half
    // Zero flags). A high-byte-only zero must NOT raise the 16-bit Zero flag.
    // (Carry is wired from the high ALU's ADD carry-out, so it is meaningful only
    // for ADD and reads 0 during SUB.)
    QTest::newRow("sub_equal_is_zero")  << 0x4242 << 0x4242 << 1 << true  << false << false;
    QTest::newRow("add_low_only_zero")  << 0x0100 << 0x0000 << 0 << false << false << false;
    QTest::newRow("add_high_only_zero") << 0x0001 << 0x0000 << 0 << false << false << false;

    // Sign = Result[15] (high half's Negative).
    QTest::newRow("add_sign_set")  << 0x8000 << 0x0000 << 0 << false << true  << false;
    QTest::newRow("add_sign_clear") << 0x7FFF << 0x0000 << 0 << false << false << false;

    // Carry = carry-out of bit 15 (high half), proving the low->high carry chain
    // reaches the top of the word.
    QTest::newRow("add_carry_out")  << 0xFFFF << 0x0001 << 0 << true  << false << true;
    QTest::newRow("add_no_carry")   << 0x0001 << 0x0001 << 0 << false << false << false;
}

void TestLevel7ALU16Bit::testFlags16Bit()
{
    QFETCH(int, opA);
    QFETCH(int, opB);
    QFETCH(int, aluOp);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedSign);
    QFETCH(bool, expectedCarry);

    auto &f = *s_level7Alu16bit;

    for (int i = 0; i < 16; ++i) {
        f.opAIn[i]->setOn((opA >> i) & 1);
        f.opBIn[i]->setOn((opB >> i) & 1);
    }
    for (int i = 0; i < 3; ++i) {
        f.opIn[i]->setOn((aluOp >> i) & 1);
    }

    f.sim->update();

    QCOMPARE(getInputStatus(f.zeroFlag), expectedZero);
    QCOMPARE(getInputStatus(f.signFlag), expectedSign);
    QCOMPARE(getInputStatus(f.carryFlag), expectedCarry);
}
