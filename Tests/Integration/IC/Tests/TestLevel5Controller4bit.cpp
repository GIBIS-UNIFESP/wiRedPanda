// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel5Controller4bit.h"

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

void TestLevel5Controller4Bit::initTestCase()
{
}

void TestLevel5Controller4Bit::cleanup()
{
}

void TestLevel5Controller4Bit::testController4Bit_data()
{
    QTest::addColumn<int>("opcode");

    // Test all 16 possible 4-bit opcodes
    // Decoding logic:
    //   ctrl[1:0] = opcode[1:0]  (ALU operation selection)
    //   ctrl[2] = opcode[3] OR opcode[2]  (register write enable)
    //   ctrl[3] = opcode[3]  (memory write enable)

    // Range 0x0-0x3 (00xx): ALU operations only
    QTest::newRow("opcode 0x0 (ALU ADD, no writes)")
        << 0b0000;
    QTest::newRow("opcode 0x1 (ALU SUB, no writes)")
        << 0b0001;
    QTest::newRow("opcode 0x2 (ALU AND, no writes)")
        << 0b0010;
    QTest::newRow("opcode 0x3 (ALU OR, no writes)")
        << 0b0011;

    // Range 0x4-0x7 (01xx): ALU + register write
    QTest::newRow("opcode 0x4 (ALU ADD, register write)")
        << 0b0100;
    QTest::newRow("opcode 0x5 (ALU SUB, register write)")
        << 0b0101;
    QTest::newRow("opcode 0x6 (ALU AND, register write)")
        << 0b0110;
    QTest::newRow("opcode 0x7 (ALU OR, register write)")
        << 0b0111;

    // Range 0x8-0xB (10xx): ALU + memory write
    QTest::newRow("opcode 0x8 (ALU ADD, memory write)")
        << 0b1000;
    QTest::newRow("opcode 0x9 (ALU SUB, memory write)")
        << 0b1001;
    QTest::newRow("opcode 0xA (ALU AND, memory write)")
        << 0b1010;
    QTest::newRow("opcode 0xB (ALU OR, memory write)")
        << 0b1011;

    // Range 0xC-0xF (11xx): ALU + register + memory write
    QTest::newRow("opcode 0xC (ALU ADD, register + memory write)")
        << 0b1100;
    QTest::newRow("opcode 0xD (ALU SUB, register + memory write)")
        << 0b1101;
    QTest::newRow("opcode 0xE (ALU AND, register + memory write)")
        << 0b1110;
    QTest::newRow("opcode 0xF (ALU OR, register + memory write)")
        << 0b1111;
}

void TestLevel5Controller4Bit::testController4Bit()
{
    QFETCH(int, opcode);

    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Create 4-bit opcode input
    QVector<InputSwitch *> opcode_inputs;
    for (int i = 0; i < 4; i++) {
        InputSwitch *sw = new InputSwitch();
        builder.add(sw);
        sw->setLabel(QString("opcode[%1]").arg(i));
        sw->setPos(50 + i * 60, 100);
        opcode_inputs.append(sw);
    }

    // Load the controller IC
    IC *controller = loadBuildingBlockIC("level5_controller_4bit.panda");
    builder.add(controller);

    // Create output LEDs for all 4 control bits
    QVector<Led *> ctrl_outputs;
    for (int i = 0; i < 4; i++) {
        Led *led = new Led();
        builder.add(led);
        led->setLabel(QString("ctrl[%1]").arg(i));
        led->setPos(50 + i * 60, 200);
        ctrl_outputs.append(led);
    }

    // Connect opcode inputs to controller
    for (int i = 0; i < 4; i++) {
        builder.connect(opcode_inputs[i], 0, controller, QString("opcode[%1]").arg(i));
    }

    // Connect controller outputs to LEDs
    for (int i = 0; i < 4; i++) {
        builder.connect(controller, QString("ctrl[%1]").arg(i), ctrl_outputs[i], 0);
    }

    Simulation *sim = builder.initSimulation();
    sim->update();

    // Set opcode input
    setMultiBitInput(opcode_inputs, opcode);
    for (int i = 0; i < 20; ++i) {
        sim->update();
    }

    // Read control output (all 4 bits)
    int controlValue = readMultiBitOutput(QVector<GraphicElement *>(ctrl_outputs.begin(), ctrl_outputs.end()), 0);

    // Verify decoding logic:
    // ctrl[1:0] = opcode[1:0] (ALU operation selection)
    // ctrl[2] = opcode[3] OR opcode[2] (register write enable)
    // ctrl[3] = opcode[3] (memory write enable)

    int expectedCtrl0 = (opcode >> 0) & 1;
    int expectedCtrl1 = (opcode >> 1) & 1;
    int expectedCtrl2 = ((opcode >> 3) | (opcode >> 2)) & 1;  // OR gate output
    int expectedCtrl3 = (opcode >> 3) & 1;
    int expectedAll = (expectedCtrl3 << 3) | (expectedCtrl2 << 2) | (expectedCtrl1 << 1) | expectedCtrl0;

    QCOMPARE(controlValue, expectedAll);

    // Verify individual control bits
    QCOMPARE((controlValue >> 0) & 1, expectedCtrl0);  // ctrl[0]
    QCOMPARE((controlValue >> 1) & 1, expectedCtrl1);  // ctrl[1]
    QCOMPARE((controlValue >> 2) & 1, expectedCtrl2);  // ctrl[2]
    QCOMPARE((controlValue >> 3) & 1, expectedCtrl3);  // ctrl[3]
}

void TestLevel5Controller4Bit::testControllerStructure()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *controller = loadBuildingBlockIC("level5_controller_4bit.panda");
    builder.add(controller);

    // Verify IC has been loaded
    QVERIFY(controller != nullptr);

    // Create dummy elements for connection testing
    InputSwitch *data = new InputSwitch();
    builder.add(data);

    Led *out = new Led();
    builder.add(out);

    // Verify IC port structure
    // Inputs: 4 opcode bits = 4 inputs
    // Outputs: 4 control bits = 4 outputs
    QCOMPARE(controller->inputSize(), 4);   // 4 opcode bits
    QCOMPARE(controller->outputSize(), 4);  // 4 control bits

    // Verify port connections work (don't throw)
    try {
        builder.connect(data, 0, controller, "opcode[0]");   // Connect opcode[0] input
        builder.connect(controller, "ctrl[0]", out, 0);      // Connect ctrl[0] output
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("IC port access failed: %1").arg(e.what())));
    }
}
