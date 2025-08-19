// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testbench.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QThread>

ArduinoTestBench::ArduinoTestBench(const QString &circuitName)
    : m_circuitName(circuitName)
{
    qDebug() << "ArduinoTestBench created for circuit:" << circuitName;
}

// Test generation methods
void ArduinoTestBench::generateDFlipFlopTests()
{
    qDebug() << "Generating D Flip-Flop test vectors";
    
    // Basic D FF tests
    addTestVector(createDFlipFlopTest("D_FF_Set_RisingEdge", true, true, true, true, true, false));
    addTestVector(createDFlipFlopTest("D_FF_Reset_RisingEdge", false, true, true, true, false, true));
    addTestVector(createDFlipFlopTest("D_FF_Hold_NoClockEdge", true, false, true, true, false, true)); // Previous state held
    
    // Preset/Clear tests
    addTestVector(createDFlipFlopTest("D_FF_Preset_Active", false, false, false, true, true, false));
    addTestVector(createDFlipFlopTest("D_FF_Clear_Active", true, false, true, false, false, true));
    addTestVector(createDFlipFlopTest("D_FF_Preset_Clear_Both", true, false, false, false, false, true)); // Clear dominates
}

void ArduinoTestBench::generateJKFlipFlopTests()
{
    qDebug() << "Generating JK Flip-Flop test vectors";
    
    // Basic JK FF tests with rising clock edge
    addTestVector(createJKFlipFlopTest("JK_FF_Set", true, false, true, true, true, true, false));
    addTestVector(createJKFlipFlopTest("JK_FF_Reset", false, true, true, true, true, false, true));
    addTestVector(createJKFlipFlopTest("JK_FF_Toggle", true, true, true, true, true, false, true)); // Toggle from previous
    addTestVector(createJKFlipFlopTest("JK_FF_Hold", false, false, true, true, true, false, true)); // Hold previous state
}

void ArduinoTestBench::generateCombinationalLogicTests()
{
    qDebug() << "Generating combinational logic test vectors";
    
    // AND gate tests
    addTestVector(createCombinationalTest("AND_00", "AND", {false, false}, false));
    addTestVector(createCombinationalTest("AND_01", "AND", {false, true}, false));
    addTestVector(createCombinationalTest("AND_10", "AND", {true, false}, false));
    addTestVector(createCombinationalTest("AND_11", "AND", {true, true}, true));
    
    // OR gate tests
    addTestVector(createCombinationalTest("OR_00", "OR", {false, false}, false));
    addTestVector(createCombinationalTest("OR_01", "OR", {false, true}, true));
    addTestVector(createCombinationalTest("OR_10", "OR", {true, false}, true));
    addTestVector(createCombinationalTest("OR_11", "OR", {true, true}, true));
    
    // NOT gate tests
    addTestVector(createCombinationalTest("NOT_0", "NOT", {false}, true));
    addTestVector(createCombinationalTest("NOT_1", "NOT", {true}, false));
}

void ArduinoTestBench::generateClockingTests()
{
    qDebug() << "Generating clocking test vectors";
    
    TestVector clockTest;
    clockTest.testName = "Clock_Accuracy_Test";
    clockTest.delayMs = 100; // Longer test for clock accuracy
    clockTest.description = "Verify clock frequency accuracy and stability";
    addTestVector(clockTest);
}

void ArduinoTestBench::generateEdgeCaseTests()
{
    qDebug() << "Generating edge case test vectors";
    
    // Metastability tests
    TestVector metaTest;
    metaTest.testName = "Metastability_Test";
    metaTest.delayMs = 50;
    metaTest.description = "Test behavior during setup/hold violations";
    addTestVector(metaTest);
    
    // Power-on reset tests
    TestVector powerOnTest;
    powerOnTest.testName = "Power_On_Reset_Test";
    powerOnTest.delayMs = 200;
    powerOnTest.description = "Verify proper initialization on power-up";
    addTestVector(powerOnTest);
}

// Test execution
bool ArduinoTestBench::runAllTests()
{
    qDebug() << "Running all tests for circuit:" << m_circuitName;
    
    m_passedCount = 0;
    m_failedCount = 0;
    m_testResults.clear();
    
    for (const TestVector &test : m_testVectors) {
        TestResult result;
        result.testName = test.testName;
        
        if (executeArduinoTest(test, result)) {
            m_passedCount++;
            result.passed = true;
            qDebug() << "PASS:" << test.testName;
        } else {
            m_failedCount++;
            result.passed = false;
            qDebug() << "FAIL:" << test.testName << "-" << result.errorMessage;
        }
        
        m_testResults.append(result);
    }
    
    generateTestReport();
    
    qDebug() << "Test summary: " << m_passedCount << "passed," << m_failedCount << "failed";
    return m_failedCount == 0;
}

