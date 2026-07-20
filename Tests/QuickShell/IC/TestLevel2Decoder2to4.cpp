// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/IC/TestLevel2Decoder2to4.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Integration/IC/Tests/Cpu/CpuCommon.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"
#include "Tests/QuickShell/QuickCpuTestUtils.h"

using TestUtils::inputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct Decoder2to4Fixture {
    std::unique_ptr<QuickCircuitBuilder> builder;
    IC *ic = nullptr;
    InputSwitch *addressBits[2] = {};
    Led *outputs[4] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        builder = std::make_unique<QuickCircuitBuilder>();

        for (int i = 0; i < 2; ++i) {
            addressBits[i] = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
        }
        for (int i = 0; i < 4; ++i) {
            outputs[i] = static_cast<Led *>(builder->addOwnedElement(new Led()));
        }

        ic = static_cast<IC *>(builder->addOwnedElement(loadBuildingBlockIC("level2_decoder_2to4.panda")));

        builder->connect(addressBits[0], 0, ic, "addr[0]");
        builder->connect(addressBits[1], 0, ic, "addr[1]");
        for (int i = 0; i < 4; ++i) {
            builder->connect(ic, QString("out[%1]").arg(i), outputs[i], 0);
        }

        sim = builder->initSimulation();
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
        QCOMPARE(inputStatus(f.outputs[i]), expectedState);
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
            if (inputStatus(f.outputs[i])) {
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
            if (inputStatus(f.outputs[i])) {
                activeCount++;
            }
        }
        QCOMPARE(activeCount, 1);
    }
}

// Active-high Enable (74138-style chip select): Enable=0 forces every output
// low; Enable=1 restores normal decoding. Built standalone with Enable wired so
// the shared fixture (which leaves Enable unconnected → defaulted high) is
// untouched.
void TestLevel2Decoder2To4::testEnableGating()
{
    auto builder = std::make_unique<QuickCircuitBuilder>();

    auto *a0 = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
    auto *a1 = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
    auto *en = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
    Led *outs[4] = {};
    for (auto &o : outs) {
        o = static_cast<Led *>(builder->addOwnedElement(new Led()));
    }

    auto *ic = static_cast<IC *>(builder->addOwnedElement(loadBuildingBlockIC("level2_decoder_2to4.panda")));
    builder->connect(a0, 0, ic, "addr[0]");
    builder->connect(a1, 0, ic, "addr[1]");
    builder->connect(en, 0, ic, "Enable");
    for (int i = 0; i < 4; ++i) {
        builder->connect(ic, QString("out[%1]").arg(i), outs[i], 0);
    }
    auto *sim = builder->initSimulation();

    // Address 2 selected, Enable low → all outputs forced low.
    a0->setOn(false);
    a1->setOn(true);
    en->setOn(false);
    sim->update();
    for (auto *o : outs) {
        QVERIFY(!inputStatus(o));
    }

    // Enable high → output 2 active, the rest low.
    en->setOn(true);
    sim->update();
    for (int i = 0; i < 4; ++i) {
        QCOMPARE(inputStatus(outs[i]), i == 2);
    }
}
