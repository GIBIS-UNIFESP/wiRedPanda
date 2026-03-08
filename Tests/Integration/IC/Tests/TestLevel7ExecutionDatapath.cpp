// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel7ExecutionDatapath.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::setMultiBitInput;
using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel7ExecutionDatapath::initTestCase()
{
}

void TestLevel7ExecutionDatapath::cleanup()
{
}

void TestLevel7ExecutionDatapath::testExecutionDatapath_data()
{
    QTest::addColumn<int>("operandA");
    QTest::addColumn<int>("operandB");
    QTest::addColumn<int>("opCode");
    QTest::addColumn<int>("expectedResult");
    QTest::addColumn<bool>("expectedZero");
    QTest::addColumn<bool>("expectedSign");

    // OpCode: 0=ADD, 1=SUB, 2=AND, 3=OR, 4=XOR, 5=NOT, 6=SHL, 7=SHR

    // ADD tests
    QTest::newRow("ADD: 0x50 + 0x30 = 0x80") << 0x50 << 0x30 << 0 << 0x80 << false << true;   // Result is negative (MSB=1)
    QTest::newRow("ADD: 0x10 + 0x20 = 0x30") << 0x10 << 0x20 << 0 << 0x30 << false << false;  // Result is positive
    QTest::newRow("ADD: 0xFF + 0x01 = 0x00") << 0xFF << 0x01 << 0 << 0x00 << true << false;   // Zero flag should be set (wraparound)

    // SUB tests
    QTest::newRow("SUB: 0x50 - 0x30 = 0x20") << 0x50 << 0x30 << 1 << 0x20 << false << false;  // Positive result
    QTest::newRow("SUB: 0x30 - 0x50 = 0xE0") << 0x30 << 0x50 << 1 << 0xE0 << false << true;   // Negative (2's complement)
    QTest::newRow("SUB: 0x50 - 0x50 = 0x00") << 0x50 << 0x50 << 1 << 0x00 << true << false;   // Zero flag should be set

    // AND tests
    QTest::newRow("AND: 0xF0 & 0x0F = 0x00") << 0xF0 << 0x0F << 2 << 0x00 << true << false;   // Zero result
    QTest::newRow("AND: 0xF0 & 0xF0 = 0xF0") << 0xF0 << 0xF0 << 2 << 0xF0 << false << true;   // Negative result

    // OR tests
    QTest::newRow("OR: 0xF0 | 0x0F = 0xFF") << 0xF0 << 0x0F << 3 << 0xFF << false << true;    // All bits set, negative
    QTest::newRow("OR: 0x00 | 0x00 = 0x00") << 0x00 << 0x00 << 3 << 0x00 << true << false;    // Zero result

    // XOR tests
    QTest::newRow("XOR: 0xAA ^ 0x55 = 0xFF") << 0xAA << 0x55 << 4 << 0xFF << false << true;   // All bits set
    QTest::newRow("XOR: 0xAA ^ 0xAA = 0x00") << 0xAA << 0xAA << 4 << 0x00 << true << false;   // Zero result

    // NOT tests (B operand is ignored)
    QTest::newRow("NOT: ~0xAA = 0x55") << 0xAA << 0x00 << 5 << 0x55 << false << false;        // Positive result
    QTest::newRow("NOT: ~0x7F = 0x80") << 0x7F << 0x00 << 5 << 0x80 << false << true;         // Negative result
    QTest::newRow("NOT: ~0xFF = 0x00") << 0xFF << 0x00 << 5 << 0x00 << true << false;         // Zero result

    // SHL tests (left shift, B operand might control shift amount, but testing basic SHL)
    QTest::newRow("SHL: 0x40 << 1 = 0x80") << 0x40 << 0x00 << 6 << 0x80 << false << true;     // Negative result after shift
    QTest::newRow("SHL: 0x01 << 1 = 0x02") << 0x01 << 0x00 << 6 << 0x02 << false << false;    // Positive result

    // SHR tests (right shift, B operand might control shift amount, but testing basic SHR)
    QTest::newRow("SHR: 0x80 >> 1 = 0x40") << 0x80 << 0x00 << 7 << 0x40 << false << false;    // Positive result after shift
    QTest::newRow("SHR: 0x01 >> 1 = 0x00") << 0x01 << 0x00 << 7 << 0x00 << true << false;     // Zero result after shift
}

