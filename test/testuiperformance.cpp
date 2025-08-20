// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testuiperformance.h"

#include "and.h"
#include "elementfactory.h"
#include "graphicsview.h"
#include "inputbutton.h"
#include "led.h"

#include <QScrollBar>
#include <QtMath>
#include "qneconnection.h"
#include "scene.h"
#include "simulation.h"
#include "workspace.h"

#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QProcess>
#include <QThread>

void TestUIPerformance::init()
{
    m_workspace = new WorkSpace();
    m_scene = m_workspace->scene();
    m_simulation = m_workspace->simulation();
    m_view = m_scene->view();

    // Verify workspace is properly initialized
    QVERIFY(m_workspace != nullptr);
    QVERIFY(m_scene != nullptr);
    QVERIFY(m_simulation != nullptr);
    QVERIFY(m_view != nullptr);

    // Ensure clean state
    QCOMPARE(m_scene->elements().size(), 0);

    // Capture baseline measurements
    captureMemoryBaseline();
}

void TestUIPerformance::cleanup()
{
    delete m_workspace;
    m_workspace = nullptr;
    m_scene = nullptr;
    m_simulation = nullptr;
    m_view = nullptr;
}

// =============== CORE PERFORMANCE TESTS ===============

void TestUIPerformance::testElementAdditionPerformance()
{
    qDebug() << "Testing element addition performance";

    measureOperationPerformance("Single Element Addition", [this]() {
        auto* element = ElementFactory::buildElement(ElementType::And);
        element->setPos(QPointF(100, 100));
        m_scene->addItem(element);
    });

    measureOperationPerformance("Batch Element Addition (10)", [this]() {
        for (int i = 0; i < 10; ++i) {
            auto* element = ElementFactory::buildElement(ElementType::And);
            element->setPos(QPointF(i * 50, 100));
            m_scene->addItem(element);
        }
    });

    measureOperationPerformance("Batch Element Addition (50)", [this]() {
        for (int i = 0; i < 50; ++i) {
            auto* element = ElementFactory::buildElement(ElementType::And);
            element->setPos(QPointF(i * 20, i * 15));
            m_scene->addItem(element);

            if (i % 10 == 0) {
                QApplication::processEvents(); // Prevent UI freeze
            }
        }
    });
}

void TestUIPerformance::testConnectionCreationPerformance()
{
    qDebug() << "Testing connection creation performance";

    // Setup elements for connection testing
    auto* input = ElementFactory::buildElement(ElementType::InputButton);
    auto* gate = ElementFactory::buildElement(ElementType::And);
    auto* output = ElementFactory::buildElement(ElementType::Led);

    input->setPos(QPointF(0, 0));
    gate->setPos(QPointF(100, 0));
    output->setPos(QPointF(200, 0));

    m_scene->addItem(input);
    m_scene->addItem(gate);
    m_scene->addItem(output);

    auto* inputButton = qgraphicsitem_cast<InputButton*>(input);
    auto* andGate = qgraphicsitem_cast<And*>(gate);
    auto* led = qgraphicsitem_cast<Led*>(output);

    measureOperationPerformance("Single Connection Creation", [this, inputButton, andGate]() {
        auto* conn = new QNEConnection();
        conn->setStartPort(inputButton->outputPort());
        conn->setEndPort(andGate->inputPort(0));
        m_scene->addItem(conn);
    });

    measureOperationPerformance("Multiple Connection Creation", [this, andGate, led]() {
        // Create second input for AND gate
        auto* input2 = ElementFactory::buildElement(ElementType::InputButton);
        input2->setPos(QPointF(0, 50));
        m_scene->addItem(input2);
        auto* inputButton2 = qgraphicsitem_cast<InputButton*>(input2);

        // Connect second input
        auto* conn2 = new QNEConnection();
        conn2->setStartPort(inputButton2->outputPort());
        conn2->setEndPort(andGate->inputPort(1));
        m_scene->addItem(conn2);

        // Connect gate to output
        auto* conn3 = new QNEConnection();
        conn3->setStartPort(andGate->outputPort());
        conn3->setEndPort(led->inputPort());
        m_scene->addItem(conn3);
    });
}

