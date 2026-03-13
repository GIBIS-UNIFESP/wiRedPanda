// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel6RegisterFile8x8.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using CPUTestUtils::loadBuildingBlockIC;

void TestLevel6RegisterFile8X8::initTestCase()
{
}

void TestLevel6RegisterFile8X8::cleanup()
{
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

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch writeAddr[3], readAddr1In[3], readAddr2In[3];
    InputSwitch dataIn[8];
    Led readData1[8], readData2[8];
    InputSwitch we, clk;

    IC *regFileIC = loadBuildingBlockIC("level6_register_file_8x8.panda");

    builder.add(&we, &clk, regFileIC);
    for (int i = 0; i < 3; ++i) {
        builder.add(&writeAddr[i], &readAddr1In[i], &readAddr2In[i]);
    }
    for (int i = 0; i < 8; ++i) {
        builder.add(&dataIn[i], &readData1[i], &readData2[i]);
    }

    for (int i = 0; i < 3; ++i) {
        builder.connect(&writeAddr[i], 0, regFileIC, QString("Write_Addr[%1]").arg(i));
        builder.connect(&readAddr1In[i], 0, regFileIC, QString("Read_Addr1[%1]").arg(i));
        builder.connect(&readAddr2In[i], 0, regFileIC, QString("Read_Addr2[%1]").arg(i));
    }

    for (int i = 0; i < 8; ++i) {
        builder.connect(&dataIn[i], 0, regFileIC, QString("Data_In[%1]").arg(i));
        builder.connect(regFileIC, QString("Read_Data1[%1]").arg(i), &readData1[i], 0);
        builder.connect(regFileIC, QString("Read_Data2[%1]").arg(i), &readData2[i], 0);
    }

    builder.connect(&we, 0, regFileIC, "Write_Enable");
    builder.connect(&clk, 0, regFileIC, "Clock");

    auto *simulation = builder.initSimulation();

    // Initialize
    we.setOn(false);
    clk.setOn(false);
    simulation->update();

    // ========== WRITE DATA TO REGISTER ==========
    // Write a distinctive pattern: invert bits for non-zero pattern
    int testData = 0xAA ^ readAddr;  // Create unique pattern with guaranteed non-zero for most cases

    // Set write address
    for (int i = 0; i < 3; ++i) {
        writeAddr[i].setOn((readAddr >> i) & 1);
    }
    simulation->update();

    // Set data to write
    for (int i = 0; i < 8; ++i) {
        dataIn[i].setOn((testData >> i) & 1);
    }
    simulation->update();

    // Enable write
    we.setOn(true);
    simulation->update();

    // Apply clock pulse
    TestUtils::clockCycle(simulation, &clk);

    // Disable write
    we.setOn(false);
    simulation->update();

    // ========== READ FROM REGISTER ==========
    // Set read addresses
    for (int i = 0; i < 3; ++i) {
        readAddr1In[i].setOn((readAddr >> i) & 1);
        readAddr2In[i].setOn((readAddr >> i) & 1);  // Same address on both ports
    }
    simulation->update();

    // Allow read to propagate
    simulation->update();

    // Read outputs from both ports
    int readValue1 = 0, readValue2 = 0;
    for (int i = 0; i < 8; ++i) {
        if (TestUtils::getInputStatus(&readData1[i])) {
            readValue1 |= (1 << i);
        }
        if (TestUtils::getInputStatus(&readData2[i])) {
            readValue2 |= (1 << i);
        }
    }

    // Verify both ports return the same value
    QCOMPARE(readValue1, readValue2);
    // Verify outputs match what we wrote
    QCOMPARE(readValue1, testData);
}
