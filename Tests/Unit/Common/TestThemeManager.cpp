// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Common/TestThemeManager.h"

#include <QStyleHints>

#include "App/Core/Settings.h"
#include "App/Core/ThemeManager.h"
#include "Tests/Common/TestUtils.h"

void TestThemeManager::cleanup()
{
    // Restore to default theme after each test
    ThemeManager::setTheme(Theme::Light);
}

// ============================================================
// Theme Loading Tests (5 tests)
// ============================================================

void TestThemeManager::testDefaultThemeInitialization()
{
    // Test that theme initializes to a valid default
    Theme currentTheme = ThemeManager::theme();
    QVERIFY(currentTheme == Theme::Light || currentTheme == Theme::Dark || currentTheme == Theme::System);

    // Should have valid attributes
    ThemeAttributes attrs = ThemeManager::attributes();
    QVERIFY(attrs.m_sceneBgBrush.isValid());
    QVERIFY(attrs.m_sceneBgDots.isValid());
}

void TestThemeManager::testLoadThemeFromSettings()
{
    // Set theme to Dark via setTheme (which saves to Settings)
    ThemeManager::setTheme(Theme::Dark);
    QCOMPARE(ThemeManager::theme(), Theme::Dark);

    // Verify it's stored in Settings
    int savedTheme = static_cast<int>(Settings::theme());
    QCOMPARE(savedTheme, static_cast<int>(Theme::Dark));

    // Reset to Light
    ThemeManager::setTheme(Theme::Light);
    QCOMPARE(ThemeManager::theme(), Theme::Light);

    savedTheme = static_cast<int>(Settings::theme());
    QCOMPARE(savedTheme, static_cast<int>(Theme::Light));
}

void TestThemeManager::testThemePersistence()
{
    // Set Dark theme and verify it persists in settings
    ThemeManager::setTheme(Theme::Dark);
    QCOMPARE(ThemeManager::theme(), Theme::Dark);

    int darkValue = static_cast<int>(Theme::Dark);
    int savedValue = static_cast<int>(Settings::theme());
    QCOMPARE(savedValue, darkValue);

    // Switch back to Light
    ThemeManager::setTheme(Theme::Light);
    QCOMPARE(ThemeManager::theme(), Theme::Light);

    int lightValue = static_cast<int>(Theme::Light);
    savedValue = static_cast<int>(Settings::theme());
    QCOMPARE(savedValue, lightValue);
}

void TestThemeManager::testThemePathLight()
{
    // Test themePath() returns "Light" for Light theme
    ThemeManager::setTheme(Theme::Light);
    QString path = ThemeManager::themePath();
    QCOMPARE(path, QString("Light"));
}

void TestThemeManager::testThemePathDark()
{
    // Test themePath() returns "Dark" for Dark theme
    ThemeManager::setTheme(Theme::Dark);
    QString path = ThemeManager::themePath();
    QCOMPARE(path, QString("Dark"));
}

// ============================================================
// Theme Switching Tests (3 tests)
// ============================================================

void TestThemeManager::testSetThemeLight()
{
    // Set to Dark first, then switch to Light
    ThemeManager::setTheme(Theme::Dark);
    QCOMPARE(ThemeManager::theme(), Theme::Dark);

    // Switch to Light
    ThemeManager::setTheme(Theme::Light);
    QCOMPARE(ThemeManager::theme(), Theme::Light);
    QCOMPARE(ThemeManager::themePath(), QString("Light"));

    // Verify attributes changed
    ThemeAttributes attrs = ThemeManager::attributes();
    QVERIFY(attrs.m_sceneBgBrush.isValid());
}

void TestThemeManager::testSetThemeDark()
{
    // Start with Light theme
    ThemeManager::setTheme(Theme::Light);
    QCOMPARE(ThemeManager::theme(), Theme::Light);

    // Switch to Dark
    ThemeManager::setTheme(Theme::Dark);
    QCOMPARE(ThemeManager::theme(), Theme::Dark);
    QCOMPARE(ThemeManager::themePath(), QString("Dark"));

    // Verify attributes changed
    ThemeAttributes attrs = ThemeManager::attributes();
    QVERIFY(attrs.m_sceneBgBrush.isValid());
}

