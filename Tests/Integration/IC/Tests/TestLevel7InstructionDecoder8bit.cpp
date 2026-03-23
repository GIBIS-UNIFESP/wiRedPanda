// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel7InstructionDecoder8bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::setMultiBitInput;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel7InstructionDecoder8Bit::initTestCase()
{
}

void TestLevel7InstructionDecoder8Bit::cleanup()
{
}

void TestLevel7InstructionDecoder8Bit::testInstructionDecoder8Bit_data()
{
    QTest::addColumn<int>("instructionCode");
    QTest::addColumn<int>("expectedOutputLine");

    // Test representative instruction codes (not all 256)
    // Test special values
    QTest::newRow("instr 0x00 (all zeros)") << 0x00 << 0;
    QTest::newRow("instr 0xFF (all ones)") << 0xFF << 255;

    // Test single-bit patterns
    QTest::newRow("instr 0x01 (bit 0)") << 0x01 << 1;
    QTest::newRow("instr 0x02 (bit 1)") << 0x02 << 2;
    QTest::newRow("instr 0x04 (bit 2)") << 0x04 << 4;
    QTest::newRow("instr 0x08 (bit 3)") << 0x08 << 8;
    QTest::newRow("instr 0x10 (bit 4)") << 0x10 << 16;
    QTest::newRow("instr 0x20 (bit 5)") << 0x20 << 32;
    QTest::newRow("instr 0x40 (bit 6)") << 0x40 << 64;
    QTest::newRow("instr 0x80 (bit 7)") << 0x80 << 128;

    // Test mixed patterns
    QTest::newRow("instr 0x42 (0x40 | 0x02)") << 0x42 << 66;
    QTest::newRow("instr 0x81 (0x80 | 0x01)") << 0x81 << 129;
    QTest::newRow("instr 0xAA (alternating 1s)") << 0xAA << 170;
    QTest::newRow("instr 0x55 (alternating 0s)") << 0x55 << 85;

    // Test mid-range values
    QTest::newRow("instr 0x80 (128)") << 0x80 << 128;
    QTest::newRow("instr 0x7F (127)") << 0x7F << 127;
    QTest::newRow("instr 0xF0 (240)") << 0xF0 << 240;
    QTest::newRow("instr 0x0F (15)") << 0x0F << 15;
}

void TestLevel7InstructionDecoder8Bit::testInstructionDecoder8Bit()
{
    QFETCH(int, instructionCode);
    QFETCH(int, expectedOutputLine);

    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create instruction input switches (8-bit)
    QVector<InputSwitch *> instr_inputs;
    for (int i = 0; i < 8; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("instr[%1]").arg(i));
        sw->setPos(50 + i * 60, 100);
        instr_inputs.append(sw);
    }

    // Load the instruction decoder IC
    IC *decoder = loadBuildingBlockIC("level7_instruction_decoder_8bit.panda");
    builder.add(decoder);

    // Create output LEDs for selected instruction outputs (test 4 representative ones)
    // We can't create all 256 LEDs easily, so test the key ones for this test case
    QVector<Led *> op_outputs;

    // Create LEDs for instruction 0, 127, 128, 255, and the expected output
    QSet<int> test_indices;
    test_indices.insert(0);
    test_indices.insert(127);
    test_indices.insert(128);
    test_indices.insert(255);
    test_indices.insert(expectedOutputLine);

    int led_col = 0;
    for (int op_index : test_indices) {
        Led *led = new Led();
        builder.add(led);
        led->setLabel(QString("op[%1]").arg(op_index));
        led->setPos(400 + led_col * 60, 100);
        op_outputs.append(led);

        // Connect decoder output to LED
        builder.connect(decoder, QString("op[%1]").arg(op_index), led, 0);

        led_col++;
    }

    // Connect all instruction inputs to decoder
    for (int i = 0; i < 8; i++) {
        builder.connect(instr_inputs[i], 0, decoder, QString("instr[%1]").arg(i));
    }

    Simulation *sim = builder.initSimulation();
    sim->update();

    // Set instruction code
    setMultiBitInput(instr_inputs, instructionCode);
    sim->update();

    // Verify that ONLY the expected output line is active
    for (int op_index : std::as_const(test_indices)) {
        bool is_active = TestUtils::getInputStatus(
            op_outputs[test_indices.values().indexOf(op_index)], 0
        );

        if (op_index == expectedOutputLine) {
            QVERIFY2(is_active, qPrintable(QString("Expected op[%1] to be active for instruction 0x%2")
                .arg(op_index).arg(instructionCode, 0, 16)));
        } else {
            QVERIFY2(!is_active, qPrintable(QString("Expected op[%1] to be inactive for instruction 0x%2")
                .arg(op_index).arg(instructionCode, 0, 16)));
        }
    }
}

void TestLevel7InstructionDecoder8Bit::testInstructionDecoder8BitStructure()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *decoder = loadBuildingBlockIC("level7_instruction_decoder_8bit.panda");
    builder.add(decoder);

    // Verify IC has been loaded
    QVERIFY(decoder != nullptr);

    // Create dummy elements for connection testing
    InputSwitch *data = new InputSwitch();
    builder.add(data);

    Led *out = new Led();
    builder.add(out);

    // Verify IC port structure
    // Inputs: 8 instr bits = 8 inputs
    // Outputs: 256 op lines = 256 outputs
    QCOMPARE(decoder->inputSize(), 8);     // 8 instruction bits
    QCOMPARE(decoder->outputSize(), 256);  // 256 possible instructions

    // Verify port connections work (don't throw)
    try {
        builder.connect(data, 0, decoder, "instr[0]");
        builder.connect(data, 0, decoder, "instr[7]");
        builder.connect(decoder, "op[0]", out, 0);
        builder.connect(decoder, "op[255]", out, 0);
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("IC port access failed: %1").arg(e.what())));
    }
}

