// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel6StackMemoryInterface.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using CPUTestUtils::loadBuildingBlockIC;

void TestLevel6StackMemoryInterface::initTestCase()
{
}

void TestLevel6StackMemoryInterface::cleanup()
{
}

void TestLevel6StackMemoryInterface::testStackMemoryInterface_data()
{
    QTest::addColumn<int>("externalAddress");
    QTest::addColumn<int>("dataToWrite");
    QTest::addColumn<int>("useStackPointer");  // 0=use Address, 1=use SP

    // Test address selection (primary feature)
    QTest::newRow("addr_select_0x10") << 0x10 << 0x55 << 0;
    QTest::newRow("addr_select_0xFF") << 0xFF << 0xAA << 0;
    QTest::newRow("addr_select_0x00") << 0x00 << 0x33 << 0;
    QTest::newRow("use_sp_default") << 0x00 << 0xFF << 1;
}

void TestLevel6StackMemoryInterface::testStackMemoryInterface()
{
    QFETCH(int, externalAddress);
    QFETCH(int, dataToWrite);
    QFETCH(int, useStackPointer);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch dataInSwitches[8], addressSwitches[8];
    InputSwitch spLoad, spPush, spPop, spReset;
    InputSwitch addressSelect, memWrite, memRead;
    InputSwitch clk, enable;
    Led spOut[8], dataOutLeds[8], finalAddrLeds[8];

    IC *smiIC = loadBuildingBlockIC("level6_stack_memory_interface.panda");

    builder.add(&spLoad, &spPush, &spPop, &spReset, &addressSelect,
                &memWrite, &memRead, &clk, &enable, smiIC);
    for (int i = 0; i < 8; ++i) {
        builder.add(&dataInSwitches[i], &addressSwitches[i], &spOut[i], &dataOutLeds[i], &finalAddrLeds[i]);
    }

    for (int i = 0; i < 8; ++i) {
        builder.connect(&dataInSwitches[i], 0, smiIC, QString("DataIn[%1]").arg(i));
        builder.connect(&addressSwitches[i], 0, smiIC, QString("Address[%1]").arg(i));
        builder.connect(smiIC, QString("SP[%1]").arg(i), &spOut[i], 0);
        builder.connect(smiIC, QString("DataOut[%1]").arg(i), &dataOutLeds[i], 0);
        builder.connect(smiIC, QString("FinalAddress[%1]").arg(i), &finalAddrLeds[i], 0);
    }

    builder.connect(&spLoad, 0, smiIC, "SP_Load");
    builder.connect(&spPush, 0, smiIC, "SP_Push");
    builder.connect(&spPop, 0, smiIC, "SP_Pop");
    builder.connect(&spReset, 0, smiIC, "SP_Reset");
    builder.connect(&addressSelect, 0, smiIC, "AddressSelect");
    builder.connect(&memWrite, 0, smiIC, "MemWrite");
    builder.connect(&memRead, 0, smiIC, "MemRead");
    builder.connect(&clk, 0, smiIC, "Clock");
    builder.connect(&enable, 0, smiIC, "Enable");

    auto *simulation = builder.initSimulation();

    // Initialize and reset stack pointer to 0xFF
    clk.setOn(false);
    enable.setOn(true);
    spLoad.setOn(false);
    spPush.setOn(false);
    spPop.setOn(false);
    spReset.setOn(true);        // Assert SP reset (loads 0xFF)
    memWrite.setOn(false);
    memRead.setOn(false);
    simulation->update();
    TestUtils::clockCycle(simulation, &clk);  // Clock in reset value
    spReset.setOn(false);       // Release reset
    simulation->update();

    // Set address select (0=use Address input, 1=use SP)
    addressSelect.setOn(useStackPointer);

    // Set address for write
    for (int i = 0; i < 8; ++i) {
        addressSwitches[i].setOn((externalAddress >> i) & 1);
    }
    simulation->update();

    // Set data to write
    for (int i = 0; i < 8; ++i) {
        dataInSwitches[i].setOn((dataToWrite >> i) & 1);
    }
    simulation->update();

    // Enable write
    memWrite.setOn(true);
    simulation->update();
    TestUtils::clockCycle(simulation, &clk);

    // Disable write and enable read
    memWrite.setOn(false);
    memRead.setOn(true);
    simulation->update();
    TestUtils::clockCycle(simulation, &clk);

    // Read the final address (should be externalAddress when useStackPointer=0)
    int finalAddress = 0;
    for (int i = 0; i < 8; ++i) {
        if (TestUtils::getInputStatus(&finalAddrLeds[i])) {
            finalAddress |= (1 << i);
        }
    }

    // When useStackPointer=0, final address should be external address
    // When useStackPointer=1, final address should be SP (default 0xFF)
    int expectedAddress = useStackPointer ? 0xFF : externalAddress;
    QCOMPARE(finalAddress & 0xFF, expectedAddress & 0xFF);
}
