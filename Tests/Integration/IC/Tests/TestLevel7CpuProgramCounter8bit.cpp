// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel7CpuProgramCounter8bit.h"

#include <QFile>
#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
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

// Inc is the program counter's primary function but the test above only
// exercises load/load/reset. The level-7 wrapper feeds Inc into the level-6 PC's
// write-enable = OR(Load, Inc); verify successive clocked increments here.
void TestLevel7CPUProgramCounter8Bit::testIncrement() {
    auto &f = *s_level7CpuPC8bit;

    // Seed a known value via load
    setMultiBitInput(f.dataInputs, 0x10);
    f.load->setOn(true);
    f.inc->setOn(false);
    f.reset->setOn(false);
    f.sim->update();
    clockCycle(f.sim, f.clock);
    f.sim->update();
    QCOMPARE(f.readAddress(), 0x10);

    // Drop load, raise inc: each clock advances the count by one
    f.clock->setOn(false);
    f.load->setOn(false);
    f.inc->setOn(true);
    f.sim->update();
    clockCycle(f.sim, f.clock);
    f.sim->update();
    QCOMPARE(f.readAddress(), 0x11);

    f.clock->setOn(false);
    f.sim->update();
    clockCycle(f.sim, f.clock);
    f.sim->update();
    QCOMPARE(f.readAddress(), 0x12);
}

// With Load, Inc and Reset all low the level-6 PC's write-enable = OR(Load, Inc)
// is 0, so the count must survive clock edges unchanged. The wrapper only passed
// these signals through; hold was never asserted at this level.
void TestLevel7CPUProgramCounter8Bit::testHold() {
    auto &f = *s_level7CpuPC8bit;

    // Seed a known value via load
    setMultiBitInput(f.dataInputs, 0x55);
    f.load->setOn(true);
    f.inc->setOn(false);
    f.reset->setOn(false);
    f.sim->update();
    clockCycle(f.sim, f.clock);
    f.sim->update();
    QCOMPARE(f.readAddress(), 0x55);

    // Drop load: with no control asserted the value holds across clock edges,
    // even while the (now ignored) Data bus changes.
    f.load->setOn(false);
    for (int i = 0; i < 3; ++i) {
        f.clock->setOn(false);
        setMultiBitInput(f.dataInputs, 0xAA);
        f.sim->update();
        clockCycle(f.sim, f.clock);
        f.sim->update();
        QCOMPARE(f.readAddress(), 0x55);
    }
}

void TestLevel7CPUProgramCounter8Bit::testProgramCounter8BitStructure() {
    auto &f = *s_level7CpuPC8bit;

    QVERIFY(f.ic != nullptr);

    // Data[8] + Clock + Load + Inc + Reset (F26: the dangling WriteEnable
    // input was removed — the level-6 PC derives write enable internally)
    QCOMPARE(f.ic->inputSize(), 12);
    QCOMPARE(f.ic->outputSize(), 8);
}
