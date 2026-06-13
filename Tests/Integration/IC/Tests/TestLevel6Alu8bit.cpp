// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel6Alu8bit.h"

#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using TestUtils::readMultiBitOutput;
using TestUtils::setMultiBitInput;
using CPUTestUtils::loadBuildingBlockIC;

struct Alu8bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    QVector<InputSwitch *> aInputs;
    QVector<InputSwitch *> bInputs;
    QVector<InputSwitch *> opcodeInputs;
    QVector<Led *> resultOutputs;
    Led *zeroFlag = nullptr;
    Led *negativeFlag = nullptr;
    Led *carryFlag = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level6_alu_8bit.panda");
        builder.add(ic);

        for (int i = 0; i < 8; i++) {
            auto *a = new InputSwitch();
            builder.add(a);
            a->setLabel(QString("A[%1]").arg(i));
            aInputs.append(a);

            auto *b = new InputSwitch();
            builder.add(b);
            b->setLabel(QString("B[%1]").arg(i));
            bInputs.append(b);
        }

        for (int i = 0; i < 3; i++) {
            auto *op = new InputSwitch();
            builder.add(op);
            op->setLabel(QString("OpCode[%1]").arg(i));
            opcodeInputs.append(op);
        }

        for (int i = 0; i < 8; i++) {
            auto *result = new Led();
            builder.add(result);
            result->setLabel(QString("Result[%1]").arg(i));
            resultOutputs.append(result);
        }

        zeroFlag = new Led();
        builder.add(zeroFlag);
        zeroFlag->setLabel("Zero");

        negativeFlag = new Led();
        builder.add(negativeFlag);
        negativeFlag->setLabel("Negative");

        carryFlag = new Led();
        builder.add(carryFlag);
        carryFlag->setLabel("Carry");

        for (int i = 0; i < 8; i++) {
            builder.connect(aInputs[i], 0, ic, QString("A[%1]").arg(i));
            builder.connect(bInputs[i], 0, ic, QString("B[%1]").arg(i));
        }
        for (int i = 0; i < 3; i++) {
            builder.connect(opcodeInputs[i], 0, ic, QString("OpCode[%1]").arg(i));
        }

        for (int i = 0; i < 8; i++) {
            builder.connect(ic, QString("Result[%1]").arg(i), resultOutputs[i], 0);
        }
        builder.connect(ic, "Zero", zeroFlag, 0);
        builder.connect(ic, "Negative", negativeFlag, 0);
        builder.connect(ic, "Carry", carryFlag, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    void setOpCode(int code)
    {
        for (int i = 0; i < 3; i++) {
            opcodeInputs[i]->setOn((code >> i) & 1);
        }
    }

    int readResult()
    {
        return readMultiBitOutput(QVector<GraphicElement *>(resultOutputs.begin(), resultOutputs.end()), 0);
    }
};

static std::unique_ptr<Alu8bitFixture> s_level6Alu8bit;

void TestLevel6ALU8Bit::initTestCase()
{
    s_level6Alu8bit = std::make_unique<Alu8bitFixture>();
    QVERIFY(s_level6Alu8bit->build());
}

void TestLevel6ALU8Bit::cleanupTestCase()
{
    s_level6Alu8bit.reset();
}

void TestLevel6ALU8Bit::cleanup()
{
    if (s_level6Alu8bit && s_level6Alu8bit->sim) {
        s_level6Alu8bit->sim->restart();
        s_level6Alu8bit->sim->update();
    }
}

// ============================================================
// Test Cases
// ============================================================

