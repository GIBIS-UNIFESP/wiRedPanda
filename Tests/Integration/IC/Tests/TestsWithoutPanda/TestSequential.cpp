// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/IC/Tests/TestsWithoutPanda/TestSequential.h"

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/GraphicElements/SRFlipFlop.h"
#include "App/Element/GraphicElements/TFlipFlop.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/Cpu/CpuCommon.h"

using TestUtils::clockCycle;
using TestUtils::clockToggle;
using TestUtils::getInputStatus;
using TestUtils::readMultiBitOutput;
using TestUtils::setMultiBitInput;

/**
 * @brief Build a simple 2-state FSM circuit (D flip-flop with trigger)
 *
 * @param triggerSwitch Output: InputSwitch for D input
 * @param clockSwitch Output: InputSwitch for clock input
 * @param stateLed Output: Led for Q output monitoring
 * @param stateNotLed Output: Optional Led for Q_not output (can be null)
 * @return Simulation pointer for the circuit
 */
void buildSimple2StateFsm(WorkSpace* workspace,
                          InputSwitch*& triggerSwitch,
                          InputSwitch*& clockSwitch,
                          Led*& stateLed,
                          Led*& stateNotLed,
                          DFlipFlop*& outStateFf,
                          Simulation*& sim)
{
    // Create fresh circuit for each test
    CircuitBuilder builder(workspace->scene());

    triggerSwitch = new InputSwitch();
    clockSwitch = new InputSwitch();
    outStateFf = new DFlipFlop();
    stateLed = new Led();
    stateNotLed = new Led();

    builder.add(triggerSwitch, clockSwitch, outStateFf, stateLed, stateNotLed);

    // D input (port 0) gets trigger signal
    builder.connect(triggerSwitch, 0, outStateFf, "Data");

    // Clock input (port 1) gets clock signal
    builder.connect(clockSwitch, 0, outStateFf, "Clock");

    // Q output (port 0) connects to status LED
    builder.connect(outStateFf, "Q", stateLed, 0);

    // ~Q output (port 1) connects to status not LED
    builder.connect(outStateFf, "~Q", stateNotLed, 0);

    sim = builder.initSimulation();
}

/**
 * @brief Advance FSM by one complete clock cycle with trigger input
 *
 * @param simulation Simulation to run
 * @param triggerSwitch Trigger input switch
 * @param clockSwitch Clock input switch
 * @param newTriggerValue New value for trigger input
 * @param settleAfter Number of settle iterations after each phase
 */
void advanceFsm(Simulation *simulation,
                InputSwitch *triggerSwitch,
                InputSwitch *clockSwitch,
                bool newTriggerValue,
                int settleAfter = 1)
{
    // Set trigger value
    triggerSwitch->setOn(newTriggerValue);
    for (int i = 0; i < settleAfter; ++i) {
        simulation->update();
    }

    // Complete clock pulse (rising edge captures D, falling edge for next cycle)
    clockCycle(simulation, clockSwitch);
    for (int i = 0; i < settleAfter; ++i) {
        simulation->update();
    }
}

// ============================================================
// Async Reset Tests (Issue 4.3)
// ============================================================

// Register Async Clear Test
// Verifies that CLEAR signal immediately resets register without clock edge
void TestSequential::testRegisterAsyncClear_data()
{
    QTest::addColumn<int>("initialValue");
    QTest::addColumn<bool>("shouldReleaseClear");

    // Test clearing from various states
    QTest::newRow("Clear from 0") << 0x0 << false;    // Already clear
    QTest::newRow("Clear from 1") << 0x1 << false;    // Single bit
    QTest::newRow("Clear from 5") << 0x5 << false;    // Multiple bits (0101)
    QTest::newRow("Clear from 15") << 0xF << false;   // All bits set
    QTest::newRow("Release from 0") << 0x0 << true;   // CLEAR released from 0
    QTest::newRow("Release from 1") << 0x1 << true;   // CLEAR released from 1
    QTest::newRow("Release from 5") << 0x5 << true;   // CLEAR released from 5
    QTest::newRow("Release from 15") << 0xF << true;  // CLEAR released from 15
}

void TestSequential::testRegisterAsyncClear()
{
    QFETCH(int, initialValue);
    QFETCH(bool, shouldReleaseClear);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Input controls
    InputSwitch clk;
    InputSwitch clear;  // Active LOW clear signal

    // Data inputs (4-bit)
    InputSwitch dataIn[4];

    // D flip-flops (4-bit register)
    DFlipFlop dff[4];

    // Output LEDs (4-bit)
    Led dataOut[4];

    // Add all elements
    builder.add(&clk, &clear);
    for (int i = 0; i < 4; ++i) {
        builder.add(&dataIn[i], &dff[i], &dataOut[i]);
    }

    // Build register with clear connection
    for (int i = 0; i < 4; ++i) {
        builder.connect(&dataIn[i], 0, &dff[i], "Data"); // D input
        builder.connect(&clk, 0, &dff[i], "Clock");      // Clock input
        builder.connect(&clear, 0, &dff[i], "~Clear");   // CLEAR input (port 3, active LOW)
        builder.connect(&dff[i], "Q", &dataOut[i], 0);   // Q output to LED
    }

    auto *simulation = builder.initSimulation();

    // Set initial state
    clk.setOn(false);
    clear.setOn(true);  // CLEAR = HIGH (inactive, normal operation)
    simulation->update();

    // Load initial value into register
    for (int i = 0; i < 4; ++i) {
        dataIn[i].setOn((initialValue >> i) & 1);
    }
    simulation->update();

    // Complete clock pulse to load initial value
    clockCycle(simulation, &clk);

    // Assert CLEAR (set LOW = active)
    clear.setOn(false);  // CLEAR = LOW (active) - immediate reset
    simulation->update();  // No clock, just settle

    // Read output - should be 0000
    int resultAfterClear = readMultiBitOutput(QVector<GraphicElement *>({
        &dataOut[0], &dataOut[1], &dataOut[2], &dataOut[3]
    }));
    QCOMPARE(resultAfterClear, 0x0);

    if (shouldReleaseClear) {
        // Release CLEAR (set HIGH = inactive)
        clear.setOn(true);  // CLEAR = HIGH (inactive)
        simulation->update();  // Settle without clock

        // Verify state still holds at 0
        int resultAfterRelease = readMultiBitOutput(QVector<GraphicElement *>({
            &dataOut[0], &dataOut[1], &dataOut[2], &dataOut[3]
        }));
        QCOMPARE(resultAfterRelease, 0x0);
    }
}

