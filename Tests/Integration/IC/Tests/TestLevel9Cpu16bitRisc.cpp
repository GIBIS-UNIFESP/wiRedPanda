// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel9Cpu16bitRisc.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::readMultiBitOutput;
using TestUtils::setMultiBitInput;
using TestUtils::clockCycle;
using CPUTestUtils::loadBuildingBlockIC;

struct Cpu16bitRiscFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *clk = nullptr;
    InputSwitch *reset = nullptr;
    InputSwitch *progAddr[8] = {};
    InputSwitch *progData[16] = {};
    InputSwitch *progWrite = nullptr;
    InputSwitch *regProgAddr[5] = {};
    InputSwitch *regProgData[16] = {};
    InputSwitch *regProgWrite = nullptr;
    QVector<Led *> pcLeds;
    QVector<Led *> resultLeds;
    QVector<Led *> instrLeds;
    QVector<Led *> opcodeLeds;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level9_cpu_16bit_risc.panda");
        builder.add(ic);

        clk = new InputSwitch(); builder.add(clk);
        reset = new InputSwitch(); builder.add(reset);
        progWrite = new InputSwitch(); builder.add(progWrite);
        regProgWrite = new InputSwitch(); builder.add(regProgWrite);
        for (int i = 0; i < 8; i++) {
            progAddr[i] = new InputSwitch(); builder.add(progAddr[i]);
        }
        for (int i = 0; i < 16; i++) {
            progData[i] = new InputSwitch(); builder.add(progData[i]);
        }
        for (int i = 0; i < 5; i++) {
            regProgAddr[i] = new InputSwitch(); builder.add(regProgAddr[i]);
        }
        for (int i = 0; i < 16; i++) {
            regProgData[i] = new InputSwitch(); builder.add(regProgData[i]);
        }

        for (int i = 0; i < 8; i++) {
            auto *p = new Led(); builder.add(p); pcLeds.append(p);
        }
        for (int i = 0; i < 16; i++) {
            auto *r = new Led(); builder.add(r); resultLeds.append(r);
            auto *il = new Led(); builder.add(il); instrLeds.append(il);
        }
        for (int i = 0; i < 5; i++) {
            auto *o = new Led(); builder.add(o); opcodeLeds.append(o);
        }

        builder.connect(clk, 0, ic, "Clock");
        builder.connect(reset, 0, ic, "Reset");
        builder.connect(progWrite, 0, ic, "ProgWrite");
        builder.connect(regProgWrite, 0, ic, "RegProgWrite");

        for (int i = 0; i < 8; i++) {
            builder.connect(ic, QString("PC[%1]").arg(i), pcLeds[i], 0);
            builder.connect(progAddr[i], 0, ic, QString("ProgAddr[%1]").arg(i));
        }
        for (int i = 0; i < 16; i++) {
            builder.connect(ic, QString("Result[%1]").arg(i), resultLeds[i], 0);
            builder.connect(ic, QString("Instruction[%1]").arg(i), instrLeds[i], 0);
            builder.connect(progData[i], 0, ic, QString("ProgData[%1]").arg(i));
            builder.connect(regProgData[i], 0, ic, QString("RegProgData[%1]").arg(i));
        }
        for (int i = 0; i < 5; i++) {
            builder.connect(ic, QString("OpCode[%1]").arg(i), opcodeLeds[i], 0);
            builder.connect(regProgAddr[i], 0, ic, QString("RegProgAddr[%1]").arg(i));
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    int readPC() { return readMultiBitOutput(QVector<GraphicElement *>(pcLeds.begin(), pcLeds.end()), 0); }
    int readResult() { return readMultiBitOutput(QVector<GraphicElement *>(resultLeds.begin(), resultLeds.end()), 0); }
    int readInstr() { return readMultiBitOutput(QVector<GraphicElement *>(instrLeds.begin(), instrLeds.end()), 0); }
    int readOpCode() { return readMultiBitOutput(QVector<GraphicElement *>(opcodeLeds.begin(), opcodeLeds.end()), 0); }

    void resetCpu()
    {
        reset->setOn(true);
        sim->update();
        reset->setOn(false);
        sim->update();
    }

    // Hold reset while programming instruction memory. PCInc/InstrLoad are
    // unconditionally live (tied to Vcc in the generator, no phase counter to gate
    // them), so without an asserted Reset each clock pulse used to write a word
    // would also advance the fetch PC; Reset's async override on the PC/instruction
    // register keeps them pinned at 0 for the whole programming window (same
    // mechanism the multi-cycle CPU documents for its own phase counter). Call
    // run() to release Reset and start execution from PC=0.
    void beginProgramming()
    {
        progWrite->setOn(false);
        reset->setOn(true);
        sim->update();
    }

    // Release reset after beginProgramming() and start execution from PC=0.
    void run()
    {
        reset->setOn(false);
        sim->update();
    }

    // Write a 16-bit instruction word at the given program memory address
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

    // Write a 16-bit value into the register file at the given index (0..31).
    void programRegister(int regIdx, int value)
    {
        for (int i = 0; i < 5; ++i) {
            regProgAddr[i]->setOn((regIdx >> i) & 1);
        }
        for (int i = 0; i < 16; ++i) {
            regProgData[i]->setOn((value >> i) & 1);
        }
        regProgWrite->setOn(true);
        sim->update();
        clockCycle(sim, clk);
        regProgWrite->setOn(false);
        sim->update();
    }
};

