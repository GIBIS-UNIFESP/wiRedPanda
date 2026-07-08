// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel9MemoryStage16bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::readMultiBitOutput;
using TestUtils::setMultiBitInput;
using TestUtils::clockCycle;
using CPUTestUtils::loadBuildingBlockIC;

struct MemoryStage16bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    QVector<InputSwitch *> addressInputs;
    QVector<InputSwitch *> datainInputs;
    QVector<InputSwitch *> resultInputs;
    InputSwitch *memread = nullptr;
    InputSwitch *memwrite = nullptr;
    InputSwitch *clk = nullptr;
    InputSwitch *reset = nullptr;
    QVector<Led *> dataoutLeds;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level9_memory_stage_16bit.panda");
        builder.add(ic);

        for (int i = 0; i < 8; i++) {
            auto *a = new InputSwitch(); builder.add(a); addressInputs.append(a);
        }
        for (int i = 0; i < 16; i++) {
            auto *d = new InputSwitch(); builder.add(d); datainInputs.append(d);
            auto *r = new InputSwitch(); builder.add(r); resultInputs.append(r);
            auto *led = new Led(); builder.add(led); dataoutLeds.append(led);
        }

        memread = new InputSwitch(); builder.add(memread);
        memwrite = new InputSwitch(); builder.add(memwrite);
        clk = new InputSwitch(); builder.add(clk);
        reset = new InputSwitch(); builder.add(reset);

        for (int i = 0; i < 8; i++) {
            builder.connect(addressInputs[i], 0, ic, QString("Address[%1]").arg(i));
        }
        for (int i = 0; i < 16; i++) {
            builder.connect(datainInputs[i], 0, ic, QString("DataIn[%1]").arg(i));
            builder.connect(resultInputs[i], 0, ic, QString("Result[%1]").arg(i));
            builder.connect(ic, QString("DataOut[%1]").arg(i), dataoutLeds[i], 0);
        }

        builder.connect(memread, 0, ic, "MemRead");
        builder.connect(memwrite, 0, ic, "MemWrite");
        builder.connect(clk, 0, ic, "Clock");
        builder.connect(reset, 0, ic, "Reset");

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    int readDataOut()
    {
        return readMultiBitOutput(QVector<GraphicElement *>(dataoutLeds.begin(), dataoutLeds.end()));
    }
};

static std::unique_ptr<MemoryStage16bitFixture> s_level9MemoryStage16bit;

void TestLevel9MemoryStage16Bit::initTestCase()
{
    s_level9MemoryStage16bit = std::make_unique<MemoryStage16bitFixture>();
    QVERIFY(s_level9MemoryStage16bit->build());
}

void TestLevel9MemoryStage16Bit::cleanupTestCase()
{
    s_level9MemoryStage16bit.reset();
}

void TestLevel9MemoryStage16Bit::cleanup()
{
    if (s_level9MemoryStage16bit && s_level9MemoryStage16bit->sim) {
        s_level9MemoryStage16bit->sim->restart();
        s_level9MemoryStage16bit->sim->update();
    }
}

void TestLevel9MemoryStage16Bit::testMemoryStage_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("dataIn");
    QTest::addColumn<int>("result");
    QTest::addColumn<bool>("memRead");
    QTest::addColumn<bool>("memWrite");
    QTest::addColumn<int>("expectedOut");

    QTest::newRow("passthrough 0x8421 (no mem op)") << 0x00 << 0x0000 << 0x8421 << false << false << 0x8421;
    QTest::newRow("passthrough 0x0000") << 0xFF << 0x0000 << 0x0000 << false << false << 0x0000;
    QTest::newRow("passthrough 0xAABB") << 0x55 << 0xBBCC << 0xAABB << false << false << 0xAABB;

    QTest::newRow("write 0x4222 to addr 0x00") << 0x00 << 0x4222 << 0xFFFF << false << true << 0xFFFF;
    QTest::newRow("write 0x5533 to addr 0x10") << 0x10 << 0x5533 << 0x7777 << false << true << 0x7777;

    QTest::newRow("read from addr 0x00 (empty)") << 0x00 << 0x0000 << 0xFFFF << true << false << 0x0000;
    QTest::newRow("read from addr 0x10 (empty)") << 0x10 << 0x0000 << 0xFFFF << true << false << 0x0000;
}

void TestLevel9MemoryStage16Bit::testMemoryStage()
{
    QFETCH(int, address);
    QFETCH(int, dataIn);
    QFETCH(int, result);
    QFETCH(bool, memRead);
    QFETCH(bool, memWrite);
    QFETCH(int, expectedOut);

    auto &f = *s_level9MemoryStage16bit;

    setMultiBitInput(f.addressInputs, address);
    setMultiBitInput(f.datainInputs, dataIn);
    setMultiBitInput(f.resultInputs, result);
    f.memread->setOn(memRead);
    f.memwrite->setOn(memWrite);
    f.sim->update();

    if (memWrite) {
        clockCycle(f.sim, f.clk);
    }

    QCOMPARE(f.readDataOut(), expectedOut);
}

