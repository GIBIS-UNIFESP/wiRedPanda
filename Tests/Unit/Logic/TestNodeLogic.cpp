// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Logic/TestNodeLogic.h"

#include <QTest>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputVCC.h"
#include "App/Element/GraphicElements/Node.h"

static void initNodeSrc(GraphicElement &elm) { elm.initSimulationVectors(0, 1); }
static void initNodeElm(GraphicElement &elm) { elm.initSimulationVectors(elm.inputSize(), elm.outputSize()); }

void TestNodeLogic::init()
{
    std::generate(m_inputs.begin(), m_inputs.end(), [] {
        auto *src = new InputVcc();
        initNodeSrc(*src);
        return src;
    });
}

void TestNodeLogic::cleanup()
{
    qDeleteAll(m_inputs);
    m_inputs.fill(nullptr);
}

void TestNodeLogic::testNodeChainPropagation()
{
    // Three Nodes in series: input -> n1 -> n2 -> n3
    Node n1, n2, n3;
    initNodeElm(n1); initNodeElm(n2); initNodeElm(n3);
    n1.connectPredecessor(0, m_inputs.at(0), 0);
    n2.connectPredecessor(0, &n1, 0);
    n3.connectPredecessor(0, &n2, 0);

    m_inputs.at(0)->setOutputValue(true);
    n1.updateLogic();
    n2.updateLogic();
    n3.updateLogic();
    QCOMPARE(n3.outputValue(), Status::Active);

    m_inputs.at(0)->setOutputValue(false);
    n1.updateLogic();
    n2.updateLogic();
    n3.updateLogic();
    QCOMPARE(n3.outputValue(), Status::Inactive);
}

void TestNodeLogic::testNodeFanOut()
{
    Node node;
    And and1, and2;
    initNodeElm(node); initNodeElm(and1); initNodeElm(and2);

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
    QCOMPARE(and1.outputValue(), Status::Active);
    QCOMPARE(and2.outputValue(), Status::Active);

    // Drive node LOW -> both AND gates go LOW
    m_inputs.at(0)->setOutputValue(false);
    node.updateLogic();
    and1.updateLogic();
    and2.updateLogic();
    QCOMPARE(and1.outputValue(), Status::Inactive);
    QCOMPARE(and2.outputValue(), Status::Inactive);
}

