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
using TestUtils::inputStatus;
using CPUTestUtils::loadBuildingBlockIC;

// Fully-wired fetch-stage harness (the inline testFetchStage above leaves the
// program-write path and the OpCode/RegisterAddr/RawInstr outputs unconnected).
// Mirrors the DecodeStage/ExecuteStage/MemoryStage fixtures in this directory.
struct FetchStageFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *pcData[8] = {};
    InputSwitch *progAddr[8] = {};
    InputSwitch *progData[8] = {};
    InputSwitch *clk = nullptr;
    InputSwitch *reset = nullptr;
    InputSwitch *pcLoad = nullptr;
    InputSwitch *pcInc = nullptr;
    InputSwitch *instrLoad = nullptr;
    InputSwitch *progWrite = nullptr;
    Led *pc[8] = {};
    Led *instr[8] = {};
    Led *raw[8] = {};
    Led *opcode[5] = {};
    Led *regAddr[3] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level8_fetch_stage.panda");

        clk = new InputSwitch();
        reset = new InputSwitch();
        pcLoad = new InputSwitch();
        pcInc = new InputSwitch();
        instrLoad = new InputSwitch();
        progWrite = new InputSwitch();
        builder.add(ic, clk, reset, pcLoad, pcInc, instrLoad, progWrite);

        for (int i = 0; i < 8; ++i) {
            pcData[i] = new InputSwitch();
            progAddr[i] = new InputSwitch();
            progData[i] = new InputSwitch();
            pc[i] = new Led();
            instr[i] = new Led();
            raw[i] = new Led();
            builder.add(pcData[i], progAddr[i], progData[i], pc[i], instr[i], raw[i]);
        }
        for (int i = 0; i < 5; ++i) {
            opcode[i] = new Led();
            builder.add(opcode[i]);
        }
        for (int i = 0; i < 3; ++i) {
            regAddr[i] = new Led();
            builder.add(regAddr[i]);
        }

        for (int i = 0; i < 8; ++i) {
            builder.connect(pcData[i], 0, ic, QString("PCData[%1]").arg(i));
            builder.connect(progAddr[i], 0, ic, QString("ProgAddr[%1]").arg(i));
            builder.connect(progData[i], 0, ic, QString("ProgData[%1]").arg(i));
            builder.connect(ic, QString("PC[%1]").arg(i), pc[i], 0);
            builder.connect(ic, QString("Instruction[%1]").arg(i), instr[i], 0);
            builder.connect(ic, QString("RawInstr[%1]").arg(i), raw[i], 0);
        }
        for (int i = 0; i < 5; ++i) {
            builder.connect(ic, QString("OpCode[%1]").arg(i), opcode[i], 0);
        }
        for (int i = 0; i < 3; ++i) {
            builder.connect(ic, QString("RegisterAddr[%1]").arg(i), regAddr[i], 0);
        }
        builder.connect(clk, 0, ic, "Clock");
        builder.connect(reset, 0, ic, "Reset");
        builder.connect(pcLoad, 0, ic, "PCLoad");
        builder.connect(pcInc, 0, ic, "PCInc");
        builder.connect(instrLoad, 0, ic, "InstrLoad");
        builder.connect(progWrite, 0, ic, "ProgWrite");

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    // One clock pulse, matching the inline testFetchStage's known-good ordering.
    void pulse()
    {
        clk->setOn(true);
        sim->update();
        clk->setOn(false);
        sim->update();
    }

    // ProgWrite routes ProgAddr->mux->imem and enables the imem write; a clock
    // edge stores ProgData at that address.
    void program(int addr, int data)
    {
        for (int i = 0; i < 8; ++i) {
            progAddr[i]->setOn((addr >> i) & 1);
            progData[i]->setOn((data >> i) & 1);
        }
        progWrite->setOn(true);
        sim->update();
        pulse();
        progWrite->setOn(false);
        sim->update();
    }

    void setPC(int addr)
    {
        for (int i = 0; i < 8; ++i) {
            pcData[i]->setOn((addr >> i) & 1);
        }
        pcLoad->setOn(true);
        sim->update();
        pulse();
        pcLoad->setOn(false);
        sim->update();
    }

    // Latch the memory-addressed instruction into the instruction register.
    void latch()
    {
        instrLoad->setOn(true);
        sim->update();
        pulse();
        instrLoad->setOn(false);
        sim->update();
    }

    static int readBits(Led *const *leds, int n)
    {
        int v = 0;
        for (int i = 0; i < n; ++i) {
            if (inputStatus(leds[i])) {
                v |= (1 << i);
            }
        }
        return v;
    }

    int readPC() { return readBits(pc, 8); }
    int readInstr() { return readBits(instr, 8); }
    int readRaw() { return readBits(raw, 8); }
    int readOpCode() { return readBits(opcode, 5); }
    int readRegAddr() { return readBits(regAddr, 3); }
};

static std::unique_ptr<FetchStageFixture> s_level8FetchStage;

void TestLevel8FetchStage::initTestCase()
{
    s_level8FetchStage = std::make_unique<FetchStageFixture>();
    QVERIFY(s_level8FetchStage->build());
}

void TestLevel8FetchStage::cleanupTestCase()
{
    s_level8FetchStage.reset();
}