void TestThemeManager::testThemeChangedSignal()
{
    // Test that themeChanged() signal is emitted when theme changes
    QSignalSpy spy(&ThemeManager::instance(), &ThemeManager::themeChanged);

    // Set to Light (may already be Light, so clear spy first)
    ThemeManager::setTheme(Theme::Light);
    spy.clear();

    // Switch to Dark - should emit signal
    ThemeManager::setTheme(Theme::Dark);
    QCOMPARE(spy.size(), 1);

    spy.clear();

    // Switch back to Light - should emit signal again
    ThemeManager::setTheme(Theme::Light);
    QCOMPARE(spy.size(), 1);

    // Setting the same theme again should not emit signal
    spy.clear();
    ThemeManager::setTheme(Theme::Light);
    QCOMPARE(spy.size(), 0);  // No signal because theme didn't actually change
}

// ============================================================
// Light Theme Attributes Tests (4 tests)
// ============================================================

void TestThemeManager::testLightThemeColors()
{
    // Set to Light theme and verify basic colors are set
    ThemeManager::setTheme(Theme::Light);

    ThemeAttributes attrs = ThemeManager::attributes();

    // Verify all critical colors are valid
    QVERIFY(attrs.m_sceneBgBrush.isValid());
    QVERIFY(attrs.m_sceneBgDots.isValid());
    QVERIFY(attrs.m_selectionBrush.isValid());
    QVERIFY(attrs.m_selectionPen.isValid());
    QVERIFY(attrs.m_graphicElementLabelColor.isValid());
}

void TestThemeManager::testLightThemeSceneColors()
{
    // Test Light theme scene-specific colors
    ThemeManager::setTheme(Theme::Light);

    ThemeAttributes attrs = ThemeManager::attributes();

    // Light theme should have light background
    QColor bgBrush = attrs.m_sceneBgBrush;
    QVERIFY(bgBrush.isValid());

    // Background should be relatively bright (light theme)
    // Check that it's a yellowish/cream color (255, 255, 230)
    QVERIFY(bgBrush.red() > 200);
    QVERIFY(bgBrush.green() > 200);

    // Dots should be darkish for visibility on light background
    QColor dots = attrs.m_sceneBgDots;
    QVERIFY(dots.isValid());
}

void TestThemeManager::testLightThemeConnectionColors()
{
    // Test Light theme connection colors
    ThemeManager::setTheme(Theme::Light);

    ThemeAttributes attrs = ThemeManager::attributes();

    // Connection colors should be valid
    QVERIFY(attrs.m_connectionUnknown.isValid());
    QVERIFY(attrs.m_connectionInactive.isValid());
    QVERIFY(attrs.m_connectionActive.isValid());
    QVERIFY(attrs.m_connectionSelected.isValid());
    QVERIFY(attrs.m_connectionError.isValid());

    // Inactive should be dark green, active should be bright green
    // These specific values come from the implementation
    QCOMPARE(attrs.m_connectionInactive, QColor(Qt::darkGreen));
    QCOMPARE(attrs.m_connectionActive, QColor(Qt::green));
}

void TestThemeManager::testLightThemePortColors()
{
    // Test Light theme port/connector colors
    ThemeManager::setTheme(Theme::Light);

    ThemeAttributes attrs = ThemeManager::attributes();

    // All port colors should be valid
    QVERIFY(attrs.m_portUnknownBrush.isValid());
    QVERIFY(attrs.m_portInactiveBrush.isValid());
    QVERIFY(attrs.m_portActiveBrush.isValid());
    QVERIFY(attrs.m_portErrorBrush.isValid());

    QVERIFY(attrs.m_portUnknownPen.isValid());
    QVERIFY(attrs.m_portInactivePen.isValid());
    QVERIFY(attrs.m_portActivePen.isValid());
    QVERIFY(attrs.m_portErrorPen.isValid());

    // Hover port should be yellow
    QVERIFY(attrs.m_portHoverPort.isValid());
    QCOMPARE(attrs.m_portHoverPort, QColor(Qt::yellow));
}

// ============================================================
// Dark Theme Attributes Tests (4 tests)
// ============================================================

void TestThemeManager::testDarkThemeColors()
{
    // Set to Dark theme and verify basic colors are set
    ThemeManager::setTheme(Theme::Dark);

    ThemeAttributes attrs = ThemeManager::attributes();

    // Verify all critical colors are valid
    QVERIFY(attrs.m_sceneBgBrush.isValid());
    QVERIFY(attrs.m_sceneBgDots.isValid());
    QVERIFY(attrs.m_selectionBrush.isValid());
    QVERIFY(attrs.m_selectionPen.isValid());
    QVERIFY(attrs.m_graphicElementLabelColor.isValid());
}

