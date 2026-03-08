// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel6Alu8bit.h"

#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using TestUtils::getOutputStatus;
using TestUtils::readMultiBitOutput;
using TestUtils::setMultiBitInput;
using CPUTestUtils::loadBuildingBlockIC;

// ============================================================
// Test Cases
// ============================================================

void TestLevel6ALU8Bit::testALU8Bit() {
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create 8-bit A and B operand inputs
    QVector<InputSwitch *> a_inputs, b_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *a = new InputSwitch();
        builder.add(a);
        a->setLabel(QString("A[%1]").arg(i));
        a->setPos(50 + i * 60, 100);
        a_inputs.append(a);

        InputSwitch *b = new InputSwitch();
        builder.add(b);
        b->setLabel(QString("B[%1]").arg(i));
        b->setPos(50 + i * 60, 120);
        b_inputs.append(b);
    }

    // Create 3-bit OpCode inputs
    QVector<InputSwitch *> opcode_inputs;
    for (int i = 0; i < 3; i++) {
        InputSwitch *op = new InputSwitch();
        builder.add(op);
        op->setLabel(QString("OpCode[%1]").arg(i));
        op->setPos(600 + i * 60, 100);
        opcode_inputs.append(op);
    }

    // Load and add ALU IC
    IC *alu = loadBuildingBlockIC("level6_alu_8bit.panda");
    builder.add(alu);

    // Create output LEDs
    QVector<Led *> result_outputs;
    for (int i = 0; i < 8; i++) {
        Led *result = new Led();
        builder.add(result);
        result->setLabel(QString("Result[%1]").arg(i));
        result->setPos(50 + i * 60, 500);
        result_outputs.append(result);
    }

    Led *zero_flag = new Led();
    builder.add(zero_flag);
    zero_flag->setLabel("Zero");
    zero_flag->setPos(200, 550);

    Led *negative_flag = new Led();
    builder.add(negative_flag);
    negative_flag->setLabel("Negative");
    negative_flag->setPos(300, 550);

    // Connect inputs to ALU using semantic port labels
    for (int i = 0; i < 8; i++) {
        builder.connect(a_inputs[i], 0, alu, QString("A[%1]").arg(i));
        builder.connect(b_inputs[i], 0, alu, QString("B[%1]").arg(i));
    }
    for (int i = 0; i < 3; i++) {
        builder.connect(opcode_inputs[i], 0, alu, QString("OpCode[%1]").arg(i));
    }

    // Connect ALU outputs to LEDs using semantic port labels
    for (int i = 0; i < 8; i++) {
        builder.connect(alu, QString("Result[%1]").arg(i), result_outputs[i], 0);
    }
    builder.connect(alu, "Zero", zero_flag, 0);
    builder.connect(alu, "Negative", negative_flag, 0);

    Simulation *sim = builder.initSimulation();

    sim->update();

    // Test 1: AND operation (OpCode = 010 = 2)
    setMultiBitInput(a_inputs, CPUTestUtils::PATTERN_LOWER_NIBBLE);
    setMultiBitInput(b_inputs, CPUTestUtils::PATTERN_UPPER_NIBBLE);

    // Set OpCode = 010 (binary) = 2 (decimal)
    opcode_inputs[0]->setOn(false);  // OpCode[0] = 0
    opcode_inputs[1]->setOn(true);   // OpCode[1] = 1
    opcode_inputs[2]->setOn(false);  // OpCode[2] = 0

    sim->update();

    int result_and = readMultiBitOutput(QVector<GraphicElement *>(result_outputs.begin(), result_outputs.end()), 0);

    // Expected: 0x0F AND 0xF0 = 0x00
    // 0x0F = 0000 1111
    // 0xF0 = 1111 0000
    // AND = 0000 0000 = 0x00
    QCOMPARE(result_and, CPUTestUtils::PATTERN_ALL_ZEROS);

    // Test 2: OR operation (OpCode = 011 = 3)
    opcode_inputs[0]->setOn(true);   // OpCode[0] = 1
    opcode_inputs[1]->setOn(true);   // OpCode[1] = 1
    opcode_inputs[2]->setOn(false);  // OpCode[2] = 0
    sim->update();

    int result_or = readMultiBitOutput(QVector<GraphicElement *>(result_outputs.begin(), result_outputs.end()), 0);
    QCOMPARE(result_or, CPUTestUtils::PATTERN_ALL_ONES);  // 0x0F OR 0xF0 = 0xFF

    // Test 3: XOR operation (OpCode = 100 = 4)
    opcode_inputs[0]->setOn(false);  // OpCode[0] = 0
    opcode_inputs[1]->setOn(false);  // OpCode[1] = 0
    opcode_inputs[2]->setOn(true);   // OpCode[2] = 1
    sim->update();
    int result_xor = readMultiBitOutput(QVector<GraphicElement *>(result_outputs.begin(), result_outputs.end()), 0);
    QCOMPARE(result_xor, CPUTestUtils::PATTERN_ALL_ONES);  // 0x0F XOR 0xF0 = 0xFF

    // Test 4: NOT operation (OpCode = 101 = 5, NOT A)
    setMultiBitInput(a_inputs, 0xAA);  // A = 1010 1010
    setMultiBitInput(b_inputs, 0x00);  // B = 0000 0000 (ignored by NOT)
    opcode_inputs[0]->setOn(true);   // OpCode[0] = 1
    opcode_inputs[1]->setOn(false);  // OpCode[1] = 0
    opcode_inputs[2]->setOn(true);   // OpCode[2] = 1
    sim->update();
    int result_not = readMultiBitOutput(QVector<GraphicElement *>(result_outputs.begin(), result_outputs.end()), 0);
    QCOMPARE(result_not, 0x55);  // NOT 0xAA = 0101 0101 = 0x55

    // Test 5: Shift Left operation (OpCode = 110 = 6, A << 1)
    setMultiBitInput(a_inputs, 0x42);  // A = 0100 0010
    setMultiBitInput(b_inputs, 0x00);  // B = 0000 0000 (ignored by SHL)
    opcode_inputs[0]->setOn(false);  // OpCode[0] = 0
    opcode_inputs[1]->setOn(true);   // OpCode[1] = 1
    opcode_inputs[2]->setOn(true);   // OpCode[2] = 1
    sim->update();
    int result_shl = readMultiBitOutput(QVector<GraphicElement *>(result_outputs.begin(), result_outputs.end()), 0);
    QCOMPARE(result_shl, 0x84);  // 0x42 << 1 = 1000 0100 = 0x84

    // Test 6: Shift Right operation (OpCode = 111 = 7, A >> 1)
    setMultiBitInput(a_inputs, 0x85);  // A = 1000 0101
    setMultiBitInput(b_inputs, 0x00);  // B = 0000 0000 (ignored by SHR)
    opcode_inputs[0]->setOn(true);   // OpCode[0] = 1
    opcode_inputs[1]->setOn(true);   // OpCode[1] = 1
    opcode_inputs[2]->setOn(true);   // OpCode[2] = 1
    sim->update();
    int result_shr = readMultiBitOutput(QVector<GraphicElement *>(result_outputs.begin(), result_outputs.end()), 0);
    QCOMPARE(result_shr, 0x42);  // 0x85 >> 1 = 0100 0010 = 0x42
}

