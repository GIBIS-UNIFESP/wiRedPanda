// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel6Register8bit.h"

#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::setMultiBitInput;
using CPUTestUtils::loadBuildingBlockIC;

struct Register8bitFixture {
    std::unique_ptr<WorkSpace> workspace;
    IC *ic = nullptr;
    InputSwitch *data[8] = {};
    InputSwitch *clock = nullptr;
    InputSwitch *writeEnable = nullptr;
    InputSwitch *reset = nullptr;
    Led *q[8] = {};
    Simulation *sim = nullptr;

    bool build()
    {
        workspace = std::make_unique<WorkSpace>();
        CircuitBuilder builder(workspace->scene());

        ic = loadBuildingBlockIC("level6_register_8bit.panda");
        builder.add(ic);

        for (int i = 0; i < 8; i++) {
            data[i] = new InputSwitch();
            builder.add(data[i]);
            data[i]->setLabel(QString("Data[%1]").arg(i));
        }

        clock = new InputSwitch();
        builder.add(clock);
        clock->setLabel("Clock");

        writeEnable = new InputSwitch();
        builder.add(writeEnable);
        writeEnable->setLabel("WriteEnable");

        reset = new InputSwitch();
        builder.add(reset);
        reset->setLabel("Reset");

        for (int i = 0; i < 8; i++) {
            q[i] = new Led();
            builder.add(q[i]);
            q[i]->setLabel(QString("Q[%1]").arg(i));
        }

        for (int i = 0; i < 8; i++) {
            builder.connect(data[i], 0, ic, QString("Data[%1]").arg(i));
        }
        builder.connect(clock, 0, ic, "Clock");
        builder.connect(writeEnable, 0, ic, "WriteEnable");
        builder.connect(reset, 0, ic, "Reset");

        for (int i = 0; i < 8; i++) {
            builder.connect(ic, QString("Q[%1]").arg(i), q[i], 0);
        }

        sim = builder.initSimulation();
        sim->update();
        return true;
    }
};

static std::unique_ptr<Register8bitFixture> s_level6Reg8bit;

void TestLevel6Register8Bit::initTestCase()
{
    s_level6Reg8bit = std::make_unique<Register8bitFixture>();
    QVERIFY(s_level6Reg8bit->build());
}

void TestLevel6Register8Bit::cleanupTestCase()
{
    s_level6Reg8bit.reset();
}

void TestLevel6Register8Bit::cleanup()
{
    if (s_level6Reg8bit && s_level6Reg8bit->sim) {
        s_level6Reg8bit->sim->restart();
        s_level6Reg8bit->sim->update();
    }
}

// ============================================================
// Test Cases
// ============================================================

