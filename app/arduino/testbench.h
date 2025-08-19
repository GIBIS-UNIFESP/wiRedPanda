// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#include <QVector>
#include <QHash>

class GraphicElement;

// Test vector for validation
struct TestVector {
    QString testName;
    QHash<QString, bool> inputs;
    QHash<QString, bool> expectedOutputs;
    int delayMs = 10;
    QString description;
    
    TestVector() = default;
    TestVector(const QString &name, const QHash<QString, bool> &in, const QHash<QString, bool> &out)
        : testName(name), inputs(in), expectedOutputs(out) {}
};

// Test result tracking
struct TestResult {
    QString testName;
    bool passed = false;
    QString errorMessage;
    QHash<QString, bool> actualOutputs;
    QHash<QString, bool> expectedOutputs;
    double executionTimeMs = 0.0;
    
    TestResult() = default;
    TestResult(const QString &name, bool p, const QString &error = "")
        : testName(name), passed(p), errorMessage(error) {}
};

// Test suite for Arduino code validation
class ArduinoTestBench
{
public:
    explicit ArduinoTestBench(const QString &circuitName = "Test Circuit");
    
    // Test configuration
    void setVerbose(bool verbose) { m_verbose = verbose; }
    void setOutputFile(const QString &fileName) { m_outputFile = fileName; }
    
    // Test generation methods
    void generateDFlipFlopTests();
    void generateJKFlipFlopTests();
    void generateSRFlipFlopTests();
    void generateTFlipFlopTests();
    void generateCombinationalLogicTests();
    void generateClockingTests();
    void generateEdgeCaseTests();
    
    // Test execution
    bool runAllTests();
    bool runSpecificTest(const QString &testName);
    void generateTestReport();
    
    // Arduino simulation interface
    void setArduinoSimulator(const QString &simulatorPath) { m_simulatorPath = simulatorPath; }
    void setGeneratedCodeFile(const QString &codeFile) { m_generatedCodeFile = codeFile; }
    
    // Validation against wiRedPanda simulation
    void setWiRedPandaCircuitFile(const QString &circuitFile) { m_circuitFile = circuitFile; }
    bool compareWithSimulation();
    
    // Test statistics
    int getTotalTests() const { return m_testVectors.size(); }
    int getPassedTests() const { return m_passedCount; }
    int getFailedTests() const { return m_failedCount; }
    double getPassRate() const { return m_testVectors.isEmpty() ? 0.0 : (double)m_passedCount / m_testVectors.size() * 100.0; }
    
    // Custom test addition
    void addTestVector(const TestVector &test) { m_testVectors.append(test); }
    void addTestVectors(const QVector<TestVector> &tests) { m_testVectors.append(tests); }
    
private:
    // Test execution helpers
    bool executeArduinoTest(const TestVector &test, TestResult &result);
    bool executeSimulationTest(const TestVector &test, TestResult &result);
    bool compareResults(const TestResult &arduinoResult, const TestResult &simResult);
    
    // Test generation helpers
    TestVector createDFlipFlopTest(const QString &name, bool data, bool clock, bool preset, bool clear, 
                                   bool expectedQ, bool expectedQNot);
    TestVector createJKFlipFlopTest(const QString &name, bool j, bool k, bool clock, bool preset, bool clear,
                                    bool expectedQ, bool expectedQNot);
    TestVector createCombinationalTest(const QString &name, const QString &operation,
                                       const QVector<bool> &inputs, bool expectedOutput);
    
    // Arduino interaction
    bool compileArduinoCode();
    bool uploadToArduino();
    bool readArduinoOutputs(QHash<QString, bool> &outputs);
    void triggerArduinoInputs(const QHash<QString, bool> &inputs);
    
    // Simulation interaction
    bool loadWiRedPandaCircuit();
    bool setSimulationInputs(const QHash<QString, bool> &inputs);
    bool readSimulationOutputs(QHash<QString, bool> &outputs);
    
    // Reporting
    void writeTestReport();
    QString generateTestSummary();
    QString generateFailureAnalysis();
    
    // Member variables
    QString m_circuitName;
    QString m_simulatorPath;
    QString m_generatedCodeFile;
    QString m_circuitFile;
    QString m_outputFile;
    
    QVector<TestVector> m_testVectors;
    QVector<TestResult> m_testResults;
    
    int m_passedCount = 0;
    int m_failedCount = 0;
    bool m_verbose = false;
    
    // Test configuration
    static const int DEFAULT_DELAY_MS = 10;
    static const int EDGE_SETUP_TIME_MS = 5;
    static const int DEBOUNCE_TEST_DELAY_MS = 100;
};

// Specialized test generators
class FlipFlopTestGenerator
{
public:
    static QVector<TestVector> generateComprehensiveDFlipFlopTests();
    static QVector<TestVector> generateComprehensiveJKFlipFlopTests();
    static QVector<TestVector> generateComprehensiveSRFlipFlopTests();
    static QVector<TestVector> generateComprehensiveTFlipFlopTests();
    
    // Edge case tests
    static QVector<TestVector> generateTimingTests();
    static QVector<TestVector> generateDebouncingTests();
    static QVector<TestVector> generateAsynchronousControlTests();
    static QVector<TestVector> generateForbiddenStateTests();
};

// Combinational logic test generator
class CombinationalTestGenerator
{
public:
    static QVector<TestVector> generateTruthTableTests(const QString &operation, int inputCount);
    static QVector<TestVector> generateComplexLogicTests();
    static QVector<TestVector> generateGateDelayTests();
};

// Clock and timing test generator
class TimingTestGenerator
{
public:
    static QVector<TestVector> generateClockAccuracyTests();
    static QVector<TestVector> generateSetupHoldTests();
    static QVector<TestVector> generateRaceConditionTests();
    static QVector<TestVector> generateMultiClockTests();
};
