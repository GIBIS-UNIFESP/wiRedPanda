// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel9Cpu16bitRisc.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
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

        for (int i = 0; i < 8; i++) {
            builder.connect(ic, QString("PC[%1]").arg(i), pcLeds[i], 0);
        }
        for (int i = 0; i < 16; i++) {
            builder.connect(ic, QString("Result[%1]").arg(i), resultLeds[i], 0);
            builder.connect(ic, QString("Instr[%1]").arg(i), instrLeds[i], 0);
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
    QCOMPARE(f.ic->inputSize(), 2);
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

    for (int cycle = 0; cycle < 5; ++cycle) {
        clockCycle(f.sim, f.clk);

        int instrWord = f.readInstr();
        int opcode = f.readOpCode();

        int expectedOpCode = (instrWord >> 11) & 0x1F;
        QCOMPARE(opcode, expectedOpCode);
    }
}

