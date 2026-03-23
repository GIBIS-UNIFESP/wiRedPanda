// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Logic/TestElementLogic.h"

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Demux.h"
#include "App/Element/GraphicElements/DFlipFlop.h"
#include "App/Element/GraphicElements/DLatch.h"
#include "App/Element/GraphicElements/InputVCC.h"
#include "App/Element/GraphicElements/JKFlipFlop.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Mux.h"
#include "App/Element/GraphicElements/Nand.h"
#include "App/Element/GraphicElements/Node.h"
#include "App/Element/GraphicElements/Nor.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Element/GraphicElements/SRFlipFlop.h"
#include "App/Element/GraphicElements/SRLatch.h"
#include "App/Element/GraphicElements/TFlipFlop.h"
#include "App/Element/GraphicElements/Xnor.h"
#include "App/Element/GraphicElements/Xor.h"
#include "Tests/Common/TestUtils.h"

static void initTESrc(GraphicElement &elm) { elm.initSimulationVectors(0, 1); }
static void initTEElm(GraphicElement &elm) { elm.initSimulationVectors(elm.inputSize(), elm.outputSize()); }

void TestElementLogic::init()
{
    std::generate(m_inputs.begin(), m_inputs.end(), [] {
        auto *src = new InputVcc();
        initTESrc(*src);
        return src;
    });
}

void TestElementLogic::cleanup()
{
    qDeleteAll(m_inputs);
    m_inputs.fill(nullptr);
}

void TestElementLogic::testNode_data()
{
    QTest::addColumn<bool>("input");
    QTest::addColumn<bool>("expected");

    QTest::newRow("input 1 -> output 1") << true << true;
    QTest::newRow("input 0 -> output 0") << false << false;
}

void TestElementLogic::testNode()
{
    QFETCH(bool, input);
    QFETCH(bool, expected);

    Node elm; initTEElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);

    m_inputs.at(0)->setOutputValue(input);
    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestElementLogic::testAnd_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("expected");

    QTest::newRow("1 AND 1 = 1") << true << true << true;
    QTest::newRow("1 AND 0 = 0") << true << false << false;
    QTest::newRow("0 AND 1 = 0") << false << true << false;
    QTest::newRow("0 AND 0 = 0") << false << false << false;
}

void TestElementLogic::testAnd()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, expected);

    And elm; initTEElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestElementLogic::testOr_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("expected");

    QTest::newRow("1 OR 1 = 1") << true << true << true;
    QTest::newRow("1 OR 0 = 1") << true << false << true;
    QTest::newRow("0 OR 1 = 1") << false << true << true;
    QTest::newRow("0 OR 0 = 0") << false << false << false;
}

void TestElementLogic::testOr()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, expected);

    Or elm; initTEElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestElementLogic::testSource()
{
    InputVcc elm; initTESrc(elm);
    // InputVcc defaults to true (VCC = always high) via initSimulationDefaults
    QCOMPARE(elm.outputValue(), true);
    elm.setOutputValue(false);
    QCOMPARE(elm.outputValue(), false);
    elm.setOutputValue(true);
    QCOMPARE(elm.outputValue(), true);
}

void TestElementLogic::testMux_data()
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

void TestElementLogic::testMux()
{
    QFETCH(bool, in0);
    QFETCH(bool, in1);
    QFETCH(bool, sel);
    QFETCH(bool, expected);

    Mux elm; initTEElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);
    elm.connectPredecessor(2, m_inputs.at(2), 0);

    m_inputs.at(0)->setOutputValue(in0);
    m_inputs.at(1)->setOutputValue(in1);
    m_inputs.at(2)->setOutputValue(sel);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestElementLogic::testDemux_data()
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

void TestElementLogic::testDemux()
{
    QFETCH(bool, input);
    QFETCH(bool, sel);
    QFETCH(bool, out0);
    QFETCH(bool, out1);

    Demux elm; initTEElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input);
    m_inputs.at(1)->setOutputValue(sel);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(0), out0);
    QCOMPARE(elm.outputValue(1), out1);
}

void TestElementLogic::testDFlipFlop_data()
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

