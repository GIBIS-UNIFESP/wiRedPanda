// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTest>

#include "App/CodeGen/ArduinoCodeGen.h"

class GraphicElement;
class IC;
class QNEConnection;

class TestArduino : public QObject
{
    Q_OBJECT

private:
    // Helper structure for generated code
    struct GeneratedCode {
        QString content;
        bool success;
    };

    // Code generation helpers
    GeneratedCode generateFromElements(const QVector<GraphicElement *> &elements);
    void verifyBasicStructure(const QString &content);
    void verifyLogicOperator(const QString &content, const QString &logicOp, bool negated = false);
    void testArduinoExportHelper(const QString &icFile);
    static bool validateWithArduinoCli(const QString &inoFile);
    static bool simulateWithSimavr(const QString &elfFile, int durationMs);
    static bool runTestbench(const QString &tbInoPath, int timeoutMs = 30000);

    static QString s_cliCachePath;

    // Truth table generation from the wiRedPanda simulator
    static QVector<ArduinoCodeGen::TestVector> generateTruthTable(const QVector<GraphicElement *> &elements, int maxInputBits = 12);
    static bool isCombinationalCircuit(const QVector<GraphicElement *> &elements);

    // Flip-flop specific helpers
    void verifyFlipFlopStructure(const QString &content, const QString &type);
    void verifyAuxVariables(const QString &content, const QStringList &varNames);
    void verifyPresetClearLogic(const QString &content);

    // IC test helpers
    IC *createICWithAndGate(const QString &label);
    IC *createICContaining(IC *innerIC, const QString &label);
    QNEConnection *createConnection(GraphicElement *from, int fromPort, GraphicElement *to, int toPort);

    // Board / pin helpers
    bool checkBoardSelection(const QString &content, const QString &expectedBoard);
    QStringList extractPinAssignments(const QString &content);
    QStringList extractVariableDeclarations(const QString &content);
    bool checkPinUniqueness(const QStringList &pins);

    // Syntax / quality helpers
    bool validateArduinoSyntax(const QString &content);

private slots:
    void initTestCase();
    void cleanupTestCase();

    // Basic Generation (2 tests)
    void testBasicCircuitGeneration();
    void testEmptyCircuit();

    // Logic Gates
    void testGateGeneration_data();
    void testGateGeneration();
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

    // Board Selection (2 tests)
    void testBoardSelection();
    void testExcessivePinRequirements();

    // Special Elements (4 tests)
    void testBuzzerGeneration();
    void testDisplay7Generation();
    void testDisplay14Generation();
    void testInputRotaryGeneration();

    // IC Support (3 tests)
    void testSimpleICGeneration();
    void testNestedICHandling();
    void testICPortMapping();

    // Advanced Circuits (1 test)
    void testAdvancedRealWorldCircuits();

    // Error Handling (1 test)
    void testUnsupportedElementTypes();

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
    void testWirelessNodeGeneration();
    void testWirelessOrphanedRxCodegen();
    void testMuxDemuxIntegration();

    void testMultipleClocksInCircuit();
    void testComplexPresetClearSequence();

    void testOutputPinExhaustion();

    // Level 1 - Sequential IC tests (from real .panda files)
    void testArduinoExportDFlipFlop();
    void testArduinoExportDLatch();
    void testArduinoExportJKFlipFlop();
    void testArduinoExportSRLatch();

    // Level 2 - Combinational IC tests (from real .panda files)
    void testArduinoExportHalfAdder();
    void testArduinoExportFullAdder1Bit();
    void testArduinoExportDecoder2to4();
    void testArduinoExportDecoder3to8();
    void testArduinoExportDecoder4to16();
    void testArduinoExportMux2to1();
    void testArduinoExportMux4to1();
    void testArduinoExportMux8to1();
    void testArduinoExportParityChecker();
    void testArduinoExportParityGenerator();
    void testArduinoExportPriorityEncoder8to3();
    void testArduinoExportPriorityMux3to1();

    // Level 3 - Medium-complexity IC tests
    void testArduinoExportAluSelector5Way();
    void testArduinoExportBcd7SegmentDecoder();
    void testArduinoExportComparator4Bit();
    void testArduinoExportComparator4BitEquality();
    void testArduinoExportRegister1Bit();

    // Level 4 - Nested IC tests
    void testArduinoExportRippleAdder4Bit();
    void testArduinoExportRegister4Bit();
    void testArduinoExportBinaryCounter4Bit();

    // Unit tests for codegen utilities
    void testReservedKeywordSanitization();
    void testAccentStripping();
    void testCycleDetection();
    void testSimavrFunctionalSimulation();
};

