// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel9FetchStage16bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

struct FetchStage16bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *clk = nullptr;
    InputSwitch *reset = nullptr;
    InputSwitch *pcLoad = nullptr;
    InputSwitch *pcInc = nullptr;
    InputSwitch *instrLoad = nullptr;
    InputSwitch *pcDataIn[8] = {};
    InputSwitch *progAddr[8] = {};
    InputSwitch *progData[16] = {};
    InputSwitch *progWrite = nullptr;
    Led *pc[8] = {};
    Led *instruction[16] = {};
    Led *rawInstr[16] = {};
    Led *opCode[5] = {};
    Led *destReg[5] = {};
    Led *srcBits[6] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level9_fetch_stage_16bit.panda");

        clk = new InputSwitch();
        reset = new InputSwitch();
        pcLoad = new InputSwitch();
        pcInc = new InputSwitch();
        instrLoad = new InputSwitch();
        builder.add(clk, reset, pcLoad, pcInc, instrLoad, ic);

        for (int i = 0; i < 8; ++i) {
            pcDataIn[i] = new InputSwitch();
            pc[i] = new Led();
            progAddr[i] = new InputSwitch();
            builder.add(pcDataIn[i], pc[i], progAddr[i]);
        }
        progWrite = new InputSwitch();
        builder.add(progWrite);
        for (int i = 0; i < 16; ++i) {
            instruction[i] = new Led();
            rawInstr[i] = new Led();
            progData[i] = new InputSwitch();
            builder.add(instruction[i], rawInstr[i], progData[i]);
        }
        for (int i = 0; i < 5; ++i) {
            opCode[i] = new Led();
            destReg[i] = new Led();
            builder.add(opCode[i], destReg[i]);
        }
        for (int i = 0; i < 6; ++i) {
            srcBits[i] = new Led();
            builder.add(srcBits[i]);
        }

        builder.connect(clk, 0, ic, "Clock");
        builder.connect(reset, 0, ic, "Reset");
        builder.connect(pcLoad, 0, ic, "PCLoad");
        builder.connect(pcInc, 0, ic, "PCInc");
        builder.connect(instrLoad, 0, ic, "InstrLoad");

        for (int i = 0; i < 8; ++i) {
            builder.connect(pcDataIn[i], 0, ic, QString("PCData[%1]").arg(i));
            builder.connect(ic, QString("PC[%1]").arg(i), pc[i], 0);
            builder.connect(progAddr[i], 0, ic, QString("ProgAddr[%1]").arg(i));
        }
        builder.connect(progWrite, 0, ic, "ProgWrite");
        for (int i = 0; i < 16; ++i) {
            builder.connect(ic, QString("Instruction[%1]").arg(i), instruction[i], 0);
            builder.connect(ic, QString("RawInstr[%1]").arg(i), rawInstr[i], 0);
            builder.connect(progData[i], 0, ic, QString("ProgData[%1]").arg(i));
        }
        for (int i = 0; i < 5; ++i) {
            builder.connect(ic, QString("OpCode[%1]").arg(i), opCode[i], 0);
            builder.connect(ic, QString("DestReg[%1]").arg(i), destReg[i], 0);
        }
        for (int i = 0; i < 6; ++i) {
            builder.connect(ic, QString("SrcBits[%1]").arg(i), srcBits[i], 0);
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    int readPC()
    {
        return readMultiBitOutput(QVector<GraphicElement *>({
            pc[0], pc[1], pc[2], pc[3], pc[4], pc[5], pc[6], pc[7]
        }));
    }

    int readInstr()
    {
        return readMultiBitOutput(QVector<GraphicElement *>({
            instruction[0], instruction[1], instruction[2], instruction[3],
            instruction[4], instruction[5], instruction[6], instruction[7],
            instruction[8], instruction[9], instruction[10], instruction[11],
            instruction[12], instruction[13], instruction[14], instruction[15]
        }));
    }

    int readOpCode()
    {
        return readMultiBitOutput(QVector<GraphicElement *>({
            opCode[0], opCode[1], opCode[2], opCode[3], opCode[4]
        }));
    }

    int readDestReg()
    {
        return readMultiBitOutput(QVector<GraphicElement *>({
            destReg[0], destReg[1], destReg[2], destReg[3], destReg[4]
        }));
    }

    int readSrcBits()
    {
        return readMultiBitOutput(QVector<GraphicElement *>({
            srcBits[0], srcBits[1], srcBits[2], srcBits[3], srcBits[4], srcBits[5]
        }));
    }

    int readRawInstr()
    {
        return readMultiBitOutput(QVector<GraphicElement *>({
            rawInstr[0], rawInstr[1], rawInstr[2], rawInstr[3],
            rawInstr[4], rawInstr[5], rawInstr[6], rawInstr[7],
            rawInstr[8], rawInstr[9], rawInstr[10], rawInstr[11],
            rawInstr[12], rawInstr[13], rawInstr[14], rawInstr[15]
        }));
    }

    // Write a 16-bit word into instruction memory at the given address
    void programWord(int address, int word)
    {
        for (int i = 0; i < 8; ++i) {
            progAddr[i]->setOn((address >> i) & 1);
        }
        for (int i = 0; i < 16; ++i) {
            progData[i]->setOn((word >> i) & 1);
        }
        progWrite->setOn(true);
        sim->update();
        clockCycle(sim, clk);
        progWrite->setOn(false);
        sim->update();
    }

    void resetCpu()
    {
        clk->setOn(false);
        reset->setOn(true);
        pcLoad->setOn(false);
        pcInc->setOn(false);
        instrLoad->setOn(false);
        sim->update();
        clockCycle(sim, clk);
        reset->setOn(false);
        sim->update();
    }
};

