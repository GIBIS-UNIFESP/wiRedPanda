// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Logic/TestLogicElements.h"

#include "App/Element/LogicElements/LogicAnd.h"
#include "App/Element/LogicElements/LogicDemux.h"
#include "App/Element/LogicElements/LogicDFlipFlop.h"
#include "App/Element/LogicElements/LogicDLatch.h"
#include "App/Element/LogicElements/LogicJKFlipFlop.h"
#include "App/Element/LogicElements/LogicMux.h"
#include "App/Element/LogicElements/LogicNand.h"
#include "App/Element/LogicElements/LogicNode.h"
#include "App/Element/LogicElements/LogicNor.h"
#include "App/Element/LogicElements/LogicNot.h"
#include "App/Element/LogicElements/LogicOr.h"
#include "App/Element/LogicElements/LogicSource.h"
#include "App/Element/LogicElements/LogicSRFlipFlop.h"
#include "App/Element/LogicElements/LogicSRLatch.h"
#include "App/Element/LogicElements/LogicTFlipFlop.h"
#include "App/Element/LogicElements/LogicXnor.h"
#include "App/Element/LogicElements/LogicXor.h"
#include "Tests/Common/TestUtils.h"

void TestLogicElements::init()
{
    std::generate(m_inputs.begin(), m_inputs.end(), [] { return new LogicSource(); });
}

void TestLogicElements::cleanup()
{
    qDeleteAll(m_inputs);
    m_inputs.fill(nullptr);
}

void TestLogicElements::testLogicNode_data()
{
    QTest::addColumn<bool>("input");
    QTest::addColumn<bool>("expected");

    QTest::newRow("input 1 -> output 1") << true << true;
    QTest::newRow("input 0 -> output 0") << false << false;
}

void TestLogicElements::testLogicNode()
{
    QFETCH(bool, input);
    QFETCH(bool, expected);

    LogicNode elm;
    elm.connectPredecessor(0, m_inputs.at(0), 0);

    m_inputs.at(0)->setOutputValue(input);
    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestLogicElements::testLogicAnd_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("expected");

    QTest::newRow("1 AND 1 = 1") << true << true << true;
    QTest::newRow("1 AND 0 = 0") << true << false << false;
    QTest::newRow("0 AND 1 = 0") << false << true << false;
    QTest::newRow("0 AND 0 = 0") << false << false << false;
}

void TestLogicElements::testLogicAnd()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, expected);

    LogicAnd elm(2);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestLogicElements::testLogicOr_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("expected");

    QTest::newRow("1 OR 1 = 1") << true << true << true;
    QTest::newRow("1 OR 0 = 1") << true << false << true;
    QTest::newRow("0 OR 1 = 1") << false << true << true;
    QTest::newRow("0 OR 0 = 0") << false << false << false;
}

void TestLogicElements::testLogicOr()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, expected);

    LogicOr elm(2);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestLogicElements::testLogicSource()
{
    LogicSource elm;
    QCOMPARE(elm.outputValue(), false);
    elm.setOutputValue(true);
    QCOMPARE(elm.outputValue(), true);
    elm.setOutputValue(false);
    QCOMPARE(elm.outputValue(), false);
}

void TestLogicElements::testLogicMux_data()
{
    QTest::addColumn<bool>("in0");
    QTest::addColumn<bool>("in1");
    QTest::addColumn<bool>("sel");
    QTest::addColumn<bool>("expected");

    QTest::newRow("in0=0, in1=0, sel=0 -> 0") << false << false << false << false;
    QTest::newRow("in0=0, in1=0, sel=1 -> 0") << false << false << true << false;
    QTest::newRow("in0=0, in1=1, sel=0 -> 0") << false << true << false << false;
    QTest::newRow("in0=0, in1=1, sel=1 -> 1") << false << true << true << true;
    QTest::newRow("in0=1, in1=0, sel=0 -> 1") << true << false << false << true;
    QTest::newRow("in0=1, in1=0, sel=1 -> 0") << true << false << true << false;
    QTest::newRow("in0=1, in1=1, sel=0 -> 1") << true << true << false << true;
    QTest::newRow("in0=1, in1=1, sel=1 -> 1") << true << true << true << true;
}

void TestLogicElements::testLogicMux()
{
    QFETCH(bool, in0);
    QFETCH(bool, in1);
    QFETCH(bool, sel);
    QFETCH(bool, expected);

    LogicMux elm;
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);
    elm.connectPredecessor(2, m_inputs.at(2), 0);

    m_inputs.at(0)->setOutputValue(in0);
    m_inputs.at(1)->setOutputValue(in1);
    m_inputs.at(2)->setOutputValue(sel);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestLogicElements::testLogicDemux_data()
{
    QTest::addColumn<bool>("input");
    QTest::addColumn<bool>("sel");
    QTest::addColumn<bool>("out0");
    QTest::addColumn<bool>("out1");

    QTest::newRow("in=0, sel=0 -> out0=0, out1=0") << false << false << false << false;
    QTest::newRow("in=0, sel=1 -> out0=0, out1=0") << false << true << false << false;
    QTest::newRow("in=1, sel=0 -> out0=1, out1=0") << true << false << true << false;
    QTest::newRow("in=1, sel=1 -> out0=0, out1=1") << true << true << false << true;
}

void TestLogicElements::testLogicDemux()
{
    QFETCH(bool, input);
    QFETCH(bool, sel);
    QFETCH(bool, out0);
    QFETCH(bool, out1);

    LogicDemux elm;
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input);
    m_inputs.at(1)->setOutputValue(sel);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(0), out0);
    QCOMPARE(elm.outputValue(1), out1);
}

