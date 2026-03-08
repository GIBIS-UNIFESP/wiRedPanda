// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel5InstructionDecoder4bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::setMultiBitInput;
using TestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

void TestLevel5InstructionDecoder4Bit::initTestCase()
{
}

void TestLevel5InstructionDecoder4Bit::cleanup()
{
}

void TestLevel5InstructionDecoder4Bit::testInstructionDecoder4Bit_data()
{
    QTest::addColumn<int>("instructionCode");
    QTest::addColumn<int>("expectedActiveOutput");

    // Test all 16 possible 4-bit instruction codes
    for (int instr = 0; instr < 16; instr++) {
        QTest::newRow(qPrintable(QString("decode 0x%1").arg(instr, 1, 16)))
            << instr << instr;
    }
}

void TestLevel5InstructionDecoder4Bit::testInstructionDecoder4Bit()
{
    QFETCH(int, instructionCode);
    QFETCH(int, expectedActiveOutput);

    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create 4-bit instruction input
    QVector<InputSwitch *> instr_inputs;
    for (int i = 0; i < 4; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("instr[%1]").arg(i));
        sw->setPos(50 + i * 60, 100);
        instr_inputs.append(sw);
    }

    // Load the decoder IC
    IC *decoder = loadBuildingBlockIC("level5_instruction_decoder_4bit.panda");
    builder.add(decoder);

    // Create output LEDs for all 16 instruction lines
    QVector<Led *> op_outputs;
    for (int i = 0; i < 16; i++) {
        Led *led = new Led();
        builder.add(led);
        led->setLabel(QString("op[%1]").arg(i));
        led->setPos(50 + (i % 4) * 60, 200 + (i / 4) * 40);
        op_outputs.append(led);
    }

    // Connect instruction inputs to decoder
    for (int i = 0; i < 4; i++) {
        builder.connect(instr_inputs[i], 0, decoder, QString("instr[%1]").arg(i));
    }

    // Connect decoder outputs to LEDs
    for (int i = 0; i < 16; i++) {
        builder.connect(decoder, QString("op[%1]").arg(i), op_outputs[i], 0);
    }

    Simulation *sim = builder.initSimulation();
    sim->update();

    // Set instruction input
    setMultiBitInput(instr_inputs, instructionCode);
    sim->update();

    // Read all 16 output lines as a single value (one-hot encoding)
    int outputValue = readMultiBitOutput(QVector<GraphicElement *>(op_outputs.begin(), op_outputs.end()), 0);

    // For one-hot encoding, exactly one bit should be set
    // The output value should be (1 << expectedActiveOutput)
    int expectedValue = 1 << expectedActiveOutput;
    QCOMPARE(outputValue, expectedValue);
}

void TestLevel5InstructionDecoder4Bit::testInstructionDecoderStructure()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *decoder = loadBuildingBlockIC("level5_instruction_decoder_4bit.panda");
    builder.add(decoder);

    // Verify IC has been loaded
    QVERIFY(decoder != nullptr);

    // Create dummy elements for connection testing
    InputSwitch *data = new InputSwitch();
    builder.add(data);

    Led *out = new Led();
    builder.add(out);

    // Verify IC port structure
    // Inputs: 4 instruction bits = 4 inputs
    // Outputs: 16 instruction lines = 16 outputs
    QCOMPARE(decoder->inputSize(), 4);   // 4 instruction bits
    QCOMPARE(decoder->outputSize(), 16); // 16 output lines

    // Verify port connections work (don't throw)
    try {
        builder.connect(data, 0, decoder, "instr[0]");  // Connect instr[0] input
        builder.connect(decoder, "op[0]", out, 0);      // Connect op[0] output
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("IC port access failed: %1").arg(e.what())));
    }
}