void TestThemeManager::testDarkThemeSceneColors()
{
    // Test Dark theme scene-specific colors
    ThemeManager::setTheme(Theme::Dark);

    ThemeAttributes attrs = ThemeManager::attributes();

    // Dark theme should have dark background
    QColor bgBrush = attrs.m_sceneBgBrush;
    QVERIFY(bgBrush.isValid());

    // Background should be relatively dark
    // Check that it's a dark gray (64, 69, 82)
    QVERIFY(bgBrush.red() < 100);
    QVERIFY(bgBrush.green() < 100);
    QVERIFY(bgBrush.blue() < 100);

    // Dots should be light for visibility on dark background
    QColor dots = attrs.m_sceneBgDots;
    QVERIFY(dots.isValid());
    // Light gray dots
    QCOMPARE(dots, QColor(Qt::lightGray));
}

void TestThemeManager::testDarkThemeConnectionColors()
{
    // Test Dark theme connection colors
    ThemeManager::setTheme(Theme::Dark);

    ThemeAttributes attrs = ThemeManager::attributes();

    // Connection colors should be valid
    QVERIFY(attrs.m_connectionUnknown.isValid());
    QVERIFY(attrs.m_connectionInactive.isValid());
    QVERIFY(attrs.m_connectionActive.isValid());
    QVERIFY(attrs.m_connectionSelected.isValid());
    QVERIFY(attrs.m_connectionError.isValid());

    // Dark theme uses cyan/turquoise tones for connections
    // Inactive should be a darker cyan (greenish, not red)
    QVERIFY(attrs.m_connectionInactive.green() > attrs.m_connectionInactive.red());
    QVERIFY(attrs.m_connectionInactive.blue() > 100);  // Cyan has significant blue component
    QVERIFY(attrs.m_connectionInactive.green() > 100); // Cyan has significant green component

    // Active should be bright cyan (more green than red, lots of blue)
    QVERIFY(attrs.m_connectionActive.green() > attrs.m_connectionActive.red());
    QVERIFY(attrs.m_connectionActive.blue() > 200);     // Bright cyan
    QVERIFY(attrs.m_connectionActive.green() > 200);    // Bright cyan
}

void TestThemeManager::testDarkThemePortColors()
{
    // Test Dark theme port/connector colors
    ThemeManager::setTheme(Theme::Dark);

    ThemeAttributes attrs = ThemeManager::attributes();

    // All port colors should be valid
    QVERIFY(attrs.m_portUnknownBrush.isValid());
    QVERIFY(attrs.m_portInactiveBrush.isValid());
    QVERIFY(attrs.m_portActiveBrush.isValid());
    QVERIFY(attrs.m_portErrorBrush.isValid());

    QVERIFY(attrs.m_portUnknownPen.isValid());
    QVERIFY(attrs.m_portInactivePen.isValid());
    QVERIFY(attrs.m_portActivePen.isValid());
    QVERIFY(attrs.m_portErrorPen.isValid());

    // Hover port should still be yellow
    QVERIFY(attrs.m_portHoverPort.isValid());
    QCOMPARE(attrs.m_portHoverPort, QColor(Qt::yellow));
}

// ============================================================
// Palette Switching Tests (3 tests)
// ============================================================

void TestThemeManager::testDarkToLightPaletteSwitching()
{
#if defined(Q_OS_MAC) && QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
    QSKIP("macOS + Qt < 6.8: setColorScheme() unavailable, palette does not change on theme switch");
#endif
    // Start with Dark theme
    ThemeManager::setTheme(Theme::Dark);
    QCOMPARE(ThemeManager::theme(), Theme::Dark);

    // Get dark palette state
    QColor darkWindowColor = qApp->palette().color(QPalette::Window);

    // Switch to Light
    ThemeManager::setTheme(Theme::Light);
    QCOMPARE(ThemeManager::theme(), Theme::Light);

    // Get light palette state
    QColor lightWindowColor = qApp->palette().color(QPalette::Window);

    // Light window should be lighter than dark window
    // (higher luminance value - sum of RGB components)
    int darkLuminance = darkWindowColor.red() + darkWindowColor.green() + darkWindowColor.blue();
    int lightLuminance = lightWindowColor.red() + lightWindowColor.green() + lightWindowColor.blue();

    QVERIFY(lightLuminance > darkLuminance);
}

