// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Logic/TestStatusOps.h"

#include <QTest>

#include "App/Core/StatusOps.h"
#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Demux.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/DLatch.h"
#include "App/Element/GraphicElements/InputVCC.h"
#include "App/Element/GraphicElements/JKFlipFlop.h"
#include "App/Element/GraphicElements/Mux.h"
#include "App/Element/GraphicElements/Nand.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/Nor.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/GraphicElements/SRFlipFlop.h"
#include "App/Element/GraphicElements/SRLatch.h"
#include "App/Element/GraphicElements/TFlipFlop.h"
#include "App/Element/GraphicElements/TruthTable.h"
#include "App/Element/GraphicElements/Xnor.h"
#include "App/Element/GraphicElements/Xor.h"
#include "Tests/Common/TestUtils.h"

using S = Status;

using TestUtils::initSrc;
using TestUtils::initElm;

// ============================================================
// StatusOps binary function tests — exhaustive 4x4 truth tables
// ============================================================

void TestStatusOps::testStatusAnd_data()
{
    QTest::addColumn<Status>("a");
    QTest::addColumn<Status>("b");
    QTest::addColumn<Status>("expected");

    // Inactive dominates everything
    QTest::newRow("0 AND 0") << S::Inactive << S::Inactive << S::Inactive;
    QTest::newRow("0 AND 1") << S::Inactive << S::Active   << S::Inactive;
    QTest::newRow("1 AND 0") << S::Active   << S::Inactive << S::Inactive;
    QTest::newRow("0 AND U") << S::Inactive << S::Unknown  << S::Inactive;
    QTest::newRow("U AND 0") << S::Unknown  << S::Inactive << S::Inactive;
    QTest::newRow("0 AND X") << S::Inactive << S::Error    << S::Inactive;
    QTest::newRow("X AND 0") << S::Error    << S::Inactive << S::Inactive;

    // Active only when both Active
    QTest::newRow("1 AND 1") << S::Active   << S::Active   << S::Active;

    // Unknown propagation
    QTest::newRow("1 AND U") << S::Active   << S::Unknown  << S::Unknown;
    QTest::newRow("U AND 1") << S::Unknown  << S::Active   << S::Unknown;
    QTest::newRow("U AND U") << S::Unknown  << S::Unknown  << S::Unknown;

    // Error propagation (Error > Unknown)
    QTest::newRow("1 AND X") << S::Active   << S::Error    << S::Error;
    QTest::newRow("X AND 1") << S::Error    << S::Active   << S::Error;
    QTest::newRow("X AND U") << S::Error    << S::Unknown  << S::Error;
    QTest::newRow("U AND X") << S::Unknown  << S::Error    << S::Error;
    QTest::newRow("X AND X") << S::Error    << S::Error    << S::Error;
}

void TestStatusOps::testStatusAnd()
{
    QFETCH(Status, a);
    QFETCH(Status, b);
    QFETCH(Status, expected);
    QCOMPARE(StatusOps::statusAnd(a, b), expected);
}

void TestStatusOps::testStatusOr_data()
{
    QTest::addColumn<Status>("a");
    QTest::addColumn<Status>("b");
    QTest::addColumn<Status>("expected");

    // Active dominates everything
    QTest::newRow("1 OR 1") << S::Active   << S::Active   << S::Active;
    QTest::newRow("1 OR 0") << S::Active   << S::Inactive << S::Active;
    QTest::newRow("0 OR 1") << S::Inactive << S::Active   << S::Active;
    QTest::newRow("1 OR U") << S::Active   << S::Unknown  << S::Active;
    QTest::newRow("U OR 1") << S::Unknown  << S::Active   << S::Active;
    QTest::newRow("1 OR X") << S::Active   << S::Error    << S::Active;
    QTest::newRow("X OR 1") << S::Error    << S::Active   << S::Active;

    // Inactive only when both Inactive
    QTest::newRow("0 OR 0") << S::Inactive << S::Inactive << S::Inactive;

    // Unknown propagation
    QTest::newRow("0 OR U") << S::Inactive << S::Unknown  << S::Unknown;
    QTest::newRow("U OR 0") << S::Unknown  << S::Inactive << S::Unknown;
    QTest::newRow("U OR U") << S::Unknown  << S::Unknown  << S::Unknown;

    // Error propagation
    QTest::newRow("0 OR X") << S::Inactive << S::Error    << S::Error;
    QTest::newRow("X OR 0") << S::Error    << S::Inactive << S::Error;
    QTest::newRow("X OR U") << S::Error    << S::Unknown  << S::Error;
    QTest::newRow("U OR X") << S::Unknown  << S::Error    << S::Error;
    QTest::newRow("X OR X") << S::Error    << S::Error    << S::Error;
}

