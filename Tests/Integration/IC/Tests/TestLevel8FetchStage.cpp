// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel8FetchStage.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::setMultiBitInput;
using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel8FetchStage::initTestCase()
{
}

void TestLevel8FetchStage::cleanup()
{
}

void TestLevel8FetchStage::testFetchStage_data()
{
    QTest::addColumn<int>("initialPC");
    QTest::addColumn<int>("expectedPC");
    QTest::addColumn<int>("expectedInstr");

    // Test case: Load PC with 0x00, should fetch instruction at address 0
    QTest::newRow("fetch at address 0x00") << 0x00 << 0x00 << 0x00;

    // Test case: Load PC with 0x42, should fetch instruction at address 0x42
    QTest::newRow("fetch at address 0x42") << 0x42 << 0x42 << 0x00;

    // Test case: Load PC with 0xFF
    QTest::newRow("fetch at address 0xFF") << 0xFF << 0xFF << 0x00;

    // Test case: Load PC with 0x10
    QTest::newRow("fetch at address 0x10") << 0x10 << 0x10 << 0x00;

    // Test case: Load PC with 0x80
    QTest::newRow("fetch at address 0x80") << 0x80 << 0x80 << 0x00;
}

void TestLevel8FetchStage::testFetchStage()
{
    QFETCH(int, initialPC);
    QFETCH(int, expectedPC);
    QFETCH(int, expectedInstr);

    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create PC data input switches (8-bit)
    QVector<InputSwitch *> pcdata_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("PCData[%1]").arg(i));
        sw->setPos(50 + i * 60, 100);
        pcdata_inputs.append(sw);
    }

    // Create clock input
    InputSwitch *clk = new InputSwitch();
    builder.add(clk);
    clk->setLabel("Clock");
    clk->setPos(600, 100);

    // Create control signals
    InputSwitch *reset = new InputSwitch();
    builder.add(reset);
    reset->setLabel("Reset");
    reset->setPos(650, 100);

    InputSwitch *pcload = new InputSwitch();
    builder.add(pcload);
    pcload->setLabel("PCLoad");
    pcload->setPos(700, 100);

    InputSwitch *pcinc = new InputSwitch();
    builder.add(pcinc);
    pcinc->setLabel("PCInc");
    pcinc->setPos(750, 100);

    InputSwitch *instrload = new InputSwitch();
    builder.add(instrload);
    instrload->setLabel("InstrLoad");
    instrload->setPos(800, 100);

    // Load the fetch stage IC
    IC *fetch = loadBuildingBlockIC("level8_fetch_stage.panda");
    builder.add(fetch);

    // Create output LEDs for PC (8-bit)
    QVector<Led *> pc_leds;
    for (int i = 0; i < 8; i++) {
        Led *led = new Led();
        builder.add(led);
        led->setLabel(QString("PC[%1]").arg(i));
        led->setPos(50 + i * 60, 200);
        pc_leds.append(led);
    }

    // Create output LEDs for Instruction (8-bit)
    QVector<Led *> instr_leds;
    for (int i = 0; i < 8; i++) {
        Led *led = new Led();
        builder.add(led);
        led->setLabel(QString("Instruction[%1]").arg(i));
        led->setPos(50 + i * 60, 300);
        instr_leds.append(led);
    }

    // Connect PC data inputs
    for (int i = 0; i < 8; i++) {
        builder.connect(pcdata_inputs[i], 0, fetch, QString("PCData[%1]").arg(i));
    }

    // Connect control signals
    builder.connect(clk, 0, fetch, "Clock");
    builder.connect(reset, 0, fetch, "Reset");
    builder.connect(pcload, 0, fetch, "PCLoad");
    builder.connect(pcinc, 0, fetch, "PCInc");
    builder.connect(instrload, 0, fetch, "InstrLoad");

    // Connect outputs to LEDs
    for (int i = 0; i < 8; i++) {
        builder.connect(fetch, QString("PC[%1]").arg(i), pc_leds[i], 0);
        builder.connect(fetch, QString("Instruction[%1]").arg(i), instr_leds[i], 0);
    }

    Simulation *sim = builder.initSimulation();
    sim->update();

    // Set PC value
    setMultiBitInput(pcdata_inputs, initialPC);
    pcload->setOn(true);
    sim->update();

    // Load instruction with rising clock edge
    instrload->setOn(true);
    clk->setOn(true);
    sim->update();
    clk->setOn(false);
    sim->update();

    // Read outputs
    int pc = readMultiBitOutput(QVector<GraphicElement *>(pc_leds.begin(), pc_leds.end()), 0);
    int instr = readMultiBitOutput(QVector<GraphicElement *>(instr_leds.begin(), instr_leds.end()), 0);

    // Verify
    QCOMPARE(pc, expectedPC);
    QCOMPARE(instr, expectedInstr);
}

void TestLevel8FetchStage::testFetchStageStructure()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *fetch = loadBuildingBlockIC("level8_fetch_stage.panda");
    builder.add(fetch);

    // Verify IC has been loaded
    QVERIFY(fetch != nullptr);

    // Create dummy elements for connection testing
    InputSwitch *sw = new InputSwitch();
    builder.add(sw);

    Led *out = new Led();
    builder.add(out);

    // Verify IC port structure
    // Inputs: 8 PCData + Clock + Reset + PCLoad + PCInc + InstrLoad = 13 inputs
    // Outputs: 8 PC + 8 Instruction + 5 OpCode + 3 RegisterAddr = 24 outputs
    QCOMPARE(fetch->inputSize(), 13);   // 8 PCData + 5 control signals
    QCOMPARE(fetch->outputSize(), 24);  // 8 PC + 8 Instruction + 5 OpCode + 3 RegisterAddr

    // Verify port connections work (don't throw)
    try {
        builder.connect(sw, 0, fetch, "PCData[0]");
        builder.connect(sw, 0, fetch, "PCData[7]");
        builder.connect(sw, 0, fetch, "Clock");
        builder.connect(sw, 0, fetch, "Reset");
        builder.connect(sw, 0, fetch, "PCLoad");
        builder.connect(sw, 0, fetch, "PCInc");
        builder.connect(sw, 0, fetch, "InstrLoad");
        builder.connect(fetch, "PC[0]", out, 0);
        builder.connect(fetch, "PC[7]", out, 0);
        builder.connect(fetch, "Instruction[0]", out, 0);
        builder.connect(fetch, "OpCode[0]", out, 0);
        builder.connect(fetch, "RegisterAddr[0]", out, 0);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Fetch stage port access failed: %1").arg(e.what())));
    }
}

