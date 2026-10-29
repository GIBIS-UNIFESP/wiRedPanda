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
    void testFlipTextPixmapVariant();
    void testRotateTextPixmapVariant();
    void testFlipRotateTextPixmapVariant();
    void testFlipNonRotatablePorts();
    void testRotateNonRotatablePorts();
    void testFlipRotateNonRotatablePort();

    // Element modification commands
    void testChangeInputSizeCommand();
    void testChangeInputSizeCommandShrinkDropsAndRestoresConnection();
    void testChangeOutputSizeCommand();
    void testMorphCommand();
    void testMorphPreservesFlip();
    void testMorphPreservesVolume();
    void testMorphCommandUndoPreservesConnectionAddedWhileMorphed();
    void testUpdateCommand();
    void testUpdateCommandPropertyEditDoesNotRebuild();
    void testUpdateCommandWirelessLabelRewiresChannel();
    void testUpdateCommandKeepsAWiredPresetOnItsWire();
    void testUpdateCommandNonWirelessNodeEditKeepsFastPath();

    // Connection commands
    void testConnectionPreservation();
    void testSplitCommand();

    // Edge cases
    void testUndoEmptyStack();
    void testRedoEmptyStack();
    void testUndoLimit();

    // Regression: D13 — SplitCommand::redo must throw before allocating
    void testSplitCommandRedoAbandonedWhenUpstreamMissing();

    // Regression: F17 — toggle position must be bounds-checked (OOB write)
    void testToggleTruthTableOutputCommandBounds();

    // MorphCommand's remaining property-transfer/connection-drop branches
    void testMorphNotToNodeAdjustsPosition();
    void testMorphNodeToNotAdjustsPosition();
    void testMorphPreservesFrequency();
    void testMorphDropsAndRestoresOutputConnection();
    void testMorphUndoSkipsRestoringConnectionToDeletedPeer();
    // Undo restores the elements a morph replaced as they were
    void testMorphUndoRestoresWhatTheNewTypeLacks();
    void testMorphUndoRestoresInputsTheNewTypeCannotHave();
    void testMorphUndoWithAPictureThatIsGoneShowsTheDefault();
    void testUndoWithAPictureThatIsGoneShowsTheDefault();
    void testUndoWithASoundThatIsGoneKeepsItsPath();
    void testUndoWithAnICFileThatIsGoneLoadsTheContentItHad();

    // SetPropertyCommand: one property of elements, its values before and after, merged within an edit session
    void testSetPropertyCommandUndoesEachElementsValue();
    void testSetPropertyCommandNamesEveryElementThatHasTheProperty();
    void testSetPropertyCommandSetsAnInputsLockAndValue();
    void testSetPropertyCommandFirstRedoLeavesTheValues();
    void testSetPropertyCommandMergesASession();
    void testSetPropertyCommandMergedBackIsNoStep();
    void testSetPropertyCommandMergesOnlyTheSameChanges();
    void testSetPropertyCommandRxLabelRewiresChannel();
    void testSetPropertyCommandNonWirelessLabelDoesNotRebuild();

    // Empty-target no-ops
    void testUpdateCommandWithNoElementsIsNoOp();
    void testUpdateBlobCommandWithNoElementsIsNoOp();
    void testFlipCommandWithEmptyItemsIsNoOp();

    // SplitCommand's remaining upstream-lookup-fails throws
    void testSplitCommandRedoAbandonedWhenEndPortIsNull();
    void testSplitCommandUndoAbandonedWhenNodeMissing();

    // An undo restores an element off the grid exactly, and setPos() stores what it is given.
    void testOffGridElementRestoresExactly_data();
    void testOffGridElementRestoresExactly();
    void testSetPosIsExact();

    // A command whose record no longer matches the scene changes nothing and the stack drops it, the scene corrupted
    // behind the stack's back (mirrors testSplitCommandRedoAbandonedWhenUpstreamMissing's
    // forgetItemId technique).
    void testAddItemsUndoAbandonedWhenItemMissing();
    void testLoadItemsRefusesACountMismatch();
    void testUpdateRedoAbandonedWhenElementMissing();
    void testLoadListGrowsLinearly();
};
