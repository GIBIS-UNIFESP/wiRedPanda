// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel2FullAdder1bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;

struct FullAdder1bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *a = nullptr, *b = nullptr, *carryIn = nullptr;
    Led *sum = nullptr, *carryOut = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        a = new InputSwitch();
        b = new InputSwitch();
        carryIn = new InputSwitch();
        sum = new Led();
        carryOut = new Led();

        builder.add(a, b, carryIn, sum, carryOut);

        ic = CPUTestUtils::loadBuildingBlockIC("level2_full_adder_1bit.panda");
        builder.add(ic);

        builder.connect(a, 0, ic, "A");
        builder.connect(b, 0, ic, "B");
        builder.connect(carryIn, 0, ic, "Cin");
        builder.connect(ic, "Sum", sum, 0);
        builder.connect(ic, "Cout", carryOut, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<FullAdder1bitFixture> s_level2FullAdder1bit;

void TestLevel2FullAdder1Bit::initTestCase()
{
    s_level2FullAdder1bit = std::make_unique<FullAdder1bitFixture>();
    QVERIFY(s_level2FullAdder1bit->build());
}

void TestLevel2FullAdder1Bit::cleanupTestCase()
{
    s_level2FullAdder1bit.reset();
}

void TestLevel2FullAdder1Bit::cleanup()
{
    if (s_level2FullAdder1bit && s_level2FullAdder1bit->sim) {
        s_level2FullAdder1bit->sim->restart();
        s_level2FullAdder1bit->sim->update();
    }
}

// ============================================================
// Test: Full Adder Truth Table (Parameterized)
// ============================================================

// Full Adder: A + B + Cin -> (Sum, Cout)
// Uses 2 Half Adders + 1 OR gate
// HA1: A + B -> (s1, c1)
// HA2: s1 + Cin -> (Sum, c2)
// Cout = c1 OR c2
void TestLevel2FullAdder1Bit::testFullAdder_data()
{
    QTest::addColumn<bool>("inputA");
    QTest::addColumn<bool>("inputB");
    QTest::addColumn<bool>("inputCin");
    QTest::addColumn<bool>("expectedSum");
    QTest::addColumn<bool>("expectedCout");

    QTest::newRow("0 + 0 + 0 = 0, c=0") << false << false << false << false << false;
    QTest::newRow("0 + 0 + 1 = 1, c=0") << false << false << true << true << false;
    QTest::newRow("0 + 1 + 0 = 1, c=0") << false << true << false << true << false;
    QTest::newRow("0 + 1 + 1 = 0, c=1") << false << true << true << false << true;
    QTest::newRow("1 + 0 + 0 = 1, c=0") << true << false << false << true << false;
    QTest::newRow("1 + 0 + 1 = 0, c=1") << true << false << true << false << true;
    QTest::newRow("1 + 1 + 0 = 0, c=1") << true << true << false << false << true;
    QTest::newRow("1 + 1 + 1 = 1, c=1") << true << true << true << true << true;
}

void TestLevel2FullAdder1Bit::testFullAdder()
{
    QFETCH(bool, inputA);
    QFETCH(bool, inputB);
    QFETCH(bool, inputCin);
    QFETCH(bool, expectedSum);
    QFETCH(bool, expectedCout);

    auto &f = *s_level2FullAdder1bit;

    f.a->setOn(inputA);
    f.b->setOn(inputB);
    f.carryIn->setOn(inputCin);
    f.sim->update();

    QCOMPARE(getInputStatus(f.sum), expectedSum);
    QCOMPARE(getInputStatus(f.carryOut), expectedCout);
}

// ============================================================
// Test: CPU-Level Full Adder with All Truth Table Combinations
// ============================================================

// Truth table for Full Adder:
// A | B | Cin | Sum | Cout
// --+---+-----+-----+------
// 0 | 0 |  0  |  0  |  0
// 0 | 0 |  1  |  1  |  0
// 0 | 1 |  0  |  1  |  0
// 0 | 1 |  1  |  0  |  1
// 1 | 0 |  0  |  1  |  0
// 1 | 0 |  1  |  0  |  1
// 1 | 1 |  0  |  0  |  1
// 1 | 1 |  1  |  1  |  1
void TestLevel2FullAdder1Bit::testCpuFullAdder_data()
{
    QTest::addColumn<int>("inputA");
    QTest::addColumn<int>("inputB");
    QTest::addColumn<int>("inputCin");
    QTest::addColumn<int>("expectedSum");
    QTest::addColumn<int>("expectedCout");

    QTest::newRow("0+0+0=0, Cout=0") << 0 << 0 << 0 << 0 << 0;
    QTest::newRow("0+0+1=1, Cout=0") << 0 << 0 << 1 << 1 << 0;
    QTest::newRow("0+1+0=1, Cout=0") << 0 << 1 << 0 << 1 << 0;
    QTest::newRow("0+1+1=10, Cout=1") << 0 << 1 << 1 << 0 << 1;
    QTest::newRow("1+0+0=1, Cout=0") << 1 << 0 << 0 << 1 << 0;
    QTest::newRow("1+0+1=10, Cout=1") << 1 << 0 << 1 << 0 << 1;
    QTest::newRow("1+1+0=10, Cout=1") << 1 << 1 << 0 << 0 << 1;
    QTest::newRow("1+1+1=11, Cout=1") << 1 << 1 << 1 << 1 << 1;
}

void TestLevel2FullAdder1Bit::testCpuFullAdder()
{
    QFETCH(int, inputA);
    QFETCH(int, inputB);
    QFETCH(int, inputCin);
    QFETCH(int, expectedSum);
    QFETCH(int, expectedCout);

    auto &f = *s_level2FullAdder1bit;

    f.a->setOn(inputA != 0);
    f.b->setOn(inputB != 0);
    f.carryIn->setOn(inputCin != 0);
    f.sim->update();

    auto sumPort = f.sum->inputPort(0);
    auto carryOutPort = f.carryOut->inputPort(0);

    int sum_actual = (sumPort && sumPort->status() == Status::Active) ? 1 : 0;
    int cout_actual = (carryOutPort && carryOutPort->status() == Status::Active) ? 1 : 0;

    QCOMPARE(sum_actual, expectedSum);
    QCOMPARE(cout_actual, expectedCout);
}

// ============================================================
// Test: IC Structure and Port Validation
// ============================================================

/**
 * Test: Verify 1-bit Full Adder IC structure and port count
 *
 * Validates:
 * - IC has 3 input ports (A, B, Cin)
 * - IC has 2 output ports (Sum, Cout)
 * - IC can be loaded and instantiated
 * - Port connections work correctly
 */
void TestLevel2FullAdder1Bit::testStructure()
{
    auto &f = *s_level2FullAdder1bit;

    QVERIFY(f.ic != nullptr);
    QCOMPARE(f.ic->inputSize(), 3);   // 3 input ports: A, B, Cin
    QCOMPARE(f.ic->outputSize(), 2);  // 2 output ports: Sum, Cout
}