void TestThemeManager::testLightToDarkPaletteSwitching()
{
#if defined(Q_OS_MAC) && QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
    QSKIP("macOS + Qt < 6.8: setColorScheme() unavailable, palette does not change on theme switch");
#endif
    // Start with Light theme
    ThemeManager::setTheme(Theme::Light);
    QCOMPARE(ThemeManager::theme(), Theme::Light);

    // Get light palette state
    QColor lightWindowColor = qApp->palette().color(QPalette::Window);

    // Switch to Dark
    ThemeManager::setTheme(Theme::Dark);
    QCOMPARE(ThemeManager::theme(), Theme::Dark);

    // Get dark palette state
    QColor darkWindowColor = qApp->palette().color(QPalette::Window);

    // Dark window should be darker than light window
    int lightLuminance = lightWindowColor.red() + lightWindowColor.green() + lightWindowColor.blue();
    int darkLuminance = darkWindowColor.red() + darkWindowColor.green() + darkWindowColor.blue();

    QVERIFY(darkLuminance < lightLuminance);
}

void TestThemeManager::testRepeatedThemeSwitching()
{
#if defined(Q_OS_MAC) && QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
    QSKIP("macOS + Qt < 6.8: setColorScheme() unavailable, palette does not change on theme switch");
#endif
    // Repeatedly switch themes and verify palette consistency
    for (int i = 0; i < 3; ++i) {
        // Switch to Dark
        ThemeManager::setTheme(Theme::Dark);
        QColor darkWindow = qApp->palette().color(QPalette::Window);
        int darkLuminance = darkWindow.red() + darkWindow.green() + darkWindow.blue();

        // Switch to Light
        ThemeManager::setTheme(Theme::Light);
        QColor lightWindow = qApp->palette().color(QPalette::Window);
        int lightLuminance = lightWindow.red() + lightWindow.green() + lightWindow.blue();

        // Light should always be lighter than Dark
        QVERIFY(lightLuminance > darkLuminance);
    }
}

void TestThemeManager::testSystemColorSchemeChangeReactsWhenThemeIsSystem()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
    QSKIP("QStyleHints::colorSchemeChanged requires Qt 6.5+");
#else
    // onSystemColorSchemeChanged() is a private slot-like callback only ever
    // invoked by a real OS color-scheme change, which never happens in a
    // headless test run. QStyleHints::colorSchemeChanged is a real Qt signal
    // though, so invoking it directly (the same mechanism `emit` compiles
    // to) exercises the connected callback exactly as a genuine OS toggle
    // would, without needing platform integration.
    ThemeManager::setTheme(Theme::System);
    QCOMPARE(ThemeManager::theme(), Theme::System);

    QSignalSpy spy(&ThemeManager::instance(), &ThemeManager::themeChanged);
    QMetaObject::invokeMethod(qApp->styleHints(), "colorSchemeChanged", Qt::DirectConnection,
                              Q_ARG(Qt::ColorScheme, Qt::ColorScheme::Dark));
    QCOMPARE(spy.size(), 1);
#endif
}

void TestThemeManager::testSystemColorSchemeChangeIgnoredWhenThemeIsExplicit()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 5, 0)
    QSKIP("QStyleHints::colorSchemeChanged requires Qt 6.5+");
#else
    // The guard inside onSystemColorSchemeChanged() must actually gate
    // re-emission: an explicit (non-System) theme choice must not react to
    // OS-level color-scheme toggles.
    ThemeManager::setTheme(Theme::Dark);
    QCOMPARE(ThemeManager::theme(), Theme::Dark);

    QSignalSpy spy(&ThemeManager::instance(), &ThemeManager::themeChanged);
    QMetaObject::invokeMethod(qApp->styleHints(), "colorSchemeChanged", Qt::DirectConnection,
                              Q_ARG(Qt::ColorScheme, Qt::ColorScheme::Light));
    QCOMPARE(spy.size(), 0);
#endif
}

void TestThemeManager::testThemeAttributesSystemFallsBackToLight()
{
    // ThemeAttributes::setTheme() is only ever called by ThemeManager with an
    // already-resolved Light/Dark value (see its constructor/setTheme()) —
    // Theme::System never reaches it in practice. Constructing ThemeAttributes
    // directly lets this defensive fallback be verified for real: passing
    // System must still produce valid Light-theme colors, not a
    // default-constructed (invalid) attribute set.
    ThemeAttributes attrs;
    attrs.setTheme(Theme::System);

    ThemeAttributes lightAttrs;
    lightAttrs.setTheme(Theme::Light);

    QCOMPARE(attrs.m_sceneBgBrush, lightAttrs.m_sceneBgBrush);
    QCOMPARE(attrs.m_connectionActive, lightAttrs.m_connectionActive);
    QVERIFY(attrs.m_sceneBgBrush.isValid());
}
