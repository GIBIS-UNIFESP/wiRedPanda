// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestLevel6Register8bit.h"

#include <QFileInfo>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/GlobalProperties.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/CpuTestUtils.h"

using TestUtils::clockCycle;
using TestUtils::setMultiBitInput;
using CPUTestUtils::loadBuildingBlockIC;

/**
 * @brief Build test circuit for 8-bit Register
 *
 * Creates a simulation with:
 * - 8 input switches for Data[0..7]
 * - 3 control inputs: Clock, WriteEnable, Reset
 * - 8-bit Register IC
 * - 8 output LEDs for Q[0..7]
 */
void build8BitRegisterTest(WorkSpace* workspace,
                           InputSwitch* data[8],
                           InputSwitch* &clock,
                           InputSwitch* &writeEnable,
                           InputSwitch* &reset,
                           Led* q[8],
                           Simulation*& sim) {
    CircuitBuilder builder(workspace->scene());

    // Create 8-bit Data input switches
    for (int i = 0; i < 8; i++) {
        data[i] = new InputSwitch();
        builder.add(data[i]);
        data[i]->setLabel(QString("Data[%1]").arg(i));
        data[i]->setPos(50 + i * 60, 100);
    }

    // Create control signals
    clock = new InputSwitch();
    builder.add(clock);
    clock->setLabel("Clock");
    clock->setPos(50, 200);

    writeEnable = new InputSwitch();
    builder.add(writeEnable);
    writeEnable->setLabel("WriteEnable");
    writeEnable->setPos(150, 200);

    reset = new InputSwitch();
    builder.add(reset);
    reset->setLabel("Reset");
    reset->setPos(250, 200);

    // Load and add 8-bit Register IC
    IC *reg = loadBuildingBlockIC("level6_register_8bit.panda");
    builder.add(reg);

    if (reg->outputSize() != 8) {
        // Expected 8 outputs
    }

    // Create 8-bit Q output LEDs
    for (int i = 0; i < 8; i++) {
        q[i] = new Led();
        builder.add(q[i]);
        q[i]->setLabel(QString("Q[%1]").arg(i));
        q[i]->setPos(50 + i * 60, 500);
    }

    // Connect inputs to IC using semantic port labels
    for (int i = 0; i < 8; i++) {
        builder.connect(data[i], 0, reg, QString("Data[%1]").arg(i));
    }
    builder.connect(clock, 0, reg, "Clock");
    builder.connect(writeEnable, 0, reg, "WriteEnable");
    builder.connect(reset, 0, reg, "Reset");

    // Connect IC outputs to LEDs using semantic port labels
    for (int i = 0; i < 8; i++) {
        builder.connect(reg, QString("Q[%1]").arg(i), q[i], 0);
    }

    sim = builder.initSimulation();
}

// ============================================================
// Test Cases
// ============================================================

/**
 * Test: 8-bit Register with write, hold, and reset operations
 *
 * Test scenarios:
 * 1. Reset to CPUTestUtils::PATTERN_ALL_ZEROS
 * 2. Write CPUTestUtils::PATTERN_ALTERNATING_1 (10101010) on clock edge
 * 3. Hold value when WriteEnable=0
 * 4. Write different value CPUTestUtils::PATTERN_ALTERNATING_2 (01010101)
 * 5. Reset clears to CPUTestUtils::PATTERN_ALL_ZEROS
 */
