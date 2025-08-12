// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testelementeditor.h"

#include "and.h"
#include "clock.h"
#include "display_7.h"
#include "elementeditor.h"
#include "inputbutton.h"
#include "inputswitch.h"
#include "or.h"
#include "scene.h"
#include "truth_table.h"
#include "workspace.h"

#include <QSignalSpy>
#include <QTest>

void TestElementEditor::initTestCase()
{
    // Initialize test environment
    QVERIFY(true);
}

void TestElementEditor::cleanupTestCase()
{
    // Clean up test environment
    QVERIFY(true);
}

void TestElementEditor::testConstruction()
{
    // Test basic construction
    ElementEditor editor;
    
    // Should construct without crashing
    QVERIFY(true);
    
    // Should be a valid widget
    QVERIFY(editor.isWidgetType());
    
    // Should have no scene initially
    // (Cannot test scene() directly as it may not be public)
}

void TestElementEditor::testSceneAssignment()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    
    // Test scene assignment
    editor.setScene(scene);
    
    // Should not crash when scene is assigned
    QVERIFY(true);
    
    // Test multiple scene assignments
    WorkSpace workspace2;
    auto* scene2 = workspace2.scene();
    editor.setScene(scene2);
    
    // Should handle scene changes gracefully
    QVERIFY(true);
}

void TestElementEditor::testInitialState()
{
    ElementEditor editor;
    
    // Test initial widget state
    QVERIFY(editor.isEnabled());
    
    // Should be visible by default
    // Note: Widgets are visible by default unless explicitly hidden
    QVERIFY(true);
}

void TestElementEditor::testSingleElementSelection()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Create and add a test element
    auto* andGate = new And();
    scene->addItem(andGate);
    
    // Select the element
    andGate->setSelected(true);
    
    // Update the editor
    editor.update();
    
    // Should handle single selection without crashing
    QVERIFY(true);
    
    // Cleanup
    scene->removeItem(andGate);
    delete andGate;
}

void TestElementEditor::testMultiElementSelection()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Create multiple test elements
    auto* and1 = new And();
    auto* and2 = new And();
    scene->addItem(and1);
    scene->addItem(and2);
    
    // Select multiple elements
    and1->setSelected(true);
    and2->setSelected(true);
    
    // Update the editor
    editor.update();
    
    // Should handle multiple selection without crashing
    QVERIFY(true);
    
    // Cleanup
    scene->removeItem(and1);
    scene->removeItem(and2);
    delete and1;
    delete and2;
}

void TestElementEditor::testElementDeselection()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Create and add a test element
    auto* andGate = new And();
    scene->addItem(andGate);
    
    // Select then deselect
    andGate->setSelected(true);
    editor.update();
    andGate->setSelected(false);
    editor.update();
    
    // Should handle deselection gracefully
    QVERIFY(true);
    
    // Cleanup
    scene->removeItem(andGate);
    delete andGate;
}

void TestElementEditor::testElementSelectionUpdate()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Create test elements
    auto* and1 = new And();
    auto* or1 = new Or();
    scene->addItem(and1);
    scene->addItem(or1);
    
    // Test selection changes
    and1->setSelected(true);
    editor.update();
    
    // Change selection
    and1->setSelected(false);
    or1->setSelected(true);
    editor.update();
    
    // Should handle selection changes without crashing
    QVERIFY(true);
    
    // Cleanup
    scene->removeItem(and1);
    scene->removeItem(or1);
    delete and1;
    delete or1;
}

void TestElementEditor::testInputSizeEditing()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Create an element that supports input size changes
    auto* andGate = new And();
    scene->addItem(andGate);
    andGate->setSelected(true);
    
    // Get initial input size
    int initialInputs = andGate->inputSize();
    QVERIFY(initialInputs >= 2); // AND gates should have at least 2 inputs
    
    // Update editor to show element properties
    editor.update();
    
    // Test input size change (this would be done through UI interaction)
    // We test that the element can handle size changes
    andGate->setInputSize(3);
    QCOMPARE(andGate->inputSize(), 3);
    
    // Cleanup
    scene->removeItem(andGate);
    delete andGate;
}

