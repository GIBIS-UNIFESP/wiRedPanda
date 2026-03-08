// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestSceneState : public QObject
{
    Q_OBJECT

private slots:

    // Scene bounds and resize tests
    void testSceneRectAfterAddElement();
    void testSceneRectAfterRemoveElement();
    void testResizeSceneExpandsBounds();
    void testSceneRectMultipleElements();

    // Scene item queries and filtering
    void testItemAtValidPosition();
    void testItemAtEmptyPosition();
    void testItemsAtMultiple();
    void testVisibleElementsInViewport();
    void testElementsInRectangle();
    void testSelectedElementsFiltering();

    // Connection state management
    void testEditedConnectionSetGet();
    void testEditedConnectionClears();
    void testActiveConnectionTracking();
    void testConnectionStateTransitions();

    // Hover port state
    void testHoverPortSetGet();
    void testHoverPortRelease();
    void testHoverPortTracking();

    // Autosave and update requests
    void testCheckUpdateRequestTriggers();
    void testUpdateBlockingDuringTransaction();
    void testSceneCleanupAfterLoad();

    // Element and connection counts
    void testElementCountAfterAddition();
    void testElementCountAfterRemoval();
    void testConnectionCountTracking();
    void testMixedElementTypes();

    // Z-order and rendering
    void testElementZOrderOnSelection();
    void testConnectionZOrderBehindElements();
    void testBringToFrontAfterSelection();

    // Scene state persistence
    void testSceneRetainsSizeAfterClear();
    void testElementPropertiesAfterMove();
    void testConnectionPropertiesAfterElementMove();
};

