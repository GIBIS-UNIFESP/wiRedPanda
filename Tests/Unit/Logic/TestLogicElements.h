// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QVector>

class LogicInput;

class TestLogicElements : public QObject
{
    Q_OBJECT

private slots:
    void cleanup();
    void init();
    void testLogicNode_data();
    void testLogicNode();
    void testLogicAnd_data();
    void testLogicAnd();
    void testLogicOr_data();
    void testLogicOr();
    void testLogicInput();
    void testLogicMux_data();
    void testLogicMux();
    void testLogicDemux_data();
    void testLogicDemux();
    void testLogicDFlipFlop_data();
    void testLogicDFlipFlop();
    void testLogicDLatch_data();
    void testLogicDLatch();
    void testLogicJKFlipFlop_data();
    void testLogicJKFlipFlop();
    void testLogicSRFlipFlop_data();
    void testLogicSRFlipFlop();
    void testLogicTFlipFlop_data();
    void testLogicTFlipFlop();
    void testLogicXnor_data();
    void testLogicXnor();
    void testLogicNot_data();
    void testLogicNot();

    // NOT gate circuit context tests
    void testNotGateViaBuilder_data();
    void testNotGateViaBuilder();

    void testNotGateInAnd_data();
    void testNotGateInAnd();

    void testLogicNand_data();
    void testLogicNand();
    void testLogicNor_data();
    void testLogicNor();
    void testLogicXor_data();
    void testLogicXor();
    void testLogicSRLatch_data();
    void testLogicSRLatch();

    void testLogic3InputAnd_data();
    void testLogic3InputAnd();

    void testLogic3InputOr_data();
    void testLogic3InputOr();

    void testLogic4InputAnd_data();
    void testLogic4InputAnd();

    void testLogic4InputOr_data();
    void testLogic4InputOr();

    void testLogic3InputXor_data();
    void testLogic3InputXor();

    void testLogic3InputXnor_data();
    void testLogic3InputXnor();

    void testLogic5InputAnd_data();
    void testLogic5InputAnd();

    void testLogic5InputOr_data();
    void testLogic5InputOr();

    void testLogic5InputNand_data();
    void testLogic5InputNand();

    void testLogic5InputNor_data();
    void testLogic5InputNor();

    void testLogic8InputAnd_data();
    void testLogic8InputAnd();

    void testLogic8InputOr_data();
    void testLogic8InputOr();

    void testLogic8InputNand_data();
    void testLogic8InputNand();

    void testLogic8InputNor_data();
    void testLogic8InputNor();

    void testFanOut_data();
    void testFanOut();

    void testLogic6InputAnd_data();
    void testLogic6InputAnd();

private:
    QVector<LogicInput *> m_inputs{8};
};

