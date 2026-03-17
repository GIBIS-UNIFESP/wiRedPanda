// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Logic/TestLogicElementsErrors.h"

#include <QtTest>

#include "App/Element/LogicElements/LogicAnd.h"
#include "App/Element/LogicElements/LogicNot.h"
#include "App/Element/LogicElements/LogicOr.h"
#include "App/Element/LogicElements/LogicSource.h"

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
    LogicSource in0, in1, in2, in3;

    gate.connectPredecessor(0, &in0, 0);
    gate.connectPredecessor(1, &in1, 0);
    gate.connectPredecessor(2, &in2, 0);
    gate.connectPredecessor(3, &in3, 0);  // highest valid index

    in0.setOutputValue(true);
    in1.setOutputValue(true);
    in2.setOutputValue(true);
    in3.setOutputValue(true);
    gate.updateLogic();
    QCOMPARE(gate.outputValue() == Status::Active, true);  // all inputs true

    // The boundary port controls the output correctly
    in3.setOutputValue(false);
    gate.updateLogic();
    QCOMPARE(gate.outputValue() == Status::Active, false);  // boundary input correctly propagated
}

/**
 * Test: An unconnected predecessor (nullptr) propagates Unknown through the gate.
 * A null input port means the circuit is incomplete, which is reflected as Unknown
 * on all outputs — regardless of the connected inputs' values.
 */
void TestLogicElementsErrors::testNullPredecessor()
{
    // 2-input AND: connect port 0, leave port 1 null
    LogicAnd andGate(2);
    LogicSource input;
    andGate.connectPredecessor(0, &input, 0);

    input.setOutputValue(true);
    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), Status::Unknown);  // null port 1 → Unknown

    input.setOutputValue(false);
    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), Status::Unknown);  // null port 1 → Unknown

    // 2-input OR: connect port 0, leave port 1 null
    LogicOr orGate(2);
    orGate.connectPredecessor(0, &input, 0);

    input.setOutputValue(true);
    orGate.updateLogic();
    QCOMPARE(orGate.outputValue(), Status::Unknown);  // null port 1 → Unknown

    input.setOutputValue(false);
    orGate.updateLogic();
    QCOMPARE(orGate.outputValue(), Status::Unknown);  // null port 1 → Unknown
}

/**
 * Test: Gate with all inputs never connected (all predecessors null).
 * All null predecessors produce Invalid outputs — the simulation cannot compute
 * a result from an incomplete circuit.
 */
void TestLogicElementsErrors::testDisconnectedInput()
{
    // AND with no predecessors connected → Unknown
    LogicAnd andGate(2);
    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), Status::Unknown);

    // OR with no predecessors connected → Unknown
    LogicOr orGate(2);
    orGate.updateLogic();
    QCOMPARE(orGate.outputValue(), Status::Unknown);

    // NOT with no predecessor connected → Unknown
    LogicNot notGate;
    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), Status::Unknown);
}

/**
 * Test: Calling connectPredecessor() twice on the same port overwrites the
 * first connection with the second. The first source is silently replaced.
 */
void TestLogicElementsErrors::testMultipleConnectionsSamePort()
{
    LogicAnd gate(2);
    LogicSource first, second, other;

    // Connect port 0 to 'first', then overwrite with 'second'
    gate.connectPredecessor(0, &first, 0);
    gate.connectPredecessor(1, &other, 0);
    gate.connectPredecessor(0, &second, 0);  // overwrites port 0

    first.setOutputValue(true);   // disconnected — no effect
    second.setOutputValue(false); // active on port 0
    other.setOutputValue(true);   // active on port 1

    gate.updateLogic();
    QCOMPARE(gate.outputValue() == Status::Active, false);  // AND(second=false, other=true) = false

    second.setOutputValue(true);
    gate.updateLogic();
    QCOMPARE(gate.outputValue() == Status::Active, true);   // AND(second=true, other=true) = true

    // Verify 'first' truly has no effect
    first.setOutputValue(false);
    gate.updateLogic();
    QCOMPARE(gate.outputValue() == Status::Active, true);   // still true; 'first' is not wired
}

/**
 * Test: Rapid state changes don't cause issues.
 * Verifies gate stability under rapid input toggling.
 */
void TestLogicElementsErrors::testRapidStateChanges()
{
    LogicAnd andGate(2);
    LogicSource input1, input2;

    andGate.connectPredecessor(0, &input1, 0);
    andGate.connectPredecessor(1, &input2, 0);

    for (int i = 0; i < 1000; ++i) {
        input1.setOutputValue(i % 2 == 0);
        input2.setOutputValue(i % 3 == 0);
        andGate.updateLogic();
        bool expected = (i % 2 == 0) && (i % 3 == 0);
        QCOMPARE(andGate.outputValue() == Status::Active, expected);
    }
}

/**
 * Test: A gate with null predecessors produces Unknown outputs automatically.
 * No explicit validate() call is needed — invalidity flows through the value
 * system: null predecessor → inputValue() returns Unknown → all outputs set Unknown.
 */
void TestLogicElementsErrors::testUnconnectedGate()
{
    LogicAnd gate(2);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), Status::Unknown);  // null predecessors → Unknown

    LogicNot notGate;
    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), Status::Unknown);  // null predecessor → Unknown
}

/**
 * Test: Signal propagates correctly through a deep cascade of AND gates.
 */
