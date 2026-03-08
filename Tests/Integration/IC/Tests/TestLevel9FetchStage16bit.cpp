// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel9FetchStage16bit.h"

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
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel9FetchStage16Bit::initTestCase()
{
}

void TestLevel9FetchStage16Bit::cleanup()
{
}

void TestLevel9FetchStage16Bit::testFetchStage16Bit_data()
{
    QTest::addColumn<int>("pcData");
    QTest::addColumn<int>("expectedPC");

    // Test basic PC load and output
    QTest::newRow("pc_load_0x00") << 0x00 << 0x00;
    QTest::newRow("pc_load_0x01") << 0x01 << 0x01;
    QTest::newRow("pc_load_0xFF") << 0xFF << 0xFF;
    QTest::newRow("pc_load_0x80") << 0x80 << 0x80;
}

void TestLevel9FetchStage16Bit::testFetchStage16Bit()
{
    QFETCH(int, pcData);
    QFETCH(int, expectedPC);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Control signals
    InputSwitch clk;
    InputSwitch reset;
    InputSwitch pcLoad;
    InputSwitch pcInc;
    InputSwitch instrLoad;

    // PC data input
    InputSwitch pcDataIn[8];

    // Output signals
    Led pc[8];
    Led instruction[16];
    Led opCode[5];
    Led destReg[5];
    Led srcBits[6];

    // Load Fetch Stage 16-bit IC
    IC *fetchIC = loadBuildingBlockIC("level9_fetch_stage_16bit.panda");

    // Add elements
    builder.add(&clk, &reset, &pcLoad, &pcInc, &instrLoad, fetchIC);
    for (int i = 0; i < 8; ++i) {
        builder.add(&pcDataIn[i], &pc[i]);
    }
    for (int i = 0; i < 16; ++i) {
        builder.add(&instruction[i]);
    }
    for (int i = 0; i < 5; ++i) {
        builder.add(&opCode[i], &destReg[i]);
    }
    for (int i = 0; i < 6; ++i) {
        builder.add(&srcBits[i]);
    }

    // Connect control signals
    builder.connect(&clk, 0, fetchIC, "Clock");
    builder.connect(&reset, 0, fetchIC, "Reset");
    builder.connect(&pcLoad, 0, fetchIC, "PCLoad");
    builder.connect(&pcInc, 0, fetchIC, "PCInc");
    builder.connect(&instrLoad, 0, fetchIC, "InstrLoad");

    // Connect PC data input
    for (int i = 0; i < 8; ++i) {
        builder.connect(&pcDataIn[i], 0, fetchIC, QString("PCData[%1]").arg(i));
    }

    // Connect PC output
    for (int i = 0; i < 8; ++i) {
        builder.connect(fetchIC, QString("PC[%1]").arg(i), &pc[i], 0);
    }

    // Connect instruction output
    for (int i = 0; i < 16; ++i) {
        builder.connect(fetchIC, QString("Instruction[%1]").arg(i), &instruction[i], 0);
    }

    // Connect opcode output
    for (int i = 0; i < 5; ++i) {
        builder.connect(fetchIC, QString("OpCode[%1]").arg(i), &opCode[i], 0);
    }

    // Connect dest reg output
    for (int i = 0; i < 5; ++i) {
        builder.connect(fetchIC, QString("DestReg[%1]").arg(i), &destReg[i], 0);
    }

    // Connect src bits output
    for (int i = 0; i < 6; ++i) {
        builder.connect(fetchIC, QString("SrcBits[%1]").arg(i), &srcBits[i], 0);
    }

    auto *simulation = builder.initSimulation();

    // Initialize
    clk.setOn(false);
    reset.setOn(true);
    pcLoad.setOn(false);
    pcInc.setOn(false);
    instrLoad.setOn(false);
    simulation->update();

    // Reset pulse
    clockCycle(simulation, &clk);

    reset.setOn(false);
    simulation->update();

    // Load PC value
    for (int i = 0; i < 8; ++i) {
        pcDataIn[i].setOn((pcData >> i) & 1);
    }

    pcLoad.setOn(true);
    simulation->update();

    // Clock pulse to load
    clockCycle(simulation, &clk);

    pcLoad.setOn(false);
    simulation->update();

    // Read PC output
    int resultPC = readMultiBitOutput(QVector<GraphicElement *>({
        &pc[0], &pc[1], &pc[2], &pc[3],
        &pc[4], &pc[5], &pc[6], &pc[7]
    }));

    QCOMPARE(resultPC, expectedPC);
}

