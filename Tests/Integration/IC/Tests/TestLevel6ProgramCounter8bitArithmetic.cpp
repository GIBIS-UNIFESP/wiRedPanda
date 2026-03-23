// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel6ProgramCounter8bitArithmetic.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::readMultiBitOutput;
using TestUtils::setMultiBitInput;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel6ProgramCounter8BitArithmetic::testProgramCounter8BitWithPCPlus1() {
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create 8-bit Data input
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

    InputSwitch *inc = new InputSwitch();
    builder.add(inc);
    inc->setLabel("Inc");
    inc->setPos(600, 160);

    InputSwitch *reset = new InputSwitch();
    builder.add(reset);
    reset->setLabel("Reset");
    reset->setPos(600, 180);

    // Load and add PC IC (Level 6 - has both pc and pc_plus_1 outputs)
    IC *pc = loadBuildingBlockIC("level6_program_counter_8bit_arithmetic.panda");
    builder.add(pc);

    // Create output LEDs for pc[0-7]
    QVector<Led *> pc_outputs;
    for (int i = 0; i < 8; i++) {
        Led *p = new Led();
        builder.add(p);
        p->setLabel(QString("pc[%1]").arg(i));
        p->setPos(50 + i * 60, 500);
        pc_outputs.append(p);
    }

    // Create output LEDs for pc_plus_1[0-7]
    QVector<Led *> pc_plus_1_outputs;
    for (int i = 0; i < 8; i++) {
        Led *p1 = new Led();
        builder.add(p1);
        p1->setLabel(QString("pc_plus_1[%1]").arg(i));
        p1->setPos(50 + i * 60, 550);
        pc_plus_1_outputs.append(p1);
    }

    // Connect inputs to PC IC using semantic port labels
    for (int i = 0; i < 8; i++) {
        builder.connect(data_inputs[i], 0, pc, QString("loadValue[%1]").arg(i));
    }
    builder.connect(clock, 0, pc, "clock");
    builder.connect(load, 0, pc, "load");
    builder.connect(inc, 0, pc, "inc");
    builder.connect(reset, 0, pc, "reset");

    // Connect PC outputs to LEDs
    for (int i = 0; i < 8; i++) {
        builder.connect(pc, QString("pc[%1]").arg(i), pc_outputs[i], 0);
        builder.connect(pc, QString("pc_plus_1[%1]").arg(i), pc_plus_1_outputs[i], 0);
    }

    Simulation *sim = builder.initSimulation();
    sim->update();

    // Test 1: Load 0x42 and verify pc_plus_1 = 0x43
    setMultiBitInput(data_inputs, 0x42);
    load->setOn(true);
    reset->setOn(false);
    sim->update();
    clockCycle(sim, clock);
    sim->update();

    int pc_val = readMultiBitOutput(QVector<GraphicElement *>(pc_outputs.begin(), pc_outputs.end()), 0);
    int pc_plus_1_val = readMultiBitOutput(QVector<GraphicElement *>(pc_plus_1_outputs.begin(), pc_plus_1_outputs.end()), 0);
    QCOMPARE(pc_val, 0x42);
    QCOMPARE(pc_plus_1_val, 0x43);  // 0x42 + 1 = 0x43

    // Test 2: Increment to 0x43 and verify pc_plus_1 = 0x44
    clock->setOn(false);
    load->setOn(false);
    inc->setOn(true);
    sim->update();
    clockCycle(sim, clock);
    sim->update();

    pc_val = readMultiBitOutput(QVector<GraphicElement *>(pc_outputs.begin(), pc_outputs.end()), 0);
    pc_plus_1_val = readMultiBitOutput(QVector<GraphicElement *>(pc_plus_1_outputs.begin(), pc_plus_1_outputs.end()), 0);
    QCOMPARE(pc_val, 0x43);
    QCOMPARE(pc_plus_1_val, 0x44);  // 0x43 + 1 = 0x44

    // Test 3: Increment to 0xFF and verify pc_plus_1 = 0x00 (wraparound)
    clock->setOn(false);
    setMultiBitInput(data_inputs, 0xFF);
    load->setOn(true);
    inc->setOn(false);
    sim->update();
    clockCycle(sim, clock);
    sim->update();

    pc_val = readMultiBitOutput(QVector<GraphicElement *>(pc_outputs.begin(), pc_outputs.end()), 0);
    pc_plus_1_val = readMultiBitOutput(QVector<GraphicElement *>(pc_plus_1_outputs.begin(), pc_plus_1_outputs.end()), 0);
    QCOMPARE(pc_val, 0xFF);
    QCOMPARE(pc_plus_1_val, 0x00);  // 0xFF + 1 = 0x100, but 8-bit = 0x00
}

