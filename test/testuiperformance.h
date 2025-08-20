// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "testuitestframework.h"

#include <QObject>
#include <QTest>

class Scene;
class Simulation;
class WorkSpace;
class GraphicsView;

/*!
 * @class TestUIPerformance
 * @brief Dedicated performance and responsiveness testing for UI operations
 *
 * This test class focuses specifically on measuring and validating UI performance,
 * responsiveness, and scalability under various load conditions.
 */
class TestUIPerformance : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    // Core performance tests
    void testElementAdditionPerformance();
    void testConnectionCreationPerformance();
    void testLargeCircuitRenderingPerformance();
    void testZoomingPerformance();

    // Responsiveness testing
    void testUIResponsivenessUnderElementLoad();
    void testUIResponsivenessUnderSimulationLoad();
    void testUIResponsivenessDuringFileOperations();
    void testUIResponsivenessWithManyConnections();

    // Memory and resource testing
    void testMemoryUsageGrowth();
    void testMemoryLeakDetection();
    void testResourceCleanupEfficiency();
    void testLongRunningSessionStability();

    // Animation and visual performance
    void testAnimationSmoothness();
    void testRenderingFrameRate();
    void testRedrawOptimization();
    void testViewportUpdateEfficiency();

    // Load testing scenarios
    void testPerformanceWith100Elements();
    void testPerformanceWith500Elements();
    void testPerformanceWith1000Elements();
    void testPerformanceWithComplexConnections();

    // Real-world workflow performance
    void testCircuitBuildingWorkflowPerformance();
    void testCircuitModificationWorkflowPerformance();
    void testSimulationStartStopPerformance();
    void testFileLoadSavePerformance();

    // Enhanced performance tests
    void testScrollingPerformance();
    void testZoomPerformance();
    void testDragDropPerformance();
    void testSelectionPerformance();
    void testRenderingPerformance();
    void testUndoRedoPerformance();
    void testSimulationPerformanceUnderLoad();
    void testConcurrentOperationPerformance();
    void testExtremeLimitPerformance();
    void testMemoryUsageUnderLoad();
    void testUIResponsivenessMetrics();
    void benchmarkElementTypes();
    void profileHotCodePaths();

private:
    // Performance measurement helpers
    void measureOperationPerformance(const QString &operationName, std::function<void()> operation);
    void validatePerformanceThresholds(const UITestFramework::PerformanceMetrics &metrics, const QString &operation);
    void logPerformanceBaseline(const QString &testName, qint64 timeMs);

    // Load generation helpers
    void addManyElementsToScene(int count, const QString &elementType = "And");
    void createManyConnections(int count);
    void simulateHeavySimulationLoad();
    void generateComplexCircuitTopology();

    // Memory testing helpers
    void captureMemoryBaseline();
    qint64 measureCurrentMemoryUsage();
    bool detectMemoryLeak(std::function<void()> operation, int iterations = 10);
    void validateMemoryCleanup();

    // Animation and rendering helpers
    void measureFrameRate(int durationMs = 2000);
    void validateSmoothScrolling();
    void testRenderingUnderLoad();
    void measureRedrawFrequency();

    // Benchmark helpers
    void runPerformanceBenchmark(const QString &benchmarkName, std::function<void()> benchmark);
    void comparePerformanceAgainstBaseline(const QString &testName, qint64 currentTimeMs);
    void generatePerformanceReport();

    WorkSpace *m_workspace{nullptr};
    Scene *m_scene{nullptr};
    Simulation *m_simulation{nullptr};
    GraphicsView *m_view{nullptr};

    // Performance tracking
    qint64 m_memoryBaseline{0};
    QHash<QString, qint64> m_performanceBaselines;
    QStringList m_performanceLog;
};
