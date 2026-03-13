// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel7FlagRegister.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::setMultiBitInput;
using TestUtils::readMultiBitOutput;
using TestUtils::clockCycle;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel7FlagRegister::initTestCase()
{
}

void TestLevel7FlagRegister::cleanup()
{
}

void TestLevel7FlagRegister::testFlagRegister_data()
{
    QTest::addColumn<int>("flagInput");
    QTest::addColumn<int>("expectedFlagOutput");

    // Test loading various flag combinations
    // Flags: [3]=Overflow, [2]=Carry, [1]=Sign, [0]=Zero
    QTest::newRow("load 0000 (no flags set)")  << 0x0 << 0x0;
    QTest::newRow("load 0001 (Zero flag)")     << 0x1 << 0x1;
    QTest::newRow("load 0010 (Sign flag)")     << 0x2 << 0x2;
    QTest::newRow("load 0100 (Carry flag)")    << 0x4 << 0x4;
    QTest::newRow("load 1000 (Overflow flag)") << 0x8 << 0x8;
    QTest::newRow("load 1111 (all flags)")     << 0xF << 0xF;
    QTest::newRow("load 0101 (Z and C)")       << 0x5 << 0x5;
    QTest::newRow("load 1010 (O and S)")       << 0xA << 0xA;
}

void TestLevel7FlagRegister::testFlagRegister()
{
    QFETCH(int, flagInput);
    QFETCH(int, expectedFlagOutput);

    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create 4-bit flag input
    QVector<InputSwitch *> flag_inputs;
    for (int i = 0; i < 4; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("FlagIn[%1]").arg(i));
        sw->setPos(50 + i * 60, 100);
        flag_inputs.append(sw);
    }

    // Create control signals
    InputSwitch *load = new InputSwitch();
    builder.add(load);
    load->setLabel("Load");
    load->setPos(350, 100);

    InputSwitch *clk = new InputSwitch();
    builder.add(clk);
    clk->setLabel("Clock");
    clk->setPos(400, 100);

    // Load the flag register IC
    IC *flag_reg = loadBuildingBlockIC("level7_flag_register.panda");
    builder.add(flag_reg);

    // Create output LEDs for flag outputs
    QVector<Led *> flag_outputs;
    for (int i = 0; i < 4; i++) {
        Led *led = new Led();
        builder.add(led);
        led->setLabel(QString("FlagOut[%1]").arg(i));
        led->setPos(50 + i * 60, 200);
        flag_outputs.append(led);
    }

    // Connect flag inputs to register
    const QStringList flag_names = {"Zero", "Sign", "Carry", "Overflow"};
    for (int i = 0; i < 4; i++) {
        builder.connect(flag_inputs[i], 0, flag_reg, QString("FlagIn[%1]_%2").arg(i).arg(flag_names[i]));
    }

    // Connect control signals
    builder.connect(load, 0, flag_reg, "Load");
    builder.connect(clk, 0, flag_reg, "Clock");

    // Connect outputs to LEDs
    for (int i = 0; i < 4; i++) {
        builder.connect(flag_reg, QString("FlagOut[%1]_%2").arg(i).arg(flag_names[i]),
                       flag_outputs[i], 0);
    }

    Simulation *sim = builder.initSimulation();
    sim->update();

    // Set flag inputs
    setMultiBitInput(flag_inputs, flagInput);
    load->setOn(true);  // Enable load
    sim->update();

    // Clock pulse to load flags
    clockCycle(sim, clk);
    sim->update();

    // Read flag outputs after loading
    int flagOutput = readMultiBitOutput(QVector<GraphicElement *>(flag_outputs.begin(), flag_outputs.end()), 0);

    // Verify flags match expected after loading
    QCOMPARE(flagOutput, expectedFlagOutput);

    // Test hold path: disable load and verify flags don't change
    load->setOn(false);
    sim->update();

    // Clock multiple times while load=0 to verify flags are held
    for (int i = 0; i < 3; i++) {
        // Change input values to different pattern
        setMultiBitInput(flag_inputs, (flagInput ^ 0xF));  // Flip all bits
        sim->update();

        // Clock pulse while load is disabled
        clockCycle(sim, clk);
        sim->update();

        // Verify flags have NOT changed despite input change and clock
        int heldFlagOutput = readMultiBitOutput(QVector<GraphicElement *>(flag_outputs.begin(), flag_outputs.end()), 0);
        QVERIFY2(heldFlagOutput == expectedFlagOutput,
                 qPrintable(QString("Hold path failed at iteration %1: flags changed when load=0 (got 0x%2, expected 0x%3)")
                     .arg(i + 1)
                     .arg(heldFlagOutput, 2, 16, QChar('0'))
                     .arg(expectedFlagOutput, 2, 16, QChar('0'))));
    }
}

void TestLevel7FlagRegister::testFlagRegisterStructure()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *flag_reg = loadBuildingBlockIC("level7_flag_register.panda");
    builder.add(flag_reg);

    // Verify IC has been loaded
    QVERIFY(flag_reg != nullptr);

    // Create dummy elements for connection testing
    InputSwitch *data = new InputSwitch();
    builder.add(data);

    Led *out = new Led();
    builder.add(out);

    // Verify IC port structure
    // Inputs: 4 FlagIn + Load + Clock = 6 inputs
    // Outputs: 4 FlagOut = 4 outputs
    QCOMPARE(flag_reg->inputSize(), 6);   // 4 flag inputs + Load + Clock
    QCOMPARE(flag_reg->outputSize(), 4);  // 4 flag outputs

    // Verify port connections work (don't throw)
    try {
        builder.connect(data, 0, flag_reg, "FlagIn[0]_Zero");
        builder.connect(data, 0, flag_reg, "Load");
        builder.connect(data, 0, flag_reg, "Clock");
        builder.connect(flag_reg, "FlagOut[0]_Zero", out, 0);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("IC port access failed: %1").arg(e.what())));
    }
}