void TestLevel6Register8Bit::test8BitRegister() {
    auto &f = *s_level6Reg8bit;

    // Allow circuit to settle
    f.sim->update();

    // Test 1: Reset to 0x00
    f.reset->setOn(true);
    f.sim->update();

    // Test 2: Write 0xAA on clock edge
    f.reset->setOn(false);

    QVector<InputSwitch *> data_vec1(f.data, f.data + 8);
    setMultiBitInput(data_vec1, CPUTestUtils::PATTERN_ALTERNATING_1);

    f.writeEnable->setOn(true);
    f.sim->update();

    clockCycle(f.sim, f.clock);

    int write_val = CPUTestUtils::get8BitValue(f.q);
    QCOMPARE(write_val, CPUTestUtils::PATTERN_ALTERNATING_1);

    // Test 3: Hold value when WriteEnable=0
    QVector<InputSwitch *> data_vec2(f.data, f.data + 8);
    setMultiBitInput(data_vec2, CPUTestUtils::PATTERN_ALTERNATING_2);

    f.writeEnable->setOn(false);
    f.sim->update();

    for (int cycle = 0; cycle < 5; cycle++) {
        f.sim->update();
    }

    clockCycle(f.sim, f.clock);

    QCOMPARE(CPUTestUtils::get8BitValue(f.q), CPUTestUtils::PATTERN_ALTERNATING_1);

    // Test 4: Re-enable writing
    f.writeEnable->setOn(true);
    f.sim->update();

    QVector<InputSwitch *> data_vec3(f.data, f.data + 8);
    setMultiBitInput(data_vec3, CPUTestUtils::PATTERN_ALTERNATING_2);
    f.sim->update();

    if (f.clock->isOn()) {
        f.clock->setOn(false);
        f.sim->update();
    }
    clockCycle(f.sim, f.clock);

    QCOMPARE(CPUTestUtils::get8BitValue(f.q), CPUTestUtils::PATTERN_ALTERNATING_2);

    // Test 5: Write 0xFF
    QVector<InputSwitch *> data_vec4(f.data, f.data + 8);
    setMultiBitInput(data_vec4, CPUTestUtils::PATTERN_ALL_ONES);
    f.writeEnable->setOn(true);
    f.sim->update();

    if (f.clock->isOn()) {
        f.clock->setOn(false);
        f.sim->update();
    }
    clockCycle(f.sim, f.clock);
    int q_after_ff = CPUTestUtils::get8BitValue(f.q);
    QCOMPARE(q_after_ff, CPUTestUtils::PATTERN_ALL_ONES);

    // Test 6: Hold after known good state
    QVector<InputSwitch *> data_vec5(f.data, f.data + 8);
    setMultiBitInput(data_vec5, CPUTestUtils::PATTERN_ALL_ZEROS);

    f.writeEnable->setOn(false);
    f.sim->update();
    int q_after_hold_settle = CPUTestUtils::get8BitValue(f.q);

    QCOMPARE(q_after_hold_settle, q_after_ff);

    if (f.clock->isOn()) {
        f.clock->setOn(false);
        f.sim->update();
    }
    clockCycle(f.sim, f.clock);

    // Test individual Q output mapping
    f.reset->setOn(true);
    f.sim->update();
    f.reset->setOn(false);
    QVector<InputSwitch *> test_data(f.data, f.data + 8);
    setMultiBitInput(test_data, 0x01);
    f.writeEnable->setOn(true);
    f.sim->update();

    if (f.clock->isOn()) {
        f.clock->setOn(false);
        f.sim->update();
    }
    clockCycle(f.sim, f.clock);
    int q_after_0x01 = CPUTestUtils::get8BitValue(f.q);
    QCOMPARE(q_after_0x01, 0x01);

    setMultiBitInput(test_data, 0x02);
    f.sim->update();
    if (f.clock->isOn()) {
        f.clock->setOn(false);
        f.sim->update();
    }
    clockCycle(f.sim, f.clock);
    int q_after_0x02 = CPUTestUtils::get8BitValue(f.q);
    QCOMPARE(q_after_0x02, 0x02);

    setMultiBitInput(test_data, 0x80);
    f.sim->update();
    if (f.clock->isOn()) {
        f.clock->setOn(false);
        f.sim->update();
    }
    clockCycle(f.sim, f.clock);
    int q_after_0x80 = CPUTestUtils::get8BitValue(f.q);
    QCOMPARE(q_after_0x80, 0x80);

    // Mid-range test values
    f.reset->setOn(true);
    f.sim->update();
    f.reset->setOn(false);
    f.sim->update();

    struct MidRangeTestCase {
        int value;
        const char* description;
    };

    MidRangeTestCase midRangeValues[] = {
        {CPUTestUtils::PATTERN_ALTERNATING_2, "alternating bits (01010101)"},
        {0x33, "grouped bits (00110011)"},
        {0x7F, "boundary (01111111)"},
        {0x15, "sparse bits (00010101)"},
        {0x40, "single mid-bit (01000000)"},
        {CPUTestUtils::PATTERN_UPPER_NIBBLE, "upper nibble (11110000)"},
        {CPUTestUtils::PATTERN_LOWER_NIBBLE, "lower nibble (00001111)"},
    };

    for (size_t i = 0; i < sizeof(midRangeValues) / sizeof(midRangeValues[0]); i++) {
        int testValue = midRangeValues[i].value;

        QVector<InputSwitch *> phase2_data(f.data, f.data + 8);
        setMultiBitInput(phase2_data, testValue);
        f.writeEnable->setOn(true);
        f.sim->update();

        if (f.clock->isOn()) {
            f.clock->setOn(false);
            f.sim->update();
        }

        clockCycle(f.sim, f.clock);
        int q_value = CPUTestUtils::get8BitValue(f.q);

        QCOMPARE(q_value, testValue);

        f.writeEnable->setOn(false);
        f.sim->update();

        if (f.clock->isOn()) {
            f.clock->setOn(false);
            f.sim->update();
        }

        clockCycle(f.sim, f.clock);
        int q_hold_value = CPUTestUtils::get8BitValue(f.q);

        QCOMPARE(q_hold_value, testValue);
    }
}