void TestElementLogic::testDFlipFlop()
{
    QFETCH(bool, lastClk);
    QFETCH(bool, data);
    QFETCH(bool, clk);
    QFETCH(bool, preset);
    QFETCH(bool, clear);
    QFETCH(bool, expectedQ);
    QFETCH(bool, expectedNotQ);

    DFlipFlop elm; initTEElm(elm);
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

void TestElementLogic::testDLatch_data()
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

void TestElementLogic::testDLatch()
{
    QFETCH(bool, data);
    QFETCH(bool, enable);
    QFETCH(bool, prevState);
    QFETCH(bool, expectedQ);

    DLatch elm; initTEElm(elm);
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

void TestElementLogic::testJKFlipFlop_data()
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

void TestElementLogic::testJKFlipFlop()
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

    JKFlipFlop elm; initTEElm(elm);
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

void TestElementLogic::testSRFlipFlop_data()
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

void TestElementLogic::testSRFlipFlop()
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

    SRFlipFlop elm; initTEElm(elm);
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

void TestElementLogic::testTFlipFlop_data()
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

void TestElementLogic::testTFlipFlop()
{
    QFETCH(bool, lastClk);
    QFETCH(bool, t);
    QFETCH(bool, clk);
    QFETCH(bool, preset);
    QFETCH(bool, clear);
    QFETCH(bool, prevState);
    QFETCH(bool, expectedQ);
    QFETCH(bool, expectedNotQ);

    TFlipFlop elm; initTEElm(elm);
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

void TestElementLogic::testXnor_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("expected");

    QTest::newRow("1 XNOR 1 = 1") << true << true << true;
    QTest::newRow("1 XNOR 0 = 0") << true << false << false;
    QTest::newRow("0 XNOR 1 = 0") << false << true << false;
    QTest::newRow("0 XNOR 0 = 1") << false << false << true;
}

void TestElementLogic::testXnor()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, expected);

    Xnor elm; initTEElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestElementLogic::testNot_data()
{
    QTest::addColumn<bool>("input");
    QTest::addColumn<bool>("expected");

    QTest::newRow("NOT 1 = 0") << true << false;
    QTest::newRow("NOT 0 = 1") << false << true;
}

void TestElementLogic::testNot()
{
    QFETCH(bool, input);
    QFETCH(bool, expected);

    Not elm; initTEElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);

    m_inputs.at(0)->setOutputValue(input);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

// Comprehensive circuit-context test: NOT gate alone in an isolated circuit
void TestElementLogic::testNotGateViaBuilder_data()
{
    QTest::addColumn<bool>("input");
    QTest::addColumn<bool>("expected");

    QTest::newRow("NOT gate input=1 (HIGH) -> output=0 (LOW)") << true << false;
    QTest::newRow("NOT gate input=0 (LOW) -> output=1 (HIGH)") << false << true;
}

void TestElementLogic::testNotGateViaBuilder()
{
    QFETCH(bool, input);
    QFETCH(bool, expected);

    Not notGate; initTEElm(notGate);
    notGate.connectPredecessor(0, m_inputs.at(0), 0);

    m_inputs.at(0)->setOutputValue(input);
    notGate.updateLogic();

    QCOMPARE(notGate.outputValue(), expected);
}

// Comprehensive circuit-context test: NOT gate connected to AND gate (like in decoder)
void TestElementLogic::testNotGateInAnd_data()
{
    QTest::addColumn<bool>("input0");
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("expectedNotInput0");
    QTest::addColumn<bool>("expectedAndOutput");

    QTest::newRow("AND(1, NOT(0)) = AND(1, 1) = 1") << true << false << true << true;
    QTest::newRow("AND(1, NOT(1)) = AND(1, 0) = 0") << true << true << false << false;
    QTest::newRow("AND(0, NOT(0)) = AND(0, 1) = 0") << false << false << true << false;
    QTest::newRow("AND(0, NOT(1)) = AND(0, 0) = 0") << false << true << false << false;
}

void TestElementLogic::testNotGateInAnd()
{
    QFETCH(bool, input0);
    QFETCH(bool, input1);
    QFETCH(bool, expectedNotInput0);
    Q_UNUSED(expectedNotInput0);  // Using for clarity in test data
    QFETCH(bool, expectedAndOutput);

    Not invGate; initTEElm(invGate);
    invGate.connectPredecessor(0, m_inputs.at(1), 0);

    And andGate; initTEElm(andGate);
    andGate.connectPredecessor(0, m_inputs.at(0), 0);
    andGate.connectPredecessor(1, &invGate, 0);

    m_inputs.at(0)->setOutputValue(input0);
    m_inputs.at(1)->setOutputValue(input1);

    invGate.updateLogic();
    QCOMPARE(invGate.outputValue(), !input1);

    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), expectedAndOutput);
}

