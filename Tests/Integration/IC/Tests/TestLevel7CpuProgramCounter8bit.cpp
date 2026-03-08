// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel7CpuProgramCounter8bit.h"

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

void TestLevel7CPUProgramCounter8Bit::testProgramCounter8Bit() {
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

    InputSwitch *write_enable = new InputSwitch();
    builder.add(write_enable);
    write_enable->setLabel("WriteEnable");
    write_enable->setPos(600, 200);

    // Load and add PC IC
    IC *pc = loadBuildingBlockIC("level7_cpu_program_counter_8bit.panda");
    builder.add(pc);

    // Create output LEDs
    QVector<Led *> address_outputs;
    for (int i = 0; i < 8; i++) {
        Led *addr = new Led();
        builder.add(addr);
        addr->setLabel(QString("Address[%1]").arg(i));
        addr->setPos(50 + i * 60, 500);
        address_outputs.append(addr);
    }

    // Connect inputs to PC IC using semantic port labels
    for (int i = 0; i < 8; i++) {
        builder.connect(data_inputs[i], 0, pc, QString("Data[%1]").arg(i));
    }
    builder.connect(clock, 0, pc, "Clock");
    builder.connect(load, 0, pc, "Load");
    builder.connect(inc, 0, pc, "Inc");
    builder.connect(reset, 0, pc, "Reset");
    builder.connect(write_enable, 0, pc, "WriteEnable");

    // Connect PC outputs to Address LEDs using semantic port labels
    for (int i = 0; i < 8; i++) {
        builder.connect(pc, QString("Address[%1]").arg(i), address_outputs[i], 0);
    }

    Simulation *sim = builder.initSimulation();
    sim->update();

    // Test 1: Load address 0x42
    setMultiBitInput(data_inputs, 0x42);
    load->setOn(true);
    write_enable->setOn(true);
    reset->setOn(false);

    sim->update();
    clockCycle(sim, clock);  // LOW->HIGH rising edge: captures 0x42
    sim->update();

    int addr = readMultiBitOutput(QVector<GraphicElement *>(address_outputs.begin(), address_outputs.end()), 0);
    QCOMPARE(addr, 0x42);

    // Test 2: Load another address 0x20
    // Reset clock to LOW for consistent test conditions
    clock->setOn(false);
    sim->update();

    setMultiBitInput(data_inputs, 0x20);
    sim->update();
    clockCycle(sim, clock);  // LOW->HIGH rising edge: captures 0x20
    sim->update();

    addr = readMultiBitOutput(QVector<GraphicElement *>(address_outputs.begin(), address_outputs.end()), 0);
    QCOMPARE(addr, 0x20);

    // Test 3: Reset to 0x00
    clock->setOn(false);
    reset->setOn(true);
    sim->update();
    addr = readMultiBitOutput(QVector<GraphicElement *>(address_outputs.begin(), address_outputs.end()), 0);
    QCOMPARE(addr, CPUTestUtils::PATTERN_ALL_ZEROS);
}

void TestLevel7CPUProgramCounter8Bit::testProgramCounter8BitStructure() {
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *pc = loadBuildingBlockIC("level7_cpu_program_counter_8bit.panda");

    builder.add(pc);

    // Verify IC has been loaded
    QVERIFY(pc != nullptr);

    // Create dummy connections to verify port access
    InputSwitch *data = new InputSwitch();
    builder.add(data);

    Led *addr = new Led();
    builder.add(addr);

    // Verify IC port structure
    QCOMPARE(pc->inputSize(), 13);  // 8 Data + Clock + Load + Inc + Reset + WriteEnable
    QCOMPARE(pc->outputSize(), 8);  // 8 Address bits

    // Verify port connections work (don't throw)
    try {
        builder.connect(data, 0, pc, "Data[0]");         // Connect Data[0] input
        builder.connect(pc, "Address[0]", addr, 0);    // Connect Address[0] output
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("IC port access failed: %1").arg(e.what())));
    }
}