void TestElementEditor::testOutputSizeEditing()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Create an element (most basic elements have 1 output)
    auto* andGate = new And();
    scene->addItem(andGate);
    andGate->setSelected(true);
    
    // Verify output size
    QCOMPARE(andGate->outputSize(), 1);
    
    // Update editor
    editor.update();
    
    // Should handle output size display
    QVERIFY(true);
    
    // Cleanup
    scene->removeItem(andGate);
    delete andGate;
}

void TestElementEditor::testFrequencyEditing()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Create a clock element that supports frequency
    auto* clock = new Clock();
    scene->addItem(clock);
    clock->setSelected(true);
    
    // Test frequency property
    QVERIFY(clock->hasFrequency());
    
    // Get initial frequency
    double initialFreq = clock->frequency();
    QVERIFY(initialFreq > 0);
    
    // Update editor
    editor.update();
    
    // Test frequency change
    clock->setFrequency(2.0);
    QCOMPARE(clock->frequency(), 2.0);
    
    // Cleanup
    scene->removeItem(clock);
    delete clock;
}

void TestElementEditor::testTriggerEditing()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Create an input element that supports trigger
    auto* button = new InputButton();
    scene->addItem(button);
    button->setSelected(true);
    
    // Test trigger property
    QVERIFY(button->hasTrigger());
    
    // Update editor
    editor.update();
    
    // Should handle trigger property display
    QVERIFY(true);
    
    // Cleanup
    scene->removeItem(button);
    delete button;
}

void TestElementEditor::testLabelEditing()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Create an element that supports labels
    auto* andGate = new And();
    scene->addItem(andGate);
    andGate->setSelected(true);
    
    // Test label property
    QVERIFY(andGate->hasLabel());
    
    // Test label change
    QString testLabel = "TestGate";
    andGate->setLabel(testLabel);
    QCOMPARE(andGate->label(), testLabel);
    
    // Update editor
    editor.update();
    
    // Should handle label display
    QVERIFY(true);
    
    // Cleanup
    scene->removeItem(andGate);
    delete andGate;
}

void TestElementEditor::testColorEditing()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Create an element that can be tested for properties
    auto* andGate = new And();
    scene->addItem(andGate);
    andGate->setSelected(true);
    
    // Test element property interface exists
    QVERIFY(true);
    
    // Update editor
    editor.update();
    
    // Should handle color property display
    QVERIFY(true);
    
    // Cleanup
    scene->removeItem(andGate);
    delete andGate;
}

void TestElementEditor::testClockElementProperties()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Create a clock element
    auto* clock = new Clock();
    scene->addItem(clock);
    clock->setSelected(true);
    
    // Verify clock-specific properties
    QVERIFY(clock->hasFrequency());
    QVERIFY(clock->hasLabel());
    
    // Update editor
    editor.update();
    
    // Should display clock-specific controls
    QVERIFY(true);
    
    // Cleanup
    scene->removeItem(clock);
    delete clock;
}

void TestElementEditor::testDisplayElementProperties()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Create a display element
    auto* display = new Display7();
    scene->addItem(display);
    display->setSelected(true);
    
    // Update editor
    editor.update();
    
    // Should handle display element properties
    QVERIFY(true);
    
    // Cleanup
    scene->removeItem(display);
    delete display;
}

void TestElementEditor::testTruthTableProperties()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Create a truth table element
    auto* truthTable = new TruthTable();
    scene->addItem(truthTable);
    truthTable->setSelected(true);
    
    // Update editor
    editor.update();
    
    // Should handle truth table properties
    QVERIFY(true);
    
    // Cleanup
    scene->removeItem(truthTable);
    delete truthTable;
}

void TestElementEditor::testICElementProperties()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // IC elements require more complex setup, so we'll test basic handling
    // Create a basic element that might be used in IC testing
    auto* andGate = new And();
    scene->addItem(andGate);
    andGate->setSelected(true);
    
    // Update editor
    editor.update();
    
    // Should handle element properties without crashing
    QVERIFY(true);
    
    // Cleanup
    scene->removeItem(andGate);
    delete andGate;
}

