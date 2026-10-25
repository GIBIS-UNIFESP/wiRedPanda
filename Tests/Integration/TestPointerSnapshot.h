// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/**
 * \brief Pins what a pointer reaches: a hit grid over every small fixture and over overlapping elements, an Error
 * wire's click width, and each pointer gesture as its own test.
 * \details Each grid cell records the topmost element or wire whose shape holds the point (what a plain click lands
 * on), Scene::portAt() and Scene::itemAt(), by element id. The gestures are real QGraphicsSceneMouseEvents sent to the
 * scene of a workspace, whose view the clone-drag needs. The tests that copy, cut or paste hold CTest's clipboard
 * resource lock.
 */
class TestPointerSnapshot : public QObject
{
    Q_OBJECT

private slots:
    void testHitGridMatchesGolden_data();
    void testHitGridMatchesGolden();
    void testEveryGoldenHasAFixture();
    void testOverlapHitGridMatchesGolden();
    void testErrorWireHitWidthMatchesGolden();
    void testClickSelectsWireAlone();
    void testCtrlClickOnWireAdds();
    void testCtrlClickOnElementAddsAndStartsCloneDrag();
    void testShiftClickToggles();
    void testEmptyClickClearsSelection();
    void testPressOnElementClearsSelectedWire();
    void testReleaseWithoutMovementLeavesPressedElementAlone();
    void testDragMovesEverySelectedElement();
    void testDragOfUnselectedElementMovesItAlone();
    void testDragOfTopOverlappingElementIsOneUndoStep();
    void testPortOnTopAnswersWhereTwoAreInReach();
    void testRubberBandSelectsWhatItCovers();
    void testDragFromOutputToInputConnects();
    void testClickOnOutputRoutesUntilPressOnInput();
    void testPressOnEmptySpaceCancelsRouting();
    void testDragFromFreeInputToOutputConnects();
    void testPressOnConnectedInputDetachesWire();
    void testLoneWireDelete();
    void testLoneWireCopyAndCutClearClipboard();
    void testLoneWireDuplicateDoesNothing();
    void testLoneWireContextMenu();
    void testLoneWireSelectAll();
    void testLoneWireZoomToSelection();
    void testDuplicateSelectsItsCopiesWiresIncluded();
    void testCloneDropThroughSceneEvents();
    void testPasteAddsSelectedCopies();
};