void TestLevel6Register8Bit::test8BitRegisterSequential() {
    auto &f = *s_level6Reg8bit;

    f.sim->update();

    // Initialize: reset register
    f.reset->setOn(true);
    f.sim->update();
    f.reset->setOn(false);
    f.sim->update();

    f.writeEnable->setOn(true);
    int pass_count = 0;
    int total_operations = 0;

    struct SequentialTestCase {
        int value;
        const char* description;
    };

    SequentialTestCase sequentialValues[] = {
        {0x01, "0x01 - Power of 2: bit 0"},
        {0x02, "0x02 - Power of 2: bit 1"},
        {0x04, "0x04 - Power of 2: bit 2"},
        {0x08, "0x08 - Power of 2: bit 3"},
        {0x10, "0x10 - Power of 2: bit 4"},
        {0x20, "0x20 - Power of 2: bit 5"},
        {0x40, "0x40 - Power of 2: bit 6"},
        {0x80, "0x80 - Power of 2: bit 7"},
        {CPUTestUtils::PATTERN_ALTERNATING_1, "CPUTestUtils::PATTERN_ALTERNATING_1 - Pattern: 10101010"},
        {CPUTestUtils::PATTERN_ALTERNATING_2, "CPUTestUtils::PATTERN_ALTERNATING_2 - Pattern: 01010101"},
        {0x7F, "0x7F - Boundary: 01111111"},
        {0xFE, "0xFE - Boundary: 11111110"},
        {CPUTestUtils::PATTERN_ALL_ONES, "CPUTestUtils::PATTERN_ALL_ONES - All ones: 11111111"},
        {CPUTestUtils::PATTERN_ALL_ZEROS, "CPUTestUtils::PATTERN_ALL_ZEROS - All zeros: 00000000"},
        {0x33, "0x33 - Pattern: 00110011"},
        {0xCC, "0xCC - Pattern: 11001100"},
        {CPUTestUtils::PATTERN_LOWER_NIBBLE, "CPUTestUtils::PATTERN_LOWER_NIBBLE - Nibble: 00001111"},
        {CPUTestUtils::PATTERN_UPPER_NIBBLE, "CPUTestUtils::PATTERN_UPPER_NIBBLE - Nibble: 11110000"},
        {0x15, "0x15 - Sparse: 00010101"},
        {0x2A, "0x2A - Sparse: 00101010"},
    };

    total_operations = sizeof(sequentialValues) / sizeof(sequentialValues[0]);

    for (int i = 0; i < total_operations; i++) {
        int testValue = sequentialValues[i].value;

        QVector<InputSwitch *> seq_data(f.data, f.data + 8);
        setMultiBitInput(seq_data, testValue);
        f.sim->update();

        if (f.clock->isOn()) {
            f.clock->setOn(false);
            f.sim->update();
        }

        clockCycle(f.sim, f.clock);
        int q_value = CPUTestUtils::get8BitValue(f.q);

        if (q_value == testValue) {
            pass_count++;
        } else {
            QCOMPARE(q_value, testValue);
            return;
        }
    }

    QCOMPARE(pass_count, total_operations);
}