void TestLevel6ALU8Bit::testALU8BitStructure() {
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *alu = nullptr;
    try {
        alu = loadBuildingBlockIC("level6_alu_8bit.panda");
    } catch (const std::runtime_error &e) {
        QFAIL(e.what());
        return;
    }

    builder.add(alu);

    // Verify IC has been loaded
    QVERIFY(alu != nullptr);

    // Create dummy connections to verify port access
    InputSwitch *a = new InputSwitch();
    builder.add(a);

    Led *result = new Led();
    builder.add(result);

    // Verify IC port structure
    // Inputs: A[0-7] on 0-7, OpCode[0-2] on 8-10, B[0-7] on 11-18 = 19 inputs
    // Outputs: Result[0-7] + Flags = 10 outputs
    QCOMPARE(alu->inputSize(), 19);   // 8 A + 3 OpCode + 8 B
    QCOMPARE(alu->outputSize(), 11);  // Result[0-7] + Zero + Negative + Carry

    // Verify port connections work (don't throw)
    try {
        builder.connect(a, 0, alu, "A[0]");               // Connect A[0] input
        builder.connect(alu, "Result[0]", result, 0);   // Connect output port
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("IC port access failed: %1").arg(e.what())));
    }
}

void TestLevel6ALU8Bit::testALU8BitInputPortIsolation_data()
{
    QTest::addColumn<int>("bitPosition");

    // Test each input bit individually to verify port isolation
    // When setting A[i]=1 and all other A bits=0, and B=0xFF, AND operation should give only Result[i]=1
    for (int i = 0; i < 8; i++) {
        QTest::newRow(QString("input_bit_%1").arg(i).toLatin1())
            << i;
    }
}

