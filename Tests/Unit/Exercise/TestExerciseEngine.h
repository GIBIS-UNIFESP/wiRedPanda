// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestExerciseEngine : public QObject
{
    Q_OBJECT

private slots:
    void testRetranslateBeforeLoadIsNoOp();
    void testRetranslateWhileInactiveIsNoOp();
    void testRetranslatePreservesProgressAndData();
    void testRetranslateEmitsRetranslatedOnly();
    void testNegativeMinCountClampsToZero();

    // Regression: an unknown element type in requiredElements silently made a step
    // permanently unsolvable with no diagnostic, unlike every other malformed-content path
    // in this feature.
    void testUnknownElementTypeWarnsAndNeverAdvances();

    // ExerciseOverlay::scaledFontPx() (#14 accessibility: font-scale-hostile hardcoded px sizes)
    void testOverlayFontScalesWithApplicationFont();

    // StepEngineCore::loadFromResource()'s malformed-content guards, via ExerciseEngine
    void testLoadFromResourceMissingFileFails();
    void testLoadFromResourceInvalidJsonFails();
    void testLoadFromResourceMissingIdOrTitleFails();
    void testLoadFromResourceEmptyStepsArrayFails();

    // fillExerciseStepFields()'s no-"key" branch (raw text, no translation lookup)
    void testStepWithoutKeyUsesRawInstructionAndHintDirectly();

    // setScene()'s reconnect-on-switch and connect-while-active branches
    void testSetSceneReconnectsWhenCalledTwice();
    void testSetSceneWhileActiveConnectsImmediately();

    // start()/stop()/goToPreviousStep() guard clauses and real transitions
    void testStartWithoutLoadIsNoOp();
    void testAdvanceStepWhileInactiveIsNoOp();
    void testStopWhileActiveDisconnectsAndEmits();
    void testStopWhileInactiveIsNoOp();
    void testGoToPreviousStepMovesBackAndPersists();
    void testGoToPreviousStepAtFirstStepIsNoOp();
    void testGoToPreviousStepWhileInactiveIsNoOp();

    // advance()'s reachedEnd path: markCompleted() + exerciseCompleted, never exercised before
    void testAdvanceStepReachesEndMarksCompletedAndEmits();

    // retranslate()'s reload-failure path
    void testRetranslateReloadFailureDeactivatesEngine();

    // currentStepData() before any load
    void testCurrentStepDataBeforeLoadReturnsEmptyStep();

    // onCircuitChanged()'s guard clause, its own precondition for even being connected
    void testOnCircuitChangedGuardsAgainstInactiveOrSceneless();

    // The exercise "happy path": real requirement validation driving auto-advance, and its
    // negative counterparts -- none of this was exercised before (only the always-fails
    // unknown-type path was).
    void testOnCircuitChangedDoesNotAdvanceOnObserveStep();
    void testOnCircuitChangedAutoAdvancesWhenElementRequirementSatisfied();
    void testOnCircuitChangedDoesNotAdvanceWhenElementCountInsufficient();
    void testValidateConnectionsUnknownTypeWarnsAndNeverAdvances();
    void testOnCircuitChangedAutoAdvancesWhenConnectionRequirementSatisfied();
    void testOnCircuitChangedDoesNotAdvanceWhenConnectionCountInsufficient();

    // ExerciseOverlay: constructor/setupUi()/applyTheme(), previously untested beyond
    // scaledFontPx() itself
    void testOverlayConstructionBuildsUiWidgets();
    void testOverlayAppliesThemeReactsToThemeChange();
    void testOverlayOnStepChangedUpdatesLabelsAndButtons();
    void testOverlayOnStepChangedObserveStepEnablesNext();
    void testOverlayOnStepChangedLastStepShowsFinish();
    void testOverlayOnStepChangedResetsHintVisibility();
    void testOverlayHintButtonTogglesVisibility();
    void testOverlayPrevButtonCallsGoToPreviousStep();
    void testOverlayNextButtonCallsAdvanceStep();
    void testOverlayCloseButtonEmitsCloseRequested();
    void testOverlayOnRetranslatedUpdatesTextsFromCurrentEngineState();
    void testOverlayOnExerciseCompletedShowsCompletionUiAndClosesOnNextClick();
    void testOverlayRepositionToParentMovesToBottomCenterOfParent();
    void testOverlayRepositionToParentNoParentIsNoOp();
    void testOverlayPaintEventDrawsRoundedBackground();
};
