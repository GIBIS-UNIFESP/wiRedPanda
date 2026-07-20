// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/IC/TestLevel5ClockGatedDecoder.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"
#include "Tests/QuickShell/QuickCpuTestUtils.h"

using TestUtils::inputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct ClockGatedDecoderFixture {
    std::unique_ptr<QuickCircuitBuilder> builder;
    IC *ic = nullptr;
    InputSwitch *addressBits[3] = {};
    InputSwitch *clockGate = nullptr;
    InputSwitch *writeEnable = nullptr;
    Led *outputs[8] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        builder = std::make_unique<QuickCircuitBuilder>();

        for (int i = 0; i < 3; ++i) {
            addressBits[i] = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
        }
        clockGate = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
        writeEnable = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));

        for (int i = 0; i < 8; ++i) {
            outputs[i] = static_cast<Led *>(builder->addOwnedElement(new Led()));
        }

        ic = static_cast<IC *>(builder->addOwnedElement(loadBuildingBlockIC("level5_clock_gated_decoder.panda")));

        for (int i = 0; i < 3; ++i) {
            builder->connect(addressBits[i], 0, ic, QString("addr%1").arg(i));
        }
        builder->connect(clockGate, 0, ic, "Clock");
        builder->connect(writeEnable, 0, ic, "writeEnable");

        for (int i = 0; i < 8; ++i) {
            builder->connect(ic, QString("out%1").arg(i), outputs[i], 0);
        }

        sim = builder->initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<ClockGatedDecoderFixture> s_level5ClockGatedDecoder;

void TestLevel5ClockGatedDecoder::initTestCase()
{
    s_level5ClockGatedDecoder = std::make_unique<ClockGatedDecoderFixture>();
    QVERIFY(s_level5ClockGatedDecoder->build());
}

void TestLevel5ClockGatedDecoder::cleanupTestCase()
{
    s_level5ClockGatedDecoder.reset();
}

void TestLevel5ClockGatedDecoder::cleanup()
{
    if (s_level5ClockGatedDecoder && s_level5ClockGatedDecoder->sim) {
        s_level5ClockGatedDecoder->sim->restart();
        s_level5ClockGatedDecoder->sim->update();
    }
}

// ============================================================
// Clock Gated Decoder Tests
// ============================================================

void TestLevel5ClockGatedDecoder::testClockGatedDecoder_data()
{
    QTest::addColumn<int>("addressValue");
    QTest::addColumn<bool>("clockEnable");
    QTest::addColumn<int>("expectedActiveOutput");

    // Test with clock enabled (gate open) - 3-to-8 decoder
    QTest::newRow("address_000_enabled") << 0 << true << 0;
    QTest::newRow("address_001_enabled") << 1 << true << 1;
    QTest::newRow("address_010_enabled") << 2 << true << 2;
    QTest::newRow("address_011_enabled") << 3 << true << 3;
    QTest::newRow("address_100_enabled") << 4 << true << 4;
    QTest::newRow("address_101_enabled") << 5 << true << 5;
    QTest::newRow("address_110_enabled") << 6 << true << 6;
    QTest::newRow("address_111_enabled") << 7 << true << 7;

    // Test with clock disabled (gate closed) - no outputs should be active
    QTest::newRow("address_000_disabled") << 0 << false << -1;
    QTest::newRow("address_111_disabled") << 7 << false << -1;

    // Test sequential scan with gate control
    QTest::newRow("sequential_scan") << -1 << true << -1;
}

void TestLevel5ClockGatedDecoder::testClockGatedDecoder()
{
    QFETCH(int, addressValue);
    QFETCH(bool, clockEnable);
    QFETCH(int, expectedActiveOutput);

    auto &f = *s_level5ClockGatedDecoder;

    if (addressValue == -1) {
        // Sequential scan test with gate enabled
        f.clockGate->setOn(true);
        f.writeEnable->setOn(true);
        f.sim->update();

        for (int addr = 0; addr < 8; addr++) {
            for (int i = 0; i < 3; ++i) {
                f.addressBits[i]->setOn((addr >> i) & 1);
            }
            f.sim->update();

            int activeCount = 0;
            int activeOutput = -1;
            for (int i = 0; i < 8; i++) {
                if (inputStatus(f.outputs[i])) {
                    activeCount++;
                    activeOutput = i;
                }
            }
            QCOMPARE(activeCount, 1);
            QCOMPARE(activeOutput, addr);
        }
    } else {
        for (int i = 0; i < 3; ++i) {
            f.addressBits[i]->setOff();
        }
        f.clockGate->setOn(clockEnable);
        f.writeEnable->setOn(true);
        f.sim->update();

        for (int i = 0; i < 3; ++i) {
            f.addressBits[i]->setOn((addressValue >> i) & 1);
        }
        f.sim->update();

        if (clockEnable) {
            for (int i = 0; i < 8; i++) {
                bool expectedState = (i == expectedActiveOutput);
                QCOMPARE(inputStatus(f.outputs[i]), expectedState);
            }
        } else {
            for (int i = 0; i < 8; i++) {
                QCOMPARE(inputStatus(f.outputs[i]), false);
            }
        }
    }
}

// The decoder is gated by clock AND writeEnable. The data-driven test above
// exercises the clock=0 path; this covers the symmetric writeEnable=0 path:
// with the clock high and a valid address, deasserting writeEnable must still
// force every output low, and reasserting it restores the one-hot decode.
void TestLevel5ClockGatedDecoder::testWriteEnableGating()
{
    auto &f = *s_level5ClockGatedDecoder;

    f.clockGate->setOn(true);
    for (int i = 0; i < 3; ++i) {
        f.addressBits[i]->setOn((5 >> i) & 1);  // address = 5
    }

    // writeEnable=0: all outputs low despite clock high and a valid address
    f.writeEnable->setOn(false);
    f.sim->update();
    for (int i = 0; i < 8; i++) {
        QCOMPARE(inputStatus(f.outputs[i]), false);
    }

    // writeEnable=1: output 5 active (one-hot decode restored)
    f.writeEnable->setOn(true);
    f.sim->update();
    for (int i = 0; i < 8; i++) {
        QCOMPARE(inputStatus(f.outputs[i]), i == 5);
    }
}
