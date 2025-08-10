// Copyright 2015 - 2025, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestCommands : public QObject
{
    Q_OBJECT

private slots:
    void testAddDeleteCommands();
    void testAddItemsCommand();
    void testDeleteItemsCommand();
    void testUndoRedoChain();
    void testCommandUtilityFunctions();
    
    // Test utility functions
    void testStoreIds();
    void testStoreOtherIds();
    void testLoadList();
    void testFindElm();
    void testFindConn();
    void testFindElements();
    void testFindItems();
    void testAddDeleteItems();
    void testSaveLoadItems();
    
    // Test all command types
    void testRotateCommand();
    void testMoveCommand();
    void testUpdateCommand();
    void testSplitCommand();
    void testMorphCommand();
    void testChangeInputSizeCommand();
    void testFlipCommand();
    void testChangeOutputSizeCommand();
    void testToggleTruthTableOutputCommand();
    
    // Integration tests
    void testCommandChaining();
    void testComplexCommandSequences();
};