static std::unique_ptr<FetchStage16bitFixture> s_level9FetchStage16bit;

void TestLevel9FetchStage16Bit::initTestCase()
{
    s_level9FetchStage16bit = std::make_unique<FetchStage16bitFixture>();
    QVERIFY(s_level9FetchStage16bit->build());
}

void TestLevel9FetchStage16Bit::cleanupTestCase()
{
    s_level9FetchStage16bit.reset();
}

void TestLevel9FetchStage16Bit::cleanup()
{
    if (s_level9FetchStage16bit && s_level9FetchStage16bit->sim) {
        s_level9FetchStage16bit->sim->restart();
        s_level9FetchStage16bit->sim->update();
    }
}

void TestLevel9FetchStage16Bit::testFetchStage16Bit_data()
{
    QTest::addColumn<int>("pcData");
    QTest::addColumn<int>("expectedPC");

    QTest::newRow("pc_load_0x00") << 0x00 << 0x00;
    QTest::newRow("pc_load_0x01") << 0x01 << 0x01;
    QTest::newRow("pc_load_0xFF") << 0xFF << 0xFF;
    QTest::newRow("pc_load_0x80") << 0x80 << 0x80;
}

void TestLevel9FetchStage16Bit::testFetchStage16Bit()
{
    QFETCH(int, pcData);
    QFETCH(int, expectedPC);

    auto &f = *s_level9FetchStage16bit;

    f.resetCpu();

    // Load PC value
    for (int i = 0; i < 8; ++i) {
        f.pcDataIn[i]->setOn((pcData >> i) & 1);
    }

    f.pcLoad->setOn(true);
    f.sim->update();

    clockCycle(f.sim, f.clk);

    f.pcLoad->setOn(false);
    f.sim->update();

    QCOMPARE(f.readPC(), expectedPC);
}

void TestLevel9FetchStage16Bit::testPCIncrement()
{
    auto &f = *s_level9FetchStage16bit;

    f.resetCpu();

    f.pcInc->setOn(true);
    f.sim->update();
    for (int i = 0; i < 5; i++) {
        clockCycle(f.sim, f.clk);
    }
    f.pcInc->setOn(false);
    f.sim->update();

    QCOMPARE(f.readPC(), 5);
}

void TestLevel9FetchStage16Bit::testPCLoadValue()
{
    auto &f = *s_level9FetchStage16bit;

    f.resetCpu();

    constexpr int TARGET_PC = 0x42;
    for (int i = 0; i < 8; ++i) {
        f.pcDataIn[i]->setOn((TARGET_PC >> i) & 1);
    }
    f.pcLoad->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clk);
    f.pcLoad->setOn(false);
    f.sim->update();

    QCOMPARE(f.readPC(), TARGET_PC);
}

