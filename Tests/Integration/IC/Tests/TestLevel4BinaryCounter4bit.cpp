// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4BinaryCounter4bit.h"

#include <QFile>

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

struct BinaryCounter4bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *clk = nullptr;
    Led *counterOut[4] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        clk = new InputSwitch();
        builder.add(clk);

        ic = loadBuildingBlockIC("level4_binary_counter_4bit.panda");
        builder.add(ic);

        for (int i = 0; i < 4; ++i) {
            counterOut[i] = new Led();
            builder.add(counterOut[i]);
            builder.connect(ic, QString("Q%1").arg(i), counterOut[i], 0);
        }
        builder.connect(clk, 0, ic, "CLK");

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<BinaryCounter4bitFixture> s_level4BinaryCounter4bit;

void TestLevel4BinaryCounter4Bit::initTestCase()
{
    s_level4BinaryCounter4bit = std::make_unique<BinaryCounter4bitFixture>();
    QVERIFY(s_level4BinaryCounter4bit->build());
}

void TestLevel4BinaryCounter4Bit::cleanupTestCase()
{
    s_level4BinaryCounter4bit.reset();
}

void TestLevel4BinaryCounter4Bit::cleanup()
{
    if (s_level4BinaryCounter4bit && s_level4BinaryCounter4bit->sim) {
        s_level4BinaryCounter4bit->sim->restart();
        s_level4BinaryCounter4bit->sim->update();
    }
}

void TestLevel4BinaryCounter4Bit::testBinaryCounter_data()
{
    QTest::addColumn<int>("numCycles");
    QTest::addColumn<int>("expectedFinalCount");

    // Count up 1 cycle
    QTest::newRow("count 1 cycle") << 1 << 1;

    // Count up 3 cycles
    QTest::newRow("count 3 cycles") << 3 << 3;

    // Count up 8 cycles
    QTest::newRow("count 8 cycles") << 8 << 8;

    // Count up 15 cycles
    QTest::newRow("count 15 cycles") << 15 << 15;

    // Count up 16 cycles (wrap to 0)
    QTest::newRow("count 16 cycles (wrap)") << 16 << 0;

    // Count up 17 cycles (wrap to 1)
    QTest::newRow("count 17 cycles") << 17 << 1;
}

void TestLevel4BinaryCounter4Bit::testBinaryCounter()
{
    QFETCH(int, numCycles);
    QFETCH(int, expectedFinalCount);

    auto &f = *s_level4BinaryCounter4bit;

    f.clk->setOn(false);
    f.sim->update();

    for (int cycle = 0; cycle < numCycles; ++cycle) {
        clockCycle(f.sim, f.clk);
    }

    // Extra clock cycle to allow IC outputs to fully propagate
    clockCycle(f.sim, f.clk);

    int result = readMultiBitOutput(QVector<GraphicElement *>({
        f.counterOut[0], f.counterOut[1], f.counterOut[2], f.counterOut[3]
    }));

    QCOMPARE(result & 0xF, expectedFinalCount & 0xF);
}

