// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QVector>

class InputVcc;

class TestElementLogic : public QObject
{
    Q_OBJECT

private slots:
    void cleanup();
    void init();
    void testNode_data();
    void testNode();
    void testAnd_data();
    void testAnd();
    void testOr_data();
    void testOr();
    void testSource();
    void testSourceMultiOutput();
    void testSink();
    void testMux_data();
    void testMux();
    void testDemux_data();
    void testDemux();
    void testDFlipFlop_data();
    void testDFlipFlop();
    void testDLatch_data();
    void testDLatch();
    void testJKFlipFlop_data();
    void testJKFlipFlop();
    void testSRFlipFlop_data();
    void testSRFlipFlop();
    void testTFlipFlop_data();
    void testTFlipFlop();
    void testXnor_data();
    void testXnor();
    void testNot_data();
    void testNot();

    // NOT gate circuit context tests
    void testNotGateViaBuilder_data();
    void testNotGateViaBuilder();

    void testNotGateInAnd_data();
    void testNotGateInAnd();

    void testNand_data();
    void testNand();
    void testNor_data();
    void testNor();
    void testXor_data();
    void testXor();
    void testSRLatch_data();
    void testSRLatch();

    void test3InputAnd_data();
    void test3InputAnd();

    void test3InputOr_data();
    void test3InputOr();

    void test4InputAnd_data();
    void test4InputAnd();

    void test4InputOr_data();
    void test4InputOr();

    void test3InputXor_data();
    void test3InputXor();

    void test3InputXnor_data();
    void test3InputXnor();

    void test5InputAnd_data();
    void test5InputAnd();

    void test5InputOr_data();
    void test5InputOr();

    void test5InputNand_data();
    void test5InputNand();

    void test5InputNor_data();
    void test5InputNor();

    void test8InputAnd_data();
    void test8InputAnd();

    void test8InputOr_data();
    void test8InputOr();

    void test8InputNand_data();
    void test8InputNand();

    void test8InputNor_data();
    void test8InputNor();

    void testFanOut_data();
    void testFanOut();

    void test6InputAnd_data();
    void test6InputAnd();

private:
    QVector<InputVcc *> m_inputs{8};
};