void TestStatusOps::testStatusOr()
{
    QFETCH(Status, a);
    QFETCH(Status, b);
    QFETCH(Status, expected);
    QCOMPARE(StatusOps::statusOr(a, b), expected);
}

void TestStatusOps::testStatusNot_data()
{
    QTest::addColumn<Status>("input");
    QTest::addColumn<Status>("expected");

    QTest::newRow("NOT 0") << S::Inactive << S::Active;
    QTest::newRow("NOT 1") << S::Active   << S::Inactive;
    QTest::newRow("NOT U") << S::Unknown  << S::Unknown;
    QTest::newRow("NOT X") << S::Error    << S::Error;
}

void TestStatusOps::testStatusNot()
{
    QFETCH(Status, input);
    QFETCH(Status, expected);
    QCOMPARE(StatusOps::statusNot(input), expected);
}

void TestStatusOps::testStatusXor_data()
{
    QTest::addColumn<Status>("a");
    QTest::addColumn<Status>("b");
    QTest::addColumn<Status>("expected");

    // Known inputs — standard XOR
    QTest::newRow("0 XOR 0") << S::Inactive << S::Inactive << S::Inactive;
    QTest::newRow("0 XOR 1") << S::Inactive << S::Active   << S::Active;
    QTest::newRow("1 XOR 0") << S::Active   << S::Inactive << S::Active;
    QTest::newRow("1 XOR 1") << S::Active   << S::Active   << S::Inactive;

    // XOR requires all known — any Unknown → Unknown
    QTest::newRow("U XOR 0") << S::Unknown  << S::Inactive << S::Unknown;
    QTest::newRow("0 XOR U") << S::Inactive << S::Unknown  << S::Unknown;
    QTest::newRow("U XOR 1") << S::Unknown  << S::Active   << S::Unknown;
    QTest::newRow("1 XOR U") << S::Active   << S::Unknown  << S::Unknown;
    QTest::newRow("U XOR U") << S::Unknown  << S::Unknown  << S::Unknown;

    // Error takes priority over Unknown
    QTest::newRow("X XOR 0") << S::Error    << S::Inactive << S::Error;
    QTest::newRow("0 XOR X") << S::Inactive << S::Error    << S::Error;
    QTest::newRow("X XOR 1") << S::Error    << S::Active   << S::Error;
    QTest::newRow("1 XOR X") << S::Active   << S::Error    << S::Error;
    QTest::newRow("X XOR U") << S::Error    << S::Unknown  << S::Error;
    QTest::newRow("U XOR X") << S::Unknown  << S::Error    << S::Error;
    QTest::newRow("X XOR X") << S::Error    << S::Error    << S::Error;
}

void TestStatusOps::testStatusXor()
{
    QFETCH(Status, a);
    QFETCH(Status, b);
    QFETCH(Status, expected);
    QCOMPARE(StatusOps::statusXor(a, b), expected);
}

// ============================================================
// StatusOps fold (multi-input) function tests
// ============================================================

void TestStatusOps::testStatusAndAll()
{
    // All Active → Active
    QCOMPARE(StatusOps::statusAndAll({S::Active, S::Active, S::Active}), S::Active);

    // One Inactive dominates
    QCOMPARE(StatusOps::statusAndAll({S::Active, S::Inactive, S::Active}), S::Inactive);

    // Inactive dominates even with Unknown/Error
    QCOMPARE(StatusOps::statusAndAll({S::Unknown, S::Inactive, S::Error}), S::Inactive);

    // Unknown without Inactive → Unknown
    QCOMPARE(StatusOps::statusAndAll({S::Active, S::Unknown, S::Active}), S::Unknown);

    // Error without Inactive → Error (Error > Unknown)
    QCOMPARE(StatusOps::statusAndAll({S::Active, S::Error, S::Active}), S::Error);

    // Error before Unknown in scan order
    QCOMPARE(StatusOps::statusAndAll({S::Active, S::Error, S::Unknown}), S::Error);

    // Single element
    QCOMPARE(StatusOps::statusAndAll({S::Active}), S::Active);
    QCOMPARE(StatusOps::statusAndAll({S::Inactive}), S::Inactive);
    QCOMPARE(StatusOps::statusAndAll({S::Unknown}), S::Unknown);
    QCOMPARE(StatusOps::statusAndAll({S::Error}), S::Error);
}

