// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel7InstructionDecoder8bit.h"

#include <QFile>
#include <QFileInfo>
#include <QMap>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using TestUtils::setMultiBitInput;
using CPUTestUtils::loadBuildingBlockIC;

struct InstrDecoder8bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    QVector<InputSwitch *> instrInputs;
    QMap<int, Led *> opOutputs;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level7_instruction_decoder_8bit.panda");
        builder.add(ic);

        for (int i = 0; i < 8; i++) {
            auto *sw = new InputSwitch();
            builder.add(sw);
            sw->setLabel(QString("instr[%1]").arg(i));
            instrInputs.append(sw);
        }

        // Connect all instruction inputs to decoder
        for (int i = 0; i < 8; i++) {
            builder.connect(instrInputs[i], 0, ic, QString("instr[%1]").arg(i));
        }

        // Connect outputs for all indices tested in _data()
        const int testIndices[] = {0, 1, 2, 4, 8, 15, 16, 32, 64, 66, 85, 127, 128, 129, 170, 240, 255};
        for (int idx : testIndices) {
            auto *led = new Led();
            builder.add(led);
            led->setLabel(QString("op[%1]").arg(idx));
            builder.connect(ic, QString("op[%1]").arg(idx), led, 0);
            opOutputs[idx] = led;
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<InstrDecoder8bitFixture> s_level7InstrDec8bit;

void TestLevel7InstructionDecoder8Bit::initTestCase()
{
    s_level7InstrDec8bit = std::make_unique<InstrDecoder8bitFixture>();
    QVERIFY(s_level7InstrDec8bit->build());
}

void TestLevel7InstructionDecoder8Bit::cleanupTestCase()
{
    s_level7InstrDec8bit.reset();
}

void TestLevel7InstructionDecoder8Bit::cleanup()
{
    if (s_level7InstrDec8bit && s_level7InstrDec8bit->sim) {
        s_level7InstrDec8bit->sim->restart();
        s_level7InstrDec8bit->sim->update();
    }
}

void TestLevel7InstructionDecoder8Bit::testInstructionDecoder8Bit_data()
{
    QTest::addColumn<int>("instructionCode");
    QTest::addColumn<int>("expectedOutputLine");

    QTest::newRow("instr 0x00 (all zeros)") << 0x00 << 0;
    QTest::newRow("instr 0xFF (all ones)") << 0xFF << 255;

    QTest::newRow("instr 0x01 (bit 0)") << 0x01 << 1;
    QTest::newRow("instr 0x02 (bit 1)") << 0x02 << 2;
    QTest::newRow("instr 0x04 (bit 2)") << 0x04 << 4;
    QTest::newRow("instr 0x08 (bit 3)") << 0x08 << 8;
    QTest::newRow("instr 0x10 (bit 4)") << 0x10 << 16;
    QTest::newRow("instr 0x20 (bit 5)") << 0x20 << 32;
    QTest::newRow("instr 0x40 (bit 6)") << 0x40 << 64;
    QTest::newRow("instr 0x80 (bit 7)") << 0x80 << 128;

    QTest::newRow("instr 0x42 (0x40 | 0x02)") << 0x42 << 66;
    QTest::newRow("instr 0x81 (0x80 | 0x01)") << 0x81 << 129;
    QTest::newRow("instr 0xAA (alternating 1s)") << 0xAA << 170;
    QTest::newRow("instr 0x55 (alternating 0s)") << 0x55 << 85;

    QTest::newRow("instr 0x80 (128)") << 0x80 << 128;
    QTest::newRow("instr 0x7F (127)") << 0x7F << 127;
    QTest::newRow("instr 0xF0 (240)") << 0xF0 << 240;
    QTest::newRow("instr 0x0F (15)") << 0x0F << 15;
}

void TestLevel7InstructionDecoder8Bit::testInstructionDecoder8Bit()
{
    QFETCH(int, instructionCode);
    QFETCH(int, expectedOutputLine);

    auto &f = *s_level7InstrDec8bit;

    setMultiBitInput(f.instrInputs, instructionCode);
    f.sim->update();

    // Check the indices we have connected
    QSet<int> testIndices;
    testIndices.insert(0);
    testIndices.insert(127);
    testIndices.insert(128);
    testIndices.insert(255);
    testIndices.insert(expectedOutputLine);

    for (int opIndex : std::as_const(testIndices)) {
        if (!f.opOutputs.contains(opIndex)) {
            continue;
        }
        bool isActive = getInputStatus(f.opOutputs[opIndex], 0);

        if (opIndex == expectedOutputLine) {
            QVERIFY2(isActive, qPrintable(QString("Expected op[%1] to be active for instruction 0x%2")
                .arg(opIndex).arg(instructionCode, 0, 16)));
        } else {
            QVERIFY2(!isActive, qPrintable(QString("Expected op[%1] to be inactive for instruction 0x%2")
                .arg(opIndex).arg(instructionCode, 0, 16)));
        }
    }
}

void TestLevel7InstructionDecoder8Bit::testInstructionDecoder8BitStructure()
{
    auto &f = *s_level7InstrDec8bit;

    QVERIFY(f.ic != nullptr);

    QCOMPARE(f.ic->inputSize(), 8);
    QCOMPARE(f.ic->outputSize(), 256);
}

