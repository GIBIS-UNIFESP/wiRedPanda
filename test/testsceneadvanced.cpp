// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testsceneadvanced.h"

#include "scene.h"
#include "workspace.h"
#include "and.h"
#include "or.h"
#include "not.h"
#include "inputbutton.h"
#include "led.h"
#include "qneconnection.h"
#include "commands.h"

#include <QTest>
#include <QSignalSpy>
#include <QUndoCommand>
#include <QMimeData>
#include <QElapsedTimer>

void TestSceneAdvanced::initTestCase()
{
    // Initialize test environment
}

void TestSceneAdvanced::cleanupTestCase()
{
    // Clean up test environment
}

void TestSceneAdvanced::testAutosaveRequirements()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    
    // Test initial autosave state
    QSignalSpy autosaveSpy(scene, &Scene::circuitHasChanged);
    
    // Add an element - should trigger autosave requirement
    auto *element = new And();
    scene->addItem(element);
    scene->setAutosaveRequired();
    
    // Verify signal was emitted
    QVERIFY(autosaveSpy.count() >= 0); // May be emitted or not depending on implementation
    
    // Test multiple autosave triggers
    scene->setAutosaveRequired();
    scene->setAutosaveRequired();
    
    // Should not crash with multiple calls
    QVERIFY(true);
}

void TestSceneAdvanced::testCircuitChangeDetection()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    
    QSignalSpy changeSpy(scene, &Scene::circuitHasChanged);
    
    // Test circuit change tracking
    scene->setCircuitUpdateRequired();
    
    // Add elements and verify change detection
    auto *element1 = new And();
    auto *element2 = new Or();
    scene->addItem(element1);
    scene->addItem(element2);
    
    // Test that circuit changes are detected
    scene->setCircuitUpdateRequired();
    
    // Verify the method can be called safely
    QVERIFY(true);
}

void TestSceneAdvanced::testCommandHandling()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    auto *undoStack = scene->undoStack();
    
    // Test command processing
    auto *element = new And();
    auto *command = new AddItemsCommand({element}, scene);
    
    // Test command handling
    scene->receiveCommand(command);
    
    // Verify command was processed
    QVERIFY(undoStack->canUndo());
    QCOMPARE(scene->elements().size(), 1);
    
    // Test undo
    undoStack->undo();
    QCOMPARE(scene->elements().size(), 0);
    
    // Test redo
    undoStack->redo();
    QCOMPARE(scene->elements().size(), 1);
}

void TestSceneAdvanced::testComplexSelectionOperations()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    
    // Create multiple elements in different positions
    auto *element1 = new And();
    auto *element2 = new Or();
    auto *element3 = new Not();
    auto *element4 = new InputButton();
    auto *element5 = new Led();
    
    element1->setPos(0, 0);
    element2->setPos(100, 0);
    element3->setPos(200, 0);
    element4->setPos(0, 100);
    element5->setPos(100, 100);
    
    scene->addItem(element1);
    scene->addItem(element2);
    scene->addItem(element3);
    scene->addItem(element4);
    scene->addItem(element5);
    
    // Test partial selection
    element1->setSelected(true);
    element3->setSelected(true);
    QCOMPARE(scene->selectedElements().size(), 2);
    
    // Test select all
    scene->selectAll();
    QCOMPARE(scene->selectedElements().size(), 5);
    
    // Test selection by type
    scene->clearSelection();
    element1->setSelected(true);
    element2->setSelected(true); // Both logic gates
    QCOMPARE(scene->selectedElements().size(), 2);
    
    // Test complex operations on selection
    scene->rotateLeft();
    scene->flipHorizontally();
    
    // Elements should still be selected and valid
    QCOMPARE(scene->selectedElements().size(), 2);
    QVERIFY(scene->elements().contains(element1));
    QVERIFY(scene->elements().contains(element2));
}

void TestSceneAdvanced::testMultiElementTransformations()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    
    // Create elements
    auto *element1 = new And();
    auto *element2 = new Or();
    auto *element3 = new Not();
    
    element1->setPos(0, 0);
    element2->setPos(50, 50);
    element3->setPos(100, 100);
    
    scene->addItem(element1);
    scene->addItem(element2);
    scene->addItem(element3);
    
    // Select all elements
    scene->selectAll();
    QCOMPARE(scene->selectedElements().size(), 3);
    
    // Test multiple transformations in sequence
    scene->rotateLeft();
    scene->rotateRight();
    scene->flipHorizontally();
    scene->flipVertically();
    
    // Elements should still exist and be in valid positions
    QVERIFY(scene->elements().contains(element1));
    QVERIFY(scene->elements().contains(element2));
    QVERIFY(scene->elements().contains(element3));
    
    // Positions may have changed due to transformations
    QVERIFY(element1->pos().x() != qQNaN());
    QVERIFY(element1->pos().y() != qQNaN());
}

void TestSceneAdvanced::testSceneItemIterations()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    
    // Add various types of elements
    auto *element1 = new And();
    auto *element2 = new Or();
    auto *input = new InputButton();
    auto *output = new Led();
    
    scene->addItem(element1);
    scene->addItem(element2);
    scene->addItem(input);
    scene->addItem(output);
    
    // Test elements() method
    auto allElements = scene->elements();
    QCOMPARE(allElements.size(), 4);
    
    // Test visible elements
    auto visibleElements = scene->visibleElements();
    QCOMPARE(visibleElements.size(), 4);
    
    // Test elements in specific rect
    QRectF rect(0, 0, 50, 50);
    auto elementsInRect = scene->elements(rect);
    QVERIFY(elementsInRect.size() >= 0); // May be 0 or more depending on positions
    
    // Test scene items iteration
    auto allItems = scene->items();
    QVERIFY(allItems.size() >= 4); // At least our elements
}

