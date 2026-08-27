// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestElementEditor : public QObject
{
    Q_OBJECT

private slots:

    void initTestCase();

    void testCreation();
    void testSetScene();
    void testRetranslateUi();
    void testSetCurrentElementsEmpty();
    void testSetCurrentElementsGate();
    void testSetCurrentElementsLed();
    void testAppearanceStateTileSelection();
    void testFillColorComboBox();

    // Regression: B21 — combo-box rebuild must not push spurious port-size commands
    void testSelectionDoesNotPushPortSizeCommandB21();

    void testRenameActionFocusesAndSelectsLabelField();
    void testChangeTriggerActionFocusesAndSelectsTriggerField();

    void testContextMenuRenameActionFocusesLabelField();
    void testContextMenuTriggerActionFocusesTriggerField();
    void testContextMenuFrequencyActionFocusesSpinBox();
    void testContextMenuAppearanceRevertSetsDefaultAppearance();
    void testContextMenuEditSubcircuitEmitsForEmbeddedIC();
    void testContextMenuEmbedSubcircuitEmitsForFileBackedIC();
    void testContextMenuExtractToFileEmitsForEmbeddedIC();

    void testUpdateElementAppearanceCancelledDialogIsNoOp();
    void testUpdateElementAppearanceViaTileGridUpdatesSelectedStateOnly();
    void testUpdateElementAppearanceFallbackAppliesToAllSelected();

    void testApplyCapabilitiesMixedWirelessModeShowsPlaceholder();
    void testApplyCapabilitiesMixedFrequencyShowsPlaceholder();
    void testApplyCapabilitiesMuxShowsDataInputOptions();
    void testApplyCapabilitiesDemuxShowsOutputOptions();
    void testApplyCapabilitiesGenericMultiOutputShowsCuratedOptions();
    void testApplyCapabilitiesMixedLockStateShowsPartiallyChecked();

    void testApplyVolumeProperty();
    void testApplyWirelessModeChangeSeversExistingConnection();

    void testBlobNameEditingFinishedIgnoresUnchangedOrEmpty();
    void testBlobNameEditingFinishedRejectsCollision();
    void testBlobNameEditingFinishedRenamesSuccessfully();

    void testInputIndexChangedGuardsEmptyOrDisabled();
    void testInputIndexChangedSendsChangePortSizeCommand();
    void testOutputIndexChangedGuardsEmptyOrDisabled();
    void testOutputIndexChangedSendsChangePortSizeCommand();

    void testOutputValueChangedForInputRotary();
    void testOutputValueChangedForRegularInput();

    void testInputLockedGuardsEmptyOrDisabled();
    void testInputLockedSetsLockState();

    void testTruthTablePopulatesTableAndShows();
    void testSetTruthTablePropositionTogglesOutputBit();

    void testAudioBoxGuardsNonAudioBoxSelection();
    void testAudioBoxCancelledDialogIsNoOp();
    void testAudioBoxSetsAudioAndUpdatesLabel();

    // --- Propagation delay (temporal simulation) ---

    /// The Timing group box AND the Prop. delay field must both be shown for a gate. Gating the
    /// box on hasFrequency||hasDelay alone is true only for Clock/Buzzer, and would hide the
    /// field for every element type that actually has a propagation delay.
    void testPropagationDelayFieldVisibleForGate();
    /// Elements without a propagation delay (an LED) keep the field hidden.
    void testPropagationDelayFieldHiddenWithoutDelay();
    /// Editing the spin box writes an override, and entering the type default clears it again.
    void testPropagationDelayRoundTripAndDefaultClearsOverride();
    /// A mixed selection shows the -1 "many values" sentinel and leaves both elements untouched.
    void testPropagationDelayMixedSelectionIsNoOp();
    /// The edited delay must reach the running engine without a topology rebuild, and undo
    /// must put the old one back.
    void testPropagationDelayReachesEngineAndUndoRestores();
};
