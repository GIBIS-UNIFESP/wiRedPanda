// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel2ParityChecker.h"

#include "App/Core/Common.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct ParityCheckerFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *swD[4] = {};
    InputSwitch *swP = nullptr;
    Led *ledResult = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        for (int i = 0; i < 4; ++i) {
            swD[i] = new InputSwitch();
            builder.add(swD[i]);
        }
        swP = new InputSwitch();
        ledResult = new Led();
        builder.add(swP, ledResult);

        ic = loadBuildingBlockIC("level2_parity_checker.panda");
        builder.add(ic);

        for (int i = 0; i < 4; ++i) {
            builder.connect(swD[i], 0, ic, QString("data[%1]").arg(i));
        }
        builder.connect(swP, 0, ic, "data[4]");
        builder.connect(ic, "parity", ledResult, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<ParityCheckerFixture> s_level2ParityChecker;

void TestLevel2ParityChecker::initTestCase()
{
    s_level2ParityChecker = std::make_unique<ParityCheckerFixture>();
    QVERIFY(s_level2ParityChecker->build());
}

void TestLevel2ParityChecker::cleanupTestCase()
{
    s_level2ParityChecker.reset();
}

void TestLevel2ParityChecker::cleanup()
{
    if (s_level2ParityChecker && s_level2ParityChecker->sim) {
        s_level2ParityChecker->sim->restart();
        s_level2ParityChecker->sim->update();
    }
}

void TestLevel2ParityChecker::testOddParityChecker_data()
{
    QTest::addColumn<int>("dataInput");    // 4 data bits (D0-D3)
    QTest::addColumn<int>("parityBit");    // 1 parity bit
    QTest::addColumn<int>("expectedResult"); // 1 = valid odd parity, 0 = error detected

    // Valid odd parity cases: total 1's should be ODD
    // Data: 0000, Parity: 1 (total = 1 one = ODD) ✓
    QTest::newRow("0000 + P1 = valid") << 0x0 << 1 << 1;
    // Data: 0001, Parity: 0 (total = 1 one = ODD) ✓
    QTest::newRow("0001 + P0 = valid") << 0x1 << 0 << 1;
    // Data: 0011, Parity: 0 (total = 2 ones = EVEN) ✗ (but with parity → 2 = error)
    QTest::newRow("0011 + P0 = error") << 0x3 << 0 << 0;
    // Data: 0011, Parity: 1 (total = 3 ones = ODD) ✓
    QTest::newRow("0011 + P1 = valid") << 0x3 << 1 << 1;
    // Data: 0101, Parity: 1 (total = 3 ones = ODD) ✓
    QTest::newRow("0101 + P1 = valid") << 0x5 << 1 << 1;
    // Data: 0111, Parity: 0 (total = 3 ones = ODD) ✓
    QTest::newRow("0111 + P0 = valid") << 0x7 << 0 << 1;
    // Data: 1111, Parity: 1 (total = 5 ones = ODD) ✓
    QTest::newRow("1111 + P1 = valid") << 0xF << 1 << 1;
    // Data: 1111, Parity: 0 (total = 4 ones = EVEN) ✗
    QTest::newRow("1111 + P0 = error") << 0xF << 0 << 0;
    // Data: 0000, Parity: 0 (total = 0 ones = EVEN) ✗
    QTest::newRow("0000 + P0 = error") << 0x0 << 0 << 0;
    // Data: 1001, Parity: 0 (total = 2 ones = EVEN) ✗
    QTest::newRow("1001 + P0 = error") << 0x9 << 0 << 0;
    // Data: 1010, Parity: 1 (total = 3 ones = ODD) ✓
    QTest::newRow("1010 + P1 = valid") << 0xA << 1 << 1;
}

void TestLevel2ParityChecker::testOddParityChecker()
{
    QFETCH(int, dataInput);
    QFETCH(int, parityBit);
    QFETCH(int, expectedResult);

    auto &f = *s_level2ParityChecker;

    for (int i = 0; i < 4; ++i) {
        f.swD[i]->setOn(static_cast<bool>(dataInput & (1 << i)));
    }
    f.swP->setOn(static_cast<bool>(parityBit));
    f.sim->update();

    QCOMPARE(getInputStatus(f.ledResult) ? 1 : 0, expectedResult);
}

