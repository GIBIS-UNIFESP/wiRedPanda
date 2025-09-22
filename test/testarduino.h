// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryFile>
#include <QVector>

#include "enums.h"

class GraphicElement;
class InputSwitch;
class Led;
class And;
class QNEConnection;

class TestArduino : public QObject
{
    Q_OBJECT

private slots:
    void cleanup();
    void init();

    // Phase 1: Foundation tests
    void testBasicCodeGeneration();
    void testFileOutputValidation();
    void testBoardSelection();

    // Phase 2: Core element tests
    void testInputButtonGeneration();
    void testInputSwitchGeneration();
    void testLedGeneration();
    void testBasicLogicGates();

    // Phase 3: Advanced element tests
    void testComplexLogicGates();
    void testSequentialElements();
    void testSpecialElements();
    void testBuzzerGeneration();
    void testDisplay7Generation();

    // Phase 4: Complex features
    void testSimpleICGeneration();
    void testNestedICHandling();
    void testICPortMapping();
    void testMultiElementCircuits();
    void testPinAssignmentUniqueness();
    void testVariableNaming();
    void testAdvancedRealWorldCircuits();
    void testPerformanceAndEdgeCases();

    // Phase 5: Quality Assurance tests
    void testCodeQualityValidation();
    void testErrorHandlingVerification();
    void testTestCoverageAnalysis();
    void testArduinoCodeCompilation();

    // Error handling tests
    void testEmptyCircuit();
    void testExcessivePinRequirements();

    // Phase 6: Regression Testing
    void testArduinoRegressionCorpus();
    void testIndividualCorpusFiles();
    void testCorpusCodeQuality();

private:
    // Test helper methods
    GraphicElement* createInputSwitch(const QString& label = "");
    GraphicElement* createLed(const QString& label = "");
    GraphicElement* createLogicGate(Enums::ElementType type, int inputs = 2);
    GraphicElement* createSequentialElement(Enums::ElementType type, const QString& label = "");
    GraphicElement* createSpecialElement(Enums::ElementType type, const QString& label = "");
    GraphicElement* createIC(const QString& label = "", const QVector<GraphicElement*>& internalElements = {});
    void connectElements(GraphicElement* from, GraphicElement* to, int fromPort = 0, int toPort = 0);
    QVector<GraphicElement*> createSimpleCircuit();
    QVector<GraphicElement*> createComplexMultiElementCircuit();

    // Validation helper methods
    bool validateArduinoCode(const QString& code);
    QStringList extractVariableDeclarations(const QString& code);
    QStringList extractPinAssignments(const QString& code);
    bool checkPinUniqueness(const QStringList& pins);
    bool checkBoardSelection(const QString& code, const QString& expectedBoard);

    // Phase 5: Quality Assurance helper methods
    bool validateArduinoSyntax(const QString& code);
    bool checkCodingStandards(const QString& code);
    QStringList extractFunctionCalls(const QString& code);
    bool verifyArduinoStructure(const QString& code);
    int calculateCodeComplexity(const QString& code);
    bool testCodeCompilation(const QString& code, const QString& board = "uno");

    // File management
    QString generateCodeToString(const QVector<GraphicElement*>& elements);

    // Phase 6: Regression Testing helper methods
    QVector<GraphicElement*> loadPandaFile(const QString& filePath);
    QString loadExpectedCode(const QString& inoFilePath);
    bool compareArduinoCode(const QString& generated, const QString& expected, QString& diffReport);
    QStringList getCorpusPandaFiles() const;
    QString normalizeArduinoCode(const QString& code) const;
    struct RegressionResult {
        QString filename;
        bool passed;
        QString errorMessage;
        QString diffReport;
    };

    // Test infrastructure
    QVector<GraphicElement*> m_testElements;
    QVector<QNEConnection*> m_testConnections;
    QTemporaryFile* m_tempFile;
};