void TestStatusOps::testStatusOrAll()
{
    // All Inactive → Inactive
    QCOMPARE(StatusOps::statusOrAll({S::Inactive, S::Inactive, S::Inactive}), S::Inactive);

    // One Active dominates
    QCOMPARE(StatusOps::statusOrAll({S::Inactive, S::Active, S::Inactive}), S::Active);

    // Active dominates even with Unknown/Error
    QCOMPARE(StatusOps::statusOrAll({S::Unknown, S::Active, S::Error}), S::Active);

    // Unknown without Active → Unknown
    QCOMPARE(StatusOps::statusOrAll({S::Inactive, S::Unknown, S::Inactive}), S::Unknown);

    // Error without Active → Error
    QCOMPARE(StatusOps::statusOrAll({S::Inactive, S::Error, S::Inactive}), S::Error);

    // Error before Unknown in scan order
    QCOMPARE(StatusOps::statusOrAll({S::Inactive, S::Error, S::Unknown}), S::Error);

    // Single element
    QCOMPARE(StatusOps::statusOrAll({S::Active}), S::Active);
    QCOMPARE(StatusOps::statusOrAll({S::Inactive}), S::Inactive);
    QCOMPARE(StatusOps::statusOrAll({S::Unknown}), S::Unknown);
    QCOMPARE(StatusOps::statusOrAll({S::Error}), S::Error);
}

void TestStatusOps::testStatusXorAll()
{
    // Standard parity
    QCOMPARE(StatusOps::statusXorAll({S::Active, S::Inactive}), S::Active);
    QCOMPARE(StatusOps::statusXorAll({S::Active, S::Active}), S::Inactive);
    QCOMPARE(StatusOps::statusXorAll({S::Active, S::Active, S::Active}), S::Active);
    QCOMPARE(StatusOps::statusXorAll({S::Inactive, S::Inactive}), S::Inactive);

    // Unknown propagates (XOR needs all known)
    QCOMPARE(StatusOps::statusXorAll({S::Active, S::Unknown}), S::Unknown);
    QCOMPARE(StatusOps::statusXorAll({S::Inactive, S::Unknown}), S::Unknown);

    // Error takes priority over Unknown (consistent with AND/OR folds)
    QCOMPARE(StatusOps::statusXorAll({S::Active, S::Error}), S::Error);
    QCOMPARE(StatusOps::statusXorAll({S::Unknown, S::Error}), S::Error);
    QCOMPARE(StatusOps::statusXorAll({S::Error, S::Unknown}), S::Error);

    // Single element
    QCOMPARE(StatusOps::statusXorAll({S::Active}), S::Active);
    QCOMPARE(StatusOps::statusXorAll({S::Inactive}), S::Inactive);
    QCOMPARE(StatusOps::statusXorAll({S::Unknown}), S::Unknown);
    QCOMPARE(StatusOps::statusXorAll({S::Error}), S::Error);
}

// ============================================================
// Gate-level 4-state integration tests (using GraphicElement API)
// ============================================================

void TestStatusOps::testAndGateUnknownDomination()
{
    And gate; InputVcc in0, in1;
    initElm(gate); initSrc(in0); initSrc(in1);
    gate.connectPredecessor(0, &in0, 0);
    gate.connectPredecessor(1, &in1, 0);

    // AND(0, U) = 0 — Inactive dominates
    in0.setOutputValue(S::Inactive);
    in1.setOutputValue(S::Unknown);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Inactive);

    // AND(1, U) = U — can't determine
    in0.setOutputValue(S::Active);
    in1.setOutputValue(S::Unknown);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Unknown);

    // AND(0, X) = 0 — Inactive dominates Error
    in0.setOutputValue(S::Inactive);
    in1.setOutputValue(S::Error);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Inactive);

    // AND(1, X) = X — Error propagates
    in0.setOutputValue(S::Active);
    in1.setOutputValue(S::Error);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Error);
}

void TestStatusOps::testOrGateUnknownDomination()
{
    Or gate; InputVcc in0, in1;
    initElm(gate); initSrc(in0); initSrc(in1);
    gate.connectPredecessor(0, &in0, 0);
    gate.connectPredecessor(1, &in1, 0);

    // OR(1, U) = 1 — Active dominates
    in0.setOutputValue(S::Active);
    in1.setOutputValue(S::Unknown);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Active);

    // OR(0, U) = U — can't determine
    in0.setOutputValue(S::Inactive);
    in1.setOutputValue(S::Unknown);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Unknown);

    // OR(1, X) = 1 — Active dominates Error
    in0.setOutputValue(S::Active);
    in1.setOutputValue(S::Error);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Active);

    // OR(0, X) = X — Error propagates
    in0.setOutputValue(S::Inactive);
    in1.setOutputValue(S::Error);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Error);
}

