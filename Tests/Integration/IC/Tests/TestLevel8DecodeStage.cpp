// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel8DecodeStage.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::readMultiBitOutput;
using TestUtils::setMultiBitInput;
using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel8DecodeStage::initTestCase()
{
}

void TestLevel8DecodeStage::cleanup()
{
}

void TestLevel8DecodeStage::testDecodeStage_data()
{
    QTest::addColumn<int>("opcode");
    QTest::addColumn<int>("expectedALUOp");
    QTest::addColumn<bool>("expectedRegWrite");
    QTest::addColumn<bool>("expectedMemRead");
    QTest::addColumn<bool>("expectedMemWrite");

    // OpCode[0-2] = ALU operation
    // OpCode[3-4] = control bits
    // Bit 4: 0 = ALU (RegWrite), 1 = Memory
    // Bit 3: 0 = Read, 1 = Write (when bit 4 = 1)

    // ADD (0x00): ALUOp=0, RegWrite=1, MemRead=0, MemWrite=0
    QTest::newRow("ADD (0x00)") << 0x00 << 0 << true << false << false;

    // SUB (0x01): ALUOp=1, RegWrite=1, MemRead=0, MemWrite=0
    QTest::newRow("SUB (0x01)") << 0x01 << 1 << true << false << false;

    // AND (0x02): ALUOp=2, RegWrite=1, MemRead=0, MemWrite=0
    QTest::newRow("AND (0x02)") << 0x02 << 2 << true << false << false;

    // OR (0x03): ALUOp=3, RegWrite=1, MemRead=0, MemWrite=0
    QTest::newRow("OR (0x03)") << 0x03 << 3 << true << false << false;

    // XOR (0x04): ALUOp=4, RegWrite=1, MemRead=0, MemWrite=0
    QTest::newRow("XOR (0x04)") << 0x04 << 4 << true << false << false;

    // NOT (0x05): ALUOp=5, RegWrite=1, MemRead=0, MemWrite=0
    QTest::newRow("NOT (0x05)") << 0x05 << 5 << true << false << false;

    // SHL (0x06): ALUOp=6, RegWrite=1, MemRead=0, MemWrite=0
    QTest::newRow("SHL (0x06)") << 0x06 << 6 << true << false << false;

    // SHR (0x07): ALUOp=7, RegWrite=1, MemRead=0, MemWrite=0
    QTest::newRow("SHR (0x07)") << 0x07 << 7 << true << false << false;

    // LOAD (OpCode[4-3]=10): ALUOp=?, RegWrite=0, MemRead=1, MemWrite=0
    QTest::newRow("LOAD (0x10)") << 0x10 << 0 << false << true << false;

    // STORE (OpCode[4-3]=11): ALUOp=?, RegWrite=0, MemRead=0, MemWrite=1
    QTest::newRow("STORE (0x18)") << 0x18 << 0 << false << false << true;

    // Extended ALU with bit 4 set but bit 3=0 (LOAD variant): ALUOp=5, MemRead=1
    QTest::newRow("LOAD-ALUOp5 (0x15)") << 0x15 << 5 << false << true << false;

    // Extended with bits 4,3 set (STORE variant): ALUOp=7, MemWrite=1
    QTest::newRow("STORE-ALUOp7 (0x1F)") << 0x1F << 7 << false << false << true;
}