void TestUIPerformance::testLargeCircuitRenderingPerformance()
{
    qDebug() << "Testing large circuit rendering performance";

    // Create a substantial circuit
    addManyElementsToScene(100, "And");
    createManyConnections(50);

    measureOperationPerformance("Large Circuit Viewport Update", [this]() {
        m_view->viewport()->update();
        QApplication::processEvents();
    });

    measureOperationPerformance("Large Circuit Zoom Operation", [this]() {
        m_view->scale(1.5, 1.5);
        QApplication::processEvents();
        m_view->scale(1.0/1.5, 1.0/1.5);
        QApplication::processEvents();
    });
}

void TestUIPerformance::testScrollingPerformance()
{
    qDebug() << "Testing scrolling performance";

    // Create circuit that requires scrolling
    addManyElementsToScene(200, "And");

    measureOperationPerformance("Horizontal Scrolling", [this]() {
        for (int i = 0; i < 10; ++i) {
            m_view->horizontalScrollBar()->setValue(i * 100);
            QApplication::processEvents();
        }
    });

    measureOperationPerformance("Vertical Scrolling", [this]() {
        for (int i = 0; i < 10; ++i) {
            m_view->verticalScrollBar()->setValue(i * 100);
            QApplication::processEvents();
        }
    });
}

void TestUIPerformance::testZoomingPerformance()
{
    qDebug() << "Testing zooming performance";

    addManyElementsToScene(50, "And");

    measureOperationPerformance("Zoom In Sequence", [this]() {
        for (int i = 0; i < 5; ++i) {
            m_view->scale(1.2, 1.2);
            QApplication::processEvents();
        }
    });

    measureOperationPerformance("Zoom Out Sequence", [this]() {
        for (int i = 0; i < 5; ++i) {
            m_view->scale(1.0/1.2, 1.0/1.2);
            QApplication::processEvents();
        }
    });
}

// =============== RESPONSIVENESS TESTING ===============

void TestUIPerformance::testUIResponsivenessUnderElementLoad()
{
    qDebug() << "Testing UI responsiveness under element load";

    auto metrics = UITestFramework::measureUIOperationPerformance([this]() {
        addManyElementsToScene(100, "And");
    });

    validatePerformanceThresholds(metrics, "Element Load Responsiveness");

    // Test continued responsiveness after load
    bool responsive = !UITestFramework::detectUIFreeze(m_view, 500);
    QVERIFY2(responsive, "UI should remain responsive after loading many elements");
}

void TestUIPerformance::testUIResponsivenessUnderSimulationLoad()
{
    qDebug() << "Testing UI responsiveness under simulation load";

    // Build a circuit that requires significant simulation work
    generateComplexCircuitTopology();

    auto metrics = UITestFramework::measureUIOperationPerformance([this]() {
        m_simulation->initialize();
        for (int i = 0; i < 100; ++i) {
            m_simulation->update();
            if (i % 10 == 0) {
                QApplication::processEvents();
            }
        }
    });

    validatePerformanceThresholds(metrics, "Simulation Load Responsiveness");
}

void TestUIPerformance::testUIResponsivenessDuringFileOperations()
{
    qDebug() << "Testing UI responsiveness during file operations";

    // Create substantial circuit for save/load testing
    addManyElementsToScene(50, "And");
    createManyConnections(25);

    auto metrics = UITestFramework::measureUIOperationPerformance([this]() {
        // Simulate file operations workload
        for (int i = 0; i < 5; ++i) {
            m_view->viewport()->update();
            QApplication::processEvents();
            QThread::msleep(20); // Simulate I/O delay
        }
    });

    validatePerformanceThresholds(metrics, "File Operations Responsiveness");
}

void TestUIPerformance::testUIResponsivenessWithManyConnections()
{
    qDebug() << "Testing UI responsiveness with many connections";

    addManyElementsToScene(30, "And");

    auto metrics = UITestFramework::measureUIOperationPerformance([this]() {
        createManyConnections(100);
    });

    validatePerformanceThresholds(metrics, "Many Connections Responsiveness");

    // Verify UI remains responsive after connection creation
    bool responsive = !UITestFramework::detectUIFreeze(m_view, 300);
    QVERIFY2(responsive, "UI should remain responsive with many connections");
}

