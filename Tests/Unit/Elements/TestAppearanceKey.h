// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>

/**
 * \brief What an appearance key holds that the body snapshot's states do not vary: an empty Text's hint in the
 * language shown, the colour of a display with no segment lit, a dial's marks at one position, coordinates compared exactly, and turns given past a
 * full one or below zero.
 */
class TestAppearanceKey : public QObject
{
    Q_OBJECT

private slots:
    void testAnEmptyTextsKeyHoldsItsHintInTheLanguageShown();
    void testAnUnlitDisplaysKeyLeavesOutItsColour();
    void testADialsKeyHoldsHowManyPositionsItMarks();
    void testKeysCompareEveryCoordinateExactly();
    void testEquivalentTurnsShareAKey();
};
