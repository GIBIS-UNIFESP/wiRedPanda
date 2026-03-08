// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel2Decoder4to16.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using TestUtils::getOutputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct Decoder4to16Fixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *swAddr[4] = {};
    Led *ledOut[16] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        for (int i = 0; i < 4; ++i) {
            swAddr[i] = new InputSwitch();
            builder.add(swAddr[i]);
        }
        for (int i = 0; i < 16; ++i) {
            ledOut[i] = new Led();
            builder.add(ledOut[i]);
        }

        ic = loadBuildingBlockIC("level2_decoder_4to16.panda");
        builder.add(ic);

        for (int i = 0; i < 4; ++i) {
            builder.connect(swAddr[i], 0, ic, QString("addr[%1]").arg(i));
        }
        for (int i = 0; i < 16; ++i) {
            builder.connect(ic, QString("out[%1]").arg(i), ledOut[i], 0);
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<Decoder4to16Fixture> s_level2Decoder4to16;

void TestLevel2Decoder4To16::initTestCase()
{
    s_level2Decoder4to16 = std::make_unique<Decoder4to16Fixture>();
    QVERIFY(s_level2Decoder4to16->build());
}

void TestLevel2Decoder4To16::cleanupTestCase()
{
    s_level2Decoder4to16.reset();
}

void TestLevel2Decoder4To16::cleanup()
{
    if (s_level2Decoder4to16 && s_level2Decoder4to16->sim) {
        s_level2Decoder4to16->sim->restart();
        s_level2Decoder4to16->sim->update();
    }
}

// ============================================================
// 4-to-16 Decoder Tests
// ============================================================

/**
 * Test: 4-to-16 Decoder (4-bit address -> 16 one-hot outputs)
 * Selects one of 16 output lines based on 4-bit address input
 */
void TestLevel2Decoder4To16::test4to16Decoder_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("expectedActive");

    // For address N, output N should be active (1), others inactive (0)
    for (int i = 0; i < 16; ++i) {
        QTest::newRow(QString("address %1").arg(i).toLatin1().data()) << i << i;
    }
}

void TestLevel2Decoder4To16::test4to16Decoder()
{
    QFETCH(int, address);
    QFETCH(int, expectedActive);

    auto &f = *s_level2Decoder4to16;

    for (int i = 0; i < 4; ++i) {
        f.swAddr[i]->setOn((address >> i) & 1);
    }
    f.sim->update();

    for (int i = 0; i < 16; ++i) {
        bool actual = getInputStatus(f.ledOut[i]);
        bool expected = (i == expectedActive);
        QCOMPARE(actual, expected);
    }
}

// ============================================================
// Edge Cases
// ============================================================

void TestLevel2Decoder4To16::test4to16DecoderMaxAddress_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("expectedOutput");

    // Maximum address: 1111 (15) - should produce one-hot output
    QTest::newRow("max address 15") << 15 << 15;
    // Minimum address: 0000 (0) - should produce one-hot output
    QTest::newRow("min address 0") << 0 << 0;
}

void TestLevel2Decoder4To16::test4to16DecoderMaxAddress()
{
    QFETCH(int, address);
    QFETCH(int, expectedOutput);

    auto &f = *s_level2Decoder4to16;

    for (int i = 0; i < 4; ++i) {
        f.swAddr[i]->setOn((address >> i) & 1);
    }
    f.sim->update();

    int activeCount = 0;
    int activeIndex = -1;
    for (int i = 0; i < 16; ++i) {
        if (getInputStatus(f.ledOut[i])) {
            activeCount++;
            activeIndex = i;
        }
    }

    QCOMPARE(activeCount, 1);
    QCOMPARE(activeIndex, expectedOutput);
}

