// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel9RegisterFile32x16.h"

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

struct RegFile32x16Fixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    QVector<InputSwitch *> writeAddr;
    QVector<InputSwitch *> readAddr1;
    QVector<InputSwitch *> dataIn;
    InputSwitch *we = nullptr;
    InputSwitch *clk = nullptr;
    QVector<Led *> readData1;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level9_register_file_32x16.panda");
        builder.add(ic);

        we = new InputSwitch(); builder.add(we);
        clk = new InputSwitch(); builder.add(clk);

        for (int i = 0; i < 5; ++i) {
            auto *w = new InputSwitch(); builder.add(w); writeAddr.append(w);
            auto *r = new InputSwitch(); builder.add(r); readAddr1.append(r);
        }
        for (int i = 0; i < 16; ++i) {
            auto *d = new InputSwitch(); builder.add(d); dataIn.append(d);
            auto *led = new Led(); builder.add(led); readData1.append(led);
        }

        for (int i = 0; i < 5; ++i) {
            builder.connect(writeAddr[i], 0, ic, QString("Write_Addr[%1]").arg(i));
            builder.connect(readAddr1[i], 0, ic, QString("Read_Addr1[%1]").arg(i));
        }
        for (int i = 0; i < 16; ++i) {
            builder.connect(dataIn[i], 0, ic, QString("Data_In[%1]").arg(i));
            builder.connect(ic, QString("Read_Data1[%1]").arg(i), readData1[i], 0);
        }
        builder.connect(we, 0, ic, "WriteEnable");
        builder.connect(clk, 0, ic, "Clock");

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    int readData() { return readMultiBitOutput(QVector<GraphicElement *>(readData1.begin(), readData1.end())); }

    void writeAt(int addr, int value)
    {
        setMultiBitInput(writeAddr, addr);
        setMultiBitInput(dataIn, value);
        we->setOn(true);
        sim->update();
        clockCycle(sim, clk);
        we->setOn(false);
        sim->update();
    }

    int readAt(int addr)
    {
        setMultiBitInput(readAddr1, addr);
        sim->update();
        return readData();
    }
};

static std::unique_ptr<RegFile32x16Fixture> s_level9RegFile32x16;

void TestLevel9RegisterFile32X16::initTestCase()
{
    s_level9RegFile32x16 = std::make_unique<RegFile32x16Fixture>();
    QVERIFY(s_level9RegFile32x16->build());
}

void TestLevel9RegisterFile32X16::cleanupTestCase()
{
    s_level9RegFile32x16.reset();
}

void TestLevel9RegisterFile32X16::cleanup()
{
    if (s_level9RegFile32x16 && s_level9RegFile32x16->sim) {
        s_level9RegFile32x16->sim->restart();
        s_level9RegFile32x16->sim->update();
    }
}

void TestLevel9RegisterFile32X16::testRegisterFileStructure()
{
    auto &f = *s_level9RegFile32x16;
    QVERIFY(f.ic != nullptr);
    // Write_Addr[5] + Read_Addr1[5] + Data_In[16] + WriteEnable + Clock
    QCOMPARE(f.ic->inputSize(), 28);
    // Read_Data1[16] -- single read port (num_read_ports=1: OperandA in the
    // RISC CPU this was built for is always an immediate, never a register)
    QCOMPARE(f.ic->outputSize(), 16);
}

void TestLevel9RegisterFile32X16::testRegisterFile_data()
{
    QTest::addColumn<int>("addr");

    // Addresses spanning all four groups of the read port's mux tree (level 0
    // groups registers 0-7/8-15/16-23/24-31 into <=8-input muxes, since a flat
    // 37-port mux would exceed the engine's Mux port cap -- these specifically
    // exercise the group boundaries, not just one group repeatedly).
    QTest::newRow("addr_0_group0_first") << 0;
    QTest::newRow("addr_7_group0_last") << 7;
    QTest::newRow("addr_8_group1_first") << 8;
    QTest::newRow("addr_15_group1_last") << 15;
    QTest::newRow("addr_16_group2_first") << 16;
    QTest::newRow("addr_23_group2_last") << 23;
    QTest::newRow("addr_24_group3_first") << 24;
    QTest::newRow("addr_31_group3_last") << 31;
}

void TestLevel9RegisterFile32X16::testRegisterFile()
{
    QFETCH(int, addr);

    auto &f = *s_level9RegFile32x16;

    // Distinctive per-address pattern so aliasing between registers would be caught.
    int testData = (0xA5A5 ^ (addr * 0x1111)) & 0xFFFF;

    f.writeAt(addr, testData);

    QCOMPARE(f.readAt(addr), testData);
}

// Writes distinct words to registers in every mux-tree group and confirms each
// reads back independently -- the tree-based read port (added because a flat
// 37-port Mux exceeds the engine's port cap) routes each bit through several
// muxes across two levels, so a wrong wire could alias two registers in
// different groups without any single-address test catching it.
void TestLevel9RegisterFile32X16::testMultiAddressStorage()
{
    auto &f = *s_level9RegFile32x16;

    const QVector<QPair<int, int>> writes = {
        {0, 0x1111}, {5, 0x2222}, {8, 0x3333}, {13, 0x4444},
        {16, 0x5555}, {21, 0x6666}, {24, 0x7777}, {31, 0x8888},
    };

    for (const auto &write : writes) {
        f.writeAt(write.first, write.second);
    }

    for (const auto &write : writes) {
        QCOMPARE(f.readAt(write.first), write.second);
    }
}