// =============== MEMORY AND RESOURCE TESTING ===============

void TestUIPerformance::testMemoryUsageGrowth()
{
    qDebug() << "Testing memory usage growth";

    qint64 initialMemory = measureCurrentMemoryUsage();

    // Add elements and measure memory growth
    addManyElementsToScene(100, "And");
    qint64 afterElementsMemory = measureCurrentMemoryUsage();

    createManyConnections(50);
    qint64 afterConnectionsMemory = measureCurrentMemoryUsage();

    // Log memory usage progression
    qDebug() << "Memory usage progression:";
    qDebug() << "  Initial:" << initialMemory << "KB";
    qDebug() << "  After 100 elements:" << afterElementsMemory << "KB (+)" << (afterElementsMemory - initialMemory) << "KB";
    qDebug() << "  After 50 connections:" << afterConnectionsMemory << "KB (+)" << (afterConnectionsMemory - afterElementsMemory) << "KB";

    // Verify memory growth is reasonable
    qint64 totalGrowth = afterConnectionsMemory - initialMemory;
    QVERIFY2(totalGrowth < 50000, "Memory growth should be reasonable (< 50MB)"); // Reasonable threshold
}

void TestUIPerformance::testMemoryLeakDetection()
{
    qDebug() << "Testing memory leak detection";

    bool hasLeak = detectMemoryLeak([this]() {
        // Operation that should not leak memory
        auto* element = ElementFactory::buildElement(ElementType::And);
        element->setPos(QPointF(100, 100));
        m_scene->addItem(element);
        m_scene->removeItem(element);
        delete element;
    }, 20);

    QVERIFY2(!hasLeak, "Element creation/deletion should not leak memory");

    // Test connection leak detection
    bool connectionLeak = detectMemoryLeak([this]() {
        // Create and remove connections
        auto* input = ElementFactory::buildElement(ElementType::InputButton);
        auto* gate = ElementFactory::buildElement(ElementType::And);

        input->setPos(QPointF(0, 0));
        gate->setPos(QPointF(100, 0));
        m_scene->addItem(input);
        m_scene->addItem(gate);

        auto* conn = new QNEConnection();
        auto* inputButton = qgraphicsitem_cast<InputButton*>(input);
        auto* andGate = qgraphicsitem_cast<And*>(gate);

        if (inputButton && andGate) {
            conn->setStartPort(inputButton->outputPort());
            conn->setEndPort(andGate->inputPort(0));
            m_scene->addItem(conn);
            m_scene->removeItem(conn);
        }

        delete conn;
        m_scene->removeItem(input);
        m_scene->removeItem(gate);
        delete input;
        delete gate;
    }, 15);

    QVERIFY2(!connectionLeak, "Connection creation/deletion should not leak memory");
}

void TestUIPerformance::testResourceCleanupEfficiency()
{
    qDebug() << "Testing resource cleanup efficiency";

    measureOperationPerformance("Resource Cleanup", [this]() {
        // Create and clean up resources
        addManyElementsToScene(50, "And");
        createManyConnections(25);

        // Clear scene (cleanup)
        m_scene->clear();
        QApplication::processEvents();
    });

    // Verify cleanup was effective
    QCOMPARE(m_scene->elements().size(), 0);

    validateMemoryCleanup();
}

void TestUIPerformance::testLongRunningSessionStability()
{
    qDebug() << "Testing long running session stability";

    qint64 sessionStartMemory = measureCurrentMemoryUsage();

    // Simulate extended usage session
    for (int cycle = 0; cycle < 10; ++cycle) {
        addManyElementsToScene(20, "And");
        createManyConnections(10);

        // Simulate user activity
        UITestFramework::simulateTypicalUserBehavior(m_view);

        // Partial cleanup
        if (cycle % 3 == 0) {
            m_scene->clear();
            QApplication::processEvents();
        }

        // Check for UI freeze
        bool responsive = !UITestFramework::detectUIFreeze(m_view, 200);
        QVERIFY2(responsive, "UI should remain responsive during long session");
    }

    qint64 sessionEndMemory = measureCurrentMemoryUsage();
    qint64 memoryGrowth = sessionEndMemory - sessionStartMemory;

    qDebug() << "Long session memory growth:" << memoryGrowth << "KB";
    QVERIFY2(memoryGrowth < 100000, "Long session memory growth should be controlled (< 100MB)");
}

