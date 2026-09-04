// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>
#include <QTest>

/// Covers QuickElementEditor (App/QuickShell/Chrome/QuickElementEditor), the property-panel
/// presenter, mirroring the corresponding slice of Tests/Integration/TestMainWindowGui.cpp's
/// "ElementEditor operations" sections, plus this class's own right-click-context-menu-backing
/// surface: prepareContextMenu()/morphCandidates()/morphSelectionTo() are where Rename/Change
/// trigger/Change color/Morph-to in Main.qml's elementContextMenu resolve, not CanvasItem itself.
///
/// Only `Clock` has `ElementConstraints::hasDelay == true`; `QuickElementEditor::setDelaySteps()`
/// guards on `m_caps.hasDelay` and no-ops otherwise, so this class's delay test exercises Clock
/// rather than Node.
///
/// Not covered: Audio/Volume/AudioBox property setters (thin pass-throughs, same shape as
/// Color/Label) and the appearance tile-grid multi-state path (needs a multi-state element,
/// e.g. Led's color states or Display7's segment states; this pass only exercises the
/// single-state flag path).
class TestQuickElementEditor : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testSetCanvasBindsSelectionAndRefreshesOnChange();
    void testEmptySelectionClearsFields();

    void testLabelSetAppliesUndoably();
    void testDivergentLabelShowsPlaceholderUntilEdited();
    void testColorSetAppliesUndoablyAndColorOptionsNonEmpty();
    void testTriggerSetForcesUppercaseAndAppliesUndoably();
    void testTriggerRejectsMultiCharacter();
    void testDivergentTriggerShowsPlaceholderUntilEdited();
    void testDelaySetAppliesUndoably();
    void testInputSizeChangeIsUndoable();
    void testOutputSizeChangeIsUndoable();
    void testLockedStateChangeIsUndoable();
    void testOutputValueChangeIsUndoable();

    void testWirelessModeChangeSeversStaleConnection();
    void testWirelessModeRejectsDuplicateTxLabel();
    void testWirelessModeAllowsUniqueTxLabelRename();
    void testWirelessModeAllowsRxNodeToShareLabelWithTx();

    void testPrepareContextMenuPopulatesGateMorphCandidates();
    void testPrepareContextMenuExcludesSelectionsOwnType();
    void testMorphSelectionToAppliesToWholeSelection();

    // --- Tab/Shift+Tab element cycling (mirrors master's TestElementTabNavigator) ---
    void testReadingOrderIsRowMajor();
    void testReadingOrderTieBreaksLeftToRight();
    void testReadingOrderEmptyAndSingle();
    void testCycleSelectionTabAdvancesToNextElement();
    void testCycleSelectionBacktabAdvancesToPreviousElement();
    void testCycleSelectionWrapsAround();
    void testCycleSelectionIgnoresNonSingleSelection();
    void testCycleSelectionRevertsWhenNoOtherElementHasField();

    void testChangeAppearancePicksFileAndAppliesUndoably();
    void testChangeAppearanceCancelledDialogIsNoop();
    void testResetAppearanceRestoresDefault();

    void testBlobRenameAppliesUndoably();
    void testBlobRenameRejectsDuplicateName();

    void testOpenTruthTableBuildsRowsAndEmitsSignal();
    void testToggleTruthTableCellPushesUndoableCommand();

private:
    QTemporaryDir m_tempDir;
    QString m_fixtureDir;
};