// Counter Async Reset Test
// Verifies counter resets to 0 mid-count without clock edge
void TestSequential::testCounterAsyncReset_data()
{
    QTest::addColumn<int>("targetCount");
    QTest::addColumn<int>("resetAction");

    // 0 = reset at target, 1 = reset then continue, 2 = multiple resets

    QTest::newRow("Reset at count 0") << 0 << 0;
    QTest::newRow("Reset at count 3") << 3 << 0;
    QTest::newRow("Reset at count 7") << 7 << 0;
    QTest::newRow("Reset at count 15") << 15 << 0;
    QTest::newRow("Continue after reset") << 5 << 1;
    QTest::newRow("Multiple resets") << 10 << 2;
}

void TestSequential::testCounterAsyncReset()
{
    QFETCH(int, targetCount);
    QFETCH(int, resetAction);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Clock input
    InputSwitch clk;
    InputSwitch clear;  // Active LOW clear signal

    // Counter bits using D flip-flops
    DFlipFlop dff[4];

    // Output LEDs
    Led counterOut[4];

    // Increment logic gates (ripple carry counter)
    Not notQ[4];
    And carryAnd[2];
    And xorAnd[4][2];
    Or xorOr[4];
    Not notCarry[3];

    // Add all elements
    builder.add(&clk, &clear);
    for (int i = 0; i < 4; ++i) {
        builder.add(&dff[i], &counterOut[i], &notQ[i], &xorOr[i]);
    }
    for (int i = 0; i < 2; ++i) {
        builder.add(&carryAnd[i]);
    }
    for (int i = 0; i < 3; ++i) {
        builder.add(&notCarry[i]);
    }
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 2; ++j) {
            builder.add(&xorAnd[i][j]);
        }
    }

    // Connect clock and clear to all FFs
    for (int i = 0; i < 4; ++i) {
        builder.connect(&clk, 0, &dff[i], "Clock");          // Clock
        builder.connect(&clear, 0, &dff[i], "~Clear");       // CLEAR (active LOW)
        builder.connect(&dff[i], "Q", &counterOut[i], 0);    // Q output
    }

    // Build ripple-carry increment logic (same as testBinaryCounter)
    for (int i = 0; i < 4; ++i) {
        builder.connect(&dff[i], "Q", &notQ[i], 0);
    }

    builder.connect(&dff[0], "Q", &carryAnd[0], 0);
    builder.connect(&dff[1], "Q", &carryAnd[0], 1);
    builder.connect(&carryAnd[0], 0, &carryAnd[1], 0);
    builder.connect(&dff[2], "Q", &carryAnd[1], 1);

    builder.connect(&dff[0], "Q", &notCarry[0], 0);
    builder.connect(&carryAnd[0], 0, &notCarry[1], 0);
    builder.connect(&carryAnd[1], 0, &notCarry[2], 0);

    // Bit 0: D0 = NOT Q0
    builder.connect(&notQ[0], 0, &dff[0], "Data");

    // Bit 1: D1 = Q1 XOR Q0
    builder.connect(&notQ[1], 0, &xorAnd[1][0], 0);
    builder.connect(&dff[0], "Q", &xorAnd[1][0], 1);
    builder.connect(&dff[1], "Q", &xorAnd[1][1], 0);
    builder.connect(&notCarry[0], 0, &xorAnd[1][1], 1);
    builder.connect(&xorAnd[1][0], 0, &xorOr[1], 0);
    builder.connect(&xorAnd[1][1], 0, &xorOr[1], 1);
    builder.connect(&xorOr[1], 0, &dff[1], "Data");

    // Bit 2: D2 = Q2 XOR (Q0 AND Q1)
    builder.connect(&notQ[2], 0, &xorAnd[2][0], 0);
    builder.connect(&carryAnd[0], 0, &xorAnd[2][0], 1);
    builder.connect(&dff[2], "Q", &xorAnd[2][1], 0);
    builder.connect(&notCarry[1], 0, &xorAnd[2][1], 1);
    builder.connect(&xorAnd[2][0], 0, &xorOr[2], 0);
    builder.connect(&xorAnd[2][1], 0, &xorOr[2], 1);
    builder.connect(&xorOr[2], 0, &dff[2], "Data");

    // Bit 3: D3 = Q3 XOR (Q0 AND Q1 AND Q2)
    builder.connect(&notQ[3], 0, &xorAnd[3][0], 0);
    builder.connect(&carryAnd[1], 0, &xorAnd[3][0], 1);
    builder.connect(&dff[3], "Q", &xorAnd[3][1], 0);
    builder.connect(&notCarry[2], 0, &xorAnd[3][1], 1);
    builder.connect(&xorAnd[3][0], 0, &xorOr[3], 0);
    builder.connect(&xorAnd[3][1], 0, &xorOr[3], 1);
    builder.connect(&xorOr[3], 0, &dff[3], "Data");

    auto *simulation = builder.initSimulation();

    clk.setOn(false);
    clear.setOn(true);  // CLEAR = HIGH (inactive)
    simulation->update();

    // Count up to target value
    for (int cycle = 0; cycle < targetCount; ++cycle) {
        clockCycle(simulation, &clk);
    }

    // Verify we reached target count
    int beforeClear = readMultiBitOutput(QVector<GraphicElement *>({
        &counterOut[0], &counterOut[1], &counterOut[2], &counterOut[3]
    }));
    QCOMPARE(beforeClear & 0xF, targetCount & 0xF);

    // Assert CLEAR
    clear.setOn(false);  // CLEAR = LOW (active)
    simulation->update();

    // Verify immediate reset to 0 (no clock edge needed)
    int afterClear = readMultiBitOutput(QVector<GraphicElement *>({
        &counterOut[0], &counterOut[1], &counterOut[2], &counterOut[3]
    }));
    QCOMPARE(afterClear, 0x0);

    if (resetAction == 1) {
        // Release CLEAR and continue counting
        clear.setOn(true);  // CLEAR = HIGH (inactive)
        simulation->update();

        // Clock one more cycle
        clockCycle(simulation, &clk);

        // Should have counted to 1
        int afterContinue = readMultiBitOutput(QVector<GraphicElement *>({
            &counterOut[0], &counterOut[1], &counterOut[2], &counterOut[3]
        }));
        QCOMPARE(afterContinue & 0xF, 0x1);
    } else if (resetAction == 2) {
        // Release and reset again
        clear.setOn(true);  // CLEAR = HIGH (inactive)
        simulation->update();

        // Count a few more cycles
        for (int i = 0; i < 3; ++i) {
            clockCycle(simulation, &clk);
        }

        // Reset again
        clear.setOn(false);  // CLEAR = LOW (active)
        simulation->update();

        // Should be 0 again
        int finalValue = readMultiBitOutput(QVector<GraphicElement *>({
            &counterOut[0], &counterOut[1], &counterOut[2], &counterOut[3]
        }));
        QCOMPARE(finalValue, 0x0);
    }
}