void TestLevel6ALU8Bit::testALU8BitInputPortIsolation()
{
    QFETCH(int, bitPosition);

    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create 8-bit A and B operand inputs
    QVector<InputSwitch *> a_inputs, b_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *a = new InputSwitch();
        builder.add(a);
        a->setLabel(QString("A[%1]").arg(i));
        a_inputs.append(a);

        InputSwitch *b = new InputSwitch();
        builder.add(b);
        b->setLabel(QString("B[%1]").arg(i));
        b_inputs.append(b);
    }

    // Create 3-bit OpCode inputs
    QVector<InputSwitch *> opcode_inputs;
    for (int i = 0; i < 3; i++) {
        InputSwitch *op = new InputSwitch();
        builder.add(op);
        op->setLabel(QString("OpCode[%1]").arg(i));
        opcode_inputs.append(op);
    }

    // Load and add ALU IC
    IC *alu = loadBuildingBlockIC("level6_alu_8bit.panda");
    builder.add(alu);

    // Create output LEDs
    QVector<Led *> result_outputs;
    for (int i = 0; i < 8; i++) {
        Led *result = new Led();
        builder.add(result);
        result->setLabel(QString("Result[%1]").arg(i));
        result_outputs.append(result);
    }

    // Create flag outputs
    Led *zero_flag = new Led();
    builder.add(zero_flag);
    Led *negative_flag = new Led();
    builder.add(negative_flag);

    // Connect all inputs to ALU
    for (int i = 0; i < 8; i++) {
        builder.connect(a_inputs[i], 0, alu, QString("A[%1]").arg(i));
        builder.connect(b_inputs[i], 0, alu, QString("B[%1]").arg(i));
    }
    for (int i = 0; i < 3; i++) {
        builder.connect(opcode_inputs[i], 0, alu, QString("OpCode[%1]").arg(i));
    }

    // Connect all outputs
    for (int i = 0; i < 8; i++) {
        builder.connect(alu, QString("Result[%1]").arg(i), result_outputs[i], 0);
    }
    builder.connect(alu, "Zero", zero_flag, 0);
    builder.connect(alu, "Negative", negative_flag, 0);

    Simulation *sim = builder.initSimulation();
    sim->update();

    // Set OpCode = 010 (binary) = 2 (decimal) for AND operation
    opcode_inputs[0]->setOn(false);  // OpCode[0] = 0
    opcode_inputs[1]->setOn(true);   // OpCode[1] = 1
    opcode_inputs[2]->setOn(false);  // OpCode[2] = 0

    // Set B to all 1s (0xFF)
    for (int i = 0; i < 8; i++) {
        b_inputs[i]->setOn(true);
    }

    // Set A with only bit at bitPosition = 1, all others = 0
    for (int i = 0; i < 8; i++) {
        a_inputs[i]->setOn(i == bitPosition);
    }

    sim->update();

    // Read result
    int result = readMultiBitOutput(QVector<GraphicElement *>(result_outputs.begin(), result_outputs.end()), 0);

    // Expected: only bit at bitPosition should be 1
    // Result = A AND B = (only bitPosition set) AND (all bits set) = only bitPosition set
    int expected = (1 << bitPosition);

    QCOMPARE(result, expected);
}

