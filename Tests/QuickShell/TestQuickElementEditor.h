// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTemporaryDir>
#include <QTest>

/// Phase 7e (part 5) of the qtquick-rewrite plan: real coverage for QuickElementEditor
/// (App/QuickShell/Chrome/QuickElementEditor), the property-panel presenter that had zero
/// permanent tests anywhere despite being fully ported since Phase 4 sub-step 5 (including its
/// later "named deferrals" follow-up, which added Appearance/AudioBox/TruthTable/blob-rename --
/// see project_quickelementeditor_scope.md, now stale on that point). Mirrors the corresponding
/// slice of Tests/Integration/TestMainWindowGui.cpp's "ElementEditor operations" sections, plus
/// this class's own right-click-context-menu-backing surface (prepareContextMenu()/
/// morphCandidates()/morphSelectionTo() -- Rename/Change trigger/Change color/Morph-to in
/// Main.qml's elementContextMenu all resolve here, not in CanvasItem itself), closing most of
/// the plan's "context menu operations" gap without needing to drive real QML Menu items.
///
/// One deliberate deviation from the Widgets original, not a blind port: TestMainWindowGui::
/// testChangeDelay() exercises the delay slider against a `Node` -- but only `Clock` actually
/// has ElementConstraints::hasDelay == true (confirmed by reading every GraphicElements/*.cpp
/// constraints table); Node's slider is merely visible-when-hidden in the Widgets UI (its
/// QSlider::valueChanged is wired unconditionally to apply(), and that original test only
/// asserts the *slider's* value, never node->delay()). QuickElementEditor::setDelaySteps()
/// explicitly guards on m_caps.hasDelay and no-ops otherwise, so this class's own test uses
/// Clock -- a real, meaningful delay change, not the Widgets test's degenerate case.
///
/// Not covered here (real, scoped-out follow-ups): Audio/Volume/AudioBox property setters
/// (thin, low-risk pass-throughs, same shape as Color/Label already covered) and the
/// appearance tile-grid multi-state path (needs a real multi-state element, e.g. Led's color
/// states or Display7's segment states -- this pass only exercises the single-state flag path).
///
/// Phase 7f follow-up (Tests/Unit/Ui/TestDialogs.cpp's disposition): its 5
/// testElementEditorRejectsDuplicateTxLabel-family tests are this same class's wireless-Tx
/// duplicate-label guard, seen from a different angle than testWirelessModeRejectsDuplicateTxLabel
/// above -- two of its five scenarios are genuinely new coverage (a same-node label rename that
/// must NOT collide with itself; an Rx node sharing a Tx's label, which the guard must allow
/// since it only fires for Tx candidates), added below. ClockDialog/LengthDialog's own 11 tests
/// (min/max/range/constructor-default checks on a QSpinBox/QSlider) have NO C++ equivalent to
/// port at all, confirmed by reading QuickDolphinController.cpp: RangeDialog.qml's min/max
/// range is a plain QML property passed in at each call site, with zero backing C++
/// clamp/validate logic (applyClockWave()/setLength() take the value as-is, unclamped) -- a
/// real architectural shift (declarative range binding replacing a widget's own min()/max()),
/// not a testing gap this pass can close.
class TestQuickElementEditor : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void testSetCanvasBindsSelectionAndRefreshesOnChange();
    void testEmptySelectionClearsFields();

    void testLabelSetAppliesUndoably();
    void testColorSetAppliesUndoablyAndColorOptionsNonEmpty();
    void testTriggerSetForcesUppercaseAndAppliesUndoably();
    void testTriggerRejectsMultiCharacter();
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
