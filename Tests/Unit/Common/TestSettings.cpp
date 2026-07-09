// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Common/TestSettings.h"

#include <QFileInfo>
#include <QSettings>

#include "App/Core/Settings.h"
#include "App/Core/ThemeManager.h"
#include "Tests/Common/TestUtils.h"

void TestSettings::initTestCase()
{
    // The whole harness is redirected to a temporary QSettings path by
    // TestUtils::setupTestEnvironment() (called in runTestSuite() before the
    // Application is constructed), so the tests below can freely mutate the
    // Settings:: store without touching the developer's real configuration.
}

void TestSettings::init()
{
    // Clear the (redirected) application settings store before each test so
    // every test starts from a blank state. This must clear the store the
    // Settings:: API actually uses — the test bodies mutate Settings::, not a
    // separate test-only organization/application pair. Settings::fileName()
    // resolves to the same ini file the Settings singleton writes.
    QSettings appSettings(Settings::fileName(), QSettings::IniFormat);
    appSettings.clear();
    appSettings.sync();
}

void TestSettings::cleanup()
{
    // Clean up after each test
    QSettings appSettings(Settings::fileName(), QSettings::IniFormat);
    appSettings.clear();
    appSettings.sync();
}

// ============================================================
// Basic Operations Tests
// ============================================================

void TestSettings::testSettingsFileName()
{
    // Get the settings file path
    QString fileName = Settings::fileName();
    QVERIFY(!fileName.isEmpty());

    // Verify it's a valid path
    QFileInfo fileInfo(fileName);
    QVERIFY(!fileInfo.absolutePath().isEmpty());
}

// ============================================================
// Typed Accessors Tests
// ============================================================

void TestSettings::testTypedMainWindowGeometry()
{
    QByteArray geometry("test_geometry");
    Settings::setMainWindowGeometry(geometry);
    QCOMPARE(Settings::mainWindowGeometry(), geometry);
}

void TestSettings::testTypedMainWindowState()
{
    QByteArray state("test_state");
    Settings::setMainWindowState(state);
    QCOMPARE(Settings::mainWindowState(), state);
}

void TestSettings::testTypedSplitterGeometry()
{
    QByteArray geometry("splitter_geometry");
    Settings::setSplitterGeometry(geometry);
    QCOMPARE(Settings::splitterGeometry(), geometry);
}

void TestSettings::testTypedSplitterState()
{
    QByteArray state("splitter_state");
    Settings::setSplitterState(state);
    QCOMPARE(Settings::splitterState(), state);
}

void TestSettings::testTypedDolphinGeometry()
{
    QByteArray geometry("dolphin_geometry");
    Settings::setDolphinGeometry(geometry);
    QCOMPARE(Settings::dolphinGeometry(), geometry);
}

void TestSettings::testTypedFastMode()
{
    Settings::setFastMode(true);
    QCOMPARE(Settings::fastMode(), true);

    Settings::setFastMode(false);
    QCOMPARE(Settings::fastMode(), false);
}

void TestSettings::testTypedLabelsUnderIcons()
{
    // Fresh install (init() cleared the store): toolbar labels default to visible so
    // the beginner audience gets readable buttons.
    QCOMPARE(Settings::labelsUnderIcons(), true);

    Settings::setLabelsUnderIcons(true);
    QCOMPARE(Settings::labelsUnderIcons(), true);

    Settings::setLabelsUnderIcons(false);
    QCOMPARE(Settings::labelsUnderIcons(), false);
}

void TestSettings::testTypedLanguage()
{
    QStringList languages = {"en", "pt_BR", "es", "fr", "de"};

    for (const QString &lang : languages) {
        Settings::setLanguage(lang);
        QCOMPARE(Settings::language(), lang);
    }
}

void TestSettings::testTypedTheme()
{
    Settings::setTheme(Theme::Light);
    QCOMPARE(Settings::theme(), Theme::Light);

    Settings::setTheme(Theme::Dark);
    QCOMPARE(Settings::theme(), Theme::Dark);
}

void TestSettings::testTypedRecentFiles()
{
    QStringList recentFiles = {
        "/path/to/file1.panda",
        "/path/to/file2.panda",
        "/path/to/file3.panda"
    };

    Settings::setRecentFiles(recentFiles);
    QCOMPARE(Settings::recentFiles(), recentFiles);
}

void TestSettings::testTypedAutosaveFiles()
{
    QStringList autosaveFiles = {
        "/tmp/.circuit1.XXXXX.panda",
        "/tmp/.circuit2.XXXXX.panda"
    };

    Settings::setAutosaveFiles(autosaveFiles);
    QCOMPARE(Settings::autosaveFiles(), autosaveFiles);
}
