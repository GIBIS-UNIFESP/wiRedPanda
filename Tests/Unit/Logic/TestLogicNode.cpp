// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Logic/TestLogicNode.h"

#include <QTest>

#include "App/Element/LogicElements/LogicAnd.h"
#include "App/Element/LogicElements/LogicInput.h"
#include "App/Element/LogicElements/LogicNode.h"

void TestLogicNode::init()
{
    std::generate(m_inputs.begin(), m_inputs.end(), [] { return new LogicInput(); });
}

void TestLogicNode::cleanup()
{
    qDeleteAll(m_inputs);
    m_inputs.fill(nullptr);
}

void TestLogicNode::testLogicNodeChainPropagation()
{
    // Three LogicNodes in series: input → n1 → n2 → n3
    // Signal must propagate all the way through on every tick.
    LogicNode n1, n2, n3;
    n1.connectPredecessor(0, m_inputs.at(0), 0);
    n2.connectPredecessor(0, &n1, 0);
    n3.connectPredecessor(0, &n2, 0);

    m_inputs.at(0)->setOutputValue(true);
    n1.updateLogic();
    n2.updateLogic();
    n3.updateLogic();
    QCOMPARE(n3.outputValue(), true);

    m_inputs.at(0)->setOutputValue(false);
    n1.updateLogic();
    n2.updateLogic();
    n3.updateLogic();
    QCOMPARE(n3.outputValue(), false);
}

void TestLogicNode::testLogicNodeFanOut()
{
    // One LogicNode feeds two AND gates.
    // When the node is HIGH and both ANDs have their second input HIGH, all AND outputs must be HIGH.
    // Driving the node LOW must pull all AND outputs LOW regardless of the second inputs.
    LogicNode node;
    LogicAnd and1(2), and2(2);

    node.connectPredecessor(0, m_inputs.at(0), 0);
    and1.connectPredecessor(0, &node, 0);
    and1.connectPredecessor(1, m_inputs.at(1), 0);
    and2.connectPredecessor(0, &node, 0);
    and2.connectPredecessor(1, m_inputs.at(2), 0);

    m_inputs.at(0)->setOutputValue(true);
    m_inputs.at(1)->setOutputValue(true);
    m_inputs.at(2)->setOutputValue(true);
    node.updateLogic();
    and1.updateLogic();
    and2.updateLogic();
    QCOMPARE(and1.outputValue(), true);
    QCOMPARE(and2.outputValue(), true);

    // Drive node LOW → both AND gates go LOW
    m_inputs.at(0)->setOutputValue(false);
    node.updateLogic();
    and1.updateLogic();
    and2.updateLogic();
    QCOMPARE(and1.outputValue(), false);
    QCOMPARE(and2.outputValue(), false);
}
