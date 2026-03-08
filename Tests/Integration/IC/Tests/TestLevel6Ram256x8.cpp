// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel6Ram256x8.h"

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
using TestUtils::clockCycle;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel6RAM256X8::initTestCase()
{
}

void TestLevel6RAM256X8::cleanup()
{
}

void TestLevel6RAM256X8::testRAM256x8_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("writeData");
    QTest::addColumn<int>("readAddress");
    QTest::addColumn<int>("expectedReadData");

    // Test cases: write to address, then read from address
    QTest::newRow("write 0x42 to addr 0, read addr 0")
        << 0 << 0x42 << 0 << 0x42;
    QTest::newRow("write 0xFF to addr 3, read addr 3")
        << 3 << 0xFF << 3 << 0xFF;
    QTest::newRow("write 0xAA to addr 5, read addr 5")
        << 5 << 0xAA << 5 << 0xAA;
    QTest::newRow("write 0x55 to addr 7, read addr 7")
        << 7 << 0x55 << 7 << 0x55;
}

void TestLevel6RAM256X8::testRAM256x8()
{
    QFETCH(int, address);
    QFETCH(int, writeData);
    QFETCH(int, readAddress);
    QFETCH(int, expectedReadData);

    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create 8-bit Address input
    QVector<InputSwitch *> address_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("Address[%1]").arg(i));
        sw->setPos(50 + i * 60, 100);
        address_inputs.append(sw);
    }

    // Create 8-bit DataIn input
    QVector<InputSwitch *> data_in_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("DataIn[%1]").arg(i));
        sw->setPos(50 + i * 60, 140);
        data_in_inputs.append(sw);
    }

    // Create control signals
    InputSwitch *we = new InputSwitch();
    builder.add(we);
    we->setLabel("WriteEnable");
    we->setPos(600, 140);

    InputSwitch *clk = new InputSwitch();
    builder.add(clk);
    clk->setLabel("Clock");
    clk->setPos(650, 140);

    // Load the RAM IC
    IC *ram = loadBuildingBlockIC("level6_ram_256x8.panda");
    builder.add(ram);

    // Create output LEDs for DataOut
    QVector<Led *> data_out_outputs;
    for (int i = 0; i < 8; i++) {
        Led *led = new Led();
        builder.add(led);
        led->setLabel(QString("DataOut[%1]").arg(i));
        led->setPos(50 + i * 60, 200);
        data_out_outputs.append(led);
    }

    // Connect address inputs to RAM
    for (int i = 0; i < 8; i++) {
        builder.connect(address_inputs[i], 0, ram, QString("Address[%1]").arg(i));
    }

    // Connect data inputs to RAM
    for (int i = 0; i < 8; i++) {
        builder.connect(data_in_inputs[i], 0, ram, QString("DataIn[%1]").arg(i));
    }

    // Connect control signals to RAM
    builder.connect(we, 0, ram, "WriteEnable");
    builder.connect(clk, 0, ram, "Clock");

    // Connect RAM outputs to LEDs
    for (int i = 0; i < 8; i++) {
        builder.connect(ram, QString("DataOut[%1]").arg(i), data_out_outputs[i], 0);
    }

    Simulation *sim = builder.initSimulation();
    sim->update();

    // Step 1: Write data to address
    setMultiBitInput(address_inputs, address);
    setMultiBitInput(data_in_inputs, writeData);
    we->setOn(true);   // Enable write
    sim->update();

    // Clock pulse to perform write
    clk->setOn(false);
    sim->update();
    clockCycle(sim, clk);
    sim->update();

    // Step 2: Read from address
    setMultiBitInput(address_inputs, readAddress);
    we->setOn(false);  // Disable write (enable read)
    sim->update();

    // Read output should now show the data from selected address
    int readData = readMultiBitOutput(QVector<GraphicElement *>(data_out_outputs.begin(), data_out_outputs.end()), 0);

    // Verify read data matches expected value
    QCOMPARE(readData, expectedReadData);
}

void TestLevel6RAM256X8::testRAMStructure()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *ram = loadBuildingBlockIC("level6_ram_256x8.panda");
    builder.add(ram);

    // Verify IC has been loaded
    QVERIFY(ram != nullptr);

    // Create dummy elements for connection testing
    InputSwitch *data = new InputSwitch();
    builder.add(data);

    Led *out = new Led();
    builder.add(out);

    // Verify IC port structure
    // Inputs: 8 Address + 8 DataIn + WriteEnable + Clock = 18 inputs
    // Outputs: 8 DataOut = 8 outputs
    QCOMPARE(ram->inputSize(), 18);  // 8 Address + 8 DataIn + WriteEnable + Clock
    QCOMPARE(ram->outputSize(), 8);  // 8 DataOut

    // Verify port connections work (don't throw)
    try {
        builder.connect(data, 0, ram, "Address[0]");       // Connect Address[0]
        builder.connect(data, 0, ram, "DataIn[0]");        // Connect DataIn[0]
        builder.connect(data, 0, ram, "WriteEnable");      // Connect WriteEnable
        builder.connect(data, 0, ram, "Clock");            // Connect Clock
        builder.connect(ram, "DataOut[0]", out, 0);        // Connect DataOut[0]
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("IC port access failed: %1").arg(e.what())));
    }
}

