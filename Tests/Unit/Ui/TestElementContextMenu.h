// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestElementContextMenu : public QObject
{
    Q_OBJECT

private slots:

    // Context menu action tests (3 tests)
    void testRotateRightAction();
    void testMorphMenuAction();
    void testICSubcircuitAction();

    // Real ElementContextMenu::exec() menu-driving tests
    void testDismissWithoutChoosingIsNoOp();
    void testRotateLeftAndRightActionsPushCommands();
    void testFlipHorizontalAndVerticalActionsPushCommands();
    void testColorSubmenuPopulatesAndDispatches();
    void testCopyCutDeleteActionsDelegateToScene();
    void testMorphSubmenuGateSingleInput();
    void testMorphSubmenuGateMultiInputExcludesSameType();
    void testMorphSubmenuInputGroup();
    void testMorphSubmenuMemoryTwoInputEmptiesSubmenu();
    void testMorphSubmenuMemoryFourInput();
    void testMorphSubmenuMemoryFiveInput();
    void testMorphSubmenuOutputDisplay();
    void testMorphSubmenuOutputNonDisplay();
    void testMorphSubmenuGroupWithNoMorphTargetsRemovesSubmenu();
    void testMorphActionDispatchesMorphCommand();
    void testUnknownMenuOptionThrows();
};