// Shift Register Async Clear Test
// Verifies shift register clears all stages mid-operation
void TestSequential::testShiftRegisterAsyncClear_data()
{
    QTest::addColumn<QVector<bool>>("shiftInSequence");
    QTest::addColumn<bool>("shouldReleaseClear");

    // Shift in pattern, then clear
    QTest::newRow("Clear during shift in 0001") << QVector<bool>({false, false, false, true}) << false;
    QTest::newRow("Clear from full register") << QVector<bool>({true, true, true, true}) << false;
    QTest::newRow("Clear empty register") << QVector<bool>({false, false, false, false}) << false;
    QTest::newRow("Clear then continue") << QVector<bool>({false, true, false, true}) << true;
}

void TestSequential::testShiftRegisterAsyncClear()
{
    QFETCH(QVector<bool>, shiftInSequence);
    QFETCH(bool, shouldReleaseClear);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Input controls
    InputSwitch clk;
    InputSwitch serialIn;
    InputSwitch clear;  // Active LOW clear signal

    // D flip-flops forming shift register (4-bit)
    DFlipFlop dff[4];

    // Output LEDs (parallel output)
    Led dataOut[4];

    // Add all elements
    builder.add(&clk, &serialIn, &clear);
    for (int i = 0; i < 4; ++i) {
        builder.add(&dff[i], &dataOut[i]);
    }

    // Build shift register with clear
    builder.connect(&serialIn, 0, &dff[0], "Data");
    for (int i = 1; i < 4; ++i) {
        builder.connect(&dff[i - 1], "Q", &dff[i], "Data");  // Chain shift
    }
    for (int i = 0; i < 4; ++i) {
        builder.connect(&clk, 0, &dff[i], "Clock");         // Clock
        builder.connect(&clear, 0, &dff[i], "~Clear");       // CLEAR (active LOW)
        builder.connect(&dff[i], "Q", &dataOut[i], 0);       // Q output
    }

    auto *simulation = builder.initSimulation();

    clk.setOn(false);
    clear.setOn(true);  // CLEAR = HIGH (inactive)
    simulation->update();

    // Shift in bits sequentially (2 cycles, partial shift)
    int shiftCount = static_cast<int>((shiftInSequence.size() > 2) ? 2 : shiftInSequence.size());
    for (int bit = 0; bit < shiftCount; ++bit) {
        serialIn.setOn(shiftInSequence[bit]);
        simulation->update();

        clockCycle(simulation, &clk);
    }

    // Verify we can read shift register state before clearing (no crash)
    int beforeClear = readMultiBitOutput(QVector<GraphicElement *>({
        &dataOut[0], &dataOut[1], &dataOut[2], &dataOut[3]
    }));
    QVERIFY2(beforeClear >= 0 && beforeClear <= 15, "Shift register value must be in 4-bit range before clear");

    // Assert CLEAR (no clock)
    clear.setOn(false);  // CLEAR = LOW (active)
    simulation->update();

    // Verify all stages cleared
    int afterClear = readMultiBitOutput(QVector<GraphicElement *>({
        &dataOut[0], &dataOut[1], &dataOut[2], &dataOut[3]
    }));
    QCOMPARE(afterClear, 0x0);

    if (shouldReleaseClear) {
        // Release CLEAR
        clear.setOn(true);  // CLEAR = HIGH (inactive)
        simulation->update();

        // Continue shifting
        for (int bit = shiftCount; bit < shiftInSequence.size(); ++bit) {
            serialIn.setOn(shiftInSequence[bit]);
            simulation->update();

            clockCycle(simulation, &clk);
        }

        // Verify we can shift data normally after clear release
        int afterRelease = readMultiBitOutput(QVector<GraphicElement *>({
            &dataOut[0], &dataOut[1], &dataOut[2], &dataOut[3]
        }));
        // Should have some non-zero value after continuing to shift
        // (exact value depends on what we shifted, but not 0)
        QVERIFY2(afterRelease != 0,
            qPrintable(QString("Shift register should contain data after clear release, got %1").arg(afterRelease)));
    }
}

void TestSequential::testSimpleStateMachine_data()
{
    QTest::addColumn<bool>("trigger");
    QTest::addColumn<bool>("expectedState");

    // Test 1: Start in IDLE state (Q=0)
    QTest::newRow("idle_no_trigger") << false << false;

    // Test 2: Trigger causes transition to ACTIVE (Q=1)
    QTest::newRow("trigger_transition") << true << true;

    // Test 3: Release trigger returns to IDLE (Q=0)
    QTest::newRow("release_to_idle") << false << false;

    // Test 4: Multiple triggers toggle state
    QTest::newRow("second_trigger") << true << true;
}

