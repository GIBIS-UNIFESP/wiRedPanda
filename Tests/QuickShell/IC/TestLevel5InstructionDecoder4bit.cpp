// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/IC/TestLevel5InstructionDecoder4bit.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/QuickShell/IC/CpuTestUtils.h"
#include "Tests/QuickShell/IC/QuickTestUtils.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"

using QuickTestUtils::setMultiBitInput;
using QuickTestUtils::readMultiBitOutput;
using CPUTestUtils::loadBuildingBlockIC;

struct InstructionDecoder4bitFixture {
    std::unique_ptr<QuickCircuitBuilder> builder;
    IC *ic = nullptr;
    QVector<InputSwitch *> instrInputs;
    QVector<Led *> opOutputs;
    Simulation *sim = nullptr;

    bool build()
    {
        builder = std::make_unique<QuickCircuitBuilder>();

        ic = loadBuildingBlockIC("level5_instruction_decoder_4bit.panda");
        builder->addOwnedElement(ic);

        for (int i = 0; i < 4; ++i) {
            auto *sw = new InputSwitch();
            builder->addOwnedElement(sw);
            instrInputs.append(sw);
            builder->connect(sw, 0, ic, QString("instr[%1]").arg(i));
        }
        for (int i = 0; i < 16; ++i) {
            auto *led = new Led();
            builder->addOwnedElement(led);
            opOutputs.append(led);
            builder->connect(ic, QString("op[%1]").arg(i), led, 0);
        }

        sim = builder->initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<InstructionDecoder4bitFixture> s_level5InstructionDecoder4bit;

void TestLevel5InstructionDecoder4Bit::initTestCase()
{
    s_level5InstructionDecoder4bit = std::make_unique<InstructionDecoder4bitFixture>();
    QVERIFY(s_level5InstructionDecoder4bit->build());
}

void TestLevel5InstructionDecoder4Bit::cleanupTestCase()
{
    s_level5InstructionDecoder4bit.reset();
}

void TestLevel5InstructionDecoder4Bit::cleanup()
{
    if (s_level5InstructionDecoder4bit && s_level5InstructionDecoder4bit->sim) {
        s_level5InstructionDecoder4bit->sim->restart();
        s_level5InstructionDecoder4bit->sim->update();
    }
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

    auto &f = *s_level5InstructionDecoder4bit;

    setMultiBitInput(f.instrInputs, instructionCode);
    f.sim->update();

    int outputValue = readMultiBitOutput(QVector<GraphicElement *>(f.opOutputs.begin(), f.opOutputs.end()), 0);

    int expectedValue = 1 << expectedActiveOutput;
    QCOMPARE(outputValue, expectedValue);
}

void TestLevel5InstructionDecoder4Bit::testInstructionDecoderStructure()
{
    auto &f = *s_level5InstructionDecoder4bit;

    QVERIFY(f.ic != nullptr);
    QCOMPARE(f.ic->inputSize(), 4);
    QCOMPARE(f.ic->outputSize(), 16);
}
