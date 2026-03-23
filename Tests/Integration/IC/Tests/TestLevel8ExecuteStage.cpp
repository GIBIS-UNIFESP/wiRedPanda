// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel8ExecuteStage.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::readMultiBitOutput;
using TestUtils::setMultiBitInput;
using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel8ExecuteStage::initTestCase()
{
}

void TestLevel8ExecuteStage::cleanup()
{
}

void TestLevel8ExecuteStage::testExecuteStage_data()
{
    QTest::addColumn<int>("operandA");
    QTest::addColumn<int>("operandB");
    QTest::addColumn<int>("aluOp");
    QTest::addColumn<int>("expectedResult");
    QTest::addColumn<bool>("expectedZero");
    QTest::addColumn<bool>("expectedSign");

    // ADD (OpCode 0)
    QTest::newRow("ADD 0x42 + 0x33 = 0x75") << 0x42 << 0x33 << 0 << 0x75 << false << false;
    QTest::newRow("ADD 0xFF + 0x01 = 0x00 (zero)") << 0xFF << 0x01 << 0 << 0x00 << true << false;
    QTest::newRow("ADD 0x80 + 0x80 = 0x00 (zero, sign)") << 0x80 << 0x80 << 0 << 0x00 << true << false;

    // SUB (OpCode 1)
    QTest::newRow("SUB 0x50 - 0x30 = 0x20") << 0x50 << 0x30 << 1 << 0x20 << false << false;
    QTest::newRow("SUB 0x42 - 0x42 = 0x00 (zero)") << 0x42 << 0x42 << 1 << 0x00 << true << false;
    QTest::newRow("SUB 0x30 - 0x50 = 0xE0 (sign)") << 0x30 << 0x50 << 1 << 0xE0 << false << true;

    // AND (OpCode 2)
    QTest::newRow("AND 0xF0 & 0x0F = 0x00") << 0xF0 << 0x0F << 2 << 0x00 << true << false;
    QTest::newRow("AND 0xFF & 0x42 = 0x42") << 0xFF << 0x42 << 2 << 0x42 << false << false;
    QTest::newRow("AND 0xAA & 0x55 = 0x00") << 0xAA << 0x55 << 2 << 0x00 << true << false;

    // OR (OpCode 3)
    QTest::newRow("OR 0xF0 | 0x0F = 0xFF") << 0xF0 << 0x0F << 3 << 0xFF << false << true;
    QTest::newRow("OR 0x42 | 0x00 = 0x42") << 0x42 << 0x00 << 3 << 0x42 << false << false;
    QTest::newRow("OR 0x00 | 0x00 = 0x00 (zero)") << 0x00 << 0x00 << 3 << 0x00 << true << false;

    // XOR (OpCode 4)
    QTest::newRow("XOR 0xFF ^ 0xFF = 0x00") << 0xFF << 0xFF << 4 << 0x00 << true << false;
    QTest::newRow("XOR 0xFF ^ 0x00 = 0xFF") << 0xFF << 0x00 << 4 << 0xFF << false << true;
    QTest::newRow("XOR 0xAA ^ 0x55 = 0xFF") << 0xAA << 0x55 << 4 << 0xFF << false << true;

    // NOT (OpCode 5)
    QTest::newRow("NOT ~0x00 = 0xFF") << 0x00 << 0x00 << 5 << 0xFF << false << true;
    QTest::newRow("NOT ~0xFF = 0x00") << 0xFF << 0x00 << 5 << 0x00 << true << false;
    QTest::newRow("NOT ~0x42 = 0xBD") << 0x42 << 0x00 << 5 << 0xBD << false << true;

    // SHL (OpCode 6)
    QTest::newRow("SHL 0x42 << 1 = 0x84") << 0x42 << 0x00 << 6 << 0x84 << false << true;
    QTest::newRow("SHL 0x01 << 1 = 0x02") << 0x01 << 0x00 << 6 << 0x02 << false << false;
    QTest::newRow("SHL 0x80 << 1 = 0x00 (overflow)") << 0x80 << 0x00 << 6 << 0x00 << true << false;

    // SHR (OpCode 7)
    QTest::newRow("SHR 0x84 >> 1 = 0x42") << 0x84 << 0x00 << 7 << 0x42 << false << false;
    QTest::newRow("SHR 0xFF >> 1 = 0x7F") << 0xFF << 0x00 << 7 << 0x7F << false << false;
    QTest::newRow("SHR 0x01 >> 1 = 0x00 (zero)") << 0x01 << 0x00 << 7 << 0x00 << true << false;
}

