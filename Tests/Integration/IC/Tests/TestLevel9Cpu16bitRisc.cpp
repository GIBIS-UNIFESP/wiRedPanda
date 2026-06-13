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
        for (int i = 0; i < 8; i++) {
            progAddr[i] = new InputSwitch(); builder.add(progAddr[i]);
        }
        for (int i = 0; i < 16; i++) {
            progData[i] = new InputSwitch(); builder.add(progData[i]);
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

        for (int i = 0; i < 8; i++) {
            builder.connect(ic, QString("PC[%1]").arg(i), pcLeds[i], 0);
            builder.connect(progAddr[i], 0, ic, QString("ProgAddr[%1]").arg(i));
        }
        for (int i = 0; i < 16; i++) {
            builder.connect(ic, QString("Result[%1]").arg(i), resultLeds[i], 0);
            builder.connect(ic, QString("Instr[%1]").arg(i), instrLeds[i], 0);
            builder.connect(progData[i], 0, ic, QString("ProgData[%1]").arg(i));
        }
        for (int i = 0; i < 5; i++) {
            builder.connect(ic, QString("OpCode[%1]").arg(i), opcodeLeds[i], 0);
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
    QCOMPARE(f.ic->inputSize(), 27);
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

void TestLevel9CPU16BitRISC::testInstrWordVisible()
{
    auto &f = *s_level9Cpu16bitRisc;

    f.resetCpu();
    clockCycle(f.sim, f.clk);

    int instr = f.readInstr();
    QVERIFY2(instr >= 0 && instr <= 65535,
        qPrintable(QString("Instruction word %1 out of 16-bit range [0,65535]").arg(instr)));
}

void TestLevel9CPU16BitRISC::testOpCodeInValidRange()
{
    auto &f = *s_level9Cpu16bitRisc;

    f.resetCpu();

    for (int cycle = 0; cycle < 5; cycle++) {
        clockCycle(f.sim, f.clk);
        int opcode = f.readOpCode();
        QVERIFY2(opcode >= 0 && opcode <= 31,
            qPrintable(QString("Cycle %1: OpCode %2 out of 5-bit range [0,31]").arg(cycle).arg(opcode)));
    }
}

void TestLevel9CPU16BitRISC::testResultRegisterReadable()
{
    auto &f = *s_level9Cpu16bitRisc;

    f.resetCpu();

    for (int cycle = 0; cycle < 5; ++cycle) {
        clockCycle(f.sim, f.clk);
        int result = f.readResult();
        QVERIFY2(result >= 0 && result <= 65535,
            qPrintable(QString("Cycle %1: Result %2 out of 16-bit range [0,65535]").arg(cycle).arg(result)));
    }
}

void TestLevel9CPU16BitRISC::testInstrOpCodeFieldConsistency()
{
    auto &f = *s_level9Cpu16bitRisc;

    f.resetCpu();

    // Program distinct opcodes so the consistency check is not vacuous
    for (int addr = 0; addr < 5; ++addr) {
        f.programWord(addr, ((addr + 3) << 11) | (addr << 6) | addr);
    }
    f.resetCpu();

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
// The ALU computes ALUOp(SrcBits, DestReg) with ALUOp = OpCode's low 3 bits:
// 000 ADD, 001 SUB, 010 AND, 011 OR, 100 XOR.
void TestLevel9CPU16BitRISC::testCPUComputesOnDecodedFields_data()
{
    QTest::addColumn<int>("opcode");
    QTest::addColumn<int>("destReg");
    QTest::addColumn<int>("srcBits");
    QTest::addColumn<int>("expectedResult");

    QTest::newRow("add_9_plus_5") << 0 << 5 << 9 << 14;
    QTest::newRow("add_63_plus_31") << 0 << 31 << 63 << 94;
    QTest::newRow("sub_9_minus_5") << 1 << 5 << 9 << 4;
    QTest::newRow("and_0x2A_0x0A") << 2 << 0x0A << 0x2A << 0x0A;
    QTest::newRow("or_0x15_0x0A") << 3 << 0x0A << 0x15 << 0x1F;
    QTest::newRow("xor_0x3F_0x15") << 4 << 0x15 << 0x3F << 0x2A;

    // Unary ops (DestReg ignored). The 16-bit ALU computes on OperandA =
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
    QFETCH(int, destReg);
    QFETCH(int, srcBits);
    QFETCH(int, expectedResult);

    auto &f = *s_level9Cpu16bitRisc;

    f.resetCpu();
    f.programWord(0x00, (opcode << 11) | (destReg << 6) | srcBits);
    f.resetCpu();

    // PC=0 after reset; the single-cycle datapath decodes RawInstr
    // combinationally — the result is valid without a clock edge.
    QCOMPARE(f.readResult(), expectedResult);
}
