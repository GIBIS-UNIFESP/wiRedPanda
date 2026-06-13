// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel2Decoder3to8.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct Decoder3to8Fixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *swAddr[3] = {};
    InputSwitch *enable = nullptr;
    Led *ledOut[8] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        for (int i = 0; i < 3; ++i) {
            swAddr[i] = new InputSwitch();
            builder.add(swAddr[i]);
        }
        enable = new InputSwitch();
        builder.add(enable);
        for (int i = 0; i < 8; ++i) {
            ledOut[i] = new Led();
            builder.add(ledOut[i]);
        }

        ic = loadBuildingBlockIC("level2_decoder_3to8.panda");
        builder.add(ic);

        builder.connect(swAddr[0], 0, ic, "addr[0]");
        builder.connect(swAddr[1], 0, ic, "addr[1]");
        builder.connect(swAddr[2], 0, ic, "addr[2]");
        builder.connect(enable, 0, ic, "enable");

        for (int i = 0; i < 8; ++i) {
            builder.connect(ic, QString("out[%1]").arg(i), ledOut[i], 0);
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<Decoder3to8Fixture> s_level2Decoder3to8;

void TestLevel2Decoder3To8::initTestCase()
{
    s_level2Decoder3to8 = std::make_unique<Decoder3to8Fixture>();
    QVERIFY(s_level2Decoder3to8->build());
}

void TestLevel2Decoder3To8::cleanupTestCase()
{
    s_level2Decoder3to8.reset();
}

void TestLevel2Decoder3To8::cleanup()
{
    if (s_level2Decoder3to8 && s_level2Decoder3to8->sim) {
        s_level2Decoder3to8->sim->restart();
        s_level2Decoder3to8->sim->update();
    }
}

// ============================================================
// 3-to-8 Decoder Tests
// ============================================================

void TestLevel2Decoder3To8::test3to8Decoder_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("expectedActive"); // -1 = none active

    // enable=1: address N activates output N
    for (int a = 0; a < 8; ++a) {
        QTest::newRow(qPrintable(QString("en address %1").arg(a))) << a << true << a;
    }
    // enable=0: every output forced low, regardless of address
    for (int a = 0; a < 8; ++a) {
        QTest::newRow(qPrintable(QString("dis address %1").arg(a))) << a << false << -1;
    }
}

void TestLevel2Decoder3To8::test3to8Decoder()
{
    QFETCH(int, address);
    QFETCH(bool, enabled);
    QFETCH(int, expectedActive);

    auto &f = *s_level2Decoder3to8;

    f.enable->setOn(enabled);
    for (int i = 0; i < 3; ++i) {
        f.swAddr[i]->setOn((address >> i) & 1);
    }
    f.sim->update();

    for (int i = 0; i < 8; ++i) {
        bool expected = (i == expectedActive);
        QCOMPARE(getInputStatus(f.ledOut[i]), expected);
    }
}