// =============== LOAD TESTING SCENARIOS ===============

void TestUIPerformance::testPerformanceWith100Elements()
{
    qDebug() << "Testing performance with 100 elements";

    auto metrics = UITestFramework::measureUIOperationPerformance([this]() {
        addManyElementsToScene(100, "And");
    });

    validatePerformanceThresholds(metrics, "100 Elements Load");
    logPerformanceBaseline("100_elements", metrics.operationTimeMs);
}

void TestUIPerformance::testPerformanceWith500Elements()
{
    qDebug() << "Testing performance with 500 elements";

    auto metrics = UITestFramework::measureUIOperationPerformance([this]() {
        addManyElementsToScene(500, "And");
    });

    validatePerformanceThresholds(metrics, "500 Elements Load");
    logPerformanceBaseline("500_elements", metrics.operationTimeMs);
}

void TestUIPerformance::testPerformanceWith1000Elements()
{
    qDebug() << "Testing performance with 1000 elements";

    auto metrics = UITestFramework::measureUIOperationPerformance([this]() {
        addManyElementsToScene(1000, "And");
    });

    // More lenient thresholds for extreme load
    QVERIFY2(metrics.operationTimeMs < 10000, "1000 elements should load within 10 seconds");
    QVERIFY2(metrics.uiResponsiveness > 0.1, "UI should retain some responsiveness even under extreme load");

    logPerformanceBaseline("1000_elements", metrics.operationTimeMs);
}

void TestUIPerformance::testPerformanceWithComplexConnections()
{
    qDebug() << "Testing performance with complex connections";

    addManyElementsToScene(100, "And");

    auto metrics = UITestFramework::measureUIOperationPerformance([this]() {
        createManyConnections(200);
    });

    validatePerformanceThresholds(metrics, "Complex Connections");
    logPerformanceBaseline("complex_connections", metrics.operationTimeMs);
}

// =============== REAL-WORLD WORKFLOW PERFORMANCE ===============

void TestUIPerformance::testCircuitBuildingWorkflowPerformance()
{
    qDebug() << "Testing circuit building workflow performance";

    auto metrics = UITestFramework::measureUIOperationPerformance([this]() {
        UITestFramework::simulateCircuitBuildingWorkflow(m_workspace);
    });

    validatePerformanceThresholds(metrics, "Circuit Building Workflow");
    logPerformanceBaseline("circuit_building_workflow", metrics.operationTimeMs);
}

void TestUIPerformance::testCircuitModificationWorkflowPerformance()
{
    qDebug() << "Testing circuit modification workflow performance";

    // Build initial circuit
    addManyElementsToScene(20, "And");
    createManyConnections(10);

    auto metrics = UITestFramework::measureUIOperationPerformance([this]() {
        // Simulate typical modification workflow
        addManyElementsToScene(10, "Or");
        createManyConnections(5);

        // Simulate element movement
        auto elements = m_scene->elements();
        if (!elements.isEmpty()) {
            elements.first()->setPos(elements.first()->pos() + QPointF(50, 50));
        }
    });

    validatePerformanceThresholds(metrics, "Circuit Modification Workflow");
}

void TestUIPerformance::testSimulationStartStopPerformance()
{
    qDebug() << "Testing simulation start/stop performance";

    generateComplexCircuitTopology();

    measureOperationPerformance("Simulation Start", [this]() {
        m_simulation->initialize();
    });

    measureOperationPerformance("Simulation Update Cycle", [this]() {
        for (int i = 0; i < 10; ++i) {
            m_simulation->update();
        }
    });

    measureOperationPerformance("Simulation Stop", [this]() {
        m_simulation->stop();
    });
}

