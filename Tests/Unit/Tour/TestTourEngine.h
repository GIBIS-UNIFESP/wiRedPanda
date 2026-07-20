// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestTourEngine : public QObject
{
    Q_OBJECT

private slots:
    void testRetranslateBeforeLoadIsNoOp();
    void testRetranslateWhileInactiveIsNoOp();
    void testRetranslatePreservesProgressAndData();
    void testRetranslateEmitsRetranslatedOnly();

    // TourOverlay::scaledFontPx() (#14 accessibility: font-scale-hostile hardcoded px sizes)
    void testOverlayFontScalesWithApplicationFont();

    void testStartWithoutLoadIsNoOp();
    void testAdvanceStepWhileInactiveIsNoOp();
    void testStopWhileActiveEmitsAndDeactivates();
    void testStopWhileInactiveIsNoOp();
    void testGoToPreviousStepMovesBackAndPersists();
    void testGoToPreviousStepAtFirstStepIsNoOp();
    void testGoToPreviousStepWhileInactiveIsNoOp();
    void testAdvanceStepReachesEndMarksCompletedAndEmits();
    void testLoadFromResourceMissingFileFails();
    void testLoadFromResourceInvalidJsonFails();
    void testLoadFromResourceMissingIdOrTitleFails();
    void testLoadFromResourceEmptyStepsArrayFails();
    void testStepWithoutKeyUsesRawTitleAndBodyDirectly();
    void testCurrentStepDataBeforeLoadReturnsEmptyStep();
    void testRetranslateReloadFailureDeactivatesEngine();

    // TourOverlay
    void testOverlayConstructionBuildsUiWidgets();
    void testOverlayAppliesThemeReactsToThemeChange();
    void testOverlayOnStepChangedUpdatesLabelsAndButtons();
    void testOverlayOnStepChangedEnablesPrevAfterFirstStep();
    void testOverlayOnStepChangedLastStepShowsFinish();
    void testOverlayOnStepChangedSkipsResolverForEmptyOrNoneTarget();
    void testOverlayOnStepChangedResolvesHighlightRectForRealTarget();
    void testOverlayOnRetranslatedUpdatesLabelsAndButtons();
    void testOverlayOnTourFinishedHidesAndEmitsCloseRequested();
    void testOverlaySetParentWindowReparentsAndResizes();
    void testOverlayEventFilterResizesOnParentResize();
    void testOverlayPrevButtonCallsGoToPreviousStep();
    void testOverlayNextButtonCallsAdvanceStep();
    void testOverlayCloseButtonEmitsCloseRequested();
    void testRepositionCalloutCentersWhenNoHighlight();
    void testRepositionCalloutPositionsBelowSpotlight();
    void testRepositionCalloutPositionsAboveWhenNoRoomBelow();
    void testRepositionCalloutCentersRightWhenNoRoomAboveOrBelow();
    void testPaintEventDrawsWithoutHighlight();
    void testPaintEventReResolvesHighlightRectFromResolver();
};