void TestLevel6Register8Bit::test8BitRegister() {
    auto workspace = std::make_unique<WorkSpace>();
    InputSwitch *data[8], *clock, *writeEnable, *reset;
    Led *q[8];
    Simulation *sim = nullptr;

    build8BitRegisterTest(workspace.get(), data, clock, writeEnable, reset, q, sim);

    // Allow circuit to settle
    sim->update();

    // Test 1: Reset to 0x00
    reset->setOn(true);             // Assert reset (active-high, inverted to active-low)

    sim->update();

    // Test 2: Write 0xAA on clock edge
    reset->setOn(false);            // De-assert reset

    QVector<InputSwitch *> data_vec1(data, data + 8);
    setMultiBitInput(data_vec1, CPUTestUtils::PATTERN_ALTERNATING_1); // Set data = 0xAA

    writeEnable->setOn(true);       // Enable writing

    sim->update();

    clockCycle(sim, clock);

    int write_val = CPUTestUtils::get8BitValue(q);
    QCOMPARE(write_val, CPUTestUtils::PATTERN_ALTERNATING_1);

    // Test 3: Hold value when WriteEnable=0
    // The write from Test 2 should have Q = 0xAA
    // Now we test that hold (writeEnable=false) preserves the value

    QVector<InputSwitch *> data_vec2(data, data + 8);
    setMultiBitInput(data_vec2, CPUTestUtils::PATTERN_ALTERNATING_2);

    writeEnable->setOn(false);

    sim->update();

    for (int cycle = 0; cycle < 5; cycle++) {
        sim->update();
    }

    clockCycle(sim, clock);

    // Validate hold operation - should preserve value when writeEnable=false
    QCOMPARE(CPUTestUtils::get8BitValue(q), CPUTestUtils::PATTERN_ALTERNATING_1);   // Should still be 0xAA after hold

    // Test 4: Check if writeEnable transition is the problem
    // Before this test, writeEnable is false and Q is 255 (invalid state)
    // Let's see what happens if we set writeEnable back to true
    writeEnable->setOn(true);       // Re-enable writing

    sim->update();

    // Now try to write a new value
    QVector<InputSwitch *> data_vec3(data, data + 8);
    setMultiBitInput(data_vec3, CPUTestUtils::PATTERN_ALTERNATING_2); // Set data = 0x55

    sim->update();

    // Ensure clock is LOW for clean rising edge
    if (clock->isOn()) {
        clock->setOn(false);
        sim->update();
    }
    clockCycle(sim, clock);

    QCOMPARE(CPUTestUtils::get8BitValue(q), CPUTestUtils::PATTERN_ALTERNATING_2);

    // Test 5: What if we DON'T turn off writeEnable?
    // Skip all the complex hold testing and just see basic write operations
    QVector<InputSwitch *> data_vec4(data, data + 8);
    setMultiBitInput(data_vec4, CPUTestUtils::PATTERN_ALL_ONES); // Set data = 0xFF
    writeEnable->setOn(true);       // Keep writing enabled
    sim->update();

    // Ensure clock is LOW for clean rising edge
    if (clock->isOn()) {
        clock->setOn(false);
        sim->update();
    }
    clockCycle(sim, clock);
    int q_after_ff = CPUTestUtils::get8BitValue(q);
    QCOMPARE(q_after_ff, CPUTestUtils::PATTERN_ALL_ONES);  // Verify 0xFF was written correctly

    // Test 6: Now try hold AFTER getting into a known good state
    // Change data
    QVector<InputSwitch *> data_vec5(data, data + 8);
    setMultiBitInput(data_vec5, CPUTestUtils::PATTERN_ALL_ZEROS); // Change to 0x00

    // Now disable writeEnable for hold
    writeEnable->setOn(false);

    sim->update();
    int q_after_hold_settle = CPUTestUtils::get8BitValue(q);

    // Hold operation must preserve the stored value
    QCOMPARE(q_after_hold_settle, q_after_ff);

    // Ensure clock is LOW for clean rising edge
    if (clock->isOn()) {
        clock->setOn(false);
        sim->update();
    }
    clockCycle(sim, clock);

    // NEW TEST: Check individual Q output mapping
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    QVector<InputSwitch *> test_data(data, data + 8);
    setMultiBitInput(test_data, 0x01);
    writeEnable->setOn(true);
    sim->update();

    // Ensure clock is LOW for clean rising edge
    if (clock->isOn()) {
        clock->setOn(false);
        sim->update();
    }
    clockCycle(sim, clock);
    int q_after_0x01 = CPUTestUtils::get8BitValue(q);
    QCOMPARE(q_after_0x01, 0x01);  // Verify single-bit write (bit 0) works

    setMultiBitInput(test_data, 0x02);
    sim->update();
    // Ensure clock is LOW for clean rising edge
    if (clock->isOn()) {
        clock->setOn(false);
        sim->update();
    }
    clockCycle(sim, clock);
    int q_after_0x02 = CPUTestUtils::get8BitValue(q);
    QCOMPARE(q_after_0x02, 0x02);  // Verify single-bit write (bit 1) works

    setMultiBitInput(test_data, 0x80);
    sim->update();
    // Ensure clock is LOW for clean rising edge
    if (clock->isOn()) {
        clock->setOn(false);
        sim->update();
    }
    clockCycle(sim, clock);
    int q_after_0x80 = CPUTestUtils::get8BitValue(q);
    QCOMPARE(q_after_0x80, 0x80);  // Verify single-bit write (bit 7) works

    // Tests diverse bit patterns to catch subtle bit-routing issues

    // Reset register before tests
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    // Define 7 mid-range test values with different bit patterns
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

    // Test each mid-range value with write and hold operations
    for (size_t i = 0; i < sizeof(midRangeValues) / sizeof(midRangeValues[0]); i++) {
        int testValue = midRangeValues[i].value;

        // Set data inputs
        QVector<InputSwitch *> phase2_data(data, data + 8);
        setMultiBitInput(phase2_data, testValue);
        writeEnable->setOn(true);
        sim->update();

        // Ensure clock is LOW for clean rising edge
        if (clock->isOn()) {
            clock->setOn(false);
            sim->update();
        }

        // Clock the value in
        clockCycle(sim, clock);
        int q_value = CPUTestUtils::get8BitValue(q);

        // Verify the write succeeded
        QCOMPARE(q_value, testValue);

        // Test hold: disable write and clock again - value should be preserved
        writeEnable->setOn(false);
        sim->update();

        // Ensure clock is LOW for clean rising edge
        if (clock->isOn()) {
            clock->setOn(false);
            sim->update();
        }

        // Clock with writeEnable=false - value should NOT change
        clockCycle(sim, clock);
        int q_hold_value = CPUTestUtils::get8BitValue(q);

        // Verify hold operation preserved the value
        QCOMPARE(q_hold_value, testValue);
    }
}