void TestStatusOps::testNotGateUnknownPassthrough()
{
    Not gate; InputVcc in0;
    initElm(gate); initSrc(in0);
    gate.connectPredecessor(0, &in0, 0);

    in0.setOutputValue(S::Unknown);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Unknown);

    in0.setOutputValue(S::Error);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Error);

    // Verify normal operation still works
    in0.setOutputValue(S::Active);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Inactive);

    in0.setOutputValue(S::Inactive);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Active);
}

void TestStatusOps::testNandGateUnknownDomination()
{
    Nand gate; InputVcc in0, in1;
    initElm(gate); initSrc(in0); initSrc(in1);
    gate.connectPredecessor(0, &in0, 0);
    gate.connectPredecessor(1, &in1, 0);

    // NAND(0, U) = NOT(AND(0, U)) = NOT(0) = 1
    in0.setOutputValue(S::Inactive);
    in1.setOutputValue(S::Unknown);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Active);

    // NAND(1, U) = NOT(AND(1, U)) = NOT(U) = U
    in0.setOutputValue(S::Active);
    in1.setOutputValue(S::Unknown);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Unknown);

    // NAND(1, X) = NOT(X) = X
    in0.setOutputValue(S::Active);
    in1.setOutputValue(S::Error);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Error);
}

void TestStatusOps::testNorGateUnknownDomination()
{
    Nor gate; InputVcc in0, in1;
    initElm(gate); initSrc(in0); initSrc(in1);
    gate.connectPredecessor(0, &in0, 0);
    gate.connectPredecessor(1, &in1, 0);

    // NOR(1, U) = NOT(OR(1, U)) = NOT(1) = 0
    in0.setOutputValue(S::Active);
    in1.setOutputValue(S::Unknown);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Inactive);

    // NOR(0, U) = NOT(OR(0, U)) = NOT(U) = U
    in0.setOutputValue(S::Inactive);
    in1.setOutputValue(S::Unknown);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Unknown);

    // NOR(0, X) = NOT(X) = X
    in0.setOutputValue(S::Inactive);
    in1.setOutputValue(S::Error);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Error);
}

void TestStatusOps::testXorGateUnknownPropagation()
{
    Xor gate; InputVcc in0, in1;
    initElm(gate); initSrc(in0); initSrc(in1);
    gate.connectPredecessor(0, &in0, 0);
    gate.connectPredecessor(1, &in1, 0);

    // XOR needs all known — any Unknown → Unknown
    in0.setOutputValue(S::Active);
    in1.setOutputValue(S::Unknown);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Unknown);

    in0.setOutputValue(S::Inactive);
    in1.setOutputValue(S::Unknown);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Unknown);

    // Error takes priority
    in0.setOutputValue(S::Active);
    in1.setOutputValue(S::Error);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Error);
}

void TestStatusOps::testXnorGateUnknownPropagation()
{
    Xnor gate; InputVcc in0, in1;
    initElm(gate); initSrc(in0); initSrc(in1);
    gate.connectPredecessor(0, &in0, 0);
    gate.connectPredecessor(1, &in1, 0);

    // XNOR = NOT(XOR) — Unknown still propagates
    in0.setOutputValue(S::Active);
    in1.setOutputValue(S::Unknown);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Unknown);

    in0.setOutputValue(S::Active);
    in1.setOutputValue(S::Error);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Error);

    // Verify normal operation
    in0.setOutputValue(S::Active);
    in1.setOutputValue(S::Active);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Active);  // XNOR(1,1) = 1
}

void TestStatusOps::testErrorPropagationThroughChain()
{
    // Chain: Source → NOT → AND → output
    InputVcc src, other;
    Not notGate;
    And andGate;
    initSrc(src); initSrc(other);
    initElm(notGate); initElm(andGate);

    notGate.connectPredecessor(0, &src, 0);
    andGate.connectPredecessor(0, &notGate, 0);
    andGate.connectPredecessor(1, &other, 0);

    src.setOutputValue(S::Error);
    other.setOutputValue(S::Active);

    notGate.updateLogic();
    QCOMPARE(notGate.outputValue(), S::Error);  // NOT(X) = X

    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), S::Error);  // AND(X, 1) = X

    // Now test domination: AND(X, 0) = 0
    other.setOutputValue(S::Inactive);
    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), S::Inactive);  // Inactive dominates Error
}