static std::unique_ptr<Cpu16bitRiscFixture> s_level9Cpu16bitRisc;

void TestLevel9CPU16BitRISC::initTestCase()
{
    s_level9Cpu16bitRisc = std::make_unique<Cpu16bitRiscFixture>();
    QVERIFY(s_level9Cpu16bitRisc->build());
}

void TestLevel9CPU16BitRISC::cleanupTestCase()
{
    s_level9Cpu16bitRisc.reset();
}

void TestLevel9CPU16BitRISC::cleanup()
{
    if (s_level9Cpu16bitRisc && s_level9Cpu16bitRisc->sim) {
        s_level9Cpu16bitRisc->sim->restart();
        s_level9Cpu16bitRisc->sim->update();
    }
}

void TestLevel9CPU16BitRISC::testCPUStructure()
{
    auto &f = *s_level9Cpu16bitRisc;
    QVERIFY(f.ic != nullptr);
    // Clock + Reset + ProgAddr[8] + ProgData[16] + ProgWrite (F53)
    //   + RegProgAddr[5] + RegProgData[16] + RegProgWrite (real register file)
    QCOMPARE(f.ic->inputSize(), 49);
    // PC[8] + Result[16] + Instruction[16] + OpCode[5]
    QCOMPARE(f.ic->outputSize(), 45);
}

void TestLevel9CPU16BitRISC::testCPUExecution_data()
{
    QTest::addColumn<int>("cycles");
    QTest::addColumn<int>("expectedPC");

    QTest::newRow("initial state") << 0 << 0x00;
    QTest::newRow("after 1 cycle") << 1 << 0x01;
    QTest::newRow("after 2 cycles") << 2 << 0x02;
    QTest::newRow("after 3 cycles") << 3 << 0x03;
    QTest::newRow("after 5 cycles") << 5 << 0x05;
    QTest::newRow("after 10 cycles") << 10 << 0x0A;
}

void TestLevel9CPU16BitRISC::testCPUExecution()
{
    QFETCH(int, cycles);
    QFETCH(int, expectedPC);

    auto &f = *s_level9Cpu16bitRisc;

    f.resetCpu();

    for (int i = 0; i < cycles; i++) {
        clockCycle(f.sim, f.clk);
    }

    QCOMPARE(f.readPC(), expectedPC);
}

void TestLevel9CPU16BitRISC::testCPUReset()
{
    auto &f = *s_level9Cpu16bitRisc;

    f.resetCpu();

    for (int i = 0; i < 5; i++) {
        clockCycle(f.sim, f.clk);
    }
    QCOMPARE(f.readPC(), 5);

    f.resetCpu();
    QCOMPARE(f.readPC(), 0);
}

// PCInc is unconditionally tied to Vcc in the generator (no phase counter to gate
// it like the multi-cycle 8-bit CPU has), so the fetch PC only stays at 0 across a
// multi-word programming sequence because beginProgramming()'s asserted Reset
// overrides PCInc on every one of those clock pulses. Confirms the fix for the
// PC-drift-during-programming finding.
void TestLevel9CPU16BitRISC::testPCDoesNotDriftDuringProgramming()
{
    auto &f = *s_level9Cpu16bitRisc;

    f.beginProgramming();
    for (int addr = 0; addr < 4; ++addr) {
        f.programWord(addr, ((addr + 1) << 11) | (addr << 6) | addr);
    }
    f.run();

    QCOMPARE(f.readPC(), 0);
}

void TestLevel9CPU16BitRISC::testInstrWordVisible()
{
    auto &f = *s_level9Cpu16bitRisc;

    // Program a distinctive, byte-asymmetric instruction word and confirm the
    // registered Instruction output reflects exactly what was stored (this also
    // exercises the two-byte-lane instruction memory: low byte 0x34, high 0x12).
    // No other test checks readInstr() against a programmed word.
    f.beginProgramming();
    f.programWord(0x00, 0x1234);
    f.run();
    clockCycle(f.sim, f.clk);   // latch imem[0] into the instruction register

    QCOMPARE(f.readInstr(), 0x1234);
}

