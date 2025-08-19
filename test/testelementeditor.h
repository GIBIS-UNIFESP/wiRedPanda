// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class ElementEditor;
class GraphicElement;
class Scene;
class WorkSpace;

class TestElementEditor : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    // Basic functionality tests
    void testElementEditorCreation();
    void testSetScene();
    void testElementSelection();
    void testMultipleElementSelection();

    // Element property editing tests
    void testLabelEditing();
    void testInputSizeModification();
    void testOutputSizeModification();
    void testElementRotation();
    void testPriorityChange();

    // Color and skin tests
    void testColorSelection();
    // void testSkinChanges(); // Opens modal dialog
    void testDefaultSkin();
    // void testSkinUpdate(); // Opens modal dialog

    // Truth table tests (modal dialogs disabled for headless testing)
    // void testTruthTableCreation(); // Opens modal dialog
    void testTruthTableEditing();
    void testTruthTableValidation();

    // Audio tests (modal dialogs disabled for headless testing)
    // void testAudioConfiguration(); // Opens modal dialog
    // void testAudioBoxDialog(); // Opens modal dialog

    // Element type specific tests
    void testClockFrequencyEditing();
    void testDelayConfiguration();
    void testTriggerConfiguration();

    // Multi-selection tests
    void testMixedElementSelection();
    void testBulkPropertyChanges();
    void testPropertyConflictHandling();

    // Validation and error handling tests
    void testInvalidInputHandling();
    void testPropertyValidation();
    void testUndoRedoIntegration();

    // UI interaction tests (removed modal dialog tests)
    // void testContextMenu(); // Requires user input
    void testKeyboardShortcuts();
    void testEventFiltering();

    // Edge case tests
    void testEmptySelection();
    void testSingleElementTypes();
    void testMaxInputOutputLimits();

    // Theme and localization tests
    void testThemeUpdates();
    void testRetranslation();

private:
    // Helper methods for creating test scenarios
    WorkSpace* createTestWorkspace();
    void selectElements(const QList<GraphicElement*>& elements);
    void simulateUserInput(const QString& property, const QString& value);
    void verifyPropertyChange(GraphicElement* element, const QString& property, const QString& expectedValue);
    QList<GraphicElement*> createMixedElementSelection();
    QList<GraphicElement*> createHomogeneousElementSelection();

    // UI interaction helpers
    void simulateColorSelection(const QString& colorName);
    void simulateTruthTableEdit(int row, int column, bool value);
    void simulateContextMenuAction(const QString& actionName);

    // Verification helpers
    void verifyUIState(const QString& expectedState);
    void verifyElementProperties(const QList<GraphicElement*>& elements);
    void verifyTruthTableState();

    // Test data
    WorkSpace* m_workspace = nullptr;
    Scene* m_scene = nullptr;
    ElementEditor* m_elementEditor = nullptr;
};