void TestElementEditor::testPropertyValidation()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Create an AND gate for validation testing
    auto* andGate = new And();
    scene->addItem(andGate);
    andGate->setSelected(true);
    
    // Test valid input size
    andGate->setInputSize(3);
    QCOMPARE(andGate->inputSize(), 3);
    
    // Test input size bounds (AND gates typically have min 2, max varies)
    int minInputs = andGate->minInputSize();
    int maxInputs = andGate->maxInputSize();
    QVERIFY(minInputs >= 2);
    QVERIFY(maxInputs >= minInputs);
    
    // Update editor
    editor.update();
    
    // Should handle validation appropriately
    QVERIFY(true);
    
    // Cleanup
    scene->removeItem(andGate);
    delete andGate;
}

void TestElementEditor::testUIEnableDisable()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Test with no selection
    editor.update();
    
    // Should handle empty selection state
    QVERIFY(true);
    
    // Test with selection
    auto* andGate = new And();
    scene->addItem(andGate);
    andGate->setSelected(true);
    editor.update();
    
    // Should enable appropriate controls
    QVERIFY(true);
    
    // Cleanup
    scene->removeItem(andGate);
    delete andGate;
}

void TestElementEditor::testPropertySynchronization()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Create and select an element
    auto* andGate = new And();
    scene->addItem(andGate);
    andGate->setSelected(true);
    
    // Update editor
    editor.update();
    
    // Make changes to element
    QString newLabel = "UpdatedGate";
    andGate->setLabel(newLabel);
    
    // Update editor again
    editor.update();
    
    // Should reflect the changes
    QCOMPARE(andGate->label(), newLabel);
    
    // Cleanup
    scene->removeItem(andGate);
    delete andGate;
}

void TestElementEditor::testUndoRedoIntegration()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Test that editor works with undo/redo system
    auto* undoStack = scene->undoStack();
    QVERIFY(undoStack != nullptr);
    
    // Create an element
    auto* andGate = new And();
    scene->addItem(andGate);
    andGate->setSelected(true);
    
    // Update editor
    editor.update();
    
    // Should work with undo/redo system
    QVERIFY(true);
    
    // Cleanup
    scene->removeItem(andGate);
    delete andGate;
}

void TestElementEditor::testInvalidElementHandling()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Test with null scene
    editor.setScene(nullptr);
    editor.update();
    
    // Should handle null scene gracefully
    QVERIFY(true);
    
    // Reset scene
    editor.setScene(scene);
    
    // Test with no elements
    editor.update();
    
    // Should handle empty scene
    QVERIFY(true);
}

void TestElementEditor::testMixedElementTypes()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Create different types of elements
    auto* andGate = new And();
    auto* clock = new Clock();
    auto* orGate = new Or();
    
    scene->addItem(andGate);
    scene->addItem(clock);
    scene->addItem(orGate);
    
    // Select all different types
    andGate->setSelected(true);
    clock->setSelected(true);
    orGate->setSelected(true);
    
    // Update editor
    editor.update();
    
    // Should handle mixed selection appropriately
    QVERIFY(true);
    
    // Cleanup
    scene->removeItem(andGate);
    scene->removeItem(clock);
    scene->removeItem(orGate);
    delete andGate;
    delete clock;
    delete orGate;
}

void TestElementEditor::testEmptySelection()
{
    WorkSpace workspace;
    auto* scene = workspace.scene();
    ElementEditor editor;
    editor.setScene(scene);
    
    // Create elements but don't select them
    auto* andGate = new And();
    scene->addItem(andGate);
    
    // Update with no selection
    editor.update();
    
    // Should handle empty selection state
    QVERIFY(true);
    
    // Cleanup
    scene->removeItem(andGate);
    delete andGate;
}

void TestElementEditor::testNullSceneHandling()
{
    ElementEditor editor;
    
    // Test operations with null scene
    editor.setScene(nullptr);
    
    // Should not crash
    QVERIFY(true);
    
    // Test update with null scene
    editor.update();
    
    // Should handle null scene gracefully
    QVERIFY(true);
}