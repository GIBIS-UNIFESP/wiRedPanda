// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/Cpu/TestCPUInstructionExecute.h"

#include "Tests/Integration/IC/Tests/Cpu/CpuHelpers.h"

using TestUtils::readMultiBitOutput;

// ============================================================
// Consolidated Test Group: InstructionExecute
// ============================================================
// This consolidated test class merges the following test classes:
// - TestCPU32InstructionExecuteArithmetic
// - TestCPU33InstructionExecuteLogical
// - TestCPU34InstructionExecuteFlags
// - TestCPU35InstructionExecute
//

void TestCPUInstructionExecute::initTestCase()
{
    TestUtils::setupTestEnvironment();
}

void TestCPUInstructionExecute::cleanup()
{
}

void TestCPUInstructionExecute::testInstructionExecute()
{
    QFETCH(int, srcReg1);
    QFETCH(int, srcReg2);
    QFETCH(int, aluOp);
    QFETCH(int, expectedResult);
    // Integration test: Register Bank + ALU
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
    // Set inputs using manual loop (same as testALU8bit)
    for (int i = 0; i < 8; i++) {
        a[i]->setOn((srcReg1 >> i) & 1);
        b[i]->setOn((srcReg2 >> i) & 1);
    }
    for (int i = 0; i < 3; i++) {
        op[i]->setOn((aluOp >> i) & 1);
    }
    // Settle circuit
    sim->update();
    // Read actual result
    int execResult = readMultiBitOutput(result);
    QCOMPARE(execResult, expectedResult);
}

void TestCPUInstructionExecute::testInstructionExecuteArithmetic()
{
    QFETCH(int, srcReg1);
    QFETCH(int, srcReg2);
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
    for (int i = 0; i < 8; i++) {
        a[i]->setOn((srcReg1 >> i) & 1);
        b[i]->setOn((srcReg2 >> i) & 1);
    }
    for (int i = 0; i < 3; i++) {
        op[i]->setOn((aluOp >> i) & 1);
    }
    sim->update();
    int execResult = readMultiBitOutput(result);
    QCOMPARE(execResult, expectedResult);
}

void TestCPUInstructionExecute::testInstructionExecuteArithmetic_data()
{
    QTest::addColumn<int>("srcReg1");
    QTest::addColumn<int>("srcReg2");
    QTest::addColumn<int>("aluOp");
    QTest::addColumn<int>("expectedResult");
    QTest::newRow("exec_add_r1_r2") << 0x05 << 0x03 << (int)ALU_ADD << 0x08;
    QTest::newRow("exec_sub_r4_r5") << 0x0A << 0x04 << (int)ALU_SUB << 0x06;
    QTest::newRow("exec_nop_r0") << 0x42 << 0x00 << (int)ALU_ADD << 0x42;
}

void TestCPUInstructionExecute::testInstructionExecuteFlags()
{
    QFETCH(int, srcReg1);
    QFETCH(int, srcReg2);
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
    for (int i = 0; i < 8; i++) {
        a[i]->setOn((srcReg1 >> i) & 1);
        b[i]->setOn((srcReg2 >> i) & 1);
    }
    for (int i = 0; i < 3; i++) {
        op[i]->setOn((aluOp >> i) & 1);
    }
    sim->update();
    int execResult = readMultiBitOutput(result);
    QCOMPARE(execResult, expectedResult);
}

void TestCPUInstructionExecute::testInstructionExecuteFlags_data()
{
    QTest::addColumn<int>("srcReg1");
    QTest::addColumn<int>("srcReg2");
    QTest::addColumn<int>("aluOp");
    QTest::addColumn<int>("expectedResult");
    QTest::newRow("exec_add_zero") << 0x05 << 0x05 << (int)ALU_SUB << 0x00;
    QTest::newRow("exec_subtract_negative") << 0x05 << 0x0A << (int)ALU_SUB << 251;
    QTest::newRow("exec_flags_carry") << 0xFF << 0x01 << (int)ALU_ADD << 0x00;
    QTest::newRow("exec_dependency_chain") << 0x10 << 0x20 << (int)ALU_ADD << 0x30;
}

void TestCPUInstructionExecute::testInstructionExecuteLogical()
{
    QFETCH(int, srcReg1);
    QFETCH(int, srcReg2);
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
    for (int i = 0; i < 8; i++) {
        a[i]->setOn((srcReg1 >> i) & 1);
        b[i]->setOn((srcReg2 >> i) & 1);
    }
    for (int i = 0; i < 3; i++) {
        op[i]->setOn((aluOp >> i) & 1);
    }
    sim->update();
    int execResult = readMultiBitOutput(result);
    QCOMPARE(execResult, expectedResult);
}

void TestCPUInstructionExecute::testInstructionExecuteLogical_data()
{
    QTest::addColumn<int>("srcReg1");
    QTest::addColumn<int>("srcReg2");
    QTest::addColumn<int>("aluOp");
    QTest::addColumn<int>("expectedResult");
    QTest::newRow("exec_and_r0_r1") << 0xF0 << 0x0F << (int)ALU_AND << 0x00;
    QTest::newRow("exec_or_r3_r4") << 0xF0 << 0x0F << (int)ALU_OR << 0xFF;
    QTest::newRow("exec_xor_r2_r3") << 0xAA << 0x55 << (int)ALU_XOR << 0xFF;
}

void TestCPUInstructionExecute::testInstructionExecute_data()
{
    QTest::addColumn<int>("srcReg1");
    QTest::addColumn<int>("srcReg2");
    QTest::addColumn<int>("aluOp");
    QTest::addColumn<int>("expectedResult");
    QTest::newRow("exec_add_r1_r2") << 0x05 << 0x03 << (int)ALU_ADD << 0x08;
    QTest::newRow("exec_sub_r4_r5") << 0x0A << 0x04 << (int)ALU_SUB << 0x06;
    QTest::newRow("exec_and_r0_r1") << 0xF0 << 0x0F << (int)ALU_AND << 0x00;
    QTest::newRow("exec_or_r3_r4") << 0xF0 << 0x0F << (int)ALU_OR << 0xFF;
    QTest::newRow("exec_xor_r2_r3") << 0xAA << 0x55 << (int)ALU_XOR << 0xFF;
    QTest::newRow("exec_nop_r0") << 0x42 << 0x00 << (int)ALU_ADD << 0x42;
    QTest::newRow("exec_add_zero") << 0x05 << 0x05 << (int)ALU_SUB << 0x00;
    QTest::newRow("exec_subtract_negative") << 0x05 << 0x0A << (int)ALU_SUB << 251;
    QTest::newRow("exec_flags_carry") << 0xFF << 0x01 << (int)ALU_ADD << 0x00;
    QTest::newRow("exec_dependency_chain") << 0x10 << 0x20 << (int)ALU_ADD << 0x30;
}