// Program a known word and verify the fetched fields are decoded LSB-first
// (F53: the fields used to be emitted MSB-first, and with no programming
// interface the memory was always blank, so the old field assertions only
// ever compared 0 == 0).
void TestLevel9FetchStage16Bit::testInstructionFieldsDecoded()
{
    auto &f = *s_level9FetchStage16bit;

    f.resetCpu();

    // OpCode=0b10101 (0x15), DestReg=0b01010 (0x0A), SrcBits=0b101010 (0x2A)
    constexpr int WORD = (0x15 << 11) | (0x0A << 6) | 0x2A;  // 0xAAAA
    f.programWord(0x00, WORD);

    // PC=0 after reset; the unregistered word is visible immediately
    QCOMPARE(f.readRawInstr(), WORD);

    // Load the instruction register
    f.instrLoad->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clk);
    f.instrLoad->setOn(false);
    f.sim->update();

    QCOMPARE(f.readInstr(), WORD);
    QCOMPARE(f.readOpCode(), 0x15);
    QCOMPARE(f.readDestReg(), 0x0A);
    QCOMPARE(f.readSrcBits(), 0x2A);

    // A byte-ASYMMETRIC word catches a low/high byte-lane swap between the two
    // instruction_memory_interface lanes — 0xAAAA above is byte-symmetric, so a
    // swap would be invisible. 0x1234 = low byte 0x34, high byte 0x12 ->
    // OpCode 0x02, DestReg 0x08, SrcBits 0x34.
    constexpr int WORD2 = 0x1234;
    f.programWord(0x00, WORD2);
    QCOMPARE(f.readRawInstr(), WORD2);

    f.instrLoad->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clk);
    f.instrLoad->setOn(false);
    f.sim->update();

    QCOMPARE(f.readInstr(), WORD2);
    QCOMPARE(f.readOpCode(), 0x02);
    QCOMPARE(f.readDestReg(), 0x08);
    QCOMPARE(f.readSrcBits(), 0x34);
}

// The instruction register holds its word while InstrLoad=0 and loads the
// fetched word on the next rising edge when InstrLoad=1 (F53: InstrLoad
// used to be a dead input — there was no IR at all).
void TestLevel9FetchStage16Bit::testInstructionRegisterHold()
{
    auto &f = *s_level9FetchStage16bit;

    f.resetCpu();

    constexpr int WORD_A = 0xAAAA;
    constexpr int WORD_B = (0x0A << 11) | (0x15 << 6) | 0x15;  // 0x5555

    // Load WORD_A through the IR
    f.programWord(0x00, WORD_A);
    f.instrLoad->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clk);
    f.instrLoad->setOn(false);
    f.sim->update();
    QCOMPARE(f.readInstr(), WORD_A);

    // Overwrite memory at the same address: RawInstr follows, the IR holds
    f.programWord(0x00, WORD_B);
    QCOMPARE(f.readRawInstr(), WORD_B);
    QCOMPARE(f.readInstr(), WORD_A);

    // A clock edge with InstrLoad still low: the IR keeps holding
    clockCycle(f.sim, f.clk);
    QCOMPARE(f.readInstr(), WORD_A);

    // Raise InstrLoad: the next edge loads the new word
    f.instrLoad->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clk);
    f.instrLoad->setOn(false);
    f.sim->update();
    QCOMPARE(f.readInstr(), WORD_B);
}

void TestLevel9FetchStage16Bit::testSrcBitsMatchInstruction()
{
    auto &f = *s_level9FetchStage16bit;

    f.resetCpu();

    // Program distinct SrcBits patterns at addresses 0..4 and walk the PC
    for (int addr = 0; addr < 5; ++addr) {
        f.programWord(addr, ((addr + 1) << 11) | (addr << 6) | (0x3F - addr));
    }

    f.instrLoad->setOn(true);
    f.pcInc->setOn(true);
    f.sim->update();

    for (int pc_state = 0; pc_state < 5; ++pc_state) {
        clockCycle(f.sim, f.clk);

        int instrWord = f.readInstr();
        int srcBitsVal = f.readSrcBits();

        int expectedSrcBits = instrWord & 0x3F;
        QCOMPARE(srcBitsVal, expectedSrcBits);
    }
}

void TestLevel9FetchStage16Bit::testResetClearsPC()
{
    auto &f = *s_level9FetchStage16bit;

    f.resetCpu();

    // Increment PC 5 times
    f.pcInc->setOn(true);
    f.sim->update();
    for (int i = 0; i < 5; ++i) {
        clockCycle(f.sim, f.clk);
    }
    f.pcInc->setOn(false);
    f.sim->update();

    QCOMPARE(f.readPC(), 5);

    // Apply reset
    f.reset->setOn(true);
    clockCycle(f.sim, f.clk);
    f.reset->setOn(false);
    f.sim->update();

    QCOMPARE(f.readPC(), 0);
}

void TestLevel9FetchStage16Bit::testPCWrapAround()
{
    auto &f = *s_level9FetchStage16bit;

    f.resetCpu();

    // Load PC with 0xFF
    constexpr int TARGET_PC = 0xFF;
    for (int i = 0; i < 8; ++i) {
        f.pcDataIn[i]->setOn((TARGET_PC >> i) & 1);
    }
    f.pcLoad->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clk);
    f.pcLoad->setOn(false);
    f.sim->update();

    QCOMPARE(f.readPC(), 0xFF);

    // Increment once to wrap around
    f.pcInc->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clk);
    f.pcInc->setOn(false);
    f.sim->update();

    QCOMPARE(f.readPC(), 0x00);
}