void TestLevel6ALU8Bit::testALU8Bit() {
    auto &f = *s_level6Alu8bit;

    // Test 1: AND operation (OpCode = 010 = 2)
    setMultiBitInput(f.aInputs, CPUTestUtils::PATTERN_LOWER_NIBBLE);
    setMultiBitInput(f.bInputs, CPUTestUtils::PATTERN_UPPER_NIBBLE);

    f.setOpCode(2);
    f.sim->update();

    int result_and = f.readResult();
    QCOMPARE(result_and, CPUTestUtils::PATTERN_ALL_ZEROS);

    // Test 2: OR operation (OpCode = 011 = 3)
    f.setOpCode(3);
    f.sim->update();

    int result_or = f.readResult();
    QCOMPARE(result_or, CPUTestUtils::PATTERN_ALL_ONES);

    // Test 3: XOR operation (OpCode = 100 = 4)
    f.setOpCode(4);
    f.sim->update();
    int result_xor = f.readResult();
    QCOMPARE(result_xor, CPUTestUtils::PATTERN_ALL_ONES);

    // Test 4: NOT operation (OpCode = 101 = 5, NOT A)
    setMultiBitInput(f.aInputs, 0xAA);
    setMultiBitInput(f.bInputs, 0x00);
    f.setOpCode(5);
    f.sim->update();
    int result_not = f.readResult();
    QCOMPARE(result_not, 0x55);

    // Test 5: Shift Left operation (OpCode = 110 = 6, A << 1)
    setMultiBitInput(f.aInputs, 0x42);
    setMultiBitInput(f.bInputs, 0x00);
    f.setOpCode(6);
    f.sim->update();
    int result_shl = f.readResult();
    QCOMPARE(result_shl, 0x84);

    // Test 6: Shift Right operation (OpCode = 111 = 7, A >> 1)
    setMultiBitInput(f.aInputs, 0x85);
    setMultiBitInput(f.bInputs, 0x00);
    f.setOpCode(7);
    f.sim->update();
    int result_shr = f.readResult();
    QCOMPARE(result_shr, 0x42);
}

void TestLevel6ALU8Bit::testALU8BitStructure() {
    auto &f = *s_level6Alu8bit;

    QVERIFY(f.ic != nullptr);

    // A[8] + B[8] + OpCode[3] + CarryIn + SubCarryIn (F26) + ShrIn + ShlIn (F61)
    QCOMPARE(f.ic->inputSize(), 23);
    // Result[8] + Zero + Negative + Carry + SubCarryOut (F26)
    QCOMPARE(f.ic->outputSize(), 12);
}

void TestLevel6ALU8Bit::testALU8BitInputPortIsolation_data()
{
    QTest::addColumn<int>("bitPosition");

    for (int i = 0; i < 8; i++) {
        QTest::newRow(QString("input_bit_%1").arg(i).toLatin1())
            << i;
    }
}

void TestLevel6ALU8Bit::testALU8BitInputPortIsolation()
{
    QFETCH(int, bitPosition);

    auto &f = *s_level6Alu8bit;

    // Set OpCode = 010 (binary) = 2 (decimal) for AND operation
    f.setOpCode(2);

    // Set B to all 1s (0xFF)
    for (int i = 0; i < 8; i++) {
        f.bInputs[i]->setOn(true);
    }

    // Set A with only bit at bitPosition = 1, all others = 0
    for (int i = 0; i < 8; i++) {
        f.aInputs[i]->setOn(i == bitPosition);
    }

    f.sim->update();

    int result = f.readResult();
    int expected = (1 << bitPosition);

    QCOMPARE(result, expected);
}

void TestLevel6ALU8Bit::testALU8BitOutputPortIsolation_data()
{
    QTest::addColumn<int>("bitPosition");

    for (int i = 0; i < 8; i++) {
        QTest::newRow(QString("output_bit_%1").arg(i).toLatin1())
            << i;
    }
}