void TestElementLogic::testNand_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("expected");

    QTest::newRow("1 NAND 1 = 0") << true << true << false;
    QTest::newRow("1 NAND 0 = 1") << true << false << true;
    QTest::newRow("0 NAND 1 = 1") << false << true << true;
    QTest::newRow("0 NAND 0 = 1") << false << false << true;
}

void TestElementLogic::testNand()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, expected);

    Nand elm; initTEElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestElementLogic::testNor_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("expected");

    QTest::newRow("1 NOR 1 = 0") << true << true << false;
    QTest::newRow("1 NOR 0 = 0") << true << false << false;
    QTest::newRow("0 NOR 1 = 0") << false << true << false;
    QTest::newRow("0 NOR 0 = 1") << false << false << true;
}

void TestElementLogic::testNor()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, expected);

    Nor elm; initTEElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestElementLogic::testXor_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("expected");

    QTest::newRow("0 XOR 0 = 0") << false << false << false;
    QTest::newRow("0 XOR 1 = 1") << false << true << true;
    QTest::newRow("1 XOR 0 = 1") << true << false << true;
    QTest::newRow("1 XOR 1 = 0") << true << true << false;
}

void TestElementLogic::testXor()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, expected);

    Xor elm; initTEElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestElementLogic::testSRLatch_data()
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

void TestElementLogic::testSRLatch()
{
    QFETCH(bool, set);
    QFETCH(bool, reset);
    QFETCH(bool, prevState);
    QFETCH(bool, expectedQ);

    SRLatch elm; initTEElm(elm);
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

void TestElementLogic::test3InputAnd_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("input3");
    QTest::addColumn<bool>("expected");

    QTest::newRow("0 AND 0 AND 0 = 0") << false << false << false << false;
    QTest::newRow("0 AND 0 AND 1 = 0") << false << false << true << false;
    QTest::newRow("0 AND 1 AND 0 = 0") << false << true << false << false;
    QTest::newRow("0 AND 1 AND 1 = 0") << false << true << true << false;
    QTest::newRow("1 AND 0 AND 0 = 0") << true << false << false << false;
    QTest::newRow("1 AND 0 AND 1 = 0") << true << false << true << false;
    QTest::newRow("1 AND 1 AND 0 = 0") << true << true << false << false;
    QTest::newRow("1 AND 1 AND 1 = 1") << true << true << true << true;
}

void TestElementLogic::test3InputAnd()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, input3);
    QFETCH(bool, expected);

    And elm; elm.setInputSize(3); initTEElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);
    elm.connectPredecessor(2, m_inputs.at(2), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);
    m_inputs.at(2)->setOutputValue(input3);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestElementLogic::test3InputOr_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("input3");
    QTest::addColumn<bool>("expected");

    QTest::newRow("0 OR 0 OR 0 = 0") << false << false << false << false;
    QTest::newRow("0 OR 0 OR 1 = 1") << false << false << true << true;
    QTest::newRow("0 OR 1 OR 0 = 1") << false << true << false << true;
    QTest::newRow("0 OR 1 OR 1 = 1") << false << true << true << true;
    QTest::newRow("1 OR 0 OR 0 = 1") << true << false << false << true;
    QTest::newRow("1 OR 0 OR 1 = 1") << true << false << true << true;
    QTest::newRow("1 OR 1 OR 0 = 1") << true << true << false << true;
    QTest::newRow("1 OR 1 OR 1 = 1") << true << true << true << true;
}

