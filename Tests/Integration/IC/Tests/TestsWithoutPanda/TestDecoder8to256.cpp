// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestsWithoutPanda/TestDecoder8to256.h"

#include <QFileInfo>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Not.h"
#include "Tests/Common/TestUtils.h"

using TestUtils::getInputStatus;

namespace {

// Builds an 8-to-256 decoder circuit: 8 inputs, 8 NOTs, 256 8-input ANDs, 256 LEDs.
// setAddress() sets the address bits and calls simulation->update().
struct Decoder8To256Fixture {
    WorkSpace workspace;
    CircuitBuilder builder;
    InputSwitch swAddr[8];
    Not notGates[8];
    And andGates[256];
    Led ledOut[256];
    Simulation *simulation = nullptr;

    Decoder8To256Fixture()
        : builder(workspace.scene())
    {
        for (int i = 0; i < 8; ++i)
            builder.add(&swAddr[i], &notGates[i]);

        for (int i = 0; i < 256; ++i) {
            andGates[i].setInputSize(8);
            builder.add(&andGates[i], &ledOut[i]);
        }

        for (int i = 0; i < 8; ++i)
            builder.connect(&swAddr[i], 0, &notGates[i], 0);

        for (int addr = 0; addr < 256; ++addr) {
            And *andGate = &andGates[addr];
            for (int bit = 0; bit < 8; ++bit) {
                bool bitSet = (addr & (1 << bit)) != 0;
                if (bitSet)
                    builder.connect(&swAddr[bit], 0, andGate, bit);
                else
                    builder.connect(&notGates[bit], 0, andGate, bit);
            }
            builder.connect(andGate, 0, &ledOut[addr], 0);
        }

        simulation = builder.initSimulation();
    }

    void setAddress(int address)
    {
        for (int i = 0; i < 8; ++i)
            swAddr[i].setOn((address >> i) & 1);
        simulation->update();
    }
};
} // namespace

void TestDecoder8To256::initTestCase()
{
    TestUtils::setupTestEnvironment();
}

// ============================================================
// 8-to-256 Decoder Tests (Full Address Range Memory Decoder)
// ============================================================

/**
 * Test: 8-to-256 Address Decoder
 *
 * This decoder expands memory addressing from 3-bit (8 locations)
 * to full 8-bit (256 locations) for larger address spaces.
 *
 * Architecture:
 * - Input: 8 address bits (A0-A7)
 * - Output: 256 one-hot decoded lines
 * - Application: Memory address decoding for 256-location RAM
 *
 * Test Strategy:
 * - Create full decoder with 256 outputs
 * - Test representative address patterns
 * - Verify one-hot property (only one output active per address)
 */

void TestDecoder8To256::test8to256Decoder_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("expectedActive");

    // Test boundary addresses
    QTest::newRow("address 0x00 (min)") << 0x00 << 0x00;
    QTest::newRow("address 0xFF (max)") << 0xFF << 0xFF;

    // Test power-of-2 addresses
    QTest::newRow("address 0x01") << 0x01 << 0x01;
    QTest::newRow("address 0x02") << 0x02 << 0x02;
    QTest::newRow("address 0x04") << 0x04 << 0x04;
    QTest::newRow("address 0x08") << 0x08 << 0x08;
    QTest::newRow("address 0x10") << 0x10 << 0x10;
    QTest::newRow("address 0x20") << 0x20 << 0x20;
    QTest::newRow("address 0x40") << 0x40 << 0x40;
    QTest::newRow("address 0x80") << 0x80 << 0x80;

    // Test alternating bit patterns
    QTest::newRow("address 0x55 (alternating 01010101)") << 0x55 << 0x55;
    QTest::newRow("address 0xAA (alternating 10101010)") << 0xAA << 0xAA;

    // Test mid-range addresses
    QTest::newRow("address 0x42") << 0x42 << 0x42;
    QTest::newRow("address 0x7F") << 0x7F << 0x7F;
}

void TestDecoder8To256::test8to256Decoder()
{
    QFETCH(int, address);
    QFETCH(int, expectedActive);

    Decoder8To256Fixture f;
    f.setAddress(address);

    // Verify only the expected output is active (one-hot property)
    for (int i = 0; i < 256; ++i) {
        bool expected = (i == expectedActive);
        QCOMPARE(getInputStatus(&f.ledOut[i]), expected);
    }
}

void TestDecoder8To256::test8to256DecoderAddressSpace_data()
{
    QTest::addColumn<int>("address");

    // Test a representative sample of the full 256-address space
    // to verify the decoder handles the complete range correctly
    QTest::newRow("low_byte_0x00") << 0x00;
    QTest::newRow("low_byte_0x0F") << 0x0F;
    QTest::newRow("high_byte_low_0x10") << 0x10;
    QTest::newRow("high_byte_mid_0x7F") << 0x7F;
    QTest::newRow("high_byte_mid_0x80") << 0x80;
    QTest::newRow("high_byte_high_0xF0") << 0xF0;
    QTest::newRow("high_byte_max_0xFF") << 0xFF;
}

void TestDecoder8To256::test8to256DecoderAddressSpace()
{
    QFETCH(int, address);

    Decoder8To256Fixture f;
    f.setAddress(address);

    // Verify that the decoder correctly selects the address
    // and that only one output is active
    bool foundActive = false;
    int activeCount = 0;

    for (int i = 0; i < 256; ++i) {
        if (getInputStatus(&f.ledOut[i])) {
            foundActive = true;
            activeCount++;
            QCOMPARE(i, address);  // The active output should match the input address
        }
    }

    QVERIFY2(foundActive, "No output was active for the given address");
    QCOMPARE(activeCount, 1);  // Exactly one output should be active (one-hot property)
}

void TestDecoder8To256::test8to256DecoderOneHot_data()
{
    QTest::addColumn<int>("address");

    // Test that verifies the one-hot property:
    // For any address, exactly one output should be active
    QTest::newRow("test_onehot_0x00") << 0x00;
    QTest::newRow("test_onehot_0x15") << 0x15;
    QTest::newRow("test_onehot_0x2A") << 0x2A;
    QTest::newRow("test_onehot_0x55") << 0x55;
    QTest::newRow("test_onehot_0xAA") << 0xAA;
    QTest::newRow("test_onehot_0xF0") << 0xF0;
    QTest::newRow("test_onehot_0xFF") << 0xFF;
}

void TestDecoder8To256::test8to256DecoderOneHot()
{
    QFETCH(int, address);

    Decoder8To256Fixture f;
    f.setAddress(address);

    // Verify one-hot property: exactly one output active
    int activeCount = 0;
    int activeAddress = -1;

    for (int i = 0; i < 256; ++i) {
        if (getInputStatus(&f.ledOut[i])) {
            activeCount++;
            activeAddress = i;
        }
    }

    // Verify exactly one output is active
    QCOMPARE(activeCount, 1);
    // Verify it's the correct output for the given address
    QCOMPARE(activeAddress, address);
}