void TestLogicElements::testLogicDFlipFlop_data()
{
    QTest::addColumn<bool>("lastClk");
    QTest::addColumn<bool>("data");
    QTest::addColumn<bool>("clk");
    QTest::addColumn<bool>("preset");
    QTest::addColumn<bool>("clear");
    QTest::addColumn<bool>("expectedQ");
    QTest::addColumn<bool>("expectedNotQ");

    QTest::newRow("Clk up and D=0") << false << false << true << true << true << false << true;
    QTest::newRow("Clk up and D=1") << false << true << true << true << true << true << false;
    QTest::newRow("Preset=false") << false << false << false << false << true << true << false;
    QTest::newRow("Clear=false") << false << false << true << true << false << false << true;
    QTest::newRow("Clear and Preset=false") << false << false << true << false << false << true << true;
    QTest::newRow("Clk down D=0 maintain") << true << false << false << true << true << true << false;
    QTest::newRow("Clk down D=1 maintain") << true << true << false << true << true << true << false;
}

void TestLogicElements::testLogicDFlipFlop()
{
    QFETCH(bool, lastClk);
    QFETCH(bool, data);
    QFETCH(bool, clk);
    QFETCH(bool, preset);
    QFETCH(bool, clear);
    QFETCH(bool, expectedQ);
    QFETCH(bool, expectedNotQ);

    LogicDFlipFlop elm;
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);
    elm.connectPredecessor(2, m_inputs.at(2), 0);
    elm.connectPredecessor(3, m_inputs.at(3), 0);

    m_inputs.at(0)->setOutputValue(data);
    m_inputs.at(1)->setOutputValue(lastClk);
    m_inputs.at(2)->setOutputValue(false);
    m_inputs.at(3)->setOutputValue(false);

    elm.updateLogic();

    elm.setOutputValue(0, lastClk);
    elm.setOutputValue(1, !lastClk);

    m_inputs.at(0)->setOutputValue(data);
    m_inputs.at(1)->setOutputValue(clk);
    m_inputs.at(2)->setOutputValue(preset);
    m_inputs.at(3)->setOutputValue(clear);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(0), expectedQ);
    QCOMPARE(elm.outputValue(1), expectedNotQ);
}

void TestLogicElements::testLogicDLatch_data()
{
    QTest::addColumn<bool>("data");
    QTest::addColumn<bool>("enable");
    QTest::addColumn<bool>("prevState");
    QTest::addColumn<bool>("expectedQ");

    QTest::newRow("D=0, E=0, prev=0 -> Q=0") << false << false << false << false;
    QTest::newRow("D=0, E=0, prev=1 -> Q=1") << false << false << true << true;
    QTest::newRow("D=1, E=0, prev=0 -> Q=0") << true << false << false << false;
    QTest::newRow("D=1, E=0, prev=1 -> Q=1") << true << false << true << true;
    QTest::newRow("D=0, E=1, prev=X -> Q=0") << false << true << false << false;
    QTest::newRow("D=1, E=1, prev=X -> Q=1") << true << true << false << true;
}

void TestLogicElements::testLogicDLatch()
{
    QFETCH(bool, data);
    QFETCH(bool, enable);
    QFETCH(bool, prevState);
    QFETCH(bool, expectedQ);

    LogicDLatch elm;
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    elm.setOutputValue(0, prevState);
    elm.setOutputValue(1, !prevState);

    m_inputs.at(0)->setOutputValue(data);
    m_inputs.at(1)->setOutputValue(enable);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(0), expectedQ);
    QCOMPARE(elm.outputValue(1), !expectedQ);
}

void TestLogicElements::testLogicJKFlipFlop_data()
{
    QTest::addColumn<bool>("lastClk");
    QTest::addColumn<bool>("j");
    QTest::addColumn<bool>("clk");
    QTest::addColumn<bool>("k");
    QTest::addColumn<bool>("preset");
    QTest::addColumn<bool>("clear");
    QTest::addColumn<bool>("prevState");
    QTest::addColumn<bool>("expectedQ");
    QTest::addColumn<bool>("expectedNotQ");

    QTest::newRow("Preset=false") << false << false << false << false << false << true << false << true << false;
    QTest::newRow("Clear=false") << false << false << false << false << true << false << false << false << true;
    QTest::newRow("Clear and Preset=false") << false << false << false << true << false << false << false << true << true;
    QTest::newRow("Clk down J=0 maintain") << true << true << false << false << true << true << false << false << true;
    QTest::newRow("Clk down J=1 maintain") << true << true << false << false << true << true << false << false << true;
    QTest::newRow("Clk up J=1 K=1 swap (0)") << false << true << true << true << true << true << false << true << false;
    QTest::newRow("Clk up J=1 K=1 swap (1)") << false << true << true << true << true << true << true << false << true;
    QTest::newRow("Clk up J=1 K=0 (0)") << false << true << true << false << true << true << false << true << false;
    QTest::newRow("Clk up J=1 K=0 (1)") << false << true << true << false << true << true << true << true << false;
    QTest::newRow("Clk up J=0 K=1 (0)") << false << false << true << true << true << true << false << false << true;
    QTest::newRow("Clk up J=0 K=1 (1)") << false << false << true << true << true << true << true << false << true;
    QTest::newRow("Clk up J=0 K=0 (0)") << false << false << true << false << true << true << false << false << true;
    QTest::newRow("Clk up J=0 K=0 (1)") << false << false << true << false << true << true << true << true << false;
}

void TestLogicElements::testLogicJKFlipFlop()
{
    QFETCH(bool, lastClk);
    QFETCH(bool, j);
    QFETCH(bool, clk);
    QFETCH(bool, k);
    QFETCH(bool, preset);
    QFETCH(bool, clear);
    QFETCH(bool, prevState);
    QFETCH(bool, expectedQ);
    QFETCH(bool, expectedNotQ);

    LogicJKFlipFlop elm;
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);
    elm.connectPredecessor(2, m_inputs.at(2), 0);
    elm.connectPredecessor(3, m_inputs.at(3), 0);
    elm.connectPredecessor(4, m_inputs.at(4), 0);

    elm.updateLogic();

    m_inputs.at(0)->setOutputValue(j);
    m_inputs.at(1)->setOutputValue(lastClk);
    m_inputs.at(2)->setOutputValue(k);
    m_inputs.at(3)->setOutputValue(false);
    m_inputs.at(4)->setOutputValue(false);

    elm.updateLogic();

    elm.setOutputValue(0, prevState);
    elm.setOutputValue(1, !prevState);

    m_inputs.at(0)->setOutputValue(j);
    m_inputs.at(1)->setOutputValue(clk);
    m_inputs.at(2)->setOutputValue(k);
    m_inputs.at(3)->setOutputValue(preset);
    m_inputs.at(4)->setOutputValue(clear);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(0), expectedQ);
    QCOMPARE(elm.outputValue(1), expectedNotQ);
}

