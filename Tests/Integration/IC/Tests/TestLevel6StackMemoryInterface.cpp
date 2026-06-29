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

using TestUtils::getInputStatus;
using TestUtils::clockCycle;
using CPUTestUtils::loadBuildingBlockIC;

struct StackMemoryInterfaceFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *dataInSwitches[8] = {};
    InputSwitch *addressSwitches[8] = {};
    InputSwitch *spLoad = nullptr;
    InputSwitch *spPush = nullptr;
    InputSwitch *spPop = nullptr;
    InputSwitch *spReset = nullptr;
    InputSwitch *addressSelect = nullptr;
    InputSwitch *memWrite = nullptr;
    InputSwitch *clk = nullptr;
    Led *spOut[8] = {};
    Led *dataOutLeds[8] = {};
    Led *finalAddrLeds[8] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level6_stack_memory_interface.panda");

        spLoad = new InputSwitch();
        spPush = new InputSwitch();
        spPop = new InputSwitch();
        spReset = new InputSwitch();
        addressSelect = new InputSwitch();
        memWrite = new InputSwitch();
        clk = new InputSwitch();

        builder.add(spLoad, spPush, spPop, spReset, addressSelect,
                    memWrite, clk, ic);

        for (int i = 0; i < 8; ++i) {
            dataInSwitches[i] = new InputSwitch();
            addressSwitches[i] = new InputSwitch();
            spOut[i] = new Led();
            dataOutLeds[i] = new Led();
            finalAddrLeds[i] = new Led();
            builder.add(dataInSwitches[i], addressSwitches[i], spOut[i], dataOutLeds[i], finalAddrLeds[i]);
        }

        for (int i = 0; i < 8; ++i) {
            builder.connect(dataInSwitches[i], 0, ic, QString("DataIn[%1]").arg(i));
            builder.connect(addressSwitches[i], 0, ic, QString("Address[%1]").arg(i));
            builder.connect(ic, QString("SP[%1]").arg(i), spOut[i], 0);
            builder.connect(ic, QString("DataOut[%1]").arg(i), dataOutLeds[i], 0);
            builder.connect(ic, QString("FinalAddress[%1]").arg(i), finalAddrLeds[i], 0);
        }

        builder.connect(spLoad, 0, ic, "SP_Load");
        builder.connect(spPush, 0, ic, "SP_Push");
        builder.connect(spPop, 0, ic, "SP_Pop");
        builder.connect(spReset, 0, ic, "SP_Reset");
        builder.connect(addressSelect, 0, ic, "AddressSelect");
        builder.connect(memWrite, 0, ic, "MemWrite");
        builder.connect(clk, 0, ic, "Clock");

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    int readSP()
    {
        int v = 0;
        for (int i = 0; i < 8; ++i) {
            if (TestUtils::getInputStatus(spOut[i])) {
                v |= (1 << i);
            }
        }
        return v;
    }

    int readDataOut()
    {
        int v = 0;
        for (int i = 0; i < 8; ++i) {
            if (TestUtils::getInputStatus(dataOutLeds[i])) {
                v |= (1 << i);
            }
        }
        return v;
    }

    void pulse(InputSwitch *ctrl)
    {
        ctrl->setOn(true);
        sim->update();
        clockCycle(sim, clk);
        ctrl->setOn(false);
        sim->update();
    }
};

static std::unique_ptr<StackMemoryInterfaceFixture> s_level6StackMemIface;

void TestLevel6StackMemoryInterface::initTestCase()
{
    s_level6StackMemIface = std::make_unique<StackMemoryInterfaceFixture>();
    QVERIFY(s_level6StackMemIface->build());
}

void TestLevel6StackMemoryInterface::cleanupTestCase()
{
    s_level6StackMemIface.reset();
}

void TestLevel6StackMemoryInterface::cleanup()
{
    if (s_level6StackMemIface && s_level6StackMemIface->sim) {
        s_level6StackMemIface->sim->restart();
        s_level6StackMemIface->sim->update();
    }
}

void TestLevel6StackMemoryInterface::testStackMemoryInterface_data()
{
    QTest::addColumn<int>("externalAddress");
    QTest::addColumn<int>("dataToWrite");
    QTest::addColumn<int>("useStackPointer");

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

    auto &f = *s_level6StackMemIface;

    // Initialize
    f.clk->setOn(false);
    f.spLoad->setOn(false);
    f.spPush->setOn(false);
    f.spPop->setOn(false);
    f.spReset->setOn(false);
    f.memWrite->setOn(false);
    f.sim->update();

    // Set address select
    f.addressSelect->setOn(useStackPointer);

    // Set address for write
    for (int i = 0; i < 8; ++i) {
        f.addressSwitches[i]->setOn((externalAddress >> i) & 1);
    }
    f.sim->update();

    // Set data to write
    for (int i = 0; i < 8; ++i) {
        f.dataInSwitches[i]->setOn((dataToWrite >> i) & 1);
    }
    f.sim->update();

    // Enable write
    f.memWrite->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clk);

    // Disable write (reads are asynchronous — no MemRead gate)
    f.memWrite->setOn(false);
    f.sim->update();
    clockCycle(f.sim, f.clk);

    // Read the final address
    int finalAddress = 0;
    for (int i = 0; i < 8; ++i) {
        if (getInputStatus(f.finalAddrLeds[i])) {
            finalAddress |= (1 << i);
        }
    }

    int expectedAddress = useStackPointer ? 0xFF : externalAddress;
    QCOMPARE(finalAddress & 0xFF, expectedAddress & 0xFF);
}

// End-to-end stack semantics through this IC: push decrements SP, a write lands
// at the SP-addressed word, and popping returns values in LIFO order. The
// components (stack pointer, RAM, address mux) are tested separately, but the
// push -> write-via-SP -> pop -> read-via-SP round-trip was never exercised here.
void TestLevel6StackMemoryInterface::testStackLifoRoundTrip()
{
    auto &f = *s_level6StackMemIface;

    f.spLoad->setOn(false);
    f.spPush->setOn(false);
    f.spPop->setOn(false);
    f.memWrite->setOn(false);
    f.addressSelect->setOn(true);  // address comes from the stack pointer
    f.sim->update();
    f.pulse(f.spReset);
    QCOMPARE(f.readSP(), 0xFF);

    auto writeAtSp = [&](int value) {
        for (int i = 0; i < 8; ++i) {
            f.dataInSwitches[i]->setOn((value >> i) & 1);
        }
        f.sim->update();
        f.pulse(f.memWrite);
    };

    // Push, then store 0xAB at SP = 0xFE
    f.pulse(f.spPush);
    QCOMPARE(f.readSP(), 0xFE);
    writeAtSp(0xAB);
    QCOMPARE(f.readDataOut(), 0xAB);  // DataOut reflects the SP-addressed word

    // Push, then store 0xCD at SP = 0xFD
    f.pulse(f.spPush);
    QCOMPARE(f.readSP(), 0xFD);
    writeAtSp(0xCD);
    QCOMPARE(f.readDataOut(), 0xCD);

    // Pop -> SP = 0xFE: the word read back must be the earlier 0xAB (LIFO)
    f.pulse(f.spPop);
    QCOMPARE(f.readSP(), 0xFE);
    QCOMPARE(f.readDataOut(), 0xAB);

    // Pop -> SP = 0xFF (stack emptied back to its reset top)
    f.pulse(f.spPop);
    QCOMPARE(f.readSP(), 0xFF);
}