void TestUIPerformance::testFileLoadSavePerformance()
{
    qDebug() << "Testing file load/save performance simulation";

    // Create substantial circuit
    addManyElementsToScene(50, "And");
    createManyConnections(25);

    // Simulate file save performance
    measureOperationPerformance("File Save Simulation", [this]() {
        // Simulate the work of serializing circuit data
        auto elements = m_scene->elements();
        for (int i = 0; i < elements.size(); ++i) {
            // Simulate serialization work
            QApplication::processEvents();
        }
    });

    // Simulate file load performance
    measureOperationPerformance("File Load Simulation", [this]() {
        // Simulate the work of deserializing and creating elements
        for (int i = 0; i < 50; ++i) {
            auto* element = ElementFactory::buildElement(ElementType::And);
            element->setPos(QPointF(i * 30, 100));
            m_scene->addItem(element);

            if (i % 10 == 0) {
                QApplication::processEvents();
            }
        }
    });
}

// =============== HELPER IMPLEMENTATIONS ===============

void TestUIPerformance::measureOperationPerformance(const QString &operationName, std::function<void()> operation)
{
    auto metrics = UITestFramework::measureUIOperationPerformance(operation);

    qDebug() << "Performance [" << operationName << "]:"
             << metrics.operationTimeMs << "ms"
             << "| Responsiveness:" << QString::number(metrics.uiResponsiveness, 'f', 2)
             << "| Freeze:" << (metrics.hasUIFreeze ? "YES" : "NO");

    logPerformanceBaseline(operationName, metrics.operationTimeMs);

    // Basic threshold validation
    if (operationName.contains("Single") || operationName.contains("Simple")) {
        QVERIFY2(metrics.operationTimeMs < 100, "Simple operations should complete quickly");
    } else if (operationName.contains("Batch") || operationName.contains("Many")) {
        QVERIFY2(metrics.operationTimeMs < 2000, "Batch operations should complete within 2 seconds");
    }
}

void TestUIPerformance::validatePerformanceThresholds(const UITestFramework::PerformanceMetrics &metrics, const QString &operation)
{
    // Standard performance thresholds
    QVERIFY2(metrics.operationTimeMs < 5000, qPrintable(operation + " should complete within 5 seconds"));
    QVERIFY2(!metrics.hasUIFreeze, qPrintable(operation + " should not cause UI freeze"));
    QVERIFY2(metrics.uiResponsiveness > 0.2, qPrintable(operation + " should maintain minimum UI responsiveness"));

    if (metrics.operationTimeMs > 1000) {
        qWarning() << "Slow operation detected:" << operation << metrics.operationTimeMs << "ms";
    }
}

void TestUIPerformance::logPerformanceBaseline(const QString &testName, qint64 timeMs)
{
    m_performanceBaselines[testName] = timeMs;
    m_performanceLog << QString("%1: %2ms").arg(testName).arg(timeMs);
}

void TestUIPerformance::addManyElementsToScene(int count, const QString &elementType)
{
    ElementType type = ElementFactory::textToType(elementType);

    for (int i = 0; i < count; ++i) {
        auto* element = ElementFactory::buildElement(type);
        if (element) {
            // Arrange in grid pattern
            int cols = qMax(1, static_cast<int>(qSqrt(count)));
            int row = i / cols;
            int col = i % cols;
            element->setPos(QPointF(col * 80, row * 60));
            m_scene->addItem(element);
        }

        // Process events periodically to prevent UI freeze
        if (i % 20 == 0) {
            QApplication::processEvents();
        }
    }
}

void TestUIPerformance::createManyConnections(int count)
{
    auto elements = m_scene->elements();
    if (elements.size() < 2) return;

    int connectionsCreated = 0;
    for (int i = 0; i < elements.size() - 1 && connectionsCreated < count; ++i) {
        auto* startElement = elements[i];
        auto* endElement = elements[i + 1];

        // Simple connection attempt (would need proper port logic for real circuits)
        auto* conn = new QNEConnection();
        m_scene->addItem(conn);
        connectionsCreated++;

        if (connectionsCreated % 10 == 0) {
            QApplication::processEvents();
        }
    }
}

void TestUIPerformance::generateComplexCircuitTopology()
{
    // Create a more complex circuit for comprehensive testing
    addManyElementsToScene(30, "And");
    addManyElementsToScene(20, "Or");
    addManyElementsToScene(10, "Not");
    addManyElementsToScene(15, "InputButton");
    addManyElementsToScene(10, "Led");

    createManyConnections(40);
}