void TestLogicElementsErrors::testDeepCascading()
{
    // Chain: (in1 AND in2) → stage1 → AND in3 → stage2 → AND in4 → stage3
    LogicSource in1, in2, in3, in4;
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
    QCOMPARE(stage3.outputValue() == Status::Active, true);

    // One false at the start propagates through the full chain
    in1.setOutputValue(false);
    stage1.updateLogic();
    stage2.updateLogic();
    stage3.updateLogic();
    QCOMPARE(stage1.outputValue() == Status::Active, false);
    QCOMPARE(stage2.outputValue() == Status::Active, false);
    QCOMPARE(stage3.outputValue() == Status::Active, false);

    // Restore in1; inject false midway via in3
    in1.setOutputValue(true);
    in3.setOutputValue(false);
    stage1.updateLogic();
    stage2.updateLogic();
    stage3.updateLogic();
    QCOMPARE(stage1.outputValue() == Status::Active, true);
    QCOMPARE(stage2.outputValue() == Status::Active, false);  // false injected here
    QCOMPARE(stage3.outputValue() == Status::Active, false);
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
    QCOMPARE(andGate.outputValue(0) == Status::Active, false);
    QCOMPARE(orGate.outputValue(0) == Status::Active, false);
    QCOMPARE(notGate.outputValue(0) == Status::Active, false);

    // LogicSource supports configurable output counts
    // Constructor sets output 0 to defaultValue; remaining outputs start false
    LogicSource input(true, 4);  // 4 outputs: index 0 = true, indices 1-3 = false
    QCOMPARE(input.outputSize(), 4);
    QCOMPARE(input.outputValue(0) == Status::Active, true);
    QCOMPARE(input.outputValue(3) == Status::Active, false);  // highest valid index (outputSize() - 1)

    // Verify each output is independently settable
    input.setOutputValue(1, true);
    input.setOutputValue(3, true);
    QCOMPARE(input.outputValue(0) == Status::Active, true);
    QCOMPARE(input.outputValue(1) == Status::Active, true);
    QCOMPARE(input.outputValue(2) == Status::Active, false);
    QCOMPARE(input.outputValue(3) == Status::Active, true);
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

    // Initial output is Unknown (uninitialized 4-state logic).
    // NOT(Unknown) = Unknown, so the self-loop stays stuck without seeding.
    QCOMPARE(notGate.outputValue(), Status::Unknown);

    // Seed with a known value to break the Unknown fixed point.
    notGate.setOutputValue(0, Status::Inactive);

    // Each call reads the current (stale) output and inverts it
    notGate.updateLogic();
    QCOMPARE(notGate.outputValue() == Status::Active, true);   // NOT(false) = true

    notGate.updateLogic();
    QCOMPARE(notGate.outputValue() == Status::Active, false);  // NOT(true) = false

    notGate.updateLogic();
    QCOMPARE(notGate.outputValue() == Status::Active, true);   // stable single-step oscillation
}

/**
 * Test: Exhaustive truth table for a 4-input AND gate (all 16 combinations).
 * AND outputs true only when all four inputs are true (combo = 0b1111 = 15).
 */
void TestLogicElementsErrors::testInputValueBoundary()
{
    LogicAnd gate(4);
    LogicSource in0, in1, in2, in3;

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
        QCOMPARE(gate.outputValue() == Status::Active, combo == 15);
    }
}

/**
 * Test: Gates with 0 inputs return their identity element.
 * std::all_of / std::any_of over an empty range:
 *   AND(∅) = Active   (all_of over {} = true)
 *   OR(∅)  = Inactive (any_of over {} = false)
 */
void TestLogicElementsErrors::testGateWithZeroInputs()
{
    LogicAnd andGate(0);
    andGate.updateLogic();
    QCOMPARE(andGate.outputValue() == Status::Active, true);  // identity of AND

    LogicOr orGate(0);
    orGate.updateLogic();
    QCOMPARE(orGate.outputValue() == Status::Active, false);  // identity of OR
}

/**
 * Test: Status::Unknown propagates downstream through a chain of gates.
 * A gate with a null input produces Unknown. A downstream gate reading
 * that Invalid output must also produce Invalid — no gate should silently
 * convert Unknown into Active or Inactive.
 */
void TestLogicElementsErrors::testInvalidPropagatesChain()
{
    // AND with one null input → outputs Unknown
    LogicAnd andGate(2);
    LogicNot notGate;
    LogicSource input;

    andGate.connectPredecessor(0, &input, 0);
    // port 1 is left null → AND will produce Unknown
    notGate.connectPredecessor(0, &andGate, 0);

    input.setOutputValue(true);
    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), Status::Unknown);  // null port → Unknown

    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), Status::Unknown);  // Unknown propagates downstream

    // Confirm it's not specific to true input: same with false
    input.setOutputValue(false);
    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), Status::Unknown);

    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), Status::Unknown);
}

/**
 * Test: setOutputValue(bool) convenience overload maps correctly to Status.
 * true  → Status::Active
 * false → Status::Inactive
 * Neither produces Status::Unknown — this overload is the UI→simulation
 * boundary where a switch is inherently on or off, never invalid.
 */
void TestLogicElementsErrors::testBoolOverloadMapsToStatus()
{
    // Default-output overload
    LogicSource src(false, 2);

    src.setOutputValue(true);
    QCOMPARE(src.outputValue(0), Status::Active);

    src.setOutputValue(false);
    QCOMPARE(src.outputValue(0), Status::Inactive);

    // Indexed overload
    src.setOutputValue(1, true);
    QCOMPARE(src.outputValue(1), Status::Active);

    src.setOutputValue(1, false);
    QCOMPARE(src.outputValue(1), Status::Inactive);

    // Neither overload can produce Unknown
    QVERIFY(src.outputValue(0) != Status::Unknown);
    QVERIFY(src.outputValue(1) != Status::Unknown);
}
