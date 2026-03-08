// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Logic/TestLogicElementsErrors.h"

#include <QtTest>

#include "App/Element/LogicElements/LogicAnd.h"
#include "App/Element/LogicElements/LogicInput.h"
#include "App/Element/LogicElements/LogicNot.h"
#include "App/Element/LogicElements/LogicOr.h"

// ============================================================
// Robustness Tests — Error Conditions and Boundary Behavior
// ============================================================

/**
 * Test: connectPredecessor() has no bounds checking — out-of-range indices
 * are undefined behavior and cannot be tested safely. This test verifies the
 * valid boundary (highest valid index = numInputs - 1) works correctly.
 */
void TestLogicElementsErrors::testInvalidInputPortIndex()
{
    // 4-input AND gate: valid input port indices are 0..3
    LogicAnd gate(4);
    LogicInput in0, in1, in2, in3;

    gate.connectPredecessor(0, &in0, 0);
    gate.connectPredecessor(1, &in1, 0);
    gate.connectPredecessor(2, &in2, 0);
    gate.connectPredecessor(3, &in3, 0);  // highest valid index

    in0.setOutputValue(true);
    in1.setOutputValue(true);
    in2.setOutputValue(true);
    in3.setOutputValue(true);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), true);  // all inputs true

    // The boundary port controls the output correctly
    in3.setOutputValue(false);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), false);  // boundary input correctly propagated
}

/**
 * Test: An unconnected predecessor (nullptr) is treated as false by inputValue().
 * Connect only some ports; others remain null from construction.
 */
void TestLogicElementsErrors::testNullPredecessor()
{
    // 2-input AND: connect port 0, leave port 1 null
    LogicAnd andGate(2);
    LogicInput input;
    andGate.connectPredecessor(0, &input, 0);

    input.setOutputValue(true);
    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), false);  // AND(true, null=false) = false

    input.setOutputValue(false);
    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), false);  // AND(false, null=false) = false

    // 2-input OR: connect port 0, leave port 1 null
    LogicOr orGate(2);
    orGate.connectPredecessor(0, &input, 0);

    input.setOutputValue(true);
    orGate.updateLogic();
    QCOMPARE(orGate.outputValue(), true);  // OR(true, null=false) = true

    input.setOutputValue(false);
    orGate.updateLogic();
    QCOMPARE(orGate.outputValue(), false);  // OR(false, null=false) = false
}

/**
 * Test: Gate with all inputs never connected (all predecessors null).
 * Since m_isValid starts as true and validate() is never called, updateInputs()
 * proceeds and inputValue() returns false for each null predecessor.
 */
void TestLogicElementsErrors::testDisconnectedInput()
{
    // AND(false, false) = false
    LogicAnd andGate(2);
    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), false);

    // OR(false, false) = false
    LogicOr orGate(2);
    orGate.updateLogic();
    QCOMPARE(orGate.outputValue(), false);

    // NOT(false) = true — single unconnected input is treated as false
    LogicNot notGate;
    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), true);
}

/**
 * Test: Calling connectPredecessor() twice on the same port overwrites the
 * first connection with the second. The first source is silently replaced.
 */
void TestLogicElementsErrors::testMultipleConnectionsSamePort()
{
    LogicAnd gate(2);
    LogicInput first, second, other;

    // Connect port 0 to 'first', then overwrite with 'second'
    gate.connectPredecessor(0, &first, 0);
    gate.connectPredecessor(1, &other, 0);
    gate.connectPredecessor(0, &second, 0);  // overwrites port 0

    first.setOutputValue(true);   // disconnected — no effect
    second.setOutputValue(false); // active on port 0
    other.setOutputValue(true);   // active on port 1

    gate.updateLogic();
    QCOMPARE(gate.outputValue(), false);  // AND(second=false, other=true) = false

    second.setOutputValue(true);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), true);   // AND(second=true, other=true) = true

    // Verify 'first' truly has no effect
    first.setOutputValue(false);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), true);   // still true; 'first' is not wired
}

/**
 * Test: Rapid state changes don't cause issues.
 * Verifies gate stability under rapid input toggling.
 */
void TestLogicElementsErrors::testRapidStateChanges()
{
    LogicAnd andGate(2);
    LogicInput input1, input2;

    andGate.connectPredecessor(0, &input1, 0);
    andGate.connectPredecessor(1, &input2, 0);

    for (int i = 0; i < 1000; ++i) {
        input1.setOutputValue(i % 2 == 0);
        input2.setOutputValue(i % 3 == 0);
        andGate.updateLogic();
        bool expected = (i % 2 == 0) && (i % 3 == 0);
        QCOMPARE(andGate.outputValue(), expected);
    }
}

/**
 * Test: validate() marks a gate invalid if any predecessor is null.
 * After validation, updateLogic() becomes a no-op (updateInputs returns false)
 * and the output stays at its initial value (false).
 */
void TestLogicElementsErrors::testUnconnectedGate()
{
    // Call validate() explicitly: all predecessors null → m_isValid = false
    LogicAnd gate(2);
    gate.validate();
    QVERIFY(!gate.isValid());

    gate.updateLogic();
    QCOMPARE(gate.outputValue(), false);  // output unchanged (no-op)

    LogicNot notGate;
    notGate.validate();
    QVERIFY(!notGate.isValid());

    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), false);  // output unchanged (no-op), NOT NOT executed
}

/**
 * Test: Signal propagates correctly through a deep cascade of AND gates.
 */
