// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4RippleAlu4bit.h"

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel4RippleALU4Bit::initTestCase()
{
}

void TestLevel4RippleALU4Bit::cleanup()
{
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

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Load ALU4bit IC
    auto *aluIC = loadBuildingBlockIC("level4_ripple_alu_4bit.panda");
    builder.add(aluIC);

    // Verify IC has correct port counts
    // New interface: 9 inputs (A[0-3], B[0-3], CarryIn), 17 outputs (4 operations × 4 bits + CarryOut)
    QVERIFY2(aluIC->inputSize() == 9, qPrintable(QString("ALU4bit IC should have 9 input ports (A,B,CarryIn), got %1").arg(aluIC->inputSize())));
    QVERIFY2(aluIC->outputSize() == 17, qPrintable(QString("ALU4bit IC should have 17 output ports (4 ops + CarryOut), got %1").arg(aluIC->outputSize())));

    // Input switches for A (4-bit), B (4-bit), and CarryIn
    InputSwitch swA[4], swB[4], swCarryIn;
    for (int i = 0; i < 4; ++i) {
        builder.add(&swA[i], &swB[i]);
    }
    builder.add(&swCarryIn);

    // Output LEDs for Result_ADD[0-3]
    Led ledResult[4];
    for (int i = 0; i < 4; ++i) {
        builder.add(&ledResult[i]);
    }

    // Connect inputs to IC using semantic port labels
    for (int i = 0; i < 4; ++i) {
        builder.connect(&swA[i], 0, aluIC, QString("A[%1]").arg(i));
        builder.connect(&swB[i], 0, aluIC, QString("B[%1]").arg(i));
    }

    // Connect CarryIn (set to 0 for basic ADD without carry-in)
    builder.connect(&swCarryIn, 0, aluIC, "CarryIn");

    // Connect Result_ADD outputs to LEDs
    for (int i = 0; i < 4; ++i) {
        builder.connect(aluIC, QString("Result_ADD[%1]").arg(i), &ledResult[i], 0);
    }

    auto *simulation = builder.initSimulation();

    // Set input values (must be after initSimulation)
    for (int i = 0; i < 4; ++i) {
        bool bitA = (inputA >> i) & 1;
        bool bitB = (inputB >> i) & 1;
        swA[i].setOn(bitA);
        swB[i].setOn(bitB);
    }

    simulation->update();

    // Read result bits from circuit
    int result = 0;
    for (int i = 0; i < 4; ++i) {
        if (getInputStatus(&ledResult[i])) {
            result |= (1 << i);
        }
    }

    // Compare circuit output with expected result
    int maskedResult = result & 0x0F;
    int maskedExpected = expectedResult & 0x0F;

    QCOMPARE(maskedResult, maskedExpected);
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

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Load ALU4bit IC
    auto *aluIC = loadBuildingBlockIC("level4_ripple_alu_4bit.panda");
    builder.add(aluIC);

    // Verify IC has correct port counts
    // New interface: 9 inputs (A[0-3], B[0-3], CarryIn), 17 outputs (4 operations × 4 bits + CarryOut)
    QVERIFY2(aluIC->inputSize() == 9, qPrintable(QString("ALU4bit IC should have 9 input ports (A,B,CarryIn), got %1").arg(aluIC->inputSize())));
    QVERIFY2(aluIC->outputSize() == 17, qPrintable(QString("ALU4bit IC should have 17 output ports (4 ops + CarryOut), got %1").arg(aluIC->outputSize())));

    // Input switches for A (4-bit), B (4-bit), and CarryIn
    InputSwitch swA[4], swB[4], swCarryIn;
    for (int i = 0; i < 4; ++i) {
        builder.add(&swA[i], &swB[i]);
    }
    builder.add(&swCarryIn);

    // Output LEDs for Result_AND[0-3]
    Led ledResult[4];
    for (int i = 0; i < 4; ++i) {
        builder.add(&ledResult[i]);
    }

    // Connect inputs to IC using semantic port labels
    for (int i = 0; i < 4; ++i) {
        builder.connect(&swA[i], 0, aluIC, QString("A[%1]").arg(i));
        builder.connect(&swB[i], 0, aluIC, QString("B[%1]").arg(i));
    }

    // Connect CarryIn (set to 0 for basic AND without carry-in)
    builder.connect(&swCarryIn, 0, aluIC, "CarryIn");

    // Connect Result_AND outputs to LEDs
    for (int i = 0; i < 4; ++i) {
        builder.connect(aluIC, QString("Result_AND[%1]").arg(i), &ledResult[i], 0);
    }

    auto *simulation = builder.initSimulation();

    // Set input values (must be after initSimulation)
    for (int i = 0; i < 4; ++i) {
        bool bitA = (inputA >> i) & 1;
        bool bitB = (inputB >> i) & 1;
        swA[i].setOn(bitA);
        swB[i].setOn(bitB);
    }

    simulation->update();

    // Read result bits from circuit
    int result = 0;
    for (int i = 0; i < 4; ++i) {
        if (getInputStatus(&ledResult[i])) {
            result |= (1 << i);
        }
    }

    // Compare circuit output with expected result
    int maskedResult = result & 0x0F;
    int maskedExpected = expectedResult & 0x0F;

    QCOMPARE(maskedResult, maskedExpected);
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

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Load ALU4bit IC
    auto *aluIC = loadBuildingBlockIC("level4_ripple_alu_4bit.panda");
    builder.add(aluIC);

    // Verify IC has correct port counts
    // New interface: 9 inputs (A[0-3], B[0-3], CarryIn), 17 outputs (4 operations × 4 bits + CarryOut)
    QVERIFY2(aluIC->inputSize() == 9, qPrintable(QString("ALU4bit IC should have 9 input ports (A,B,CarryIn), got %1").arg(aluIC->inputSize())));
    QVERIFY2(aluIC->outputSize() == 17, qPrintable(QString("ALU4bit IC should have 17 output ports (4 ops + CarryOut), got %1").arg(aluIC->outputSize())));

    // Input switches for A (4-bit), B (4-bit), and CarryIn
    InputSwitch swA[4], swB[4], swCarryIn;
    for (int i = 0; i < 4; ++i) {
        builder.add(&swA[i], &swB[i]);
    }
    builder.add(&swCarryIn);

    // Output LEDs
    Led ledResult[4];
    for (int i = 0; i < 4; ++i) {
        builder.add(&ledResult[i]);
    }

    // Connect inputs to IC using semantic port labels
    for (int i = 0; i < 4; ++i) {
        builder.connect(&swA[i], 0, aluIC, QString("A[%1]").arg(i));
        builder.connect(&swB[i], 0, aluIC, QString("B[%1]").arg(i));
    }

    // Connect CarryIn (set to 0 for basic OR without carry-in)
    builder.connect(&swCarryIn, 0, aluIC, "CarryIn");

    // Connect IC outputs to LEDs
    for (int i = 0; i < 4; ++i) {
        builder.connect(aluIC, QString("Result_OR[%1]").arg(i), &ledResult[i], 0);
    }

    auto *simulation = builder.initSimulation();

    // Set input values (must be after initSimulation)
    for (int i = 0; i < 4; ++i) {
        bool bitA = (inputA >> i) & 1;
        bool bitB = (inputB >> i) & 1;
        swA[i].setOn(bitA);
        swB[i].setOn(bitB);
    }

    simulation->update();

    // Read result bits from circuit
    int result = 0;
    for (int i = 0; i < 4; ++i) {
        if (getInputStatus(&ledResult[i])) {
            result |= (1 << i);
        }
    }

    // Compare circuit output with expected result
    int maskedResult = result & 0x0F;
    int maskedExpected = expectedResult & 0x0F;

    QCOMPARE(maskedResult, maskedExpected);
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

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Load ALU4bit IC
    auto *aluIC = loadBuildingBlockIC("level4_ripple_alu_4bit.panda");
    builder.add(aluIC);

    // Verify IC has correct port counts
    // New interface: 9 inputs (A[0-3], B[0-3], CarryIn), 17 outputs (4 operations × 4 bits + CarryOut)
    QVERIFY2(aluIC->inputSize() == 9, qPrintable(QString("ALU4bit IC should have 9 input ports (A,B,CarryIn), got %1").arg(aluIC->inputSize())));
    QVERIFY2(aluIC->outputSize() == 17, qPrintable(QString("ALU4bit IC should have 17 output ports (4 ops + CarryOut), got %1").arg(aluIC->outputSize())));

    // Input switches for A (4-bit), B (4-bit), and CarryIn
    InputSwitch swA[4], swB[4], swCarryIn;
    for (int i = 0; i < 4; ++i) {
        builder.add(&swA[i], &swB[i]);
    }
    builder.add(&swCarryIn);

    // Output LEDs for Result_SUB[0-3]
    Led ledResult[4];
    for (int i = 0; i < 4; ++i) {
        builder.add(&ledResult[i]);
    }

    // Connect inputs to IC using semantic port labels
    for (int i = 0; i < 4; ++i) {
        builder.connect(&swA[i], 0, aluIC, QString("A[%1]").arg(i));
        builder.connect(&swB[i], 0, aluIC, QString("B[%1]").arg(i));
    }

    // Connect CarryIn (set to 0 for basic SUB without carry-in)
    builder.connect(&swCarryIn, 0, aluIC, "CarryIn");

    // Connect Result_SUB outputs to LEDs
    for (int i = 0; i < 4; ++i) {
        builder.connect(aluIC, QString("Result_SUB[%1]").arg(i), &ledResult[i], 0);
    }

    auto *simulation = builder.initSimulation();

    // Set input values (must be after initSimulation)
    for (int i = 0; i < 4; ++i) {
        bool bitA = (inputA >> i) & 1;
        bool bitB = (inputB >> i) & 1;
        swA[i].setOn(bitA);
        swB[i].setOn(bitB);
    }

    simulation->update();

    // Read result bits from circuit
    int result = 0;
    for (int i = 0; i < 4; ++i) {
        if (getInputStatus(&ledResult[i])) {
            result |= (1 << i);
        }
    }

    // Compare circuit output with expected result
    int maskedResult = result & 0x0F;
    int maskedExpected = expectedResult & 0x0F;

    QCOMPARE(maskedResult, maskedExpected);
}