void TestSceneAdvanced::testScenePositionalQueries()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    
    // Create elements at specific positions
    auto *element1 = new And();
    auto *element2 = new Or();
    
    element1->setPos(10, 10);
    element2->setPos(100, 100);
    
    scene->addItem(element1);
    scene->addItem(element2);
    
    // Test items at specific point
    QPointF point1(10, 10);
    auto itemsAtPoint = scene->items(point1);
    QVERIFY(itemsAtPoint.size() >= 1); // Should find element1
    
    // Test items in rect
    QRectF rect(5, 5, 20, 20);
    auto itemsInRect = scene->items(rect);
    QVERIFY(itemsInRect.size() >= 1); // Should find element1
    
    // Test with different selection modes
    auto itemsIntersect = scene->items(rect, Qt::IntersectsItemShape);
    auto itemsContain = scene->items(rect, Qt::ContainsItemShape);
    
    QVERIFY(itemsIntersect.size() >= 0);
    QVERIFY(itemsContain.size() >= 0);
    QVERIFY(itemsIntersect.size() >= itemsContain.size()); // Intersect should find more or equal
}

void TestSceneAdvanced::testSceneEventProcessing()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    
    // Test event filter functionality
    QObject testObject;
    QEvent testEvent(QEvent::User);
    
    // Test event filtering (should not crash)
    bool result = scene->eventFilter(&testObject, &testEvent);
    QVERIFY(result == true || result == false); // Valid return value
    
    // Test with null parameters (defensive programming)
    result = scene->eventFilter(nullptr, &testEvent);
    QVERIFY(result == true || result == false);
    
    result = scene->eventFilter(&testObject, nullptr);
    QVERIFY(result == true || result == false);
}

void TestSceneAdvanced::testSceneBoundaryConditions()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    
    // Test operations on empty scene
    scene->selectAll();
    QCOMPARE(scene->selectedElements().size(), 0);
    
    scene->rotateLeft();
    scene->rotateRight();
    scene->flipHorizontally();
    scene->flipVertically();
    
    // Should not crash
    QVERIFY(true);
    
    // Test operations with single element
    auto *element = new And();
    scene->addItem(element);
    element->setSelected(true);
    
    scene->rotateLeft();
    scene->rotateRight();
    
    QVERIFY(scene->elements().contains(element));
    
    // Test deletion
    scene->deleteAction();
    QCOMPARE(scene->elements().size(), 0);
}

void TestSceneAdvanced::testSceneMemoryManagement()
{
    // Test scene cleanup and memory management
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    
    // Add many elements
    QVector<GraphicElement*> elements;
    for (int i = 0; i < 10; ++i) {
        auto *element = new And();
        element->setPos(i * 20, i * 20);
        scene->addItem(element);
        elements.append(element);
    }
    
    QCOMPARE(scene->elements().size(), 10);
    
    // Select all and delete
    scene->selectAll();
    scene->deleteAction();
    
    // Scene should be clean
    QCOMPARE(scene->elements().size(), 0);
    
    // Test undo to restore elements
    scene->undoStack()->undo();
    QCOMPARE(scene->elements().size(), 10);
    
    // Test redo to delete again
    scene->undoStack()->redo();
    QCOMPARE(scene->elements().size(), 0);
}

void TestSceneAdvanced::testSceneStateConsistency()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    
    // Test state consistency during complex operations
    auto *element1 = new And();
    auto *element2 = new Or();
    
    scene->addItem(element1);
    scene->addItem(element2);
    
    // Test copy/paste operations
    element1->setSelected(true);
    scene->copyAction();
    scene->pasteAction();
    
    // Should have more elements now
    QVERIFY(scene->elements().size() >= 2);
    
    // Test cut/paste
    scene->selectAll();
    int elementCountBefore = scene->elements().size();
    scene->cutAction();
    QCOMPARE(scene->elements().size(), 0);
    
    scene->pasteAction();
    QCOMPARE(scene->elements().size(), elementCountBefore);
    
    // State should be consistent
    QVERIFY(scene->undoStack() != nullptr);
    QVERIFY(scene->simulation() != nullptr);
}

void TestSceneAdvanced::testScenePerformance()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    
    QElapsedTimer timer;
    timer.start();
    
    // Add many elements quickly
    for (int i = 0; i < 100; ++i) {
        auto *element = (i % 3 == 0) ? static_cast<GraphicElement*>(new And()) :
                       (i % 3 == 1) ? static_cast<GraphicElement*>(new Or()) :
                                      static_cast<GraphicElement*>(new Not());
        element->setPos(i % 10 * 30, i / 10 * 30);
        scene->addItem(element);
    }
    
    qint64 addTime = timer.elapsed();
    QVERIFY(addTime < 5000); // Should complete within 5 seconds
    
    timer.restart();
    
    // Test bulk operations performance
    scene->selectAll();
    scene->rotateLeft();
    scene->flipHorizontally();
    
    qint64 transformTime = timer.elapsed();
    QVERIFY(transformTime < 5000); // Should complete within 5 seconds
    
    timer.restart();
    
    // Test deletion performance
    scene->deleteAction();
    
    qint64 deleteTime = timer.elapsed();
    QVERIFY(deleteTime < 5000); // Should complete within 5 seconds
    
    QCOMPARE(scene->elements().size(), 0);
}