void TestLogicElementsErrors::testDeepCascading()
{
    // Chain: (in1 AND in2) → stage1 → AND in3 → stage2 → AND in4 → stage3
    LogicInput in1, in2, in3, in4;
    LogicAnd stage1(2), stage2(2), stage3(2);

    stage1.connectPredecessor(0, &in1, 0);
    stage1.connectPredecessor(1, &in2, 0);
    stage2.connectPredecessor(0, &stage1, 0);
    stage2.connectPredecessor(1, &in3, 0);
    stage3.connectPredecessor(0, &stage2, 0);
    stage3.connectPredecessor(1, &in4, 0);

    // All true → output true
    in1.setOutputValue(true);
    in2.setOutputValue(true);
    in3.setOutputValue(true);
    in4.setOutputValue(true);
    stage1.updateLogic();
    stage2.updateLogic();
    stage3.updateLogic();
    QCOMPARE(stage3.outputValue(), true);

    // One false at the start propagates through the full chain
    in1.setOutputValue(false);
    stage1.updateLogic();
    stage2.updateLogic();
    stage3.updateLogic();
    QCOMPARE(stage1.outputValue(), false);
    QCOMPARE(stage2.outputValue(), false);
    QCOMPARE(stage3.outputValue(), false);

    // Restore in1; inject false midway via in3
    in1.setOutputValue(true);
    in3.setOutputValue(false);
    stage1.updateLogic();
    stage2.updateLogic();
    stage3.updateLogic();
    QCOMPARE(stage1.outputValue(), true);
    QCOMPARE(stage2.outputValue(), false);  // false injected here
    QCOMPARE(stage3.outputValue(), false);
}

/**
 * Test: outputSize() correctly reports the number of output ports, and
 * outputValue() works correctly at valid boundary indices.
 *
 * Note: QList::at() (used internally) calls Q_ASSERT_X for out-of-range
 * access — it does NOT throw. Out-of-range access is undefined behavior in
 * release builds and abort() in debug builds; it cannot be tested safely.
 */
void TestLogicElementsErrors::testInvalidOutputPortIndex()
{
    // All standard logic gates have exactly 1 output
    LogicAnd andGate(2);
    LogicOr orGate(3);
    LogicNot notGate;

    QCOMPARE(andGate.outputSize(), 1);
    QCOMPARE(orGate.outputSize(), 1);
    QCOMPARE(notGate.outputSize(), 1);

    // Valid boundary index (outputSize() - 1 = 0) is accessible
    QCOMPARE(andGate.outputValue(0), false);
    QCOMPARE(orGate.outputValue(0), false);
    QCOMPARE(notGate.outputValue(0), false);

    // LogicInput supports configurable output counts
    // Constructor sets output 0 to defaultValue; remaining outputs start false
    LogicInput input(true, 4);  // 4 outputs: index 0 = true, indices 1-3 = false
    QCOMPARE(input.outputSize(), 4);
    QCOMPARE(input.outputValue(0), true);
    QCOMPARE(input.outputValue(3), false);  // highest valid index (outputSize() - 1)

    // Verify each output is independently settable
    input.setOutputValue(1, true);
    input.setOutputValue(3, true);
    QCOMPARE(input.outputValue(0), true);
    QCOMPARE(input.outputValue(1), true);
    QCOMPARE(input.outputValue(2), false);
    QCOMPARE(input.outputValue(3), true);
}

/**
 * Test: A gate connected to itself (feedback cycle) does not crash or
 * infinite-loop. It reads its own stale output on each updateLogic() call.
 */
void TestLogicElementsErrors::testConnectionCycles()
{
    LogicNot notGate;
    // Self-loop: NOT gate's output feeds back to its own input
    notGate.connectPredecessor(0, &notGate, 0);

    // Initial output is false
    QCOMPARE(notGate.outputValue(), false);

    // Each call reads the current (stale) output and inverts it
    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), true);   // NOT(false) = true

    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), false);  // NOT(true) = false

    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), true);   // stable single-step oscillation
}

/**
 * Test: Exhaustive truth table for a 4-input AND gate (all 16 combinations).
 * AND outputs true only when all four inputs are true (combo = 0b1111 = 15).
 */
void TestLogicElementsErrors::testInputValueBoundary()
{
    LogicAnd gate(4);
    LogicInput in0, in1, in2, in3;

    gate.connectPredecessor(0, &in0, 0);
    gate.connectPredecessor(1, &in1, 0);
    gate.connectPredecessor(2, &in2, 0);
    gate.connectPredecessor(3, &in3, 0);

    for (int combo = 0; combo < 16; ++combo) {
        in0.setOutputValue((combo & 0x1) != 0);
        in1.setOutputValue((combo & 0x2) != 0);
        in2.setOutputValue((combo & 0x4) != 0);
        in3.setOutputValue((combo & 0x8) != 0);
        gate.updateLogic();
        QCOMPARE(gate.outputValue(), combo == 15);
    }
}

/**
 * Test: Gates with 0 inputs return their identity element.
 * std::accumulate over an empty range returns the init value:
 *   AND(∅) = true   (init = true,  bit_and)
 *   OR(∅)  = false  (init = false, bit_or)
 */
void TestLogicElementsErrors::testGateWithZeroInputs()
{
    LogicAnd andGate(0);
    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), true);  // identity of AND

    LogicOr orGate(0);
    orGate.updateLogic();
    QCOMPARE(orGate.outputValue(), false);  // identity of OR
}