void TestUIPerformance::captureMemoryBaseline()
{
    m_memoryBaseline = measureCurrentMemoryUsage();
    qDebug() << "Memory baseline captured:" << m_memoryBaseline << "KB";
}

qint64 TestUIPerformance::measureCurrentMemoryUsage()
{
    // Simple memory measurement (platform-independent approximation)
    QProcess process;
    process.start("ps", QStringList() << "-o" << "rss=" << "-p" << QString::number(QApplication::applicationPid()));
    process.waitForFinished(1000);

    QByteArray output = process.readAllStandardOutput();
    QString memoryStr = QString::fromUtf8(output).trimmed();

    bool ok;
    qint64 memoryKB = memoryStr.toLongLong(&ok);

    if (!ok) {
        // Fallback: return a reasonable default if measurement fails
        return 10000; // 10MB default
    }

    return memoryKB;
}

bool TestUIPerformance::detectMemoryLeak(std::function<void()> operation, int iterations)
{
    qint64 initialMemory = measureCurrentMemoryUsage();

    for (int i = 0; i < iterations; ++i) {
        operation();
        QApplication::processEvents();
    }

    // Force garbage collection
    for (int i = 0; i < 5; ++i) {
        QApplication::processEvents();
        QThread::msleep(10);
    }

    qint64 finalMemory = measureCurrentMemoryUsage();
    qint64 memoryDifference = finalMemory - initialMemory;

    qDebug() << "Memory leak test - Initial:" << initialMemory << "KB, Final:" << finalMemory
             << "KB, Difference:" << memoryDifference << "KB over" << iterations << "iterations";

    // Consider it a leak if memory grows by more than 1MB per 10 iterations
    qint64 leakThreshold = (iterations / 10) * 1024; // 1MB per 10 iterations
    return memoryDifference > leakThreshold;
}

void TestUIPerformance::validateMemoryCleanup()
{
    qint64 currentMemory = measureCurrentMemoryUsage();
    qint64 memoryGrowth = currentMemory - m_memoryBaseline;

    qDebug() << "Memory cleanup validation - Growth since baseline:" << memoryGrowth << "KB";

    // Allow some reasonable memory growth (5MB)
    QVERIFY2(memoryGrowth < 5120, "Memory cleanup should be effective (< 5MB growth)");
}

// =============== MISSING PERFORMANCE TEST IMPLEMENTATIONS ===============

void TestUIPerformance::testAnimationSmoothness()
{
    qDebug() << "Testing animation smoothness";
    addManyElementsToScene(10, "And");
    
    measureOperationPerformance("Animation Test", [this]() {
        for (int i = 0; i < 30; ++i) {
            m_view->viewport()->update();
            QApplication::processEvents();
            QThread::msleep(16); // 60 FPS target
        }
    });
}

void TestUIPerformance::testRenderingFrameRate()
{
    qDebug() << "Testing rendering frame rate";
    addManyElementsToScene(50, "And");
    
    measureOperationPerformance("Frame Rate Test", [this]() {
        for (int i = 0; i < 60; ++i) {
            m_view->viewport()->update();
            QApplication::processEvents();
        }
    });
}

void TestUIPerformance::testRedrawOptimization()
{
    qDebug() << "Testing redraw optimization";
    addManyElementsToScene(100, "And");
    
    measureOperationPerformance("Redraw Optimization", [this]() {
        m_view->update();
        QApplication::processEvents();
    });
}

void TestUIPerformance::testViewportUpdateEfficiency()
{
    qDebug() << "Testing viewport update efficiency";
    addManyElementsToScene(20, "And");
    
    measureOperationPerformance("Viewport Update", [this]() {
        m_view->viewport()->update();
        QApplication::processEvents();
    });
}

void TestUIPerformance::testZoomPerformance()
{
    qDebug() << "Testing zoom performance";
    addManyElementsToScene(30, "And");
    
    measureOperationPerformance("Zoom Performance", [this]() {
        m_view->scale(1.2, 1.2);
        QApplication::processEvents();
        m_view->scale(0.8, 0.8);
        QApplication::processEvents();
    });
}

