// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/*!
 * @class TestElementEditor
 * @brief Test suite for ElementEditor UI component
 * 
 * Tests the functionality of the ElementEditor widget, which provides
 * the user interface for editing circuit element properties. Covers
 * dynamic control adaptation, element property synchronization, and
 * multi-element selection handling.
 */
class TestElementEditor : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // Basic construction and initialization
    void testConstruction();
    void testSceneAssignment();
    void testInitialState();
    
    // Element selection and property display
    void testSingleElementSelection();
    void testMultiElementSelection();
    void testElementDeselection();
    void testElementSelectionUpdate();
    
    // Property editing functionality
    void testInputSizeEditing();
    void testOutputSizeEditing();
    void testFrequencyEditing();
    void testTriggerEditing();
    void testLabelEditing();
    void testColorEditing();
    
    // Special element types
    void testClockElementProperties();
    void testDisplayElementProperties();
    void testTruthTableProperties();
    void testICElementProperties();
    
    // UI interaction and validation
    void testPropertyValidation();
    void testUIEnableDisable();
    void testPropertySynchronization();
    void testUndoRedoIntegration();
    
    // Error handling and edge cases
    void testInvalidElementHandling();
    void testMixedElementTypes();
    void testEmptySelection();
    void testNullSceneHandling();
};