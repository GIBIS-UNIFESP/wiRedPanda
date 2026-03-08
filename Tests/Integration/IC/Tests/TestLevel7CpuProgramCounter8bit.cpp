// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel7CpuProgramCounter8bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::readMultiBitOutput;
using TestUtils::setMultiBitInput;
using CPUTestUtils::loadBuildingBlockIC;

struct CpuPC8bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    QVector<InputSwitch *> dataInputs;
    InputSwitch *clock = nullptr;
    InputSwitch *load = nullptr;
    InputSwitch *inc = nullptr;
    InputSwitch *reset = nullptr;
    InputSwitch *writeEnable = nullptr;
    QVector<Led *> addressOutputs;
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level7_cpu_program_counter_8bit.panda");
        builder.add(ic);

        for (int i = 0; i < 8; i++) {
            auto *data = new InputSwitch();
            builder.add(data);
            data->setLabel(QString("Data[%1]").arg(i));
            dataInputs.append(data);
        }

        clock = new InputSwitch();
        builder.add(clock);
        clock->setLabel("Clock");

        load = new InputSwitch();
        builder.add(load);
        load->setLabel("Load");

        inc = new InputSwitch();
        builder.add(inc);
        inc->setLabel("Inc");

        reset = new InputSwitch();
        builder.add(reset);
        reset->setLabel("Reset");

        writeEnable = new InputSwitch();
        builder.add(writeEnable);
        writeEnable->setLabel("WriteEnable");

        for (int i = 0; i < 8; i++) {
            auto *addr = new Led();
            builder.add(addr);
            addr->setLabel(QString("Address[%1]").arg(i));
            addressOutputs.append(addr);
        }

        for (int i = 0; i < 8; i++) {
            builder.connect(dataInputs[i], 0, ic, QString("Data[%1]").arg(i));
        }
        builder.connect(clock, 0, ic, "Clock");
        builder.connect(load, 0, ic, "Load");
        builder.connect(inc, 0, ic, "Inc");
        builder.connect(reset, 0, ic, "Reset");
        builder.connect(writeEnable, 0, ic, "WriteEnable");

        for (int i = 0; i < 8; i++) {
            builder.connect(ic, QString("Address[%1]").arg(i), addressOutputs[i], 0);
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }

    int readAddress()
    {
        return readMultiBitOutput(QVector<GraphicElement *>(addressOutputs.begin(), addressOutputs.end()), 0);
    }
};

static std::unique_ptr<CpuPC8bitFixture> s_level7CpuPC8bit;

void TestLevel7CPUProgramCounter8Bit::initTestCase()
{
    s_level7CpuPC8bit = std::make_unique<CpuPC8bitFixture>();
    QVERIFY(s_level7CpuPC8bit->build());
}

void TestLevel7CPUProgramCounter8Bit::cleanupTestCase()
{
    s_level7CpuPC8bit.reset();
}

void TestLevel7CPUProgramCounter8Bit::cleanup()
{
    if (s_level7CpuPC8bit && s_level7CpuPC8bit->sim) {
        s_level7CpuPC8bit->sim->restart();
        s_level7CpuPC8bit->sim->update();
    }
}

void TestLevel7CPUProgramCounter8Bit::testProgramCounter8Bit() {
    auto &f = *s_level7CpuPC8bit;

    // Test 1: Load address 0x42
    setMultiBitInput(f.dataInputs, 0x42);
    f.load->setOn(true);
    f.writeEnable->setOn(true);
    f.reset->setOn(false);

    f.sim->update();
    clockCycle(f.sim, f.clock);
    f.sim->update();

    QCOMPARE(f.readAddress(), 0x42);

    // Test 2: Load another address 0x20
    f.clock->setOn(false);
    f.sim->update();

    setMultiBitInput(f.dataInputs, 0x20);
    f.sim->update();
    clockCycle(f.sim, f.clock);
    f.sim->update();

    QCOMPARE(f.readAddress(), 0x20);

    // Test 3: Reset to 0x00
    f.clock->setOn(false);
    f.reset->setOn(true);
    f.sim->update();
    QCOMPARE(f.readAddress(), CPUTestUtils::PATTERN_ALL_ZEROS);
}

void TestLevel7CPUProgramCounter8Bit::testProgramCounter8BitStructure() {
    auto &f = *s_level7CpuPC8bit;

    QVERIFY(f.ic != nullptr);

    QCOMPARE(f.ic->inputSize(), 13);
    QCOMPARE(f.ic->outputSize(), 8);
}