void TestLevel8ExecuteStage::testExecuteStage()
{
    QFETCH(int, operandA);
    QFETCH(int, operandB);
    QFETCH(int, aluOp);
    QFETCH(int, expectedResult);
    QFETCH(bool, expectedZero);
    QFETCH(bool, expectedSign);

    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create OperandA input switches (8-bit)
    QVector<InputSwitch *> operandA_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("OperandA[%1]").arg(i));
        sw->setPos(50 + i * 60, 100);
        operandA_inputs.append(sw);
    }

    // Create OperandB input switches (8-bit)
    QVector<InputSwitch *> operandB_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("OperandB[%1]").arg(i));
        sw->setPos(50 + i * 60, 140);
        operandB_inputs.append(sw);
    }

    // Create ALUOp input switches (3-bit)
    QVector<InputSwitch *> aluop_inputs;
    for (int i = 0; i < 3; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("ALUOp[%1]").arg(i));
        sw->setPos(650 + i * 60, 100);
        aluop_inputs.append(sw);
    }

    // Create clock and reset
    InputSwitch *clk = new InputSwitch();
    builder.add(clk);
    clk->setLabel("Clock");
    clk->setPos(900, 100);

    InputSwitch *reset = new InputSwitch();
    builder.add(reset);
    reset->setLabel("Reset");
    reset->setPos(900, 140);

    // Load the execute stage IC
    IC *execute = loadBuildingBlockIC("level8_execute_stage.panda");
    builder.add(execute);

    // Create output LEDs for Result (8-bit)
    QVector<Led *> result_leds;
    for (int i = 0; i < 8; i++) {
        Led *led = new Led();
        builder.add(led);
        led->setLabel(QString("Result[%1]").arg(i));
        led->setPos(50 + i * 60, 200);
        result_leds.append(led);
    }

    // Create flag output LEDs
    Led *zero_led = new Led();
    builder.add(zero_led);
    zero_led->setLabel("Zero");
    zero_led->setPos(650, 200);

    Led *sign_led = new Led();
    builder.add(sign_led);
    sign_led->setLabel("Sign");
    sign_led->setPos(710, 200);

    // Connect OperandA
    for (int i = 0; i < 8; i++) {
        builder.connect(operandA_inputs[i], 0, execute, QString("OperandA[%1]").arg(i));
    }

    // Connect OperandB
    for (int i = 0; i < 8; i++) {
        builder.connect(operandB_inputs[i], 0, execute, QString("OperandB[%1]").arg(i));
    }

    // Connect ALUOp
    for (int i = 0; i < 3; i++) {
        builder.connect(aluop_inputs[i], 0, execute, QString("ALUOp[%1]").arg(i));
    }

    // Connect control signals
    builder.connect(clk, 0, execute, "Clock");
    builder.connect(reset, 0, execute, "Reset");

    // Connect outputs to LEDs
    for (int i = 0; i < 8; i++) {
        builder.connect(execute, QString("Result[%1]").arg(i), result_leds[i], 0);
    }
    builder.connect(execute, "Zero", zero_led, 0);
    builder.connect(execute, "Sign", sign_led, 0);

    Simulation *sim = builder.initSimulation();
    sim->update();

    // Set operand values
    setMultiBitInput(operandA_inputs, operandA);
    setMultiBitInput(operandB_inputs, operandB);
    setMultiBitInput(aluop_inputs, aluOp);
    sim->update();

    // Read outputs
    int result = readMultiBitOutput(QVector<GraphicElement *>(result_leds.begin(), result_leds.end()), 0);
    bool zero = getInputStatus(zero_led);
    bool sign = getInputStatus(sign_led);

    // Verify
    QCOMPARE(result, expectedResult);
    QCOMPARE(zero, expectedZero);
    QCOMPARE(sign, expectedSign);
}

void TestLevel8ExecuteStage::testExecuteStageStructure()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *execute = loadBuildingBlockIC("level8_execute_stage.panda");
    builder.add(execute);

    // Verify IC has been loaded
    QVERIFY(execute != nullptr);

    // Create dummy elements for connection testing
    InputSwitch *sw = new InputSwitch();
    builder.add(sw);

    Led *out = new Led();
    builder.add(out);

    // Verify IC port structure
    // Inputs: 8 OperandA + 8 OperandB + 3 ALUOp + Clock + Reset = 21 inputs (datapath has extra internal input)
    // Outputs: 8 Result + Zero + Sign = 10 outputs
    QCOMPARE(execute->inputSize(), 21);  // 8 + 8 + 3 + 2 control signals + datapath internal
    QCOMPARE(execute->outputSize(), 10); // 8 result + 2 flags

    // Verify port connections work (don't throw)
    try {
        builder.connect(sw, 0, execute, "OperandA[0]");
        builder.connect(sw, 0, execute, "OperandA[7]");
        builder.connect(sw, 0, execute, "OperandB[0]");
        builder.connect(sw, 0, execute, "OperandB[7]");
        builder.connect(sw, 0, execute, "ALUOp[0]");
        builder.connect(sw, 0, execute, "ALUOp[2]");
        builder.connect(sw, 0, execute, "Clock");
        builder.connect(sw, 0, execute, "Reset");
        builder.connect(execute, "Result[0]", out, 0);
        builder.connect(execute, "Result[7]", out, 0);
        builder.connect(execute, "Zero", out, 0);
        builder.connect(execute, "Sign", out, 0);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Execute stage port access failed: %1").arg(e.what())));
    }
}

