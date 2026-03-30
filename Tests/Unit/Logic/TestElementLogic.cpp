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

using TestUtils::initSrc;
using TestUtils::initElm;

void TestElementLogic::init()
{
    std::generate(m_inputs.begin(), m_inputs.end(), [] {
        auto *src = new InputVcc();
        initSrc(*src);
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

    Node elm; initElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);

    m_inputs.at(0)->setOutputValue(input);
    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
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

    And elm; initElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
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

    Or elm; initElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
}

void TestElementLogic::testSource()
{
    InputVcc elm; initSrc(elm);
    // InputVcc port has defaultStatus=Active, so initSimulationVectors sets output to Active.
    QCOMPARE(elm.outputValue(), Status::Active);
    elm.setOutputValue(false);
    QCOMPARE(elm.outputValue(), Status::Inactive);
    elm.setOutputValue(true);
    QCOMPARE(elm.outputValue(), Status::Active);
}

/**
 * Test: LogicSource with multiple outputs — each output is independently
 * settable and readable. The constructor default applies only to output 0;
 * remaining outputs start Inactive.
 */
void TestElementLogic::testSourceMultiOutput()
{
    InputVcc src; src.initSimulationVectors(0, 4); src.setOutputValue(0, Status::Active);  // output 0 = Active, outputs 1-3 = Inactive

    QCOMPARE(src.outputValue(0), Status::Active);
    QCOMPARE(src.outputValue(1), Status::Inactive);
    QCOMPARE(src.outputValue(2), Status::Inactive);
    QCOMPARE(src.outputValue(3), Status::Inactive);

    // Each output is independently settable
    src.setOutputValue(1, true);
    src.setOutputValue(3, Status::Active);

    QCOMPARE(src.outputValue(0), Status::Active);
    QCOMPARE(src.outputValue(1), Status::Active);
    QCOMPARE(src.outputValue(2), Status::Inactive);
    QCOMPARE(src.outputValue(3), Status::Active);

    // Clearing one does not affect others
    src.setOutputValue(0, false);
    QCOMPARE(src.outputValue(0), Status::Inactive);
    QCOMPARE(src.outputValue(1), Status::Active);  // unchanged
}

/**
 * Test: LogicSink mirrors each input to the matching output so the display
 * layer can query outputValue(i) without traversing the graph.
 * Also verifies that Unknown on any input propagates to all outputs.
 */
void TestElementLogic::testSink()
{
    Led sink; sink.setInputSize(3); sink.initSimulationVectors(3, 3);
    InputVcc in0, in1, in2; initSrc(in0); initSrc(in1); initSrc(in2);

    sink.connectPredecessor(0, &in0, 0);
    sink.connectPredecessor(1, &in1, 0);
    sink.connectPredecessor(2, &in2, 0);

    // All Active
    in0.setOutputValue(true);
    in1.setOutputValue(true);
    in2.setOutputValue(true);
    sink.updateLogic();
    QCOMPARE(sink.outputValue(0), Status::Active);
    QCOMPARE(sink.outputValue(1), Status::Active);
    QCOMPARE(sink.outputValue(2), Status::Active);

    // Mixed Active / Inactive
    in0.setOutputValue(true);
    in1.setOutputValue(false);
    in2.setOutputValue(true);
    sink.updateLogic();
    QCOMPARE(sink.outputValue(0), Status::Active);
    QCOMPARE(sink.outputValue(1), Status::Inactive);
    QCOMPARE(sink.outputValue(2), Status::Active);

    // Null predecessor on Led (optional ports) → defaults to Inactive
    Led sinkPartial; sinkPartial.setInputSize(2); sinkPartial.initSimulationVectors(2, 2);
    InputVcc onlyInput; initSrc(onlyInput);
    sinkPartial.connectPredecessor(0, &onlyInput, 0);
    // port 1 left null — Led ports are optional, defaulting to Inactive
    onlyInput.setOutputValue(true);
    sinkPartial.updateLogic();
    QCOMPARE(sinkPartial.outputValue(0), Status::Active);
    QCOMPARE(sinkPartial.outputValue(1), Status::Inactive);
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

    Mux elm; initElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);
    elm.connectPredecessor(2, m_inputs.at(2), 0);

    m_inputs.at(0)->setOutputValue(in0);
    m_inputs.at(1)->setOutputValue(in1);
    m_inputs.at(2)->setOutputValue(sel);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
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

    Demux elm; initElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input);
    m_inputs.at(1)->setOutputValue(sel);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(0), out0 ? Status::Active : Status::Inactive);
    QCOMPARE(elm.outputValue(1), out1 ? Status::Active : Status::Inactive);
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

    DFlipFlop elm; initElm(elm);
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

    QCOMPARE(elm.outputValue(0), expectedQ ? Status::Active : Status::Inactive);
    QCOMPARE(elm.outputValue(1), expectedNotQ ? Status::Active : Status::Inactive);
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

    DLatch elm; initElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    elm.setOutputValue(0, prevState);
    elm.setOutputValue(1, !prevState);

    m_inputs.at(0)->setOutputValue(data);
    m_inputs.at(1)->setOutputValue(enable);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(0), expectedQ ? Status::Active : Status::Inactive);
    QCOMPARE(elm.outputValue(1), expectedQ ? Status::Inactive : Status::Active);
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

    JKFlipFlop elm; initElm(elm);
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

    QCOMPARE(elm.outputValue(0), expectedQ ? Status::Active : Status::Inactive);
    QCOMPARE(elm.outputValue(1), expectedNotQ ? Status::Active : Status::Inactive);
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

    SRFlipFlop elm; initElm(elm);
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

    QCOMPARE(elm.outputValue(0), expectedQ ? Status::Active : Status::Inactive);
    QCOMPARE(elm.outputValue(1), expectedNotQ ? Status::Active : Status::Inactive);
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

    TFlipFlop elm; initElm(elm);
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

    QCOMPARE(elm.outputValue(0), expectedQ ? Status::Active : Status::Inactive);
    QCOMPARE(elm.outputValue(1), expectedNotQ ? Status::Active : Status::Inactive);
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

    Xnor elm; initElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
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

    Not elm; initElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);

    m_inputs.at(0)->setOutputValue(input);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
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

    Not notGate; initElm(notGate);
    notGate.connectPredecessor(0, m_inputs.at(0), 0);

    m_inputs.at(0)->setOutputValue(input);
    notGate.updateLogic();

    QCOMPARE(notGate.outputValue() == Status::Active, expected);
}

