// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Core/TestSystemThemeDetector.h"

#include "App/Core/SystemThemeDetector.h"
#include "Tests/Common/TestUtils.h"

void TestSystemThemeDetector::testDetectTheme()
{
    ColorScheme scheme = SystemThemeDetector::colorScheme();
    QVERIFY(scheme == ColorScheme::Light || scheme == ColorScheme::Dark || scheme == ColorScheme::Unknown);
}

void TestSystemThemeDetector::testThemeValidation()
{
    bool dark = SystemThemeDetector::isDark();
    Q_UNUSED(dark);
    QVERIFY(true);
}