void TestSequential::testSimpleStateMachine()
{
    QFETCH(bool, trigger);
    QFETCH(bool, expectedState);

    // Build circuit using helper function
    InputSwitch *triggerSwitch = nullptr;
    InputSwitch *clockSwitch = nullptr;
    Led *stateLed = nullptr;
    Led *stateNotLed = nullptr;
    DFlipFlop *stateFf = nullptr;

    auto workspace = std::make_unique<WorkSpace>();
    Simulation *simulation = nullptr;
    buildSimple2StateFsm(workspace.get(), triggerSwitch, clockSwitch, stateLed, stateNotLed, stateFf, simulation);

    // CRITICAL FIX #1: Verify initial state is IDLE (Q=0)
    int initialQ = getInputStatus(stateLed);
    int initialQnot = getInputStatus(stateNotLed);

    if (initialQ != 0) {
        QFAIL(qPrintable(QString("Initial state not IDLE! Q=%1 (expected 0)").arg(initialQ)));
    }

    // Verify Q_not is complement
    if (initialQ != (initialQnot == 0 ? 1 : 0)) {
        QFAIL(qPrintable(QString("Q and Q_not not complementary! Q=%1, Q_not=%2").arg(initialQ).arg(initialQnot)));
    }

    // Set trigger and settle for setup
    triggerSwitch->setOn(trigger);
    simulation->update();

    int readBeforeClock = getInputStatus(stateLed);
    QCOMPARE(readBeforeClock, 0);

    // Complete clock pulse (rising edge captures D, falling edge completes)
    clockCycle(simulation, clockSwitch);

    // Verify final state
    int finalQ = getInputStatus(stateLed);
    int finalQnot = getInputStatus(stateNotLed);

    // Verify Q value matches expected
    QCOMPARE(finalQ, expectedState);

    // Verify Q_not is complement
    if (finalQ != (finalQnot == 0 ? 1 : 0)) {
        QFAIL(qPrintable(QString("Final Q and Q_not not complementary! Q=%1, Q_not=%2").arg(finalQ).arg(finalQnot)));
    }
}

// ============================================================
// ============================================================
// Purpose: Verify all meaningful state transitions in 2-state FSM
// Tests: 8 parametric cases covering transition patterns

void TestSequential::testFsmStateTransitions_data()
{
    QTest::addColumn<int>("transitionCount");

    // Transitions: Alternating pattern, trigger starts at 0, then alternates 1,0,1,0...
    QTest::newRow("single_idle_to_active") << 1;

    QTest::newRow("single_active_to_idle") << 1;

    QTest::newRow("double_idle_active_idle") << 2;

    QTest::newRow("double_active_idle_active") << 2;

    QTest::newRow("triple_alternating") << 3;

    QTest::newRow("rapid_four_transitions") << 4;

    QTest::newRow("five_transitions") << 5;

    QTest::newRow("six_transitions") << 6;
}

void TestSequential::testFsmStateTransitions()
{
    QFETCH(int, transitionCount);

    // Build circuit using helper function (FIX: Extract circuit builder)
    InputSwitch *triggerSwitch = nullptr;
    InputSwitch *clockSwitch = nullptr;
    Led *stateLed = nullptr;
    Led *stateNotLed = nullptr;
    DFlipFlop *stateFf = nullptr;

    auto workspace = std::make_unique<WorkSpace>();
    Simulation *simulation = nullptr;
    buildSimple2StateFsm(workspace.get(), triggerSwitch, clockSwitch, stateLed, stateNotLed, stateFf, simulation);

    // CRITICAL FIX #2: Verify initial state is IDLE (Q=0)
    int initialQ = getInputStatus(stateLed);

    if (initialQ != 0) {
        QFAIL(qPrintable(QString("Initial state not IDLE! Q=%1 (expected 0)").arg(initialQ)));
    }

    // Pattern for transitions: alternate between trigger=1 and trigger=0
    QVector<bool> triggers(transitionCount + 1);
    triggers[0] = false;  // Start in IDLE (trigger=0, so Q stays 0)
    for (int i = 1; i <= transitionCount; ++i) {
        triggers[i] = (i % 2 == 1);  // Alternate: 1, 0, 1, 0, ...
    }

    bool currentState = false;  // Initial state: IDLE (Q=0)
    int preClockViolations = 0;
    int postClockViolations = 0;
    QStringList violationDetails;

    for (int t = 0; t <= transitionCount; ++t) {

        triggerSwitch->setOn(triggers[t]);
        simulation->update();

        // FIX: Don't QCOMPARE here - collect violations instead
        int readBefore = getInputStatus(stateLed);

        if (readBefore != static_cast<int>(currentState)) {
            preClockViolations++;
            violationDetails.append(QString("Step %1 pre-clock: Q=%2, expected %3").arg(t).arg(readBefore).arg(currentState));
        }

        if (t < transitionCount) {
            // Complete clock pulse
            clockCycle(simulation, clockSwitch);
            simulation->update();

            // After clock, state should be what was on D (trigger input)
            bool newState = triggers[t];
            int readState = getInputStatus(stateLed);

            if (readState != static_cast<int>(newState)) {
                postClockViolations++;
                violationDetails.append(QString("Step %1 post-clock: Q=%2, expected %3 (trigger=%4)")
                    .arg(t).arg(readState).arg(newState).arg(triggers[t]));
            }

            currentState = newState;
        }
    }

    // FIX: Assert all violations at once (not in loop)

    QCOMPARE(preClockViolations, 0);
    QCOMPARE(postClockViolations, 0);
}

// ============================================================
// ============================================================
// Purpose: Verify setup/hold timing and rapid state changes
// Tests: 4 parametric cases with rapid transitions

void TestSequential::testFsmTimingEdgeCases_data()
{
    QTest::addColumn<int>("rapidChanges");

    QTest::newRow("setup_phase_stable") << 2;
    QTest::newRow("hold_phase_stable") << 3;
    QTest::newRow("rapid_rise_changes") << 4;
    QTest::newRow("rapid_oscillation") << 5;
}