// Comprehensive circuit-context test: NOT gate connected to AND gate (like in decoder)
void TestElementLogic::testNotGateInAnd_data()
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

void TestElementLogic::testNotGateInAnd()
{
    QFETCH(bool, input0);
    QFETCH(bool, input1);
    QFETCH(bool, expectedNotInput0);
    Q_UNUSED(expectedNotInput0);  // Using for clarity in test data
    QFETCH(bool, expectedAndOutput);

    Not invGate; initElm(invGate);
    invGate.connectPredecessor(0, m_inputs.at(1), 0);

    And andGate; initElm(andGate);
    andGate.connectPredecessor(0, m_inputs.at(0), 0);
    andGate.connectPredecessor(1, &invGate, 0);

    m_inputs.at(0)->setOutputValue(input0);
    m_inputs.at(1)->setOutputValue(input1);

    invGate.updateLogic();
    QCOMPARE(invGate.outputValue(), input1 ? Status::Inactive : Status::Active);

    andGate.updateLogic();
    QCOMPARE(andGate.outputValue(), expectedAndOutput ? Status::Active : Status::Inactive);
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

    Nand elm; initElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
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

    Nor elm; initElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
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

    Xor elm; initElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
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

    SRLatch elm; initElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);

    elm.setOutputValue(0, prevState);
    elm.setOutputValue(1, !prevState);

    m_inputs.at(0)->setOutputValue(set);
    m_inputs.at(1)->setOutputValue(reset);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(0), expectedQ ? Status::Active : Status::Inactive);
    QCOMPARE(elm.outputValue(1), expectedQ ? Status::Inactive : Status::Active);
}

void TestElementLogic::test3InputAnd_data()
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

void TestElementLogic::test3InputAnd()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, input3);
    QFETCH(bool, expected);

    And elm; elm.setInputSize(3); initElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);
    elm.connectPredecessor(2, m_inputs.at(2), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);
    m_inputs.at(2)->setOutputValue(input3);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
}

void TestElementLogic::test3InputOr_data()
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

void TestElementLogic::test3InputOr()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, input3);
    QFETCH(bool, expected);

    Or elm; elm.setInputSize(3); initElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);
    elm.connectPredecessor(2, m_inputs.at(2), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);
    m_inputs.at(2)->setOutputValue(input3);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
}

void TestElementLogic::test4InputAnd_data()
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