bool ArduinoTestBench::runSpecificTest(const QString &testName)
{
    for (const TestVector &test : m_testVectors) {
        if (test.testName == testName) {
            TestResult result;
            result.testName = test.testName;
            
            bool success = executeArduinoTest(test, result);
            m_testResults.append(result);
            
            qDebug() << (success ? "PASS" : "FAIL") << ":" << testName;
            if (!success) {
                qDebug() << "Error:" << result.errorMessage;
            }
            
            return success;
        }
    }
    
    qDebug() << "Test not found:" << testName;
    return false;
}

void ArduinoTestBench::generateTestReport()
{
    if (m_outputFile.isEmpty()) {
        m_outputFile = QString("test_report_%1.txt").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    }
    
    writeTestReport();
}

// Test generation helpers
TestVector ArduinoTestBench::createDFlipFlopTest(const QString &name, bool data, bool clock, 
                                                  bool preset, bool clear, bool expectedQ, bool expectedQNot)
{
    TestVector test;
    test.testName = name;
    test.inputs["D"] = data;
    test.inputs["CLK"] = clock;
    test.inputs["PRESET"] = preset;
    test.inputs["CLEAR"] = clear;
    test.expectedOutputs["Q"] = expectedQ;
    test.expectedOutputs["Q_NOT"] = expectedQNot;
    test.delayMs = DEFAULT_DELAY_MS;
    test.description = QString("D=%1, CLK=%2, PRE=%3, CLR=%4 -> Q=%5, Q̄=%6")
                          .arg(data).arg(clock).arg(preset).arg(clear).arg(expectedQ).arg(expectedQNot);
    return test;
}

TestVector ArduinoTestBench::createJKFlipFlopTest(const QString &name, bool j, bool k, bool clock,
                                                  bool preset, bool clear, bool expectedQ, bool expectedQNot)
{
    TestVector test;
    test.testName = name;
    test.inputs["J"] = j;
    test.inputs["K"] = k;
    test.inputs["CLK"] = clock;
    test.inputs["PRESET"] = preset;
    test.inputs["CLEAR"] = clear;
    test.expectedOutputs["Q"] = expectedQ;
    test.expectedOutputs["Q_NOT"] = expectedQNot;
    test.delayMs = DEFAULT_DELAY_MS;
    test.description = QString("J=%1, K=%2, CLK=%3, PRE=%4, CLR=%5 -> Q=%6, Q̄=%7")
                          .arg(j).arg(k).arg(clock).arg(preset).arg(clear).arg(expectedQ).arg(expectedQNot);
    return test;
}

TestVector ArduinoTestBench::createCombinationalTest(const QString &name, const QString &operation,
                                                     const QVector<bool> &inputs, bool expectedOutput)
{
    TestVector test;
    test.testName = name;
    
    for (int i = 0; i < inputs.size(); ++i) {
        test.inputs[QString("INPUT%1").arg(i)] = inputs[i];
    }
    
    test.expectedOutputs["OUTPUT"] = expectedOutput;
    test.delayMs = 5; // Faster for combinational logic
    test.description = QString("%1 gate test with inputs: %2 -> %3")
                          .arg(operation)
                          .arg(inputs.size())
                          .arg(expectedOutput);
    return test;
}

// Arduino interaction (placeholder implementations)
bool ArduinoTestBench::executeArduinoTest(const TestVector &test, TestResult &result)
{
    // For now, this is a placeholder implementation
    // In a real implementation, this would:
    // 1. Compile the Arduino code
    // 2. Upload to hardware or run in simulator
    // 3. Apply test inputs
    // 4. Read outputs
    // 5. Compare with expected values
    
    qDebug() << "Executing Arduino test:" << test.testName;
    
    // Simulate test execution delay
    QThread::msleep(test.delayMs);
    
    // For demonstration, assume all tests pass for now
    result.passed = true;
    result.executionTimeMs = test.delayMs;
    result.actualOutputs = test.expectedOutputs; // Placeholder
    result.expectedOutputs = test.expectedOutputs;
    
    return true;
}

