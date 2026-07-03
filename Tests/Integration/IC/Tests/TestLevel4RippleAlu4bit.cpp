// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4RippleAlu4bit.h"

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

// The ALU computes all four operations in parallel: 10 inputs (A[0-3], B[0-3],
// CarryIn, SubCarryIn), 18 outputs (Result_{ADD,SUB,AND,OR}[0-3] + CarryOut +
// SubCarryOut). One shared fixture wires every output once; each test reads
// the lanes for its operation.
struct RippleAlu4bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *swA[4] = {}, *swB[4] = {};
    InputSwitch *swCarryIn = nullptr, *swSubCarryIn = nullptr;
    Led *ledAdd[4] = {}, *ledSub[4] = {}, *ledAnd[4] = {}, *ledOr[4] = {};
    Led *ledCarryOut = nullptr, *ledSubCarryOut = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level4_ripple_alu_4bit.panda");
        builder.add(ic);

        for (int i = 0; i < 4; ++i) {
            swA[i] = new InputSwitch();
            swB[i] = new InputSwitch();
            ledAdd[i] = new Led();
            ledSub[i] = new Led();
            ledAnd[i] = new Led();
            ledOr[i] = new Led();
            builder.add(swA[i], swB[i], ledAdd[i], ledSub[i], ledAnd[i], ledOr[i]);
        }
        swCarryIn = new InputSwitch();
        swSubCarryIn = new InputSwitch();
        ledCarryOut = new Led();
        ledSubCarryOut = new Led();
        builder.add(swCarryIn, swSubCarryIn, ledCarryOut, ledSubCarryOut);

        for (int i = 0; i < 4; ++i) {
            builder.connect(swA[i], 0, ic, QString("A[%1]").arg(i));
            builder.connect(swB[i], 0, ic, QString("B[%1]").arg(i));
            builder.connect(ic, QString("Result_ADD[%1]").arg(i), ledAdd[i], 0);
            builder.connect(ic, QString("Result_SUB[%1]").arg(i), ledSub[i], 0);
            builder.connect(ic, QString("Result_AND[%1]").arg(i), ledAnd[i], 0);
            builder.connect(ic, QString("Result_OR[%1]").arg(i), ledOr[i], 0);
        }
        builder.connect(swCarryIn, 0, ic, "CarryIn");
        builder.connect(swSubCarryIn, 0, ic, "SubCarryIn");
        builder.connect(ic, "CarryOut", ledCarryOut, 0);
        builder.connect(ic, "SubCarryOut", ledSubCarryOut, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    // Standard operating point: CarryIn=0 (plain ADD), SubCarryIn=1 (the
    // two's-complement +1 so Result_SUB = A - B).
    void setOperands(int a, int b)
    {
        swCarryIn->setOn(false);
        swSubCarryIn->setOn(true);
        for (int i = 0; i < 4; ++i) {
            swA[i]->setOn((a >> i) & 1);
            swB[i]->setOn((b >> i) & 1);
        }
        sim->update();
    }

    int readLanes(Led *const *leds) const
    {
        int value = 0;
        for (int i = 0; i < 4; ++i) {
            if (getInputStatus(leds[i])) {
                value |= (1 << i);
            }
        }
        return value;
    }
};

static std::unique_ptr<RippleAlu4bitFixture> s_level4RippleAlu4bit;

void TestLevel4RippleALU4Bit::initTestCase()
{
    s_level4RippleAlu4bit = std::make_unique<RippleAlu4bitFixture>();
    QVERIFY(s_level4RippleAlu4bit->build());

    // 10 inputs (A, B, CarryIn, SubCarryIn), 18 outputs (4 ops × 4 bits +
    // CarryOut + SubCarryOut)
    QCOMPARE(s_level4RippleAlu4bit->ic->inputSize(), 10);
    QCOMPARE(s_level4RippleAlu4bit->ic->outputSize(), 18);
}

void TestLevel4RippleALU4Bit::cleanupTestCase()
{
    s_level4RippleAlu4bit.reset();
}

void TestLevel4RippleALU4Bit::cleanup()
{
    if (s_level4RippleAlu4bit && s_level4RippleAlu4bit->sim) {
        s_level4RippleAlu4bit->sim->restart();
        s_level4RippleAlu4bit->sim->update();
    }
}

// ============================================================
// Test: ALU 4-bit ADD operation
// ============================================================

void TestLevel4RippleALU4Bit::testAluAdd_data()
{
    QTest::addColumn<int>("inputA");
    QTest::addColumn<int>("inputB");
    QTest::addColumn<int>("expectedResult");

    // ADD operation (Result_ADD output)
    QTest::newRow("ADD: 3 + 5 = 8") << 3 << 5 << 8;
    QTest::newRow("ADD: 15 + 1 = 0 (wrap)") << 15 << 1 << 0;
    QTest::newRow("ADD: 0 + 0 = 0") << 0 << 0 << 0;
    QTest::newRow("ADD: 7 + 8 = 15") << 7 << 8 << 15;
}

void TestLevel4RippleALU4Bit::testAluAdd()
{
    QFETCH(int, inputA);
    QFETCH(int, inputB);
    QFETCH(int, expectedResult);

    auto &f = *s_level4RippleAlu4bit;

    f.setOperands(inputA, inputB);
    QCOMPARE(f.readLanes(f.ledAdd), expectedResult & 0x0F);
}

// ============================================================
// Test: ALU 4-bit AND operation
// ============================================================