void TestElementLogic::test3InputOr()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, input3);
    QFETCH(bool, expected);

    Or elm; elm.setInputSize(3); initTEElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);
    elm.connectPredecessor(2, m_inputs.at(2), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);
    m_inputs.at(2)->setOutputValue(input3);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected);
}

void TestElementLogic::test4InputAnd_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("input3");
    QTest::addColumn<bool>("input4");
    QTest::addColumn<bool>("expected");

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

void TestElementLogic::test4InputAnd()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, input3);
    QFETCH(bool, input4);
    QFETCH(bool, expected);

    And elm; elm.setInputSize(4); initTEElm(elm);
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

void TestElementLogic::test4InputOr_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("input3");
    QTest::addColumn<bool>("input4");
    QTest::addColumn<bool>("expected");

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

void TestElementLogic::test4InputOr()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, input3);
    QFETCH(bool, input4);
    QFETCH(bool, expected);

    Or elm; elm.setInputSize(4); initTEElm(elm);
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

void TestElementLogic::test3InputXor_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("input3");
    QTest::addColumn<bool>("expected");

    QTest::newRow("0 XOR 0 XOR 0 = 0") << false << false << false << false;
    QTest::newRow("0 XOR 0 XOR 1 = 1") << false << false << true << true;
    QTest::newRow("0 XOR 1 XOR 0 = 1") << false << true << false << true;
    QTest::newRow("0 XOR 1 XOR 1 = 0") << false << true << true << false;
    QTest::newRow("1 XOR 0 XOR 0 = 1") << true << false << false << true;
    QTest::newRow("1 XOR 0 XOR 1 = 0") << true << false << true << false;
    QTest::newRow("1 XOR 1 XOR 0 = 0") << true << true << false << false;
    QTest::newRow("1 XOR 1 XOR 1 = 1") << true << true << true << true;
}

void TestElementLogic::test3InputXor()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, input3);
    QFETCH(bool, expected);

    // Use 2-input XOR gates to simulate 3-input: (a XOR b) XOR c
    Xor xor1, xor2; initTEElm(xor1); initTEElm(xor2);
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

void TestElementLogic::test3InputXnor_data()
{
    QTest::addColumn<bool>("input1");
    QTest::addColumn<bool>("input2");
    QTest::addColumn<bool>("input3");
    QTest::addColumn<bool>("expected");

    QTest::newRow("0 XNOR 0 XNOR 0 = 0") << false << false << false << false;
    QTest::newRow("0 XNOR 0 XNOR 1 = 1") << false << false << true << true;
    QTest::newRow("0 XNOR 1 XNOR 0 = 1") << false << true << false << true;
    QTest::newRow("0 XNOR 1 XNOR 1 = 0") << false << true << true << false;
    QTest::newRow("1 XNOR 0 XNOR 0 = 1") << true << false << false << true;
    QTest::newRow("1 XNOR 0 XNOR 1 = 0") << true << false << true << false;
    QTest::newRow("1 XNOR 1 XNOR 0 = 0") << true << true << false << false;
    QTest::newRow("1 XNOR 1 XNOR 1 = 1") << true << true << true << true;
}

void TestElementLogic::test3InputXnor()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, input3);
    QFETCH(bool, expected);

    // Use 2-input XNOR gates to simulate 3-input: (a XNOR b) XNOR c
    Xnor xnor1, xnor2; initTEElm(xnor1); initTEElm(xnor2);
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

void TestElementLogic::test5InputAnd_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("expected");

    for (int i = 0; i < 32; ++i) {
        bool expected = (i == 31);  // Only all-1s = 1
        QTest::newRow(QString("5AND case %1").arg(i).toLatin1().data()) << i << expected;
    }
}

void TestElementLogic::test5InputAnd()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    And elm; elm.setInputSize(5); initTEElm(elm);
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

void TestElementLogic::test5InputOr_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("expected");

    for (int i = 0; i < 32; ++i) {
        bool expected = (i != 0);  // Only all-0s = 0
        QTest::newRow(QString("5OR case %1").arg(i).toLatin1().data()) << i << expected;
    }
}

void TestElementLogic::test5InputOr()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    Or elm; elm.setInputSize(5); initTEElm(elm);
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

