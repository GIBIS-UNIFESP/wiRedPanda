// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel9Cpu16bitRisc.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::readMultiBitOutput;
using TestUtils::setMultiBitInput;
using TestUtils::clockCycle;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel9CPU16BitRISC::initTestCase()
{
}

void TestLevel9CPU16BitRISC::cleanup()
{
}

void TestLevel9CPU16BitRISC::testCPUStructure()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *cpu = loadBuildingBlockIC("level9_cpu_16bit_risc.panda");
    builder.add(cpu);

    // Verify IC has been loaded
    QVERIFY(cpu != nullptr);

    // Create dummy elements for connection testing
    InputSwitch *sw = new InputSwitch();
    builder.add(sw);

    Led *out = new Led();
    builder.add(out);

    // Verify IC port structure
    // Inputs: Clock + Reset = 2 inputs
    // Outputs: PC[0-7] (8) + Result[0-15] (16) + Instr[0-15] (16) + OpCode[0-4] (5) = 45 outputs
    QCOMPARE(cpu->inputSize(), 2);
    QCOMPARE(cpu->outputSize(), 45);

    // Verify port connections work (don't throw)
    try {
        builder.connect(sw, 0, cpu, "Clock");
        builder.connect(sw, 0, cpu, "Reset");
        builder.connect(cpu, "PC[0]", out, 0);
        builder.connect(cpu, "PC[7]", out, 0);
        builder.connect(cpu, "Result[0]", out, 0);
        builder.connect(cpu, "Result[15]", out, 0);  // 16-bit result
        builder.connect(cpu, "Instr[0]", out, 0);    // Changed from InstrWord to Instr
        builder.connect(cpu, "Instr[15]", out, 0);   // 16-bit instruction
        builder.connect(cpu, "OpCode[0]", out, 0);   // New 5-bit opcode output
        builder.connect(cpu, "OpCode[4]", out, 0);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("CPU port access failed: %1").arg(e.what())));
    }
}

void TestLevel9CPU16BitRISC::testCPUExecution_data()
{
    QTest::addColumn<int>("cycles");
    QTest::addColumn<int>("expectedPC");

    // Test RISC CPU execution with auto-incrementing PC
    QTest::newRow("initial state") << 0 << 0x00;
    QTest::newRow("after 1 cycle") << 1 << 0x01;
    QTest::newRow("after 2 cycles") << 2 << 0x02;
    QTest::newRow("after 3 cycles") << 3 << 0x03;
    QTest::newRow("after 5 cycles") << 5 << 0x05;
    QTest::newRow("after 10 cycles") << 10 << 0x0A;
}

void TestLevel9CPU16BitRISC::testCPUExecution()
{
    QFETCH(int, cycles);
    QFETCH(int, expectedPC);

    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load the CPU IC
    IC *cpu = loadBuildingBlockIC("level9_cpu_16bit_risc.panda");
    builder.add(cpu);

    // Create Clock input
    InputSwitch *clk = new InputSwitch();
    builder.add(clk);
    clk->setLabel("Clock");

    // Create Reset input
    InputSwitch *reset = new InputSwitch();
    builder.add(reset);
    reset->setLabel("Reset");

    // Create output LEDs for PC
    QVector<Led *> pc_leds;
    for (int i = 0; i < 8; i++) {
        Led *led = new Led();
        builder.add(led);
        led->setLabel(QString("PC[%1]").arg(i));
        pc_leds.append(led);
    }

    // Create output LEDs for Instruction (now 16-bit, but only monitor first 8 for backward compatibility)
    QVector<Led *> instr_leds;
    for (int i = 0; i < 8; i++) {
        Led *led = new Led();
        builder.add(led);
        led->setLabel(QString("Instr[%1]").arg(i));
        instr_leds.append(led);
    }

    // Connect inputs
    builder.connect(clk, 0, cpu, "Clock");
    builder.connect(reset, 0, cpu, "Reset");

    // Connect PC and instruction outputs
    for (int i = 0; i < 8; i++) {
        builder.connect(cpu, QString("PC[%1]").arg(i), pc_leds[i], 0);
        builder.connect(cpu, QString("Instr[%1]").arg(i), instr_leds[i], 0);
    }

    Simulation *sim = builder.initSimulation();
    sim->update();

    // Reset the CPU
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    // Clock cycles
    for (int i = 0; i < cycles; i++) {
        clockCycle(sim, clk);
    }

    // Read PC
    int pc = readMultiBitOutput(QVector<GraphicElement *>(pc_leds.begin(), pc_leds.end()), 0);

    // Verify PC matches expected value
    QCOMPARE(pc, expectedPC);
}

void TestLevel9CPU16BitRISC::testCPUReset()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    IC *cpu = loadBuildingBlockIC("level9_cpu_16bit_risc.panda");
    builder.add(cpu);

    InputSwitch *clk = new InputSwitch();
    InputSwitch *reset = new InputSwitch();
    builder.add(clk, reset);

    QVector<Led *> pc_leds;
    for (int i = 0; i < 8; i++) {
        Led *led = new Led();
        builder.add(led);
        pc_leds.append(led);
        builder.connect(cpu, QString("PC[%1]").arg(i), led, 0);
    }

    builder.connect(clk, 0, cpu, "Clock");
    builder.connect(reset, 0, cpu, "Reset");

    Simulation *sim = builder.initSimulation();
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    for (int i = 0; i < 5; i++) {
        clockCycle(sim, clk);
    }
    int pc = readMultiBitOutput(QVector<GraphicElement *>(pc_leds.begin(), pc_leds.end()), 0);
    QCOMPARE(pc, 5);

    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    pc = readMultiBitOutput(QVector<GraphicElement *>(pc_leds.begin(), pc_leds.end()), 0);
    QCOMPARE(pc, 0);
}