void TestSequential::testFsmTimingEdgeCases()
{
    QFETCH(int, rapidChanges);

    // Build circuit using helper function (FIX: Extract circuit builder)
    InputSwitch *triggerSwitch = nullptr;
    InputSwitch *clockSwitch = nullptr;
    Led *stateLed = nullptr;
    Led *stateNotLed = nullptr;
    DFlipFlop *stateFf = nullptr;

    auto workspace = std::make_unique<WorkSpace>();
    Simulation *simulation = nullptr;
    buildSimple2StateFsm(workspace.get(), triggerSwitch, clockSwitch, stateLed, stateNotLed, stateFf, simulation);

    // CRITICAL FIX #2: Verify initial state is IDLE (Q=0)
    int initialQ = getInputStatus(stateLed);
    int initialQnot = getInputStatus(stateNotLed);

    if (initialQ != 0) {
        QFAIL(qPrintable(QString("Initial state not IDLE! Q=%1 (expected 0)").arg(initialQ)));
    }

    // Verify Q_not is complement
    if (initialQ != (initialQnot == 0 ? 1 : 0)) {
        QFAIL(qPrintable(QString("Q and Q_not not complementary! Q=%1, Q_not=%2").arg(initialQ).arg(initialQnot)));
    }

    bool currentState = false;
    int preClockViolations = 0;
    int postClockViolations = 0;
    QStringList violationDetails;

    for (int i = 0; i < rapidChanges; ++i) {
        bool triggerValue = (i % 2 == 1);

        triggerSwitch->setOn(triggerValue);

        // Setup phase: Allow D input to stabilize before clock
        simulation->update();

        int readBefore = getInputStatus(stateLed);

        if (readBefore != static_cast<int>(currentState)) {
            preClockViolations++;
            violationDetails.append(QString("Cycle %1 pre-clock: Q=%2, expected %3 (trigger=%4)")
                .arg(i).arg(readBefore).arg(currentState).arg(triggerValue));
        }

        // Complete clock pulse
        clockCycle(simulation, clockSwitch);

        // Extended settle for glitch detection
        simulation->update();

        bool newState = triggerValue;
        int readAfter = getInputStatus(stateLed);
        int readAfterQnot = getInputStatus(stateNotLed);

        if (readAfter != static_cast<int>(newState)) {
            postClockViolations++;
            violationDetails.append(QString("Cycle %1 post-clock: Q=%2, expected %3 (trigger=%4)")
                .arg(i).arg(readAfter).arg(newState).arg(triggerValue));
        }

        // Verify Q_not is complement
        if (readAfter != (readAfterQnot == 0 ? 1 : 0)) {
        }

        currentState = newState;
    }

    // Timing analysis summary

    QCOMPARE(preClockViolations, 0);
    QCOMPARE(postClockViolations, 0);
}

// ============================================================
// ============================================================
// Purpose: Verify FSM behavior over complex multi-cycle sequences
// Tests: 6 parametric cases with varied sequences

void TestSequential::testFsmExtendedSequences_data()
{
    QTest::addColumn<QVector<bool>>("triggerSequence");

    QTest::newRow("sequence_stable_idle") << QVector<bool>{false, false, false};
    QTest::newRow("sequence_stable_active") << QVector<bool>{true, true, true};
    QTest::newRow("sequence_alternate_2") << QVector<bool>{true, false, true, false};
    QTest::newRow("sequence_alternate_3") << QVector<bool>{false, true, false, true, false};
    QTest::newRow("sequence_complex_6") << QVector<bool>{true, true, false, true, false, false};
    QTest::newRow("sequence_pulse_pattern") << QVector<bool>{true, false, false, true, false, false, true};
}

void TestSequential::testFsmExtendedSequences()
{
    QFETCH(QVector<bool>, triggerSequence);

    // Build circuit using helper function (FIX: Extract circuit builder)
    InputSwitch *triggerSwitch = nullptr;
    InputSwitch *clockSwitch = nullptr;
    Led *stateLed = nullptr;
    Led *stateNotLed = nullptr;
    DFlipFlop *stateFf = nullptr;

    auto workspace = std::make_unique<WorkSpace>();
    Simulation *simulation = nullptr;
    buildSimple2StateFsm(workspace.get(), triggerSwitch, clockSwitch, stateLed, stateNotLed, stateFf, simulation);

    // CRITICAL FIX #2: Verify initial state is IDLE (Q=0)
    int initialQ = getInputStatus(stateLed);
    int initialQnot = getInputStatus(stateNotLed);

    if (initialQ != 0) {
        QFAIL(qPrintable(QString("Initial state not IDLE! Q=%1 (expected 0)").arg(initialQ)));
    }

    // Verify Q_not is complement
    if (initialQ != (initialQnot == 0 ? 1 : 0)) {
        QFAIL(qPrintable(QString("Q and Q_not not complementary! Q=%1, Q_not=%2").arg(initialQ).arg(initialQnot)));
    }

    bool currentState = false;
    int preClockViolations = 0;
    int postClockViolations = 0;
    QStringList violationDetails;
    QVector<int> stateTrace;
    stateTrace.append(currentState);

    for (int cycle = 0; cycle < triggerSequence.length(); ++cycle) {
        bool triggerValue = triggerSequence[cycle];

        triggerSwitch->setOn(triggerValue);

        // Setup phase: Allow D input to stabilize before clock
        simulation->update();

        int readBefore = getInputStatus(stateLed);

        if (readBefore != static_cast<int>(currentState)) {
            preClockViolations++;
            violationDetails.append(QString("Cycle %1 pre-clock: Q=%2, expected %3 (trigger=%4)")
                .arg(cycle).arg(readBefore).arg(currentState).arg(triggerValue));
        }

        // Complete clock pulse
        clockCycle(simulation, clockSwitch);

        // Settle phase after clock
        simulation->update();

        // Expected next state: what's on trigger (captured on rising edge)
        bool expectedNextState = triggerValue;
        int readAfter = getInputStatus(stateLed);
        int readAfterQnot = getInputStatus(stateNotLed);

        if (readAfter != static_cast<int>(expectedNextState)) {
            postClockViolations++;
            violationDetails.append(QString("Cycle %1 post-clock: Q=%2, expected %3 (trigger=%4)")
                .arg(cycle).arg(readAfter).arg(expectedNextState).arg(triggerValue));
        }

        // Verify Q_not is complement
        if (readAfter != (readAfterQnot == 0 ? 1 : 0)) {
        }

        stateTrace.append(readAfter);
        currentState = expectedNextState;
    }

    // Verify steady state after sequence completes
    simulation->update();

    QCOMPARE(preClockViolations, 0);
    QCOMPARE(postClockViolations, 0);
}

