// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Integration/Logic/TestMuxDemuxComprehensive.h"

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/Demux.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/GraphicElements/Mux.h"
#include "App/Element/GraphicElements/Not.h"
#include "App/Element/GraphicElements/Or.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/Integration/IC/Tests/Cpu/CpuCommon.h"

using TestUtils::getInputStatus;

void TestMUXDEMUXComprehensive::initTestCase()
{
}

void TestMUXDEMUXComprehensive::cleanup()
{
}

// ============================================================
// MUX TESTS
// ============================================================

void TestMUXDEMUXComprehensive::testMux2to1_data()
{
    QTest::addColumn<int>("dataInput0");
    QTest::addColumn<int>("dataInput1");
    QTest::addColumn<int>("selectLine");
    QTest::addColumn<int>("expectedOutput");

    // Basic functionality: select each input
    QTest::newRow("2to1_select0_data0on") << 1 << 0 << 0 << 1;
    QTest::newRow("2to1_select0_data0off") << 0 << 1 << 0 << 0;
    QTest::newRow("2to1_select1_data1on") << 0 << 1 << 1 << 1;
    QTest::newRow("2to1_select1_data1off") << 1 << 0 << 1 << 0;

    // All data inputs off
    QTest::newRow("2to1_all_off_select0") << 0 << 0 << 0 << 0;
    QTest::newRow("2to1_all_off_select1") << 0 << 0 << 1 << 0;

    // All data inputs on
    QTest::newRow("2to1_all_on_select0") << 1 << 1 << 0 << 1;
    QTest::newRow("2to1_all_on_select1") << 1 << 1 << 1 << 1;
}

void TestMUXDEMUXComprehensive::testMux2to1()
{
    QFETCH(int, dataInput0);
    QFETCH(int, dataInput1);
    QFETCH(int, selectLine);
    QFETCH(int, expectedOutput);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch in0, in1, sel;
    Led out;
    Mux mux;

    builder.add(&in0, &in1, &sel, &out, &mux);

    // Connect 2-to-1 mux: 2 data inputs (ports 0,1) + 1 select (port 2)
    builder.connect(&in0, 0, &mux, 0);
    builder.connect(&in1, 0, &mux, 1);
    builder.connect(&sel, 0, &mux, 2);
    builder.connect(&mux, 0, &out, 0);

    auto *simulation = builder.initSimulation();

    in0.setOn(dataInput0);
    in1.setOn(dataInput1);
    sel.setOn(selectLine);
    simulation->update();

    int output = getInputStatus(&out) ? 1 : 0;
    QCOMPARE(output, expectedOutput);
}

void TestMUXDEMUXComprehensive::testMux4to1_data()
{
    QTest::addColumn<QVector<int>>("dataInputs");
    QTest::addColumn<int>("selectValue");
    QTest::addColumn<int>("expectedOutput");

    // Test all 4 select combinations with different data patterns
    QTest::newRow("4to1_select0") << QVector<int>{1, 0, 0, 0} << 0 << 1;
    QTest::newRow("4to1_select1") << QVector<int>{0, 1, 0, 0} << 1 << 1;
    QTest::newRow("4to1_select2") << QVector<int>{0, 0, 1, 0} << 2 << 1;
    QTest::newRow("4to1_select3") << QVector<int>{0, 0, 0, 1} << 3 << 1;

    // All inputs on - any select should output 1
    QTest::newRow("4to1_all_on_select0") << QVector<int>{1, 1, 1, 1} << 0 << 1;
    QTest::newRow("4to1_all_on_select3") << QVector<int>{1, 1, 1, 1} << 3 << 1;

    // All inputs off - any select should output 0
    QTest::newRow("4to1_all_off_select0") << QVector<int>{0, 0, 0, 0} << 0 << 0;
    QTest::newRow("4to1_all_off_select3") << QVector<int>{0, 0, 0, 0} << 3 << 0;

    // Alternating pattern
    QTest::newRow("4to1_alternating_select0") << QVector<int>{1, 0, 1, 0} << 0 << 1;
    QTest::newRow("4to1_alternating_select1") << QVector<int>{1, 0, 1, 0} << 1 << 0;
    QTest::newRow("4to1_alternating_select2") << QVector<int>{1, 0, 1, 0} << 2 << 1;
    QTest::newRow("4to1_alternating_select3") << QVector<int>{1, 0, 1, 0} << 3 << 0;
}

