// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel5ModuloCounter4bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

struct ModuloCounter4bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *clk = nullptr;
    InputSwitch *moduloIn[4] = {};
    Led *counterOut[4] = {};
    Led *overflowOut = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        clk = new InputSwitch();
        builder.add(clk);

        ic = loadBuildingBlockIC("level5_modulo_counter_4bit.panda");
        builder.add(ic);

        builder.connect(clk, 0, ic, "CLK");

        for (int i = 0; i < 4; ++i) {
            moduloIn[i] = new InputSwitch();
            counterOut[i] = new Led();
            builder.add(moduloIn[i], counterOut[i]);
            builder.connect(moduloIn[i], 0, ic, QString("Modulo[%1]").arg(i));
            builder.connect(ic, QString("Q[%1]").arg(i), counterOut[i], 0);
        }
        overflowOut = new Led();
        builder.add(overflowOut);
        builder.connect(ic, "Overflow", overflowOut, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    void resetCounter()
    {
        clk->setOn(false);
        sim->update();
        clockCycle(sim, clk);
    }

    int readValue()
    {
        return readMultiBitOutput(QVector<GraphicElement *>({
            counterOut[0], counterOut[1], counterOut[2], counterOut[3]
        })) & 0xF;
    }
};

static std::unique_ptr<ModuloCounter4bitFixture> s_level5ModuloCounter4bit;

void TestLevel5ModuloCounter4Bit::initTestCase()
{
    s_level5ModuloCounter4bit = std::make_unique<ModuloCounter4bitFixture>();
    QVERIFY(s_level5ModuloCounter4bit->build());
}

void TestLevel5ModuloCounter4Bit::cleanupTestCase()
{
    s_level5ModuloCounter4bit.reset();
}

void TestLevel5ModuloCounter4Bit::cleanup()
{
    if (s_level5ModuloCounter4bit && s_level5ModuloCounter4bit->sim) {
        s_level5ModuloCounter4bit->sim->restart();
        s_level5ModuloCounter4bit->sim->update();
    }
}

void TestLevel5ModuloCounter4Bit::testModuloCounter_data()
{
    QTest::addColumn<int>("modulo");
    QTest::addColumn<int>("cycles");
    QTest::addColumn<int>("expectedPattern");

    QTest::newRow("mod5_cycle6") << 5 << 6 << 1;
    QTest::newRow("mod5_complete") << 5 << 5 << 0;
    QTest::newRow("mod10_to_9") << 10 << 9 << 9;
    QTest::newRow("mod10_wrap") << 10 << 11 << 1;
}

void TestLevel5ModuloCounter4Bit::testModuloCounter()
{
    QFETCH(int, modulo);
    QFETCH(int, cycles);
    QFETCH(int, expectedPattern);

    auto &f = *s_level5ModuloCounter4bit;

    for (int i = 0; i < 4; ++i) {
        f.moduloIn[i]->setOn((modulo >> i) & 1);
    }
    f.sim->update();

    f.resetCounter();

    for (int cycle = 0; cycle < cycles; ++cycle) {
        clockCycle(f.sim, f.clk);
    }

    QCOMPARE(f.readValue(), expectedPattern & 0xF);
}

