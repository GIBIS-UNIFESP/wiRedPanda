// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4BusMux4bit.h"

#include <QFile>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

struct BusMux4bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *in0[4] = {}, *in1[4] = {};
    InputSwitch *sel = nullptr;
    Led *out[4] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        sel = new InputSwitch();
        builder.add(sel);

        ic = loadBuildingBlockIC("level4_bus_mux_4bit.panda");
        builder.add(ic);

        for (int i = 0; i < 4; ++i) {
            in0[i] = new InputSwitch();
            in1[i] = new InputSwitch();
            out[i] = new Led();
            builder.add(in0[i], in1[i], out[i]);
            builder.connect(in0[i], 0, ic, QString("In0[%1]").arg(i));
            builder.connect(in1[i], 0, ic, QString("In1[%1]").arg(i));
            builder.connect(ic, QString("Out[%1]").arg(i), out[i], 0);
        }
        builder.connect(sel, 0, ic, "Sel");

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<BusMux4bitFixture> s_level4BusMux4bit;

void TestLevel4BusMUX4Bit::initTestCase()
{
    s_level4BusMux4bit = std::make_unique<BusMux4bitFixture>();
    QVERIFY(s_level4BusMux4bit->build());
}

void TestLevel4BusMUX4Bit::cleanupTestCase()
{
    s_level4BusMux4bit.reset();
}

void TestLevel4BusMUX4Bit::cleanup()
{
    if (s_level4BusMux4bit && s_level4BusMux4bit->sim) {
        s_level4BusMux4bit->sim->restart();
        s_level4BusMux4bit->sim->update();
    }
}

void TestLevel4BusMUX4Bit::testBusMux4Bit_data()
{
    QTest::addColumn<int>("in0Value");
    QTest::addColumn<int>("in1Value");
    QTest::addColumn<bool>("select");
    QTest::addColumn<int>("expectedOutput");

    // Test select = 0 (choose In0)
    QTest::newRow("sel0_in0=0xF") << 0xF << 0x0 << false << 0xF;
    QTest::newRow("sel0_in0=0x5") << 0x5 << 0xA << false << 0x5;
    QTest::newRow("sel0_in0=0x3") << 0x3 << 0xC << false << 0x3;

    // Test select = 1 (choose In1)
    QTest::newRow("sel1_in1=0xA") << 0xF << 0xA << true << 0xA;
    QTest::newRow("sel1_in1=0xC") << 0x0 << 0xC << true << 0xC;
    QTest::newRow("sel1_in1=0x5") << 0x3 << 0x5 << true << 0x5;

    // Test all values
    QTest::newRow("0000_select0") << 0x0 << 0xF << false << 0x0;
    QTest::newRow("1111_select0") << 0xF << 0x0 << false << 0xF;
    QTest::newRow("1010_select1") << 0x0 << 0xA << true << 0xA;
}

void TestLevel4BusMUX4Bit::testBusMux4Bit()
{
    QFETCH(int, in0Value);
    QFETCH(int, in1Value);
    QFETCH(bool, select);
    QFETCH(int, expectedOutput);

    auto &f = *s_level4BusMux4bit;

    for (int i = 0; i < 4; ++i) {
        f.in0[i]->setOn((in0Value >> i) & 1);
        f.in1[i]->setOn((in1Value >> i) & 1);
    }
    f.sel->setOn(select);
    f.sim->update();

    int result = readMultiBitOutput(QVector<GraphicElement *>({f.out[0], f.out[1], f.out[2], f.out[3]}));

    QCOMPARE(result, expectedOutput);
}