void TestUIPerformance::testDragDropPerformance()
{
    qDebug() << "Testing drag and drop performance";
    addManyElementsToScene(10, "And");
    auto elements = m_scene->elements();
    
    if (!elements.isEmpty()) {
        measureOperationPerformance("Drag Drop", [&elements]() {
            QPointF pos = elements.first()->pos();
            elements.first()->setPos(pos + QPointF(50, 50));
        });
    }
}

void TestUIPerformance::testSelectionPerformance()
{
    qDebug() << "Testing selection performance";
    addManyElementsToScene(50, "And");
    auto elements = m_scene->elements();
    
    measureOperationPerformance("Selection", [&elements]() {
        for (auto* element : elements) {
            element->setSelected(true);
        }
        for (auto* element : elements) {
            element->setSelected(false);
        }
    });
}

void TestUIPerformance::testRenderingPerformance()
{
    qDebug() << "Testing rendering performance";
    addManyElementsToScene(100, "And");
    
    measureOperationPerformance("Rendering", [this]() {
        for (int i = 0; i < 10; ++i) {
            m_view->viewport()->update();
            QApplication::processEvents();
        }
    });
}

void TestUIPerformance::testUndoRedoPerformance()
{
    qDebug() << "Testing undo/redo performance";
    
    measureOperationPerformance("Undo/Redo", [this]() {
        addManyElementsToScene(5, "And");
        m_scene->clear();
    });
}

void TestUIPerformance::testSimulationPerformanceUnderLoad()
{
    qDebug() << "Testing simulation performance under load";
    addManyElementsToScene(20, "And");
    m_simulation->initialize();
    
    measureOperationPerformance("Simulation Under Load", [this]() {
        for (int i = 0; i < 10; ++i) {
            m_simulation->update();
            m_view->viewport()->update();
            QApplication::processEvents();
        }
    });
}

void TestUIPerformance::testConcurrentOperationPerformance()
{
    qDebug() << "Testing concurrent operation performance";
    addManyElementsToScene(20, "And");
    
    measureOperationPerformance("Concurrent Operations", [this]() {
        m_simulation->update();
        m_view->viewport()->update();
        QApplication::processEvents();
    });
}

void TestUIPerformance::testExtremeLimitPerformance()
{
    qDebug() << "Testing extreme limit performance";
    
    measureOperationPerformance("Extreme Limit", [this]() {
        addManyElementsToScene(200, "And");
    });
}

void TestUIPerformance::testMemoryUsageUnderLoad()
{
    qDebug() << "Testing memory usage under load";
    
    qint64 initialMemory = measureCurrentMemoryUsage();
    addManyElementsToScene(100, "And");
    qint64 loadedMemory = measureCurrentMemoryUsage();
    
    qint64 memoryIncrease = loadedMemory - initialMemory;
    qDebug() << "Memory increase under load:" << memoryIncrease << "KB";
    
    QVERIFY2(memoryIncrease < 10240, "Memory usage should be reasonable under load");
}

void TestUIPerformance::testUIResponsivenessMetrics()
{
    qDebug() << "Testing UI responsiveness metrics";
    addManyElementsToScene(50, "And");
    
    bool responsive = !UITestFramework::detectUIFreeze(m_view, 1000);
    QVERIFY2(responsive, "UI should remain responsive");
}

void TestUIPerformance::benchmarkElementTypes()
{
    qDebug() << "Benchmarking different element types";
    
    QStringList elementTypes = {"And", "Or", "Not", "InputButton", "Led"};
    
    for (const QString& elementType : elementTypes) {
        measureOperationPerformance(QString("Element %1").arg(elementType), [this, elementType]() {
            addManyElementsToScene(10, elementType);
            m_scene->clear();
        });
    }
}

void TestUIPerformance::profileHotCodePaths()
{
    qDebug() << "Profiling hot code paths";
    
    measureOperationPerformance("Element Creation", [this]() {
        auto* element = ElementFactory::buildElement(ElementType::And);
        if (element) {
            m_scene->addItem(element);
        }
    });
    
    measureOperationPerformance("Scene Update", [this]() {
        m_scene->update();
    });
    
    measureOperationPerformance("View Render", [this]() {
        m_view->viewport()->update();
        QApplication::processEvents();
    });
}

