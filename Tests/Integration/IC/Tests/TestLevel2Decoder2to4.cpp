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
        for (int i = 0; i < 4; ++i) {
            outputs[i] = new Led();
            builder.add(outputs[i]);
        }

        ic = loadBuildingBlockIC("level2_decoder_2to4.panda");
        builder.add(ic);

        builder.connect(addressBits[0], 0, ic, "addr[0]");
        builder.connect(addressBits[1], 0, ic, "addr[1]");
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
    QTest::addColumn<int>("expectedActiveOutput");

    // Test 1: Address 00 → Output 0 active
    QTest::newRow("address_00") << 0 << 0;

    // Test 2: Address 01 → Output 1 active
    QTest::newRow("address_01") << 1 << 1;

    // Test 3: Address 10 → Output 2 active
    QTest::newRow("address_10") << 2 << 2;

    // Test 4: Address 11 → Output 3 active
    QTest::newRow("address_11") << 3 << 3;
}

void TestLevel2Decoder2To4::testAddressDecoder2to4()
{
    QFETCH(int, addressValue);
    QFETCH(int, expectedActiveOutput);

    auto &f = *s_level2Decoder2to4;

    f.addressBits[0]->setOff();
    f.addressBits[1]->setOff();
    f.sim->update();

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

