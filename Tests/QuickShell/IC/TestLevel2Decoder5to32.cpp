// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/IC/TestLevel2Decoder5to32.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/QuickShell/IC/CpuTestUtils.h"
#include "Tests/QuickShell/IC/QuickTestUtils.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"

using QuickTestUtils::inputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct Decoder5to32Fixture {
    std::unique_ptr<QuickCircuitBuilder> builder;
    IC *ic = nullptr;
    InputSwitch *swAddr[5] = {};
    Led *ledOut[32] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        builder = std::make_unique<QuickCircuitBuilder>();

        for (int i = 0; i < 5; ++i) {
            swAddr[i] = new InputSwitch();
            builder->addOwnedElement(swAddr[i]);
        }
        for (int i = 0; i < 32; ++i) {
            ledOut[i] = new Led();
            builder->addOwnedElement(ledOut[i]);
        }

        ic = loadBuildingBlockIC("level2_decoder_5to32.panda");
        builder->addOwnedElement(ic);

        for (int i = 0; i < 5; ++i) {
            builder->connect(swAddr[i], 0, ic, QString("addr[%1]").arg(i));
        }
        for (int i = 0; i < 32; ++i) {
            builder->connect(ic, QString("out[%1]").arg(i), ledOut[i], 0);
        }

        sim = builder->initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<Decoder5to32Fixture> s_level2Decoder5to32;

void TestLevel2Decoder5To32::initTestCase()
{
    s_level2Decoder5to32 = std::make_unique<Decoder5to32Fixture>();
    QVERIFY(s_level2Decoder5to32->build());
}

void TestLevel2Decoder5To32::cleanupTestCase()
{
    s_level2Decoder5to32.reset();
}

void TestLevel2Decoder5To32::cleanup()
{
    if (s_level2Decoder5to32 && s_level2Decoder5to32->sim) {
        s_level2Decoder5to32->sim->restart();
        s_level2Decoder5to32->sim->update();
    }
}

// ============================================================
// 5-to-32 Decoder Tests
// ============================================================

/**
 * Test: 5-to-32 Decoder (5-bit address -> 32 one-hot outputs)
 * Selects one of 32 output lines based on 5-bit address input
 */
void TestLevel2Decoder5To32::test5to32Decoder_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("expectedActive");

    // For address N, output N should be active (1), others inactive (0)
    for (int i = 0; i < 32; ++i) {
        QTest::newRow(QString("address %1").arg(i).toLatin1().data()) << i << i;
    }
}

void TestLevel2Decoder5To32::test5to32Decoder()
{
    QFETCH(int, address);
    QFETCH(int, expectedActive);

    auto &f = *s_level2Decoder5to32;

    for (int i = 0; i < 5; ++i) {
        f.swAddr[i]->setOn((address >> i) & 1);
    }
    f.sim->update();

    for (int i = 0; i < 32; ++i) {
        bool actual = inputStatus(f.ledOut[i]);
        bool expected = (i == expectedActive);
        QCOMPARE(actual, expected);
    }
}

// Active-high Enable (74138-style chip select): Enable=0 forces every output
// low; Enable=1 restores normal decoding. Built standalone with Enable wired so
// the shared fixture (Enable unconnected → defaulted high) is untouched.
void TestLevel2Decoder5To32::testEnableGating()
{
    auto builder = std::make_unique<QuickCircuitBuilder>();

    InputSwitch *addr[5] = {};
    for (auto &a : addr) {
        a = new InputSwitch();
        builder->addOwnedElement(a);
    }
    auto *en = new InputSwitch();
    builder->addOwnedElement(en);
    Led *outs[32] = {};
    for (auto &o : outs) {
        o = new Led();
        builder->addOwnedElement(o);
    }

    auto *ic = loadBuildingBlockIC("level2_decoder_5to32.panda");
    builder->addOwnedElement(ic);
    for (int i = 0; i < 5; ++i) {
        builder->connect(addr[i], 0, ic, QString("addr[%1]").arg(i));
    }
    builder->connect(en, 0, ic, "Enable");
    for (int i = 0; i < 32; ++i) {
        builder->connect(ic, QString("out[%1]").arg(i), outs[i], 0);
    }
    auto *sim = builder->initSimulation();

    // Address 20 selected, Enable low → all outputs forced low.
    for (int i = 0; i < 5; ++i) {
        addr[i]->setOn((20 >> i) & 1);
    }
    en->setOn(false);
    sim->update();
    for (auto *o : outs) {
        QVERIFY(!inputStatus(o));
    }

    // Enable high → output 20 active, the rest low.
    en->setOn(true);
    sim->update();
    for (int i = 0; i < 32; ++i) {
        QCOMPARE(inputStatus(outs[i]), i == 20);
    }
}
