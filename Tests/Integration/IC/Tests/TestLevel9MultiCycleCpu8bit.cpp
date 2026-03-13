// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel9MultiCycleCpu8bit.h"

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

void TestLevel9MultiCycleCPU8Bit::initTestCase()
{
}

void TestLevel9MultiCycleCPU8Bit::cleanup()
{
}

void TestLevel9MultiCycleCPU8Bit::testCPUStructure()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *cpu = loadBuildingBlockIC("level9_multi_cycle_cpu_8bit.panda");
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
    // Outputs: PC[0-7] (8) + Result[0-7] (8) + CycleCounter[0-1] (2) + Instruction[0-7] (8) = 26
    QCOMPARE(cpu->inputSize(), 2);
    QCOMPARE(cpu->outputSize(), 26);

    // Verify port connections work (don't throw)
    try {
        builder.connect(sw, 0, cpu, "Clock");
        builder.connect(sw, 0, cpu, "Reset");
        builder.connect(cpu, "PC[0]", out, 0);
        builder.connect(cpu, "PC[7]", out, 0);
        builder.connect(cpu, "Result[0]", out, 0);
        builder.connect(cpu, "Result[7]", out, 0);
        builder.connect(cpu, "CycleCounter[0]", out, 0);
        builder.connect(cpu, "CycleCounter[1]", out, 0);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("CPU port access failed: %1").arg(e.what())));
    }
}

void TestLevel9MultiCycleCPU8Bit::testCycleCounter_data()
{
    QTest::addColumn<int>("cycles");
    QTest::addColumn<int>("expectedCounter");
    QTest::addColumn<int>("expectedPC");

    // Test cycle counter sequence: 00 -> 01 -> 10 -> 11 -> 00 (repeats)
    // With clock gating, PC only increments on cycle 0 (Fetch)
    QTest::newRow("initial (00, PC=0)") << 0 << 0x0 << 0x0;
    QTest::newRow("cycle 1 (01, PC=0)") << 1 << 0x1 << 0x0;  // Decode: PC stays same
    QTest::newRow("cycle 2 (10, PC=0)") << 2 << 0x2 << 0x0;  // Execute: PC stays same
    QTest::newRow("cycle 3 (11, PC=0)") << 3 << 0x3 << 0x0;  // Memory: PC stays same
    QTest::newRow("cycle 4 (00, PC=1)") << 4 << 0x0 << 0x1;  // Back to Fetch: PC increments
    QTest::newRow("cycle 5 (01, PC=1)") << 5 << 0x1 << 0x1;  // Decode: PC stays same
    QTest::newRow("after 8 cycles (00, PC=2)") << 8 << 0x0 << 0x2;  // 2 complete instructions
}

