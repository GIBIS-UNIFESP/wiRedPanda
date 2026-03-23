// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4Comparator4bit.h"

#include <QFile>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct Level4Comparator4bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *swA[4] = {}, *swB[4] = {};
    Led *ledGreater = nullptr, *ledEqual = nullptr, *ledLess = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level4_comparator_4bit.panda");
        builder.add(ic);

        for (int i = 0; i < 4; ++i) {
            swA[i] = new InputSwitch();
            swB[i] = new InputSwitch();
            builder.add(swA[i], swB[i]);
            builder.connect(swA[i], 0, ic, QString("A[%1]").arg(i));
            builder.connect(swB[i], 0, ic, QString("B[%1]").arg(i));
        }

        ledGreater = new Led();
        ledEqual = new Led();
        ledLess = new Led();
        builder.add(ledGreater, ledEqual, ledLess);

        builder.connect(ic, "Greater", ledGreater, 0);
        builder.connect(ic, "Equal", ledEqual, 0);
        builder.connect(ic, "Less", ledLess, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<Level4Comparator4bitFixture> s_level4Comparator4bit;

void TestLevel4Comparator4Bit::initTestCase()
{
    s_level4Comparator4bit = std::make_unique<Level4Comparator4bitFixture>();
    QVERIFY(s_level4Comparator4bit->build());
}

void TestLevel4Comparator4Bit::cleanupTestCase()
{
    s_level4Comparator4bit.reset();
}

void TestLevel4Comparator4Bit::cleanup()
{
    if (s_level4Comparator4bit && s_level4Comparator4bit->sim) {
        s_level4Comparator4bit->sim->restart();
        s_level4Comparator4bit->sim->update();
    }
}

void TestLevel4Comparator4Bit::testComparator4Bit_data()
{
    QTest::addColumn<int>("aValue");
    QTest::addColumn<int>("bValue");
    QTest::addColumn<bool>("expectedGreater");
    QTest::addColumn<bool>("expectedEqual");
    QTest::addColumn<bool>("expectedLess");

    // Test equal values
    QTest::newRow("equal_0000") << 0x0 << 0x0 << false << true << false;
    QTest::newRow("equal_1111") << 0xF << 0xF << false << true << false;
    QTest::newRow("equal_0101") << 0x5 << 0x5 << false << true << false;
    QTest::newRow("equal_1010") << 0xA << 0xA << false << true << false;

    // Test A > B (Greater output)
    QTest::newRow("5_greater_3") << 0x5 << 0x3 << true << false << false;
    QTest::newRow("15_greater_0") << 0xF << 0x0 << true << false << false;
    QTest::newRow("12_greater_8") << 0xC << 0x8 << true << false << false;
    QTest::newRow("11_greater_10") << 0xB << 0xA << true << false << false;  // LSB only differs

    // Test A < B (Less output)
    QTest::newRow("3_less_5") << 0x3 << 0x5 << false << false << true;
    QTest::newRow("0_less_15") << 0x0 << 0xF << false << false << true;
    QTest::newRow("8_less_12") << 0x8 << 0xC << false << false << true;
    QTest::newRow("10_less_11") << 0xA << 0xB << false << false << true;  // LSB only differs
}

void TestLevel4Comparator4Bit::testComparator4Bit()
{
    QFETCH(int, aValue);
    QFETCH(int, bValue);
    QFETCH(bool, expectedGreater);
    QFETCH(bool, expectedEqual);
    QFETCH(bool, expectedLess);

    auto &f = *s_level4Comparator4bit;

    for (int i = 0; i < 4; ++i) {
        f.swA[i]->setOn((aValue >> i) & 1);
        f.swB[i]->setOn((bValue >> i) & 1);
    }
    f.sim->update();

    QCOMPARE(getInputStatus(f.ledGreater), expectedGreater);
    QCOMPARE(getInputStatus(f.ledEqual), expectedEqual);
    QCOMPARE(getInputStatus(f.ledLess), expectedLess);
}