void TestStatusOps::testMuxUnknownSelectLine()
{
    Mux mux;  // Default 3 inputs: data0, data1, select
    InputVcc data0, data1, sel;
    initElm(mux); initSrc(data0); initSrc(data1); initSrc(sel);
    mux.connectPredecessor(0, &data0, 0);
    mux.connectPredecessor(1, &data1, 0);
    mux.connectPredecessor(2, &sel, 0);

    // Normal operation
    data0.setOutputValue(S::Active);
    data1.setOutputValue(S::Inactive);
    sel.setOutputValue(S::Inactive);  // Select data0
    mux.updateLogic();
    QCOMPARE(mux.outputValue(), S::Active);

    // Unknown select → Unknown output
    sel.setOutputValue(S::Unknown);
    mux.updateLogic();
    QCOMPARE(mux.outputValue(), S::Unknown);

    // Error select → Error output
    sel.setOutputValue(S::Error);
    mux.updateLogic();
    QCOMPARE(mux.outputValue(), S::Error);
}

void TestStatusOps::testDemuxUnknownSelectLine()
{
    Demux demux;  // Default 2 outputs: out0, out1
    InputVcc data, sel;
    initElm(demux); initSrc(data); initSrc(sel);
    demux.connectPredecessor(0, &data, 0);
    demux.connectPredecessor(1, &sel, 0);

    // Normal operation
    data.setOutputValue(S::Active);
    sel.setOutputValue(S::Inactive);  // Route to output 0
    demux.updateLogic();
    QCOMPARE(demux.outputValue(0), S::Active);
    QCOMPARE(demux.outputValue(1), S::Inactive);

    // Unknown select → all outputs Unknown
    sel.setOutputValue(S::Unknown);
    demux.updateLogic();
    QCOMPARE(demux.outputValue(0), S::Unknown);
    QCOMPARE(demux.outputValue(1), S::Unknown);

    // Error select → all outputs Error
    sel.setOutputValue(S::Error);
    demux.updateLogic();
    QCOMPARE(demux.outputValue(0), S::Error);
    QCOMPARE(demux.outputValue(1), S::Error);
}

void TestStatusOps::testMuxUnknownDataInput()
{
    Mux mux;
    InputVcc data0, data1, sel;
    initElm(mux); initSrc(data0); initSrc(data1); initSrc(sel);
    mux.connectPredecessor(0, &data0, 0);
    mux.connectPredecessor(1, &data1, 0);
    mux.connectPredecessor(2, &sel, 0);

    // Select data0, which is Unknown → output Unknown (pass-through)
    data0.setOutputValue(S::Unknown);
    data1.setOutputValue(S::Active);
    sel.setOutputValue(S::Inactive);  // Select data0
    mux.updateLogic();
    QCOMPARE(mux.outputValue(), S::Unknown);

    // Select data1, which is Active → output Active (data0 Unknown irrelevant)
    sel.setOutputValue(S::Active);
    mux.updateLogic();
    QCOMPARE(mux.outputValue(), S::Active);

    // Select data0, which is Error → output Error
    data0.setOutputValue(S::Error);
    sel.setOutputValue(S::Inactive);
    mux.updateLogic();
    QCOMPARE(mux.outputValue(), S::Error);
}

void TestStatusOps::testDemuxUnknownDataInput()
{
    Demux demux;
    InputVcc data, sel;
    initElm(demux); initSrc(data); initSrc(sel);
    demux.connectPredecessor(0, &data, 0);
    demux.connectPredecessor(1, &sel, 0);

    // Unknown data routed to output 0
    data.setOutputValue(S::Unknown);
    sel.setOutputValue(S::Inactive);
    demux.updateLogic();
    QCOMPARE(demux.outputValue(0), S::Unknown);
    QCOMPARE(demux.outputValue(1), S::Inactive);  // Non-selected stays Inactive

    // Error data routed to output 1
    data.setOutputValue(S::Error);
    sel.setOutputValue(S::Active);
    demux.updateLogic();
    QCOMPARE(demux.outputValue(0), S::Inactive);
    QCOMPARE(demux.outputValue(1), S::Error);
}

void TestStatusOps::testTruthTableUnknownInput()
{
    // 2-input, 1-output truth table implementing AND
    TruthTable tt;
    tt.setInputSize(2);
    tt.setOutputSize(1);
    QBitArray key(256, false);
    key.setBit(3);  // Row 11 (both inputs Active) → output Active
    tt.setkey(key);
    InputVcc in0, in1;
    initElm(tt); initSrc(in0); initSrc(in1);
    tt.connectPredecessor(0, &in0, 0);
    tt.connectPredecessor(1, &in1, 0);

    // Normal operation
    in0.setOutputValue(S::Active);
    in1.setOutputValue(S::Active);
    tt.updateLogic();
    QCOMPARE(tt.outputValue(), S::Active);

    // Unknown input → Unknown output (can't determine row)
    in0.setOutputValue(S::Unknown);
    tt.updateLogic();
    QCOMPARE(tt.outputValue(), S::Unknown);

    // Error input → Error output
    in0.setOutputValue(S::Error);
    tt.updateLogic();
    QCOMPARE(tt.outputValue(), S::Error);
}