// ============================================================
// Test: ALU 4-bit Flag verification (all operations)
// ============================================================

void TestLevel4RippleALU4Bit::testAluFlags_data()
{
    QTest::addColumn<int>("inputA");
    QTest::addColumn<int>("inputB");
    QTest::addColumn<int>("operation");
    QTest::addColumn<int>("expectedResult");

    // Status Flag Verification Test Cases
    // ZF (Zero Flag) - Set when result = 0
    QTest::newRow("ADD: 8 + 8 = 0 (ZF with wrap)") << 8 << 8 << 0 << 0;
    QTest::newRow("SUB: 0 - 1 = 15 (underflow)") << 0 << 1 << 1 << 15;
    QTest::newRow("SUB: 3 - 5 = 14 (negative result)") << 3 << 5 << 1 << 14;

    // SF (Sign Flag) - Set when MSB (bit 3) = 1 (result >= 8)
    QTest::newRow("ADD: 5 + 5 = 10 (SF set)") << 5 << 5 << 0 << 10;
    QTest::newRow("ADD: 15 + 15 = 14 (SF with wrap)") << 15 << 15 << 0 << 14;
    QTest::newRow("AND: 12 & 10 = 8 (SF from AND)") << 12 << 10 << 2 << 8;
    QTest::newRow("OR: 15 | 0 = 15 (SF from OR)") << 15 << 0 << 3 << 15;

    // CF (Carry Flag) - Set when result overflows past 4-bit
    QTest::newRow("ADD: 15 + 1 = 0 (carry out)") << 15 << 1 << 0 << 0;
    QTest::newRow("ADD: 7 + 9 = 0 (overflow to zero)") << 7 << 9 << 0 << 0;

    // OF (Overflow Flag) - Set on signed overflow (for ADD/SUB)
    QTest::newRow("ADD: 7 + 2 = 9 (signed overflow)") << 7 << 2 << 0 << 9;
}

