// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4RingCounter4bit.h"

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

struct RingCounter4bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *clk = nullptr, *preset = nullptr;
    Led *counterOut[4] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        clk = new InputSwitch();
        preset = new InputSwitch();
        builder.add(clk, preset);

        ic = loadBuildingBlockIC("level4_ring_counter_4bit.panda");
        builder.add(ic);

        builder.connect(clk, 0, ic, "CLK");
        builder.connect(preset, 0, ic, "PRESET");

        for (int i = 0; i < 4; ++i) {
            counterOut[i] = new Led();
            builder.add(counterOut[i]);
            builder.connect(ic, QString("Q%1").arg(i), counterOut[i], 0);
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<RingCounter4bitFixture> s_level4RingCounter4bit;

void TestLevel4RingCounter4Bit::initTestCase()
{
    s_level4RingCounter4bit = std::make_unique<RingCounter4bitFixture>();
    QVERIFY(s_level4RingCounter4bit->build());
}

void TestLevel4RingCounter4Bit::cleanupTestCase()
{
    s_level4RingCounter4bit.reset();
}

void TestLevel4RingCounter4Bit::cleanup()
{
    if (s_level4RingCounter4bit && s_level4RingCounter4bit->sim) {
        s_level4RingCounter4bit->sim->restart();
        s_level4RingCounter4bit->sim->update();
    }
}

void TestLevel4RingCounter4Bit::testRingCounter_data()
{
    QTest::addColumn<int>("cycleNumber");
    QTest::addColumn<int>("expectedPattern");

    // Sequence: 0001→0010→0100→1000→0001
    QTest::newRow("cycle 0") << 0 << 0x1;  // 0001
    QTest::newRow("cycle 1") << 1 << 0x2;  // 0010
    QTest::newRow("cycle 2") << 2 << 0x4;  // 0100
    QTest::newRow("cycle 3") << 3 << 0x8;  // 1000
    QTest::newRow("cycle 4") << 4 << 0x1;  // 0001 (wrapped)
    QTest::newRow("cycle 5") << 5 << 0x2;  // 0010
}

void TestLevel4RingCounter4Bit::testRingCounter()
{
    QFETCH(int, cycleNumber);
    QFETCH(int, expectedPattern);

    auto &f = *s_level4RingCounter4bit;

    f.clk->setOn(false);
    f.preset->setOn(true);
    f.sim->update();

    f.preset->setOn(false);
    f.sim->update();

    f.preset->setOn(true);
    f.sim->update();

    for (int cycle = 0; cycle < cycleNumber; ++cycle) {
        clockCycle(f.sim, f.clk);
    }

    int result = readMultiBitOutput(QVector<GraphicElement *>({
        f.counterOut[0], f.counterOut[1], f.counterOut[2], f.counterOut[3]
    }));

    QCOMPARE(result & 0xF, expectedPattern & 0xF);
}

