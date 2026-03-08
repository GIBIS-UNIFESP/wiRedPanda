// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel7InstructionRegister8bit.h"

#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::readMultiBitOutput;
using TestUtils::setMultiBitInput;
using CPUTestUtils::loadBuildingBlockIC;

// ============================================================
// Test Cases
// ============================================================

void TestLevel7InstructionRegister8Bit::testInstructionRegister8Bit() {
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create 8-bit Data input (instruction from memory)
    QVector<InputSwitch *> data_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *data = new InputSwitch();
        builder.add(data);
        data->setLabel(QString("Data[%1]").arg(i));
        data->setPos(50 + i * 60, 100);
        data_inputs.append(data);
    }

    // Create control signals
    InputSwitch *clock = new InputSwitch();
    builder.add(clock);
    clock->setLabel("Clock");
    clock->setPos(600, 100);

    InputSwitch *load = new InputSwitch();
    builder.add(load);
    load->setLabel("Load");
    load->setPos(600, 140);

    InputSwitch *reset = new InputSwitch();
    builder.add(reset);
    reset->setLabel("Reset");
    reset->setPos(600, 180);

    // Load and add Instruction Register IC
    IC *ir = loadBuildingBlockIC("level7_instruction_register_8bit.panda");
    builder.add(ir);

    // Create output LEDs for full instruction
    QVector<Led *> instr_outputs;
    for (int i = 0; i < 8; i++) {
        Led *instr = new Led();
        builder.add(instr);
        instr->setLabel(QString("Instr[%1]").arg(i));
        instr->setPos(50 + i * 60, 500);
        instr_outputs.append(instr);
    }

    // Create output LEDs for OpCode (bits 3-7)
    QVector<Led *> opcode_outputs;
    for (int i = 0; i < 5; i++) {
        Led *op = new Led();
        builder.add(op);
        op->setLabel(QString("OpCode[%1]").arg(i));
        op->setPos(50 + i * 60, 550);
        opcode_outputs.append(op);
    }

    // Create output LEDs for RegisterAddr (bits 0-2)
    QVector<Led *> regaddr_outputs;
    for (int i = 0; i < 3; i++) {
        Led *addr = new Led();
        builder.add(addr);
        addr->setLabel(QString("RegAddr[%1]").arg(i));
        addr->setPos(50 + i * 60, 600);
        regaddr_outputs.append(addr);
    }

    // Connect inputs to IR using semantic port labels
    for (int i = 0; i < 8; i++) {
        builder.connect(data_inputs[i], 0, ir, QString("Data[%1]").arg(i));
    }
    builder.connect(clock, 0, ir, "Clock");
    builder.connect(load, 0, ir, "Load");
    builder.connect(reset, 0, ir, "Reset");

    // Connect IR outputs to LEDs using semantic port labels
    for (int i = 0; i < 8; i++) {
        builder.connect(ir, QString("Instruction[%1]").arg(i), instr_outputs[i], 0);
    }
    for (int i = 0; i < 5; i++) {
        builder.connect(ir, QString("OpCode[%1]").arg(i), opcode_outputs[i], 0);
    }
    for (int i = 0; i < 3; i++) {
        builder.connect(ir, QString("RegisterAddr[%1]").arg(i), regaddr_outputs[i], 0);
    }

    Simulation *sim = builder.initSimulation();
    sim->update();

    // Test 1: Load instruction 0xB5 (binary: 10110101)
    // OpCode should be bits 7-3 = 10110 (binary) = 22 (decimal)
    // RegisterAddr should be bits 2-0 = 101 (binary) = 5 (decimal)
    setMultiBitInput(data_inputs, 0xB5);
    load->setOn(true);
    reset->setOn(false);

    sim->update();
    clockCycle(sim, clock);  // LOW→HIGH rising edge: captures 0xB5
    sim->update();

    int instr = readMultiBitOutput(QVector<GraphicElement *>(instr_outputs.begin(), instr_outputs.end()), 0);
    QCOMPARE(instr, 0xB5);

    // Verify opcode extraction (bits 7-3 = 22)
    int opcode = readMultiBitOutput(QVector<GraphicElement *>(opcode_outputs.begin(), opcode_outputs.end()), 0);
    QCOMPARE(opcode, 22);  // 0xB5 >> 3 = 0x16 = 22

    // Verify register address extraction (bits 2-0 = 5)
    int regaddr = readMultiBitOutput(QVector<GraphicElement *>(regaddr_outputs.begin(), regaddr_outputs.end()), 0);
    QCOMPARE(regaddr, 5);  // 0xB5 & 0x07 = 0x05 = 5

    // Test 2: Load another instruction 0x73 (binary: 01110011)
    // OpCode = 01110 = 14
    // RegisterAddr = 011 = 3
    // Reset clock to LOW for consistent test conditions
    clock->setOn(false);
    sim->update();

    setMultiBitInput(data_inputs, 0x73);
    sim->update();
    clockCycle(sim, clock);  // LOW→HIGH rising edge: captures 0x73
    sim->update();

    instr = readMultiBitOutput(QVector<GraphicElement *>(instr_outputs.begin(), instr_outputs.end()), 0);
    QCOMPARE(instr, 0x73);

    opcode = readMultiBitOutput(QVector<GraphicElement *>(opcode_outputs.begin(), opcode_outputs.end()), 0);
    QCOMPARE(opcode, 14);  // 0x73 >> 3 = 0x0E = 14

    regaddr = readMultiBitOutput(QVector<GraphicElement *>(regaddr_outputs.begin(), regaddr_outputs.end()), 0);
    QCOMPARE(regaddr, 3);  // 0x73 & 0x07 = 0x03 = 3

    // Test 3: Reset
    clock->setOn(false);
    reset->setOn(true);
    sim->update();
    instr = readMultiBitOutput(QVector<GraphicElement *>(instr_outputs.begin(), instr_outputs.end()), 0);
    QCOMPARE(instr, CPUTestUtils::PATTERN_ALL_ZEROS);
}

void TestLevel7InstructionRegister8Bit::testInstructionRegister8BitStructure() {
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *ir = loadBuildingBlockIC("level7_instruction_register_8bit.panda");

    builder.add(ir);

    // Verify IC has been loaded
    QVERIFY(ir != nullptr);

    // Create dummy connections to verify port access
    InputSwitch *data = new InputSwitch();
    builder.add(data);

    Led *instr = new Led();
    builder.add(instr);

    Led *opcode = new Led();
    builder.add(opcode);

    // Verify IC port structure
    QCOMPARE(ir->inputSize(), 11);   // 8 Data + Clock + Load + Reset
    QCOMPARE(ir->outputSize(), 16);  // 8 Instruction + 5 OpCode + 3 RegisterAddr

    // Verify port connections work (don't throw)
    try {
        builder.connect(data, 0, ir, "Data[0]");           // Connect Data[0] input
        builder.connect(ir, "Instruction[0]", instr, 0); // Connect Instruction[0] output
        builder.connect(ir, "OpCode[0]", opcode, 0);     // Connect OpCode[0] output
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("IC port access failed: %1").arg(e.what())));
    }
}