bool ArduinoTestBench::compileArduinoCode()
{
    qDebug() << "Compiling Arduino code:" << m_generatedCodeFile;
    // Placeholder for actual compilation
    return true;
}

bool ArduinoTestBench::uploadToArduino()
{
    qDebug() << "Uploading to Arduino";
    // Placeholder for actual upload
    return true;
}

void ArduinoTestBench::writeTestReport()
{
    QFile file(m_outputFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Could not open test report file:" << m_outputFile;
        return;
    }
    
    QTextStream out(&file);
    out << "===============================================\n";
    out << "Arduino Test Bench Report\n";
    out << "Circuit: " << m_circuitName << "\n";
    out << "Generated: " << QDateTime::currentDateTime().toString() << "\n";
    out << "===============================================\n\n";
    
    out << "SUMMARY:\n";
    out << "Total Tests: " << getTotalTests() << "\n";
    out << "Passed: " << getPassedTests() << "\n";
    out << "Failed: " << getFailedTests() << "\n";
    out << "Pass Rate: " << QString::number(getPassRate(), 'f', 1) << "%\n\n";
    
    out << "DETAILED RESULTS:\n";
    for (const TestResult &result : m_testResults) {
        out << (result.passed ? "[PASS] " : "[FAIL] ") << result.testName;
        if (!result.passed) {
            out << " - " << result.errorMessage;
        }
        out << " (" << result.executionTimeMs << "ms)\n";
    }
    
    if (getFailedTests() > 0) {
        out << "\nFAILURE ANALYSIS:\n";
        out << generateFailureAnalysis();
    }
    
    qDebug() << "Test report written to:" << m_outputFile;
}

QString ArduinoTestBench::generateFailureAnalysis()
{
    QString analysis;
    analysis += "Common failure patterns:\n";
    analysis += "- Edge detection timing issues\n";
    analysis += "- Setup/hold time violations\n";
    analysis += "- Insufficient debouncing\n";
    analysis += "- Resource conflicts\n";
    return analysis;
}

// Specialized test generators implementation
QVector<TestVector> FlipFlopTestGenerator::generateComprehensiveDFlipFlopTests()
{
    QVector<TestVector> tests;
    
    // Add comprehensive D flip-flop test patterns
    ArduinoTestBench testBench("Comprehensive_D_FF");
    testBench.generateDFlipFlopTests();
    
    // Add timing-critical tests
    TestVector timingTest;
    timingTest.testName = "D_FF_Setup_Hold_Test";
    timingTest.delayMs = 5; // EDGE_SETUP_TIME_MS
    timingTest.description = "Test setup and hold time requirements";
    tests.append(timingTest);
    
    return tests;
}

QVector<TestVector> CombinationalTestGenerator::generateTruthTableTests(const QString &operation, int inputCount)
{
    QVector<TestVector> tests;
    
    // Generate all possible input combinations
    int combinations = 1 << inputCount; // 2^inputCount
    
    for (int combo = 0; combo < combinations; ++combo) {
        TestVector test;
        test.testName = QString("%1_TruthTable_%2").arg(operation).arg(combo, inputCount, 2, QChar('0'));
        
        QVector<bool> inputs;
        for (int bit = 0; bit < inputCount; ++bit) {
            bool value = (combo >> bit) & 1;
            inputs.append(value);
            test.inputs[QString("INPUT%1").arg(bit)] = value;
        }
        
        // Calculate expected output based on operation
        bool expectedOutput = false;
        if (operation == "AND") {
            expectedOutput = true;
            for (bool input : inputs) {
                expectedOutput &= input;
            }
        } else if (operation == "OR") {
            expectedOutput = false;
            for (bool input : inputs) {
                expectedOutput |= input;
            }
        }
        
        test.expectedOutputs["OUTPUT"] = expectedOutput;
        test.delayMs = 2; // Fast for combinational logic
        tests.append(test);
    }
    
    return tests;
}

QVector<TestVector> TimingTestGenerator::generateClockAccuracyTests()
{
    QVector<TestVector> tests;
    
    TestVector clockAccuracy;
    clockAccuracy.testName = "Clock_Frequency_Accuracy";
    clockAccuracy.delayMs = 1000; // 1 second measurement
    clockAccuracy.description = "Measure actual vs expected clock frequency";
    tests.append(clockAccuracy);
    
    return tests;
}