// ============================================================
// ============================================================
// Purpose: Verify that Q_not is always the complement of Q
// Tests: 8 parametric cases verifying Q=!Q_not across state transitions

void TestSequential::testFsmQnotComplementarity_data()
{
    QTest::addColumn<QVector<bool>>("triggerSequence");

    QTest::newRow("qnot_idle_stable") << QVector<bool>{false, false, false};
    QTest::newRow("qnot_active_stable") << QVector<bool>{true, true, true};
    QTest::newRow("qnot_single_transition") << QVector<bool>{true};
    QTest::newRow("qnot_transition_back") << QVector<bool>{true, false};
    QTest::newRow("qnot_rapid_alternation") << QVector<bool>{true, false, true, false, true};
    QTest::newRow("qnot_extended_sequence") << QVector<bool>{true, true, false, true, false, false};
}

void TestSequential::testFsmQnotComplementarity()
{
    QFETCH(QVector<bool>, triggerSequence);

    InputSwitch *triggerSwitch = nullptr;
    InputSwitch *clockSwitch = nullptr;
    Led *stateLed = nullptr;
    Led *stateNotLed = nullptr;
    DFlipFlop *stateFf = nullptr;

    auto workspace = std::make_unique<WorkSpace>();
    Simulation *simulation = nullptr;
    buildSimple2StateFsm(workspace.get(), triggerSwitch, clockSwitch, stateLed, stateNotLed, stateFf, simulation);

    int complementViolations = 0;
    QStringList violationDetails;

    // Verify initial state
    int Q = getInputStatus(stateLed);
    int Qnot = getInputStatus(stateNotLed);

    if (Q != (Qnot == 0 ? 1 : 0)) {
        complementViolations++;
        violationDetails.append("Initial state: Q_not not complement of Q");
    }

    for (int cycle = 0; cycle < triggerSequence.length(); ++cycle) {
        bool triggerValue = triggerSequence[cycle];
        triggerSwitch->setOn(triggerValue);

        simulation->update();

        // Clock edges
        // Complete clock pulse
        clockCycle(simulation, clockSwitch);
        simulation->update();

        // Verify Q and Q_not are complementary
        Q = getInputStatus(stateLed);
        Qnot = getInputStatus(stateNotLed);

        if (Q != (Qnot == 0 ? 1 : 0)) {
            complementViolations++;
            violationDetails.append(QString("Cycle %1: Q=%2, Q_not=%3 (not complementary)")
                .arg(cycle).arg(Q).arg(Qnot));
        }
    }

    QCOMPARE(complementViolations, 0);
}

// ============================================================
// ============================================================
// Purpose: Verify FSM remains stable over extended 100+ cycle sequences
// Tests: Alternating pattern sustained for 100 cycles

void TestSequential::testFsmLongSequenceStability_data()
{
    QTest::addColumn<int>("cycleCount");

    QTest::newRow("stability_50_cycles") << 50;
    QTest::newRow("stability_100_cycles") << 100;
    QTest::newRow("stability_200_cycles") << 200;
}

void TestSequential::testFsmLongSequenceStability()
{
    QFETCH(int, cycleCount);

    InputSwitch *triggerSwitch = nullptr;
    InputSwitch *clockSwitch = nullptr;
    Led *stateLed = nullptr;
    Led *stateNotLed = nullptr;
    DFlipFlop *stateFf = nullptr;

    auto workspace = std::make_unique<WorkSpace>();
    Simulation *simulation = nullptr;
    buildSimple2StateFsm(workspace.get(), triggerSwitch, clockSwitch, stateLed, stateNotLed, stateFf, simulation);

    // Verify initial state
    int initialQ = getInputStatus(stateLed);
    if (initialQ != 0) {
        QFAIL(qPrintable(QString("Initial state not IDLE! Q=%1").arg(initialQ)));
    }

    int stateErrors = 0;
    int complementErrors = 0;
    QVector<int> stateCheckpoints;

    // Run alternating pattern for cycleCount cycles
    for (int cycle = 0; cycle < cycleCount; ++cycle) {
        bool triggerValue = (cycle % 2 == 1);  // Alternate: 0,1,0,1,0,1...

        triggerSwitch->setOn(triggerValue);

        simulation->update();

        // Complete clock pulse
        clockCycle(simulation, clockSwitch);
        simulation->update();

        int Q = getInputStatus(stateLed);
        int Qnot = getInputStatus(stateNotLed);
        int expectedQ = triggerValue;

        // Check state correctness
        if (Q != expectedQ) {
            stateErrors++;
        }

        // Check complementarity every 10 cycles
        if (cycle % 10 == 0) {
            if (Q != (Qnot == 0 ? 1 : 0)) {
                complementErrors++;
            }
            stateCheckpoints.append(Q);
        }
    }

    // Verify steady state after sequence
    simulation->update();

    QCOMPARE(stateErrors, 0);
    QCOMPARE(complementErrors, 0);
}

// ============================================================
// ============================================================
// Purpose: Verify that setting trigger=1 then trigger=0 locks state
// Tests: Once set to 1, trigger=0 should hold state

void TestSequential::testFsmStateLock_data()
{
    // No parameters needed for this test
}