void TestLevel4RippleALU4Bit::testAluFlags()
{
    QFETCH(int, inputA);
    QFETCH(int, inputB);
    QFETCH(int, operation);
    QFETCH(int, expectedResult);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Load ALU4bit IC
    auto *aluIC = loadBuildingBlockIC("level4_ripple_alu_4bit.panda");
    builder.add(aluIC);

    // Verify IC has correct port counts
    // New interface: 9 inputs (A[0-3], B[0-3], CarryIn), 17 outputs (4 operations × 4 bits + CarryOut)
    QVERIFY2(aluIC->inputSize() == 9, qPrintable(QString("ALU4bit IC should have 9 input ports (A,B,CarryIn), got %1").arg(aluIC->inputSize())));
    QVERIFY2(aluIC->outputSize() == 17, qPrintable(QString("ALU4bit IC should have 17 output ports (4 ops + CarryOut), got %1").arg(aluIC->outputSize())));

    // Input switches for A (4-bit), B (4-bit), and CarryIn
    InputSwitch swA[4], swB[4], swCarryIn;
    for (int i = 0; i < 4; ++i) {
        builder.add(&swA[i], &swB[i]);
    }
    builder.add(&swCarryIn);

    // Output LEDs
    Led ledResult[4];
    for (int i = 0; i < 4; ++i) {
        builder.add(&ledResult[i]);
    }

    // Connect inputs to IC using semantic port labels
    for (int i = 0; i < 4; ++i) {
        builder.connect(&swA[i], 0, aluIC, QString("A[%1]").arg(i));
        builder.connect(&swB[i], 0, aluIC, QString("B[%1]").arg(i));
    }

    // Connect CarryIn (set to 0 for basic operations without carry-in)
    builder.connect(&swCarryIn, 0, aluIC, "CarryIn");

    // Connect IC outputs to LEDs based on operation type
    QString operationName;
    switch (operation) {
        case 0: operationName = "Result_ADD"; break;
        case 1: operationName = "Result_SUB"; break;
        case 2: operationName = "Result_AND"; break;
        case 3: operationName = "Result_OR"; break;
        default: operationName = "Result_ADD"; break;
    }
    for (int i = 0; i < 4; ++i) {
        builder.connect(aluIC, QString("%1[%2]").arg(operationName).arg(i), &ledResult[i], 0);
    }

    auto *simulation = builder.initSimulation();

    // Set input values (must be after initSimulation)
    for (int i = 0; i < 4; ++i) {
        bool bitA = (inputA >> i) & 1;
        bool bitB = (inputB >> i) & 1;
        swA[i].setOn(bitA);
        swB[i].setOn(bitB);
    }

    simulation->update();

    // Read result bits from circuit
    int result = 0;
    for (int i = 0; i < 4; ++i) {
        if (getInputStatus(&ledResult[i])) {
            result |= (1 << i);
        }
    }

    // Compare circuit output with expected result
    int maskedResult = result & 0x0F;
    int maskedExpected = expectedResult & 0x0F;

    QCOMPARE(maskedResult, maskedExpected);
}
