// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel5ClockGatedDecoder.h"

#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel5ClockGatedDecoder::initTestCase()
{
    // Initialize test environment
}

void TestLevel5ClockGatedDecoder::cleanup()
{
    // Clean up after each test
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

    // Create circuit using ClockGatedDecoder IC (3-to-8 decoder)
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch addressBits[3];  // 3 address bits for 3-to-8 decoder
    InputSwitch clockGate;
    InputSwitch writeEnable;
    Led outputs[8];  // 8 outputs for 3-to-8 decoder

    for (int i = 0; i < 3; i++) {
        builder.add(&addressBits[i]);
    }
    builder.add(&clockGate, &writeEnable);
    for (int i = 0; i < 8; i++) {
        builder.add(&outputs[i]);
    }

    // Load ClockGatedDecoder IC
    IC *decoderIC = loadBuildingBlockIC("level5_clock_gated_decoder.panda");
    builder.add(decoderIC);

    // Connect address inputs to decoder IC (addr0, addr1, addr2 - no brackets)
    for (int i = 0; i < 3; i++) {
        builder.connect(&addressBits[i], 0, decoderIC, QString("addr%1").arg(i));
    }
    builder.connect(&clockGate, 0, decoderIC, "clock");
    builder.connect(&writeEnable, 0, decoderIC, "writeEnable");

    // Connect decoder outputs
    for (int i = 0; i < 8; i++) {
        builder.connect(decoderIC, QString("out%1").arg(i), &outputs[i], 0);
    }

    Simulation *simulation = builder.initSimulation();

    if (addressValue == -1) {
        // Sequential scan test with gate enabled
        clockGate.setOn(true);
        writeEnable.setOn(true);
        simulation->update();

        for (int addr = 0; addr < 8; addr++) {
            bool bit0 = (addr >> 0) & 1;
            bool bit1 = (addr >> 1) & 1;
            bool bit2 = (addr >> 2) & 1;
            addressBits[0].setOn(bit0);
            addressBits[1].setOn(bit1);
            addressBits[2].setOn(bit2);
            simulation->update();

            // Verify only one output is active
            int activeCount = 0;
            int activeOutput = -1;
            for (int i = 0; i < 8; i++) {
                if (TestUtils::getInputStatus(&outputs[i])) {
                    activeCount++;
                    activeOutput = i;
                }
            }
            QCOMPARE(activeCount, 1);
            QCOMPARE(activeOutput, addr);
        }
    } else {
        // Single address test with gate control
        addressBits[0].setOff();
        addressBits[1].setOff();
        addressBits[2].setOff();
        clockGate.setOn(clockEnable);
        writeEnable.setOn(true);
        simulation->update();

        // Set the address using direct bit setting
        bool bit0 = (addressValue >> 0) & 1;
        bool bit1 = (addressValue >> 1) & 1;
        bool bit2 = (addressValue >> 2) & 1;
        addressBits[0].setOn(bit0);
        addressBits[1].setOn(bit1);
        addressBits[2].setOn(bit2);

        simulation->update();

        if (clockEnable) {
            // Clock enabled: expected output should be active
            for (int i = 0; i < 8; i++) {
                bool expectedState = (i == expectedActiveOutput);
                QCOMPARE(TestUtils::getInputStatus(&outputs[i]), expectedState);
            }
        } else {
            // Clock disabled: all outputs should be inactive
            for (int i = 0; i < 8; i++) {
                QCOMPARE(TestUtils::getInputStatus(&outputs[i]), false);
            }
        }
    }
}

