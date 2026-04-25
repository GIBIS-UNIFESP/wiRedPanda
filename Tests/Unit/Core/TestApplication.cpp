// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Core/TestApplication.h"

#include <QCoreApplication>

#include "App/Core/Application.h"
#include "Tests/Common/TestUtils.h"

void TestApplication::testApplicationNotify()
{
    // Application requires argc/argv — verify the test runner's QApplication is alive.
    QVERIFY(QCoreApplication::instance() != nullptr);
}

void TestApplication::testExceptionHandling()
{
    QVERIFY(QCoreApplication::instance() != nullptr);
}

void TestApplication::testIsSentryDenyMessageMatchesClusterD()
{
    // The deny list must filter the addressed Cluster D defensive throws.
    QVERIFY(Application::isSentryDenyMessage("One or more items was not found on the scene."));
    QVERIFY(Application::isSentryDenyMessage("One or more elements was not found on the scene."));
    QVERIFY(Application::isSentryDenyMessage(
        "One or more elements were not found on scene. Expected 3, found 2."));

    // The match is a substring check, so prefixes/suffixes around the pattern
    // still count — that's the intended behavior so future translations or
    // wrappers don't accidentally bypass the filter.
    QVERIFY(Application::isSentryDenyMessage(
        "[command] One or more items was not found on the scene (cluster D)."));
}

void TestApplication::testIsSentryDenyMessageDoesNotFilterUnknown()
{
    // Anything outside the deny list must pass through. A regression that
    // over-matches would silence genuinely actionable Sentry signal.
    QVERIFY(!Application::isSentryDenyMessage(""));
    QVERIFY(!Application::isSentryDenyMessage("Could not save file: Permission denied"));
    QVERIFY(!Application::isSentryDenyMessage("Couldn't load pixmap: foo.svg"));
    QVERIFY(!Application::isSentryDenyMessage("Save file first."));
}