void TestLevel9FetchStage16Bit::testPCIncrement()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch clk, reset, pcLoad, pcInc, instrLoad;
    InputSwitch pcDataIn[8];
    Led pc[8];

    IC *fetchIC = loadBuildingBlockIC("level9_fetch_stage_16bit.panda");
    builder.add(&clk, &reset, &pcLoad, &pcInc, &instrLoad, fetchIC);
    for (int i = 0; i < 8; ++i) {
        builder.add(&pcDataIn[i], &pc[i]);
    }

    builder.connect(&clk, 0, fetchIC, "Clock");
    builder.connect(&reset, 0, fetchIC, "Reset");
    builder.connect(&pcLoad, 0, fetchIC, "PCLoad");
    builder.connect(&pcInc, 0, fetchIC, "PCInc");
    builder.connect(&instrLoad, 0, fetchIC, "InstrLoad");
    for (int i = 0; i < 8; ++i) {
        builder.connect(&pcDataIn[i], 0, fetchIC, QString("PCData[%1]").arg(i));
        builder.connect(fetchIC, QString("PC[%1]").arg(i), &pc[i], 0);
    }

    auto *sim = builder.initSimulation();

    reset.setOn(true);
    sim->update();
    clockCycle(sim, &clk);
    reset.setOn(false);
    sim->update();

    pcInc.setOn(true);
    sim->update();
    for (int i = 0; i < 5; i++) {
        clockCycle(sim, &clk);
    }
    pcInc.setOn(false);
    sim->update();

    int resultPC = readMultiBitOutput(QVector<GraphicElement *>({
        &pc[0], &pc[1], &pc[2], &pc[3], &pc[4], &pc[5], &pc[6], &pc[7]
    }));
    QCOMPARE(resultPC, 5);
}

void TestLevel9FetchStage16Bit::testPCLoadValue()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch clk, reset, pcLoad, pcInc, instrLoad;
    InputSwitch pcDataIn[8];
    Led pc[8];

    IC *fetchIC = loadBuildingBlockIC("level9_fetch_stage_16bit.panda");
    builder.add(&clk, &reset, &pcLoad, &pcInc, &instrLoad, fetchIC);
    for (int i = 0; i < 8; ++i) {
        builder.add(&pcDataIn[i], &pc[i]);
    }

    builder.connect(&clk, 0, fetchIC, "Clock");
    builder.connect(&reset, 0, fetchIC, "Reset");
    builder.connect(&pcLoad, 0, fetchIC, "PCLoad");
    builder.connect(&pcInc, 0, fetchIC, "PCInc");
    builder.connect(&instrLoad, 0, fetchIC, "InstrLoad");
    for (int i = 0; i < 8; ++i) {
        builder.connect(&pcDataIn[i], 0, fetchIC, QString("PCData[%1]").arg(i));
        builder.connect(fetchIC, QString("PC[%1]").arg(i), &pc[i], 0);
    }

    auto *sim = builder.initSimulation();

    reset.setOn(true);
    sim->update();
    clockCycle(sim, &clk);
    reset.setOn(false);
    sim->update();

    constexpr int TARGET_PC = 0x42;
    for (int i = 0; i < 8; ++i) {
        pcDataIn[i].setOn((TARGET_PC >> i) & 1);
    }
    pcLoad.setOn(true);
    sim->update();
    clockCycle(sim, &clk);
    pcLoad.setOn(false);
    sim->update();

    int resultPC = readMultiBitOutput(QVector<GraphicElement *>({
        &pc[0], &pc[1], &pc[2], &pc[3], &pc[4], &pc[5], &pc[6], &pc[7]
    }));
    QCOMPARE(resultPC, TARGET_PC);
}