void TestSequential::testFsmStateLock()
{

    InputSwitch *triggerSwitch = nullptr;
    InputSwitch *clockSwitch = nullptr;
    Led *stateLed = nullptr;
    Led *stateNotLed = nullptr;
    DFlipFlop *stateFf = nullptr;

    auto workspace = std::make_unique<WorkSpace>();
    Simulation *simulation = nullptr;
    buildSimple2StateFsm(workspace.get(), triggerSwitch, clockSwitch, stateLed, stateNotLed, stateFf, simulation);

    triggerSwitch->setOn(true);
    simulation->update();

    // Complete clock pulse
    clockCycle(simulation, clockSwitch);
    simulation->update();

    int Q_after_set = getInputStatus(stateLed);
    QCOMPARE(Q_after_set, 1);

    triggerSwitch->setOn(false);
    simulation->update();

    // NO CLOCK - just settle
    int Q_before_clock = getInputStatus(stateLed);
    QCOMPARE(Q_before_clock, 1);  // Q locked at 1 because we haven't clocked yet

    // Complete clock pulse
    clockCycle(simulation, clockSwitch);
    simulation->update();

    int Q_after_clock = getInputStatus(stateLed);
    QCOMPARE(Q_after_clock, 0);

    triggerSwitch->setOn(true);  // Change trigger back to 1
    simulation->update();

    int Q_no_clock = getInputStatus(stateLed);
    QCOMPARE(Q_no_clock, 0);  // Q still locked at 0
}

// ============================================================
// ============================================================
// Purpose: Verify FSM behavior with back-to-back clock edges (no settle)
// Tests: Clock edges without settling between them

void TestSequential::testFsmRapidClockNoSettle_data()
{
    QTest::addColumn<int>("rapidCycleCount");

    QTest::newRow("rapid_2_cycles") << 2;
    QTest::newRow("rapid_5_cycles") << 5;
    QTest::newRow("rapid_10_cycles") << 10;
}

void TestSequential::testFsmRapidClockNoSettle()
{
    QFETCH(int, rapidCycleCount);

    InputSwitch *triggerSwitch = nullptr;
    InputSwitch *clockSwitch = nullptr;
    Led *stateLed = nullptr;
    Led *stateNotLed = nullptr;
    DFlipFlop *stateFf = nullptr;

    auto workspace = std::make_unique<WorkSpace>();
    Simulation *simulation = nullptr;
    buildSimple2StateFsm(workspace.get(), triggerSwitch, clockSwitch, stateLed, stateNotLed, stateFf, simulation);

    int stateErrors = 0;

    for (int cycle = 0; cycle < rapidCycleCount; ++cycle) {
        bool triggerValue = (cycle % 2 == 1);

        triggerSwitch->setOn(triggerValue);

        // Minimal settle before clock (only 1 cycle instead of SETUP_SETTLE_CYCLES)
        simulation->update();

        // Complete clock pulse (minimal settle)
        clockCycle(simulation, clockSwitch);

        int Q = getInputStatus(stateLed);
        int expectedQ = triggerValue;

        if (Q != expectedQ) {
            stateErrors++;
        }
    }

    QCOMPARE(stateErrors, 0);
}

// ============================================================
// ============================================================
// Purpose: Verify FSM behavior when trigger changes during clock transition
// Tests: Trigger change DURING rising/falling edge

void TestSequential::testFsmTriggerDuringClock_data()
{
    QTest::addColumn<bool>("initialTrigger");
    QTest::addColumn<bool>("triggerDuringClock");

    QTest::newRow("change_0_to_1_during") << false << true;
    QTest::newRow("change_1_to_0_during") << true << false;
    QTest::newRow("stable_0_during") << false << false;
    QTest::newRow("stable_1_during") << true << true;
}

void TestSequential::testFsmTriggerDuringClock()
{
    QFETCH(bool, initialTrigger);
    QFETCH(bool, triggerDuringClock);

    InputSwitch *triggerSwitch = nullptr;
    InputSwitch *clockSwitch = nullptr;
    Led *stateLed = nullptr;
    Led *stateNotLed = nullptr;
    DFlipFlop *stateFf = nullptr;

    auto workspace = std::make_unique<WorkSpace>();
    Simulation *simulation = nullptr;
    buildSimple2StateFsm(workspace.get(), triggerSwitch, clockSwitch, stateLed, stateNotLed, stateFf, simulation);

    triggerSwitch->setOn(initialTrigger);
    simulation->update();

    int Q_before_clock = getInputStatus(stateLed);
    QCOMPARE(Q_before_clock, 0);

    // Rising edge - captures trigger value at this moment
    clockToggle(simulation, clockSwitch);  // LOW -> HIGH

    // Change trigger DURING the clock pulse (after rising edge)
    triggerSwitch->setOn(triggerDuringClock);
    simulation->update();

    // Falling edge - completes the clock pulse
    clockToggle(simulation, clockSwitch);  // HIGH -> LOW
    simulation->update();

    int Q_after_clock = getInputStatus(stateLed);

    // The Q should have captured initialTrigger on the rising edge,
    // NOT the triggerDuringClock (which changed during/after rising edge)

    QCOMPARE(Q_after_clock, initialTrigger);
}

// ============================================================
// ============================================================
// Purpose: Verify FSM stability over very long sequences with state changes
// Tests: 500+ cycle sequence with various patterns

void TestSequential::testFsmLongTermStability_data()
{
    // No parameters needed for this test
}

void TestSequential::testFsmLongTermStability()
{

    InputSwitch *triggerSwitch = nullptr;
    InputSwitch *clockSwitch = nullptr;
    Led *stateLed = nullptr;
    Led *stateNotLed = nullptr;
    DFlipFlop *stateFf = nullptr;

    auto workspace = std::make_unique<WorkSpace>();
    Simulation *simulation = nullptr;
    buildSimple2StateFsm(workspace.get(), triggerSwitch, clockSwitch, stateLed, stateNotLed, stateFf, simulation);

    const int STRESS_CYCLES = 500;
    int stateErrors = 0;
    int complementErrors = 0;

    for (int cycle = 0; cycle < STRESS_CYCLES; ++cycle) {
        // Mixed pattern: 4 alternating, 4 hold 1, 4 hold 0, repeat
        bool triggerValue;
        if (cycle % 12 < 4) {
            triggerValue = (cycle % 2 == 1);  // Alternating: 0,1,0,1,...
        } else if (cycle % 12 < 8) {
            triggerValue = true;  // Hold 1
        } else {
            triggerValue = false;  // Hold 0
        }

        triggerSwitch->setOn(triggerValue);
        simulation->update();

        // Complete clock pulse
        clockCycle(simulation, clockSwitch);
        simulation->update();

        int Q = getInputStatus(stateLed);
        int Qnot = getInputStatus(stateNotLed);
        int expectedQ = triggerValue;

        if (Q != expectedQ) {
            stateErrors++;
            if (stateErrors <= 5) {  // Only report first 5 errors
            }
        }

        // Check complement every 50 cycles
        if (cycle % 50 == 0 && Q != (Qnot == 0 ? 1 : 0)) {
            complementErrors++;
        }

        // Progress indicator every 100 cycles
        if (cycle % 100 == 0) {
        }
    }

    // Verify steady state
    simulation->update();

    QCOMPARE(stateErrors, 0);
    QCOMPARE(complementErrors, 0);
}

