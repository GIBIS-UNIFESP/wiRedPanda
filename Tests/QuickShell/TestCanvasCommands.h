// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>
#include <QTest>

/// Behavioral coverage for App/QuickShell/Canvas/CanvasCommands.h's undo/redo commands.
/// CanvasElementsCommand resolves its elements by id rather than by raw pointer.
///
/// Not covered here, tracked as a real follow-up rather than a silent gap:
/// CanvasRegisterBlobCommand/CanvasRemoveBlobCommand/CanvasRenameBlobCommand/
/// CanvasUpdateBlobCommand (embedded-IC blob bookkeeping) and
/// CanvasToggleTruthTableOutputCommand (trivial single-bit toggle, low risk).
class TestCanvasCommands : public QObject
{
    Q_OBJECT

private slots:
    void testAddItemsCommandWithConnectionsUndoRedo();
    void testDeleteSelectedRemovesConnectedWireUndoRestoresIt();

    void testMoveCommandUndoRedo();
    void testMoveMultipleElementsTogetherUndoRedo();

    void testRotateRightAccumulatesUndoUnwindsOneStep();
    void testFlipHorizontalTwiceIsIdentity();
    void testFlipVerticalUndoRedo();

    void testUpdateCommandLabelUndoRedo();

    void testChangePortSizeIncreaseInputSize();
    void testChangePortSizeDecreaseRemovesConnectionRestoredOnUndo();

    void testMorphPreservesConnections();
    void testMorphToFewerPortsDropsConnectionRestoredOnUndo();

    void testSplitCommandCreatesNodeUndoRestoresSingleConnection();

    void testUndoOnEmptyStackIsNoop();
    void testRedoOnEmptyStackIsNoop();
    void testNewOperationClearsRedoStack();

    // CanvasItem::alignLeft()/alignRight()/alignTop()/alignBottom()/alignHorizontalCenter()/
    // alignVerticalCenter()/distributeHorizontally()/distributeVertically() reuse
    // moveElementsTo()/CanvasMoveCommand rather than a dedicated command class.
    void testAlignLeftAndTopMatchEdges();
    void testAlignRightUsesSceneEdgeNotBarePos();
    void testAlignBottomUsesSceneEdgeNotBarePos();
    void testAlignHorizontalCenterUsesSceneEdge();
    void testAlignVerticalCenterUsesSceneEdge();
    void testAlignNoopBelowTwoElements();
    void testDistributeHorizontallyEqualizesGaps();
    void testDistributeVerticallyEqualizesGaps();
    void testDistributeNoopBelowThreeElements();
    void testAlignDistributeUndoRestoresPositions();

    void testLiveThemeSwitchRefreshesElementsAndConnections();

private:
    QTemporaryDir m_tempDir;
};
