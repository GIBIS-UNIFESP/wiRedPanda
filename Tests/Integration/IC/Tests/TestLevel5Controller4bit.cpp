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

struct Controller4bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    QVector<InputSwitch *> opcodeInputs;
    QVector<Led *> ctrlOutputs;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level5_controller_4bit.panda");
        builder.add(ic);

        for (int i = 0; i < 4; ++i) {
            auto *sw = new InputSwitch();
            builder.add(sw);
            opcodeInputs.append(sw);
            builder.connect(sw, 0, ic, QString("opcode[%1]").arg(i));
        }
        for (int i = 0; i < 4; ++i) {
            auto *led = new Led();
            builder.add(led);
            ctrlOutputs.append(led);
            builder.connect(ic, QString("ctrl[%1]").arg(i), led, 0);
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<Controller4bitFixture> s_level5Controller4bit;

void TestLevel5Controller4Bit::initTestCase()
{
    s_level5Controller4bit = std::make_unique<Controller4bitFixture>();
    QVERIFY(s_level5Controller4bit->build());
}

void TestLevel5Controller4Bit::cleanupTestCase()
{
    s_level5Controller4bit.reset();
}

void TestLevel5Controller4Bit::cleanup()
{
    if (s_level5Controller4bit && s_level5Controller4bit->sim) {
        s_level5Controller4bit->sim->restart();
        s_level5Controller4bit->sim->update();
    }
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

    auto &f = *s_level5Controller4bit;

    setMultiBitInput(f.opcodeInputs, opcode);
    for (int i = 0; i < 20; ++i) {
        f.sim->update();
    }

    int controlValue = readMultiBitOutput(QVector<GraphicElement *>(f.ctrlOutputs.begin(), f.ctrlOutputs.end()), 0);

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
    auto &f = *s_level5Controller4bit;

    QVERIFY(f.ic != nullptr);
    QCOMPARE(f.ic->inputSize(), 4);
    QCOMPARE(f.ic->outputSize(), 4);
}

