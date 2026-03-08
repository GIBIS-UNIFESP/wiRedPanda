// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel6Ram256x8.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::setMultiBitInput;
using TestUtils::readMultiBitOutput;
using TestUtils::clockCycle;
using CPUTestUtils::loadBuildingBlockIC;

struct Ram256x8Fixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    QVector<InputSwitch *> addressInputs;
    QVector<InputSwitch *> dataInInputs;
    InputSwitch *we = nullptr;
    InputSwitch *clk = nullptr;
    QVector<Led *> dataOutOutputs;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level6_ram_256x8.panda");
        builder.add(ic);

        for (int i = 0; i < 8; i++) {
            auto *sw = new InputSwitch();
            builder.add(sw);
            sw->setLabel(QString("Address[%1]").arg(i));
            addressInputs.append(sw);
        }

        for (int i = 0; i < 8; i++) {
            auto *sw = new InputSwitch();
            builder.add(sw);
            sw->setLabel(QString("DataIn[%1]").arg(i));
            dataInInputs.append(sw);
        }

        we = new InputSwitch();
        builder.add(we);
        we->setLabel("WriteEnable");

        clk = new InputSwitch();
        builder.add(clk);
        clk->setLabel("Clock");

        for (int i = 0; i < 8; i++) {
            auto *led = new Led();
            builder.add(led);
            led->setLabel(QString("DataOut[%1]").arg(i));
            dataOutOutputs.append(led);
        }

        for (int i = 0; i < 8; i++) {
            builder.connect(addressInputs[i], 0, ic, QString("Address[%1]").arg(i));
            builder.connect(dataInInputs[i], 0, ic, QString("DataIn[%1]").arg(i));
        }
        builder.connect(we, 0, ic, "WriteEnable");
        builder.connect(clk, 0, ic, "Clock");

        for (int i = 0; i < 8; i++) {
            builder.connect(ic, QString("DataOut[%1]").arg(i), dataOutOutputs[i], 0);
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    int readDataOut()
    {
        return readMultiBitOutput(QVector<GraphicElement *>(dataOutOutputs.begin(), dataOutOutputs.end()), 0);
    }
};

static std::unique_ptr<Ram256x8Fixture> s_level6Ram256x8;

void TestLevel6RAM256X8::initTestCase()
{
    s_level6Ram256x8 = std::make_unique<Ram256x8Fixture>();
    QVERIFY(s_level6Ram256x8->build());
}

void TestLevel6RAM256X8::cleanupTestCase()
{
    s_level6Ram256x8.reset();
}

void TestLevel6RAM256X8::cleanup()
{
    if (s_level6Ram256x8 && s_level6Ram256x8->sim) {
        s_level6Ram256x8->sim->restart();
        s_level6Ram256x8->sim->update();
    }
}

void TestLevel6RAM256X8::testRAM256x8_data()
{
    QTest::addColumn<int>("address");
    QTest::addColumn<int>("writeData");
    QTest::addColumn<int>("readAddress");
    QTest::addColumn<int>("expectedReadData");

    QTest::newRow("write 0x42 to addr 0, read addr 0")
        << 0 << 0x42 << 0 << 0x42;
    QTest::newRow("write 0xFF to addr 3, read addr 3")
        << 3 << 0xFF << 3 << 0xFF;
    QTest::newRow("write 0xAA to addr 5, read addr 5")
        << 5 << 0xAA << 5 << 0xAA;
    QTest::newRow("write 0x55 to addr 7, read addr 7")
        << 7 << 0x55 << 7 << 0x55;
}

void TestLevel6RAM256X8::testRAM256x8()
{
    QFETCH(int, address);
    QFETCH(int, writeData);
    QFETCH(int, readAddress);
    QFETCH(int, expectedReadData);

    auto &f = *s_level6Ram256x8;

    // Step 1: Write data to address
    setMultiBitInput(f.addressInputs, address);
    setMultiBitInput(f.dataInInputs, writeData);
    f.we->setOn(true);
    f.sim->update();

    f.clk->setOn(false);
    f.sim->update();
    clockCycle(f.sim, f.clk);
    f.sim->update();

    // Step 2: Read from address
    setMultiBitInput(f.addressInputs, readAddress);
    f.we->setOn(false);
    f.sim->update();

    int readData = f.readDataOut();

    QCOMPARE(readData, expectedReadData);
}

void TestLevel6RAM256X8::testRAMStructure()
{
    auto &f = *s_level6Ram256x8;

    QVERIFY(f.ic != nullptr);

    QCOMPARE(f.ic->inputSize(), 18);
    QCOMPARE(f.ic->outputSize(), 8);
}

