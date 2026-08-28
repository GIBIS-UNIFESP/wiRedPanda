// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QTest>

#include "App/CodeGen/ArduinoCodeGen.h"

class GraphicElement;
class IC;
class Connection;
class Simulation;

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
    static bool simulateWithSimavr(const QString &elfFile, int durationMs);
    static bool runTestbench(const QString &tbInoPath, int timeoutMs = 30000,
                             const QString &fqbn = QStringLiteral("arduino:avr:uno"),
                             const QString &simavrMcu = QStringLiteral("atmega328p"));

    static QString s_cliCachePath;

    // Truth table generation from the wiRedPanda simulator. When \a sim is non-null, inputs are
    // driven through the production Simulation (required for circuits containing ICs, since ICs
    // are pure structure — Simulation::initialize() flattens them into the netlist rather than
    // self-simulating); otherwise elements are settled directly via updateLogic() (sim-free path,
    // used by plain gate circuits with no IC).
    static QVector<ArduinoCodeGen::TestVector> generateTruthTable(const QVector<GraphicElement *> &elements,
                                                                   int maxInputBits = 12, Simulation *sim = nullptr);
    static bool isCombinationalCircuit(const QVector<GraphicElement *> &elements);

    // Flip-flop specific helpers
    void verifyFlipFlopStructure(const QString &content, const QString &type);
    void verifyPresetClearLogic(const QString &content);

    // IC test helpers
    IC *createICWithAndGate(const QString &label);
    IC *createICContaining(IC *innerIC, const QString &label);
    Connection *createConnection(GraphicElement *from, int fromPort, GraphicElement *to, int toPort);

    // Board / pin helpers
    bool checkBoardSelection(const QString &content, const QString &expectedBoard);
    QStringList extractPinAssignments(const QString &content);
    QStringList extractVariableDeclarations(const QString &content);
    bool checkPinUniqueness(const QStringList &pins);

    // Syntax / quality helpers
    bool validateArduinoSyntax(const QString &content);

private slots:
    /// Compiles the GENERATED SKETCH and drives it, comparing against the engine on the same
    /// circuit. A ripple counter is the shape that exposes a tick-model divergence: the sketch
    /// can advance one stage per tick where the engine advances the whole chain. Only running
    /// the sketch catches that -- emitted text that implements the wrong tick model still reads
    /// as correct.
    void testGeneratedSketchMatchesEngineOnRippleCounter();

    /// The Arduino half of the same rule: otherPortNameImpl() reaches its wireless resolution
    /// only when the port has no connections, so an Rx that also carries a physical wire would
    /// be exported following the wire the engine discarded.
    void testWirelessOverrideExportsTheWirelessDriver();

    void initTestCase();
    void cleanupTestCase();

    // Basic Generation (2 tests)
    void testBasicCircuitGeneration();
    void testEmptyCircuit();
    void testPinOverflowMessageNamesCounts();

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
    void testEmbeddedICGeneration();
    void testEmbeddedICLabelWithNewlineDoesNotInjectCode();
    void testMuxDemuxIntegration();

    void testMultipleClocksInCircuit();
    void testComplexPresetClearSequence();

    void testOutputPinExhaustion();

    // Level 1-4 IC export tests (from real .panda files), data-driven -- see
    // testArduinoExport_data() for the full fixture list (Level 1 sequential,
    // Level 2 combinational, Level 3 medium-complexity, Level 4 nested ICs).
    void testArduinoExport_data();
    void testArduinoExport();

    // Sequential (clocked) functional validation for hierarchical native-flip-flop
    // circuits — drives a clock sequence and checks the sketch against the engine.
    void testArduinoSequentialMultiCycleCpu8Bit();

    // Unit tests for codegen utilities
    void testReservedKeywordSanitization();
    void testAccentStripping();
    void testCycleDetection();
    void testSimavrFunctionalSimulation();
};
