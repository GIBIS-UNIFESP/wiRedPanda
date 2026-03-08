// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel6RegisterFile8x8.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using TestUtils::clockCycle;
using CPUTestUtils::loadBuildingBlockIC;

struct RegFile6_8x8Fixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *writeAddr[3] = {};
    InputSwitch *readAddr1In[3] = {};
    InputSwitch *readAddr2In[3] = {};
    InputSwitch *dataIn[8] = {};
    InputSwitch *we = nullptr;
    InputSwitch *clk = nullptr;
    Led *readData1[8] = {};
    Led *readData2[8] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level6_register_file_8x8.panda");

        we = new InputSwitch();
        clk = new InputSwitch();
        builder.add(we, clk, ic);

        for (int i = 0; i < 3; ++i) {
            writeAddr[i] = new InputSwitch();
            readAddr1In[i] = new InputSwitch();
            readAddr2In[i] = new InputSwitch();
            builder.add(writeAddr[i], readAddr1In[i], readAddr2In[i]);
        }
        for (int i = 0; i < 8; ++i) {
            dataIn[i] = new InputSwitch();
            readData1[i] = new Led();
            readData2[i] = new Led();
            builder.add(dataIn[i], readData1[i], readData2[i]);
        }

        for (int i = 0; i < 3; ++i) {
            builder.connect(writeAddr[i], 0, ic, QString("Write_Addr[%1]").arg(i));
            builder.connect(readAddr1In[i], 0, ic, QString("Read_Addr1[%1]").arg(i));
            builder.connect(readAddr2In[i], 0, ic, QString("Read_Addr2[%1]").arg(i));
        }

        for (int i = 0; i < 8; ++i) {
            builder.connect(dataIn[i], 0, ic, QString("Data_In[%1]").arg(i));
            builder.connect(ic, QString("Read_Data1[%1]").arg(i), readData1[i], 0);
            builder.connect(ic, QString("Read_Data2[%1]").arg(i), readData2[i], 0);
        }

        builder.connect(we, 0, ic, "Write_Enable");
        builder.connect(clk, 0, ic, "Clock");

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<RegFile6_8x8Fixture> s_level6RegFile8x8;

void TestLevel6RegisterFile8X8::initTestCase()
{
    s_level6RegFile8x8 = std::make_unique<RegFile6_8x8Fixture>();
    QVERIFY(s_level6RegFile8x8->build());
}

void TestLevel6RegisterFile8X8::cleanupTestCase()
{
    s_level6RegFile8x8.reset();
}

void TestLevel6RegisterFile8X8::cleanup()
{
    if (s_level6RegFile8x8 && s_level6RegFile8x8->sim) {
        s_level6RegFile8x8->sim->restart();
        s_level6RegFile8x8->sim->update();
    }
}

void TestLevel6RegisterFile8X8::testRegisterFile_data()
{
    QTest::addColumn<int>("readAddr");

    QTest::newRow("read_addr_0") << 0;
    QTest::newRow("read_addr_3") << 3;
    QTest::newRow("read_addr_7") << 7;
    QTest::newRow("read_addr_5") << 5;
}

void TestLevel6RegisterFile8X8::testRegisterFile()
{
    QFETCH(int, readAddr);

    auto &f = *s_level6RegFile8x8;

    // Initialize
    f.we->setOn(false);
    f.clk->setOn(false);
    f.sim->update();

    // Write a distinctive pattern
    int testData = 0xAA ^ readAddr;

    // Set write address
    for (int i = 0; i < 3; ++i) {
        f.writeAddr[i]->setOn((readAddr >> i) & 1);
    }
    f.sim->update();

    // Set data to write
    for (int i = 0; i < 8; ++i) {
        f.dataIn[i]->setOn((testData >> i) & 1);
    }
    f.sim->update();

    // Enable write
    f.we->setOn(true);
    f.sim->update();

    // Apply clock pulse
    clockCycle(f.sim, f.clk);

    // Disable write
    f.we->setOn(false);
    f.sim->update();

    // Set read addresses
    for (int i = 0; i < 3; ++i) {
        f.readAddr1In[i]->setOn((readAddr >> i) & 1);
        f.readAddr2In[i]->setOn((readAddr >> i) & 1);
    }
    f.sim->update();

    // Allow read to propagate
    f.sim->update();

    // Read outputs from both ports
    int readValue1 = 0, readValue2 = 0;
    for (int i = 0; i < 8; ++i) {
        if (getInputStatus(f.readData1[i])) {
            readValue1 |= (1 << i);
        }
        if (getInputStatus(f.readData2[i])) {
            readValue2 |= (1 << i);
        }
    }

    QCOMPARE(readValue1, readValue2);
    QCOMPARE(readValue1, testData);
}

