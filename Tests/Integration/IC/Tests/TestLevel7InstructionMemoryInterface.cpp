// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel7InstructionMemoryInterface.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::setMultiBitInput;
using TestUtils::readMultiBitOutput;
using TestUtils::clockCycle;
using CPUTestUtils::loadBuildingBlockIC;

// Fully-wired harness (Address + DataIn + WriteEnable + Clock), shared by the
// read-only and write/read tests. Mirrors the sibling level7 fixtures in this
// directory.
struct InstructionMemoryInterfaceFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    QVector<InputSwitch *> addressInputs;
    QVector<InputSwitch *> dataInputs;
    InputSwitch *writeEnable = nullptr;
    InputSwitch *clk = nullptr;
    QVector<Led *> instrOutputs;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level7_instruction_memory_interface.panda");
        builder.add(ic);

        for (int i = 0; i < 8; i++) {
            auto *addr = new InputSwitch(); builder.add(addr); addressInputs.append(addr);
            auto *data = new InputSwitch(); builder.add(data); dataInputs.append(data);
            auto *led = new Led(); builder.add(led); instrOutputs.append(led);
        }

        writeEnable = new InputSwitch(); builder.add(writeEnable);
        clk = new InputSwitch(); builder.add(clk);

        for (int i = 0; i < 8; i++) {
            builder.connect(addressInputs[i], 0, ic, QString("Address[%1]").arg(i));
            builder.connect(dataInputs[i], 0, ic, QString("DataIn[%1]").arg(i));
            builder.connect(ic, QString("Instruction[%1]").arg(i), instrOutputs[i], 0);
        }
        builder.connect(writeEnable, 0, ic, "WriteEnable");
        builder.connect(clk, 0, ic, "Clock");

        sim = builder.initSimulation();
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

    // Test various addresses to verify memory interface read capability
    // All expected values are 0x00 because the instruction memory IC returns uninitialized/default data
    // This tests that the IC correctly:
    // 1. Accepts address inputs on all 8 Address[0-7] ports
    // 2. Routes the address input to internal memory
    // 3. Returns the memory contents on Instruction[0-7] outputs
    // NOTE: For a complete memory test with actual data, the .panda file would need pre-programmed values
    // or the test would need to exercise a write-enable path (not currently implemented)

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

    // DataIn/WriteEnable stay untouched (default off) — this is a read-only
    // pass over freshly-reset (uninitialized) memory.
    QCOMPARE(f.readAt(address), expectedInstruction);
}

// The read-only test above only sees uninitialized memory (all 0x00) and never
// wires DataIn/WriteEnable, so the write port — the whole reason the interface
// exposes one — went unexercised. Program two words through the write port, then
// read each back, and confirm the documented partial-decode aliasing (only
// Address[0..2] reach the 8-word RAM, so addresses repeat modulo 8).
void TestLevel7InstructionMemoryInterface::testInstructionMemoryWriteRead()
{
    auto &f = *s_level7InstrMemInterface;

    // Program two distinct words at two distinct low addresses
    f.writeWord(0x02, 0xAB);
    f.writeWord(0x05, 0xCD);

    // Each reads back independently — proving the write port stores and the
    // async read returns the addressed word
    QCOMPARE(f.readAt(0x02), 0xAB);
    QCOMPARE(f.readAt(0x05), 0xCD);

    // Partial decode: Address[3..7] are ignored, so 0x0A (= 0x02 + 8) aliases
    // the word written at 0x02
    QCOMPARE(f.readAt(0x0A), 0xAB);
}

void TestLevel7InstructionMemoryInterface::testInstructionMemoryInterfaceStructure()
{
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *imem = loadBuildingBlockIC("level7_instruction_memory_interface.panda");
    builder.add(imem);

    // Verify IC has been loaded
    QVERIFY(imem != nullptr);

    // Create dummy elements for connection testing
    InputSwitch *addr = new InputSwitch();
    builder.add(addr);

    Led *out = new Led();
    builder.add(out);

    // Verify IC port structure
    // Inputs: 8 Address + 8 DataIn + 1 WriteEnable + 1 Clock = 18 inputs
    // Outputs: 8 Instruction = 8 outputs
    QCOMPARE(imem->inputSize(), 18);  // 8 address + 8 data + 1 write enable + 1 clock
    QCOMPARE(imem->outputSize(), 8);  // 8 instruction bits

    // Verify port connections work (don't throw and return success)
    try {
        QVERIFY2(builder.connect(addr, 0, imem, "Address[0]"), "Failed to connect Address[0]");
        QVERIFY2(builder.connect(addr, 0, imem, "Address[7]"), "Failed to connect Address[7]");
        QVERIFY2(builder.connect(addr, 0, imem, "Clock"), "Failed to connect Clock");
        QVERIFY2(builder.connect(imem, "Instruction[0]", out, 0), "Failed to connect Instruction[0]");
        QVERIFY2(builder.connect(imem, "Instruction[7]", out, 0), "Failed to connect Instruction[7]");
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("IC port access failed: %1").arg(e.what())));
    }
}
