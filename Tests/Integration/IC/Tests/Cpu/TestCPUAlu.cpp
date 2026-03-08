// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/Cpu/TestCPUAlu.h"

#include "Tests/Integration/IC/Tests/Cpu/CpuHelpers.h"

using TestUtils::readMultiBitOutput;

// ============================================================
// Consolidated Test Group: Alu
// ============================================================
// This consolidated test class merges the following test classes:
// - TestCPU7ALU8Bit
// - TestCPU8ALU8BitArithmetic
// - TestCPU9ALU8BitBitwise
// - TestCPU10ALU8BitFlags
//

void TestCPUAlu::initTestCase()
{
    // Initialize test case resources if needed
}

void TestCPUAlu::cleanup()
{
    // Clean up test resources if needed
}

void TestCPUAlu::testALU8bit()
{
    QFETCH(int, aValue);
    QFETCH(int, bValue);
    QFETCH(int, aluOp);
    QFETCH(int, expectedResult);
    QVector<InputSwitch *> a, b, op;
    QVector<Led *> result;
    Led *zeroFlag, *signFlag, *carryFlag, *overflowFlag;
    for (int i = 0; i < 8; i++) {
        a.append(new InputSwitch());
        b.append(new InputSwitch());
        result.append(new Led());
    }
    for (int i = 0; i < 3; i++) {
        op.append(new InputSwitch());
    }
    zeroFlag = new Led();
    signFlag = new Led();
    carryFlag = new Led();
    overflowFlag = new Led();
    std::unique_ptr<WorkSpace> workspace(buildALU8bit(a.data(), b.data(), op.data(), result.data(), zeroFlag, signFlag, carryFlag, overflowFlag));
    auto *sim = workspace->simulation();
    // Set inputs
    for (int i = 0; i < 8; i++) {
        a[i]->setOn((aValue >> i) & 1);
        b[i]->setOn((bValue >> i) & 1);
    }
    for (int i = 0; i < 3; i++) {
        op[i]->setOn((aluOp >> i) & 1);
    }
    // Settle circuit
    sim->update();
    // Read actual result
    int resultValue = readMultiBitOutput(result);
    QCOMPARE(resultValue, expectedResult);
}

void TestCPUAlu::testALU8bitFlags()
{
    QFETCH(int, aValue);
    QFETCH(int, bValue);
    QFETCH(int, aluOp);
    QFETCH(int, expectedResult);
    QFETCH(bool, expectedCarry);
    QFETCH(bool, expectedOverflow);
    QVector<InputSwitch *> a, b, op;
    QVector<Led *> result;
    Led *zeroFlag, *signFlag, *carryFlag, *overflowFlag;
    for (int i = 0; i < 8; i++) {
        a.append(new InputSwitch());
        b.append(new InputSwitch());
        result.append(new Led());
    }
    for (int i = 0; i < 3; i++) {
        op.append(new InputSwitch());
    }
    zeroFlag = new Led();
    signFlag = new Led();
    carryFlag = new Led();
    overflowFlag = new Led();
    std::unique_ptr<WorkSpace> workspace(buildALU8bit(a.data(), b.data(), op.data(), result.data(), zeroFlag, signFlag, carryFlag, overflowFlag));
    auto *sim = workspace->simulation();
    // Set inputs
    for (int i = 0; i < 8; i++) {
        a[i]->setOn((aValue >> i) & 1);
        b[i]->setOn((bValue >> i) & 1);
    }
    for (int i = 0; i < 3; i++) {
        op[i]->setOn((aluOp >> i) & 1);
    }
    // Settle circuit
    sim->update();
    // Read actual result
    int resultValue = readMultiBitOutput(result);
    QCOMPARE(resultValue, expectedResult);

    // Verify flags are set correctly based on the result
    bool zeroFlagSet = TestUtils::getInputStatus(zeroFlag);
    bool signFlagSet = TestUtils::getInputStatus(signFlag);
    bool carryFlagSet = TestUtils::getInputStatus(carryFlag);
    bool overflowFlagSet = TestUtils::getInputStatus(overflowFlag);

    // Sign flag should be set (1) if result has sign bit set (bit 7)
    // In 8-bit signed, values 128-255 are negative
    bool expectedSignFlag = ((expectedResult & 0x80) != 0);
    QCOMPARE(signFlagSet, expectedSignFlag);

    bool expectedZeroFlag = (expectedResult == 0);
    QCOMPARE(zeroFlagSet, expectedZeroFlag);

    QCOMPARE(carryFlagSet, expectedCarry);
    QCOMPARE(overflowFlagSet, expectedOverflow);
}

