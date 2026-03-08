// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestCommands : public QObject
{
    Q_OBJECT

private slots:
    // Basic add/delete operations
    void testAddItemsCommand();
    void testDeleteItemsCommand();
    void testAddDeleteUndoRedo();

    // Transform commands
    void testRotateCommand();
    void testMoveCommand();
    void testFlipCommand();

    // Element modification commands
    void testChangeInputSizeCommand();
    void testChangeOutputSizeCommand();
    void testMorphCommand();
    void testUpdateCommand();

    // Connection commands
    void testConnectionPreservation();
    void testSplitCommand();

    // Edge cases
    void testUndoEmptyStack();
    void testRedoEmptyStack();
    void testUndoLimit();
};
