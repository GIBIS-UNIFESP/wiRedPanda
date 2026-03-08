// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel5ProgramCounter4bit.h"

#include <QFile>
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

void TestLevel5ProgramCounter4Bit::initTestCase()
{
}

void TestLevel5ProgramCounter4Bit::cleanup()
{
}

void TestLevel5ProgramCounter4Bit::testProgramCounter4Bit_data()
{
    QTest::addColumn<int>("initialValue");
    QTest::addColumn<int>("expectedAfterLoad");
    QTest::addColumn<int>("expectedAfterIncrement");

    // Test loading and incrementing 4-bit values
    QTest::newRow("load 0x0")  << 0x0 << 0x0 << 0x1;
    QTest::newRow("load 0x5")  << 0x5 << 0x5 << 0x6;
    QTest::newRow("load 0xA")  << 0xA << 0xA << 0xB;
    QTest::newRow("load 0xF")  << 0xF << 0xF << 0x0;  // Wraparound: 0xF + 1 = 0x10 & 0x0F = 0x0
}

void TestLevel5ProgramCounter4Bit::testProgramCounter4Bit()
{
    QFETCH(int, initialValue);
    QFETCH(int, expectedAfterLoad);
    QFETCH(int, expectedAfterIncrement);

    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create 4-bit load value inputs
    QVector<InputSwitch *> load_value_inputs;
    for (int i = 0; i < 4; i++) {
        InputSwitch *lv = new InputSwitch();
        builder.add(lv);
        lv->setLabel(QString("loadValue[%1]").arg(i));
        lv->setPos(50 + i * 60, 100);
        load_value_inputs.append(lv);
    }

    // Create control signals
    InputSwitch *clock = new InputSwitch();
    builder.add(clock);
    clock->setLabel("Clock");
    clock->setPos(350, 100);

    InputSwitch *load = new InputSwitch();
    builder.add(load);
    load->setLabel("Load");
    load->setPos(350, 140);

    InputSwitch *inc = new InputSwitch();
    builder.add(inc);
    inc->setLabel("Inc");
    inc->setPos(350, 160);

    InputSwitch *reset = new InputSwitch();
    builder.add(reset);
    reset->setLabel("Reset");
    reset->setPos(350, 180);

    // Load and add PC IC
    IC *pc = loadBuildingBlockIC("level5_program_counter_4bit.panda");
    builder.add(pc);

    // Create output LEDs for pc[0-3]
    QVector<Led *> pc_outputs;
    for (int i = 0; i < 4; i++) {
        Led *p = new Led();
        builder.add(p);
        p->setLabel(QString("pc[%1]").arg(i));
        p->setPos(50 + i * 60, 300);
        pc_outputs.append(p);
    }

    // Create output LEDs for pc_plus_1[0-3]
    QVector<Led *> pc_plus_1_outputs;
    for (int i = 0; i < 4; i++) {
        Led *p1 = new Led();
        builder.add(p1);
        p1->setLabel(QString("pc_plus_1[%1]").arg(i));
        p1->setPos(50 + i * 60, 350);
        pc_plus_1_outputs.append(p1);
    }

    // Connect inputs to PC IC using semantic port labels
    for (int i = 0; i < 4; i++) {
        builder.connect(load_value_inputs[i], 0, pc, QString("loadValue[%1]").arg(i));
    }
    builder.connect(clock, 0, pc, "clock");
    builder.connect(load, 0, pc, "load");
    builder.connect(inc, 0, pc, "inc");
    builder.connect(reset, 0, pc, "reset");

    // Connect PC outputs to LEDs
    for (int i = 0; i < 4; i++) {
        builder.connect(pc, QString("pc[%1]").arg(i), pc_outputs[i], 0);
        builder.connect(pc, QString("pc_plus_1[%1]").arg(i), pc_plus_1_outputs[i], 0);
    }

    Simulation *sim = builder.initSimulation();
    sim->update();

    // Test 1: Load initial value
    setMultiBitInput(load_value_inputs, initialValue);

    load->setOn(true);
    inc->setOn(false);
    reset->setOn(false);

    sim->update();
    clockCycle(sim, clock);  // LOW→HIGH rising edge: captures load value
    sim->update();

    int pc_val = readMultiBitOutput(QVector<GraphicElement *>(pc_outputs.begin(), pc_outputs.end()), 0);
    int pc_plus_1_val = readMultiBitOutput(QVector<GraphicElement *>(pc_plus_1_outputs.begin(), pc_plus_1_outputs.end()), 0);

    QCOMPARE(pc_val, expectedAfterLoad);
    QCOMPARE(pc_plus_1_val, (expectedAfterLoad + 1) & 0x0F);  // 4-bit wraparound

    // Test 2: Increment the PC
    clock->setOn(false);
    load->setOn(false);
    inc->setOn(true);

    sim->update();
    clockCycle(sim, clock);  // LOW→HIGH rising edge: increments PC
    sim->update();

    pc_val = readMultiBitOutput(QVector<GraphicElement *>(pc_outputs.begin(), pc_outputs.end()), 0);
    pc_plus_1_val = readMultiBitOutput(QVector<GraphicElement *>(pc_plus_1_outputs.begin(), pc_plus_1_outputs.end()), 0);

    QCOMPARE(pc_val, expectedAfterIncrement);
    QCOMPARE(pc_plus_1_val, (expectedAfterIncrement + 1) & 0x0F);  // 4-bit wraparound
}

void TestLevel5ProgramCounter4Bit::testProgramCounter4BitStructure()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *pc = loadBuildingBlockIC("level5_program_counter_4bit.panda");

    builder.add(pc);

    // Verify IC has been loaded
    QVERIFY(pc != nullptr);

    // Create dummy connections to verify port access
    InputSwitch *data = new InputSwitch();
    builder.add(data);

    Led *addr = new Led();
    builder.add(addr);

    // Verify IC port structure
    // Inputs: 4 loadValue + load + inc + reset + clock = 8 inputs
    // Outputs: 4 pc + 4 pc_plus_1 = 8 outputs
    QCOMPARE(pc->inputSize(), 8);   // 4 loadValue + 4 control signals
    QCOMPARE(pc->outputSize(), 8);  // 4 pc + 4 pc_plus_1

    // Verify port connections work (don't throw)
    try {
        builder.connect(data, 0, pc, "loadValue[0]");      // Connect loadValue[0] input
        builder.connect(pc, "pc[0]", addr, 0);             // Connect pc[0] output
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("IC port access failed: %1").arg(e.what())));
    }
}