void TestLogicElements::testLogicSRFlipFlop_data()
{
    QTest::addColumn<bool>("lastClk");
    QTest::addColumn<bool>("s");
    QTest::addColumn<bool>("clk");
    QTest::addColumn<bool>("r");
    QTest::addColumn<bool>("preset");
    QTest::addColumn<bool>("clear");
    QTest::addColumn<bool>("prevState");
    QTest::addColumn<bool>("expectedQ");
    QTest::addColumn<bool>("expectedNotQ");

    QTest::newRow("Preset=false") << false << false << false << false << false << true << false << true << false;
    QTest::newRow("Clear=false") << false << false << false << false << true << false << true << false << true;
    QTest::newRow("Preset||Clear=false") << false << false << false << false << false << false << true << true << true;

    QTest::newRow("No change (0,0,0,0)") << false << false << false << false << true << true << false << false << true;
    QTest::newRow("No change (0,0,0,0) st1") << false << false << false << false << true << true << true << true << false;
    QTest::newRow("No change (0,1,0,0)") << false << true << false << false << true << true << false << false << true;
    QTest::newRow("No change (0,1,0,0) st1") << false << true << false << false << true << true << true << true << false;

    QTest::newRow("No change (0,0,0,1)") << false << false << false << true << true << true << false << false << true;
    QTest::newRow("No change (0,0,0,1) st1") << false << false << false << true << true << true << true << true << false;
    QTest::newRow("No change (1,0,1,0)") << true << false << true << false << true << true << false << false << true;
    QTest::newRow("No change (1,0,1,0) st1") << true << false << true << false << true << true << true << true << false;

    QTest::newRow("No change (1,1,1,0)") << true << true << true << false << true << true << false << false << true;
    QTest::newRow("No change (1,1,1,0) st1") << true << true << true << false << true << true << true << true << false;
    QTest::newRow("No change (1,0,1,1)") << true << false << true << true << true << true << false << false << true;
    QTest::newRow("No change (1,0,1,1) st1") << true << false << true << true << true << true << true << true << false;

    QTest::newRow("No change (0,0,1,0)") << false << false << true << false << true << true << false << false << true;
    QTest::newRow("No change (0,0,1,0) st1") << false << false << true << false << true << true << true << true << false;
    QTest::newRow("Q=0 (0,0,1,1)") << false << false << true << true << true << true << false << false << true;
    QTest::newRow("Q=0 (0,0,1,1) st1") << false << false << true << true << true << true << true << false << true;

    QTest::newRow("Q=1 (0,1,1,0)") << false << true << true << false << true << true << false << true << false;
    QTest::newRow("Q=1 (0,1,1,0) st1") << false << true << true << false << true << true << true << true << false;
    QTest::newRow("Not permitted (0,1,1,1)") << false << true << true << true << true << true << false << true << true;
    QTest::newRow("Not permitted (0,1,1,1) st1") << false << true << true << true << true << true << true << true << true;
}

void TestLogicElements::testLogicSRFlipFlop()
{
    QFETCH(bool, lastClk);
    QFETCH(bool, s);
    QFETCH(bool, clk);
    QFETCH(bool, r);
    QFETCH(bool, preset);
    QFETCH(bool, clear);
    QFETCH(bool, prevState);
    QFETCH(bool, expectedQ);
    QFETCH(bool, expectedNotQ);

    LogicSRFlipFlop elm;
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);
    elm.connectPredecessor(2, m_inputs.at(2), 0);
    elm.connectPredecessor(3, m_inputs.at(3), 0);
    elm.connectPredecessor(4, m_inputs.at(4), 0);

    elm.updateLogic();

    // Establish previous cycle values for S and R
    m_inputs.at(0)->setOutputValue(s);
    m_inputs.at(1)->setOutputValue(lastClk);
    m_inputs.at(2)->setOutputValue(r);
    m_inputs.at(3)->setOutputValue(true);
    m_inputs.at(4)->setOutputValue(true);

    elm.updateLogic();

    elm.setOutputValue(0, prevState);
    elm.setOutputValue(1, !prevState);

    // Now set the clock edge with established previous S and R values
    m_inputs.at(0)->setOutputValue(s);
    m_inputs.at(1)->setOutputValue(clk);
    m_inputs.at(2)->setOutputValue(r);
    m_inputs.at(3)->setOutputValue(preset);
    m_inputs.at(4)->setOutputValue(clear);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(0), expectedQ);
    QCOMPARE(elm.outputValue(1), expectedNotQ);
}

void TestLogicElements::testLogicTFlipFlop_data()
{
    QTest::addColumn<bool>("lastClk");
    QTest::addColumn<bool>("t");
    QTest::addColumn<bool>("clk");
    QTest::addColumn<bool>("preset");
    QTest::addColumn<bool>("clear");
    QTest::addColumn<bool>("prevState");
    QTest::addColumn<bool>("expectedQ");
    QTest::addColumn<bool>("expectedNotQ");

    QTest::newRow("No change (1,0,1) st0") << true << false << true << true << true << false << false << true;
    QTest::newRow("No change (1,1,1) st0") << true << true << true << true << true << false << false << true;
    QTest::newRow("No change (1,0,1) st1") << true << false << true << true << true << true << true << false;
    QTest::newRow("No change (1,1,1) st1") << true << true << true << true << true << true << true << false;

    QTest::newRow("No change (1,0,0) st0") << true << false << false << true << true << false << false << true;
    QTest::newRow("No change (1,0,0) st1") << true << false << false << true << true << true << true << false;

    QTest::newRow("Toggle (0,1,1) st0") << false << true << true << true << true << false << true << false;
    QTest::newRow("Toggle (0,1,1) st1") << false << true << true << true << true << true << false << true;

    QTest::newRow("Preset=false") << true << false << true << false << true << false << true << false;
    QTest::newRow("Clear=false") << true << false << true << true << false << true << false << true;
    QTest::newRow("Clear and Preset=false") << true << false << true << false << false << true << true << true;
}

