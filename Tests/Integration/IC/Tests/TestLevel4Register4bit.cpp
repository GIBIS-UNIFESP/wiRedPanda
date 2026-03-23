// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4Register4bit.h"

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

struct Register4bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *clk = nullptr, *en = nullptr;
    InputSwitch *dataIn[4] = {};
    Led *dataOut[4] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        clk = new InputSwitch();
        en = new InputSwitch();
        builder.add(clk, en);

        ic = loadBuildingBlockIC("level4_register_4bit.panda");
        builder.add(ic);

        builder.connect(clk, 0, ic, "CLK");
        builder.connect(en, 0, ic, "EN");

        for (int i = 0; i < 4; ++i) {
            dataIn[i] = new InputSwitch();
            dataOut[i] = new Led();
            builder.add(dataIn[i], dataOut[i]);
            builder.connect(dataIn[i], 0, ic, QString("D%1").arg(i));
            builder.connect(ic, QString("Q%1").arg(i), dataOut[i], 0);
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<Register4bitFixture> s_level4Register4bit;

void TestLevel4Register4Bit::initTestCase()
{
    s_level4Register4bit = std::make_unique<Register4bitFixture>();
    QVERIFY(s_level4Register4bit->build());
}

void TestLevel4Register4Bit::cleanupTestCase()
{
    s_level4Register4bit.reset();
}

void TestLevel4Register4Bit::cleanup()
{
    if (s_level4Register4bit && s_level4Register4bit->sim) {
        s_level4Register4bit->sim->restart();
        s_level4Register4bit->sim->update();
    }
}

void TestLevel4Register4Bit::test4BitRegister_data()
{
    QTest::addColumn<int>("initialValue");
    QTest::addColumn<int>("dataToLoad");
    QTest::addColumn<bool>("enable");
    QTest::addColumn<int>("expectedAfterLoad");

    // Test loading different values
    QTest::newRow("load 0 with enable") << 0xF << 0x0 << true << 0x0;
    QTest::newRow("load 5 with enable") << 0x0 << 0x5 << true << 0x5;
    QTest::newRow("load F with enable") << 0x0 << 0xF << true << 0xF;
    QTest::newRow("load 3 with enable") << 0xA << 0x3 << true << 0x3;

    // Test hold when enable is false
    QTest::newRow("hold with enable=0") << 0x7 << 0x0 << false << 0x7;
    QTest::newRow("hold 5 with enable=0") << 0x5 << 0xA << false << 0x5;

    // Test various bit patterns
    QTest::newRow("load 1010") << 0x0 << 0xA << true << 0xA;
    QTest::newRow("load 0101") << 0xF << 0x5 << true << 0x5;
}

void TestLevel4Register4Bit::test4BitRegister()
{
    QFETCH(int, initialValue);
    QFETCH(int, dataToLoad);
    QFETCH(bool, enable);
    QFETCH(int, expectedAfterLoad);

    auto &f = *s_level4Register4bit;

    // Load initial value with enable=true
    f.clk->setOn(false);
    f.en->setOn(true);
    f.sim->update();

    for (int i = 0; i < 4; ++i) {
        f.dataIn[i]->setOn((initialValue >> i) & 1);
    }
    f.sim->update();
    clockCycle(f.sim, f.clk);

    // Set enable for this test
    f.en->setOn(enable);
    f.sim->update();

    // Set data to load
    for (int i = 0; i < 4; ++i) {
        f.dataIn[i]->setOn((dataToLoad >> i) & 1);
    }
    f.sim->update();
    clockCycle(f.sim, f.clk);

    int result = readMultiBitOutput(QVector<GraphicElement *>({
        f.dataOut[0], f.dataOut[1], f.dataOut[2], f.dataOut[3]
    }));

    QCOMPARE(result, expectedAfterLoad);
}

