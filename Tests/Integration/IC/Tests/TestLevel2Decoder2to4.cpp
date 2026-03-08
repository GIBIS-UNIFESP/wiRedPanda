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

void TestLevel2Decoder2To4::initTestCase()
{
    // Initialize test environment
}

void TestLevel2Decoder2To4::cleanup()
{
    // Clean up after each test
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

    // Create circuit using Decoder2to4 IC
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch addressBits[2];
    Led out0, out1, out2, out3;

    builder.add(&addressBits[0], &addressBits[1], &out0, &out1, &out2, &out3);

    // Load Decoder2to4 IC
    IC *decoderIC = loadBuildingBlockIC("level2_decoder_2to4.panda");
    builder.add(decoderIC);

    // Connect address inputs to decoder IC using semantic labels
    builder.connect(&addressBits[0], 0, decoderIC, "addr[0]");
    builder.connect(&addressBits[1], 0, decoderIC, "addr[1]");

    // Connect decoder outputs using semantic labels
    builder.connect(decoderIC, "out[0]", &out0, 0);
    builder.connect(decoderIC, "out[1]", &out1, 0);
    builder.connect(decoderIC, "out[2]", &out2, 0);
    builder.connect(decoderIC, "out[3]", &out3, 0);

    Simulation *simulation = builder.initSimulation();

    Led *outputs[4] = {&out0, &out1, &out2, &out3};

    // Single address test
    // Initialize to 0 first
    addressBits[0].setOff();
    addressBits[1].setOff();
    simulation->update();

    // Now set the address using direct bit setting
    bool bit0 = (addressValue >> 0) & 1;
    bool bit1 = (addressValue >> 1) & 1;
    addressBits[0].setOn(bit0);
    addressBits[1].setOn(bit1);

    simulation->update();

    for (int i = 0; i < 4; i++) {
        bool expectedState = (i == expectedActiveOutput);
        QCOMPARE(TestUtils::getInputStatus(outputs[i]), expectedState);
    }
}

static Simulation *buildDecoder2to4Circuit(WorkSpace &ws,
                                            InputSwitch (&addressBits)[2],
                                            Led &out0, Led &out1, Led &out2, Led &out3)
{
    CircuitBuilder builder(ws.scene());
    builder.add(&addressBits[0], &addressBits[1], &out0, &out1, &out2, &out3);
    IC *decoderIC = loadBuildingBlockIC("level2_decoder_2to4.panda");
    builder.add(decoderIC);
    builder.connect(&addressBits[0], 0, decoderIC, "addr[0]");
    builder.connect(&addressBits[1], 0, decoderIC, "addr[1]");
    builder.connect(decoderIC, "out[0]", &out0, 0);
    builder.connect(decoderIC, "out[1]", &out1, 0);
    builder.connect(decoderIC, "out[2]", &out2, 0);
    builder.connect(decoderIC, "out[3]", &out3, 0);
    return builder.initSimulation();
}

void TestLevel2Decoder2To4::testSequentialScan()
{
    WorkSpace ws;
    InputSwitch addressBits[2];
    Led out0, out1, out2, out3;
    Simulation *sim = buildDecoder2to4Circuit(ws, addressBits, out0, out1, out2, out3);
    Led *outputs[4] = {&out0, &out1, &out2, &out3};

    for (int addr = 0; addr < 4; addr++) {
        addressBits[0].setOn((addr >> 0) & 1);
        addressBits[1].setOn((addr >> 1) & 1);
        sim->update();

        int activeCount = 0;
        int activeOutput = -1;
        for (int i = 0; i < 4; i++) {
            if (TestUtils::getInputStatus(outputs[i])) {
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
    WorkSpace ws;
    InputSwitch addressBits[2];
    Led out0, out1, out2, out3;
    Simulation *sim = buildDecoder2to4Circuit(ws, addressBits, out0, out1, out2, out3);
    Led *outputs[4] = {&out0, &out1, &out2, &out3};

    for (int addr = 0; addr < 4; addr++) {
        addressBits[0].setOn((addr >> 0) & 1);
        addressBits[1].setOn((addr >> 1) & 1);
        sim->update();

        int activeCount = 0;
        for (int i = 0; i < 4; i++) {
            if (TestUtils::getInputStatus(outputs[i])) {
                activeCount++;
            }
        }
        QCOMPARE(activeCount, 1);
    }
}