void TestStatusOps::testNodePassesUnknownAndError()
{
    Node node; InputVcc src;
    initElm(node); initSrc(src);
    node.connectPredecessor(0, &src, 0);

    src.setOutputValue(S::Unknown);
    node.updateLogic();
    QCOMPARE(node.outputValue(), S::Unknown);

    src.setOutputValue(S::Error);
    node.updateLogic();
    QCOMPARE(node.outputValue(), S::Error);

    src.setOutputValue(S::Active);
    node.updateLogic();
    QCOMPARE(node.outputValue(), S::Active);
}

void TestStatusOps::test3InputAndWithUnknown()
{
    And gate;
    gate.setInputSize(3);
    InputVcc in0, in1, in2;
    initElm(gate); initSrc(in0); initSrc(in1); initSrc(in2);
    gate.connectPredecessor(0, &in0, 0);
    gate.connectPredecessor(1, &in1, 0);
    gate.connectPredecessor(2, &in2, 0);

    // AND(1, U, 1) = U — can't determine without the middle input
    in0.setOutputValue(S::Active);
    in1.setOutputValue(S::Unknown);
    in2.setOutputValue(S::Active);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Unknown);

    // AND(1, U, 0) = 0 — Inactive dominates
    in2.setOutputValue(S::Inactive);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Inactive);

    // AND(1, X, 0) = 0 — Inactive dominates Error
    in1.setOutputValue(S::Error);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Inactive);

    // AND(1, X, 1) = X — Error propagates
    in2.setOutputValue(S::Active);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Error);
}

void TestStatusOps::test3InputOrWithUnknown()
{
    Or gate;
    gate.setInputSize(3);
    InputVcc in0, in1, in2;
    initElm(gate); initSrc(in0); initSrc(in1); initSrc(in2);
    gate.connectPredecessor(0, &in0, 0);
    gate.connectPredecessor(1, &in1, 0);
    gate.connectPredecessor(2, &in2, 0);

    // OR(0, U, 0) = U — can't determine
    in0.setOutputValue(S::Inactive);
    in1.setOutputValue(S::Unknown);
    in2.setOutputValue(S::Inactive);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Unknown);

    // OR(0, U, 1) = 1 — Active dominates
    in2.setOutputValue(S::Active);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Active);

    // OR(0, X, 1) = 1 — Active dominates Error
    in1.setOutputValue(S::Error);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Active);

    // OR(0, X, 0) = X — Error propagates
    in2.setOutputValue(S::Inactive);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Error);
}

// ============================================================
// Sequential element 4-state behavior
// ============================================================

void TestStatusOps::testDFlipFlopUnknownInputHolds()
{
    DFlipFlop dff; InputVcc d, clk, prst, clr;
    initElm(dff); initSrc(d); initSrc(clk); initSrc(prst); initSrc(clr);
    dff.connectPredecessor(0, &d, 0);
    dff.connectPredecessor(1, &clk, 0);
    dff.connectPredecessor(2, &prst, 0);
    dff.connectPredecessor(3, &clr, 0);

    prst.setOutputValue(S::Active);  // Deasserted
    clr.setOutputValue(S::Active);   // Deasserted
    d.setOutputValue(S::Unknown);
    clk.setOutputValue(S::Inactive);

    dff.updateLogic();
    // Unknown propagates via simUpdateInputs
    QCOMPARE(dff.outputValue(0), S::Unknown);
    QCOMPARE(dff.outputValue(1), S::Unknown);
}

void TestStatusOps::testDFlipFlopUnknownClockNoEdge()
{
    DFlipFlop dff; InputVcc d, clk, prst, clr;
    initElm(dff); initSrc(d); initSrc(clk); initSrc(prst); initSrc(clr);
    dff.connectPredecessor(0, &d, 0);
    dff.connectPredecessor(1, &clk, 0);
    dff.connectPredecessor(2, &prst, 0);
    dff.connectPredecessor(3, &clr, 0);

    prst.setOutputValue(S::Active);
    clr.setOutputValue(S::Active);
    d.setOutputValue(S::Inactive);
    clk.setOutputValue(S::Inactive);

    // First tick: establish known state
    dff.updateLogic();
    QCOMPARE(dff.outputValue(0), S::Inactive);  // Power-on Q=0

    // Rising edge: latch m_lastValue=Inactive
    clk.setOutputValue(S::Active);
    dff.updateLogic();
    QCOMPARE(dff.outputValue(0), S::Inactive);  // Latched Inactive

    // Return CLK to Inactive, set D=Active
    clk.setOutputValue(S::Inactive);
    dff.updateLogic();

    d.setOutputValue(S::Active);
    dff.updateLogic();

    clk.setOutputValue(S::Active);
    dff.updateLogic();
    QCOMPARE(dff.outputValue(0), S::Active);  // Latched D=Active
}