void TestMUXDEMUXComprehensive::testMux4to1()
{
    QFETCH(QVector<int>, dataInputs);
    QFETCH(int, selectValue);
    QFETCH(int, expectedOutput);

    QCOMPARE(dataInputs.size(), 4);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    QVector<InputSwitch *> dataIn(4);
    for (int i = 0; i < 4; ++i) dataIn[i] = new InputSwitch();
    InputSwitch sel0, sel1;
    Led out;
    Mux mux;

    // Resize mux to 4-to-1 (6 ports: 4 data + 2 select)
    mux.setInputSize(6);

    for (int i = 0; i < 4; ++i) {
        builder.add(dataIn[i]);
    }
    builder.add(&sel0, &sel1, &out, &mux);

    // Connect 4-to-1 mux: 4 data inputs (0-3) + 2 select lines (4-5)
    for (int i = 0; i < 4; ++i) {
        builder.connect(dataIn[i], 0, &mux, i);
    }
    builder.connect(&sel0, 0, &mux, 4);
    builder.connect(&sel1, 0, &mux, 5);
    builder.connect(&mux, 0, &out, 0);

    auto *simulation = builder.initSimulation();

    for (int i = 0; i < 4; ++i) {
        dataIn[i]->setOn(dataInputs[i]);
    }
    sel0.setOn((selectValue >> 0) & 1);
    sel1.setOn((selectValue >> 1) & 1);
    simulation->update();

    int output = getInputStatus(&out) ? 1 : 0;
    QCOMPARE(output, expectedOutput);
}

void TestMUXDEMUXComprehensive::testMux8to1_data()
{
    QTest::addColumn<int>("selectIndex");
    QTest::addColumn<int>("expectedOutput");

    // Test all 8 select combinations
    for (int i = 0; i < 8; ++i) {
        QTest::newRow(QString("8to1_select%1").arg(i).toLatin1().constData()) << i << 1;
    }
}

void TestMUXDEMUXComprehensive::testMux8to1()
{
    QFETCH(int, selectIndex);
    QFETCH(int, expectedOutput);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    QVector<InputSwitch *> dataIn(8);
    for (int i = 0; i < 8; ++i) dataIn[i] = new InputSwitch();
    QVector<InputSwitch *> selectLines(3);
    for (int i = 0; i < 3; ++i) selectLines[i] = new InputSwitch();
    Led out;
    Mux mux;

    // Resize mux to 8-to-1 (11 ports: 8 data + 3 select)
    mux.setInputSize(11);

    for (int i = 0; i < 8; ++i) {
        builder.add(dataIn[i]);
    }
    for (int i = 0; i < 3; ++i) {
        builder.add(selectLines[i]);
    }
    builder.add(&out, &mux);

    // Connect 8-to-1 mux: 8 data inputs (0-7) + 3 select lines (8-10)
    for (int i = 0; i < 8; ++i) {
        builder.connect(dataIn[i], 0, &mux, i);
    }
    for (int i = 0; i < 3; ++i) {
        builder.connect(selectLines[i], 0, &mux, 8 + i);
    }
    builder.connect(&mux, 0, &out, 0);

    auto *simulation = builder.initSimulation();

    // Only selected input is on
    dataIn[selectIndex]->setOn(true);
    for (int i = 0; i < 3; ++i) {
        selectLines[i]->setOn((selectIndex >> i) & 1);
    }
    simulation->update();

    int output = getInputStatus(&out) ? 1 : 0;
    QCOMPARE(output, expectedOutput);

    // Verify all other inputs output 0 when selected (because they remain OFF)
    for (int idx = 0; idx < 8; ++idx) {
        if (idx == selectIndex) continue;

        for (int i = 0; i < 3; ++i) {
            selectLines[i]->setOn((idx >> i) & 1);
        }
        simulation->update();

        output = getInputStatus(&out) ? 1 : 0;
        QCOMPARE(output, 0); // dataIn[idx] is OFF, so output should be 0
    }
}

