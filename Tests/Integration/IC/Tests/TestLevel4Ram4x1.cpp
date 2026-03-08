// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4Ram4x1.h"

#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/Cpu/CpuCommon.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct Ram4x1Fixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ramIC = nullptr;
    InputSwitch *addressBits[2] = {};
    InputSwitch *dataIn = nullptr;
    InputSwitch *writeEnable = nullptr;
    InputSwitch *clock = nullptr;
    Led *dataOut = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        for (int i = 0; i < 2; ++i) {
            addressBits[i] = new InputSwitch();
            builder.add(addressBits[i]);
        }
        dataIn = new InputSwitch();
        writeEnable = new InputSwitch();
        clock = new InputSwitch();
        dataOut = new Led();
        builder.add(dataIn, writeEnable, clock, dataOut);

        ramIC = loadBuildingBlockIC("level4_ram_4x1.panda");
        builder.add(ramIC);

        builder.connect(addressBits[0], 0, ramIC, "Address[0]");
        builder.connect(addressBits[1], 0, ramIC, "Address[1]");
        builder.connect(dataIn, 0, ramIC, "DataIn");
        builder.connect(writeEnable, 0, ramIC, "WriteEnable");
        builder.connect(clock, 0, ramIC, "Clock");
        builder.connect(ramIC, "DataOut", dataOut, 0);

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    void setAddress(int addr)
    {
        addressBits[0]->setOn((addr >> 0) & 1);
        addressBits[1]->setOn((addr >> 1) & 1);
    }

    void writeData(bool value, int addr)
    {
        setAddress(addr);
        dataIn->setOn(value);
        writeEnable->setOn(true);
        sim->update();
        TestUtils::clockCycle(sim, clock);
        writeEnable->setOn(false);
        sim->update();
    }

    bool readData(int addr)
    {
        setAddress(addr);
        sim->update();
        return getInputStatus(dataOut);
    }
};

static std::unique_ptr<Ram4x1Fixture> s_level4Ram4x1;

void TestLevel4RAM4X1::initTestCase()
{
    s_level4Ram4x1 = std::make_unique<Ram4x1Fixture>();
    QVERIFY(s_level4Ram4x1->build());
}

void TestLevel4RAM4X1::cleanupTestCase()
{
    s_level4Ram4x1.reset();
}

void TestLevel4RAM4X1::cleanup()
{
    if (s_level4Ram4x1 && s_level4Ram4x1->sim) {
        s_level4Ram4x1->sim->restart();
        s_level4Ram4x1->sim->update();
    }
}

void TestLevel4RAM4X1::testRamArray4x4_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<bool>("dataToWrite");

    QTest::newRow("write_addr0_1") << 0 << true;
    QTest::newRow("write_addr1_0") << 1 << false;
    QTest::newRow("write_addr2_1") << 2 << true;
    QTest::newRow("write_addr3_0") << 3 << false;
}

/**
 * @brief Build a 4x4 single-bit RAM array using RAM4x1 IC
 *
 * Architecture:
 * - RAM4x1 IC: Complete 4-cell, 1-bit wide RAM with internal decoder and mux
 *
 * IC Ports:
 *   - Inputs: Address[0], Address[1], DataIn, WriteEnable, Clock
 *   - Output: DataOut
 */
void TestLevel4RAM4X1::testRamArray4x4()
{
    QFETCH(int, address);
    QFETCH(bool, dataToWrite);

    auto &f = *s_level4Ram4x1;

    f.writeData(dataToWrite, address);
    QCOMPARE(f.readData(address), dataToWrite);
}

void TestLevel4RAM4X1::testPatternTest()
{
    auto &f = *s_level4Ram4x1;
    const bool pattern[4] = {true, false, true, false};

    for (int testAddr = 0; testAddr < 4; ++testAddr) {
        f.writeData(pattern[testAddr], testAddr);
        QCOMPARE(f.readData(testAddr), pattern[testAddr]);
    }
}

void TestLevel4RAM4X1::testWeGating()
{
    auto &f = *s_level4Ram4x1;

    f.setAddress(1);
    f.dataIn->setOn(true);
    f.writeEnable->setOn(false);  // WE disabled
    TestUtils::clockCycle(f.sim, f.clock);
    f.sim->update();

    QCOMPARE(getInputStatus(f.dataOut), false);
}

void TestLevel4RAM4X1::testHoldBehavior()
{
    auto &f = *s_level4Ram4x1;

    // Write 1 to address 2
    f.writeData(true, 2);
    QCOMPARE(f.readData(2), true);

    // Try to overwrite with WE=0
    f.setAddress(2);
    f.dataIn->setOn(false);
    f.writeEnable->setOn(false);
    TestUtils::clockCycle(f.sim, f.clock);
    f.sim->update();

    QCOMPARE(getInputStatus(f.dataOut), true);  // Should still hold 1
}

void TestLevel4RAM4X1::testIsolation()
{
    auto &f = *s_level4Ram4x1;

    f.writeData(true, 0);
    f.writeData(false, 1);

    QCOMPARE(f.readData(0), true);
    QCOMPARE(f.readData(1), false);
}

void TestLevel4RAM4X1::testSequentialWrite()
{
    auto &f = *s_level4Ram4x1;
    const bool writeSequence[] = {true, false, true, false};

    for (int writeIdx = 0; writeIdx < 4; ++writeIdx) {
        f.writeData(writeSequence[writeIdx], 0);
        QCOMPARE(getInputStatus(f.dataOut), writeSequence[writeIdx]);
    }
}