void TestStatusOps::testJKFlipFlopUnknownPresetClearNoTrigger()
{
    JKFlipFlop jkff; InputVcc j, clk, k, prst, clr;
    initElm(jkff); initSrc(j); initSrc(clk); initSrc(k);
    initSrc(prst); initSrc(clr);
    jkff.connectPredecessor(0, &j, 0);
    jkff.connectPredecessor(1, &clk, 0);
    jkff.connectPredecessor(2, &k, 0);
    jkff.connectPredecessor(3, &prst, 0);
    jkff.connectPredecessor(4, &clr, 0);

    j.setOutputValue(S::Inactive);
    clk.setOutputValue(S::Inactive);
    k.setOutputValue(S::Inactive);
    prst.setOutputValue(S::Active);
    clr.setOutputValue(S::Active);

    jkff.updateLogic();
    QCOMPARE(jkff.outputValue(0), S::Inactive);
    QCOMPARE(jkff.outputValue(1), S::Active);

    // Preset to Unknown — should NOT trigger override
    prst.setOutputValue(S::Unknown);
    jkff.updateLogic();
    QCOMPARE(jkff.outputValue(0), S::Unknown);
    QCOMPARE(jkff.outputValue(1), S::Unknown);

    // Restore Preset — outputs remain Unknown without clock edge
    prst.setOutputValue(S::Active);
    jkff.updateLogic();
    QCOMPARE(jkff.outputValue(0), S::Unknown);
    QCOMPARE(jkff.outputValue(1), S::Unknown);

    // Explicit clear restores known state
    clr.setOutputValue(S::Inactive);
    jkff.updateLogic();
    QCOMPARE(jkff.outputValue(0), S::Inactive);
    QCOMPARE(jkff.outputValue(1), S::Active);
}

void TestStatusOps::testTFlipFlopUnknownInput()
{
    TFlipFlop tff; InputVcc t, clk, prst, clr;
    initElm(tff); initSrc(t); initSrc(clk); initSrc(prst); initSrc(clr);
    tff.connectPredecessor(0, &t, 0);
    tff.connectPredecessor(1, &clk, 0);
    tff.connectPredecessor(2, &prst, 0);
    tff.connectPredecessor(3, &clr, 0);

    prst.setOutputValue(S::Active);
    clr.setOutputValue(S::Active);
    t.setOutputValue(S::Inactive);
    clk.setOutputValue(S::Inactive);

    tff.updateLogic();
    QCOMPARE(tff.outputValue(0), S::Inactive);
    QCOMPARE(tff.outputValue(1), S::Active);

    // Unknown on T input → blocks
    t.setOutputValue(S::Unknown);
    tff.updateLogic();
    QCOMPARE(tff.outputValue(0), S::Unknown);
    QCOMPARE(tff.outputValue(1), S::Unknown);

    // Error on T input → also blocks
    t.setOutputValue(S::Error);
    tff.updateLogic();
    QCOMPARE(tff.outputValue(0), S::Unknown);
    QCOMPARE(tff.outputValue(1), S::Unknown);
}

void TestStatusOps::testSRFlipFlopUnknownPresetClear()
{
    SRFlipFlop srff; InputVcc s, clk, r, prst, clr;
    initElm(srff); initSrc(s); initSrc(clk); initSrc(r);
    initSrc(prst); initSrc(clr);
    srff.connectPredecessor(0, &s, 0);
    srff.connectPredecessor(1, &clk, 0);
    srff.connectPredecessor(2, &r, 0);
    srff.connectPredecessor(3, &prst, 0);
    srff.connectPredecessor(4, &clr, 0);

    s.setOutputValue(S::Inactive);
    clk.setOutputValue(S::Inactive);
    r.setOutputValue(S::Inactive);
    prst.setOutputValue(S::Active);
    clr.setOutputValue(S::Active);

    srff.updateLogic();
    QCOMPARE(srff.outputValue(0), S::Inactive);
    QCOMPARE(srff.outputValue(1), S::Active);

    // Unknown on Preset → blocks
    prst.setOutputValue(S::Unknown);
    srff.updateLogic();
    QCOMPARE(srff.outputValue(0), S::Unknown);
    QCOMPARE(srff.outputValue(1), S::Unknown);

    // Restore Preset, explicitly clear
    prst.setOutputValue(S::Active);
    clr.setOutputValue(S::Inactive);
    srff.updateLogic();
    QCOMPARE(srff.outputValue(0), S::Inactive);
    QCOMPARE(srff.outputValue(1), S::Active);
}