void TestLevel4RippleALU4Bit::testAluAnd_data()
{
    QTest::addColumn<int>("inputA");
    QTest::addColumn<int>("inputB");
    QTest::addColumn<int>("expectedResult");

    // AND operation (Result_AND output)
    QTest::newRow("AND: 15 & 7 = 7") << 15 << 7 << 7;
    QTest::newRow("AND: 12 & 10 = 8") << 12 << 10 << 8;
    QTest::newRow("AND: 0 & 15 = 0") << 0 << 15 << 0;
}

void TestLevel4RippleALU4Bit::testAluAnd()
{
    QFETCH(int, inputA);
    QFETCH(int, inputB);
    QFETCH(int, expectedResult);

    auto &f = *s_level4RippleAlu4bit;

    f.setOperands(inputA, inputB);
    QCOMPARE(f.readLanes(f.ledAnd), expectedResult & 0x0F);
}

// ============================================================
// Test: ALU 4-bit OR operation
// ============================================================

void TestLevel4RippleALU4Bit::testAluOr_data()
{
    QTest::addColumn<int>("inputA");
    QTest::addColumn<int>("inputB");
    QTest::addColumn<int>("expectedResult");

    // OR operation (Result_OR output)
    QTest::newRow("OR: 12 | 3 = 15") << 12 << 3 << 15;
    QTest::newRow("OR: 8 | 4 = 12") << 8 << 4 << 12;
    QTest::newRow("OR: 0 | 0 = 0") << 0 << 0 << 0;
}

void TestLevel4RippleALU4Bit::testAluOr()
{
    QFETCH(int, inputA);
    QFETCH(int, inputB);
    QFETCH(int, expectedResult);

    auto &f = *s_level4RippleAlu4bit;

    f.setOperands(inputA, inputB);
    QCOMPARE(f.readLanes(f.ledOr), expectedResult & 0x0F);
}

// ============================================================
// Test: ALU 4-bit SUB operation
// ============================================================

void TestLevel4RippleALU4Bit::testAluSub_data()
{
    QTest::addColumn<int>("inputA");
    QTest::addColumn<int>("inputB");
    QTest::addColumn<int>("expectedResult");

    // SUB operation (Result_SUB output)
    QTest::newRow("SUB: 5 - 3 = 2") << 5 << 3 << 2;
    QTest::newRow("SUB: 8 - 8 = 0") << 8 << 8 << 0;
    QTest::newRow("SUB: 10 - 7 = 3") << 10 << 7 << 3;
}

void TestLevel4RippleALU4Bit::testAluSub()
{
    QFETCH(int, inputA);
    QFETCH(int, inputB);
    QFETCH(int, expectedResult);

    auto &f = *s_level4RippleAlu4bit;

    f.setOperands(inputA, inputB);
    QCOMPARE(f.readLanes(f.ledSub), expectedResult & 0x0F);
}

// ============================================================
// Test: results + carry chain across all operations
// ============================================================

void TestLevel4RippleALU4Bit::testAluResultAndCarry_data()
{
    QTest::addColumn<int>("inputA");
    QTest::addColumn<int>("inputB");
    QTest::addColumn<int>("operation");
    QTest::addColumn<int>("expectedResult");

    // Operation selects which result lanes to read: 0=ADD, 1=SUB, 2=AND, 3=OR.
    // CarryOut/SubCarryOut are checked on every row (the ALU computes them
    // combinationally from A/B regardless of which result lanes are read).
    QTest::newRow("ADD: 8 + 8 = 0 (wrap)") << 8 << 8 << 0 << 0;
    QTest::newRow("SUB: 0 - 1 = 15 (underflow)") << 0 << 1 << 1 << 15;
    QTest::newRow("SUB: 3 - 5 = 14 (negative result)") << 3 << 5 << 1 << 14;
    QTest::newRow("ADD: 5 + 5 = 10") << 5 << 5 << 0 << 10;
    QTest::newRow("ADD: 15 + 15 = 14 (wrap)") << 15 << 15 << 0 << 14;
    QTest::newRow("AND: 12 & 10 = 8") << 12 << 10 << 2 << 8;
    QTest::newRow("OR: 15 | 0 = 15") << 15 << 0 << 3 << 15;
    QTest::newRow("ADD: 15 + 1 = 0 (carry out)") << 15 << 1 << 0 << 0;
    QTest::newRow("ADD: 7 + 9 = 0 (overflow to zero)") << 7 << 9 << 0 << 0;
    QTest::newRow("ADD: 7 + 2 = 9") << 7 << 2 << 0 << 9;
}

void TestLevel4RippleALU4Bit::testAluResultAndCarry()
{
    QFETCH(int, inputA);
    QFETCH(int, inputB);
    QFETCH(int, operation);
    QFETCH(int, expectedResult);

    auto &f = *s_level4RippleAlu4bit;

    f.setOperands(inputA, inputB);

    Led *const *lanes = nullptr;
    switch (operation) {
    case 0: lanes = f.ledAdd; break;
    case 1: lanes = f.ledSub; break;
    case 2: lanes = f.ledAnd; break;
    case 3: lanes = f.ledOr; break;
    default: QFAIL("Invalid operation selector");
    }
    QCOMPARE(f.readLanes(lanes), expectedResult & 0x0F);

    // CarryOut: unsigned overflow of A + B + CarryIn(0) past 4 bits.
    QCOMPARE(getInputStatus(f.ledCarryOut), inputA + inputB > 0x0F);

    // SubCarryOut: carry out of A + ~B + SubCarryIn(1) — HIGH exactly when the
    // subtraction does NOT borrow (A >= B), the 74181-style borrow convention.
    QCOMPARE(getInputStatus(f.ledSubCarryOut), inputA >= inputB);
}