void TestLogicElements::testLogicTFlipFlop()
{
    QFETCH(bool, lastClk);
    QFETCH(bool, t);
    QFETCH(bool, clk);
    QFETCH(bool, preset);
    QFETCH(bool, clear);
    QFETCH(bool, prevState);
    QFETCH(bool, expectedQ);
    QFETCH(bool, expectedNotQ);

    LogicTFlipFlop elm;
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);
    elm.connectPredecessor(2, m_inputs.at(2), 0);
    elm.connectPredecessor(3, m_inputs.at(3), 0);

    elm.updateLogic();

    m_inputs.at(0)->setOutputValue(t);
    m_inputs.at(1)->setOutputValue(lastClk);
    m_inputs.at(2)->setOutputValue(true);
    m_inputs.at(3)->setOutputValue(true);

    elm.updateLogic();

    elm.setOutputValue(0, prevState);
    elm.setOutputValue(1, !prevState);

    m_inputs.at(0)->setOutputValue(t);
    m_inputs.at(1)->setOutputValue(clk);
    m_inputs.at(2)->setOutputValue(preset);
    m_inputs.at(3)->setOutputValue(clear);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(0), expectedQ);
    QCOMPARE(elm.outputValue(1), expectedNotQ);
}

void TestLogicElements::testLogicXnor_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("expected");

    QTest::newRow("1 XNOR 1 = 1") << true << true << true;
    QTest::newRow("1 XNOR 0 = 0") << true << false << false;
    QTest::newRow("0 XNOR 1 = 0") << false << true << false;
    QTest::newRow("0 XNOR 0 = 1") << false << false << true;
}

void TestLogicElements::testLogicXnor()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, expected);

    LogicXnor elm(2);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestLogicElements::testLogicNot_data()
{
    QTest::addColumn<bool>("input");
    QTest::addColumn<bool>("expected");

    QTest::newRow("NOT 1 = 0") << true << false;
    QTest::newRow("NOT 0 = 1") << false << true;
}

void TestLogicElements::testLogicNot()
{
    QFETCH(bool, input);
    QFETCH(bool, expected);

    LogicNot elm;
    elm.connectPredecessor(0, m_inputs.at(0), 0);

    m_inputs.at(0)->setOutputValue(input);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

// Comprehensive circuit-context test: NOT gate alone in an isolated circuit
void TestLogicElements::testNotGateViaBuilder_data()
{
    QTest::addColumn<bool>("input");
    QTest::addColumn<bool>("expected");

    QTest::newRow("NOT gate input=1 (HIGH) -> output=0 (LOW)") << true << false;
    QTest::newRow("NOT gate input=0 (LOW) -> output=1 (HIGH)") << false << true;
}

void TestLogicElements::testNotGateViaBuilder()
{
    QFETCH(bool, input);
    QFETCH(bool, expected);

    LogicNot notGate;
    notGate.connectPredecessor(0, m_inputs.at(0), 0);

    m_inputs.at(0)->setOutputValue(input);
    notGate.updateLogic();

    QCOMPARE(notGate.outputValue(), expected);
}

// Comprehensive circuit-context test: NOT gate connected to AND gate (like in decoder)
void TestLogicElements::testNotGateInAnd_data()
{
    QTest::addColumn<bool>("input0");
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("expectedNotInput0");
    QTest::addColumn<bool>("expectedAndOutput");

    // AND gate: output = input0 AND input1
    // Here we use: input0 = true, and input1 comes from NOT gate inverting input1_raw
    // So: output = input0 AND (NOT input1_raw)
    // If input0=1 and input1_raw=0 (inverted to 1): output = 1 AND 1 = 1
    QTest::newRow("AND(1, NOT(0)) = AND(1, 1) = 1") << true << false << true << true;

    // If input0=1 and input1_raw=1 (inverted to 0): output = 1 AND 0 = 0
    QTest::newRow("AND(1, NOT(1)) = AND(1, 0) = 0") << true << true << false << false;

    // If input0=0 and input1_raw=0 (inverted to 1): output = 0 AND 1 = 0
    QTest::newRow("AND(0, NOT(0)) = AND(0, 1) = 0") << false << false << true << false;

    // If input0=0 and input1_raw=1 (inverted to 0): output = 0 AND 0 = 0
    QTest::newRow("AND(0, NOT(1)) = AND(0, 0) = 0") << false << true << false << false;
}

void TestLogicElements::testNotGateInAnd()
{
    QFETCH(bool, input0);
    QFETCH(bool, input1);
    QFETCH(bool, expectedNotInput0);
    Q_UNUSED(expectedNotInput0);  // Using for clarity in test data
    QFETCH(bool, expectedAndOutput);

    LogicNot invGate;
    invGate.connectPredecessor(0, m_inputs.at(1), 0);

    LogicAnd andGate(2);
    andGate.connectPredecessor(0, m_inputs.at(0), 0);
    andGate.connectPredecessor(1, &invGate, 0);

    m_inputs.at(0)->setOutputValue(input0);
    m_inputs.at(1)->setOutputValue(input1);

    invGate.updateLogic();
    QCOMPARE(invGate.outputValue(), !input1);

    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), expectedAndOutput);
}

void TestLogicElements::testLogicNand_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("expected");

    QTest::newRow("1 NAND 1 = 0") << true << true << false;
    QTest::newRow("1 NAND 0 = 1") << true << false << true;
    QTest::newRow("0 NAND 1 = 1") << false << true << true;
    QTest::newRow("0 NAND 0 = 1") << false << false << true;
}

