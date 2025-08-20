// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "uitestframework.h"

#include <QObject>
#include <QTest>

class MainWindow;
class WorkSpace;
class Scene;
class GraphicsView;

/*!
 * @class TestUIAccessibility
 * @brief Comprehensive accessibility testing for keyboard navigation and assistive technology support
 *
 * This test class validates that the application meets accessibility standards including:
 * - Keyboard-only navigation
 * - Screen reader compatibility
 * - Focus management
 * - ARIA compliance
 * - High contrast support
 * - Motor impairment accommodations
 */
class TestUIAccessibility : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    // Core keyboard navigation tests
    void testKeyboardOnlyCircuitBuilding();
    void testKeyboardOnlyMenuNavigation();
    void testKeyboardOnlyFileOperations();
    void testTabOrderLogic();
    void testKeyboardShortcuts();

    // Focus management tests
    void testFocusManagement();
    void testFocusIndicatorVisibility();
    void testFocusTrapping();
    void testModalDialogFocus();
    void testFocusRestoration();

    // Screen reader compatibility tests
    void testScreenReaderLabels();
    void testARIAAttributes();
    void testAccessibleNames();
    void testAccessibleDescriptions();
    void testLiveRegions();

    // Visual accessibility tests
    void testHighContrastSupport();
    void testColorBlindnessSupport();
    void testFontScaling();
    void testMinimumTargetSizes();
    void testVisualIndicators();

    // Motor impairment support tests
    void testStickyKeysSupport();
    void testMouseKeysSupport();
    void testDragAndDropAlternatives();
    void testLargeTargetAreas();
    void testTimeoutAdjustments();

    // Cognitive accessibility tests
    void testSimpleNavigation();
    void testConsistentInterfaces();
    void testErrorPrevention();
    void testHelpAndDocumentation();
    void testProgressIndicators();

    // Platform-specific accessibility tests
    void testWindowsAccessibility();
    void testMacOSAccessibility();
    void testLinuxAccessibility();
    void testCrossPlatformConsistency();

    // Compliance validation tests
    void testWCAGCompliance();
    void testSection508Compliance();
    void testAccessibilityStandards();
    void testKeyboardEquivalents();

private:
    // Keyboard navigation helpers
    void simulateCompleteKeyboardWorkflow();
    void testKeyboardCircuitCreation();
    void testKeyboardElementSelection();
    void testKeyboardConnectionCreation();
    void validateKeyboardOnlyAccess(QWidget* widget);

    // Focus management helpers
    void testFocusChain(const QList<QWidget*>& expectedOrder);
    void validateFocusIndicator(QWidget* widget);
    void testFocusWrapping();
    bool verifyFocusVisible(QWidget* widget);

    // Screen reader helpers
    void testAccessibilityTree(QWidget* widget);
    void validateScreenReaderInfo(QWidget* widget);
    void testAccessibilityEvents();
    QStringList getAccessibilityIssues(QWidget* widget);

    // Visual accessibility helpers
    void testContrastRatios(QWidget* widget);
    void validateColorIndependence();
    void testScalingBehavior(qreal scaleFactor);
    void validateTargetSizes();

    // Motor impairment helpers
    void testAlternativeInputMethods();
    void validateDragDropKeyboardAlternatives();
    void testTooltipKeyboardAccess();
    void validateTimeoutHandling();

    // Compliance testing helpers
    void runWCAGAudit(QWidget* widget);
    void validateKeyboardTraps();
    void testAccessibilityShortcuts();
    void validateAccessibilityDocumentation();

    // Error detection and reporting
    void reportAccessibilityViolation(const QString& violation, QWidget* widget = nullptr);
    void generateAccessibilityReport();
    QStringList findAccessibilityBarriers();

    // Testing utilities
    void simulateScreenReaderUsage();
    void simulateKeyboardOnlyUser();
    void simulateHighContrastMode();
    void simulateAssistiveTechnology();

    MainWindow* m_mainWindow{nullptr};
    WorkSpace* m_workspace{nullptr};
    Scene* m_scene{nullptr};
    GraphicsView* m_view{nullptr};

    // Accessibility tracking
    QStringList m_accessibilityViolations;
    QStringList m_accessibilityWarnings;
    QHash<QString, bool> m_complianceChecks;
};
