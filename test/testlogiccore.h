// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QVector>

class LogicInput;

class TestLogicCore : public QObject
{
    Q_OBJECT

private slots:
    void cleanup();
    void init();

    // Combinational logic elements (normal + edge cases combined)
    void testLogicAnd();
    void testLogicOr();
    void testLogicNot();
    void testLogicNand();
    void testLogicNor();
    void testLogicXor();
    void testLogicXnor();

    // Sequential logic elements (normal + edge cases combined)
    void testLogicDFlipFlop();
    void testLogicJKFlipFlop();
    void testLogicSRFlipFlop();
    void testLogicTFlipFlop();
    void testLogicDLatch();
    void testLogicSRLatch();

    // Complex logic elements (normal + edge cases combined)
    void testLogicMux();
    void testLogicDemux();
    void testLogicTruthTable();

    // Infrastructure elements (normal + edge cases combined)
    void testLogicInput();
    void testLogicOutput();
    void testLogicNode();

    // Boundary and edge case testing
    void testMultiInputBoundaries();
    void testInvalidInputHandling();
    void testUpdateWithoutInputs();
    void testComplexBooleanExpressions();
    void testPerformanceAndStress();

    // Property-based testing
    void testLogicProperties();

    // Enhanced sequential logic testing
    void testJKFlipFlopModes();
    void testTFlipFlopToggling();

private:
    void setupInputs(int count);
    void setInputValues(const QVector<bool> &values);

    // Helper methods for combined testing
    void testAndNormalCases();
    void testAndEdgeCases();
    void testOrNormalCases();
    void testOrEdgeCases();
    void testFlipFlopNormalCases(class LogicElement *element, const QString &type);
    void testFlipFlopEdgeCases(class LogicElement *element, const QString &type);

    QVector<LogicInput *> switches{50}; // Extended for stress testing
};