void TestCPUAlu::testALU8bitFlags_data()
{
    QTest::addColumn<int>("aValue");
    QTest::addColumn<int>("bValue");
    QTest::addColumn<int>("aluOp");
    QTest::addColumn<int>("expectedResult");
    QTest::addColumn<bool>("expectedCarry");
    QTest::addColumn<bool>("expectedOverflow");
    // 5 - 5 = 0: no borrow (A>=B) → carry=1; cin[7]=cout[7]=1 → overflow=0
    QTest::newRow("alu_zero_result")          << 5   << 5   << (int)ALU_SUB << 0   << true  << false;
    // 10 - 20 = 246: borrow (A<B) → carry=0; cin[7]=cout[7]=0 → overflow=0
    QTest::newRow("alu_sign_result")          << 10  << 20  << (int)ALU_SUB << 246 << false << false;
    // 64 + 64 = 128: cin[7]=1, cout[7]=0 → overflow=1 (+64 + +64 overflows signed range); no unsigned carry → carry=0
    QTest::newRow("alu_add_pos_overflow")     << 64  << 64  << (int)ALU_ADD << 128 << false << true;
    // 127 - (-1) = 128: A=0x7F, B=0xFF, NOT(B)=0x00, cin=1; cin[7]=1, cout[7]=0 → overflow=1; borrow → carry=0
    QTest::newRow("alu_sub_pos_overflow")     << 127 << 255 << (int)ALU_SUB << 128 << false << true;
    // 0xFF + 0x01 = 0: cin[7]=1, cout[7]=1 → overflow=0 (−1+1=0, no signed overflow); unsigned carry → carry=1
    QTest::newRow("alu_add_carry_no_overflow") << 255 << 1  << (int)ALU_ADD << 0   << true  << false;
}

void TestCPUAlu::testALU8bit_data()
{
    QTest::addColumn<int>("aValue");
    QTest::addColumn<int>("bValue");
    QTest::addColumn<int>("aluOp");
    QTest::addColumn<int>("expectedResult");
    // AND operations
    QTest::newRow("alu_and_0xF0_0x0F") << 0xF0 << 0x0F << (int)ALU_AND << 0x00;
    QTest::newRow("alu_and_0xFF_0xF0") << 0xFF << 0xF0 << (int)ALU_AND << 0xF0;
    // OR operations
    QTest::newRow("alu_or_0xF0_0x0F") << 0xF0 << 0x0F << (int)ALU_OR << 0xFF;
    QTest::newRow("alu_or_0x00_0x00") << 0x00 << 0x00 << (int)ALU_OR << 0x00;
    // XOR operations
    QTest::newRow("alu_xor_0xAA_0x55") << 0xAA << 0x55 << (int)ALU_XOR << 0xFF;
    QTest::newRow("alu_xor_0xFF_0xFF") << 0xFF << 0xFF << (int)ALU_XOR << 0x00;
    // ADD operations (simplified)
    QTest::newRow("alu_add_5_3") << 5 << 3 << (int)ALU_ADD << 8;
    QTest::newRow("alu_add_0xFF_1") << 0xFF << 1 << (int)ALU_ADD << 0x00;
    // SUB operations (simplified)
    QTest::newRow("alu_sub_10_4") << 10 << 4 << (int)ALU_SUB << 6;
    QTest::newRow("alu_sub_5_10") << 5 << 10 << (int)ALU_SUB << 251;
    // Flag tests would go here
    QTest::newRow("alu_zero_result") << 5 << 5 << (int)ALU_SUB << 0;
    QTest::newRow("alu_sign_result") << 10 << 20 << (int)ALU_SUB << 246;
}