void TestLogicElements::testLogicNand()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, expected);

    LogicNand elm(2);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestLogicElements::testLogicNor_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("expected");

    QTest::newRow("1 NOR 1 = 0") << true << true << false;
    QTest::newRow("1 NOR 0 = 0") << true << false << false;
    QTest::newRow("0 NOR 1 = 0") << false << true << false;
    QTest::newRow("0 NOR 0 = 1") << false << false << true;
}

void TestLogicElements::testLogicNor()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, expected);

    LogicNor elm(2);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestLogicElements::testLogicXor_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("expected");

    QTest::newRow("0 XOR 0 = 0") << false << false << false;
    QTest::newRow("0 XOR 1 = 1") << false << true << true;
    QTest::newRow("1 XOR 0 = 1") << true << false << true;
    QTest::newRow("1 XOR 1 = 0") << true << true << false;
}

void TestLogicElements::testLogicXor()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, expected);

    LogicXor elm(2);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestLogicElements::testLogicSRLatch_data()
{
    QTest::addColumn<bool>("set");
    QTest::addColumn<bool>("reset");
    QTest::addColumn<bool>("prevState");
    QTest::addColumn<bool>("expectedQ");

    QTest::newRow("S=0, R=0, prev=0 -> Q=0") << false << false << false << false;
    QTest::newRow("S=0, R=0, prev=1 -> Q=1") << false << false << true << true;
    QTest::newRow("S=1, R=0, prev=X -> Q=1") << true << false << false << true;
    QTest::newRow("S=0, R=1, prev=X -> Q=0") << false << true << false << false;
}

void TestLogicElements::testLogicSRLatch()
{
    QFETCH(bool, set);
    QFETCH(bool, reset);
    QFETCH(bool, prevState);
    QFETCH(bool, expectedQ);

    LogicSRLatch elm;
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    elm.setOutputValue(0, prevState);
    elm.setOutputValue(1, !prevState);

    m_inputs.at(0)->setOutputValue(set);
    m_inputs.at(1)->setOutputValue(reset);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(0), expectedQ);
    QCOMPARE(elm.outputValue(1), !expectedQ);
}

void TestLogicElements::testLogic3InputAnd_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("input3");
    QTest::addColumn<bool>("expected");

    // 3-input AND: Output=1 only when all inputs are 1
    QTest::newRow("0 AND 0 AND 0 = 0") << false << false << false << false;
    QTest::newRow("0 AND 0 AND 1 = 0") << false << false << true << false;
    QTest::newRow("0 AND 1 AND 0 = 0") << false << true << false << false;
    QTest::newRow("0 AND 1 AND 1 = 0") << false << true << true << false;
    QTest::newRow("1 AND 0 AND 0 = 0") << true << false << false << false;
    QTest::newRow("1 AND 0 AND 1 = 0") << true << false << true << false;
    QTest::newRow("1 AND 1 AND 0 = 0") << true << true << false << false;
    QTest::newRow("1 AND 1 AND 1 = 1") << true << true << true << true;
}

void TestLogicElements::testLogic3InputAnd()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, input3);
    QFETCH(bool, expected);

    LogicAnd elm(3);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);
    elm.connectPredecessor(2, m_inputs.at(2), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);
    m_inputs.at(2)->setOutputValue(input3);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestLogicElements::testLogic3InputOr_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("input3");
    QTest::addColumn<bool>("expected");

    // 3-input OR: Output=0 only when all inputs are 0
    QTest::newRow("0 OR 0 OR 0 = 0") << false << false << false << false;
    QTest::newRow("0 OR 0 OR 1 = 1") << false << false << true << true;
    QTest::newRow("0 OR 1 OR 0 = 1") << false << true << false << true;
    QTest::newRow("0 OR 1 OR 1 = 1") << false << true << true << true;
    QTest::newRow("1 OR 0 OR 0 = 1") << true << false << false << true;
    QTest::newRow("1 OR 0 OR 1 = 1") << true << false << true << true;
    QTest::newRow("1 OR 1 OR 0 = 1") << true << true << false << true;
    QTest::newRow("1 OR 1 OR 1 = 1") << true << true << true << true;
}

void TestLogicElements::testLogic3InputOr()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, input3);
    QFETCH(bool, expected);

    LogicOr elm(3);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);
    elm.connectPredecessor(2, m_inputs.at(2), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);
    m_inputs.at(2)->setOutputValue(input3);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestLogicElements::testLogic4InputAnd_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("input3");
    QTest::addColumn<bool>("input4");
    QTest::addColumn<bool>("expected");

    // 4-input AND: Output=1 only when all inputs are 1 (16 cases)
    QTest::newRow("0 AND 0 AND 0 AND 0 = 0") << false << false << false << false << false;
    QTest::newRow("0 AND 0 AND 0 AND 1 = 0") << false << false << false << true << false;
    QTest::newRow("0 AND 0 AND 1 AND 0 = 0") << false << false << true << false << false;
    QTest::newRow("0 AND 0 AND 1 AND 1 = 0") << false << false << true << true << false;
    QTest::newRow("0 AND 1 AND 0 AND 0 = 0") << false << true << false << false << false;
    QTest::newRow("0 AND 1 AND 0 AND 1 = 0") << false << true << false << true << false;
    QTest::newRow("0 AND 1 AND 1 AND 0 = 0") << false << true << true << false << false;
    QTest::newRow("0 AND 1 AND 1 AND 1 = 0") << false << true << true << true << false;
    QTest::newRow("1 AND 0 AND 0 AND 0 = 0") << true << false << false << false << false;
    QTest::newRow("1 AND 0 AND 0 AND 1 = 0") << true << false << false << true << false;
    QTest::newRow("1 AND 0 AND 1 AND 0 = 0") << true << false << true << false << false;
    QTest::newRow("1 AND 0 AND 1 AND 1 = 0") << true << false << true << true << false;
    QTest::newRow("1 AND 1 AND 0 AND 0 = 0") << true << true << false << false << false;
    QTest::newRow("1 AND 1 AND 0 AND 1 = 0") << true << true << false << true << false;
    QTest::newRow("1 AND 1 AND 1 AND 0 = 0") << true << true << true << false << false;
    QTest::newRow("1 AND 1 AND 1 AND 1 = 1") << true << true << true << true << true;
}

