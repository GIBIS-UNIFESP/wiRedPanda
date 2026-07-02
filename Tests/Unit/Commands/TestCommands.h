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
    void testUpdateCommandAppliesPropagationDelayLive();
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
};