void TestLevel9MultiCycleCPU8Bit::testCycleCounter()
{
    QFETCH(int, cycles);
    QFETCH(int, expectedCounter);
    QFETCH(int, expectedPC);

    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load the CPU IC
    IC *cpu = loadBuildingBlockIC("level9_multi_cycle_cpu_8bit.panda");
    builder.add(cpu);

    // Create Clock input
    InputSwitch *clk = new InputSwitch();
    builder.add(clk);
    clk->setLabel("Clock");

    // Create Reset input
    InputSwitch *reset = new InputSwitch();
    builder.add(reset);
    reset->setLabel("Reset");

    // Create output LEDs for cycle counter
    QVector<Led *> counter_leds;
    for (int i = 0; i < 2; i++) {
        Led *led = new Led();
        builder.add(led);
        led->setLabel(QString("CycleCounter[%1]").arg(i));
        counter_leds.append(led);
    }

    // Create output LEDs for Program Counter
    QVector<Led *> pc_leds;
    for (int i = 0; i < 8; i++) {
        Led *led = new Led();
        builder.add(led);
        led->setLabel(QString("PC[%1]").arg(i));
        pc_leds.append(led);
    }

    // Connect inputs
    builder.connect(clk, 0, cpu, "Clock");
    builder.connect(reset, 0, cpu, "Reset");

    // Connect counter outputs
    for (int i = 0; i < 2; i++) {
        builder.connect(cpu, QString("CycleCounter[%1]").arg(i), counter_leds[i], 0);
    }

    // Connect PC outputs
    for (int i = 0; i < 8; i++) {
        builder.connect(cpu, QString("PC[%1]").arg(i), pc_leds[i], 0);
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

    // Read cycle counter
    int counter = readMultiBitOutput(QVector<GraphicElement *>(counter_leds.begin(), counter_leds.end()), 0);

    // Verify cycle counter matches expected value
    QCOMPARE(counter, expectedCounter);

    // Read Program Counter
    int pc = readMultiBitOutput(QVector<GraphicElement *>(pc_leds.begin(), pc_leds.end()), 0);

    // Verify PC matches expected value
    QCOMPARE(pc, expectedPC);
}

void TestLevel9MultiCycleCPU8Bit::testCPUReset()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    IC *cpu = loadBuildingBlockIC("level9_multi_cycle_cpu_8bit.panda");
    builder.add(cpu);

    InputSwitch *clk = new InputSwitch();
    InputSwitch *reset = new InputSwitch();
    builder.add(clk, reset);

    QVector<Led *> pc_leds;
    QVector<Led *> counter_leds;
    for (int i = 0; i < 8; i++) {
        Led *led = new Led();
        builder.add(led);
        pc_leds.append(led);
        builder.connect(cpu, QString("PC[%1]").arg(i), led, 0);
    }
    for (int i = 0; i < 2; i++) {
        Led *led = new Led();
        builder.add(led);
        counter_leds.append(led);
        builder.connect(cpu, QString("CycleCounter[%1]").arg(i), led, 0);
    }

    builder.connect(clk, 0, cpu, "Clock");
    builder.connect(reset, 0, cpu, "Reset");

    Simulation *sim = builder.initSimulation();
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    // Run 6 cycles (into 2nd instruction's pipeline)
    for (int i = 0; i < 6; i++) {
        clockCycle(sim, clk);
    }

    // Reset mid-execution
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    int pc = readMultiBitOutput(QVector<GraphicElement *>(pc_leds.begin(), pc_leds.end()), 0);
    int counter = readMultiBitOutput(QVector<GraphicElement *>(counter_leds.begin(), counter_leds.end()), 0);
    QCOMPARE(pc, 0);
    QCOMPARE(counter, 0);
}

void TestLevel9MultiCycleCPU8Bit::testPipelineStageSequence()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    IC *cpu = loadBuildingBlockIC("level9_multi_cycle_cpu_8bit.panda");
    builder.add(cpu);

    InputSwitch *clk = new InputSwitch();
    InputSwitch *reset = new InputSwitch();
    builder.add(clk, reset);

    QVector<Led *> counter_leds;
    for (int i = 0; i < 2; i++) {
        Led *led = new Led();
        builder.add(led);
        counter_leds.append(led);
        builder.connect(cpu, QString("CycleCounter[%1]").arg(i), led, 0);
    }

    builder.connect(clk, 0, cpu, "Clock");
    builder.connect(reset, 0, cpu, "Reset");

    Simulation *sim = builder.initSimulation();
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    // Verify CycleCounter cycles 0→1→2→3→0→... for 16 cycles
    for (int cycle = 0; cycle < 16; cycle++) {
        int counter = readMultiBitOutput(QVector<GraphicElement *>(counter_leds.begin(), counter_leds.end()), 0);
        QCOMPARE(counter, cycle % 4);
        clockCycle(sim, clk);
    }
}

void TestLevel9MultiCycleCPU8Bit::testInstructionVisibleDuringFetch()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    IC *cpu = loadBuildingBlockIC("level9_multi_cycle_cpu_8bit.panda");
    builder.add(cpu);

    InputSwitch *clk = new InputSwitch();
    InputSwitch *reset = new InputSwitch();
    builder.add(clk, reset);

    QVector<Led *> instr_leds;
    QVector<Led *> counter_leds;
    for (int i = 0; i < 8; i++) {
        Led *led = new Led();
        builder.add(led);
        instr_leds.append(led);
        builder.connect(cpu, QString("Instruction[%1]").arg(i), led, 0);
    }
    for (int i = 0; i < 2; i++) {
        Led *led = new Led();
        builder.add(led);
        counter_leds.append(led);
        builder.connect(cpu, QString("CycleCounter[%1]").arg(i), led, 0);
    }

    builder.connect(clk, 0, cpu, "Clock");
    builder.connect(reset, 0, cpu, "Reset");

    Simulation *sim = builder.initSimulation();
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    // Advance until we're in Fetch stage (counter == 0) after initial reset
    // First clock puts us in cycle 1, so advance 3 more to wrap to cycle 0
    for (int i = 0; i < 4; i++) {
        clockCycle(sim, clk);
    }

    int counter = readMultiBitOutput(QVector<GraphicElement *>(counter_leds.begin(), counter_leds.end()), 0);
    QCOMPARE(counter, 0); // Must be in Fetch stage

    // Instruction port must be readable without crashing
    int instr = readMultiBitOutput(QVector<GraphicElement *>(instr_leds.begin(), instr_leds.end()), 0);
    QVERIFY2(instr >= 0 && instr <= 255,
        qPrintable(QString("Instruction %1 out of 8-bit range [0,255]").arg(instr)));
}