void TestLevel9FetchStage16Bit::testInstructionFieldsDecoded()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch clk, reset, pcLoad, pcInc, instrLoad;
    InputSwitch pcDataIn[8];
    Led instruction[16], opCode[5], destReg[5];

    IC *fetchIC = loadBuildingBlockIC("level9_fetch_stage_16bit.panda");
    builder.add(&clk, &reset, &pcLoad, &pcInc, &instrLoad, fetchIC);
    for (int i = 0; i < 8; ++i) {
        builder.add(&pcDataIn[i]);
    }
    for (int i = 0; i < 16; ++i) {
        builder.add(&instruction[i]);
    }
    for (int i = 0; i < 5; ++i) {
        builder.add(&opCode[i], &destReg[i]);
    }

    builder.connect(&clk, 0, fetchIC, "Clock");
    builder.connect(&reset, 0, fetchIC, "Reset");
    builder.connect(&pcLoad, 0, fetchIC, "PCLoad");
    builder.connect(&pcInc, 0, fetchIC, "PCInc");
    builder.connect(&instrLoad, 0, fetchIC, "InstrLoad");
    for (int i = 0; i < 8; ++i) {
        builder.connect(&pcDataIn[i], 0, fetchIC, QString("PCData[%1]").arg(i));
    }
    for (int i = 0; i < 16; ++i) {
        builder.connect(fetchIC, QString("Instruction[%1]").arg(i), &instruction[i], 0);
    }
    for (int i = 0; i < 5; ++i) {
        builder.connect(fetchIC, QString("OpCode[%1]").arg(i), &opCode[i], 0);
        builder.connect(fetchIC, QString("DestReg[%1]").arg(i), &destReg[i], 0);
    }

    auto *sim = builder.initSimulation();

    reset.setOn(true);
    sim->update();
    clockCycle(sim, &clk);
    reset.setOn(false);
    pcInc.setOn(true);
    sim->update();
    clockCycle(sim, &clk);

    // Read full 16-bit instruction and decoded fields
    int instrWord = readMultiBitOutput(QVector<GraphicElement *>({
        &instruction[0], &instruction[1], &instruction[2], &instruction[3],
        &instruction[4], &instruction[5], &instruction[6], &instruction[7],
        &instruction[8], &instruction[9], &instruction[10], &instruction[11],
        &instruction[12], &instruction[13], &instruction[14], &instruction[15]
    }));
    int opCodeVal = readMultiBitOutput(QVector<GraphicElement *>({
        &opCode[0], &opCode[1], &opCode[2], &opCode[3], &opCode[4]
    }));
    int destRegVal = readMultiBitOutput(QVector<GraphicElement *>({
        &destReg[0], &destReg[1], &destReg[2], &destReg[3], &destReg[4]
    }));

    // OpCode must equal bits [15:11] of instruction word
    int expectedOpCode = (instrWord >> 11) & 0x1F;
    QCOMPARE(opCodeVal, expectedOpCode);

    // DestReg must equal bits [10:6] of instruction word
    int expectedDestReg = (instrWord >> 6) & 0x1F;
    QCOMPARE(destRegVal, expectedDestReg);
}

void TestLevel9FetchStage16Bit::testSrcBitsMatchInstruction()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch clk, reset, pcLoad, pcInc, instrLoad;
    InputSwitch pcDataIn[8];
    Led instruction[16], srcBits[6];

    IC *fetchIC = loadBuildingBlockIC("level9_fetch_stage_16bit.panda");
    builder.add(&clk, &reset, &pcLoad, &pcInc, &instrLoad, fetchIC);
    for (int i = 0; i < 8; ++i) {
        builder.add(&pcDataIn[i]);
    }
    for (int i = 0; i < 16; ++i) {
        builder.add(&instruction[i]);
    }
    for (int i = 0; i < 6; ++i) {
        builder.add(&srcBits[i]);
    }

    builder.connect(&clk, 0, fetchIC, "Clock");
    builder.connect(&reset, 0, fetchIC, "Reset");
    builder.connect(&pcLoad, 0, fetchIC, "PCLoad");
    builder.connect(&pcInc, 0, fetchIC, "PCInc");
    builder.connect(&instrLoad, 0, fetchIC, "InstrLoad");
    for (int i = 0; i < 8; ++i) {
        builder.connect(&pcDataIn[i], 0, fetchIC, QString("PCData[%1]").arg(i));
    }
    for (int i = 0; i < 16; ++i) {
        builder.connect(fetchIC, QString("Instruction[%1]").arg(i), &instruction[i], 0);
    }
    for (int i = 0; i < 6; ++i) {
        builder.connect(fetchIC, QString("SrcBits[%1]").arg(i), &srcBits[i], 0);
    }

    auto *sim = builder.initSimulation();

    reset.setOn(true);
    sim->update();
    clockCycle(sim, &clk);
    reset.setOn(false);
    pcInc.setOn(true);
    sim->update();

    // Test through several PC states
    for (int pc_state = 0; pc_state < 5; ++pc_state) {
        clockCycle(sim, &clk);

        int instrWord = readMultiBitOutput(QVector<GraphicElement *>({
            &instruction[0], &instruction[1], &instruction[2], &instruction[3],
            &instruction[4], &instruction[5], &instruction[6], &instruction[7],
            &instruction[8], &instruction[9], &instruction[10], &instruction[11],
            &instruction[12], &instruction[13], &instruction[14], &instruction[15]
        }));
        int srcBitsVal = readMultiBitOutput(QVector<GraphicElement *>({
            &srcBits[0], &srcBits[1], &srcBits[2], &srcBits[3], &srcBits[4], &srcBits[5]
        }));

        // SrcBits must equal bits [5:0] of instruction word
        int expectedSrcBits = instrWord & 0x3F;
        QCOMPARE(srcBitsVal, expectedSrcBits);
    }
}