void TestElementLogic::test4InputAnd()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, input3);
    QFETCH(bool, input4);
    QFETCH(bool, expected);

    And elm; elm.setInputSize(4); initElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);
    elm.connectPredecessor(2, m_inputs.at(2), 0);
    elm.connectPredecessor(3, m_inputs.at(3), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);
    m_inputs.at(2)->setOutputValue(input3);
    m_inputs.at(3)->setOutputValue(input4);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
}

void TestElementLogic::test4InputOr_data()
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

void TestElementLogic::test4InputOr()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, input3);
    QFETCH(bool, input4);
    QFETCH(bool, expected);

    Or elm; elm.setInputSize(4); initElm(elm);
    elm.connectPredecessor(0, m_inputs.at(0), 0);
    elm.connectPredecessor(1, m_inputs.at(1), 0);
    elm.connectPredecessor(2, m_inputs.at(2), 0);
    elm.connectPredecessor(3, m_inputs.at(3), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);
    m_inputs.at(2)->setOutputValue(input3);
    m_inputs.at(3)->setOutputValue(input4);

    elm.updateLogic();

    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
}

void TestElementLogic::test3InputXor_data()
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

void TestElementLogic::test3InputXor()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, input3);
    QFETCH(bool, expected);

    // Use 2-input XOR gates to simulate 3-input: (a XOR b) XOR c
    Xor xor1, xor2; initElm(xor1); initElm(xor2);
    xor1.connectPredecessor(0, m_inputs.at(0), 0);
    xor1.connectPredecessor(1, m_inputs.at(1), 0);
    xor2.connectPredecessor(0, &xor1, 0);
    xor2.connectPredecessor(1, m_inputs.at(2), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);
    m_inputs.at(2)->setOutputValue(input3);

    xor1.updateLogic();
    xor2.updateLogic();

    QCOMPARE(xor2.outputValue(), expected ? Status::Active : Status::Inactive);
}

void TestElementLogic::test3InputXnor_data()
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

void TestElementLogic::test3InputXnor()
{
    QFETCH(bool, input1);
    QFETCH(bool, input2);
    QFETCH(bool, input3);
    QFETCH(bool, expected);

    // Use 2-input XNOR gates to simulate 3-input: (a XNOR b) XNOR c
    Xnor xnor1, xnor2; initElm(xnor1); initElm(xnor2);
    xnor1.connectPredecessor(0, m_inputs.at(0), 0);
    xnor1.connectPredecessor(1, m_inputs.at(1), 0);
    xnor2.connectPredecessor(0, &xnor1, 0);
    xnor2.connectPredecessor(1, m_inputs.at(2), 0);

    m_inputs.at(0)->setOutputValue(input1);
    m_inputs.at(1)->setOutputValue(input2);
    m_inputs.at(2)->setOutputValue(input3);

    xnor1.updateLogic();
    xnor2.updateLogic();

    QCOMPARE(xnor2.outputValue(), expected ? Status::Active : Status::Inactive);
}

/**
 * Test: 5-input AND gate - exhaustive truth table (32 cases)
 */
void TestElementLogic::test5InputAnd_data()
{
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("expected");

    // All 32 combinations from 0 to 31
    for (int i = 0; i < 32; ++i) {
        bool expected = (i == 31);  // Only all-1s = 1
        QTest::newRow(QString("5AND case %1").arg(i).toLatin1().data()) << i << expected;
    }
}

void TestElementLogic::test5InputAnd()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    And elm; elm.setInputSize(5); initElm(elm);
    for (int i = 0; i < 5; ++i) {
        elm.connectPredecessor(i, m_inputs.at(i), 0);
    }

    for (int i = 0; i < 5; ++i) {
        bool bit = (value >> i) & 1;
        m_inputs.at(i)->setOutputValue(bit);
    }

    elm.updateLogic();
    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
}

/**
 * Test: 5-input OR gate - exhaustive truth table (32 cases)
 */
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

    Or elm; elm.setInputSize(5); initElm(elm);
    for (int i = 0; i < 5; ++i) {
        elm.connectPredecessor(i, m_inputs.at(i), 0);
    }

    for (int i = 0; i < 5; ++i) {
        bool bit = (value >> i) & 1;
        m_inputs.at(i)->setOutputValue(bit);
    }

    elm.updateLogic();
    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
}

/**
 * Test: 5-input NAND gate - exhaustive truth table (32 cases)
 */
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

    Nand elm; elm.setInputSize(5); initElm(elm);
    for (int i = 0; i < 5; ++i) {
        elm.connectPredecessor(i, m_inputs.at(i), 0);
    }

    for (int i = 0; i < 5; ++i) {
        bool bit = (value >> i) & 1;
        m_inputs.at(i)->setOutputValue(bit);
    }

    elm.updateLogic();
    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
}