void TestMUXDEMUXComprehensive::testMuxSelectLineTransitions_data()
{
    QTest::addColumn<int>("initialSelect");
    QTest::addColumn<int>("finalSelect");
    QTest::addColumn<int>("initialExpected");
    QTest::addColumn<int>("finalExpected");

    // Test rapid select changes
    QTest::newRow("4to1_transition_0to1") << 0 << 1 << 1 << 1;
    QTest::newRow("4to1_transition_0to3") << 0 << 3 << 1 << 1;
    QTest::newRow("4to1_transition_3to0") << 3 << 0 << 1 << 1;
    QTest::newRow("4to1_transition_1to2") << 1 << 2 << 1 << 1;
}

void TestMUXDEMUXComprehensive::testMuxSelectLineTransitions()
{
    QFETCH(int, initialSelect);
    QFETCH(int, finalSelect);
    QFETCH(int, initialExpected);
    QFETCH(int, finalExpected);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    QVector<InputSwitch *> dataIn(4);
    for (int i = 0; i < 4; ++i) dataIn[i] = new InputSwitch();
    InputSwitch sel0, sel1;
    Led out;
    Mux mux;

    // Resize mux to 4-to-1 (6 ports: 4 data + 2 select)
    mux.setInputSize(6);

    for (int i = 0; i < 4; ++i) {
        builder.add(dataIn[i]);
    }
    builder.add(&sel0, &sel1, &out, &mux);

    for (int i = 0; i < 4; ++i) {
        builder.connect(dataIn[i], 0, &mux, i);
    }
    builder.connect(&sel0, 0, &mux, 4);
    builder.connect(&sel1, 0, &mux, 5);
    builder.connect(&mux, 0, &out, 0);

    auto *simulation = builder.initSimulation();

    // All data inputs on
    for (int i = 0; i < 4; ++i) {
        dataIn[i]->setOn(true);
    }

    // Set initial select
    sel0.setOn((initialSelect >> 0) & 1);
    sel1.setOn((initialSelect >> 1) & 1);
    simulation->update();

    int output = getInputStatus(&out) ? 1 : 0;
    QCOMPARE(output, initialExpected);

    // Transition to final select
    sel0.setOn((finalSelect >> 0) & 1);
    sel1.setOn((finalSelect >> 1) & 1);
    simulation->update();

    output = getInputStatus(&out) ? 1 : 0;
    QCOMPARE(output, finalExpected);
}

void TestMUXDEMUXComprehensive::testMuxCascaded_data()
{
    QTest::addColumn<int>("primarySelect");
    QTest::addColumn<int>("secondarySelect");
    QTest::addColumn<int>("expectedOutput");

    // 4 muxes (2-to-1 each) -> 1 final mux (2-to-1)
    // This creates a 4-to-1 mux in cascaded form
    for (int i = 0; i < 4; ++i) {
        QTest::newRow(QString("cascaded_select%1").arg(i).toLatin1().constData())
            << (i & 1) << ((i >> 1) & 1) << 1;
    }
}

void TestMUXDEMUXComprehensive::testMuxCascaded()
{
    QFETCH(int, primarySelect);
    QFETCH(int, secondarySelect);
    QFETCH(int, expectedOutput);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // Create 4 data inputs
    QVector<InputSwitch *> dataIn(4);
    for (int i = 0; i < 4; ++i) dataIn[i] = new InputSwitch();
    for (int i = 0; i < 4; ++i) {
        builder.add(dataIn[i]);
        dataIn[i]->setOn(true);
    }

    // Create 2 select inputs
    InputSwitch sel0, sel1;
    builder.add(&sel0, &sel1);

    // First stage: 2 muxes (each 2-to-1)
    Mux muxA, muxB;
    builder.add(&muxA, &muxB);

    builder.connect(dataIn[0], 0, &muxA, 0);
    builder.connect(dataIn[1], 0, &muxA, 1);
    builder.connect(&sel0, 0, &muxA, 2);

    builder.connect(dataIn[2], 0, &muxB, 0);
    builder.connect(dataIn[3], 0, &muxB, 1);
    builder.connect(&sel0, 0, &muxB, 2);

    // Second stage: 1 mux (2-to-1) selecting between the two stage-1 muxes
    Mux muxFinal;
    Led out;
    builder.add(&muxFinal, &out);

    builder.connect(&muxA, 0, &muxFinal, 0);
    builder.connect(&muxB, 0, &muxFinal, 1);
    builder.connect(&sel1, 0, &muxFinal, 2);

    builder.connect(&muxFinal, 0, &out, 0);

    auto *simulation = builder.initSimulation();

    sel0.setOn(primarySelect);
    sel1.setOn(secondarySelect);
    simulation->update();

    int output = getInputStatus(&out) ? 1 : 0;
    QCOMPARE(output, expectedOutput);
}

