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
    QCOMPARE(f.ic->inputSize(), 2);
    QCOMPARE(f.ic->outputSize(), 26);
}

void TestLevel9MultiCycleCPU8Bit::testCycleCounter_data()
{
    QTest::addColumn<int>("cycles");
    QTest::addColumn<int>("expectedCounter");
    QTest::addColumn<int>("expectedPC");

    QTest::newRow("initial (00, PC=0)") << 0 << 0x0 << 0x0;
    QTest::newRow("cycle 1 (01, PC=0)") << 1 << 0x1 << 0x0;
    QTest::newRow("cycle 2 (10, PC=0)") << 2 << 0x2 << 0x0;
    QTest::newRow("cycle 3 (11, PC=0)") << 3 << 0x3 << 0x0;
    QTest::newRow("cycle 4 (00, PC=1)") << 4 << 0x0 << 0x1;
    QTest::newRow("cycle 5 (01, PC=1)") << 5 << 0x1 << 0x1;
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

void TestLevel9MultiCycleCPU8Bit::testInstructionVisibleDuringFetch()
{
    auto &f = *s_level9MultiCycleCpu;

    f.resetCpu();

    for (int i = 0; i < 4; i++) {
        clockCycle(f.sim, f.clk);
    }

    QCOMPARE(f.readCounter(), 0);

    int instr = f.readInstr();
    QVERIFY2(instr >= 0 && instr <= 255,
        qPrintable(QString("Instruction %1 out of 8-bit range [0,255]").arg(instr)));
}

void TestLevel9MultiCycleCPU8Bit::testResultRegisterReadable()
{
    auto &f = *s_level9MultiCycleCpu;

    f.resetCpu();

    for (int cycle = 0; cycle < 8; ++cycle) {
        clockCycle(f.sim, f.clk);

        int counter = f.readCounter();
        if (counter == 0) {
            int result = f.readResult();
            QVERIFY2(result >= 0 && result <= 255,
                qPrintable(QString("Cycle %1: Result %2 out of 8-bit range [0,255]").arg(cycle).arg(result)));
        }
    }
}

void TestLevel9MultiCycleCPU8Bit::testInstructionStabilityAcrossPipelineStages()
{
    auto &f = *s_level9MultiCycleCpu;

    f.resetCpu();

    QCOMPARE(f.readCounter(), 0);

    int instrFetch = f.readInstr();

    for (int stage = 1; stage < 4; ++stage) {
        clockCycle(f.sim, f.clk);

        QCOMPARE(f.readCounter(), stage);

        int instrStage = f.readInstr();
        QCOMPARE(instrStage, instrFetch);
    }
}

