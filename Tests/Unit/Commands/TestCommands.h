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
    void testChangeOutputSizeCommand();
    void testMorphCommand();
    void testMorphPreservesFlip();
    void testMorphPreservesVolume();
    void testMorphCommandUndoPreservesConnectionAddedWhileMorphed();
    void testUpdateCommand();
    void testUpdateCommandPreservesClockPhase();
    void testUpdateCommandWirelessLabelRewiresChannel();
    void testUpdateCommandNonWirelessNodeEditKeepsFastPath();

    // Connection commands
    void testConnectionPreservation();
    void testSplitCommand();

    // Edge cases
    void testUndoEmptyStack();
    void testRedoEmptyStack();
    void testUndoLimit();

    // Regression: D13 — SplitCommand::redo must throw before allocating
    void testSplitCommandRedoThrowsBeforeAllocation();

    // Regression: F17 — toggle position must be bounds-checked (OOB write)
    void testToggleTruthTableOutputCommandBounds();

    // MorphCommand's remaining property-transfer/connection-drop branches
    void testMorphNotToNodeAdjustsPosition();
    void testMorphNodeToNotAdjustsPosition();
    void testMorphPreservesFrequency();
    void testMorphDropsAndRestoresOutputConnection();
    void testMorphUndoSkipsRestoringConnectionToDeletedPeer();

    // Empty-target no-ops
    void testUpdateCommandWithNoElementsIsNoOp();
    void testUpdateBlobCommandWithNoElementsIsNoOp();
    void testFlipCommandWithEmptyItemsIsNoOp();

    // SplitCommand's remaining upstream-lookup-fails throws
    void testSplitCommandRedoThrowsWhenEndPortIsNull();
    void testSplitCommandUndoThrowsWhenUpstreamMissing();

    // CommandUtils::findItems()/findElements() throw when scene state is corrupted
    // independently of the undo stack (mirrors testSplitCommandRedoThrowsBeforeAllocation's
    // forgetItemId technique).
    void testFindItemsThrowsWhenItemMissing();
    void testLoadItemsThrowsOnCountMismatch();
    void testElementsCommandElementsThrowsWhenItemMissing();
};
