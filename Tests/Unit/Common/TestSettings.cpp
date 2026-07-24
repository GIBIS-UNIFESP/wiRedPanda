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

void TestSettings::testTypedMinimapGeometry()
{
    // Unset: invalid, so callers know to fall back to a default position/size.
    QVERIFY(!Settings::minimapGeometry().isValid());

    const QRect geometry(30, 40, 200, 150);
    Settings::setMinimapGeometry(geometry);
    QCOMPARE(Settings::minimapGeometry(), geometry);
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

void TestSettings::testTypedUpdateChecksDisabled()
{
    // Fresh store (init() cleared it): update checks are enabled by default.
    QCOMPARE(Settings::updateChecksDisabled(), false);

    Settings::setUpdateChecksDisabled(true);
    QCOMPARE(Settings::updateChecksDisabled(), true);

    Settings::setUpdateChecksDisabled(false);
    QCOMPARE(Settings::updateChecksDisabled(), false);
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

void TestSettings::testTypedHideV4Warning()
{
    // Fresh store: not hidden by default.
    QCOMPARE(Settings::hideV4Warning(), false);

    Settings::setHideV4Warning(true);
    QCOMPARE(Settings::hideV4Warning(), true);

    // setHideV4Warning(false) removes the key outright rather than storing
    // "false" (so a stale "true" from an older release can't linger under a
    // different key casing/format) — verify the round trip back to unhidden.
    Settings::setHideV4Warning(false);
    QCOMPARE(Settings::hideV4Warning(), false);
}

void TestSettings::testTypedUpdateCheckLastDate()
{
    QCOMPARE(Settings::updateCheckLastDate(), QString());

    Settings::setUpdateCheckLastDate("2026-07-19");
    QCOMPARE(Settings::updateCheckLastDate(), QString("2026-07-19"));
}

void TestSettings::testTypedUpdateCheckSkippedVersion()
{
    QCOMPARE(Settings::updateCheckSkippedVersion(), QString());

    Settings::setUpdateCheckSkippedVersion("5.2.0");
    QCOMPARE(Settings::updateCheckSkippedVersion(), QString("5.2.0"));
}

void TestSettings::testTypedCompletedExercises()
{
    QVERIFY(Settings::completedExercises().isEmpty());

    const QStringList ids = {"level1_and_gate", "level2_half_adder"};
    Settings::setCompletedExercises(ids);
    QCOMPARE(Settings::completedExercises(), ids);
}

void TestSettings::testTypedCompletedTours()
{
    QVERIFY(Settings::completedTours().isEmpty());

    const QStringList ids = {"welcome_tour", "wiring_tour"};
    Settings::setCompletedTours(ids);
    QCOMPARE(Settings::completedTours(), ids);
}

void TestSettings::testTypedMinimapVisible()
{
    // Fresh store: visible by default.
    QCOMPARE(Settings::minimapVisible(), true);

    Settings::setMinimapVisible(false);
    QCOMPARE(Settings::minimapVisible(), false);

    Settings::setMinimapVisible(true);
    QCOMPARE(Settings::minimapVisible(), true);
}

void TestSettings::testThemeOutOfRangeFallsBackToSystem()
{
    // Settings::setTheme() can never itself write an out-of-range value; this
    // simulates a corrupted/future settings file (e.g. downgrading from a
    // release that added a new Theme enumerator) by writing the raw int
    // directly, bypassing the typed setter.
    QSettings appSettings(Settings::fileName(), QSettings::IniFormat);
    appSettings.setValue("theme", 99);
    appSettings.sync();

    QCOMPARE(Settings::theme(), Theme::System);

    appSettings.setValue("theme", -1);
    appSettings.sync();
    QCOMPARE(Settings::theme(), Theme::System);
}
