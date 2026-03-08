// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>

class TestSceneUndoredo : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    // Undo/Redo & Scene State Tests
    void testReceiveCommand();
    void testPasteUndoRedo();
    void testCutUndoRedo();
    void testMultiCommandSequence();

    // DeleteItemsCommand
    void testDeleteElementAndUndo();
    void testDeleteWithConnectionsRestored();

    // RotateCommand
    void testRotateSingleElement();
    void testRotateUndoRedo();
    void testRotateAccumulates();

    // MoveCommand
    void testMoveSingleElement();
    void testMoveMultipleElements();

    // FlipCommand
    void testFlipHorizontalUndoRedo();
    void testFlipVerticalUndoRedo();

    // MorphCommand
    void testMorphTypeChange();
    void testMorphPreservesLabel();
    void testMorphPreservesConnections();

    // ChangeInputSizeCommand
    void testIncreaseInputSize();
    void testDecreaseInputSizeRemovesConnection();

    // ChangeInputSizeCommand edge cases
    void testDecreaseInputSizeMultipleConnections();

    // ChangeOutputSizeCommand
    void testChangeOutputSizeUndoRedo();
    void testChangeOutputSizeMultipleElements();

    // SplitCommand
    void testSplitConnectionCreatesNode();

    // UpdateCommand
    void testUpdateCommandUndoRedo();

    // Edge cases
    void testNewOperationClearsRedoStack();
    void testUndoBeyondBeginning();
    void testRedoBeyondEnd();

    // Additional FlipCommand edge cases
    void testFlipRotationChanges();           // rotation += 180 regardless of axis (documents FIXME bug)
    void testFlipNonRotatableElement();        // Led/InputSwitch: position flips, rotation stays 0
    void testFlipSingleElementPositionUnchanged(); // min==max → position unchanged, rotation changes

    // Additional RotateCommand edge cases
    void testRotateMultipleElementsPositionsChange(); // collective center pivot

    // Additional DeleteItemsCommand edge cases
    void testDeleteMultipleElements();         // delete 3 elements simultaneously

    // AddItemsCommand (direct, not via paste)
    void testAddItemsCommandDirect();          // items added in constructor; first redo() is no-op

    // Additional MorphCommand edge cases
    void testMorphMultipleElements();          // morph AND+OR→Xor simultaneously

    // QUndoStack clean state (save-dirty tracking)
    void testUndoStackCleanState();

private:
    QTemporaryDir m_tempDir;
};