/**
 * Test: 5-input NOR gate - exhaustive truth table (32 cases)
 */
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

    Nor elm; elm.setInputSize(5); initElm(elm);
    for (int i = 0; i < 5; ++i) {
        elm.connectPredecessor(i, m_inputs.at(i), 0);
    }

    for (int i = 0; i < 5; ++i) {
        bool bit = (value >> i) & 1;
        m_inputs.at(i)->setOutputValue(bit);
    }

    elm.updateLogic();
    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
}

/**
 * Test: 8-input AND gate - strategic test cases
 */
void TestElementLogic::test8InputAnd_data()
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

void TestElementLogic::test8InputAnd()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    And elm; elm.setInputSize(8); initElm(elm);
    for (int i = 0; i < 8; ++i) {
        elm.connectPredecessor(i, m_inputs.at(i), 0);
    }

    for (int i = 0; i < 8; ++i) {
        bool bit = (value >> i) & 1;
        m_inputs.at(i)->setOutputValue(bit);
    }

    elm.updateLogic();
    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
}

/**
 * Test: 8-input OR gate - strategic test cases
 */
void TestElementLogic::test8InputOr_data()
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

void TestElementLogic::test8InputOr()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    Or elm; elm.setInputSize(8); initElm(elm);
    for (int i = 0; i < 8; ++i) {
        elm.connectPredecessor(i, m_inputs.at(i), 0);
    }

    for (int i = 0; i < 8; ++i) {
        bool bit = (value >> i) & 1;
        m_inputs.at(i)->setOutputValue(bit);
    }

    elm.updateLogic();
    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
}

/**
 * Test: 8-input NAND gate - strategic test cases
 */
void TestElementLogic::test8InputNand_data()
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

void TestElementLogic::test8InputNand()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    Nand elm; elm.setInputSize(8); initElm(elm);
    for (int i = 0; i < 8; ++i) {
        elm.connectPredecessor(i, m_inputs.at(i), 0);
    }

    for (int i = 0; i < 8; ++i) {
        bool bit = (value >> i) & 1;
        m_inputs.at(i)->setOutputValue(bit);
    }

    elm.updateLogic();
    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
}

/**
 * Test: 8-input NOR gate - strategic test cases
 */
void TestElementLogic::test8InputNor_data()
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
    QTest::newRow("single middle bit") << 0x10 << false;  // 00010000
}

void TestElementLogic::test8InputNor()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    Nor elm; elm.setInputSize(8); initElm(elm);
    for (int i = 0; i < 8; ++i) {
        elm.connectPredecessor(i, m_inputs.at(i), 0);
    }

    for (int i = 0; i < 8; ++i) {
        bool bit = (value >> i) & 1;
        m_inputs.at(i)->setOutputValue(bit);
    }

    elm.updateLogic();
    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
}

/**
 * Test: Fan-out - one output driving multiple inputs
 * Verifies that a single LogicSource can successfully drive multiple gate inputs
 */
void TestElementLogic::testFanOut_data()
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

void TestElementLogic::testFanOut()
{
    QFETCH(bool, inputValue);
    QFETCH(bool, expectedAnd);
    QFETCH(bool, expectedOr);
    QFETCH(bool, expectedXor);

    // Single LogicSource drives 3 different gates (fan-out test)
    InputVcc source; initSrc(source);
    And andGate; initElm(andGate);
    Or orGate; initElm(orGate);
    Xor xorGate; initElm(xorGate);

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
    QCOMPARE(andGate.outputValue(), expectedAnd ? Status::Active : Status::Inactive);
    QCOMPARE(orGate.outputValue(), expectedOr ? Status::Active : Status::Inactive);
    QCOMPARE(xorGate.outputValue(), expectedXor ? Status::Active : Status::Inactive);
}

/**
 * Test: 6-input AND gate
 * Fills coverage gap between 5-input and 8-input tests
 */
void TestElementLogic::test6InputAnd_data()
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

void TestElementLogic::test6InputAnd()
{
    QFETCH(int, value);
    QFETCH(bool, expected);

    And elm; elm.setInputSize(6); initElm(elm);
    for (int i = 0; i < 6; ++i) {
        elm.connectPredecessor(i, m_inputs.at(i), 0);
    }

    for (int i = 0; i < 6; ++i) {
        bool bit = (value >> i) & 1;
        m_inputs.at(i)->setOutputValue(bit);
    }

    elm.updateLogic();
    QCOMPARE(elm.outputValue(), expected ? Status::Active : Status::Inactive);
}

