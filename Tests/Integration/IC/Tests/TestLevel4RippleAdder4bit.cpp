// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4RippleAdder4bit.h"

#include <QFileInfo>

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct RippleAdder4bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *swA[4] = {}, *swB[4] = {};
    InputSwitch *carryIn = nullptr;
    Led *ledSum[4] = {};
    Led *ledCarryOut = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level4_ripple_adder_4bit.panda");
        builder.add(ic);

        for (int i = 0; i < 4; ++i) {
            swA[i] = new InputSwitch();
            swB[i] = new InputSwitch();
            ledSum[i] = new Led();
            builder.add(swA[i], swB[i], ledSum[i]);
            builder.connect(swA[i], 0, ic, QString("A[%1]").arg(i));
            builder.connect(swB[i], 0, ic, QString("B[%1]").arg(i));
            builder.connect(ic, QString("Sum[%1]").arg(i), ledSum[i], 0);
        }

        carryIn = new InputSwitch();
        ledCarryOut = new Led();
        builder.add(carryIn, ledCarryOut);
        builder.connect(carryIn, 0, ic, "CarryIn");
        builder.connect(ic, "CarryOut", ledCarryOut, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    int readSum()
    {
        int result = 0;
        for (int i = 0; i < 4; ++i) {
            result |= (getInputStatus(ledSum[i]) ? 1 : 0) << i;
        }
        return result;
    }
};

static std::unique_ptr<RippleAdder4bitFixture> s_level4RippleAdder4bit;

void TestLevel4RippleAdder4Bit::initTestCase()
{
    s_level4RippleAdder4bit = std::make_unique<RippleAdder4bitFixture>();
    QVERIFY(s_level4RippleAdder4bit->build());
}

void TestLevel4RippleAdder4Bit::cleanupTestCase()
{
    s_level4RippleAdder4bit.reset();
}

void TestLevel4RippleAdder4Bit::cleanup()
{
    if (s_level4RippleAdder4bit && s_level4RippleAdder4bit->sim) {
        s_level4RippleAdder4bit->sim->restart();
        s_level4RippleAdder4bit->sim->update();
    }
}

void TestLevel4RippleAdder4Bit::testAdder4Bit_data()
{
    QTest::addColumn<int>("valueA");
    QTest::addColumn<int>("valueB");
    QTest::addColumn<int>("expectedSum");
    QTest::addColumn<bool>("expectedCarryOut");

    // Test cases: Real 4-bit addition with carry propagation
    // Sum is masked to 4 bits; carry-out flag indicates overflow
    QTest::newRow("0 + 0 = 0, no carry") << 0 << 0 << 0 << false;
    QTest::newRow("1 + 1 = 2, no carry") << 1 << 1 << 2 << false;
    QTest::newRow("3 + 5 = 8, no carry") << 3 << 5 << 8 << false;         // 0011 + 0101 = 1000
    QTest::newRow("1 + 7 = 8, no carry") << 1 << 7 << 8 << false;         // 0001 + 0111 = 1000
    QTest::newRow("7 + 8 = 15, no carry") << 7 << 8 << 15 << false;       // 0111 + 1000 = 1111
    QTest::newRow("4 + 4 = 8, no carry") << 4 << 4 << 8 << false;         // 0100 + 0100 = 1000
    QTest::newRow("15 + 1 = 0, CARRY OUT") << 15 << 1 << 0 << true;       // 1111 + 0001 = 10000 → overflow!
    QTest::newRow("8 + 8 = 0, CARRY OUT") << 8 << 8 << 0 << true;         // 1000 + 1000 = 10000 → overflow!
    QTest::newRow("8 + 7 = 15, no carry") << 8 << 7 << 15 << false;       // 1000 + 0111 = 1111
    QTest::newRow("15 + 15 = 14, CARRY OUT") << 15 << 15 << 14 << true;   // 1111 + 1111 = 11110 → overflow!
}

void TestLevel4RippleAdder4Bit::testAdder4Bit()
{
    QFETCH(int, valueA);
    QFETCH(int, valueB);
    QFETCH(int, expectedSum);
    QFETCH(bool, expectedCarryOut);

    auto &f = *s_level4RippleAdder4bit;

    for (int i = 0; i < 4; ++i) {
        f.swA[i]->setOn((valueA >> i) & 1);
        f.swB[i]->setOn((valueB >> i) & 1);
    }
    f.carryIn->setOn(false);
    f.sim->update();

    QCOMPARE(f.readSum(), expectedSum);
    QCOMPARE(getInputStatus(f.ledCarryOut), expectedCarryOut);
}

void TestLevel4RippleAdder4Bit::testRippleCarry_data()
{
    QTest::addColumn<int>("valueA");
    QTest::addColumn<int>("valueB");
    QTest::addColumn<int>("expectedSum");
    QTest::addColumn<bool>("expectedCarryOut");

    QTest::newRow("0 + 0 = 0, no carry") << 0 << 0 << 0 << false;
    QTest::newRow("1 + 1 = 2, no carry") << 1 << 1 << 2 << false;
    QTest::newRow("3 + 5 = 8, no carry") << 3 << 5 << 8 << false;
    QTest::newRow("1 + 7 = 8, no carry") << 1 << 7 << 8 << false;
    QTest::newRow("7 + 8 = 15, no carry") << 7 << 8 << 15 << false;
    QTest::newRow("4 + 4 = 8, no carry") << 4 << 4 << 8 << false;
    QTest::newRow("15 + 1 = 0, CARRY OUT") << 15 << 1 << 0 << true;
    QTest::newRow("8 + 8 = 0, CARRY OUT") << 8 << 8 << 0 << true;
    QTest::newRow("8 + 7 = 15, no carry") << 8 << 7 << 15 << false;
    QTest::newRow("15 + 15 = 14, CARRY OUT") << 15 << 15 << 14 << true;
}

void TestLevel4RippleAdder4Bit::testRippleCarry()
{
    QFETCH(int, valueA);
    QFETCH(int, valueB);
    QFETCH(int, expectedSum);
    QFETCH(bool, expectedCarryOut);

    auto &f = *s_level4RippleAdder4bit;

    for (int i = 0; i < 4; ++i) {
        f.swA[i]->setOn((valueA >> i) & 1);
        f.swB[i]->setOn((valueB >> i) & 1);
    }
    f.carryIn->setOn(false);
    f.sim->update();

    QCOMPARE(f.readSum(), expectedSum);
    QCOMPARE(getInputStatus(f.ledCarryOut), expectedCarryOut);
}

