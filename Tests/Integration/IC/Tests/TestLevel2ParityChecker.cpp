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
    InputSwitch *swData[8] = {};
    InputSwitch *cascadeIn = nullptr;
    Led *ledResult = nullptr;
    Led *ledEven = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        for (int i = 0; i < 8; ++i) {
            swData[i] = new InputSwitch();
            builder.add(swData[i]);
        }
        cascadeIn = new InputSwitch();
        builder.add(cascadeIn);
        ledResult = new Led();
        builder.add(ledResult);
        ledEven = new Led();
        builder.add(ledEven);

        ic = loadBuildingBlockIC("level2_parity_checker.panda");
        builder.add(ic);

        // Drive every data input — the IC is an 8-bit XOR tree, so leaving any
        // input dangling would only "pass" by relying on unconnected-port = 0.
        for (int i = 0; i < 8; ++i) {
            builder.connect(swData[i], 0, ic, QString("data[%1]").arg(i));
        }
        builder.connect(cascadeIn, 0, ic, "CascadeIn");
        builder.connect(ic, "parity", ledResult, 0);
        builder.connect(ic, "even", ledEven, 0);

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
    QTest::addColumn<int>("dataByte");       // all 8 data bits (data[0..7])
    QTest::addColumn<int>("expectedResult"); // 1 when the byte has ODD parity

    // The IC XORs all 8 inputs: output = 1 iff the byte has an odd number of 1s.
    // Cases below walk every single bit (so data[5..7] are genuinely exercised)
    // plus all-zero / all-one / mixed odd/even-weight bytes.
    QTest::newRow("0x00 (0 ones) = even")  << 0x00 << 0;
    QTest::newRow("0x01 (bit0) = odd")     << 0x01 << 1;
    QTest::newRow("0x02 (bit1) = odd")     << 0x02 << 1;
    QTest::newRow("0x04 (bit2) = odd")     << 0x04 << 1;
    QTest::newRow("0x08 (bit3) = odd")     << 0x08 << 1;
    QTest::newRow("0x10 (bit4) = odd")     << 0x10 << 1;
    QTest::newRow("0x20 (bit5) = odd")     << 0x20 << 1;
    QTest::newRow("0x40 (bit6) = odd")     << 0x40 << 1;
    QTest::newRow("0x80 (bit7) = odd")     << 0x80 << 1;
    QTest::newRow("0xFF (8 ones) = even")  << 0xFF << 0;
    QTest::newRow("0x7F (7 ones) = odd")   << 0x7F << 1;
    QTest::newRow("0xE0 (bits5-7) = odd")  << 0xE0 << 1;  // all three high bits
    QTest::newRow("0xC0 (bits6-7) = even") << 0xC0 << 0;
    QTest::newRow("0xAA (4 ones) = even")  << 0xAA << 0;  // bits 1,3,5,7
    QTest::newRow("0x55 (4 ones) = even")  << 0x55 << 0;  // bits 0,2,4,6
    QTest::newRow("0x81 (bits0,7) = even") << 0x81 << 0;
    QTest::newRow("0xB3 (5 ones) = odd")   << 0xB3 << 1;
}

void TestLevel2ParityChecker::testOddParityChecker()
{
    QFETCH(int, dataByte);
    QFETCH(int, expectedResult);

    auto &f = *s_level2ParityChecker;

    f.cascadeIn->setOn(false);   // standalone: cascade input is a no-op
    for (int i = 0; i < 8; ++i) {
        f.swData[i]->setOn(static_cast<bool>(dataByte & (1 << i)));
    }
    f.sim->update();

    QCOMPARE(getInputStatus(f.ledResult) ? 1 : 0, expectedResult);
    // Complementary even-parity output must always be the inverse of odd parity.
    QCOMPARE(getInputStatus(f.ledEven) ? 1 : 0, expectedResult ? 0 : 1);
}

// 74180-style cascade input: CascadeIn is XORed into the parity, so chaining a
// second block (its parity -> this CascadeIn) checks parity across >8 bits.
void TestLevel2ParityChecker::testParityCascadeIn()
{
    auto &f = *s_level2ParityChecker;

    // 0xE0 has odd parity (3 ones). CascadeIn=1 flips the combined parity even.
    for (int i = 0; i < 8; ++i) {
        f.swData[i]->setOn(static_cast<bool>(0xE0 & (1 << i)));
    }

    f.cascadeIn->setOn(false);
    f.sim->update();
    QVERIFY(getInputStatus(f.ledResult));
    QVERIFY(!getInputStatus(f.ledEven));

    f.cascadeIn->setOn(true);
    f.sim->update();
    QVERIFY(!getInputStatus(f.ledResult));
    QVERIFY(getInputStatus(f.ledEven));

    // 0xC0 has even parity (2 ones). CascadeIn=1 flips it to odd.
    for (int i = 0; i < 8; ++i) {
        f.swData[i]->setOn(static_cast<bool>(0xC0 & (1 << i)));
    }
    f.cascadeIn->setOn(false);
    f.sim->update();
    QVERIFY(!getInputStatus(f.ledResult));

    f.cascadeIn->setOn(true);
    f.sim->update();
    QVERIFY(getInputStatus(f.ledResult));
}
