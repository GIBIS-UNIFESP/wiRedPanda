// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel9MultiCycleCpu8bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/Cpu/Cpu8bitIsa.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::readMultiBitOutput;
using TestUtils::setMultiBitInput;
using TestUtils::clockCycle;
using CPUTestUtils::loadBuildingBlockIC;

struct MultiCycleCpu8bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *clk = nullptr;
    InputSwitch *reset = nullptr;
    InputSwitch *progWrite = nullptr;
    QVector<InputSwitch *> progAddr;
    QVector<InputSwitch *> progData;
    InputSwitch *regProgWrite = nullptr;
    QVector<InputSwitch *> regProgAddr;
    QVector<InputSwitch *> regProgData;
    QVector<Led *> pcLeds;
    QVector<Led *> resultLeds;
    QVector<Led *> counterLeds;
    QVector<Led *> instrLeds;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level9_multi_cycle_cpu_8bit.panda");
        builder.add(ic);

        clk = new InputSwitch(); builder.add(clk);
        reset = new InputSwitch(); builder.add(reset);
        progWrite = new InputSwitch(); builder.add(progWrite);
        regProgWrite = new InputSwitch(); builder.add(regProgWrite);

        for (int i = 0; i < 8; ++i) {
            auto *pa = new InputSwitch(); builder.add(pa); progAddr.append(pa);
            auto *pd = new InputSwitch(); builder.add(pd); progData.append(pd);
            auto *rpd = new InputSwitch(); builder.add(rpd); regProgData.append(rpd);
        }
        for (int i = 0; i < 3; ++i) {
            auto *rpa = new InputSwitch(); builder.add(rpa); regProgAddr.append(rpa);
        }

        for (int i = 0; i < 8; i++) {
            auto *p = new Led(); builder.add(p); pcLeds.append(p);
            auto *r = new Led(); builder.add(r); resultLeds.append(r);
            auto *il = new Led(); builder.add(il); instrLeds.append(il);
        }
        for (int i = 0; i < 2; i++) {
            auto *c = new Led(); builder.add(c); counterLeds.append(c);
        }

        builder.connect(clk, 0, ic, "Clock");
        builder.connect(reset, 0, ic, "Reset");
        builder.connect(progWrite, 0, ic, "ProgWrite");
        builder.connect(regProgWrite, 0, ic, "RegProgWrite");

        for (int i = 0; i < 8; ++i) {
            builder.connect(progAddr[i], 0, ic, QString("ProgAddr[%1]").arg(i));
            builder.connect(progData[i], 0, ic, QString("ProgData[%1]").arg(i));
            builder.connect(regProgData[i], 0, ic, QString("RegProgData[%1]").arg(i));
        }
        for (int i = 0; i < 3; ++i) {
            builder.connect(regProgAddr[i], 0, ic, QString("RegProgAddr[%1]").arg(i));
        }

        for (int i = 0; i < 8; i++) {
            builder.connect(ic, QString("PC[%1]").arg(i), pcLeds[i], 0);
            builder.connect(ic, QString("Result[%1]").arg(i), resultLeds[i], 0);
            builder.connect(ic, QString("Instruction[%1]").arg(i), instrLeds[i], 0);
        }
        for (int i = 0; i < 2; i++) {
            builder.connect(ic, QString("CycleCounter[%1]").arg(i), counterLeds[i], 0);
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    int readPC() { return readMultiBitOutput(QVector<GraphicElement *>(pcLeds.begin(), pcLeds.end()), 0); }
    int readResult() { return readMultiBitOutput(QVector<GraphicElement *>(resultLeds.begin(), resultLeds.end()), 0); }
    int readCounter() { return readMultiBitOutput(QVector<GraphicElement *>(counterLeds.begin(), counterLeds.end()), 0); }
    int readInstr() { return readMultiBitOutput(QVector<GraphicElement *>(instrLeds.begin(), instrLeds.end()), 0); }

    void resetCpu()
    {
        reset->setOn(true);
        sim->update();
        reset->setOn(false);
        sim->update();
    }

    // Hold reset (freezes the cycle counter at phase 0, so the fetch stage's
    // gated clock passes the full clock) while loading program/registers.
    void beginProgramming()
    {
        progWrite->setOn(false);
        regProgWrite->setOn(false);
        reset->setOn(true);
        sim->update();
    }

    void programInstruction(int addr, int instrByte)
    {
        setMultiBitInput(progAddr, addr);
        setMultiBitInput(progData, instrByte);
        progWrite->setOn(true);
        sim->update();
        clockCycle(sim, clk);   // counter frozen at 00 -> fetch gated clock = clock
        progWrite->setOn(false);
        sim->update();
    }

    void programRegister(int regIdx, int value)
    {
        setMultiBitInput(regProgAddr, regIdx);
        setMultiBitInput(regProgData, value);
        regProgWrite->setOn(true);
        sim->update();
        clockCycle(sim, clk);   // regfile writes on the main clock
        regProgWrite->setOn(false);
        sim->update();
    }

    // Release reset and run one full 4-phase instruction starting from phase 0.
    // The ALU result of the instruction at PC is observable once the fetch phase
    // latches the instruction register (one clock edge after release).
    void run()
    {
        reset->setOn(false);
        sim->update();
    }

    // Run a full instruction (4 phases) so its write-back completes.
    void stepInstruction()
    {
        for (int i = 0; i < 4; ++i) {
            clockCycle(sim, clk);
        }
    }
};