void TestLogicElements::testLogic4InputAnd()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, input3);
    QFETCH(bool, input4);
    QFETCH(bool, expected);

    LogicAnd elm(4);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);
    elm.connectPredecessor(2, m_inputs.at(2), 0);
    elm.connectPredecessor(3, m_inputs.at(3), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);
    m_inputs.at(2)->setOutputValue(input3);
    m_inputs.at(3)->setOutputValue(input4);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestLogicElements::testLogic4InputOr_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("input3");
    QTest::addColumn<bool>("input4");
    QTest::addColumn<bool>("expected");

    // 4-input OR: Output=0 only when all inputs are 0 (16 cases)
    QTest::newRow("0 OR 0 OR 0 OR 0 = 0") << false << false << false << false << false;
    QTest::newRow("0 OR 0 OR 0 OR 1 = 1") << false << false << false << true << true;
    QTest::newRow("0 OR 0 OR 1 OR 0 = 1") << false << false << true << false << true;
    QTest::newRow("0 OR 0 OR 1 OR 1 = 1") << false << false << true << true << true;
    QTest::newRow("0 OR 1 OR 0 OR 0 = 1") << false << true << false << false << true;
    QTest::newRow("0 OR 1 OR 0 OR 1 = 1") << false << true << false << true << true;
    QTest::newRow("0 OR 1 OR 1 OR 0 = 1") << false << true << true << false << true;
    QTest::newRow("0 OR 1 OR 1 OR 1 = 1") << false << true << true << true << true;
    QTest::newRow("1 OR 0 OR 0 OR 0 = 1") << true << false << false << false << true;
    QTest::newRow("1 OR 0 OR 0 OR 1 = 1") << true << false << false << true << true;
    QTest::newRow("1 OR 0 OR 1 OR 0 = 1") << true << false << true << false << true;
    QTest::newRow("1 OR 0 OR 1 OR 1 = 1") << true << false << true << true << true;
    QTest::newRow("1 OR 1 OR 0 OR 0 = 1") << true << true << false << false << true;
    QTest::newRow("1 OR 1 OR 0 OR 1 = 1") << true << true << false << true << true;
    QTest::newRow("1 OR 1 OR 1 OR 0 = 1") << true << true << true << false << true;
    QTest::newRow("1 OR 1 OR 1 OR 1 = 1") << true << true << true << true << true;
}

void TestLogicElements::testLogic4InputOr()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, input3);
    QFETCH(bool, input4);
    QFETCH(bool, expected);

    LogicOr elm(4);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);
    elm.connectPredecessor(2, m_inputs.at(2), 0);
    elm.connectPredecessor(3, m_inputs.at(3), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);
    m_inputs.at(2)->setOutputValue(input3);
    m_inputs.at(3)->setOutputValue(input4);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestLogicElements::testLogic3InputXor_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("input3");
    QTest::addColumn<bool>("expected");

    // 3-input XOR: Output=1 when odd number of inputs are 1
    QTest::newRow("0 XOR 0 XOR 0 = 0") << false << false << false << false;
    QTest::newRow("0 XOR 0 XOR 1 = 1") << false << false << true << true;
    QTest::newRow("0 XOR 1 XOR 0 = 1") << false << true << false << true;
    QTest::newRow("0 XOR 1 XOR 1 = 0") << false << true << true << false;
    QTest::newRow("1 XOR 0 XOR 0 = 1") << true << false << false << true;
    QTest::newRow("1 XOR 0 XOR 1 = 0") << true << false << true << false;
    QTest::newRow("1 XOR 1 XOR 0 = 0") << true << true << false << false;
    QTest::newRow("1 XOR 1 XOR 1 = 1") << true << true << true << true;
}

void TestLogicElements::testLogic3InputXor()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, input3);
    QFETCH(bool, expected);

    // Use 2-input XOR gates to simulate 3-input: (a XOR b) XOR c
    LogicXor xor1(2), xor2(2);
    xor1.connectPredecessor(0, m_inputs.at(0), 0);
    xor1.connectPredecessor(1, m_inputs.at(1), 0);
    xor2.connectPredecessor(0, &xor1, 0);
    xor2.connectPredecessor(1, m_inputs.at(2), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);
    m_inputs.at(2)->setOutputValue(input3);

    xor1.updateLogic();
    xor2.updateLogic();

    QCOMPARE(xor2.outputValue(), expected);
}

void TestLogicElements::testLogic3InputXnor_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("input3");
    QTest::addColumn<bool>("expected");

    // 3-input XNOR: Cascaded (a XNOR b) XNOR c
    // Tests cascaded XNOR behavior: (a==b) == c
    QTest::newRow("0 XNOR 0 XNOR 0 = 0") << false << false << false << false;
    QTest::newRow("0 XNOR 0 XNOR 1 = 1") << false << false << true << true;
    QTest::newRow("0 XNOR 1 XNOR 0 = 1") << false << true << false << true;
    QTest::newRow("0 XNOR 1 XNOR 1 = 0") << false << true << true << false;
    QTest::newRow("1 XNOR 0 XNOR 0 = 1") << true << false << false << true;
    QTest::newRow("1 XNOR 0 XNOR 1 = 0") << true << false << true << false;
    QTest::newRow("1 XNOR 1 XNOR 0 = 0") << true << true << false << false;
    QTest::newRow("1 XNOR 1 XNOR 1 = 1") << true << true << true << true;
}

