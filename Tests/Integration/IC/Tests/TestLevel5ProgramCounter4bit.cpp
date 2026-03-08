// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel5ProgramCounter4bit.h"

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
using TestUtils::setMultiBitInput;
using CPUTestUtils::loadBuildingBlockIC;

struct ProgramCounter4bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    QVector<InputSwitch *> loadValueInputs;
    InputSwitch *clock = nullptr, *load = nullptr, *inc = nullptr, *reset = nullptr;
    QVector<Led *> pcOutputs;
    QVector<Led *> pcPlus1Outputs;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        for (int i = 0; i < 4; ++i) {
            auto *lv = new InputSwitch();
            builder.add(lv);
            loadValueInputs.append(lv);
        }

        clock = new InputSwitch();
        load = new InputSwitch();
        inc = new InputSwitch();
        reset = new InputSwitch();
        builder.add(clock, load, inc, reset);

        ic = loadBuildingBlockIC("level5_program_counter_4bit.panda");
        builder.add(ic);

        for (int i = 0; i < 4; ++i) {
            auto *p = new Led();
            auto *p1 = new Led();
            builder.add(p, p1);
            pcOutputs.append(p);
            pcPlus1Outputs.append(p1);
        }

        for (int i = 0; i < 4; ++i) {
            builder.connect(loadValueInputs[i], 0, ic, QString("loadValue[%1]").arg(i));
            builder.connect(ic, QString("pc[%1]").arg(i), pcOutputs[i], 0);
            builder.connect(ic, QString("pc_plus_1[%1]").arg(i), pcPlus1Outputs[i], 0);
        }
        builder.connect(clock, 0, ic, "clock");
        builder.connect(load, 0, ic, "load");
        builder.connect(inc, 0, ic, "inc");
        builder.connect(reset, 0, ic, "reset");

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    int readPC() { return readMultiBitOutput(QVector<GraphicElement *>(pcOutputs.begin(), pcOutputs.end()), 0); }
    int readPCPlus1() { return readMultiBitOutput(QVector<GraphicElement *>(pcPlus1Outputs.begin(), pcPlus1Outputs.end()), 0); }
};

static std::unique_ptr<ProgramCounter4bitFixture> s_level5ProgramCounter4bit;

void TestLevel5ProgramCounter4Bit::initTestCase()
{
    s_level5ProgramCounter4bit = std::make_unique<ProgramCounter4bitFixture>();
    QVERIFY(s_level5ProgramCounter4bit->build());
}

void TestLevel5ProgramCounter4Bit::cleanupTestCase()
{
    s_level5ProgramCounter4bit.reset();
}

void TestLevel5ProgramCounter4Bit::cleanup()
{
    if (s_level5ProgramCounter4bit && s_level5ProgramCounter4bit->sim) {
        s_level5ProgramCounter4bit->sim->restart();
        s_level5ProgramCounter4bit->sim->update();
    }
}

void TestLevel5ProgramCounter4Bit::testProgramCounter4Bit_data()
{
    QTest::addColumn<int>("initialValue");
    QTest::addColumn<int>("expectedAfterLoad");
    QTest::addColumn<int>("expectedAfterIncrement");

    // Test loading and incrementing 4-bit values
    QTest::newRow("load 0x0")  << 0x0 << 0x0 << 0x1;
    QTest::newRow("load 0x5")  << 0x5 << 0x5 << 0x6;
    QTest::newRow("load 0xA")  << 0xA << 0xA << 0xB;
    QTest::newRow("load 0xF")  << 0xF << 0xF << 0x0;  // Wraparound: 0xF + 1 = 0x10 & 0x0F = 0x0
}

void TestLevel5ProgramCounter4Bit::testProgramCounter4Bit()
{
    QFETCH(int, initialValue);
    QFETCH(int, expectedAfterLoad);
    QFETCH(int, expectedAfterIncrement);

    auto &f = *s_level5ProgramCounter4bit;

    // Test 1: Load initial value
    setMultiBitInput(f.loadValueInputs, initialValue);
    f.load->setOn(true);
    f.inc->setOn(false);
    f.reset->setOn(false);

    f.sim->update();
    clockCycle(f.sim, f.clock);
    f.sim->update();

    QCOMPARE(f.readPC(), expectedAfterLoad);
    QCOMPARE(f.readPCPlus1(), (expectedAfterLoad + 1) & 0x0F);

    // Test 2: Increment the PC
    f.clock->setOn(false);
    f.load->setOn(false);
    f.inc->setOn(true);

    f.sim->update();
    clockCycle(f.sim, f.clock);
    f.sim->update();

    QCOMPARE(f.readPC(), expectedAfterIncrement);
    QCOMPARE(f.readPCPlus1(), (expectedAfterIncrement + 1) & 0x0F);
}

void TestLevel5ProgramCounter4Bit::testProgramCounter4BitStructure()
{
    auto &f = *s_level5ProgramCounter4bit;

    QVERIFY(f.ic != nullptr);
    QCOMPARE(f.ic->inputSize(), 8);
    QCOMPARE(f.ic->outputSize(), 8);
}

