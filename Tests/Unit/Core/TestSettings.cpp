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

void TestCoreSettings::testCrashReportingDefaultsToEnabledWhenKeyAbsent()
{
    // The upgrade case, and the one that would do real damage: every existing
    // installation has no crashReporting/enabled key. A plain value().toBool() returns
    // false for a missing key, which would silently switch crash reporting off for the
    // entire installed base the moment they upgraded. An absent key must read as ENABLED.
    const bool original = Settings::crashReportingEnabled();

    // Store an explicit false FIRST. Without this the test would be vacuous on a machine
    // where the key never existed: remove() would be a no-op and the assertion below
    // would pass without exercising the default at all. Starting from a stored false
    // means only a genuinely absent key can produce true.
    Settings::setCrashReportingEnabled(false);
    QCOMPARE(Settings::crashReportingEnabled(), false);

    QSettings appSettings(Settings::fileName(), QSettings::IniFormat);
    appSettings.remove("crashReporting/enabled");
    appSettings.sync();

    QVERIFY2(Settings::crashReportingEnabled(),
             "crash reporting must default to ENABLED when the key is absent, or every "
             "existing install silently stops reporting on upgrade");

    Settings::setCrashReportingEnabled(original);
}

void TestCoreSettings::testCrashReportingRoundTrip()
{
    const bool originalEnabled = Settings::crashReportingEnabled();
    const QString originalId = Settings::sentryUserId();

    Settings::setCrashReportingEnabled(false);
    QCOMPARE(Settings::crashReportingEnabled(), false);
    Settings::setCrashReportingEnabled(true);
    QCOMPARE(Settings::crashReportingEnabled(), true);

    // The anonymous id lives alongside the toggle rather than in a bare QSettings(),
    // whose default constructor resolves its organisation from QCoreApplication and so
    // used to strand the id in an unnamespaced "Unknown Organization.conf".
    Settings::setSentryUserId("test-id-1234");
    QCOMPARE(Settings::sentryUserId(), QString("test-id-1234"));
    Settings::setSentryUserId(QString());
    QVERIFY(Settings::sentryUserId().isEmpty());

    Settings::setCrashReportingEnabled(originalEnabled);
    Settings::setSentryUserId(originalId);
}
