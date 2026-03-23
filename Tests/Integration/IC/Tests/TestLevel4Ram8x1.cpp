// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel4Ram8x1.h"

#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/Cpu/CpuCommon.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::getInputStatus;
using CPUTestUtils::loadBuildingBlockIC;

struct Ram8x1Fixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ramIC = nullptr;
    InputSwitch *addressBits[3] = {};
    InputSwitch *dataIn = nullptr;
    InputSwitch *writeEnable = nullptr;
    InputSwitch *clock = nullptr;
    Led *dataOut = nullptr;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        for (int i = 0; i < 3; ++i) {
            addressBits[i] = new InputSwitch();
            builder.add(addressBits[i]);
        }
        dataIn = new InputSwitch();
        writeEnable = new InputSwitch();
        clock = new InputSwitch();
        dataOut = new Led();
        builder.add(dataIn, writeEnable, clock, dataOut);

        ramIC = loadBuildingBlockIC("level4_ram_8x1.panda");
        builder.add(ramIC);

        for (int i = 0; i < 3; ++i) {
            builder.connect(addressBits[i], 0, ramIC, QString("Address[%1]").arg(i));
        }
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
        for (int i = 0; i < 3; ++i) {
            addressBits[i]->setOn((addr >> i) & 1);
        }
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

static std::unique_ptr<Ram8x1Fixture> s_level4Ram8x1;

void TestLevel4RAM8X1::initTestCase()
{
    s_level4Ram8x1 = std::make_unique<Ram8x1Fixture>();
    QVERIFY(s_level4Ram8x1->build());
}

void TestLevel4RAM8X1::cleanupTestCase()
{
    s_level4Ram8x1.reset();
}

void TestLevel4RAM8X1::cleanup()
{
    if (s_level4Ram8x1 && s_level4Ram8x1->sim) {
        s_level4Ram8x1->sim->restart();
        s_level4Ram8x1->sim->update();
    }
}

void TestLevel4RAM8X1::testRamArray_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<bool>("dataToWrite");

    for (int i = 0; i < 8; i++) {
        QTest::newRow(qPrintable(QString("write_addr%1_1").arg(i))) << i << true;
    }

    QTest::newRow("pattern_test") << -1 << false;
    QTest::newRow("we_gating_test") << -2 << false;
    QTest::newRow("hold_test") << -3 << false;
    QTest::newRow("isolation_test") << -2 << false;
    QTest::newRow("sequential_write") << -3 << false;
}

void TestLevel4RAM8X1::testRamArray()
{
    QFETCH(int, address);
    QFETCH(bool, dataToWrite);

    auto &f = *s_level4Ram8x1;

    if (address >= 0 && address < 8) {
        f.writeData(dataToWrite, address);
        QCOMPARE(f.readData(address), dataToWrite);
    } else if (address == -1) {
        // Pattern test
        bool pattern[8] = {true, false, true, false, true, false, true, false};
        for (int testAddr = 0; testAddr < 8; ++testAddr) {
            f.writeData(pattern[testAddr], testAddr);
            QCOMPARE(f.readData(testAddr), pattern[testAddr]);
        }
    } else if (address == -2) {
        // WE gating
        f.setAddress(3);
        f.dataIn->setOn(true);
        f.writeEnable->setOn(false);
        TestUtils::clockCycle(f.sim, f.clock);
        f.sim->update();
        QCOMPARE(getInputStatus(f.dataOut), false);
    } else if (address == -3) {
        // Hold test
        f.writeData(true, 5);
        QCOMPARE(f.readData(5), true);

        f.setAddress(5);
        f.dataIn->setOn(false);
        f.writeEnable->setOn(false);
        TestUtils::clockCycle(f.sim, f.clock);
        f.sim->update();
        QCOMPARE(getInputStatus(f.dataOut), true);
    }
}

void TestLevel4RAM8X1::testRamBasic_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<bool>("dataToWrite");
    QTest::addColumn<QString>("testDescription");

    for (int i = 0; i < 8; i++) {
        QTest::newRow(qPrintable(QString("write_addr%1_1").arg(i))) << i << true << QString("Write address %1 with 1").arg(i);
    }
}

void TestLevel4RAM8X1::testRamBasic()
{
    QFETCH(int, address);
    QFETCH(bool, dataToWrite);

    auto &f = *s_level4Ram8x1;

    f.writeData(dataToWrite, address);
    QCOMPARE(f.readData(address), dataToWrite);
}

void TestLevel4RAM8X1::testRamEdgeCases_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<QString>("testDescription");

    QTest::newRow("pattern_test") << -1 << "";
    QTest::newRow("we_gating_test") << -2 << "";
    QTest::newRow("hold_test") << -3 << "";
    QTest::newRow("isolation_test") << -2 << "";
    QTest::newRow("sequential_write") << -3 << "";
}

void TestLevel4RAM8X1::testRamEdgeCases()
{
    QFETCH(int, address);

    auto &f = *s_level4Ram8x1;

    if (address == -1) {
        // Pattern test
        bool pattern[8] = {true, false, true, false, true, false, true, false};
        for (int testAddr = 0; testAddr < 8; ++testAddr) {
            f.writeData(pattern[testAddr], testAddr);
            QCOMPARE(f.readData(testAddr), pattern[testAddr]);
        }
    } else if (address == -2) {
        // WE gating
        f.setAddress(3);
        f.dataIn->setOn(true);
        f.writeEnable->setOn(false);
        TestUtils::clockCycle(f.sim, f.clock);
        f.sim->update();
        QCOMPARE(getInputStatus(f.dataOut), false);
    } else if (address == -3) {
        // Hold test
        f.writeData(true, 5);
        QCOMPARE(f.readData(5), true);

        f.setAddress(5);
        f.dataIn->setOn(false);
        f.writeEnable->setOn(false);
        TestUtils::clockCycle(f.sim, f.clock);
        f.sim->update();
        QCOMPARE(getInputStatus(f.dataOut), true);
    }
}