void TestMUXDEMUXComprehensive::testMuxDataInputPatterns_data()
{
    QTest::addColumn<QVector<int>>("pattern");
    QTest::addColumn<int>("selectIndex");
    QTest::addColumn<int>("expectedOutput");

    // Test with different bit patterns
    QTest::newRow("pattern_10101_select0") << QVector<int>{1, 0, 1, 0} << 0 << 1;
    QTest::newRow("pattern_10101_select1") << QVector<int>{1, 0, 1, 0} << 1 << 0;
    QTest::newRow("pattern_10101_select2") << QVector<int>{1, 0, 1, 0} << 2 << 1;
    QTest::newRow("pattern_10101_select3") << QVector<int>{1, 0, 1, 0} << 3 << 0;

    QTest::newRow("pattern_01010_select0") << QVector<int>{0, 1, 0, 1} << 0 << 0;
    QTest::newRow("pattern_01010_select1") << QVector<int>{0, 1, 0, 1} << 1 << 1;
    QTest::newRow("pattern_01010_select2") << QVector<int>{0, 1, 0, 1} << 2 << 0;
    QTest::newRow("pattern_01010_select3") << QVector<int>{0, 1, 0, 1} << 3 << 1;
}

void TestMUXDEMUXComprehensive::testMuxDataInputPatterns()
{
    QFETCH(QVector<int>, pattern);
    QFETCH(int, selectIndex);
    QFETCH(int, expectedOutput);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    QVector<InputSwitch *> dataIn(4);
    for (int i = 0; i < 4; ++i) dataIn[i] = new InputSwitch();
    InputSwitch sel0, sel1;
    Led out;
    Mux mux;

    // Resize mux to 4-to-1 (6 ports: 4 data + 2 select)
    mux.setInputSize(6);

    for (int i = 0; i < 4; ++i) {
        builder.add(dataIn[i]);
    }
    builder.add(&sel0, &sel1, &out, &mux);

    for (int i = 0; i < 4; ++i) {
        builder.connect(dataIn[i], 0, &mux, i);
    }
    builder.connect(&sel0, 0, &mux, 4);
    builder.connect(&sel1, 0, &mux, 5);
    builder.connect(&mux, 0, &out, 0);

    auto *simulation = builder.initSimulation();

    for (int i = 0; i < 4; ++i) {
        dataIn[i]->setOn(pattern[i]);
    }
    sel0.setOn((selectIndex >> 0) & 1);
    sel1.setOn((selectIndex >> 1) & 1);
    simulation->update();

    int output = getInputStatus(&out) ? 1 : 0;
    QCOMPARE(output, expectedOutput);
}

// ============================================================
// DEMUX TESTS
// ============================================================

void TestMUXDEMUXComprehensive::testDemux1to2_data()
{
    QTest::addColumn<int>("dataInput");
    QTest::addColumn<int>("selectLine");
    QTest::addColumn<int>("expectedOut0");
    QTest::addColumn<int>("expectedOut1");

    // Data on, select both positions
    QTest::newRow("1to2_data1_select0") << 1 << 0 << 1 << 0;
    QTest::newRow("1to2_data1_select1") << 1 << 1 << 0 << 1;

    // Data off
    QTest::newRow("1to2_data0_select0") << 0 << 0 << 0 << 0;
    QTest::newRow("1to2_data0_select1") << 0 << 1 << 0 << 0;
}

