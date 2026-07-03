// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel3Comparator4bit.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::inputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct Comparator4bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *swA[4] = {}, *swB[4] = {};
    InputSwitch *swGtIn = nullptr, *swEqIn = nullptr, *swLtIn = nullptr;
    Led *ledGreater = nullptr, *ledEqual = nullptr, *ledLess = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level3_comparator_4bit.panda");
        builder.add(ic);

        for (int i = 0; i < 4; ++i) {
            swA[i] = new InputSwitch();
            swB[i] = new InputSwitch();
            builder.add(swA[i], swB[i]);
        }
        swGtIn = new InputSwitch();
        swEqIn = new InputSwitch();
        swLtIn = new InputSwitch();
        builder.add(swGtIn, swEqIn, swLtIn);

        ledGreater = new Led();
        ledEqual = new Led();
        ledLess = new Led();
        builder.add(ledGreater, ledEqual, ledLess);

        for (int i = 0; i < 4; ++i) {
            builder.connect(swA[i], 0, ic, QString("A[%1]").arg(i));
            builder.connect(swB[i], 0, ic, QString("B[%1]").arg(i));
        }
        builder.connect(swGtIn, 0, ic, "GreaterIn");
        builder.connect(swEqIn, 0, ic, "EqualIn");
        builder.connect(swLtIn, 0, ic, "LessIn");
        builder.connect(ic, "Greater", ledGreater, 0);
        builder.connect(ic, "Equal", ledEqual, 0);
        builder.connect(ic, "Less", ledLess, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    // 74LS85 standalone tie-off: EqualIn high, GreaterIn/LessIn low.
    void tieStandalone()
    {
        swGtIn->setOn(false);
        swEqIn->setOn(true);
        swLtIn->setOn(false);
    }
};

static std::unique_ptr<Comparator4bitFixture> s_level3Comparator4bit;

void TestLevel3Comparator4Bit::initTestCase()
{
    s_level3Comparator4bit = std::make_unique<Comparator4bitFixture>();
    QVERIFY(s_level3Comparator4bit->build());
}

void TestLevel3Comparator4Bit::cleanupTestCase()
{
    s_level3Comparator4bit.reset();
}

void TestLevel3Comparator4Bit::cleanup()
{
    if (s_level3Comparator4bit && s_level3Comparator4bit->sim) {
        s_level3Comparator4bit->sim->restart();
        s_level3Comparator4bit->sim->update();
    }
}

void TestLevel3Comparator4Bit::testComparator4Bit_data()
{
    QTest::addColumn<int>("valueA");
    QTest::addColumn<int>("valueB");
    QTest::addColumn<bool>("expectedGreater");
    QTest::addColumn<bool>("expectedEqual");
    QTest::addColumn<bool>("expectedLess");

    QTest::newRow("5 > 3") << 5 << 3 << true << false << false;
    QTest::newRow("3 < 5") << 3 << 5 << false << false << true;
    QTest::newRow("5 == 5") << 5 << 5 << false << true << false;
    QTest::newRow("0 == 0") << 0 << 0 << false << true << false;
    QTest::newRow("15 > 0") << 15 << 0 << true << false << false;
    QTest::newRow("0 < 15") << 0 << 15 << false << false << true;
    QTest::newRow("7 == 7") << 7 << 7 << false << true << false;
    QTest::newRow("12 > 8") << 12 << 8 << true << false << false;
    QTest::newRow("8 < 12") << 8 << 12 << false << false << true;
    QTest::newRow("15 == 15") << 15 << 15 << false << true << false;
    QTest::newRow("10 < 11") << 10 << 11 << false << false << true;
    QTest::newRow("11 > 10") << 11 << 10 << true << false << false;
}

void TestLevel3Comparator4Bit::testComparator4Bit()
{
    QFETCH(int, valueA);
    QFETCH(int, valueB);
    QFETCH(bool, expectedGreater);
    QFETCH(bool, expectedEqual);
    QFETCH(bool, expectedLess);

    auto &f = *s_level3Comparator4bit;

    f.tieStandalone();
    for (int i = 0; i < 4; ++i) {
        f.swA[i]->setOn((valueA >> i) & 1);
        f.swB[i]->setOn((valueB >> i) & 1);
    }
    f.sim->update();

    QCOMPARE(inputStatus(f.ledGreater), expectedGreater);
    QCOMPARE(inputStatus(f.ledEqual), expectedEqual);
    QCOMPARE(inputStatus(f.ledLess), expectedLess);
}

// 74LS85 cascade behaviour: when the two nibbles are EQUAL the result follows
// the cascade inputs (this is how the more-significant stage defers to the
// less-significant one); when they DIFFER the local comparison dominates and
// the cascade inputs are ignored.
void TestLevel3Comparator4Bit::testComparator4BitCascade()
{
    auto &f = *s_level3Comparator4bit;

    auto setAB = [&](int a, int b) {
        for (int i = 0; i < 4; ++i) {
            f.swA[i]->setOn((a >> i) & 1);
            f.swB[i]->setOn((b >> i) & 1);
        }
    };
    auto setCascade = [&](bool gt, bool eq, bool lt) {
        f.swGtIn->setOn(gt);
        f.swEqIn->setOn(eq);
        f.swLtIn->setOn(lt);
    };

    // Equal nibbles (5 == 5): output mirrors whichever cascade input is asserted.
    setAB(5, 5);
    setCascade(true, false, false);   // lower stage said A>B
    f.sim->update();
    QVERIFY(inputStatus(f.ledGreater));
    QVERIFY(!inputStatus(f.ledEqual));
    QVERIFY(!inputStatus(f.ledLess));

    setCascade(false, true, false);   // lower stage said A==B
    f.sim->update();
    QVERIFY(!inputStatus(f.ledGreater));
    QVERIFY(inputStatus(f.ledEqual));
    QVERIFY(!inputStatus(f.ledLess));

    setCascade(false, false, true);   // lower stage said A<B
    f.sim->update();
    QVERIFY(!inputStatus(f.ledGreater));
    QVERIFY(!inputStatus(f.ledEqual));
    QVERIFY(inputStatus(f.ledLess));

    // Unequal nibbles: local comparison wins regardless of the cascade inputs.
    setAB(9, 4);                       // A > B locally
    setCascade(false, false, true);    // lower stage says A<B — must be ignored
    f.sim->update();
    QVERIFY(inputStatus(f.ledGreater));
    QVERIFY(!inputStatus(f.ledEqual));
    QVERIFY(!inputStatus(f.ledLess));

    setAB(4, 9);                       // A < B locally
    setCascade(true, false, false);    // lower stage says A>B — must be ignored
    f.sim->update();
    QVERIFY(!inputStatus(f.ledGreater));
    QVERIFY(!inputStatus(f.ledEqual));
    QVERIFY(inputStatus(f.ledLess));
}
