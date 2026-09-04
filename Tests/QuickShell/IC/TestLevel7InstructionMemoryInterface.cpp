// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/IC/TestLevel7InstructionMemoryInterface.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/QuickShell/IC/CpuTestUtils.h"
#include "Tests/QuickShell/IC/QuickTestUtils.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"

using QuickTestUtils::setMultiBitInput;
using QuickTestUtils::readMultiBitOutput;
using QuickTestUtils::clockCycle;
using CPUTestUtils::loadBuildingBlockIC;

// Fully-wired harness (Address + DataIn + WriteEnable + Clock), shared by the
// read-only and write/read tests. Mirrors the sibling level7 fixtures in this
// directory.
struct InstructionMemoryInterfaceFixture {
    std::unique_ptr<QuickCircuitBuilder> builder;
    IC *ic = nullptr;
    QVector<InputSwitch *> addressInputs;
    QVector<InputSwitch *> dataInputs;
    InputSwitch *writeEnable = nullptr;
    InputSwitch *clk = nullptr;
    QVector<Led *> instrOutputs;
    Simulation *sim = nullptr;

    bool build()
    {
        builder = std::make_unique<QuickCircuitBuilder>();

        ic = loadBuildingBlockIC("level7_instruction_memory_interface.panda");
        builder->addOwnedElement(ic);

        for (int i = 0; i < 8; i++) {
            auto *addr = new InputSwitch(); builder->addOwnedElement(addr); addressInputs.append(addr);
            auto *data = new InputSwitch(); builder->addOwnedElement(data); dataInputs.append(data);
            auto *led = new Led(); builder->addOwnedElement(led); instrOutputs.append(led);
        }

        writeEnable = new InputSwitch(); builder->addOwnedElement(writeEnable);
        clk = new InputSwitch(); builder->addOwnedElement(clk);

        for (int i = 0; i < 8; i++) {
            builder->connect(addressInputs[i], 0, ic, QString("Address[%1]").arg(i));
            builder->connect(dataInputs[i], 0, ic, QString("DataIn[%1]").arg(i));
            builder->connect(ic, QString("Instruction[%1]").arg(i), instrOutputs[i], 0);
        }
        builder->connect(writeEnable, 0, ic, "WriteEnable");
        builder->connect(clk, 0, ic, "Clock");

        sim = builder->initSimulation();
        sim->update();
        return true;
    }

    int readAt(int address)
    {
        setMultiBitInput(addressInputs, address);
        sim->update();
        return readMultiBitOutput(QVector<GraphicElement *>(instrOutputs.begin(), instrOutputs.end()), 0);
    }

    void writeWord(int address, int value)
    {
        setMultiBitInput(addressInputs, address);
        setMultiBitInput(dataInputs, value);
        writeEnable->setOn(true);
        sim->update();
        clockCycle(sim, clk);
        writeEnable->setOn(false);
        sim->update();
    }
};

static std::unique_ptr<InstructionMemoryInterfaceFixture> s_level7InstrMemInterface;

void TestLevel7InstructionMemoryInterface::initTestCase()
{
    s_level7InstrMemInterface = std::make_unique<InstructionMemoryInterfaceFixture>();
    QVERIFY(s_level7InstrMemInterface->build());
}

void TestLevel7InstructionMemoryInterface::cleanupTestCase()
{
    s_level7InstrMemInterface.reset();
}

void TestLevel7InstructionMemoryInterface::cleanup()
{
    if (s_level7InstrMemInterface && s_level7InstrMemInterface->sim) {
        s_level7InstrMemInterface->sim->restart();
        s_level7InstrMemInterface->sim->update();
    }
}

void TestLevel7InstructionMemoryInterface::testInstructionMemoryInterface_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("expectedInstruction");

    // All expected values are 0x00 because a freshly-reset memory IC returns
    // uninitialized/default data on a read-only pass -- see
    // testInstructionMemoryWriteRead() below for the write-then-read path.
    QTest::newRow("address 0x00") << 0x00 << 0x00;
    QTest::newRow("address 0x01") << 0x01 << 0x00;
    QTest::newRow("address 0x10") << 0x10 << 0x00;
    QTest::newRow("address 0x42") << 0x42 << 0x00;
    QTest::newRow("address 0xFF") << 0xFF << 0x00;
}

void TestLevel7InstructionMemoryInterface::testInstructionMemoryInterface()
{
    QFETCH(int, address);
    QFETCH(int, expectedInstruction);

    auto &f = *s_level7InstrMemInterface;

    // DataIn/WriteEnable stay untouched (default off) -- this is a read-only
    // pass over freshly-reset (uninitialized) memory.
    QCOMPARE(f.readAt(address), expectedInstruction);
}

// The read-only test above only sees uninitialized memory (all 0x00) and never
// wires DataIn/WriteEnable, so the write port -- the whole reason the interface
// exposes one -- went unexercised. Program two words through the write port, then
// read each back, and confirm the documented partial-decode aliasing (only
// Address[0..2] reach the 8-word RAM, so addresses repeat modulo 8).
void TestLevel7InstructionMemoryInterface::testInstructionMemoryWriteRead()
{
    auto &f = *s_level7InstrMemInterface;

    // Program two distinct words at two distinct low addresses
    f.writeWord(0x02, 0xAB);
    f.writeWord(0x05, 0xCD);

    // Each reads back independently -- proving the write port stores and the
    // async read returns the addressed word
    QCOMPARE(f.readAt(0x02), 0xAB);
    QCOMPARE(f.readAt(0x05), 0xCD);

    // Partial decode: Address[3..7] are ignored, so 0x0A (= 0x02 + 8) aliases
    // the word written at 0x02
    QCOMPARE(f.readAt(0x0A), 0xAB);
}

void TestLevel7InstructionMemoryInterface::testInstructionMemoryInterfaceStructure()
{
    auto &f = *s_level7InstrMemInterface;

    QVERIFY(f.ic != nullptr);

    // Inputs: 8 Address + 8 DataIn + 1 WriteEnable + 1 Clock = 18 inputs
    // Outputs: 8 Instruction = 8 outputs
    QCOMPARE(f.ic->inputSize(), 18);
    QCOMPARE(f.ic->outputSize(), 8);
}