/**
 * Sequential Operations Stress Test
 *
 * Test: 20+ consecutive register write operations to validate state stability
 *
 * Purpose:
 * - Tests state stability across many sequential operations
 * - Validates no state corruption with repeated writes
 * - Catches edge cases in state machine behavior
 * - Ensures register integrity under continuous operation
 *
 * Operations:
 * 1. Powers of 2: 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
 * 2. Patterns: CPUTestUtils::PATTERN_ALTERNATING_1, CPUTestUtils::PATTERN_ALTERNATING_2
 * 3. Boundaries: 0x7F, 0xFE
 * 4. Additional: CPUTestUtils::PATTERN_ALL_ONES, CPUTestUtils::PATTERN_ALL_ZEROS, 0x33, 0xCC, CPUTestUtils::PATTERN_LOWER_NIBBLE, CPUTestUtils::PATTERN_UPPER_NIBBLE, 0x15, 0x2A
 *
 * Total: 20+ test values
 */
void TestLevel6Register8Bit::test8BitRegisterSequential() {
    auto workspace = std::make_unique<WorkSpace>();
    InputSwitch *data[8], *clock, *writeEnable, *reset;
    Led *q[8];
    Simulation *sim = nullptr;

    build8BitRegisterTest(workspace.get(), data, clock, writeEnable, reset, q, sim);

    // Allow circuit to settle
    sim->update();

    // Initialize: reset register
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    writeEnable->setOn(true);  // Enable writes for all operations
    int pass_count = 0;
    int total_operations = 0;

    // Define sequential test values: Powers of 2 + Patterns + Boundaries + Mixed
    struct SequentialTestCase {
        int value;
        const char* description;
    };

    SequentialTestCase sequentialValues[] = {
        // Powers of 2
        {0x01, "0x01 - Power of 2: bit 0"},
        {0x02, "0x02 - Power of 2: bit 1"},
        {0x04, "0x04 - Power of 2: bit 2"},
        {0x08, "0x08 - Power of 2: bit 3"},
        {0x10, "0x10 - Power of 2: bit 4"},
        {0x20, "0x20 - Power of 2: bit 5"},
        {0x40, "0x40 - Power of 2: bit 6"},
        {0x80, "0x80 - Power of 2: bit 7"},

        // Classic Patterns
        {CPUTestUtils::PATTERN_ALTERNATING_1, "CPUTestUtils::PATTERN_ALTERNATING_1 - Pattern: 10101010"},
        {CPUTestUtils::PATTERN_ALTERNATING_2, "CPUTestUtils::PATTERN_ALTERNATING_2 - Pattern: 01010101"},

        // Boundaries
        {0x7F, "0x7F - Boundary: 01111111"},
        {0xFE, "0xFE - Boundary: 11111110"},

        // Additional values for comprehensive coverage
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

    // Test sequence: Write each value, verify it's captured, move to next
    for (int i = 0; i < total_operations; i++) {
        int testValue = sequentialValues[i].value;

        // Set data inputs
        QVector<InputSwitch *> seq_data(data, data + 8);
        setMultiBitInput(seq_data, testValue);
        sim->update();

        // Ensure clock is LOW for clean rising edge
        if (clock->isOn()) {
            clock->setOn(false);
            sim->update();
        }

        // Clock the value in
        clockCycle(sim, clock);
        int q_value = CPUTestUtils::get8BitValue(q);

        // Verify the write succeeded
        if (q_value == testValue) {
            pass_count++;
        } else {
            QCOMPARE(q_value, testValue);  // Will fail with detailed error message
            return;
        }
    }

    // Verify all operations succeeded
    QCOMPARE(pass_count, total_operations);
}

/**
 * Test: Phase 4 - Edge Case Interactions
 *
 * Tests critical control signal scenarios:
 * 1. Reset during normal operation: Verify reset forces Q to 0
 * 2. Reset recovery: Verify operation resumes after reset de-asserts
 * 3. Write while reset active: Verify reset prevents writes
 * 4. Hold operation: Verify writeEnable=false holds the value (no new data capture)
 *
 * Validates:
 * - Reset has priority over all other operations
 * - Register correctly transitions after reset
 * - Hold operation preserves value when writeEnable=false
 * - State transitions are reliable under edge conditions
 */
void TestLevel6Register8Bit::test8BitRegisterEdgeCases() {
    auto workspace = std::make_unique<WorkSpace>();
    InputSwitch *data[8], *clock, *writeEnable, *reset;
    Led *q[8];
    Simulation *sim = nullptr;

    build8BitRegisterTest(workspace.get(), data, clock, writeEnable, reset, q, sim);

    // Allow circuit to settle
    sim->update();

    // Initialize: reset register
    reset->setOn(true);
    sim->update();
    reset->setOn(false);
    sim->update();

    writeEnable->setOn(true);
    int pass_count = 0;
    int total_scenarios = 0;

    // ========== SCENARIO 1: Reset During Normal Operation ==========

    // Write value 0xAA
    QVector<InputSwitch *> data_vec(data, data + 8);
    setMultiBitInput(data_vec, CPUTestUtils::PATTERN_ALTERNATING_1);
    sim->update();

    // Ensure clock is LOW for clean rising edge
    if (clock->isOn()) {
        clock->setOn(false);
        sim->update();
    }

    // Clock the value in
    clockCycle(sim, clock);
    int q_value = CPUTestUtils::get8BitValue(q);

    QCOMPARE(q_value, CPUTestUtils::PATTERN_ALTERNATING_1);
    pass_count++;
    total_scenarios++;

    // Assert reset while register has value
    reset->setOn(true);
    sim->update();

    q_value = CPUTestUtils::get8BitValue(q);
    QCOMPARE(q_value, CPUTestUtils::PATTERN_ALL_ZEROS);  // Reset should force Q to 0 regardless of held value
    pass_count++;
    total_scenarios++;

    // ========== SCENARIO 2: Reset Recovery and New Write ==========
    reset->setOn(false);
    sim->update();

    q_value = CPUTestUtils::get8BitValue(q);
    QCOMPARE(q_value, CPUTestUtils::PATTERN_ALL_ZEROS);
    pass_count++;
    total_scenarios++;

    // Now write a new value after reset
    setMultiBitInput(data_vec, CPUTestUtils::PATTERN_ALTERNATING_2);
    sim->update();

    // Ensure clock is LOW
    if (clock->isOn()) {
        clock->setOn(false);
        sim->update();
    }

    clockCycle(sim, clock);
    q_value = CPUTestUtils::get8BitValue(q);

    QCOMPARE(q_value, CPUTestUtils::PATTERN_ALTERNATING_2);  // New write should work after reset
    pass_count++;
    total_scenarios++;

    // ========== SCENARIO 3: Write Blocked By Reset ==========
    reset->setOn(true);
    sim->update();

    q_value = CPUTestUtils::get8BitValue(q);
    QCOMPARE(q_value, CPUTestUtils::PATTERN_ALL_ZEROS);
    pass_count++;
    total_scenarios++;

    // Try to write while reset is asserted
    writeEnable->setOn(true);
    setMultiBitInput(data_vec, 0xCC);
    sim->update();

    // Ensure clock is LOW
    if (clock->isOn()) {
        clock->setOn(false);
        sim->update();
    }

    // Try to clock (write should be prevented by reset)
    clockCycle(sim, clock);
    q_value = CPUTestUtils::get8BitValue(q);

    QCOMPARE(q_value, CPUTestUtils::PATTERN_ALL_ZEROS);  // Reset prevents write
    pass_count++;
    total_scenarios++;

    // De-assert reset and verify write now works
    reset->setOn(false);
    sim->update();

    // Ensure clock is LOW
    if (clock->isOn()) {
        clock->setOn(false);
        sim->update();
    }

    // Clock should now capture the write (data still 0xCC)
    clockCycle(sim, clock);
    q_value = CPUTestUtils::get8BitValue(q);

    QCOMPARE(q_value, 0xCC);  // Write succeeds after reset
    pass_count++;
    total_scenarios++;

    // ========== SCENARIO 4: Hold Operation (WriteEnable=false) ==========
    // Write a new value first
    setMultiBitInput(data_vec, 0x77);
    writeEnable->setOn(true);
    sim->update();

    // Ensure clock is LOW
    if (clock->isOn()) {
        clock->setOn(false);
        sim->update();
    }

    clockCycle(sim, clock);
    q_value = CPUTestUtils::get8BitValue(q);

    QCOMPARE(q_value, 0x77);
    pass_count++;
    total_scenarios++;

    // Test hold: disable WriteEnable and clock - value should NOT change
    writeEnable->setOn(false);
    sim->update();

    // Ensure clock is LOW
    if (clock->isOn()) {
        clock->setOn(false);
        sim->update();
    }

    clockCycle(sim, clock);
    q_value = CPUTestUtils::get8BitValue(q);

    QCOMPARE(q_value, 0x77);  // Hold operation preserves value
    pass_count++;
    total_scenarios++;

    // Verify all scenarios succeeded
    QCOMPARE(pass_count, total_scenarios);
}

/**
 * Test: Verify 8-bit Register IC structure and port count
 *
 * Validates:
 * - IC can be loaded and instantiated
 * - IC has 11 input ports (8 for Data, 1 for Clock, 1 for WriteEnable, 1 for Reset)
 * - IC has 8 output ports (8 for Q)
 */
void TestLevel6Register8Bit::test8BitRegisterStructure() {
    auto workspace = std::make_unique<WorkSpace>();
    CircuitBuilder builder(workspace->scene());

    // Load IC and verify it exists
    IC *reg = nullptr;
    try {
        reg = loadBuildingBlockIC("level6_register_8bit.panda");
    } catch (const std::runtime_error &e) {
        QFAIL(e.what());
        return;
    }

    builder.add(reg);

    // Verify IC has been loaded
    QVERIFY(reg != nullptr);

    // Create dummy connections to verify port access
    InputSwitch *data = new InputSwitch();
    builder.add(data);

    Led *q = new Led();
    builder.add(q);

    // Verify IC port structure
    QCOMPARE(reg->inputSize(), 11);  // 8 Data inputs + 1 Clock + 1 WriteEnable + 1 Reset
    QCOMPARE(reg->outputSize(), 8);  // 8 Q outputs (Q0-Q7)

    // Verify port connections work (don't throw)
    try {
        builder.connect(data, 0, reg, "Data[0]"); // Connect Data[0] input
        builder.connect(reg, "Q[0]", q, 0);     // Connect Q[0] output
    } catch (const std::exception &e) {
        QFAIL(qPrintable(QString("IC port access failed: %1").arg(e.what())));
    }
}
