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
 * @class TestUIInteraction
 * @brief Tests UI interactions including mouse clicks and drag operations to build circuits
 *
 * This test class verifies that circuits can be built through UI interactions,
 * simulating user clicks, drags, and element placement operations.
 */
class TestUIInteraction : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    // Enhanced UI interaction tests
    void testBuildSimpleCircuitWithClicks();
    void testDragAndDropElements();
    void testConnectionCreation();

    // Comprehensive workflow testing
    void testCompleteCircuitBuildingWorkflow();
    void testCircuitModificationWorkflow();
    void testErrorRecoveryWorkflow();
    void testUndoRedoInteractionWorkflow();

    // Advanced interaction patterns
    void testMultiSelectAndBulkOperations();
    void testContextMenuInteractions();
    void testKeyboardOnlyNavigation();
    void testDragDropWithSnapping();
    void testElementAlignmentAndGrid();

    // UI feedback validation
    void testHoverStateVisualFeedback();
    void testSelectionHighlighting();
    void testConnectionPreviewFeedback();
    void testInvalidActionFeedback();

    // Performance and responsiveness
    void testUIResponsivenessUnderLoad();
    void testLargeCircuitUIPerformance();
    void testScrollingAndZoomPerformance();

    // Error scenario testing
    void testUIErrorHandling();
    void testInvalidInputRecovery();
    void testUIStateConsistency();

private:
    // Enhanced helper functions
    void setupWorkspace();
    void simulateMouseClick(const QPointF &pos);
    void simulateMouseDrag(const QPointF &start, const QPointF &end);
    void verifyCircuitBuilt();
    void addElementToScene(const QString &elementType, const QPointF &position);
    void connectElements(const QPointF &outputPos, const QPointF &inputPos);
    int countConnections() const;

    // Advanced workflow helpers
    void executeComplexUserWorkflow(const QStringList &workflowSteps);
    void validateVisualFeedbackResponse(const QString &action);
    void simulateRealisticUserBehavior();
    void testKeyboardAccessibility();
    void validateUIPerformanceMetrics();

    // Circuit-specific helpers
    void buildComplexTestCircuit();
    void simulateCircuitModification();
    void testCircuitValidationFeedback();
    void validateConnectionRouting();

    // Error and edge case helpers
    void simulateInvalidUserActions();
    bool testUIRecoveryScenarios();
    void validateErrorMessageDisplay();
    bool verifyUIStateConsistency();

    WorkSpace *m_workspace{nullptr};
    Scene *m_scene{nullptr};
    Simulation *m_simulation{nullptr};
    GraphicsView *m_view{nullptr};
};
