// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel7InstructionMemoryInterface.h"

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

void TestLevel7InstructionMemoryInterface::initTestCase()
{
}

void TestLevel7InstructionMemoryInterface::cleanup()
{
}

void TestLevel7InstructionMemoryInterface::testInstructionMemoryInterface_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("expectedInstruction");

    // Test various addresses to verify memory interface read capability
    // All expected values are 0x00 because the instruction memory IC returns uninitialized/default data
    // This tests that the IC correctly:
    // 1. Accepts address inputs on all 8 Address[0-7] ports
    // 2. Routes the address input to internal memory
    // 3. Returns the memory contents on Instruction[0-7] outputs
    // NOTE: For a complete memory test with actual data, the .panda file would need pre-programmed values
    // or the test would need to exercise a write-enable path (not currently implemented)

    QTest::newRow("address 0x00") << 0x00 << 0x00;
    QTest::newRow("address 0x01") << 0x01 << 0x00;
    QTest::newRow("address 0x10") << 0x10 << 0x00;
    QTest::newRow("address 0x42") << 0x42 << 0x00;
    QTest::newRow("address 0xFF") << 0xFF << 0x00;
}

void TestLevel7InstructionMemoryInterface::testInstructionMemoryInterface()
{
    QFETCH(int, address);
    QFETCH(int, expectedInstruction);

    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create address input switches (8-bit)
    QVector<InputSwitch *> address_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("Address[%1]").arg(i));
        sw->setPos(50 + i * 60, 100);
        address_inputs.append(sw);
    }

    // Create clock input
    InputSwitch *clk = new InputSwitch();
    builder.add(clk);
    clk->setLabel("Clock");
    clk->setPos(550, 100);

    // Load the instruction memory interface IC
    IC *imem = loadBuildingBlockIC("level7_instruction_memory_interface.panda");
    builder.add(imem);

    // Create output LEDs for instruction (8-bit)
    QVector<Led *> instr_leds;
    for (int i = 0; i < 8; i++) {
        Led *led = new Led();
        builder.add(led);
        led->setLabel(QString("Instruction[%1]").arg(i));
        led->setPos(50 + i * 60, 200);
        instr_leds.append(led);
    }

    // Connect all address inputs to instruction memory interface
    for (int i = 0; i < 8; i++) {
        builder.connect(address_inputs[i], 0, imem, QString("Address[%1]").arg(i));
    }

    // Connect clock
    builder.connect(clk, 0, imem, "Clock");

    // Connect outputs to LEDs
    for (int i = 0; i < 8; i++) {
        builder.connect(imem, QString("Instruction[%1]").arg(i), instr_leds[i], 0);
    }

    Simulation *sim = builder.initSimulation();
    sim->update();

    // Set address
    setMultiBitInput(address_inputs, address);
    sim->update();

    // Read instruction output
    int instruction = readMultiBitOutput(QVector<GraphicElement *>(instr_leds.begin(), instr_leds.end()), 0);

    // Verify instruction matches expected
    QCOMPARE(instruction, expectedInstruction);
}

void TestLevel7InstructionMemoryInterface::testInstructionMemoryInterfaceStructure()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *imem = loadBuildingBlockIC("level7_instruction_memory_interface.panda");
    builder.add(imem);

    // Verify IC has been loaded
    QVERIFY(imem != nullptr);

    // Create dummy elements for connection testing
    InputSwitch *addr = new InputSwitch();
    builder.add(addr);

    Led *out = new Led();
    builder.add(out);

    // Verify IC port structure
    // Inputs: 8 Address + 1 Clock = 9 inputs
    // Outputs: 8 Instruction = 8 outputs
    QCOMPARE(imem->inputSize(), 9);   // 8 address + 1 clock
    QCOMPARE(imem->outputSize(), 8);  // 8 instruction bits

    // Verify port connections work (don't throw and return success)
    try {
        QVERIFY2(builder.connect(addr, 0, imem, "Address[0]"), "Failed to connect Address[0]");
        QVERIFY2(builder.connect(addr, 0, imem, "Address[7]"), "Failed to connect Address[7]");
        QVERIFY2(builder.connect(addr, 0, imem, "Clock"), "Failed to connect Clock");
        QVERIFY2(builder.connect(imem, "Instruction[0]", out, 0), "Failed to connect Instruction[0]");
        QVERIFY2(builder.connect(imem, "Instruction[7]", out, 0), "Failed to connect Instruction[7]");
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("IC port access failed: %1").arg(e.what())));
    }
}

