// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

/// Phase 7b of the qtquick-rewrite plan: real behavioral coverage for
/// App/QuickShell/Canvas/CanvasCommands.h, which claims 1:1 parity with App/Scene/Commands.h
/// but had zero permanent test of its own until now (every check this rewrite produced ran
/// through a temporary, reverted Main.cpp diagnostic hook -- see the plan's Phase 7 section
/// and project memory project_phase7_audit_zero_quick_test_coverage.md). Mirrors the intent of
/// Tests/Unit/Commands/TestCommands.cpp and Tests/Unit/Scene/TestSceneUndoredo.cpp against the
/// real CanvasItem/Canvas*Command API instead of Scene/Commands.h -- not a line-by-line port,
/// since the two APIs don't correspond 1:1 (id-based CanvasElementsCommand resolution vs.
/// Scene's QGraphicsScene-item-pointer resolution).
///
/// Deliberately out of scope for this pass, tracked as a real follow-up rather than a silent
/// gap: CanvasRegisterBlobCommand/CanvasRemoveBlobCommand/CanvasRenameBlobCommand/
/// CanvasUpdateBlobCommand (embedded-IC blob bookkeeping -- CanvasICRegistry's own Phase 3
/// verification already exercised the underlying registry logic once) and
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
};
