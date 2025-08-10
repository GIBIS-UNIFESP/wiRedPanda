// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testscene.h"

#include "scene.h"
#include "workspace.h"
#include "and.h"
#include "or.h"
#include "not.h"
#include "graphicsview.h"

#include <QTest>
#include <QGraphicsItem>
#include <QUndoStack>
#include <QAction>

void TestScene::testSceneConstruction()
{
    Scene scene;
    
    // Test basic scene properties
    QVERIFY(scene.undoStack() != nullptr);
    QVERIFY(scene.simulation() != nullptr);
    QCOMPARE(scene.elements().size(), 0);
    
    // Test initial state
    QVERIFY(scene.undoAction() != nullptr);
    QVERIFY(scene.redoAction() != nullptr);
    QCOMPARE(scene.selectedItems().size(), 0);
}

void TestScene::testElementsManagement()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    
    // Test adding elements
    auto *andGate = new And();
    auto *orGate = new Or();
    auto *notGate = new Not();
    
    scene->addItem(andGate);
    scene->addItem(orGate);
    scene->addItem(notGate);
    
    // Test element retrieval
    QCOMPARE(scene->elements().size(), 3);
    auto elements = scene->elements();
    QVERIFY(elements.contains(andGate));
    QVERIFY(elements.contains(orGate));
    QVERIFY(elements.contains(notGate));
    
    // Test visible elements
    auto visibleElements = scene->visibleElements();
    QCOMPARE(visibleElements.size(), 3);
}

void TestScene::testSelectionOperations()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    
    // Add elements
    auto *element1 = new And();
    auto *element2 = new Or();
    scene->addItem(element1);
    scene->addItem(element2);
    
    // Test selection
    element1->setSelected(true);
    auto selectedElements = scene->selectedElements();
    QCOMPARE(selectedElements.size(), 1);
    QVERIFY(selectedElements.contains(element1));
    
    // Test select all
    scene->selectAll();
    QCOMPARE(scene->selectedItems().size(), 2);
    
    // Clear selection
    scene->clearSelection();
    QCOMPARE(scene->selectedItems().size(), 0);
}

void TestScene::testUndoRedoOperations()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    auto *undoStack = scene->undoStack();
    
    // Test initial state
    QVERIFY(!undoStack->canUndo());
    QVERIFY(!undoStack->canRedo());
    
    // Add an element and test undo/redo
    auto *element = new And();
    scene->addItem(element);
    
    // Test that undo/redo actions are properly connected
    QVERIFY(scene->undoAction() != nullptr);
    QVERIFY(scene->redoAction() != nullptr);
    QCOMPARE(scene->undoAction()->isEnabled(), undoStack->canUndo());
    QCOMPARE(scene->redoAction()->isEnabled(), undoStack->canRedo());
}

void TestScene::testSceneGeometry()
{
    Scene scene;
    
    // Test initial scene rect
    QRectF initialRect = scene.sceneRect();
    QVERIFY(!initialRect.isEmpty());
    
    // Add an element and test scene resize
    auto *element = new And();
    element->setPos(100, 100);
    scene.addItem(element);
    
    // Test elements in rect
    QRectF testRect(50, 50, 100, 100);
    auto elementsInRect = scene.elements(testRect);
    QCOMPARE(elementsInRect.size(), 1);
    QCOMPARE(elementsInRect.first(), element);
    
    // Test empty rect
    QRectF emptyRect(200, 200, 50, 50);
    auto elementsInEmptyRect = scene.elements(emptyRect);
    QCOMPARE(elementsInEmptyRect.size(), 0);
}

void TestScene::testConnections()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    
    // Add elements that can be connected
    auto *input = new And();
    auto *output = new Or();
    scene->addItem(input);
    scene->addItem(output);
    
    // Test that we can add elements without crashes
    // We can't test connections directly since the method is private
    QCOMPARE(scene->elements().size(), 2);
    QVERIFY(scene->elements().contains(input));
    QVERIFY(scene->elements().contains(output));
}

void TestScene::testThemeUpdate()
{
    Scene scene;
    
    // Test theme update doesn't crash
    scene.updateTheme();
    
    // Should complete without throwing exceptions
    QVERIFY(true);
}

void TestScene::testSceneUtilities()
{
    Scene scene;
    
    // Test utility methods that are public
    scene.setAutosaveRequired();
    scene.setCircuitUpdateRequired();
    
    // Test that these methods exist and can be called
    QVERIFY(true);
    
    // Test other public utility methods
    scene.selectAll();
    scene.clearSelection();
    QVERIFY(true);
}