void TestLevel6Register8Bit::test8BitRegisterEdgeCases() {
    auto &f = *s_level6Reg8bit;

    f.sim->update();

    // Initialize: reset register
    f.reset->setOn(true);
    f.sim->update();
    f.reset->setOn(false);
    f.sim->update();

    f.writeEnable->setOn(true);
    int pass_count = 0;
    int total_scenarios = 0;

    // SCENARIO 1: Reset During Normal Operation
    QVector<InputSwitch *> data_vec(f.data, f.data + 8);
    setMultiBitInput(data_vec, CPUTestUtils::PATTERN_ALTERNATING_1);
    f.sim->update();

    if (f.clock->isOn()) {
        f.clock->setOn(false);
        f.sim->update();
    }

    clockCycle(f.sim, f.clock);
    int q_value = CPUTestUtils::get8BitValue(f.q);

    QCOMPARE(q_value, CPUTestUtils::PATTERN_ALTERNATING_1);
    pass_count++;
    total_scenarios++;

    f.reset->setOn(true);
    f.sim->update();

    q_value = CPUTestUtils::get8BitValue(f.q);
    QCOMPARE(q_value, CPUTestUtils::PATTERN_ALL_ZEROS);
    pass_count++;
    total_scenarios++;

    // SCENARIO 2: Reset Recovery and New Write
    f.reset->setOn(false);
    f.sim->update();

    q_value = CPUTestUtils::get8BitValue(f.q);
    QCOMPARE(q_value, CPUTestUtils::PATTERN_ALL_ZEROS);
    pass_count++;
    total_scenarios++;

    setMultiBitInput(data_vec, CPUTestUtils::PATTERN_ALTERNATING_2);
    f.sim->update();

    if (f.clock->isOn()) {
        f.clock->setOn(false);
        f.sim->update();
    }

    clockCycle(f.sim, f.clock);
    q_value = CPUTestUtils::get8BitValue(f.q);

    QCOMPARE(q_value, CPUTestUtils::PATTERN_ALTERNATING_2);
    pass_count++;
    total_scenarios++;

    // SCENARIO 3: Write Blocked By Reset
    f.reset->setOn(true);
    f.sim->update();

    q_value = CPUTestUtils::get8BitValue(f.q);
    QCOMPARE(q_value, CPUTestUtils::PATTERN_ALL_ZEROS);
    pass_count++;
    total_scenarios++;

    f.writeEnable->setOn(true);
    setMultiBitInput(data_vec, 0xCC);
    f.sim->update();

    if (f.clock->isOn()) {
        f.clock->setOn(false);
        f.sim->update();
    }

    clockCycle(f.sim, f.clock);
    q_value = CPUTestUtils::get8BitValue(f.q);

    QCOMPARE(q_value, CPUTestUtils::PATTERN_ALL_ZEROS);
    pass_count++;
    total_scenarios++;

    f.reset->setOn(false);
    f.sim->update();

    if (f.clock->isOn()) {
        f.clock->setOn(false);
        f.sim->update();
    }

    clockCycle(f.sim, f.clock);
    q_value = CPUTestUtils::get8BitValue(f.q);

    QCOMPARE(q_value, 0xCC);
    pass_count++;
    total_scenarios++;

    // SCENARIO 4: Hold Operation (WriteEnable=false)
    setMultiBitInput(data_vec, 0x77);
    f.writeEnable->setOn(true);
    f.sim->update();

    if (f.clock->isOn()) {
        f.clock->setOn(false);
        f.sim->update();
    }

    clockCycle(f.sim, f.clock);
    q_value = CPUTestUtils::get8BitValue(f.q);

    QCOMPARE(q_value, 0x77);
    pass_count++;
    total_scenarios++;

    f.writeEnable->setOn(false);
    f.sim->update();

    if (f.clock->isOn()) {
        f.clock->setOn(false);
        f.sim->update();
    }

    clockCycle(f.sim, f.clock);
    q_value = CPUTestUtils::get8BitValue(f.q);

    QCOMPARE(q_value, 0x77);
    pass_count++;
    total_scenarios++;

    QCOMPARE(pass_count, total_scenarios);
}

void TestLevel6Register8Bit::test8BitRegisterStructure() {
    auto &f = *s_level6Reg8bit;

    QVERIFY(f.ic != nullptr);

    QCOMPARE(f.ic->inputSize(), 11);
    QCOMPARE(f.ic->outputSize(), 8);
}

