// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QSignalSpy>
#include <QTest>

class TestThemeManager : public QObject
{
    Q_OBJECT

private slots:
    void cleanup();

    // Theme Loading Tests (5 tests)
    void testDefaultThemeInitialization();
    void testLoadThemeFromSettings();
    void testThemePersistence();
    void testThemePathLight();
    void testThemePathDark();

    // Theme Switching Tests (3 tests)
    void testSetThemeLight();
    void testSetThemeDark();
    void testThemeChangedSignal();

    // Light Theme Attributes Tests (4 tests)
    void testLightThemeColors();
    void testLightThemeSceneColors();
    void testLightThemeConnectionColors();
    void testLightThemePortColors();

    // Dark Theme Attributes Tests (4 tests)
    void testDarkThemeColors();
    void testDarkThemeSceneColors();
    void testDarkThemeConnectionColors();
    void testDarkThemePortColors();

    // Palette Switching Tests (3 tests)
    void testDarkToLightPaletteSwitching();
    void testLightToDarkPaletteSwitching();
    void testRepeatedThemeSwitching();
};