void TestLevel9MemoryStage16Bit::testMemoryStageStructure()
{
    auto &f = *s_level9MemoryStage16bit;

    QVERIFY(f.ic != nullptr);
    // Address[8] + DataIn[16] + Result[16] + MemRead + MemWrite + Clock + Reset
    QCOMPARE(f.ic->inputSize(), 44);
    // DataOut[16]
    QCOMPARE(f.ic->outputSize(), 16);
}

// Reset clears the backing memory (both byte lanes), same convention as
// level8_memory_stage's own Reset (F54): async clear down the RAM stack.
void TestLevel9MemoryStage16Bit::testMemoryStageReset()
{
    auto &f = *s_level9MemoryStage16bit;

    // Write 0x5AA5 to address 3
    setMultiBitInput(f.addressInputs, 0x03);
    setMultiBitInput(f.datainInputs, 0x5AA5);
    setMultiBitInput(f.resultInputs, 0x0000);
    f.memread->setOn(false);
    f.memwrite->setOn(true);
    f.reset->setOn(false);
    f.sim->update();
    clockCycle(f.sim, f.clk);

    // Read it back
    f.memwrite->setOn(false);
    f.memread->setOn(true);
    f.sim->update();
    QCOMPARE(f.readDataOut(), 0x5AA5);

    // Assert Reset: the stored word must clear (async, no clock needed)
    f.reset->setOn(true);
    f.sim->update();
    QCOMPARE(f.readDataOut(), 0x0000);

    // Release Reset: memory stays cleared
    f.reset->setOn(false);
    f.sim->update();
    QCOMPARE(f.readDataOut(), 0x0000);
}

// Distinct words must be stored at distinct addresses and read back
// independently, and the documented partial decode (only Address[0:2] reach
// the two 8-word RAM lanes; high bits alias mod 8) must hold for the full
// 16-bit word (both byte lanes), not just one.
void TestLevel9MemoryStage16Bit::testMultiAddressStorage()
{
    auto &f = *s_level9MemoryStage16bit;

    auto writeAt = [&](int addr, int data) {
        setMultiBitInput(f.addressInputs, addr);
        setMultiBitInput(f.datainInputs, data);
        setMultiBitInput(f.resultInputs, 0x0000);
        f.memread->setOn(false);
        f.memwrite->setOn(true);
        f.sim->update();
        clockCycle(f.sim, f.clk);
        f.memwrite->setOn(false);
        f.sim->update();
    };
    auto readAt = [&](int addr) {
        setMultiBitInput(f.addressInputs, addr);
        f.memread->setOn(true);
        f.sim->update();
        int v = f.readDataOut();
        f.memread->setOn(false);
        f.sim->update();
        return v;
    };

    writeAt(0x02, 0x1111);
    writeAt(0x05, 0x2222);

    // Each address holds its own word
    QCOMPARE(readAt(0x02), 0x1111);
    QCOMPARE(readAt(0x05), 0x2222);

    // Partial decode: Address[3..7] are ignored, so 0x0A (= 0x02 + 8) aliases 0x02
    QCOMPARE(readAt(0x0A), 0x1111);
}

void TestLevel9MemoryStage16Bit::testResultPassthroughBitIsolation_data()
{
    QTest::addColumn<int>("bitPosition");
    for (int i = 0; i < 16; ++i) {
        QTest::newRow(QString("result_bit_%1").arg(i).toLatin1()) << i;
    }
}

void TestLevel9MemoryStage16Bit::testResultPassthroughBitIsolation()
{
    QFETCH(int, bitPosition);

    auto &f = *s_level9MemoryStage16bit;

    // Passthrough mode (MemRead=0, MemWrite=0): a one-hot Result must appear as a
    // one-hot DataOut at the same position — no bit-lane cross-wiring in the
    // output mux's In0 path, in either byte lane.
    f.memread->setOn(false);
    f.memwrite->setOn(false);
    setMultiBitInput(f.resultInputs, 1 << bitPosition);
    f.sim->update();

    QCOMPARE(f.readDataOut(), 1 << bitPosition);
}

void TestLevel9MemoryStage16Bit::testStoredDataBitIsolation_data()
{
    QTest::addColumn<int>("bitPosition");
    for (int i = 0; i < 16; ++i) {
        QTest::newRow(QString("data_bit_%1").arg(i).toLatin1()) << i;
    }
}

void TestLevel9MemoryStage16Bit::testStoredDataBitIsolation()
{
    QFETCH(int, bitPosition);

    auto &f = *s_level9MemoryStage16bit;

    // Store a one-hot DataIn, then read it back: the word must round-trip as a
    // one-hot DataOut — proving DataIn[i] -> memory -> DataOut[i] with no
    // cross-wiring through either RAM lane and the read mux, for every bit
    // including across the low/high byte-lane boundary (bits 7/8).
    setMultiBitInput(f.addressInputs, 0x04);
    setMultiBitInput(f.datainInputs, 1 << bitPosition);
    setMultiBitInput(f.resultInputs, 0x0000);
    f.memread->setOn(false);
    f.memwrite->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clk);
    f.memwrite->setOn(false);

    f.memread->setOn(true);
    f.sim->update();
    QCOMPARE(f.readDataOut(), 1 << bitPosition);
    f.memread->setOn(false);
}