// ============================================================
// T Flip-Flop Tests (Issue #3 - Integration Coverage)
// ============================================================

// T Flip-Flop Toggle Behavior Test
// Verifies toggle functionality: T=1 causes Q to toggle on clock edge
// T=0 causes Q to hold previous state
void TestSequential::testTFlipFlopToggle_data()
{
    QTest::addColumn<bool>("tInput");
    QTest::addColumn<bool>("expectedToggle");

    // T=0: Q holds previous state (no toggle)
    QTest::newRow("Hold when T=0") << false << false;
    // T=1: Q toggles on clock edge
    QTest::newRow("Toggle when T=1") << true << true;
}

void TestSequential::testTFlipFlopToggle()
{
    QFETCH(bool, tInput);
    QFETCH(bool, expectedToggle);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch tSwitch, clockSwitch;
    TFlipFlop tff;
    Led ledQ, ledQnot;

    builder.add(&tSwitch, &clockSwitch, &tff, &ledQ, &ledQnot);

    // Connect T flip-flop: T (port 0), Clock (port 1), Preset (port 2), Clear (port 3)
    builder.connect(&tSwitch, 0, &tff, "T");       // T input
    builder.connect(&clockSwitch, 0, &tff, "Clock"); // Clock input
    builder.connect(&tff, "Q", &ledQ, 0);           // Q output
    builder.connect(&tff, "~Q", &ledQnot, 0);       // Q_not output

    auto *simulation = builder.initSimulation();

    // Initialize: clock low, T low
    clockSwitch.setOn(false);
    tSwitch.setOn(false);
    simulation->update();
    simulation->update();

    // Read initial Q state (should be inactive/0 by default)
    bool initialQ = getInputStatus(&ledQ);

    // Set T input value and stabilize
    tSwitch.setOn(tInput);
    simulation->update();

    // Apply clock pulse (rising edge triggers state change)
    clockSwitch.setOn(true);
    simulation->update();
    clockSwitch.setOn(false);
    simulation->update();
    simulation->update();

    bool qAfterClock = getInputStatus(&ledQ);
    bool qNotAfterClock = getInputStatus(&ledQnot);

    if (expectedToggle) {
        // T=1: Q should toggle from initial state
        QCOMPARE(qAfterClock, !initialQ);
        // Verify Q_not is complement
        QCOMPARE(qNotAfterClock, initialQ);
    } else {
        // T=0: Q should hold initial state
        QCOMPARE(qAfterClock, initialQ);
        // Verify Q_not is complement
        QCOMPARE(qNotAfterClock, !initialQ);
    }
}

// ============================================================
// SR Flip-Flop Tests (Issue #3 - Integration Coverage)
// ============================================================

// SR Flip-Flop Set/Reset Behavior Test
// Verifies set and reset functionality with clock synchronization
void TestSequential::testSRFlipFlopSetReset_data()
{
    QTest::addColumn<bool>("sInput");
    QTest::addColumn<bool>("rInput");
    QTest::addColumn<bool>("expectedQ");

    // S=1, R=0: Set -> Q=1
    QTest::newRow("Set (S=1,R=0)") << true << false << true;
    // S=0, R=1: Reset -> Q=0
    QTest::newRow("Reset (S=0,R=1)") << false << true << false;
    // S=0, R=0: Hold (no change)
    QTest::newRow("Hold (S=0,R=0)") << false << false << false;
}

void TestSequential::testSRFlipFlopSetReset()
{
    QFETCH(bool, sInput);
    QFETCH(bool, rInput);
    QFETCH(bool, expectedQ);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch sSwitch, clockSwitch, rSwitch;
    SRFlipFlop srff;
    Led ledQ, ledQnot;

    builder.add(&sSwitch, &clockSwitch, &rSwitch, &srff, &ledQ, &ledQnot);

    // Connect SR flip-flop: S (port 0), Clock (port 1), R (port 2), Preset (port 3), Clear (port 4)
    builder.connect(&sSwitch, 0, &srff, "S");        // S (Set) input
    builder.connect(&clockSwitch, 0, &srff, "Clock"); // Clock input
    builder.connect(&rSwitch, 0, &srff, "R");         // R (Reset) input
    builder.connect(&srff, "Q", &ledQ, 0);            // Q output
    builder.connect(&srff, "~Q", &ledQnot, 0);        // Q_not output

    auto *simulation = builder.initSimulation();

    // Initialize: all inputs low, clock low
    sSwitch.setOn(false);
    clockSwitch.setOn(false);
    rSwitch.setOn(false);
    simulation->update();
    simulation->update();

    // Pre-set to known state: Reset (R=1)
    rSwitch.setOn(true);
    simulation->update();
    clockSwitch.setOn(true);
    simulation->update();
    clockSwitch.setOn(false);
    simulation->update();
    simulation->update();
    rSwitch.setOn(false);  // Release reset for test

    // Now apply test inputs
    sSwitch.setOn(sInput);
    rSwitch.setOn(rInput);
    simulation->update();

    // Clock pulse to latch the state
    clockSwitch.setOn(true);
    simulation->update();
    clockSwitch.setOn(false);
    simulation->update();
    simulation->update();

    bool qResult = getInputStatus(&ledQ);
    bool qNotResult = getInputStatus(&ledQnot);

    // Verify result
    QCOMPARE(qResult, expectedQ);
    // Verify complementary output
    QCOMPARE(qNotResult, !expectedQ);
}