void TestMUXDEMUXComprehensive::testDemux1to2()
{
    QFETCH(int, dataInput);
    QFETCH(int, selectLine);
    QFETCH(int, expectedOut0);
    QFETCH(int, expectedOut1);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch in, sel;
    Led out0, out1;
    Demux demux;

    builder.add(&in, &sel, &out0, &out1, &demux);

    builder.connect(&in, 0, &demux, 0);
    builder.connect(&sel, 0, &demux, 1);
    builder.connect(&demux, 0, &out0, 0);
    builder.connect(&demux, 1, &out1, 0);

    auto *simulation = builder.initSimulation();

    in.setOn(dataInput);
    sel.setOn(selectLine);
    simulation->update();

    int output0 = getInputStatus(&out0) ? 1 : 0;
    int output1 = getInputStatus(&out1) ? 1 : 0;

    QCOMPARE(output0, expectedOut0);
    QCOMPARE(output1, expectedOut1);
}

void TestMUXDEMUXComprehensive::testDemux1to4_data()
{
    QTest::addColumn<int>("selectIndex");

    for (int i = 0; i < 4; ++i) {
        QTest::newRow(QString("1to4_select%1").arg(i).toLatin1().constData()) << i;
    }
}

void TestMUXDEMUXComprehensive::testDemux1to4()
{
    QFETCH(int, selectIndex);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch in;
    QVector<InputSwitch *> selectLines(2);
    for (int i = 0; i < 2; ++i) selectLines[i] = new InputSwitch();
    QVector<Led *> outs(4);
    for (int i = 0; i < 4; ++i) outs[i] = new Led();
    Demux demux;

    // Resize demux to 1-to-4 (4 outputs) BEFORE adding to builder
    demux.setOutputSize(4);

    builder.add(&in);
    for (int i = 0; i < 2; ++i) {
        builder.add(selectLines[i]);
    }
    for (int i = 0; i < 4; ++i) {
        builder.add(outs[i]);
    }
    builder.add(&demux);

    in.setOn(true);
    builder.connect(&in, 0, &demux, 0);
    for (int i = 0; i < 2; ++i) {
        builder.connect(selectLines[i], 0, &demux, 1 + i);
    }
    for (int i = 0; i < 4; ++i) {
        builder.connect(&demux, i, outs[i], 0);
    }

    auto *simulation = builder.initSimulation();

    for (int i = 0; i < 2; ++i) {
        selectLines[i]->setOn((selectIndex >> i) & 1);
    }
    simulation->update();

    // Only selected output should be on
    for (int i = 0; i < 4; ++i) {
        int output = getInputStatus(outs[i]) ? 1 : 0;
        if (i == selectIndex) {
            QCOMPARE(output, 1);
        } else {
            QCOMPARE(output, 0);
        }
    }
}

void TestMUXDEMUXComprehensive::testDemux1to8_data()
{
    QTest::addColumn<int>("selectIndex");

    for (int i = 0; i < 8; ++i) {
        QTest::newRow(QString("1to8_select%1").arg(i).toLatin1().constData()) << i;
    }
}

void TestMUXDEMUXComprehensive::testDemux1to8()
{
    QFETCH(int, selectIndex);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch in;
    QVector<InputSwitch *> selectLines(3);
    for (int i = 0; i < 3; ++i) selectLines[i] = new InputSwitch();
    QVector<Led *> outs(8);
    for (int i = 0; i < 8; ++i) outs[i] = new Led();
    Demux demux;

    // Resize demux to 1-to-8 (8 outputs)
    demux.setOutputSize(8);

    builder.add(&in);
    for (int i = 0; i < 3; ++i) {
        builder.add(selectLines[i]);
    }
    for (int i = 0; i < 8; ++i) {
        builder.add(outs[i]);
    }
    builder.add(&demux);

    in.setOn(true);
    builder.connect(&in, 0, &demux, 0);
    for (int i = 0; i < 3; ++i) {
        builder.connect(selectLines[i], 0, &demux, 1 + i);
    }
    for (int i = 0; i < 8; ++i) {
        builder.connect(&demux, i, outs[i], 0);
    }

    auto *simulation = builder.initSimulation();

    for (int i = 0; i < 3; ++i) {
        selectLines[i]->setOn((selectIndex >> i) & 1);
    }
    simulation->update();

    // Only selected output should be on
    for (int i = 0; i < 8; ++i) {
        int output = getInputStatus(outs[i]) ? 1 : 0;
        if (i == selectIndex) {
            QCOMPARE(output, 1);
        } else {
            QCOMPARE(output, 0);
        }
    }
}

