// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryFile>
#include <QVector>

class GraphicElement;
class WorkSpace;

class TestArduino : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    // Basic circuit generation tests
    void testBasicAndCircuit();
    void testBasicOrCircuit();
    void testBasicNotCircuit();
    void testCombinationalLogic();

    // Sequential logic generation tests
    void testDFlipFlopGeneration();
    void testJKFlipFlopGeneration();
    void testSRFlipFlopGeneration();
    void testTFlipFlopGeneration();
    void testDLatchGeneration();

    // Clock and timing tests
    void testClockGeneration();
    void testMultipleClocks();

    // Input/Output tests
    void testInputButtonMapping();
    void testInputSwitchMapping();
    void testLedOutputMapping();
    void testMultipleInputsOutputs();

    // Variable naming and pin assignment tests
    void testVariableNaming();
    void testPinAssignment();
    void testNameCollisionHandling();
    void testForbiddenCharacterHandling();

    // Complex circuit tests
    void testComplexCombinationalCircuit();
    void testSequentialCircuitWithClock();
    void testMixedLogicCircuit();

    // Error handling tests
    void testEmptyCircuit();
    void testDisconnectedElements();
    void testInvalidElementTypes();
    void testInsufficientPins();

    // Code structure and syntax tests
    void testGeneratedCodeStructure();
    void testArduinoSyntaxCompliance();
    void testVariableDeclarations();
    void testSetupFunction();
    void testLoopFunction();

    // Edge case tests
    void testMaximumInputOutputs();
    void testElementLabeling();
    void testPortNaming();

private:
    // Helper methods for creating test circuits
    WorkSpace* createTestWorkspace();
    QVector<GraphicElement*> createBasicAndCircuit();
    QVector<GraphicElement*> createBasicOrCircuit();
    QVector<GraphicElement*> createFlipFlopCircuit();
    QVector<GraphicElement*> createClockCircuit();
    QVector<GraphicElement*> createComplexCircuit();

    // Helper methods for verifying generated code
    QString generateArduinoCode(const QVector<GraphicElement*>& elements);
    void verifyCodeStructure(const QString& code);
    void verifyVariableDeclarations(const QString& code, const QStringList& expectedVars);
    void verifySetupFunction(const QString& code, const QStringList& expectedPins);
    void verifyLoopFunction(const QString& code);
    void verifyArduinoSyntax(const QString& code);
    bool containsSubstring(const QString& code, const QString& substring);
    QStringList extractVariableNames(const QString& code);
    QStringList extractPinAssignments(const QString& code);

    // Test data
    WorkSpace* m_workspace = nullptr;
    QTemporaryFile* m_tempFile = nullptr;
};
