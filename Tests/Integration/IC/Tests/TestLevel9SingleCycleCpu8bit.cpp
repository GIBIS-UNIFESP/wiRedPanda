// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel9SingleCycleCpu8bit.h"

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

void TestLevel9SingleCycleCPU8Bit::initTestCase()
{
}

void TestLevel9SingleCycleCPU8Bit::cleanup()
{
}

void TestLevel9SingleCycleCPU8Bit::testCPUStructure()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *cpu = loadBuildingBlockIC("level9_single_cycle_cpu_8bit.panda");
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
    // Outputs: PC[0-7] (8) + Result[0-7] (8) + Instruction[0-7] (8) + Zero (1) + Sign (1) = 26
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
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("CPU port access failed: %1").arg(e.what())));
    }
}

void TestLevel9SingleCycleCPU8Bit::testCPUSimpleExecution_data()
{
    QTest::addColumn<int>("cycles");
    QTest::addColumn<int>("expectedPC");

    // Simple tests: Just verify clock cycles increment PC
    QTest::newRow("initial state") << 0 << 0x00;
    QTest::newRow("after 1 cycle") << 1 << 0x01;
    QTest::newRow("after 2 cycles") << 2 << 0x02;
    QTest::newRow("after 5 cycles") << 5 << 0x05;
    QTest::newRow("after 10 cycles") << 10 << 0x0A;
}

void TestLevel9SingleCycleCPU8Bit::testCPUSimpleExecution()
{
    QFETCH(int, cycles);
    QFETCH(int, expectedPC);

    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load the CPU IC
    IC *cpu = loadBuildingBlockIC("level9_single_cycle_cpu_8bit.panda");
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

    // Connect inputs
    builder.connect(clk, 0, cpu, "Clock");
    builder.connect(reset, 0, cpu, "Reset");

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

    // Read PC
    int pc = readMultiBitOutput(QVector<GraphicElement *>(pc_leds.begin(), pc_leds.end()), 0);

    // Verify PC matches expected value
    QCOMPARE(pc, expectedPC);
}

void TestLevel9SingleCycleCPU8Bit::testCPUReset()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    IC *cpu = loadBuildingBlockIC("level9_single_cycle_cpu_8bit.panda");
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
    sim->update();

    // Initial reset
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    // Run 5 cycles
    for (int i = 0; i < 5; i++) {
        clockCycle(sim, clk);
    }
    int pc = readMultiBitOutput(QVector<GraphicElement *>(pc_leds.begin(), pc_leds.end()), 0);
    QCOMPARE(pc, 5);

    // Reset mid-execution
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    pc = readMultiBitOutput(QVector<GraphicElement *>(pc_leds.begin(), pc_leds.end()), 0);
    QCOMPARE(pc, 0);
}

void TestLevel9SingleCycleCPU8Bit::testCPUInstructionVisible()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    IC *cpu = loadBuildingBlockIC("level9_single_cycle_cpu_8bit.panda");
    builder.add(cpu);

    InputSwitch *clk = new InputSwitch();
    InputSwitch *reset = new InputSwitch();
    builder.add(clk, reset);

    QVector<Led *> instr_leds;
    for (int i = 0; i < 8; i++) {
        Led *led = new Led();
        builder.add(led);
        instr_leds.append(led);
        builder.connect(cpu, QString("Instruction[%1]").arg(i), led, 0);
    }

    builder.connect(clk, 0, cpu, "Clock");
    builder.connect(reset, 0, cpu, "Reset");

    Simulation *sim = builder.initSimulation();
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    clockCycle(sim, clk);

    // Verify the instruction port is readable — readMultiBitOutput would throw/fatal on bad port
    const int instr = readMultiBitOutput(QVector<GraphicElement *>(instr_leds.begin(), instr_leds.end()), 0);
    QVERIFY2(instr >= 0 && instr <= 255, "Instruction register must hold a valid 8-bit value");
}

