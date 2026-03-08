// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTest>

class WorkSpace;
class QNEConnection;
struct GeneratedCode;

class TestArduino : public QObject
{
    Q_OBJECT

private:
    // Helper structure for generated code
    struct GeneratedCode {
        QString content;
        bool success;
    };

    // Helper methods to reduce boilerplate
    GeneratedCode generateFromElements(const QVector<class GraphicElement *> &elements);
    void verifyBasicStructure(const QString &content);
    void verifyLogicOperator(const QString &content, const QString &logicOp, bool negated = false);

    // Flip-flop specific helpers
    void verifyFlipFlopStructure(const QString &content, const QString &type);
    void verifyAuxVariables(const QString &content, const QStringList &varNames);
    void verifyPresetClearLogic(const QString &content);

private slots:

    // Basic Generation (2 tests)
    void testBasicCircuitGeneration();
    void testEmptyCircuit();

    // Logic Gates (4 tests)
    void testAndGateGeneration();
    void testOrGateGeneration();
    void testNotGateGeneration();
    void testMixedLogicGates();

    // Sequential Logic (4 tests)
    void testClockGeneration();
    void testFlipFlopGeneration();
    void testLatchGeneration();
    void testClockWithFrequency();

    // Pin and Variable Mapping (3 tests)
    void testPinMapping();
    void testVariableNaming();
    void testVariableNameSanitization();

    // Input/Output Elements (2 tests)
    void testInputElementHandling();
    void testOutputElementHandling();

    // Integration (2 tests)
    void testComplexCircuit();
    void testCodeStructure();

    // Error Handling (1 test)
    void testICElementError();

    void testNandGateGeneration();
    void testNorGateGeneration();
    void testXnorGateGeneration();

    void testUnconnectedLogicGate();

    void testTFlipFlopGeneration();
    void testJKFlipFlopGeneration();
    void testSRFlipFlopGeneration();

    void testFlipFlopPresetClear();
    void testMultipleOutputElement();

    void testOutputPortNaming();
    void testComplexElementNaming();
    void testNameSanitizationEdgeCases();

    void testUnconnectedInput();
    void testDefaultValuePropagation();
    void testPartiallyConnectedLogic();
    void testHighPinCountCircuit();

    void testMultipleInputConnections();
    void testChainedLogicGates();

    void testPresetClearInteraction();
    void testFlipFlopInitialState();
    void testLatchEdgeTriggeringLogic();

    void testFileCreationFailure();
    void testReadOnlyDirectoryHandling();

    void testDeeplyNestedElementNaming();
    void testNodeElementGeneration();
    void testMuxDemuxIntegration();

    void testXorGateAllCombinations();
    void testMultipleClocksInCircuit();
    void testComplexPresetClearSequence();

    void testOutputPinExhaustion();
    void testUnsupportedElementTypes();
};
