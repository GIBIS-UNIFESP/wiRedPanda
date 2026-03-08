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

void TestDecoder8To256::test8to256Decoder()
{
    Decoder8To256Fixture f;

    // Test addresses: boundaries, powers of 2, alternating patterns, mid-range
    const int addresses[] = {
        0x00, 0xFF,                         // boundary
        0x01, 0x02, 0x04, 0x08,             // power-of-2
        0x10, 0x20, 0x40, 0x80,
        0x55, 0xAA,                         // alternating bits
        0x0F, 0x15, 0x2A, 0x42, 0x7F, 0xF0 // mid-range
    };

    for (int address : addresses) {
        f.setAddress(address);

        // Verify one-hot property: exactly one output active at the correct address
        int activeCount = 0;
        for (int i = 0; i < 256; ++i) {
            if (getInputStatus(&f.ledOut[i])) {
                ++activeCount;
                QCOMPARE(i, address);
            }
        }
        QCOMPARE(activeCount, 1);
    }
}

