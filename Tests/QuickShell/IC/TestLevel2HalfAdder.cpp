// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/IC/TestLevel2HalfAdder.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"
#include "Tests/QuickShell/QuickCpuTestUtils.h"

using TestUtils::inputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct HalfAdderFixture {
    std::unique_ptr<QuickCircuitBuilder> builder;
    IC *ic = nullptr;
    InputSwitch *swA = nullptr, *swB = nullptr;
    Led *ledSum = nullptr, *ledCarry = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        builder = std::make_unique<QuickCircuitBuilder>();

        swA = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
        swB = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
        ledSum = static_cast<Led *>(builder->addOwnedElement(new Led()));
        ledCarry = static_cast<Led *>(builder->addOwnedElement(new Led()));

        ic = static_cast<IC *>(builder->addOwnedElement(loadBuildingBlockIC("level2_half_adder.panda")));

        builder->connect(swA, 0, ic, "A");
        builder->connect(swB, 0, ic, "B");
        builder->connect(ic, "Sum", ledSum, 0);
        builder->connect(ic, "Carry", ledCarry, 0);

        sim = builder->initSimulation();
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

    QCOMPARE(inputStatus(f.ledSum), expectedSum);
    QCOMPARE(inputStatus(f.ledCarry), expectedCarry);
}
