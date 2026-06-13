// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel2Decoder2to4.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/Cpu/CpuCommon.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct Decoder2to4Fixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *addressBits[2] = {};
    InputSwitch *enable = nullptr;
    Led *outputs[4] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        for (int i = 0; i < 2; ++i) {
            addressBits[i] = new InputSwitch();
            builder.add(addressBits[i]);
        }
        enable = new InputSwitch();
        builder.add(enable);
        for (int i = 0; i < 4; ++i) {
            outputs[i] = new Led();
            builder.add(outputs[i]);
        }

        ic = loadBuildingBlockIC("level2_decoder_2to4.panda");
        builder.add(ic);

        builder.connect(addressBits[0], 0, ic, "addr[0]");
        builder.connect(addressBits[1], 0, ic, "addr[1]");
        builder.connect(enable, 0, ic, "enable");
        for (int i = 0; i < 4; ++i) {
            builder.connect(ic, QString("out[%1]").arg(i), outputs[i], 0);
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<Decoder2to4Fixture> s_level2Decoder2to4;

void TestLevel2Decoder2To4::initTestCase()
{
    s_level2Decoder2to4 = std::make_unique<Decoder2to4Fixture>();
    QVERIFY(s_level2Decoder2to4->build());
}

void TestLevel2Decoder2To4::cleanupTestCase()
{
    s_level2Decoder2to4.reset();
}

void TestLevel2Decoder2To4::cleanup()
{
    if (s_level2Decoder2to4 && s_level2Decoder2to4->sim) {
        s_level2Decoder2to4->sim->restart();
        s_level2Decoder2to4->sim->update();
    }
}

// ============================================================
// Address Decoder (2→4) Tests
// ============================================================

void TestLevel2Decoder2To4::testAddressDecoder2to4_data()
{
    QTest::addColumn<int>("addressValue");
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("expectedActiveOutput"); // -1 = none active

    // enable=1: address N activates output N
    QTest::newRow("en address_00") << 0 << true << 0;
    QTest::newRow("en address_01") << 1 << true << 1;
    QTest::newRow("en address_10") << 2 << true << 2;
    QTest::newRow("en address_11") << 3 << true << 3;

    // enable=0: every output forced low, regardless of address
    QTest::newRow("dis address_00") << 0 << false << -1;
    QTest::newRow("dis address_01") << 1 << false << -1;
    QTest::newRow("dis address_10") << 2 << false << -1;
    QTest::newRow("dis address_11") << 3 << false << -1;
}

void TestLevel2Decoder2To4::testAddressDecoder2to4()
{
    QFETCH(int, addressValue);
    QFETCH(bool, enabled);
    QFETCH(int, expectedActiveOutput);

    auto &f = *s_level2Decoder2to4;

    f.enable->setOn(enabled);
    f.addressBits[0]->setOn((addressValue >> 0) & 1);
    f.addressBits[1]->setOn((addressValue >> 1) & 1);
    f.sim->update();

    for (int i = 0; i < 4; i++) {
        bool expectedState = (i == expectedActiveOutput);
        QCOMPARE(getInputStatus(f.outputs[i]), expectedState);
    }
}

void TestLevel2Decoder2To4::testSequentialScan()
{
    auto &f = *s_level2Decoder2to4;
    f.enable->setOn(true);

    for (int addr = 0; addr < 4; addr++) {
        f.addressBits[0]->setOn((addr >> 0) & 1);
        f.addressBits[1]->setOn((addr >> 1) & 1);
        f.sim->update();

        int activeCount = 0;
        int activeOutput = -1;
        for (int i = 0; i < 4; i++) {
            if (getInputStatus(f.outputs[i])) {
                activeCount++;
                activeOutput = i;
            }
        }
        QCOMPARE(activeCount, 1);
        QCOMPARE(activeOutput, addr);
    }
}

void TestLevel2Decoder2To4::testMutualExclusivity()
{
    auto &f = *s_level2Decoder2to4;
    f.enable->setOn(true);

    for (int addr = 0; addr < 4; addr++) {
        f.addressBits[0]->setOn((addr >> 0) & 1);
        f.addressBits[1]->setOn((addr >> 1) & 1);
        f.sim->update();

        int activeCount = 0;
        for (int i = 0; i < 4; i++) {
            if (getInputStatus(f.outputs[i])) {
                activeCount++;
            }
        }
        QCOMPARE(activeCount, 1);
    }
}