static std::unique_ptr<MultiCycleCpu8bitFixture> s_level9MultiCycleCpu;

void TestLevel9MultiCycleCPU8Bit::initTestCase()
{
    s_level9MultiCycleCpu = std::make_unique<MultiCycleCpu8bitFixture>();
    QVERIFY(s_level9MultiCycleCpu->build());
}

void TestLevel9MultiCycleCPU8Bit::cleanupTestCase()
{
    s_level9MultiCycleCpu.reset();
}

void TestLevel9MultiCycleCPU8Bit::cleanup()
{
    if (s_level9MultiCycleCpu && s_level9MultiCycleCpu->sim) {
        s_level9MultiCycleCpu->sim->restart();
        s_level9MultiCycleCpu->sim->update();
    }
}

void TestLevel9MultiCycleCPU8Bit::testCPUStructure()
{
    auto &f = *s_level9MultiCycleCpu;
    QVERIFY(f.ic != nullptr);
    // Clock + Reset + ProgAddr[8] + ProgData[8] + ProgWrite
    //       + RegProgAddr[3] + RegProgData[8] + RegProgWrite = 31 (matches single-cycle)
    QCOMPARE(f.ic->inputSize(), 31);
    // PC[8] + Result[8] + Instruction[8] + CycleCounter[2] = 26
    QCOMPARE(f.ic->outputSize(), 26);
}

void TestLevel9MultiCycleCPU8Bit::testCycleCounter_data()
{
    QTest::addColumn<int>("cycles");
    QTest::addColumn<int>("expectedCounter");
    QTest::addColumn<int>("expectedPC");

    // PC increments at the START of each 4-cycle instruction group (when the
    // cycle counter leaves phase 0); PC = ceil(cycles / 4) under the engine's
    // synchronous semantics. Matches the exported SystemVerilog.
    QTest::newRow("initial (00, PC=0)") << 0 << 0x0 << 0x0;
    QTest::newRow("cycle 1 (01, PC=1)") << 1 << 0x1 << 0x1;
    QTest::newRow("cycle 2 (10, PC=1)") << 2 << 0x2 << 0x1;
    QTest::newRow("cycle 3 (11, PC=1)") << 3 << 0x3 << 0x1;
    QTest::newRow("cycle 4 (00, PC=1)") << 4 << 0x0 << 0x1;
    QTest::newRow("cycle 5 (01, PC=2)") << 5 << 0x1 << 0x2;
    QTest::newRow("after 8 cycles (00, PC=2)") << 8 << 0x0 << 0x2;
}

void TestLevel9MultiCycleCPU8Bit::testCycleCounter()
{
    QFETCH(int, cycles);
    QFETCH(int, expectedCounter);
    QFETCH(int, expectedPC);

    auto &f = *s_level9MultiCycleCpu;

    f.resetCpu();

    for (int i = 0; i < cycles; i++) {
        clockCycle(f.sim, f.clk);
    }

    QCOMPARE(f.readCounter(), expectedCounter);
    QCOMPARE(f.readPC(), expectedPC);
}

void TestLevel9MultiCycleCPU8Bit::testCPUReset()
{
    auto &f = *s_level9MultiCycleCpu;

    f.resetCpu();

    for (int i = 0; i < 6; i++) {
        clockCycle(f.sim, f.clk);
    }

    f.resetCpu();

    QCOMPARE(f.readPC(), 0);
    QCOMPARE(f.readCounter(), 0);
}

void TestLevel9MultiCycleCPU8Bit::testPipelineStageSequence()
{
    auto &f = *s_level9MultiCycleCpu;

    f.resetCpu();

    for (int cycle = 0; cycle < 16; cycle++) {
        QCOMPARE(f.readCounter(), cycle % 4);
        clockCycle(f.sim, f.clk);
    }
}

