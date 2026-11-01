// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

class TestSequentialLogic : public QObject
{
    Q_OBJECT

private slots:
    // Port count and type verification
    void testDFlipFlop();
    void testDLatch();
    void testJKFlipFlop();
    void testSRFlipFlop();
    void testSRLatch();
    void testTFlipFlop();

    // Behavioral tests
    void testDFlipFlopBehavior();
    void testDLatchBehavior();
    void testJKFlipFlopBehavior();
    void testSRFlipFlopBehavior();
    void testSRLatchBehavior();
    void testTFlipFlopBehavior();
    void testSRFlipFlopWithOneOfSAndROpen();
    void testTFlipFlopWithTOpen();

    // resetSimState() tests: the four flip-flops keep an edge-detection history, which a reset returns to power-on's;
    // the latches keep none
    void testDFlipFlopResetSimState();
    void testJKFlipFlopResetSimState();
    void testSRFlipFlopResetSimState();
    void testTFlipFlopResetSimState();

    // A flip-flop's kernel history (Element::kernelState()): an edge latches the data of the evaluation before, the
    // power-on history is its descriptor's, and a reset, a save and a restore carry it
    void testDFlipFlopLatchesTheDBeforeTheEdge();
    void testFirstEvaluationWithTheClockHighIsAnEdge_data();
    void testFirstEvaluationWithTheClockHighIsAnEdge();
    void testFlipFlopHistoryFollowsResetSaveAndRestore();

    // A presenter draws the picture of the theme it is handed, never the current one's; the body is every theme's
    void testAThemedBodyIsDrawnInTheThemeHanded();
    // Their picture is the theme's: a refresh after a theme switch keeps it, and a custom appearance stays custom
    void testThemedPictureFollowsTheThemeThroughARefresh();
    void testThemeSwitchKeepsACustomAppearance();
    // An undo snapshot taken in one theme restores the current theme's picture, not a custom one
    void testUndoAfterAThemeSwitchShowsTheCurrentTheme();
};
