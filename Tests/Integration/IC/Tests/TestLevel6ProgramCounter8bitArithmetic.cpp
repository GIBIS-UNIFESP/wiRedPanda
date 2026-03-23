// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel6ProgramCounter8bitArithmetic.h"

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
using TestUtils::setMultiBitInput;
using CPUTestUtils::loadBuildingBlockIC;

struct ProgramCounter8bitArithFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    QVector<InputSwitch *> dataInputs;
    InputSwitch *clock = nullptr;
    InputSwitch *load = nullptr;
    InputSwitch *inc = nullptr;
    InputSwitch *reset = nullptr;
    QVector<Led *> pcOutputs;
    QVector<Led *> pcPlus1Outputs;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level6_program_counter_8bit_arithmetic.panda");
        builder.add(ic);

        for (int i = 0; i < 8; i++) {
            auto *data = new InputSwitch();
            builder.add(data);
            data->setLabel(QString("Data[%1]").arg(i));
            dataInputs.append(data);
        }

        clock = new InputSwitch();
        builder.add(clock);
        clock->setLabel("Clock");

        load = new InputSwitch();
        builder.add(load);
        load->setLabel("Load");

        inc = new InputSwitch();
        builder.add(inc);
        inc->setLabel("Inc");

        reset = new InputSwitch();
        builder.add(reset);
        reset->setLabel("Reset");

        for (int i = 0; i < 8; i++) {
            auto *p = new Led();
            builder.add(p);
            p->setLabel(QString("pc[%1]").arg(i));
            pcOutputs.append(p);

            auto *p1 = new Led();
            builder.add(p1);
            p1->setLabel(QString("pc_plus_1[%1]").arg(i));
            pcPlus1Outputs.append(p1);
        }

        for (int i = 0; i < 8; i++) {
            builder.connect(dataInputs[i], 0, ic, QString("loadValue[%1]").arg(i));
        }
        builder.connect(clock, 0, ic, "clock");
        builder.connect(load, 0, ic, "load");
        builder.connect(inc, 0, ic, "inc");
        builder.connect(reset, 0, ic, "reset");

        for (int i = 0; i < 8; i++) {
            builder.connect(ic, QString("pc[%1]").arg(i), pcOutputs[i], 0);
            builder.connect(ic, QString("pc_plus_1[%1]").arg(i), pcPlus1Outputs[i], 0);
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    int readPC()
    {
        return readMultiBitOutput(QVector<GraphicElement *>(pcOutputs.begin(), pcOutputs.end()), 0);
    }

    int readPCPlus1()
    {
        return readMultiBitOutput(QVector<GraphicElement *>(pcPlus1Outputs.begin(), pcPlus1Outputs.end()), 0);
    }
};

static std::unique_ptr<ProgramCounter8bitArithFixture> s_level6PC8bitArith;

void TestLevel6ProgramCounter8BitArithmetic::initTestCase()
{
    s_level6PC8bitArith = std::make_unique<ProgramCounter8bitArithFixture>();
    QVERIFY(s_level6PC8bitArith->build());
}

void TestLevel6ProgramCounter8BitArithmetic::cleanupTestCase()
{
    s_level6PC8bitArith.reset();
}

void TestLevel6ProgramCounter8BitArithmetic::cleanup()
{
    if (s_level6PC8bitArith && s_level6PC8bitArith->sim) {
        s_level6PC8bitArith->sim->restart();
        s_level6PC8bitArith->sim->update();
    }
}

void TestLevel6ProgramCounter8BitArithmetic::testProgramCounter8BitWithPCPlus1() {
    auto &f = *s_level6PC8bitArith;

    // Test 1: Load 0x42 and verify pc_plus_1 = 0x43
    setMultiBitInput(f.dataInputs, 0x42);
    f.load->setOn(true);
    f.reset->setOn(false);
    f.sim->update();
    clockCycle(f.sim, f.clock);
    f.sim->update();

    QCOMPARE(f.readPC(), 0x42);
    QCOMPARE(f.readPCPlus1(), 0x43);

    // Test 2: Increment to 0x43 and verify pc_plus_1 = 0x44
    f.clock->setOn(false);
    f.load->setOn(false);
    f.inc->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clock);
    f.sim->update();

    QCOMPARE(f.readPC(), 0x43);
    QCOMPARE(f.readPCPlus1(), 0x44);

    // Test 3: Load 0xFF and verify pc_plus_1 = 0x00 (wraparound)
    f.clock->setOn(false);
    setMultiBitInput(f.dataInputs, 0xFF);
    f.load->setOn(true);
    f.inc->setOn(false);
    f.sim->update();
    clockCycle(f.sim, f.clock);
    f.sim->update();

    QCOMPARE(f.readPC(), 0xFF);
    QCOMPARE(f.readPCPlus1(), 0x00);
}

