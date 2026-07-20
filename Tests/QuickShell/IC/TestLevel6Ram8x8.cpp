// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/IC/TestLevel6Ram8x8.h"

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "Tests/QuickShell/QuickCircuitBuilder.h"
#include "Tests/QuickShell/QuickCpuTestUtils.h"

using TestUtils::setMultiBitInput;
using TestUtils::readMultiBitOutput;
using TestUtils::clockCycle;
using CPUTestUtils::loadBuildingBlockIC;

struct Ram8x8Fixture {
    std::unique_ptr<QuickCircuitBuilder> builder;
    IC *ic = nullptr;
    QVector<InputSwitch *> addressInputs;
    QVector<InputSwitch *> dataInInputs;
    InputSwitch *we = nullptr;
    InputSwitch *clk = nullptr;
    InputSwitch *reset = nullptr;
    QVector<Led *> dataOutOutputs;
    Simulation *sim = nullptr;

    bool build()
    {
        builder = std::make_unique<QuickCircuitBuilder>();

        ic = static_cast<IC *>(builder->addOwnedElement(loadBuildingBlockIC("level6_ram_8x8.panda")));

        for (int i = 0; i < 3; i++) {
            auto *sw = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
            sw->setLabel(QString("Address[%1]").arg(i));
            addressInputs.append(sw);
        }

        for (int i = 0; i < 8; i++) {
            auto *sw = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
            sw->setLabel(QString("DataIn[%1]").arg(i));
            dataInInputs.append(sw);
        }

        we = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
        we->setLabel("WriteEnable");

        clk = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
        clk->setLabel("Clock");

        reset = static_cast<InputSwitch *>(builder->addOwnedElement(new InputSwitch()));
        reset->setLabel("Reset");

        for (int i = 0; i < 8; i++) {
            auto *led = static_cast<Led *>(builder->addOwnedElement(new Led()));
            led->setLabel(QString("DataOut[%1]").arg(i));
            dataOutOutputs.append(led);
        }

        for (int i = 0; i < 3; i++) {
            builder->connect(addressInputs[i], 0, ic, QString("Address[%1]").arg(i));
        }
        for (int i = 0; i < 8; i++) {
            builder->connect(dataInInputs[i], 0, ic, QString("DataIn[%1]").arg(i));
        }
        builder->connect(we, 0, ic, "WriteEnable");
        builder->connect(clk, 0, ic, "Clock");
        builder->connect(reset, 0, ic, "Reset");

        for (int i = 0; i < 8; i++) {
            builder->connect(ic, QString("DataOut[%1]").arg(i), dataOutOutputs[i], 0);
        }

        sim = builder->initSimulation();
        sim->update();
        return true;
    }

    int readDataOut()
    {
        return readMultiBitOutput(QVector<GraphicElement *>(dataOutOutputs.begin(), dataOutOutputs.end()), 0);
    }
};

static std::unique_ptr<Ram8x8Fixture> s_level6Ram8x8;

void TestLevel6RAM8X8::initTestCase()
{
    s_level6Ram8x8 = std::make_unique<Ram8x8Fixture>();
    QVERIFY(s_level6Ram8x8->build());
}

void TestLevel6RAM8X8::cleanupTestCase()
{
    s_level6Ram8x8.reset();
}

void TestLevel6RAM8X8::cleanup()
{
    if (s_level6Ram8x8 && s_level6Ram8x8->sim) {
        s_level6Ram8x8->sim->restart();
        s_level6Ram8x8->sim->update();
    }
}

void TestLevel6RAM8X8::testRAM8x8_data()
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

void TestLevel6RAM8X8::testRAM8x8()
{
    QFETCH(int, address);
    QFETCH(int, writeData);
    QFETCH(int, readAddress);
    QFETCH(int, expectedReadData);

    auto &f = *s_level6Ram8x8;

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

void TestLevel6RAM8X8::testRAMStructure()
{
    auto &f = *s_level6Ram8x8;

    QVERIFY(f.ic != nullptr);

    QCOMPARE(f.ic->inputSize(), 14);
    QCOMPARE(f.ic->outputSize(), 8);
}

// Reset (F54) is fanned out to every ram_8x1 column's own async clear, but no
// test in this file ever asserted it — only level8_memory_stage's Reset test
// exercises it, two IC layers away. Verify it directly: a written word must
// clear when Reset is asserted.
void TestLevel6RAM8X8::testRAMReset()
{
    auto &f = *s_level6Ram8x8;

    setMultiBitInput(f.addressInputs, 0x03);
    setMultiBitInput(f.dataInInputs, 0x5A);
    f.we->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clk);
    f.we->setOn(false);
    f.sim->update();
    QCOMPARE(f.readDataOut(), 0x5A);

    // Assert Reset: the stored word clears asynchronously (no clock needed)
    f.reset->setOn(true);
    f.sim->update();
    QCOMPARE(f.readDataOut(), 0x00);

    // Release Reset: memory stays cleared
    f.reset->setOn(false);
    f.sim->update();
    QCOMPARE(f.readDataOut(), 0x00);
}
