// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel5RegisterFile8x8.h"

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

struct RegFile8x8Fixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *writeAddr[3] = {};
    InputSwitch *readAddr[3] = {};
    InputSwitch *writeData[8] = {};
    InputSwitch *writeEnable = nullptr;
    InputSwitch *clock = nullptr;
    Led *readData[8] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level5_register_file_8x8.panda");
        builder.add(ic);

        writeEnable = new InputSwitch();
        clock = new InputSwitch();
        builder.add(writeEnable, clock);

        for (int i = 0; i < 3; ++i) {
            writeAddr[i] = new InputSwitch();
            readAddr[i] = new InputSwitch();
            builder.add(writeAddr[i], readAddr[i]);
        }
        for (int i = 0; i < 8; ++i) {
            writeData[i] = new InputSwitch();
            readData[i] = new Led();
            builder.add(writeData[i], readData[i]);
        }

        for (int i = 0; i < 3; ++i) {
            builder.connect(writeAddr[i], 0, ic, QString("Write_Addr[%1]").arg(i));
            builder.connect(readAddr[i], 0, ic, QString("Read_Addr1[%1]").arg(i));
        }
        for (int i = 0; i < 8; ++i) {
            builder.connect(writeData[i], 0, ic, QString("Data_In[%1]").arg(i));
        }
        builder.connect(writeEnable, 0, ic, "Write_Enable");
        builder.connect(clock, 0, ic, "Clock");

        for (int i = 0; i < 8; ++i) {
            builder.connect(ic, QString("Read_Data1[%1]").arg(i), readData[i], 0);
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    void setWriteAddr(int addr)
    {
        for (int i = 0; i < 3; ++i) {
            writeAddr[i]->setOn((addr >> i) & 1);
        }
    }

    void setReadAddr(int addr)
    {
        for (int i = 0; i < 3; ++i) {
            readAddr[i]->setOn((addr >> i) & 1);
        }
    }

    void writeReg(int addr, int value)
    {
        setWriteAddr(addr);
        for (int bit = 0; bit < 8; ++bit) {
            writeData[bit]->setOn((value >> bit) & 1);
        }
        writeEnable->setOn(true);
        sim->update();
        clockCycle(sim, clock);
        sim->update();
        writeEnable->setOff();
        sim->update();
    }

    int readReg(int addr)
    {
        setReadAddr(addr);
        sim->update();
        int value = 0;
        for (int bit = 0; bit < 8; ++bit) {
            if (getInputStatus(readData[bit])) {
                value |= (1 << bit);
            }
        }
        return value;
    }
};

static std::unique_ptr<RegFile8x8Fixture> s_level5RegFile8x8;

void TestLevel5RegisterFile8X8::initTestCase()
{
    s_level5RegFile8x8 = std::make_unique<RegFile8x8Fixture>();
    QVERIFY(s_level5RegFile8x8->build());
}

void TestLevel5RegisterFile8X8::cleanupTestCase()
{
    s_level5RegFile8x8.reset();
}

void TestLevel5RegisterFile8X8::cleanup()
{
    if (s_level5RegFile8x8 && s_level5RegFile8x8->sim) {
        s_level5RegFile8x8->sim->restart();
        s_level5RegFile8x8->sim->update();
    }
}

void TestLevel5RegisterFile8X8::testRegisterFile8x8_data()
{
    QTest::addColumn<int>("registerAddr");
    QTest::addColumn<int>("dataToWrite");

    for (int i = 0; i < 8; i++) {
        QTest::newRow(qPrintable(QString("write_reg%1").arg(i)))
            << i << (i * 2);
    }

    QTest::newRow("independence_test") << -1 << 0;
}

void TestLevel5RegisterFile8X8::testRegisterFile8x8()
{
    QFETCH(int, registerAddr);
    QFETCH(int, dataToWrite);

    auto &f = *s_level5RegFile8x8;

    if (registerAddr >= 0 && registerAddr < 8) {
        f.writeReg(registerAddr, dataToWrite);
        QCOMPARE(f.readReg(registerAddr), dataToWrite);
    } else if (registerAddr == -1) {
        int pattern[8] = {0, 2, 4, 6, 8, 10, 12, 14};

        for (int regIdx = 0; regIdx < 8; ++regIdx) {
            f.writeReg(regIdx, pattern[regIdx]);
        }

        for (int regIdx = 0; regIdx < 8; ++regIdx) {
            QCOMPARE(f.readReg(regIdx), pattern[regIdx]);
        }
    }
}

void TestLevel5RegisterFile8X8::testBoundary8x8_data()
{
    QTest::addColumn<int>("boundaryValue");
    QTest::addColumn<QString>("testDescription");

    QTest::newRow("8x8_boundary_zero") << 0x00 << "8x8: Write boundary value 0x00 to all registers";
    QTest::newRow("8x8_boundary_ones") << 0xFF << "8x8: Write boundary value 0xFF to all registers";
    QTest::newRow("8x8_boundary_signbit") << 0x80 << "8x8: Write boundary value 0x80 (sign bit) to all registers";
}

void TestLevel5RegisterFile8X8::testBoundary8x8()
{
    QFETCH(int, boundaryValue);

    auto &f = *s_level5RegFile8x8;

    for (int regIdx = 0; regIdx < 8; ++regIdx) {
        f.writeReg(regIdx, boundaryValue);
    }

    for (int regIdx = 0; regIdx < 8; ++regIdx) {
        QCOMPARE(f.readReg(regIdx), boundaryValue);
    }
}