void TestStatusOps::testDLatchUnknownInput()
{
    DLatch latch; InputVcc d, en;
    initElm(latch); initSrc(d); initSrc(en);
    latch.connectPredecessor(0, &d, 0);
    latch.connectPredecessor(1, &en, 0);

    // Unknown on D → blocks
    d.setOutputValue(S::Unknown);
    en.setOutputValue(S::Active);
    latch.updateLogic();
    QCOMPARE(latch.outputValue(0), S::Unknown);
    QCOMPARE(latch.outputValue(1), S::Unknown);

    // Known inputs: D=Active, Enable=Active → transparent latch
    d.setOutputValue(S::Active);
    latch.updateLogic();
    QCOMPARE(latch.outputValue(0), S::Active);
    QCOMPARE(latch.outputValue(1), S::Inactive);

    // Error on Enable → blocks
    en.setOutputValue(S::Error);
    latch.updateLogic();
    QCOMPARE(latch.outputValue(0), S::Unknown);
    QCOMPARE(latch.outputValue(1), S::Unknown);
}

void TestStatusOps::testSRLatchUnknownInput()
{
    SRLatch latch; InputVcc s, r;
    initElm(latch); initSrc(s); initSrc(r);
    latch.connectPredecessor(0, &s, 0);
    latch.connectPredecessor(1, &r, 0);

    // Unknown on S input → blocks
    s.setOutputValue(S::Unknown);
    r.setOutputValue(S::Inactive);
    latch.updateLogic();
    QCOMPARE(latch.outputValue(0), S::Unknown);
    QCOMPARE(latch.outputValue(1), S::Unknown);

    // Restore known inputs — latch should compute
    s.setOutputValue(S::Active);
    r.setOutputValue(S::Inactive);
    latch.updateLogic();
    QCOMPARE(latch.outputValue(0), S::Active);
    QCOMPARE(latch.outputValue(1), S::Inactive);
}

// ============================================================
// simUpdateInputs vs simUpdateInputsAllowUnknown
// ============================================================

void TestStatusOps::testUpdateInputsBlocksUnknown()
{
    DFlipFlop dff; InputVcc d, clk, prst, clr;
    initElm(dff); initSrc(d); initSrc(clk); initSrc(prst); initSrc(clr);
    dff.connectPredecessor(0, &d, 0);
    dff.connectPredecessor(1, &clk, 0);
    dff.connectPredecessor(2, &prst, 0);
    dff.connectPredecessor(3, &clr, 0);

    d.setOutputValue(S::Unknown);
    clk.setOutputValue(S::Inactive);
    prst.setOutputValue(S::Active);
    clr.setOutputValue(S::Active);

    dff.updateLogic();
    QCOMPARE(dff.outputValue(0), S::Unknown);
    QCOMPARE(dff.outputValue(1), S::Unknown);
}

void TestStatusOps::testUpdateInputsBlocksError()
{
    DFlipFlop dff; InputVcc d, clk, prst, clr;
    initElm(dff); initSrc(d); initSrc(clk); initSrc(prst); initSrc(clr);
    dff.connectPredecessor(0, &d, 0);
    dff.connectPredecessor(1, &clk, 0);
    dff.connectPredecessor(2, &prst, 0);
    dff.connectPredecessor(3, &clr, 0);

    d.setOutputValue(S::Error);
    clk.setOutputValue(S::Inactive);
    prst.setOutputValue(S::Active);
    clr.setOutputValue(S::Active);

    dff.updateLogic();
    QCOMPARE(dff.outputValue(0), S::Unknown);  // simUpdateInputs forces Unknown on Error
    QCOMPARE(dff.outputValue(1), S::Unknown);
}

void TestStatusOps::testUpdateInputsAllowUnknownPassesThrough()
{
    And gate; InputVcc in0, in1;
    initElm(gate); initSrc(in0); initSrc(in1);
    gate.connectPredecessor(0, &in0, 0);
    gate.connectPredecessor(1, &in1, 0);

    in0.setOutputValue(S::Inactive);
    in1.setOutputValue(S::Unknown);

    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Inactive);  // Inactive dominates Unknown
}

void TestStatusOps::testUpdateInputsAllowUnknownBlocksNullPredecessor()
{
    And gate; InputVcc in0;
    initElm(gate); initSrc(in0);
    gate.connectPredecessor(0, &in0, 0);
    // Input 1 left unconnected (null predecessor)

    in0.setOutputValue(S::Active);
    gate.updateLogic();
    QCOMPARE(gate.outputValue(), S::Unknown);  // Null predecessor → Unknown
}