void TestLevel6ALU8Bit::testALU8BitOutputPortIsolation()
{
    QFETCH(int, bitPosition);

    auto &f = *s_level6Alu8bit;

    // Set OpCode = 011 (binary) = 3 (decimal) for OR operation
    f.setOpCode(3);

    // Test: Set A[bitPosition]=1, all other A bits=0, B=all 0s
    for (int i = 0; i < 8; i++) {
        f.aInputs[i]->setOn(i == bitPosition);
        f.bInputs[i]->setOn(false);
    }

    f.sim->update();

    // Verify only the tested bit has the expected value
    for (int i = 0; i < 8; i++) {
        bool bit_result = getInputStatus(f.resultOutputs[i], 0);
        bool expected = (i == bitPosition);

        QCOMPARE(bit_result, expected);
    }
}

// ADD and SUB exercise the carry/borrow chain across the 4-bit nibble boundary
// (the whole point of composing two ripple_alu_4bit halves), and the
// Zero/Negative/Carry flags. None of this was previously covered -- the
// operation test only ran AND/OR/XOR/NOT/SHL/SHR. CarryIn defaults to 0 and
// SubCarryIn to 1 (the two's-complement +1), so ADD = A+B and SUB = A-B.
void TestLevel6ALU8Bit::testArithmetic_data()
{
    QTest::addColumn<int>("opcode");
    QTest::addColumn<int>("a");
    QTest::addColumn<int>("b");
    QTest::addColumn<int>("expectedResult");
    QTest::addColumn<bool>("expectedZero");
    QTest::addColumn<bool>("expectedNegative");
    QTest::addColumn<int>("expectedCarry");  // -1 = do not check (SUB uses the parallel ADD carry)

    // ADD (opcode 0): Result = A + B; Carry = unsigned overflow out of bit 7.
    QTest::newRow("ADD nibble-carry 0x0F+0x01=0x10") << 0 << 0x0F << 0x01 << 0x10 << false << false << 0;
    QTest::newRow("ADD 0x08+0x08=0x10")              << 0 << 0x08 << 0x08 << 0x10 << false << false << 0;
    QTest::newRow("ADD overflow 0xFF+0x01=0x00")     << 0 << 0xFF << 0x01 << 0x00 << true  << false << 1;
    QTest::newRow("ADD negative 0x7F+0x01=0x80")     << 0 << 0x7F << 0x01 << 0x80 << false << true  << 0;
    QTest::newRow("ADD zero 0x00+0x00=0x00")         << 0 << 0x00 << 0x00 << 0x00 << true  << false << 0;
    QTest::newRow("ADD 0x3C+0x14=0x50")              << 0 << 0x3C << 0x14 << 0x50 << false << false << 0;

    // SUB (opcode 1): Result = A - B via two's complement (SubCarryIn=1 default).
    QTest::newRow("SUB nibble-borrow 0x10-0x01=0x0F") << 1 << 0x10 << 0x01 << 0x0F << false << false << -1;
    QTest::newRow("SUB underflow 0x00-0x01=0xFF")     << 1 << 0x00 << 0x01 << 0xFF << false << true  << -1;
    QTest::newRow("SUB zero 0x05-0x05=0x00")          << 1 << 0x05 << 0x05 << 0x00 << true  << false << -1;
    QTest::newRow("SUB 0x80-0x01=0x7F")               << 1 << 0x80 << 0x01 << 0x7F << false << false << -1;
}

void TestLevel6ALU8Bit::testArithmetic()
{
    QFETCH(int, opcode);
    QFETCH(int, a);
    QFETCH(int, b);
    QFETCH(int, expectedResult);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedNegative);
    QFETCH(int, expectedCarry);

    auto &f = *s_level6Alu8bit;

    setMultiBitInput(f.aInputs, a);
    setMultiBitInput(f.bInputs, b);
    f.setOpCode(opcode);
    f.sim->update();

    QCOMPARE(f.readResult(), expectedResult);
    QCOMPARE(getInputStatus(f.zeroFlag), expectedZero);
    QCOMPARE(getInputStatus(f.negativeFlag), expectedNegative);
    if (expectedCarry >= 0) {
        QCOMPARE(getInputStatus(f.carryFlag), expectedCarry != 0);
    }
}
