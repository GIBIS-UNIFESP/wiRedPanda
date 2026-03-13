// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel8MemoryStage.h"

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

void TestLevel8MemoryStage::initTestCase()
{
}

void TestLevel8MemoryStage::cleanup()
{
}

void TestLevel8MemoryStage::testMemoryStage_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("dataIn");
    QTest::addColumn<int>("result");
    QTest::addColumn<bool>("memRead");
    QTest::addColumn<bool>("memWrite");
    QTest::addColumn<int>("expectedOut");

    // Pass-through tests: MemRead=0, MemWrite=0, return result directly
    QTest::newRow("passthrough 0x84 (no mem op)") << 0x00 << 0x00 << 0x84 << false << false << 0x84;
    QTest::newRow("passthrough 0x00") << 0xFF << 0x00 << 0x00 << false << false << 0x00;
    QTest::newRow("passthrough 0xAA") << 0x55 << 0xBB << 0xAA << false << false << 0xAA;

    // Write then immediate pass-through (MemWrite=1, output shows DataIn via pass-through initially)
    // Note: write is synchronous, so output during write shows pass-through result
    QTest::newRow("write 0x42 to addr 0x00") << 0x00 << 0x42 << 0xFF << false << true << 0xFF;
    QTest::newRow("write 0x55 to addr 0x10") << 0x10 << 0x55 << 0x77 << false << true << 0x77;

    // Read test (assuming data was previously in memory from initialization)
    QTest::newRow("read from addr 0x00 (empty)") << 0x00 << 0x00 << 0xFF << true << false << 0x00;
    QTest::newRow("read from addr 0x10 (empty)") << 0x10 << 0x00 << 0xFF << true << false << 0x00;
}

void TestLevel8MemoryStage::testMemoryStage()
{
    QFETCH(int, address);
    QFETCH(int, dataIn);
    QFETCH(int, result);
    QFETCH(bool, memRead);
    QFETCH(bool, memWrite);
    QFETCH(int, expectedOut);

    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create Address input switches (8-bit)
    QVector<InputSwitch *> address_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("Address[%1]").arg(i));
        sw->setPos(50 + i * 60, 100);
        address_inputs.append(sw);
    }

    // Create DataIn input switches (8-bit)
    QVector<InputSwitch *> datain_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("DataIn[%1]").arg(i));
        sw->setPos(50 + i * 60, 140);
        datain_inputs.append(sw);
    }

    // Create Result input switches (8-bit)
    QVector<InputSwitch *> result_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("Result[%1]").arg(i));
        sw->setPos(50 + i * 60, 180);
        result_inputs.append(sw);
    }

    // Create control signals
    InputSwitch *memread = new InputSwitch();
    builder.add(memread);
    memread->setLabel("MemRead");
    memread->setPos(650, 100);

    InputSwitch *memwrite = new InputSwitch();
    builder.add(memwrite);
    memwrite->setLabel("MemWrite");
    memwrite->setPos(710, 100);

    InputSwitch *clk = new InputSwitch();
    builder.add(clk);
    clk->setLabel("Clock");
    clk->setPos(770, 100);

    InputSwitch *reset = new InputSwitch();
    builder.add(reset);
    reset->setLabel("Reset");
    reset->setPos(830, 100);

    // Load the memory stage IC
    IC *mem_stage = loadBuildingBlockIC("level8_memory_stage.panda");
    builder.add(mem_stage);

    // Create output LEDs for DataOut (8-bit)
    QVector<Led *> dataout_leds;
    for (int i = 0; i < 8; i++) {
        Led *led = new Led();
        builder.add(led);
        led->setLabel(QString("DataOut[%1]").arg(i));
        led->setPos(50 + i * 60, 250);
        dataout_leds.append(led);
    }

    // Connect all inputs
    for (int i = 0; i < 8; i++) {
        builder.connect(address_inputs[i], 0, mem_stage, QString("Address[%1]").arg(i));
        builder.connect(datain_inputs[i], 0, mem_stage, QString("DataIn[%1]").arg(i));
        builder.connect(result_inputs[i], 0, mem_stage, QString("Result[%1]").arg(i));
    }

    builder.connect(memread, 0, mem_stage, "MemRead");
    builder.connect(memwrite, 0, mem_stage, "MemWrite");
    builder.connect(clk, 0, mem_stage, "Clock");
    builder.connect(reset, 0, mem_stage, "Reset");

    // Connect outputs
    for (int i = 0; i < 8; i++) {
        builder.connect(mem_stage, QString("DataOut[%1]").arg(i), dataout_leds[i], 0);
    }

    Simulation *sim = builder.initSimulation();
    sim->update();

    // Set input values
    setMultiBitInput(address_inputs, address);
    setMultiBitInput(datain_inputs, dataIn);
    setMultiBitInput(result_inputs, result);
    memread->setOn(memRead);
    memwrite->setOn(memWrite);
    sim->update();

    // For write operations, pulse the clock to latch data
    if (memWrite) {
        clockCycle(sim, clk);
    }

    // Read output
    int dataout = readMultiBitOutput(QVector<GraphicElement *>(dataout_leds.begin(), dataout_leds.end()), 0);

    // Verify
    QCOMPARE(dataout, expectedOut);
}

void TestLevel8MemoryStage::testMemoryStageStructure()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *mem_stage = loadBuildingBlockIC("level8_memory_stage.panda");
    builder.add(mem_stage);

    // Verify IC has been loaded
    QVERIFY(mem_stage != nullptr);

    // Create dummy elements for connection testing
    InputSwitch *sw = new InputSwitch();
    builder.add(sw);

    Led *out = new Led();
    builder.add(out);

    // Verify IC port structure
    // Inputs: 8 Address + 8 DataIn + 8 Result + MemRead + MemWrite + Clock + Reset = 28 inputs
    // Outputs: 8 DataOut = 8 outputs
    QCOMPARE(mem_stage->inputSize(), 28);  // 8+8+8+4 control signals (RAM has extra internal signals)
    QCOMPARE(mem_stage->outputSize(), 8);  // 8 data output bits

    // Verify port connections work (don't throw)
    try {
        builder.connect(sw, 0, mem_stage, "Address[0]");
        builder.connect(sw, 0, mem_stage, "Address[7]");
        builder.connect(sw, 0, mem_stage, "DataIn[0]");
        builder.connect(sw, 0, mem_stage, "DataIn[7]");
        builder.connect(sw, 0, mem_stage, "Result[0]");
        builder.connect(sw, 0, mem_stage, "Result[7]");
        builder.connect(sw, 0, mem_stage, "MemRead");
        builder.connect(sw, 0, mem_stage, "MemWrite");
        builder.connect(sw, 0, mem_stage, "Clock");
        builder.connect(sw, 0, mem_stage, "Reset");
        builder.connect(mem_stage, "DataOut[0]", out, 0);
        builder.connect(mem_stage, "DataOut[7]", out, 0);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Memory stage port access failed: %1").arg(e.what())));
    }
}