void TestLevel8FetchStage::cleanup()
{
    if (s_level8FetchStage && s_level8FetchStage->sim) {
        s_level8FetchStage->sim->restart();
        s_level8FetchStage->sim->update();
    }
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

// Program real instructions into the stage's instruction memory and fetch them
// back, exercising the program-write path and the OpCode/RegisterAddr/RawInstr
// outputs that the original test (which only ever saw 0x00) never touched.
void TestLevel8FetchStage::testProgramAndFetch()
{
    auto &f = *s_level8FetchStage;

    // Word at low address 5: 0xB5 -> OpCode = 0xB5>>3 = 22, RegisterAddr = 0xB5&7 = 5
    f.program(0x05, 0xB5);
    f.setPC(0x05);
    QCOMPARE(f.readRaw(), 0xB5);  // unregistered async read reflects memory immediately

    f.latch();
    QCOMPARE(f.readInstr(), 0xB5);
    QCOMPARE(f.readOpCode(), 22);
    QCOMPARE(f.readRegAddr(), 5);

    // A second, independent word at low address 2: 0x73 -> OpCode 14, RegisterAddr 3
    f.program(0x02, 0x73);
    f.setPC(0x02);
    QCOMPARE(f.readRaw(), 0x73);

    f.latch();
    QCOMPARE(f.readInstr(), 0x73);
    QCOMPARE(f.readOpCode(), 14);
    QCOMPARE(f.readRegAddr(), 3);
}

// PCInc advances the program counter — the core fetch-advance behavior, wired in
// the fixture but never asserted by the original test.
void TestLevel8FetchStage::testPCIncrement()
{
    auto &f = *s_level8FetchStage;

    f.setPC(0x10);
    QCOMPARE(f.readPC(), 0x10);

    f.pcInc->setOn(true);
    f.sim->update();
    f.pulse();
    QCOMPARE(f.readPC(), 0x11);

    f.pulse();
    QCOMPARE(f.readPC(), 0x12);

    f.pcInc->setOn(false);
}

// Reset clears the PC and the instruction register (both wired to the stage's
// Reset). Async — no clock needed. Normal counting resumes after release.
void TestLevel8FetchStage::testReset()
{
    auto &f = *s_level8FetchStage;

    f.program(0x03, 0x99);
    f.setPC(0x03);
    f.latch();
    QCOMPARE(f.readInstr(), 0x99);
    QVERIFY(f.readPC() != 0x00);

    // Assert Reset: PC and the latched instruction both clear asynchronously
    f.reset->setOn(true);
    f.sim->update();
    QCOMPARE(f.readPC(), 0x00);
    QCOMPARE(f.readInstr(), 0x00);

    // Release and resume: increment counts up from 0
    f.reset->setOn(false);
    f.pcInc->setOn(true);
    f.sim->update();
    f.pulse();
    QCOMPARE(f.readPC(), 0x01);
    f.pcInc->setOn(false);
}

// With every control input low (PCLoad/PCInc/InstrLoad/ProgWrite = 0) a clock
// edge must be a no-op: both the program counter and the latched instruction
// hold. The level-7 PC/IR hold paths are tested individually (F74), but the
// integrated stage was never checked for spurious clocking.
void TestLevel8FetchStage::testHold()
{
    auto &f = *s_level8FetchStage;

    // Establish known state: instruction 0xB5 at PC 0x05
    f.program(0x05, 0xB5);
    f.setPC(0x05);
    f.latch();
    QCOMPARE(f.readPC(), 0x05);
    QCOMPARE(f.readInstr(), 0xB5);

    // All controls already low after the helpers; pulse the clock repeatedly and
    // confirm neither the PC nor the registered instruction changes.
    for (int i = 0; i < 3; ++i) {
        f.pulse();
        QCOMPARE(f.readPC(), 0x05);
        QCOMPARE(f.readInstr(), 0xB5);
    }
}

void TestLevel8FetchStage::testPCDataBitIsolation_data()
{
    QTest::addColumn<int>("bitPosition");
    for (int i = 0; i < 8; ++i) {
        QTest::newRow(QString("pc_bit_%1").arg(i).toLatin1()) << i;
    }
}

void TestLevel8FetchStage::testPCDataBitIsolation()
{
    QFETCH(int, bitPosition);

    auto &f = *s_level8FetchStage;

    // A one-hot PCData must load to a one-hot PC at the same position.
    f.setPC(1 << bitPosition);
    QCOMPARE(f.readPC(), 1 << bitPosition);
}

void TestLevel8FetchStage::testInstructionBitIsolation_data()
{
    QTest::addColumn<int>("bitPosition");
    for (int i = 0; i < 8; ++i) {
        QTest::newRow(QString("instr_bit_%1").arg(i).toLatin1()) << i;
    }
}

void TestLevel8FetchStage::testInstructionBitIsolation()
{
    QFETCH(int, bitPosition);

    auto &f = *s_level8FetchStage;

    // A one-hot instruction must round-trip one-hot through both the async read
    // (RawInstr) and the registered output (Instruction) — no bit-lane
    // cross-wiring across the program/memory/register path.
    f.program(0x04, 1 << bitPosition);
    f.setPC(0x04);
    QCOMPARE(f.readRaw(), 1 << bitPosition);

    f.latch();
    QCOMPARE(f.readInstr(), 1 << bitPosition);
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
    // Inputs: 8 PCData + Clock + Reset + PCLoad + PCInc + InstrLoad + 8 ProgAddr + 8 ProgData + ProgWrite = 30
    // Outputs: 8 PC + 8 Instruction + 5 OpCode + 3 RegisterAddr = 24 outputs
    QCOMPARE(fetch->inputSize(), 30);   // 8 PCData + 5 control + 8 ProgAddr + 8 ProgData + 1 ProgWrite
    QCOMPARE(fetch->outputSize(), 32);  // 8 PC + 8 Instruction + 5 OpCode + 3 RegisterAddr + 8 RawInstr

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
