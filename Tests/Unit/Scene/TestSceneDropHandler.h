// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestSceneDropHandler : public QObject
{
    Q_OBJECT

private slots:

    void testIsSupportedDropFormatRecognizesAllFourMimeTypes();
    void testIsSupportedDropFormatRejectsUnrecognizedMimeType();
    void testHandleCloneDragClonesSelectionAtOffsetPosition();
    void testHandleCloneDragAcceptsLegacyMimeFormat();
    void testHandleNewElementDropAddsElementAtScenePosition();
    void testAddFromMimeDataAddsElementAtGivenScenePos();
    void testAddFromMimeDataKeepsDefaultPositionWhenScenePosOmitted();
    void testAddFromMimeDataThatDoesNotReadReclaimsTheMimeData();

    // The scene's picture of an in-process drag (DragPreview), where the drop will put what it shows
    void testPaletteDragGhostIsWhereTheDropPutsTheElement();
    void testCloneDragGhostIsWhereTheDropPutsTheCopies();
    void testDragGhostLeavesWithTheDrag();
    void testDragFromAnotherProcessHasNoGhost();

    // A drop grows the scene over what it adds and leaves the view where it was
    void testPaletteDropGrowsTheSceneOverTheElement();
    void testCloneDropKeepsTheViewStill();
};