void TestLevel7ExecutionDatapath::testExecutionDatapath()
{
    QFETCH(int, operandA);
    QFETCH(int, operandB);
    QFETCH(int, opCode);
    QFETCH(int, expectedResult);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedSign);

    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create OperandA inputs (8-bit)
    QVector<InputSwitch *> operandA_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("OperandA[%1]").arg(i));
        sw->setPos(50 + i * 60, 100);
        operandA_inputs.append(sw);
    }

    // Create OperandB inputs (8-bit)
    QVector<InputSwitch *> operandB_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("OperandB[%1]").arg(i));
        sw->setPos(50 + i * 60, 140);
        operandB_inputs.append(sw);
    }

    // Create OpCode inputs (3-bit)
    QVector<InputSwitch *> opcode_inputs;
    for (int i = 0; i < 3; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("OpCode[%1]").arg(i));
        sw->setPos(550 + i * 60, 100);
        opcode_inputs.append(sw);
    }

    // Load the execution datapath IC
    IC *datapath = loadBuildingBlockIC("level7_execution_datapath.panda");
    builder.add(datapath);

    // Create output LEDs for Result (8-bit)
    QVector<Led *> result_leds;
    for (int i = 0; i < 8; i++) {
        Led *led = new Led();
        builder.add(led);
        led->setLabel(QString("Result[%1]").arg(i));
        led->setPos(50 + i * 60, 200);
        result_leds.append(led);
    }

    // Create Zero flag output LED
    Led *zero_led = new Led();
    builder.add(zero_led);
    zero_led->setLabel("Zero");
    zero_led->setPos(550, 200);

    // Create Sign flag output LED
    Led *sign_led = new Led();
    builder.add(sign_led);
    sign_led->setLabel("Sign");
    sign_led->setPos(550, 240);

    // Connect all inputs to execution datapath
    for (int i = 0; i < 8; i++) {
        builder.connect(operandA_inputs[i], 0, datapath, QString("OperandA[%1]").arg(i));
        builder.connect(operandB_inputs[i], 0, datapath, QString("OperandB[%1]").arg(i));
    }

    for (int i = 0; i < 3; i++) {
        builder.connect(opcode_inputs[i], 0, datapath, QString("OpCode[%1]").arg(i));
    }

    // Connect outputs to LEDs
    for (int i = 0; i < 8; i++) {
        builder.connect(datapath, QString("Result[%1]").arg(i), result_leds[i], 0);
    }

    builder.connect(datapath, "Zero", zero_led, 0);
    builder.connect(datapath, "Sign", sign_led, 0);

    Simulation *sim = builder.initSimulation();
    sim->update();

    // Set input data
    setMultiBitInput(operandA_inputs, operandA);
    setMultiBitInput(operandB_inputs, operandB);
    setMultiBitInput(opcode_inputs, opCode);
    sim->update();

    // Read outputs
    int result = readMultiBitOutput(QVector<GraphicElement *>(result_leds.begin(), result_leds.end()), 0);
    bool zero = TestUtils::getInputStatus(zero_led, 0);
    bool sign = TestUtils::getInputStatus(sign_led, 0);

    // Verify results
    QCOMPARE(result, expectedResult);
    QCOMPARE(zero, expectedZero);
    QCOMPARE(sign, expectedSign);
}

void TestLevel7ExecutionDatapath::testExecutionDatapathStructure()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *datapath = loadBuildingBlockIC("level7_execution_datapath.panda");
    builder.add(datapath);

    // Verify IC has been loaded
    QVERIFY(datapath != nullptr);

    // Create dummy elements for connection testing
    InputSwitch *data = new InputSwitch();
    builder.add(data);

    Led *out = new Led();
    builder.add(out);

    // Verify IC port structure
    // Inputs: 8 OperandA + 8 OperandB + 3 OpCode = 19 inputs
    // Outputs: 8 Result + 1 Zero + 1 Sign = 10 outputs
    QCOMPARE(datapath->inputSize(), 19);   // 8+8+3
    QCOMPARE(datapath->outputSize(), 10);  // 8+1+1

    // Verify port connections work (don't throw)
    try {
        builder.connect(data, 0, datapath, "OperandA[0]");
        builder.connect(data, 0, datapath, "OperandB[0]");
        builder.connect(data, 0, datapath, "OpCode[0]");
        builder.connect(datapath, "Result[0]", out, 0);
        builder.connect(datapath, "Zero", out, 0);
        builder.connect(datapath, "Sign", out, 0);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("IC port access failed: %1").arg(e.what())));
    }
}
