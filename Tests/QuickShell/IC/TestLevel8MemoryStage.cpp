// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/IC/TestLevel8MemoryStage.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/QuickShell/IC/CpuTestUtils.h"
#include "Tests/QuickShell/IC/QuickTestUtils.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"

using QuickTestUtils::readMultiBitOutput;
using QuickTestUtils::setMultiBitInput;
using QuickTestUtils::clockCycle;
using CPUTestUtils::loadBuildingBlockIC;

struct MemoryStageFixture {
    std::unique_ptr<QuickCircuitBuilder> builder;
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
        builder = std::make_unique<QuickCircuitBuilder>();

        ic = loadBuildingBlockIC("level8_memory_stage.panda");
        builder->addOwnedElement(ic);

        for (int i = 0; i < 8; i++) {
            auto *a = new InputSwitch(); builder->addOwnedElement(a); addressInputs.append(a);
            auto *d = new InputSwitch(); builder->addOwnedElement(d); datainInputs.append(d);
            auto *r = new InputSwitch(); builder->addOwnedElement(r); resultInputs.append(r);
            auto *led = new Led(); builder->addOwnedElement(led); dataoutLeds.append(led);
        }

        memread = new InputSwitch(); builder->addOwnedElement(memread);
        memwrite = new InputSwitch(); builder->addOwnedElement(memwrite);
        clk = new InputSwitch(); builder->addOwnedElement(clk);
        reset = new InputSwitch(); builder->addOwnedElement(reset);

        for (int i = 0; i < 8; i++) {
            builder->connect(addressInputs[i], 0, ic, QString("Address[%1]").arg(i));
            builder->connect(datainInputs[i], 0, ic, QString("DataIn[%1]").arg(i));
            builder->connect(resultInputs[i], 0, ic, QString("Result[%1]").arg(i));
            builder->connect(ic, QString("DataOut[%1]").arg(i), dataoutLeds[i], 0);
        }

        builder->connect(memread, 0, ic, "MemRead");
        builder->connect(memwrite, 0, ic, "MemWrite");
        builder->connect(clk, 0, ic, "Clock");
        builder->connect(reset, 0, ic, "Reset");

        sim = builder->initSimulation();
        sim->update();
        return true;
    }

    int readDataOut()
    {
        return readMultiBitOutput(QVector<GraphicElement *>(dataoutLeds.begin(), dataoutLeds.end()), 0);
    }
};

static std::unique_ptr<MemoryStageFixture> s_level8MemoryStage;

void TestLevel8MemoryStage::initTestCase()
{
    s_level8MemoryStage = std::make_unique<MemoryStageFixture>();
    QVERIFY(s_level8MemoryStage->build());
}

void TestLevel8MemoryStage::cleanupTestCase()
{
    s_level8MemoryStage.reset();
}

void TestLevel8MemoryStage::cleanup()
{
    if (s_level8MemoryStage && s_level8MemoryStage->sim) {
        s_level8MemoryStage->sim->restart();
        s_level8MemoryStage->sim->update();
    }
}

void TestLevel8MemoryStage::testMemoryStage_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("dataIn");
    QTest::addColumn<int>("result");
    QTest::addColumn<bool>("memRead");
    QTest::addColumn<bool>("memWrite");
    QTest::addColumn<int>("expectedOut");

    QTest::newRow("passthrough 0x84 (no mem op)") << 0x00 << 0x00 << 0x84 << false << false << 0x84;
    QTest::newRow("passthrough 0x00") << 0xFF << 0x00 << 0x00 << false << false << 0x00;
    QTest::newRow("passthrough 0xAA") << 0x55 << 0xBB << 0xAA << false << false << 0xAA;

    QTest::newRow("write 0x42 to addr 0x00") << 0x00 << 0x42 << 0xFF << false << true << 0xFF;
    QTest::newRow("write 0x55 to addr 0x10") << 0x10 << 0x55 << 0x77 << false << true << 0x77;

    QTest::newRow("read from addr 0x00 (empty)") << 0x00 << 0x00 << 0xFF << true << false << 0x00;
    QTest::newRow("read from addr 0x10 (empty)") << 0x10 << 0x00 << 0xFF << true << false << 0x00;
}

void TestLevel8MemoryStage::testMemoryStage()
{
    QFETCH(int, address);
    QFETCH(int, dataIn);
    QFETCH(int, result);
    QFETCH(bool, memRead);
    QFETCH(bool, memWrite);
    QFETCH(int, expectedOut);

    auto &f = *s_level8MemoryStage;

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

// MemRead and MemWrite asserted simultaneously -- the 4th combination of the two independent
// control switches, never exercised by testMemoryStage_data() (which only covers passthrough,
// write-only, and read-only). Empirically confirmed (via this exact sequence): MemRead always
// wins on DataOut (it reflects the RAM's current content), and a same-cycle MemWrite updates
// that same address on the clock edge -- so DataOut reads the OLD value before the edge and
// the JUST-WRITTEN value after it (real read-after-write-same-address RAM behavior, not an
// undefined/don't-care combination).
void TestLevel8MemoryStage::testMemReadAndWriteBothAsserted()
{
    auto &f = *s_level8MemoryStage;

    // Pre-populate address 0x05 with a known, distinct value via a normal write.
    setMultiBitInput(f.addressInputs, 0x05);
    setMultiBitInput(f.datainInputs, 0x77);
    setMultiBitInput(f.resultInputs, 0x00);
    f.memread->setOn(false);
    f.memwrite->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clk);
    f.memwrite->setOn(false);
    f.sim->update();

    // Now assert both MemRead and MemWrite at the same address, with a DIFFERENT DataIn.
    setMultiBitInput(f.datainInputs, 0x33);
    f.memread->setOn(true);
    f.memwrite->setOn(true);
    f.sim->update();
    QCOMPARE(f.readDataOut(), 0x77);

    clockCycle(f.sim, f.clk);
    QCOMPARE(f.readDataOut(), 0x33);
}

