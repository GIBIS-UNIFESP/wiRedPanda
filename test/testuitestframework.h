// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>

#include <QElapsedTimer>
#include <QObject>
#include <QPointF>
#include <QStringList>
#include <QTest>
#include <QWidget>

class WorkSpace;
class Scene;
class GraphicsView;
class QGraphicsItem;

/*!
 * @class UITestFramework
 * @brief Advanced UI testing utilities for comprehensive user interaction simulation
 *
 * This framework provides enhanced testing capabilities for:
 * - Realistic user workflow simulation
 * - Performance and responsiveness measurement
 * - Visual feedback validation
 * - Accessibility compliance testing
 * - Error scenario handling
 */
class UITestFramework
{
public:
    // Performance measurement
    struct PerformanceMetrics {
        qint64 operationTimeMs;
        double uiResponsiveness;
        bool hasUIFreeze;
        int frameDropCount;
    };

    // User interaction simulation
    static void simulateRealisticUserPause(int minMs = 100, int maxMs = 500);
    static void simulateTypicalUserBehavior(QWidget* widget);
    static void simulateComplexUserWorkflow(const QStringList &workflowSteps);

    // Mouse interaction enhancements
    static void simulateMouseHover(QWidget* widget, const QPoint &pos, int durationMs = 500);
    static void simulateMouseDragWithFeedback(QWidget* widget, const QPoint &start, const QPoint &end);
    static void simulateDoubleClickSequence(QWidget* widget, const QPoint &pos);
    static void simulateRightClickContextMenu(QWidget* widget, const QPoint &pos);

    // Keyboard interaction simulation
    static void simulateKeyboardNavigation(QWidget* startWidget, const QList<Qt::Key> &keySequence);
    static void simulateKeyboardShortcuts(QWidget* widget, const QStringList &shortcuts);
    static void simulateTypingSequence(QWidget* widget, const QString &text, int charDelayMs = 50);

    // Performance testing
    static PerformanceMetrics measureUIOperationPerformance(std::function<void()> operation);
    static bool detectUIFreeze(QWidget* widget, int timeoutMs = 500);
    static double calculateUIResponsiveness(QWidget* widget, int testDurationMs = 2000);
    static void validateAnimationSmoothness(QWidget* widget, int animationDurationMs = 1000);

    // Visual feedback validation
    static bool validateHoverStateVisualChange(QWidget* widget, const QPoint &pos);
    static bool validateSelectionHighlighting(QWidget* widget);
    static bool validateVisualFeedback(QWidget* widget, const QString &expectedState);
    static void captureUIState(QWidget* widget, const QString &stateName);
    static bool compareUIStates(const QString &state1, const QString &state2);

    // Accessibility testing
    static bool validateKeyboardAccessibility(QWidget* widget);
    static QStringList findAccessibilityIssues(QWidget* widget);
    static bool testTabOrderLogic(QWidget* widget);
    static bool validateFocusManagement(QWidget* widget);
    static bool checkARIALabelsPresence(QWidget* widget);

    // Error scenario testing
    static void simulateNetworkDisconnection();
    static void simulateFileSystemError();
    static void simulateMemoryPressure();
    static void simulateInvalidUserInput(QWidget* widget);
    static bool validateErrorRecovery(QWidget* widget, const QString &errorScenario);

    // Circuit-specific UI testing
    static void simulateCircuitBuildingWorkflow(WorkSpace* workspace);
    static bool validateCircuitConnectionFeedback(Scene* scene);
    static void testElementPlacementAccuracy(Scene* scene, const QStringList &elementTypes);
    static bool validateWireRoutingFeedback(Scene* scene);

    // Workflow validation
    static bool validateCompleteUserJourney(QWidget* widget, const QStringList &journeySteps);
    static void recordUserInteractionSession(QWidget* widget, const QString &sessionName);
    static void playbackUserSession(QWidget* widget, const QString &sessionName);

    // Cross-platform testing
    static void testHighDPIScaling(QWidget* widget, const QList<qreal> &dpiScales);
    static bool validatePlatformConsistency(QWidget* widget);
    static void testTouchscreenCompatibility(QWidget* widget);

    // Advanced validation
    static bool validateUIThreadResponsiveness();
    static void measureMemoryUsageUnderUILoad(QWidget* widget);
    static bool detectMemoryLeaksInUIOperations(std::function<void()> operation);

    // Helper methods (public for test access)
    static void waitForUIUpdate(int timeoutMs = 100);
    static bool isWidgetResponsive(QWidget* widget);
    static QPoint findWidgetCenter(QWidget* widget);
    static void logPerformanceMetric(const QString &operation, qint64 timeMs);
    static QString captureWidgetState(QWidget* widget);

private:
    static QElapsedTimer s_performanceTimer;
    static QStringList s_capturedStates;
    static int s_frameCounter;
};
