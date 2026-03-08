// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel7InstructionRegister8bit.h"

#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::readMultiBitOutput;
using TestUtils::setMultiBitInput;
using CPUTestUtils::loadBuildingBlockIC;

struct InstrRegister8bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    QVector<InputSwitch *> dataInputs;
    InputSwitch *clock = nullptr;
    InputSwitch *load = nullptr;
    InputSwitch *reset = nullptr;
    QVector<Led *> instrOutputs;
    QVector<Led *> opcodeOutputs;
    QVector<Led *> regaddrOutputs;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level7_instruction_register_8bit.panda");
        builder.add(ic);

        for (int i = 0; i < 8; i++) {
            auto *data = new InputSwitch(); builder.add(data); dataInputs.append(data);
        }

        clock = new InputSwitch(); builder.add(clock);
        load = new InputSwitch(); builder.add(load);
        reset = new InputSwitch(); builder.add(reset);

        for (int i = 0; i < 8; i++) {
            auto *led = new Led(); builder.add(led); instrOutputs.append(led);
        }
        for (int i = 0; i < 5; i++) {
            auto *led = new Led(); builder.add(led); opcodeOutputs.append(led);
        }
        for (int i = 0; i < 3; i++) {
            auto *led = new Led(); builder.add(led); regaddrOutputs.append(led);
        }

        for (int i = 0; i < 8; i++) {
            builder.connect(dataInputs[i], 0, ic, QString("Data[%1]").arg(i));
            builder.connect(ic, QString("Instruction[%1]").arg(i), instrOutputs[i], 0);
        }
        builder.connect(clock, 0, ic, "Clock");
        builder.connect(load, 0, ic, "Load");
        builder.connect(reset, 0, ic, "Reset");

        for (int i = 0; i < 5; i++) {
            builder.connect(ic, QString("OpCode[%1]").arg(i), opcodeOutputs[i], 0);
        }
        for (int i = 0; i < 3; i++) {
            builder.connect(ic, QString("RegisterAddr[%1]").arg(i), regaddrOutputs[i], 0);
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    int readInstr()
    {
        return readMultiBitOutput(QVector<GraphicElement *>(instrOutputs.begin(), instrOutputs.end()), 0);
    }

    int readOpCode()
    {
        return readMultiBitOutput(QVector<GraphicElement *>(opcodeOutputs.begin(), opcodeOutputs.end()), 0);
    }

    int readRegAddr()
    {
        return readMultiBitOutput(QVector<GraphicElement *>(regaddrOutputs.begin(), regaddrOutputs.end()), 0);
    }
};

static std::unique_ptr<InstrRegister8bitFixture> s_level7InstrReg8bit;

void TestLevel7InstructionRegister8Bit::initTestCase()
{
    s_level7InstrReg8bit = std::make_unique<InstrRegister8bitFixture>();
    QVERIFY(s_level7InstrReg8bit->build());
}

void TestLevel7InstructionRegister8Bit::cleanupTestCase()
{
    s_level7InstrReg8bit.reset();
}

void TestLevel7InstructionRegister8Bit::cleanup()
{
    if (s_level7InstrReg8bit && s_level7InstrReg8bit->sim) {
        s_level7InstrReg8bit->sim->restart();
        s_level7InstrReg8bit->sim->update();
    }
}

void TestLevel7InstructionRegister8Bit::testInstructionRegister8Bit() {
    auto &f = *s_level7InstrReg8bit;

    // Test 1: Load instruction 0xB5
    setMultiBitInput(f.dataInputs, 0xB5);
    f.load->setOn(true);
    f.reset->setOn(false);

    f.sim->update();
    clockCycle(f.sim, f.clock);
    f.sim->update();

    QCOMPARE(f.readInstr(), 0xB5);
    QCOMPARE(f.readOpCode(), 22);
    QCOMPARE(f.readRegAddr(), 5);

    // Test 2: Load instruction 0x73
    f.clock->setOn(false);
    f.sim->update();

    setMultiBitInput(f.dataInputs, 0x73);
    f.sim->update();
    clockCycle(f.sim, f.clock);
    f.sim->update();

    QCOMPARE(f.readInstr(), 0x73);
    QCOMPARE(f.readOpCode(), 14);
    QCOMPARE(f.readRegAddr(), 3);

    // Test 3: Reset
    f.clock->setOn(false);
    f.reset->setOn(true);
    f.sim->update();
    QCOMPARE(f.readInstr(), CPUTestUtils::PATTERN_ALL_ZEROS);
}

void TestLevel7InstructionRegister8Bit::testInstructionRegister8BitStructure() {
    auto &f = *s_level7InstrReg8bit;

    QVERIFY(f.ic != nullptr);

    QCOMPARE(f.ic->inputSize(), 11);
    QCOMPARE(f.ic->outputSize(), 16);
}