// Program a known instruction and confirm the fetch stage actually fetches it
// (previously a vacuous range check on blank memory).
void TestLevel9MultiCycleCPU8Bit::testInstructionVisibleDuringFetch()
{
    auto &f = *s_level9MultiCycleCpu;

    const int instr = encodeInstruction(ADD, 1);
    f.beginProgramming();
    f.programInstruction(0x00, instr);
    f.run();

    // Phase 0: the fetched word is available; latch it into the IR
    clockCycle(f.sim, f.clk);
    QCOMPARE(f.readInstr(), instr);
}

// Program operands + an ADD and confirm the deterministic computed result
// (previously a vacuous range check on the blank-memory result).
void TestLevel9MultiCycleCPU8Bit::testResultRegisterReadable()
{
    auto &f = *s_level9MultiCycleCpu;

    f.beginProgramming();
    f.programRegister(0, 0x12);   // R0 (accumulator)
    f.programRegister(1, 0x34);   // R1
    f.programInstruction(0x00, encodeInstruction(ADD, 1));
    f.run();

    clockCycle(f.sim, f.clk);     // latch IR -> decode/execute combinational
    QCOMPARE(f.readResult(), 0x46);
}

void TestLevel9MultiCycleCPU8Bit::testInstructionStabilityAcrossPipelineStages()
{
    auto &f = *s_level9MultiCycleCpu;

    // Program a known instruction so the stability check is meaningful
    f.beginProgramming();
    f.programInstruction(0x00, encodeInstruction(XOR, 2));
    f.run();

    // Latch the instruction in the fetch phase, then hold it across decode/
    // execute/memory phases of the same instruction.
    clockCycle(f.sim, f.clk);
    int instrFetched = f.readInstr();

    for (int stage = 1; stage < 4; ++stage) {
        QCOMPARE(f.readCounter(), stage);
        int instrStage = f.readInstr();
        QCOMPARE(instrStage, instrFetched);
        clockCycle(f.sim, f.clk);
    }
}

// Full ISA: program R0/R1 and one ALU instruction, run it, and verify the
// computed result. The accumulator model is identical to the single-cycle CPU
// (OperandA=R0, OperandB=R1).
void TestLevel9MultiCycleCPU8Bit::testALUExecution_data()
{
    QTest::addColumn<int>("aluOp");
    QTest::addColumn<int>("operandA");
    QTest::addColumn<int>("operandB");
    QTest::addColumn<int>("expectedResult");

    QTest::newRow("ADD 3+5=8")        << static_cast<int>(ADD) << 3    << 5    << 8;
    QTest::newRow("ADD 0xFF+1=0x00")  << static_cast<int>(ADD) << 0xFF << 0x01 << 0x00;
    QTest::newRow("SUB 9-5=4")        << static_cast<int>(SUB) << 9    << 5    << 4;
    QTest::newRow("SUB 0-1=0xFF")     << static_cast<int>(SUB) << 0    << 1    << 0xFF;
    QTest::newRow("AND 0xF0&0x0F=0")  << static_cast<int>(AND) << 0xF0 << 0x0F << 0x00;
    QTest::newRow("AND 0xFF&0x42")    << static_cast<int>(AND) << 0xFF << 0x42 << 0x42;
    QTest::newRow("OR 0xF0|0x0F=0xFF") << static_cast<int>(OR) << 0xF0 << 0x0F << 0xFF;
    QTest::newRow("XOR 0xAA^0x55=0xFF") << static_cast<int>(XOR) << 0xAA << 0x55 << 0xFF;
    QTest::newRow("XOR N^N=0")        << static_cast<int>(XOR) << 0x42 << 0x42 << 0x00;
    QTest::newRow("NOT ~0x42=0xBD")   << static_cast<int>(NOT) << 0x42 << 0x00 << 0xBD;
    QTest::newRow("NOT ~0x00=0xFF")   << static_cast<int>(NOT) << 0x00 << 0x00 << 0xFF;
    QTest::newRow("SHL 0x42<<1=0x84") << static_cast<int>(SHL) << 0x42 << 0x00 << 0x84;
    QTest::newRow("SHL 0x80<<1=0x00") << static_cast<int>(SHL) << 0x80 << 0x00 << 0x00;
    QTest::newRow("SHR 0x84>>1=0x42") << static_cast<int>(SHR) << 0x84 << 0x00 << 0x42;
    QTest::newRow("SHR 0x01>>1=0x00") << static_cast<int>(SHR) << 0x01 << 0x00 << 0x00;
}

