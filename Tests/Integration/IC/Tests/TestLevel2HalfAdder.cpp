// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel2HalfAdder.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct HalfAdderFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *swA = nullptr, *swB = nullptr;
    Led *ledSum = nullptr, *ledCarry = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        swA = new InputSwitch();
        swB = new InputSwitch();
        ledSum = new Led();
        ledCarry = new Led();

        builder.add(swA, swB, ledSum, ledCarry);

        ic = loadBuildingBlockIC("level2_half_adder.panda");
        builder.add(ic);

        builder.connect(swA, 0, ic, "A");
        builder.connect(swB, 0, ic, "B");
        builder.connect(ic, "Sum", ledSum, 0);
        builder.connect(ic, "Carry", ledCarry, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<HalfAdderFixture> s_level2HalfAdder;

void TestLevel2HalfAdder::initTestCase()
{
    s_level2HalfAdder = std::make_unique<HalfAdderFixture>();
    QVERIFY(s_level2HalfAdder->build());
}

void TestLevel2HalfAdder::cleanupTestCase()
{
    s_level2HalfAdder.reset();
}

void TestLevel2HalfAdder::cleanup()
{
    if (s_level2HalfAdder && s_level2HalfAdder->sim) {
        s_level2HalfAdder->sim->restart();
        s_level2HalfAdder->sim->update();
    }
}

// ============================================================
// Half Adder IC Tests
// ============================================================

// Half Adder: A + B -> (Sum, Carry)
// Sum = A XOR B
// Carry = A AND B
void TestLevel2HalfAdder::testHalfAdder_data()
{
    QTest::addColumn<bool>("inputA");
    QTest::addColumn<bool>("inputB");
    QTest::addColumn<bool>("expectedSum");
    QTest::addColumn<bool>("expectedCarry");

    QTest::newRow("0 + 0 = 0, c=0") << false << false << false << false;
    QTest::newRow("0 + 1 = 1, c=0") << false << true << true << false;
    QTest::newRow("1 + 0 = 1, c=0") << true << false << true << false;
    QTest::newRow("1 + 1 = 0, c=1") << true << true << false << true;
}

void TestLevel2HalfAdder::testHalfAdder()
{
    QFETCH(bool, inputA);
    QFETCH(bool, inputB);
    QFETCH(bool, expectedSum);
    QFETCH(bool, expectedCarry);

    auto &f = *s_level2HalfAdder;

    f.swA->setOn(inputA);
    f.swB->setOn(inputB);
    f.sim->update();

    QCOMPARE(getInputStatus(f.ledSum), expectedSum);
    QCOMPARE(getInputStatus(f.ledCarry), expectedCarry);
}