void TestLevel8MemoryStage::testMemoryStageStructure()
{
    auto &f = *s_level8MemoryStage;

    QVERIFY(f.ic != nullptr);
    QCOMPARE(f.ic->inputSize(), 28);
    QCOMPARE(f.ic->outputSize(), 8);
}

// Reset clears the backing memory, implemented as an async clear down the RAM
// stack (the ram_8x1 cells' ~Clear).
void TestLevel8MemoryStage::testMemoryStageReset()
{
    auto &f = *s_level8MemoryStage;

    // Write 0x5A to address 3
    setMultiBitInput(f.addressInputs, 0x03);
    setMultiBitInput(f.datainInputs, 0x5A);
    setMultiBitInput(f.resultInputs, 0x00);
    f.memread->setOn(false);
    f.memwrite->setOn(true);
    f.reset->setOn(false);
    f.sim->update();
    clockCycle(f.sim, f.clk);

    // Read it back
    f.memwrite->setOn(false);
    f.memread->setOn(true);
    f.sim->update();
    QCOMPARE(f.readDataOut(), 0x5A);

    // Assert Reset: the stored word must clear (async, no clock needed)
    f.reset->setOn(true);
    f.sim->update();
    QCOMPARE(f.readDataOut(), 0x00);

    // Release Reset: memory stays cleared
    f.reset->setOn(false);
    f.sim->update();
    QCOMPARE(f.readDataOut(), 0x00);
}

// Distinct words must be stored at distinct addresses and read back
// independently — the existing tests only ever round-trip a single address, so a
// bug aliasing all addresses to one word would pass. Also asserts the documented
// partial decode (only Address[0:2] reach the 8-word RAM; high bits alias mod 8).
void TestLevel8MemoryStage::testMultiAddressStorage()
{
    auto &f = *s_level8MemoryStage;

    auto writeAt = [&](int addr, int data) {
        setMultiBitInput(f.addressInputs, addr);
        setMultiBitInput(f.datainInputs, data);
        setMultiBitInput(f.resultInputs, 0x00);
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

    writeAt(0x02, 0x11);
    writeAt(0x05, 0x22);

    // Each address holds its own word
    QCOMPARE(readAt(0x02), 0x11);
    QCOMPARE(readAt(0x05), 0x22);

    // Partial decode: Address[3..7] are ignored, so 0x0A (= 0x02 + 8) aliases 0x02
    QCOMPARE(readAt(0x0A), 0x11);
}

void TestLevel8MemoryStage::testResultPassthroughBitIsolation_data()
{
    QTest::addColumn<int>("bitPosition");
    for (int i = 0; i < 8; ++i) {
        QTest::newRow(QString("result_bit_%1").arg(i).toLatin1()) << i;
    }
}

void TestLevel8MemoryStage::testResultPassthroughBitIsolation()
{
    QFETCH(int, bitPosition);

    auto &f = *s_level8MemoryStage;

    // Passthrough mode (MemRead=0, MemWrite=0): a one-hot Result must appear as a
    // one-hot DataOut at the same position — no bit-lane cross-wiring in the
    // output mux's In0 path.
    f.memread->setOn(false);
    f.memwrite->setOn(false);
    setMultiBitInput(f.resultInputs, 1 << bitPosition);
    f.sim->update();

    QCOMPARE(f.readDataOut(), 1 << bitPosition);
}

void TestLevel8MemoryStage::testStoredDataBitIsolation_data()
{
    QTest::addColumn<int>("bitPosition");
    for (int i = 0; i < 8; ++i) {
        QTest::newRow(QString("data_bit_%1").arg(i).toLatin1()) << i;
    }
}

void TestLevel8MemoryStage::testStoredDataBitIsolation()
{
    QFETCH(int, bitPosition);

    auto &f = *s_level8MemoryStage;

    // Store a one-hot DataIn, then read it back: the word must round-trip as a
    // one-hot DataOut — proving DataIn[i] -> memory -> DataOut[i] with no
    // cross-wiring through the RAM and read mux.
    setMultiBitInput(f.addressInputs, 0x04);
    setMultiBitInput(f.datainInputs, 1 << bitPosition);
    setMultiBitInput(f.resultInputs, 0x00);
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