void TestLevel9CPU16BitRISC::testOpCodeInValidRange()
{
    auto &f = *s_level9Cpu16bitRisc;

    // Program a real instruction whose OpCode field is the maximum 5-bit value
    // and confirm the decoded OpCode output matches it.
    constexpr int OPCODE = 0x1F;
    f.beginProgramming();
    f.programWord(0x00, (OPCODE << 11) | (0x0A << 6) | 0x15);
    f.run();
    clockCycle(f.sim, f.clk);   // OpCode is decoded from the registered instruction

    QCOMPARE(f.readOpCode(), OPCODE);
}

void TestLevel9CPU16BitRISC::testResultRegisterReadable()
{
    auto &f = *s_level9Cpu16bitRisc;

    // Program a real SUB instruction and confirm the ALU result output:
    // OperandA = SrcBits = 0x20, OperandB = Reg[5] = 0x05 (DestReg addresses
    // the register file now), SUB -> 0x1B.
    f.beginProgramming();
    f.programRegister(5, 0x05);
    f.programWord(0x00, (1 << 11) | (0x05 << 6) | 0x20);
    f.run();

    // Single-cycle decode is combinational from RawInstr — valid without a clock.
    QCOMPARE(f.readResult(), 0x1B);
}

void TestLevel9CPU16BitRISC::testInstrOpCodeFieldConsistency()
{
    auto &f = *s_level9Cpu16bitRisc;

    f.beginProgramming();

    // Program distinct opcodes so the consistency check is not vacuous
    for (int addr = 0; addr < 5; ++addr) {
        f.programWord(addr, ((addr + 3) << 11) | (addr << 6) | addr);
    }
    f.run();

    for (int cycle = 0; cycle < 5; ++cycle) {
        clockCycle(f.sim, f.clk);

        int instrWord = f.readInstr();
        int opcode = f.readOpCode();

        int expectedOpCode = (instrWord >> 11) & 0x1F;
        QCOMPARE(opcode, expectedOpCode);
    }
}

// Program real instructions and verify the ALU computes on the decoded
// fields (F53: the fields used to reach the ALU bit-reversed, and with no
// programming interface this was untestable — memory was always blank).
// Word layout: [15:11]=OpCode, [10:6]=DestReg, [5:0]=SrcBits.
// The ALU computes ALUOp(SrcBits, Reg[DestReg]) with ALUOp = OpCode's low 3
// bits: 000 ADD, 001 SUB, 010 AND, 011 OR, 100 XOR. DestReg addresses the
// register file now (it used to be a raw zero-extended ALU operand) — every
// row pre-loads register R1 with the value the old "destReg" column meant as
// an operand, and the instruction's DestReg field is fixed at 1, so the
// expectedResult values below are unchanged from before the register-file
// rework.
void TestLevel9CPU16BitRISC::testCPUComputesOnDecodedFields_data()
{
    QTest::addColumn<int>("opcode");
    QTest::addColumn<int>("operandB");
    QTest::addColumn<int>("srcBits");
    QTest::addColumn<int>("expectedResult");

    QTest::newRow("add_9_plus_5") << 0 << 5 << 9 << 14;
    QTest::newRow("add_63_plus_31") << 0 << 31 << 63 << 94;
    QTest::newRow("sub_9_minus_5") << 1 << 5 << 9 << 4;
    QTest::newRow("and_0x2A_0x0A") << 2 << 0x0A << 0x2A << 0x0A;
    QTest::newRow("or_0x15_0x0A") << 3 << 0x0A << 0x15 << 0x1F;
    QTest::newRow("xor_0x3F_0x15") << 4 << 0x15 << 0x3F << 0x2A;

    // Unary ops (Reg[DestReg] ignored). The 16-bit ALU computes on OperandA =
    // zero-extended SrcBits: NOT is the full 16-bit complement; SHL/SHR fill
    // with 0 (verified against the level-7 ALU semantics).
    QTest::newRow("not_0x3F") << 5 << 0 << 0x3F << 0xFFC0;
    QTest::newRow("not_0x00") << 5 << 0 << 0x00 << 0xFFFF;
    QTest::newRow("shl_0x3F") << 6 << 0 << 0x3F << 0x7E;
    QTest::newRow("shl_0x01") << 6 << 0 << 0x01 << 0x02;
    QTest::newRow("shr_0x3E") << 7 << 0 << 0x3E << 0x1F;
    QTest::newRow("shr_0x08") << 7 << 0 << 0x08 << 0x04;
}