void TestLevel9FetchStage16Bit::testResetClearsPC()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch clk, reset, pcLoad, pcInc, instrLoad;
    InputSwitch pcDataIn[8];
    Led pc[8];

    IC *fetchIC = loadBuildingBlockIC("level9_fetch_stage_16bit.panda");
    builder.add(&clk, &reset, &pcLoad, &pcInc, &instrLoad, fetchIC);
    for (int i = 0; i < 8; ++i) {
        builder.add(&pcDataIn[i], &pc[i]);
    }

    builder.connect(&clk, 0, fetchIC, "Clock");
    builder.connect(&reset, 0, fetchIC, "Reset");
    builder.connect(&pcLoad, 0, fetchIC, "PCLoad");
    builder.connect(&pcInc, 0, fetchIC, "PCInc");
    builder.connect(&instrLoad, 0, fetchIC, "InstrLoad");
    for (int i = 0; i < 8; ++i) {
        builder.connect(&pcDataIn[i], 0, fetchIC, QString("PCData[%1]").arg(i));
        builder.connect(fetchIC, QString("PC[%1]").arg(i), &pc[i], 0);
    }

    auto *sim = builder.initSimulation();

    reset.setOn(true);
    sim->update();
    clockCycle(sim, &clk);
    reset.setOn(false);
    sim->update();

    // Increment PC 5 times
    pcInc.setOn(true);
    sim->update();
    for (int i = 0; i < 5; ++i) {
        clockCycle(sim, &clk);
    }
    pcInc.setOn(false);
    sim->update();

    int pcBeforeReset = readMultiBitOutput(QVector<GraphicElement *>({
        &pc[0], &pc[1], &pc[2], &pc[3], &pc[4], &pc[5], &pc[6], &pc[7]
    }));
    QCOMPARE(pcBeforeReset, 5);

    // Apply reset
    reset.setOn(true);
    clockCycle(sim, &clk);
    reset.setOn(false);
    sim->update();

    int pcAfterReset = readMultiBitOutput(QVector<GraphicElement *>({
        &pc[0], &pc[1], &pc[2], &pc[3], &pc[4], &pc[5], &pc[6], &pc[7]
    }));
    QCOMPARE(pcAfterReset, 0);
}

void TestLevel9FetchStage16Bit::testPCWrapAround()
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch clk, reset, pcLoad, pcInc, instrLoad;
    InputSwitch pcDataIn[8];
    Led pc[8];

    IC *fetchIC = loadBuildingBlockIC("level9_fetch_stage_16bit.panda");
    builder.add(&clk, &reset, &pcLoad, &pcInc, &instrLoad, fetchIC);
    for (int i = 0; i < 8; ++i) {
        builder.add(&pcDataIn[i], &pc[i]);
    }

    builder.connect(&clk, 0, fetchIC, "Clock");
    builder.connect(&reset, 0, fetchIC, "Reset");
    builder.connect(&pcLoad, 0, fetchIC, "PCLoad");
    builder.connect(&pcInc, 0, fetchIC, "PCInc");
    builder.connect(&instrLoad, 0, fetchIC, "InstrLoad");
    for (int i = 0; i < 8; ++i) {
        builder.connect(&pcDataIn[i], 0, fetchIC, QString("PCData[%1]").arg(i));
        builder.connect(fetchIC, QString("PC[%1]").arg(i), &pc[i], 0);
    }

    auto *sim = builder.initSimulation();

    reset.setOn(true);
    sim->update();
    clockCycle(sim, &clk);
    reset.setOn(false);
    sim->update();

    // Load PC with 0xFF
    constexpr int TARGET_PC = 0xFF;
    for (int i = 0; i < 8; ++i) {
        pcDataIn[i].setOn((TARGET_PC >> i) & 1);
    }
    pcLoad.setOn(true);
    sim->update();
    clockCycle(sim, &clk);
    pcLoad.setOn(false);
    sim->update();

    int pcLoaded = readMultiBitOutput(QVector<GraphicElement *>({
        &pc[0], &pc[1], &pc[2], &pc[3], &pc[4], &pc[5], &pc[6], &pc[7]
    }));
    QCOMPARE(pcLoaded, 0xFF);

    // Increment once to wrap around
    pcInc.setOn(true);
    sim->update();
    clockCycle(sim, &clk);
    pcInc.setOn(false);
    sim->update();

    int pcWrapped = readMultiBitOutput(QVector<GraphicElement *>({
        &pc[0], &pc[1], &pc[2], &pc[3], &pc[4], &pc[5], &pc[6], &pc[7]
    }));
    QCOMPARE(pcWrapped, 0x00);
}