void TestMUXDEMUXComprehensive::testDemuxDataInputVariations_data()
{
    QTest::addColumn<int>("dataValue");
    QTest::addColumn<int>("selectIndex");

    // Test with data on and off
    QTest::newRow("1to4_data1_select0") << 1 << 0;
    QTest::newRow("1to4_data1_select3") << 1 << 3;
    QTest::newRow("1to4_data0_select0") << 0 << 0;
    QTest::newRow("1to4_data0_select3") << 0 << 3;
}

void TestMUXDEMUXComprehensive::testDemuxDataInputVariations()
{
    QFETCH(int, dataValue);
    QFETCH(int, selectIndex);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch in;
    QVector<InputSwitch *> selectLines(2);
    for (int i = 0; i < 2; ++i) selectLines[i] = new InputSwitch();
    QVector<Led *> outs(4);
    for (int i = 0; i < 4; ++i) outs[i] = new Led();
    Demux demux;

    // Resize demux to 1-to-4 (4 outputs) BEFORE adding to builder
    demux.setOutputSize(4);

    builder.add(&in);
    for (int i = 0; i < 2; ++i) {
        builder.add(selectLines[i]);
    }
    for (int i = 0; i < 4; ++i) {
        builder.add(outs[i]);
    }
    builder.add(&demux);

    builder.connect(&in, 0, &demux, 0);
    for (int i = 0; i < 2; ++i) {
        builder.connect(selectLines[i], 0, &demux, 1 + i);
    }
    for (int i = 0; i < 4; ++i) {
        builder.connect(&demux, i, outs[i], 0);
    }

    auto *simulation = builder.initSimulation();

    in.setOn(dataValue);
    for (int i = 0; i < 2; ++i) {
        selectLines[i]->setOn((selectIndex >> i) & 1);
    }
    simulation->update();

    // Only selected output should match data value
    for (int i = 0; i < 4; ++i) {
        int output = getInputStatus(outs[i]) ? 1 : 0;
        if (i == selectIndex) {
            QCOMPARE(output, dataValue);
        } else {
            QCOMPARE(output, 0);
        }
    }
}

void TestMUXDEMUXComprehensive::testDemuxSelectTransitions_data()
{
    QTest::addColumn<int>("initialSelect");
    QTest::addColumn<int>("finalSelect");

    QTest::newRow("1to4_transition_0to3") << 0 << 3;
    QTest::newRow("1to4_transition_3to0") << 3 << 0;
    QTest::newRow("1to4_transition_1to2") << 1 << 2;
}

void TestMUXDEMUXComprehensive::testDemuxSelectTransitions()
{
    QFETCH(int, initialSelect);
    QFETCH(int, finalSelect);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    InputSwitch in;
    QVector<InputSwitch *> selectLines(2);
    for (int i = 0; i < 2; ++i) selectLines[i] = new InputSwitch();
    QVector<Led *> outs(4);
    for (int i = 0; i < 4; ++i) outs[i] = new Led();
    Demux demux;

    // Resize demux to 1-to-4 (4 outputs) BEFORE adding to builder
    demux.setOutputSize(4);

    builder.add(&in);
    for (int i = 0; i < 2; ++i) {
        builder.add(selectLines[i]);
    }
    for (int i = 0; i < 4; ++i) {
        builder.add(outs[i]);
    }
    builder.add(&demux);

    in.setOn(true);
    builder.connect(&in, 0, &demux, 0);
    for (int i = 0; i < 2; ++i) {
        builder.connect(selectLines[i], 0, &demux, 1 + i);
    }
    for (int i = 0; i < 4; ++i) {
        builder.connect(&demux, i, outs[i], 0);
    }

    auto *simulation = builder.initSimulation();

    // Set initial select
    for (int i = 0; i < 2; ++i) {
        selectLines[i]->setOn((initialSelect >> i) & 1);
    }
    simulation->update();

    // Verify initial state
    for (int i = 0; i < 4; ++i) {
        int output = getInputStatus(outs[i]) ? 1 : 0;
        if (i == initialSelect) {
            QCOMPARE(output, 1);
        } else {
            QCOMPARE(output, 0);
        }
    }

    // Transition to final select
    for (int i = 0; i < 2; ++i) {
        selectLines[i]->setOn((finalSelect >> i) & 1);
    }
    simulation->update();

    // Verify final state
    for (int i = 0; i < 4; ++i) {
        int output = getInputStatus(outs[i]) ? 1 : 0;
        if (i == finalSelect) {
            QCOMPARE(output, 1);
        } else {
            QCOMPARE(output, 0);
        }
    }
}