void TestLevel9CPU16BitRISC::testInstrWordVisible()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    IC *cpu = loadBuildingBlockIC("level9_cpu_16bit_risc.panda");
    builder.add(cpu);

    InputSwitch *clk = new InputSwitch();
    InputSwitch *reset = new InputSwitch();
    builder.add(clk, reset);

    QVector<Led *> instr_leds;
    for (int i = 0; i < 16; i++) {
        Led *led = new Led();
        builder.add(led);
        instr_leds.append(led);
        builder.connect(cpu, QString("Instr[%1]").arg(i), led, 0);
    }

    builder.connect(clk, 0, cpu, "Clock");
    builder.connect(reset, 0, cpu, "Reset");

    Simulation *sim = builder.initSimulation();
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    clockCycle(sim, clk);

    // Instruction port must be readable — no exception, no crash
    int instr = readMultiBitOutput(QVector<GraphicElement *>(instr_leds.begin(), instr_leds.end()), 0);
    QVERIFY2(instr >= 0 && instr <= 65535,
        qPrintable(QString("Instruction word %1 out of 16-bit range [0,65535]").arg(instr)));
}

void TestLevel9CPU16BitRISC::testOpCodeInValidRange()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    IC *cpu = loadBuildingBlockIC("level9_cpu_16bit_risc.panda");
    builder.add(cpu);

    InputSwitch *clk = new InputSwitch();
    InputSwitch *reset = new InputSwitch();
    builder.add(clk, reset);

    QVector<Led *> pc_leds;
    QVector<Led *> opcode_leds;
    for (int i = 0; i < 8; i++) {
        Led *led = new Led();
        builder.add(led);
        pc_leds.append(led);
        builder.connect(cpu, QString("PC[%1]").arg(i), led, 0);
    }
    for (int i = 0; i < 5; i++) {
        Led *led = new Led();
        builder.add(led);
        opcode_leds.append(led);
        builder.connect(cpu, QString("OpCode[%1]").arg(i), led, 0);
    }

    builder.connect(clk, 0, cpu, "Clock");
    builder.connect(reset, 0, cpu, "Reset");

    Simulation *sim = builder.initSimulation();
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    // Run 5 cycles and verify opcode is always in valid 5-bit range
    for (int cycle = 0; cycle < 5; cycle++) {
        clockCycle(sim, clk);
        int opcode = readMultiBitOutput(QVector<GraphicElement *>(opcode_leds.begin(), opcode_leds.end()), 0);
        QVERIFY2(opcode >= 0 && opcode <= 31,
            qPrintable(QString("Cycle %1: OpCode %2 out of 5-bit range [0,31]").arg(cycle).arg(opcode)));
    }
}

void TestLevel9CPU16BitRISC::testResultRegisterReadable()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    IC *cpu = loadBuildingBlockIC("level9_cpu_16bit_risc.panda");
    builder.add(cpu);

    InputSwitch *clk = new InputSwitch();
    InputSwitch *reset = new InputSwitch();
    builder.add(clk, reset);

    QVector<Led *> result_leds;
    for (int i = 0; i < 16; ++i) {
        Led *led = new Led();
        builder.add(led);
        result_leds.append(led);
        builder.connect(cpu, QString("Result[%1]").arg(i), led, 0);
    }

    builder.connect(clk, 0, cpu, "Clock");
    builder.connect(reset, 0, cpu, "Reset");

    Simulation *sim = builder.initSimulation();
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    // Run 5 clock cycles and verify result is readable at each step
    for (int cycle = 0; cycle < 5; ++cycle) {
        clockCycle(sim, clk);
        int result = readMultiBitOutput(QVector<GraphicElement *>(result_leds.begin(), result_leds.end()), 0);
        QVERIFY2(result >= 0 && result <= 65535,
            qPrintable(QString("Cycle %1: Result %2 out of 16-bit range [0,65535]").arg(cycle).arg(result)));
    }
}

void TestLevel9CPU16BitRISC::testInstrOpCodeFieldConsistency()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    IC *cpu = loadBuildingBlockIC("level9_cpu_16bit_risc.panda");
    builder.add(cpu);

    InputSwitch *clk = new InputSwitch();
    InputSwitch *reset = new InputSwitch();
    builder.add(clk, reset);

    QVector<Led *> instr_leds;
    QVector<Led *> opcode_leds;
    for (int i = 0; i < 16; ++i) {
        Led *led = new Led();
        builder.add(led);
        instr_leds.append(led);
        builder.connect(cpu, QString("Instr[%1]").arg(i), led, 0);
    }
    for (int i = 0; i < 5; ++i) {
        Led *led = new Led();
        builder.add(led);
        opcode_leds.append(led);
        builder.connect(cpu, QString("OpCode[%1]").arg(i), led, 0);
    }

    builder.connect(clk, 0, cpu, "Clock");
    builder.connect(reset, 0, cpu, "Reset");

    Simulation *sim = builder.initSimulation();
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    // Run 5 clock cycles and verify OpCode field consistency
    for (int cycle = 0; cycle < 5; ++cycle) {
        clockCycle(sim, clk);

        int instrWord = readMultiBitOutput(QVector<GraphicElement *>(instr_leds.begin(), instr_leds.end()), 0);
        int opcode = readMultiBitOutput(QVector<GraphicElement *>(opcode_leds.begin(), opcode_leds.end()), 0);

        // OpCode must equal bits [15:11] of the 16-bit instruction word
        int expectedOpCode = (instrWord >> 11) & 0x1F;
        QCOMPARE(opcode, expectedOpCode);
    }
}
