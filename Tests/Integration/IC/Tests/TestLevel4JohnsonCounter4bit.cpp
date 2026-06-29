// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4JohnsonCounter4bit.h"

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

struct JohnsonCounter4bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *clk = nullptr, *preset = nullptr, *ce = nullptr, *load = nullptr;
    InputSwitch *data[4] = {};
    Led *counterOut[4] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        clk = new InputSwitch();
        preset = new InputSwitch();
        ce = new InputSwitch();
        load = new InputSwitch();
        builder.add(clk, preset, ce, load);
        for (int i = 0; i < 4; ++i) {
            data[i] = new InputSwitch();
            builder.add(data[i]);
        }

        ic = loadBuildingBlockIC("level4_johnson_counter_4bit.panda");
        builder.add(ic);

        builder.connect(clk, 0, ic, "CLK");
        builder.connect(preset, 0, ic, "Init");
        builder.connect(ce, 0, ic, "CountEnable");
        builder.connect(load, 0, ic, "Load");
        for (int i = 0; i < 4; ++i) {
            builder.connect(data[i], 0, ic, QString("Data[%1]").arg(i));
        }

        for (int i = 0; i < 4; ++i) {
            counterOut[i] = new Led();
            builder.add(counterOut[i]);
            builder.connect(ic, QString("Q%1").arg(i), counterOut[i], 0);
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    // Normal-operation tie-off: counting enabled, no parallel load.
    void tieRun()
    {
        ce->setOn(true);
        load->setOn(false);
        for (int i = 0; i < 4; ++i) {
            data[i]->setOn(false);
        }
    }

    void presetPulse()
    {
        preset->setOn(false);  // inactive (active-HIGH)
        sim->update();
        preset->setOn(true);   // active -> init to 0001
        sim->update();
        preset->setOn(false);  // release
        sim->update();
    }

    int readCount()
    {
        return readMultiBitOutput(QVector<GraphicElement *>({
            counterOut[0], counterOut[1], counterOut[2], counterOut[3]
        })) & 0xF;
    }
};

static std::unique_ptr<JohnsonCounter4bitFixture> s_level4JohnsonCounter4bit;

void TestLevel4JohnsonCounter4Bit::initTestCase()
{
    s_level4JohnsonCounter4bit = std::make_unique<JohnsonCounter4bitFixture>();
    QVERIFY(s_level4JohnsonCounter4bit->build());
}

void TestLevel4JohnsonCounter4Bit::cleanupTestCase()
{
    s_level4JohnsonCounter4bit.reset();
}

void TestLevel4JohnsonCounter4Bit::cleanup()
{
    if (s_level4JohnsonCounter4bit && s_level4JohnsonCounter4bit->sim) {
        s_level4JohnsonCounter4bit->sim->restart();
        s_level4JohnsonCounter4bit->sim->update();
    }
}

void TestLevel4JohnsonCounter4Bit::testJohnsonCounter_data()
{
    QTest::addColumn<int>("cycleNumber");
    QTest::addColumn<int>("expectedPattern");

    // Sequence: inverted feedback ring counter
    // 0001→0011→0111→1111→1110→1100→1000→0000→0001
    QTest::newRow("cycle 0") << 0 << 0x1;  // 0001
    QTest::newRow("cycle 1") << 1 << 0x3;  // 0011
    QTest::newRow("cycle 2") << 2 << 0x7;  // 0111
    QTest::newRow("cycle 3") << 3 << 0xF;  // 1111
    QTest::newRow("cycle 4") << 4 << 0xE;  // 1110
    QTest::newRow("cycle 5") << 5 << 0xC;  // 1100
    QTest::newRow("cycle 6") << 6 << 0x8;  // 1000
    QTest::newRow("cycle 7") << 7 << 0x0;  // 0000
}

void TestLevel4JohnsonCounter4Bit::testJohnsonCounter()
{
    QFETCH(int, cycleNumber);
    QFETCH(int, expectedPattern);

    auto &f = *s_level4JohnsonCounter4bit;

    f.tieRun();          // counting enabled, no load
    f.clk->setOn(false);
    f.sim->update();
    f.presetPulse();     // initialise to 0001

    for (int cycle = 0; cycle < cycleNumber; ++cycle) {
        clockCycle(f.sim, f.clk);
    }

    QCOMPARE(f.readCount(), expectedPattern & 0xF);
}

// CountEnable=0 freezes the twisted-ring sequence; =1 resumes it.
void TestLevel4JohnsonCounter4Bit::testCountEnableHold()
{
    auto &f = *s_level4JohnsonCounter4bit;
    f.tieRun();
    f.clk->setOn(false);
    f.sim->update();
    f.presetPulse();              // 0001

    clockCycle(f.sim, f.clk);     // 0011
    QCOMPARE(f.readCount(), 0x3);

    // Pause: edges with CountEnable low hold the pattern.
    f.ce->setOn(false);
    f.sim->update();
    for (int i = 0; i < 3; ++i) {
        clockCycle(f.sim, f.clk);
    }
    QCOMPARE(f.readCount(), 0x3);

    // Resume: next edge advances to 0111.
    f.ce->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clk);
    QCOMPARE(f.readCount(), 0x7);
}

// Synchronous parallel Load captures an arbitrary value on the next edge.
void TestLevel4JohnsonCounter4Bit::testParallelLoad()
{
    auto &f = *s_level4JohnsonCounter4bit;
    f.tieRun();
    f.clk->setOn(false);
    f.preset->setOn(false);       // keep Init inactive
    f.sim->update();

    // Load 0x5 = 0101.
    f.data[0]->setOn(true);
    f.data[1]->setOn(false);
    f.data[2]->setOn(true);
    f.data[3]->setOn(false);
    f.load->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clk);
    QCOMPARE(f.readCount(), 0x5);

    f.load->setOn(false);
    f.sim->update();
}