void TestLevel9CPU16BitRISC::testCPUComputesOnDecodedFields()
{
    QFETCH(int, opcode);
    QFETCH(int, operandB);
    QFETCH(int, srcBits);
    QFETCH(int, expectedResult);

    auto &f = *s_level9Cpu16bitRisc;

    constexpr int destReg = 1;

    f.beginProgramming();
    f.programRegister(destReg, operandB);
    f.programWord(0x00, (opcode << 11) | (destReg << 6) | srcBits);
    f.run();

    // PC=0 after reset; the single-cycle datapath decodes RawInstr
    // combinationally — the result is valid without a clock edge.
    QCOMPARE(f.readResult(), expectedResult);
}

// The two previously-dead OpCode high bits ([4:3]) now select instruction
// class (level8_decode_stage's existing MemRead/MemWrite derivation): 00=ALU
// (tested above), 10=Load, 11=Store. Setting them to a non-ALU class must
// stop the ALU's low-3-bits op from reaching Result and switch to memory
// behavior instead — the inverse of what the old dead-bits test checked.
static int encodeMemInstr(bool isStore, int destReg, int srcBits)
{
    const int opClass = isStore ? 0x18 : 0x10;  // OpCode[4:3] = 11 (Store) or 10 (Load)
    return (opClass << 11) | (destReg << 6) | srcBits;
}

void TestLevel9CPU16BitRISC::testISA_Store()
{
    auto &f = *s_level9Cpu16bitRisc;

    f.beginProgramming();
    f.programRegister(3, 0xABCD);
    // Store Reg[3] to mem[5] (SrcBits low 3 bits alias modulo 8) at addr 0,
    // then a Load from the same address at addr 1 — both written up front,
    // inside the same held-Reset programming window (calling programWord()
    // again after run() would re-trigger the PC/address-mux-drift problem
    // testPCDoesNotDriftDuringProgramming guards against, just mid-execution
    // instead of at the very start).
    f.programWord(0x00, encodeMemInstr(true, 3, 5));
    f.programWord(0x01, encodeMemInstr(false, 4, 5));  // Load mem[5] -> Reg[4]
    f.run();

    // Store's MemRead=0, so Result live-shows the ALU passthrough, not the
    // stored value — only a subsequent Load can observe what was written.
    clockCycle(f.sim, f.clk);  // execute the Store (mem[5]=0xABCD), advance PC to 1

    // PC=1: the Load instruction is now live; its combinational read of
    // mem[5] reflects the value the Store just committed on the prior edge.
    QCOMPARE(f.readResult(), 0xABCD);
}

void TestLevel9CPU16BitRISC::testISA_Load()
{
    auto &f = *s_level9Cpu16bitRisc;

    // Data memory starts at all zeros; Load from any address should give 0.
    f.beginProgramming();
    f.programWord(0x00, encodeMemInstr(false, 1, 2));
    f.run();

    QCOMPARE(f.readResult(), 0x0000);
}

// The fix for the "Load never persists anywhere" gap found during review:
// level8_decode_stage's own RegWrite output excludes Load (it's NOT(OpCode[4]),
// true for ALU ops only), so this CPU derives its own write-back enable —
// NOT(MemWrite) — instead. Confirms Load actually writes Reg[DestReg].
void TestLevel9CPU16BitRISC::testLoadWritesBackToRegister()
{
    auto &f = *s_level9Cpu16bitRisc;

    f.beginProgramming();
    f.programRegister(6, 0x1357);
    f.programWord(0x00, encodeMemInstr(true, 6, 1));   // mem[1] = Reg[6] = 0x1357
    f.programWord(0x01, encodeMemInstr(false, 7, 1));  // Reg[7] = mem[1]
    // ADD SrcBits=0 with DestReg=7 reads Reg[7] combinationally as OperandB
    // (OperandA=0), so its live Result equals whatever Reg[7] holds.
    f.programWord(0x02, (0 << 11) | (7 << 6) | 0);
    f.run();

    clockCycle(f.sim, f.clk);  // PC 0->1: execute the Store (mem[1] = 0x1357)
    clockCycle(f.sim, f.clk);  // PC 1->2: execute the Load (Reg[7] <= mem[1]); ADD now live

    QCOMPARE(f.readResult(), 0x1357);
}

void TestLevel9CPU16BitRISC::testStoreDoesNotWriteBackToRegister()
{
    auto &f = *s_level9Cpu16bitRisc;

    f.beginProgramming();
    f.programRegister(2, 0x2222);
    f.programWord(0x00, encodeMemInstr(true, 2, 3));  // Store Reg[2] to mem[3]
    // ADD SrcBits=0 with DestReg=2 reads Reg[2] back; if Store had clobbered
    // it, this would no longer read 0x2222.
    f.programWord(0x01, (0 << 11) | (2 << 6) | 0);
    f.run();

    clockCycle(f.sim, f.clk);  // advance PC to 1, execute the Store

    QCOMPARE(f.readResult(), 0x2222);
}