void TestLevel9MultiCycleCPU8Bit::testALUExecution()
{
    QFETCH(int, aluOp);
    QFETCH(int, operandA);
    QFETCH(int, operandB);
    QFETCH(int, expectedResult);

    auto &f = *s_level9MultiCycleCpu;

    f.beginProgramming();
    f.programRegister(1, operandB);
    f.programRegister(0, operandA);
    f.programInstruction(0x00, encodeInstruction(aluOp, 1));
    f.run();

    clockCycle(f.sim, f.clk);     // latch IR; ALU result is combinational
    QCOMPARE(f.readResult(), expectedResult);
}

// Drive an ALU operand from a high register (R3/R5/R6/R7) so the high
// register-address bit (Instruction[2] -> register file Read_Addr2[2]) is
// exercised — the ISA test above only ever addresses R1.
void TestLevel9MultiCycleCPU8Bit::testHighRegisterOperand()
{
    auto &f = *s_level9MultiCycleCpu;

    const int regs[] = {3, 5, 6, 7};
    for (int reg : regs) {
        f.sim->restart();
        f.sim->update();

        const int operandB = 0x10 + reg;
        f.beginProgramming();
        f.programRegister(reg, operandB);
        f.programRegister(0, 0x10);
        f.programInstruction(0x00, encodeInstruction(ADD, reg));
        f.run();

        clockCycle(f.sim, f.clk);   // latch IR; ALU result combinational
        QVERIFY2(f.readResult() == (0x10 + operandB),
            qPrintable(QString("ADD R%1: expected 0x%2, got 0x%3")
                .arg(reg).arg(0x10 + operandB, 0, 16).arg(f.readResult(), 0, 16)));
    }
}

// A 3-instruction program threading the accumulator through the register
// write-back path: R0 = ((R0+R1)-R2) & R1.
void TestLevel9MultiCycleCPU8Bit::testMultipleInstructions()
{
    auto &f = *s_level9MultiCycleCpu;

    f.beginProgramming();
    f.programRegister(0, 0x10);   // R0 accumulator
    f.programRegister(1, 0x20);   // R1
    f.programRegister(2, 0x05);   // R2
    f.programInstruction(0, encodeInstruction(ADD, 1));  // R0 = 0x10 + 0x20 = 0x30
    f.programInstruction(1, encodeInstruction(SUB, 2));  // R0 = 0x30 - 0x05 = 0x2B
    f.programInstruction(2, encodeInstruction(AND, 1));  // R0 = 0x2B & 0x20 = 0x20
    f.run();

    // Check each instruction's result before ITS OWN write-back lands (Execute is
    // combinational, so the live Result reflects exactly that instruction's inputs
    // only up until the phase-3 edge writes back and the next instruction's fetch
    // takes over) -- this is what actually exercises the write-back-to-next-read
    // path (ADD's write-back feeding SUB's operand, SUB's feeding AND's), rather
    // than only checking the final value: an AND against a single set bit (0x20)
    // would just as well "pass" on a wrong upstream result that happens to keep
    // that bit set (e.g. a miscomputed ADD of 0x31 still yields SUB=0x2C, and
    // 0x2C & 0x20 == 0x20 too), so checking only the end result has little power
    // to catch a broken intermediate step.
    clockCycle(f.sim, f.clk);     // phase 0->1: IR loads ADD R1, ALU computes live
    QCOMPARE(f.readResult(), 0x30);
    clockCycle(f.sim, f.clk);     // phase 1->2
    clockCycle(f.sim, f.clk);     // phase 2->3
    clockCycle(f.sim, f.clk);     // phase 3->0: writes R0=0x30 back; IR still holds ADD
    clockCycle(f.sim, f.clk);     // phase 0->1: IR loads SUB R2, ALU computes live off the new R0

    QCOMPARE(f.readResult(), 0x2B);
    clockCycle(f.sim, f.clk);
    clockCycle(f.sim, f.clk);
    clockCycle(f.sim, f.clk);     // phase 3->0: writes R0=0x2B back; IR still holds SUB
    clockCycle(f.sim, f.clk);     // phase 0->1: IR loads AND R1, ALU computes live off the new R0

    QCOMPARE(f.readResult(), 0x20);
}

// STORE the accumulator to data memory, then LOAD it back.
void TestLevel9MultiCycleCPU8Bit::testStoreLoad()
{
    auto &f = *s_level9MultiCycleCpu;

    f.beginProgramming();
    f.programRegister(0, 0xAB);
    f.programInstruction(0, encodeStore(3));   // mem[3] = R0 = 0xAB
    f.programInstruction(1, encodeLoad(3));    // Result = mem[3]
    f.run();

    f.stepInstruction();          // execute STORE
    f.stepInstruction();          // execute LOAD
    QCOMPARE(f.readResult(), 0xAB);
}
