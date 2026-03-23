// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4BusMux8bit.h"

#include <QFile>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

struct BusMux8bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *in0[8] = {}, *in1[8] = {};
    InputSwitch *sel = nullptr;
    Led *out[8] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        sel = new InputSwitch();
        builder.add(sel);

        ic = loadBuildingBlockIC("level4_bus_mux_8bit.panda");
        builder.add(ic);

        for (int i = 0; i < 8; ++i) {
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

static std::unique_ptr<BusMux8bitFixture> s_level4BusMux8bit;

void TestLevel4BusMUX8Bit::initTestCase()
{
    s_level4BusMux8bit = std::make_unique<BusMux8bitFixture>();
    QVERIFY(s_level4BusMux8bit->build());
}

void TestLevel4BusMUX8Bit::cleanupTestCase()
{
    s_level4BusMux8bit.reset();
}

void TestLevel4BusMUX8Bit::cleanup()
{
    if (s_level4BusMux8bit && s_level4BusMux8bit->sim) {
        s_level4BusMux8bit->sim->restart();
        s_level4BusMux8bit->sim->update();
    }
}

void TestLevel4BusMUX8Bit::testBusMux8Bit_data()
{
    QTest::addColumn<int>("in0Value");
    QTest::addColumn<int>("in1Value");
    QTest::addColumn<bool>("select");
    QTest::addColumn<int>("expectedOutput");

    // Test select = 0 (choose In0)
    QTest::newRow("sel0_in0=0x00") << 0x00 << 0xFF << false << 0x00;
    QTest::newRow("sel0_in0=0xFF") << 0xFF << 0x00 << false << 0xFF;
    QTest::newRow("sel0_in0=0xAA") << 0xAA << 0x55 << false << 0xAA;

    // Test select = 1 (choose In1)
    QTest::newRow("sel1_in1=0xFF") << 0x00 << 0xFF << true << 0xFF;
    QTest::newRow("sel1_in1=0x55") << 0xAA << 0x55 << true << 0x55;
    QTest::newRow("sel1_in1=0x3C") << 0xC3 << 0x3C << true << 0x3C;

    // Test 8-bit patterns
    QTest::newRow("pattern_10101010_sel0") << 0xAA << 0x55 << false << 0xAA;
    QTest::newRow("pattern_01010101_sel1") << 0xAA << 0x55 << true << 0x55;
}

void TestLevel4BusMUX8Bit::testBusMux8Bit()
{
    QFETCH(int, in0Value);
    QFETCH(int, in1Value);
    QFETCH(bool, select);
    QFETCH(int, expectedOutput);

    auto &f = *s_level4BusMux8bit;

    for (int i = 0; i < 8; ++i) {
        f.in0[i]->setOn((in0Value >> i) & 1);
        f.in1[i]->setOn((in1Value >> i) & 1);
    }
    f.sel->setOn(select);
    f.sim->update();

    int result = readMultiBitOutput(QVector<GraphicElement *>({
        f.out[0], f.out[1], f.out[2], f.out[3],
        f.out[4], f.out[5], f.out[6], f.out[7]
    }));

    QCOMPARE(result, expectedOutput);
}