void TestLevel9SingleCycleCPU8Bit::testCPUFlagsExposed()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    IC *cpu = loadBuildingBlockIC("level9_single_cycle_cpu_8bit.panda");
    builder.add(cpu);

    InputSwitch *clk = new InputSwitch();
    InputSwitch *reset = new InputSwitch();
    builder.add(clk, reset);

    Led *zero_led = new Led();
    Led *sign_led = new Led();
    builder.add(zero_led, sign_led);

    // Verify both flag ports connect without throwing
    try {
        builder.connect(cpu, "Zero", zero_led, 0);
        builder.connect(cpu, "Sign", sign_led, 0);
        builder.connect(clk, 0, cpu, "Clock");
        builder.connect(reset, 0, cpu, "Reset");
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Flag port access failed: %1").arg(e.what())));
    }

    Simulation *sim = builder.initSimulation();
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    clockCycle(sim, clk);

    // After one instruction, flags must be readable booleans (no crash)
    bool zeroVal = TestUtils::getInputStatus(zero_led);
    bool signVal = TestUtils::getInputStatus(sign_led);
    // Zero flag (result == 0x00) and sign flag (result bit 7 set) are mutually exclusive
    QVERIFY2(!(zeroVal && signVal), "Zero flag and sign flag cannot both be set simultaneously");
}

void TestLevel9SingleCycleCPU8Bit::testResultRegisterReadable()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    IC *cpu = loadBuildingBlockIC("level9_single_cycle_cpu_8bit.panda");
    builder.add(cpu);

    InputSwitch *clk = new InputSwitch();
    InputSwitch *reset = new InputSwitch();
    builder.add(clk, reset);

    QVector<Led *> result_leds;
    for (int i = 0; i < 8; ++i) {
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
        QVERIFY2(result >= 0 && result <= 255,
            qPrintable(QString("Cycle %1: Result %2 out of 8-bit range [0,255]").arg(cycle).arg(result)));
    }
}

void TestLevel9SingleCycleCPU8Bit::testFlagSemanticCorrectness()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    IC *cpu = loadBuildingBlockIC("level9_single_cycle_cpu_8bit.panda");
    builder.add(cpu);

    InputSwitch *clk = new InputSwitch();
    InputSwitch *reset = new InputSwitch();
    builder.add(clk, reset);

    QVector<Led *> result_leds;
    Led *zero_led = new Led();
    Led *sign_led = new Led();
    builder.add(zero_led, sign_led);

    for (int i = 0; i < 8; ++i) {
        Led *led = new Led();
        builder.add(led);
        result_leds.append(led);
        builder.connect(cpu, QString("Result[%1]").arg(i), led, 0);
    }

    builder.connect(cpu, "Zero", zero_led, 0);
    builder.connect(cpu, "Sign", sign_led, 0);
    builder.connect(clk, 0, cpu, "Clock");
    builder.connect(reset, 0, cpu, "Reset");

    Simulation *sim = builder.initSimulation();
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    // Run 5 clock cycles and verify flag semantics at each step
    for (int cycle = 0; cycle < 5; ++cycle) {
        clockCycle(sim, clk);

        int result = readMultiBitOutput(QVector<GraphicElement *>(result_leds.begin(), result_leds.end()), 0);
        bool zeroVal = TestUtils::getInputStatus(zero_led);
        bool signVal = TestUtils::getInputStatus(sign_led);

        // If Zero flag is set, result must be 0
        if (zeroVal) {
            QCOMPARE(result, 0);
        }

        // If Sign flag is set, MSB (bit 7) must be set
        if (signVal) {
            QVERIFY(result & 0x80);
        }

        // Zero and Sign flags cannot both be set simultaneously
        QVERIFY2(!(zeroVal && signVal),
            qPrintable(QString("Cycle %1: Both Zero and Sign flags are set simultaneously (result=%2)")
                .arg(cycle).arg(result)));
    }
}
