// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testcommands.h"

#include "and.h"
#include "or.h"
#include "not.h"
#include "commands.h"
#include "scene.h"
#include "workspace.h"

#include <QTest>

void TestCommands::testAddDeleteCommands()
{
    QList<QGraphicsItem *> items{new And(), new And(), new And(), new And()};

    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();
    undoStack->setUndoLimit(1);
    scene->receiveCommand(new AddItemsCommand(items, scene));

    QCOMPARE(scene->elements().size(), items.size());

    undoStack->undo();
    undoStack->redo();
    undoStack->undo();
    undoStack->redo();
    undoStack->undo();
    undoStack->redo();
    scene->receiveCommand(new DeleteItemsCommand(scene->items(), scene));
    undoStack->undo();
    undoStack->redo();
    undoStack->undo();
    undoStack->redo();
    undoStack->undo();
    undoStack->redo();

    QCOMPARE(scene->elements().size(), 0);
    QCOMPARE(undoStack->index(), 1);
}

void TestCommands::testAddItemsCommand()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();
    
    // Test adding single item through scene command system
    QList<QGraphicsItem *> singleItem{new And()};
    
    QCOMPARE(scene->elements().size(), 0);
    scene->receiveCommand(new AddItemsCommand(singleItem, scene));
    QCOMPARE(scene->elements().size(), 1);
    
    // Test undo
    undoStack->undo();
    QCOMPARE(scene->elements().size(), 0);
    
    // Test redo
    undoStack->redo();
    QCOMPARE(scene->elements().size(), 1);
}

void TestCommands::testDeleteItemsCommand()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    
    // Add items first
    QList<QGraphicsItem *> items{new Or(), new Not()};
    scene->receiveCommand(new AddItemsCommand(items, scene));
    QCOMPARE(scene->elements().size(), 2);
    
    // Test deleting items
    DeleteItemsCommand deleteCmd(scene->items(), scene);
    deleteCmd.redo();
    QCOMPARE(scene->elements().size(), 0);
    deleteCmd.undo();
    QCOMPARE(scene->elements().size(), 2);
}

void TestCommands::testUndoRedoChain()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    auto *undoStack = scene->undoStack();
    
    // Test complex undo/redo sequence
    scene->receiveCommand(new AddItemsCommand({new And()}, scene));
    QCOMPARE(scene->elements().size(), 1);
    QCOMPARE(undoStack->canUndo(), true);
    
    scene->receiveCommand(new AddItemsCommand({new Or()}, scene));
    QCOMPARE(scene->elements().size(), 2);
    
    // Undo last command
    undoStack->undo();
    QCOMPARE(scene->elements().size(), 1);
    QCOMPARE(undoStack->canRedo(), true);
    
    // Redo
    undoStack->redo();
    QCOMPARE(scene->elements().size(), 2);
    
    // Undo all
    undoStack->undo();
    undoStack->undo();
    QCOMPARE(scene->elements().size(), 0);
    QCOMPARE(undoStack->canUndo(), false);
}

void TestCommands::testCommandUtilityFunctions()
{
    WorkSpace workspace;
    auto *scene = workspace.scene();
    
    // Add some elements to test utility functions
    auto *andGate = new And();
    auto *orGate = new Or();
    scene->receiveCommand(new AddItemsCommand({andGate, orGate}, scene));
    
    // Test element finding functions exist and work with valid elements
    auto elements = scene->elements();
    QVERIFY(elements.size() >= 2);
    
    // Test that elements are properly managed
    for (auto *element : elements) {
        QVERIFY(element != nullptr);
        QVERIFY(element->id() >= 0);
    }
}