void TestLevel6ALU8Bit::testALU8BitOutputPortIsolation_data()
{
    QTest::addColumn<int>("bitPosition");

    // Test each output bit independently
    for (int i = 0; i < 8; i++) {
        QTest::newRow(QString("output_bit_%1").arg(i).toLatin1())
            << i;
    }
}

void TestLevel6ALU8Bit::testALU8BitOutputPortIsolation()
{
    QFETCH(int, bitPosition);

    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create 8-bit A and B operand inputs
    QVector<InputSwitch *> a_inputs, b_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *a = new InputSwitch();
        builder.add(a);
        a_inputs.append(a);

        InputSwitch *b = new InputSwitch();
        builder.add(b);
        b_inputs.append(b);
    }

    // Create 3-bit OpCode inputs
    QVector<InputSwitch *> opcode_inputs;
    for (int i = 0; i < 3; i++) {
        InputSwitch *op = new InputSwitch();
        builder.add(op);
        opcode_inputs.append(op);
    }

    // Load and add ALU IC
    IC *alu = loadBuildingBlockIC("level6_alu_8bit.panda");
    builder.add(alu);

    // Create output LEDs for all result bits
    QVector<Led *> result_outputs;
    for (int i = 0; i < 8; i++) {
        Led *result = new Led();
        builder.add(result);
        result_outputs.append(result);
    }

    // Create flag outputs
    Led *zero_flag = new Led();
    builder.add(zero_flag);
    Led *negative_flag = new Led();
    builder.add(negative_flag);

    // Connect all inputs
    for (int i = 0; i < 8; i++) {
        builder.connect(a_inputs[i], 0, alu, QString("A[%1]").arg(i));
        builder.connect(b_inputs[i], 0, alu, QString("B[%1]").arg(i));
    }
    for (int i = 0; i < 3; i++) {
        builder.connect(opcode_inputs[i], 0, alu, QString("OpCode[%1]").arg(i));
    }

    // Connect all outputs
    for (int i = 0; i < 8; i++) {
        builder.connect(alu, QString("Result[%1]").arg(i), result_outputs[i], 0);
    }
    builder.connect(alu, "Zero", zero_flag, 0);
    builder.connect(alu, "Negative", negative_flag, 0);

    Simulation *sim = builder.initSimulation();
    sim->update();

    // Set OpCode = 011 (binary) = 3 (decimal) for OR operation
    opcode_inputs[0]->setOn(true);   // OpCode[0] = 1
    opcode_inputs[1]->setOn(true);   // OpCode[1] = 1
    opcode_inputs[2]->setOn(false);  // OpCode[2] = 0

    // Test: Set A[bitPosition]=1, all other A bits=0, B=all 0s
    // With OR: Result[bitPosition] = 1, all others = 0
    for (int i = 0; i < 8; i++) {
        a_inputs[i]->setOn(i == bitPosition);
        b_inputs[i]->setOn(false);
    }

    sim->update();

    // Verify only the tested bit has the expected value
    // All other bits should be 0
    for (int i = 0; i < 8; i++) {
        bool bit_result = TestUtils::getInputStatus(result_outputs[i], 0);
        bool expected = (i == bitPosition);

        QCOMPARE(bit_result, expected);
    }
}

