// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Core/TestSettings.h"

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