void TestElementLogic::test5InputNand_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("expected");

    for (int i = 0; i < 32; ++i) {
        bool expected = (i != 31);  // All-1s = 0, rest = 1
        QTest::newRow(QString("5NAND case %1").arg(i).toLatin1().data()) << i << expected;
    }
}

void TestElementLogic::test5InputNand()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    Nand elm; elm.setInputSize(5); initTEElm(elm);
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

void TestElementLogic::test5InputNor_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("expected");

    for (int i = 0; i < 32; ++i) {
        bool expected = (i == 0);  // All-0s = 1, rest = 0
        QTest::newRow(QString("5NOR case %1").arg(i).toLatin1().data()) << i << expected;
    }
}

void TestElementLogic::test5InputNor()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    Nor elm; elm.setInputSize(5); initTEElm(elm);
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

void TestElementLogic::test8InputAnd_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("expected");

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

void TestElementLogic::test8InputAnd()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    And elm; elm.setInputSize(8); initTEElm(elm);
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

void TestElementLogic::test8InputOr_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("expected");

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

void TestElementLogic::test8InputOr()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    Or elm; elm.setInputSize(8); initTEElm(elm);
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

void TestElementLogic::test8InputNand_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("expected");

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

void TestElementLogic::test8InputNand()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    Nand elm; elm.setInputSize(8); initTEElm(elm);
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

void TestElementLogic::test8InputNor_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("expected");

    QTest::newRow("all 0s") << 0 << true;
    QTest::newRow("all 1s") << 255 << false;
    QTest::newRow("first 1 only") << 1 << false;
    QTest::newRow("first 4 set") << 15 << false;
    QTest::newRow("first 7 set") << 127 << false;
    QTest::newRow("all but last") << 254 << false;
    QTest::newRow("last 1 only") << 128 << false;
    QTest::newRow("alternating even") << 0x55 << false;  // 01010101
    QTest::newRow("alternating odd") << 0xAA << false;   // 10101010
    QTest::newRow("single middle bit") << 0x10 << false;  // 00010000
}

void TestElementLogic::test8InputNor()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    Nor elm; elm.setInputSize(8); initTEElm(elm);
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

void TestElementLogic::testFanOut_data()
{
    QTest::addColumn<bool>("inputValue");
    QTest::addColumn<bool>("expectedAnd");
    QTest::addColumn<bool>("expectedOr");
    QTest::addColumn<bool>("expectedXor");

    QTest::newRow("all gates input=0") << false << false << false << false;
    QTest::newRow("all gates input=1") << true << true << true << false;
}

void TestElementLogic::testFanOut()
{
    QFETCH(bool, inputValue);
    QFETCH(bool, expectedAnd);
    QFETCH(bool, expectedOr);
    QFETCH(bool, expectedXor);

    InputVcc source; initTESrc(source);
    And andGate; initTEElm(andGate);
    Or orGate; initTEElm(orGate);
    Xor xorGate; initTEElm(xorGate);

    andGate.connectPredecessor(0, &source, 0);
    andGate.connectPredecessor(1, &source, 0);

    orGate.connectPredecessor(0, &source, 0);
    orGate.connectPredecessor(1, &source, 0);

    xorGate.connectPredecessor(0, &source, 0);
    xorGate.connectPredecessor(1, &source, 0);

    source.setOutputValue(inputValue);

    andGate.updateLogic();
    orGate.updateLogic();
    xorGate.updateLogic();

    QCOMPARE(andGate.outputValue(), expectedAnd);
    QCOMPARE(orGate.outputValue(), expectedOr);
    QCOMPARE(xorGate.outputValue(), expectedXor);
}

void TestElementLogic::test6InputAnd_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("expected");

    QTest::newRow("all 0s") << 0 << false;
    QTest::newRow("all 1s") << 63 << true;
    QTest::newRow("5 set, last 0") << 31 << false;
    QTest::newRow("last 5 set, first 0") << 62 << false;
    QTest::newRow("alternating 01") << 0x15 << false;
    QTest::newRow("alternating 10") << 0x2A << false;
}

void TestElementLogic::test6InputAnd()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    And elm; elm.setInputSize(6); initTEElm(elm);
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