void TestLevel9MultiCycleCPU8Bit::testResultRegisterReadable()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    IC *cpu = loadBuildingBlockIC("level9_multi_cycle_cpu_8bit.panda");
    builder.add(cpu);

    InputSwitch *clk = new InputSwitch();
    InputSwitch *reset = new InputSwitch();
    builder.add(clk, reset);

    QVector<Led *> result_leds;
    QVector<Led *> counter_leds;
    for (int i = 0; i < 8; ++i) {
        Led *led = new Led();
        builder.add(led);
        result_leds.append(led);
        builder.connect(cpu, QString("Result[%1]").arg(i), led, 0);
    }
    for (int i = 0; i < 2; ++i) {
        Led *led = new Led();
        builder.add(led);
        counter_leds.append(led);
        builder.connect(cpu, QString("CycleCounter[%1]").arg(i), led, 0);
    }

    builder.connect(clk, 0, cpu, "Clock");
    builder.connect(reset, 0, cpu, "Reset");

    Simulation *sim = builder.initSimulation();
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    // Run 2 complete instruction cycles (8 clock cycles total)
    // Each instruction takes 4 cycles (Fetch, Decode, Execute, Memory)
    for (int cycle = 0; cycle < 8; ++cycle) {
        clockCycle(sim, clk);

        // After each complete 4-cycle instruction, read result
        int counter = readMultiBitOutput(QVector<GraphicElement *>(counter_leds.begin(), counter_leds.end()), 0);
        if (counter == 0) {  // Back to Fetch stage means instruction complete
            int result = readMultiBitOutput(QVector<GraphicElement *>(result_leds.begin(), result_leds.end()), 0);
            QVERIFY2(result >= 0 && result <= 255,
                qPrintable(QString("Cycle %1: Result %2 out of 8-bit range [0,255]").arg(cycle).arg(result)));
        }
    }
}

void TestLevel9MultiCycleCPU8Bit::testInstructionStabilityAcrossPipelineStages()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    IC *cpu = loadBuildingBlockIC("level9_multi_cycle_cpu_8bit.panda");
    builder.add(cpu);

    InputSwitch *clk = new InputSwitch();
    InputSwitch *reset = new InputSwitch();
    builder.add(clk, reset);

    QVector<Led *> instr_leds;
    QVector<Led *> counter_leds;
    for (int i = 0; i < 8; ++i) {
        Led *led = new Led();
        builder.add(led);
        instr_leds.append(led);
        builder.connect(cpu, QString("Instruction[%1]").arg(i), led, 0);
    }
    for (int i = 0; i < 2; ++i) {
        Led *led = new Led();
        builder.add(led);
        counter_leds.append(led);
        builder.connect(cpu, QString("CycleCounter[%1]").arg(i), led, 0);
    }

    builder.connect(clk, 0, cpu, "Clock");
    builder.connect(reset, 0, cpu, "Reset");

    Simulation *sim = builder.initSimulation();
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    // After reset, counter should be 0 (Fetch stage)
    int counter = readMultiBitOutput(QVector<GraphicElement *>(counter_leds.begin(), counter_leds.end()), 0);
    QCOMPARE(counter, 0);

    // Read instruction in Fetch stage
    int instrFetch = readMultiBitOutput(QVector<GraphicElement *>(instr_leds.begin(), instr_leds.end()), 0);

    // Advance through 3 more pipeline stages (Decode, Execute, Memory) within same instruction
    for (int stage = 1; stage < 4; ++stage) {
        clockCycle(sim, clk);

        // Verify counter advances to next stage
        counter = readMultiBitOutput(QVector<GraphicElement *>(counter_leds.begin(), counter_leds.end()), 0);
        QCOMPARE(counter, stage);

        // Read instruction at this stage and verify it's unchanged
        int instrStage = readMultiBitOutput(QVector<GraphicElement *>(instr_leds.begin(), instr_leds.end()), 0);
        QCOMPARE(instrStage, instrFetch);
    }
}