void TestLogicElements::testLogic3InputXnor()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, input3);
    QFETCH(bool, expected);

    // Use 2-input XNOR gates to simulate 3-input: (a XNOR b) XNOR c
    LogicXnor xnor1(2), xnor2(2);
    xnor1.connectPredecessor(0, m_inputs.at(0), 0);
    xnor1.connectPredecessor(1, m_inputs.at(1), 0);
    xnor2.connectPredecessor(0, &xnor1, 0);
    xnor2.connectPredecessor(1, m_inputs.at(2), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);
    m_inputs.at(2)->setOutputValue(input3);

    xnor1.updateLogic();
    xnor2.updateLogic();

    QCOMPARE(xnor2.outputValue(), expected);
}

/**
 * Test: 5-input AND gate - exhaustive truth table (32 cases)
 */
void TestLogicElements::testLogic5InputAnd_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("expected");

    // All 32 combinations from 0 to 31
    for (int i = 0; i < 32; ++i) {
        bool expected = (i == 31);  // Only all-1s = 1
        QTest::newRow(QString("5AND case %1").arg(i).toLatin1().data()) << i << expected;
    }
}

void TestLogicElements::testLogic5InputAnd()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    LogicAnd elm(5);
    for (int i = 0; i < 5; ++i) {
        elm.connectPredecessor(i, m_inputs.at(i), 0);
    }

    for (int i = 0; i < 5; ++i) {
        bool bit = (value >> i) & 1;
        m_inputs.at(i)->setOutputValue(bit);
    }

    elm.updateLogic();
    QCOMPARE(elm.outputValue(), expected);
}

/**
 * Test: 5-input OR gate - exhaustive truth table (32 cases)
 */
void TestLogicElements::testLogic5InputOr_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("expected");

    for (int i = 0; i < 32; ++i) {
        bool expected = (i != 0);  // Only all-0s = 0
        QTest::newRow(QString("5OR case %1").arg(i).toLatin1().data()) << i << expected;
    }
}

void TestLogicElements::testLogic5InputOr()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    LogicOr elm(5);
    for (int i = 0; i < 5; ++i) {
        elm.connectPredecessor(i, m_inputs.at(i), 0);
    }

    for (int i = 0; i < 5; ++i) {
        bool bit = (value >> i) & 1;
        m_inputs.at(i)->setOutputValue(bit);
    }

    elm.updateLogic();
    QCOMPARE(elm.outputValue(), expected);
}

/**
 * Test: 5-input NAND gate - exhaustive truth table (32 cases)
 */
void TestLogicElements::testLogic5InputNand_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("expected");

    for (int i = 0; i < 32; ++i) {
        bool expected = (i != 31);  // All-1s = 0, rest = 1
        QTest::newRow(QString("5NAND case %1").arg(i).toLatin1().data()) << i << expected;
    }
}

void TestLogicElements::testLogic5InputNand()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    LogicNand elm(5);
    for (int i = 0; i < 5; ++i) {
        elm.connectPredecessor(i, m_inputs.at(i), 0);
    }

    for (int i = 0; i < 5; ++i) {
        bool bit = (value >> i) & 1;
        m_inputs.at(i)->setOutputValue(bit);
    }

    elm.updateLogic();
    QCOMPARE(elm.outputValue(), expected);
}

/**
 * Test: 5-input NOR gate - exhaustive truth table (32 cases)
 */
void TestLogicElements::testLogic5InputNor_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("expected");

    for (int i = 0; i < 32; ++i) {
        bool expected = (i == 0);  // All-0s = 1, rest = 0
        QTest::newRow(QString("5NOR case %1").arg(i).toLatin1().data()) << i << expected;
    }
}

void TestLogicElements::testLogic5InputNor()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    LogicNor elm(5);
    for (int i = 0; i < 5; ++i) {
        elm.connectPredecessor(i, m_inputs.at(i), 0);
    }

    for (int i = 0; i < 5; ++i) {
        bool bit = (value >> i) & 1;
        m_inputs.at(i)->setOutputValue(bit);
    }

    elm.updateLogic();
    QCOMPARE(elm.outputValue(), expected);
}

/**
 * Test: 8-input AND gate - strategic test cases
 */
void TestLogicElements::testLogic8InputAnd_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("expected");

    // Strategic cases for 8-input AND
    QTest::newRow("all 0s") << 0 << false;
    QTest::newRow("all 1s") << 255 << true;
    QTest::newRow("first 1 only") << 1 << false;
    QTest::newRow("first 4 set") << 15 << false;
    QTest::newRow("first 7 set") << 127 << false;
    QTest::newRow("all but last") << 254 << false;
    QTest::newRow("alternating even") << 0x55 << false;  // 01010101
    QTest::newRow("alternating odd") << 0xAA << false;   // 10101010
    QTest::newRow("low 4 bits") << 0x0F << false;        // 00001111
    QTest::newRow("high 4 bits") << 0xF0 << false;       // 11110000
}

void TestLogicElements::testLogic8InputAnd()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    LogicAnd elm(8);
    for (int i = 0; i < 8; ++i) {
        elm.connectPredecessor(i, m_inputs.at(i), 0);
    }

    for (int i = 0; i < 8; ++i) {
        bool bit = (value >> i) & 1;
        m_inputs.at(i)->setOutputValue(bit);
    }

    elm.updateLogic();
    QCOMPARE(elm.outputValue(), expected);
}

/**
 * Test: 8-input OR gate - strategic test cases
 */
void TestLogicElements::testLogic8InputOr_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("expected");

    // Strategic cases for 8-input OR
    QTest::newRow("all 0s") << 0 << false;
    QTest::newRow("all 1s") << 255 << true;
    QTest::newRow("first 1 only") << 1 << true;
    QTest::newRow("first 4 set") << 15 << true;
    QTest::newRow("first 7 set") << 127 << true;
    QTest::newRow("all but last") << 254 << true;
    QTest::newRow("last 1 only") << 128 << true;
    QTest::newRow("alternating even") << 0x55 << true;   // 01010101
    QTest::newRow("alternating odd") << 0xAA << true;    // 10101010
    QTest::newRow("single middle bit") << 0x10 << true;  // 00010000
}