void TestMUXDEMUXComprehensive::testMuxDemuxChained_data()
{
    QTest::addColumn<int>("dataInputIndex");
    QTest::addColumn<int>("muxSelect");
    QTest::addColumn<int>("demuxSelect");

    // Chain: 4 inputs -> 4-to-1 Mux -> 1-to-4 Demux -> 4 outputs
    // Data should flow through: input[i] -> mux (select mux input) -> demux output (select demux output)
    for (int data = 0; data < 4; ++data) {
        for (int demux = 0; demux < 4; ++demux) {
            if (data == demux) {
                QTest::newRow(QString("chained_data%1_demux%2").arg(data).arg(demux).toLatin1().constData())
                    << data << data << demux;
            }
        }
    }
}

void TestMUXDEMUXComprehensive::testMuxDemuxChained()
{
    QFETCH(int, dataInputIndex);
    QFETCH(int, muxSelect);
    QFETCH(int, demuxSelect);

    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());

    // 4 data inputs
    QVector<InputSwitch *> dataIn(4);
    for (int i = 0; i < 4; ++i) dataIn[i] = new InputSwitch();
    for (int i = 0; i < 4; ++i) {
        builder.add(dataIn[i]);
        dataIn[i]->setOn(i == dataInputIndex);
    }

    // Mux selects
    InputSwitch muxSel0, muxSel1;
    builder.add(&muxSel0, &muxSel1);

    // Demux selects
    InputSwitch demuxSel0, demuxSel1;
    builder.add(&demuxSel0, &demuxSel1);

    // Mux and Demux
    Mux mux;
    Demux demux;

    // Resize mux and demux BEFORE adding to builder
    mux.setInputSize(6);
    demux.setOutputSize(4);

    builder.add(&mux, &demux);

    // 4 outputs
    QVector<Led *> outs(4);
    for (int i = 0; i < 4; ++i) outs[i] = new Led();
    for (int i = 0; i < 4; ++i) {
        builder.add(outs[i]);
    }

    // Connect inputs to mux
    for (int i = 0; i < 4; ++i) {
        builder.connect(dataIn[i], 0, &mux, i);
    }
    builder.connect(&muxSel0, 0, &mux, 4);
    builder.connect(&muxSel1, 0, &mux, 5);

    // Connect mux to demux
    builder.connect(&mux, 0, &demux, 0);

    // Connect demux selects
    builder.connect(&demuxSel0, 0, &demux, 1);
    builder.connect(&demuxSel1, 0, &demux, 2);

    // Connect demux outputs
    for (int i = 0; i < 4; ++i) {
        builder.connect(&demux, i, outs[i], 0);
    }

    auto *simulation = builder.initSimulation();

    // Set mux select
    muxSel0.setOn((muxSelect >> 0) & 1);
    muxSel1.setOn((muxSelect >> 1) & 1);

    // Set demux select
    demuxSel0.setOn((demuxSelect >> 0) & 1);
    demuxSel1.setOn((demuxSelect >> 1) & 1);

    simulation->update();

    // Only the demux-selected output should be on
    for (int i = 0; i < 4; ++i) {
        int output = getInputStatus(outs[i]) ? 1 : 0;
        if (i == demuxSelect) {
            QCOMPARE(output, 1);
        } else {
            QCOMPARE(output, 0);
        }
    }
}