void TestLevel8DecodeStage::testDecodeStage()
{
    QFETCH(int, opcode);
    QFETCH(int, expectedALUOp);
    QFETCH(bool, expectedRegWrite);
    QFETCH(bool, expectedMemRead);
    QFETCH(bool, expectedMemWrite);

    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create OpCode input switches (5-bit)
    QVector<InputSwitch *> opcode_inputs;
    for (int i = 0; i < 5; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("OpCode[%1]").arg(i));
        sw->setPos(50 + i * 60, 100);
        opcode_inputs.append(sw);
    }

    // Create clock (not used, but available)
    InputSwitch *clk = new InputSwitch();
    builder.add(clk);
    clk->setLabel("Clock");
    clk->setPos(350, 100);

    // Create reset (not used, but available)
    InputSwitch *reset = new InputSwitch();
    builder.add(reset);
    reset->setLabel("Reset");
    reset->setPos(400, 100);

    // Load the decode stage IC
    IC *decode = loadBuildingBlockIC("level8_decode_stage.panda");
    builder.add(decode);

    // Create output LEDs for ALUOp (3-bit)
    QVector<Led *> aluop_leds;
    for (int i = 0; i < 3; i++) {
        Led *led = new Led();
        builder.add(led);
        led->setLabel(QString("ALUOp[%1]").arg(i));
        led->setPos(50 + i * 60, 200);
        aluop_leds.append(led);
    }

    // Create output LEDs for control signals
    Led *regwrite_led = new Led();
    builder.add(regwrite_led);
    regwrite_led->setLabel("RegWrite");
    regwrite_led->setPos(250, 200);

    Led *memread_led = new Led();
    builder.add(memread_led);
    memread_led->setLabel("MemRead");
    memread_led->setPos(310, 200);

    Led *memwrite_led = new Led();
    builder.add(memwrite_led);
    memwrite_led->setLabel("MemWrite");
    memwrite_led->setPos(370, 200);

    // Connect OpCode inputs
    for (int i = 0; i < 5; i++) {
        builder.connect(opcode_inputs[i], 0, decode, QString("OpCode[%1]").arg(i));
    }

    // Connect control inputs
    builder.connect(clk, 0, decode, "Clock");
    builder.connect(reset, 0, decode, "Reset");

    // Connect outputs to LEDs
    for (int i = 0; i < 3; i++) {
        builder.connect(decode, QString("ALUOp[%1]").arg(i), aluop_leds[i], 0);
    }
    builder.connect(decode, "RegWrite", regwrite_led, 0);
    builder.connect(decode, "MemRead", memread_led, 0);
    builder.connect(decode, "MemWrite", memwrite_led, 0);

    Simulation *sim = builder.initSimulation();
    sim->update();

    // Set OpCode value
    setMultiBitInput(opcode_inputs, opcode);
    sim->update();

    // Read outputs
    int aluop = readMultiBitOutput(QVector<GraphicElement *>(aluop_leds.begin(), aluop_leds.end()), 0);
    bool regwrite = getInputStatus(regwrite_led);
    bool memread = getInputStatus(memread_led);
    bool memwrite = getInputStatus(memwrite_led);

    // Verify
    QCOMPARE(aluop, expectedALUOp);
    QCOMPARE(regwrite, expectedRegWrite);
    QCOMPARE(memread, expectedMemRead);
    QCOMPARE(memwrite, expectedMemWrite);
}

void TestLevel8DecodeStage::testDecodeStageStructure()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *decode = loadBuildingBlockIC("level8_decode_stage.panda");
    builder.add(decode);

    // Verify IC has been loaded
    QVERIFY(decode != nullptr);

    // Create dummy elements for connection testing
    InputSwitch *sw = new InputSwitch();
    builder.add(sw);

    Led *out = new Led();
    builder.add(out);

    // Verify IC port structure
    // Inputs: 5 OpCode + Clock + Reset = 7 inputs
    // Outputs: 3 ALUOp + RegWrite + MemRead + MemWrite = 6 outputs
    QCOMPARE(decode->inputSize(), 7);   // 5 OpCode + Clock + Reset
    QCOMPARE(decode->outputSize(), 6);  // 3 ALUOp + 3 control signals

    // Verify port connections work (don't throw)
    try {
        builder.connect(sw, 0, decode, "OpCode[0]");
        builder.connect(sw, 0, decode, "OpCode[4]");
        builder.connect(sw, 0, decode, "Clock");
        builder.connect(sw, 0, decode, "Reset");
        builder.connect(decode, "ALUOp[0]", out, 0);
        builder.connect(decode, "ALUOp[2]", out, 0);
        builder.connect(decode, "RegWrite", out, 0);
        builder.connect(decode, "MemRead", out, 0);
        builder.connect(decode, "MemWrite", out, 0);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("Decode stage port access failed: %1").arg(e.what())));
    }
}
