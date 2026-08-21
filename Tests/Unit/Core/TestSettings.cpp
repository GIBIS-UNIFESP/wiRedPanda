// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Core/TestSettings.h"

#include <QSettings>

#include "App/Core/Settings.h"
#include "Tests/Common/TestUtils.h"

void TestCoreSettings::testSettingsRoundTrip()
{
    QString originalLang = Settings::language();
    bool originalFastMode = Settings::fastMode();
    bool originalLabels = Settings::labelsUnderIcons();
    bool originalIcPreviewDisabled = Settings::icPreviewDisabled();

    Settings::setLanguage("pt_BR");
    Settings::setFastMode(true);
    Settings::setLabelsUnderIcons(true);
    Settings::setIcPreviewDisabled(true);

    QCOMPARE(Settings::language(), QString("pt_BR"));
    QCOMPARE(Settings::fastMode(), true);
    QCOMPARE(Settings::labelsUnderIcons(), true);
    QCOMPARE(Settings::icPreviewDisabled(), true);

    Settings::setLanguage(originalLang);
    Settings::setFastMode(originalFastMode);
    Settings::setLabelsUnderIcons(originalLabels);
    Settings::setIcPreviewDisabled(originalIcPreviewDisabled);
}

void TestCoreSettings::testThemeSystemUpperBoundRoundTrip()
{
    // Settings::theme() bounds-checks a stored value against Theme::System as its upper
    // bound ("saved <= static_cast<int>(Theme::System)"). An off-by-one that excluded
    // exactly that boundary (e.g. "<" instead of "<=") would slip past every other Settings
    // test, which only round-trips Light/Dark or feeds grossly out-of-range raw ints.
    const Theme original = Settings::theme();

    Settings::setTheme(Theme::System);
    QCOMPARE(Settings::theme(), Theme::System);

    Settings::setTheme(original);
}

void TestCoreSettings::testHideV4WarningFalseRemovesKey()
{
    // setHideV4Warning(false) must remove the settings key outright rather than storing a
    // falsy value, so a stale value from an older release/format can't linger under it.
    // hideV4Warning()'s bool getter alone can't distinguish "removed" from "stored false".
    Settings::setHideV4Warning(true);
    QCOMPARE(Settings::hideV4Warning(), true);

    Settings::setHideV4Warning(false);
    QCOMPARE(Settings::hideV4Warning(), false);

    QSettings appSettings(Settings::fileName(), QSettings::IniFormat);
    QVERIFY2(!appSettings.contains("hideV4Warning"),
             "setHideV4Warning(false) must remove the key, not store a falsy value");
}