void TestLogicElements::testLogic8InputOr()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    LogicOr elm(8);
    for (int i = 0; i < 8; ++i) {
        elm.connectPredecessor(i, m_inputs.at(i), 0);
    }

    for (int i = 0; i < 8; ++i) {
        bool bit = (value >> i) & 1;
        m_inputs.at(i)->setOutputValue(bit);
    }

    elm.updateLogic();
    QCOMPARE(elm.outputValue(), expected);
}

/**
 * Test: 8-input NAND gate - strategic test cases
 */
void TestLogicElements::testLogic8InputNand_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("expected");

    // Strategic cases for 8-input NAND (inverted AND)
    QTest::newRow("all 0s") << 0 << true;
    QTest::newRow("all 1s") << 255 << false;
    QTest::newRow("first 1 only") << 1 << true;
    QTest::newRow("first 4 set") << 15 << true;
    QTest::newRow("first 7 set") << 127 << true;
    QTest::newRow("all but last") << 254 << true;
    QTest::newRow("alternating even") << 0x55 << true;   // 01010101
    QTest::newRow("alternating odd") << 0xAA << true;    // 10101010
    QTest::newRow("low 4 bits") << 0x0F << true;         // 00001111
}

void TestLogicElements::testLogic8InputNand()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    LogicNand elm(8);
    for (int i = 0; i < 8; ++i) {
        elm.connectPredecessor(i, m_inputs.at(i), 0);
    }

    for (int i = 0; i < 8; ++i) {
        bool bit = (value >> i) & 1;
        m_inputs.at(i)->setOutputValue(bit);
    }

    elm.updateLogic();
    QCOMPARE(elm.outputValue(), expected);
}

/**
 * Test: 8-input NOR gate - strategic test cases
 */
void TestLogicElements::testLogic8InputNor_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("expected");

    // Strategic cases for 8-input NOR (inverted OR)
    QTest::newRow("all 0s") << 0 << true;
    QTest::newRow("all 1s") << 255 << false;
    QTest::newRow("first 1 only") << 1 << false;
    QTest::newRow("first 4 set") << 15 << false;
    QTest::newRow("first 7 set") << 127 << false;
    QTest::newRow("all but last") << 254 << false;
    QTest::newRow("last 1 only") << 128 << false;
    QTest::newRow("alternating even") << 0x55 << false;  // 01010101
    QTest::newRow("alternating odd") << 0xAA << false;   // 10101010
    QTest::newRow("single middle bit") << 0x10 << false; // 00010000
}

void TestLogicElements::testLogic8InputNor()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    LogicNor elm(8);
    for (int i = 0; i < 8; ++i) {
        elm.connectPredecessor(i, m_inputs.at(i), 0);
    }

    for (int i = 0; i < 8; ++i) {
        bool bit = (value >> i) & 1;
        m_inputs.at(i)->setOutputValue(bit);
    }

    elm.updateLogic();
    QCOMPARE(elm.outputValue(), expected);
}

/**
 * Test: Fan-out - one output driving multiple inputs
 * Verifies that a single LogicSource can successfully drive multiple gate inputs
 */
void TestLogicElements::testFanOut_data()
{
    QTest::addColumn<bool>("inputValue");
    QTest::addColumn<bool>("expectedAnd");
    QTest::addColumn<bool>("expectedOr");
    QTest::addColumn<bool>("expectedXor");

    // When input is 0: AND(0,0)=0, OR(0,0)=0, XOR(0,0)=0
    QTest::newRow("all gates input=0") << false << false << false << false;

    // When input is 1: AND(1,1)=1, OR(1,1)=1, XOR(1,1)=0
    QTest::newRow("all gates input=1") << true << true << true << false;
}

void TestLogicElements::testFanOut()
{
    QFETCH(bool, inputValue);
    QFETCH(bool, expectedAnd);
    QFETCH(bool, expectedOr);
    QFETCH(bool, expectedXor);

    // Single LogicSource drives 3 different gates (fan-out test)
    LogicSource source;
    LogicAnd andGate(2);
    LogicOr orGate(2);
    LogicXor xorGate(2);

    // Connect same source to all three gates
    andGate.connectPredecessor(0, &source, 0);
    andGate.connectPredecessor(1, &source, 0);

    orGate.connectPredecessor(0, &source, 0);
    orGate.connectPredecessor(1, &source, 0);

    xorGate.connectPredecessor(0, &source, 0);
    xorGate.connectPredecessor(1, &source, 0);

    // Set input value
    source.setOutputValue(inputValue);

    // Update all gates
    andGate.updateLogic();
    orGate.updateLogic();
    xorGate.updateLogic();

    // Verify all gates receive the same signal correctly
    QCOMPARE(andGate.outputValue(), expectedAnd);
    QCOMPARE(orGate.outputValue(), expectedOr);
    QCOMPARE(xorGate.outputValue(), expectedXor);
}

/**
 * Test: 6-input AND gate
 * Fills coverage gap between 5-input and 8-input tests
 */
void TestLogicElements::testLogic6InputAnd_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("expected");

    // Key cases for 6-input AND: only all-1s = 1, rest = 0
    QTest::newRow("all 0s") << 0 << false;
    QTest::newRow("all 1s") << 63 << true;  // 0b111111 = 63
    QTest::newRow("5 set, last 0") << 31 << false;  // 0b011111
    QTest::newRow("last 5 set, first 0") << 62 << false;  // 0b111110
    QTest::newRow("alternating 01") << 0x15 << false;  // 0b010101
    QTest::newRow("alternating 10") << 0x2A << false;  // 0b101010
}

void TestLogicElements::testLogic6InputAnd()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    LogicAnd elm(6);
    for (int i = 0; i < 6; ++i) {
        elm.connectPredecessor(i, m_inputs.at(i), 0);
    }

    for (int i = 0; i < 6; ++i) {
        bool bit = (value >> i) & 1;
        m_inputs.at(i)->setOutputValue(bit);
    }

    elm.updateLogic();
    QCOMPARE(elm.outputValue(), expected);
}

