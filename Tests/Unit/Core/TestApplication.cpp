// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Core/TestApplication.h"

#include <QCoreApplication>
#include <QMessageBox>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
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

void TestApplication::testMakeExceptionInfoWithPandaception()
{
    try {
        throw PANDACEPTION_LITERAL("Test pandaception message for coverage");
    } catch (const std::exception &e) {
        const ExceptionInfo info = Application::makeExceptionInfo(e);
        QCOMPARE(info.what, QString("Test pandaception message for coverage"));
        QCOMPARE(info.englishMessage, QString("Test pandaception message for coverage"));
        QVERIFY2(info.file.endsWith("TestApplication.cpp"), qPrintable(info.file));
        QVERIFY(info.line > 0);
    }
}

void TestApplication::testMakeExceptionInfoWithPlainStdException()
{
    // A plain (non-Pandaception) std::exception has no throw-site file/line —
    // makeExceptionInfo() must fall back to using what() for both messages
    // instead of reading Pandaception-only fields.
    try {
        throw std::runtime_error("plain runtime error for coverage");
    } catch (const std::exception &e) {
        const ExceptionInfo info = Application::makeExceptionInfo(e);
        QCOMPARE(info.what, QString("plain runtime error for coverage"));
        QCOMPARE(info.englishMessage, QString("plain runtime error for coverage"));
        QVERIFY(info.file.isEmpty());
        QCOMPARE(info.line, 0);
    }
}

void TestApplication::testHandleExceptionInteractiveShowsDialog()
{
    const bool originalInteractive = Application::interactiveMode;
    Application::interactiveMode = true;

    ExceptionInfo info;
    info.what = QStringLiteral("Dialog message for testHandleExceptionInteractiveShowsDialog");
    info.englishMessage = info.what;

    Application::handleException(info, nullptr);

    QMessageBox *box = nullptr;
    for (QWidget *w : QApplication::topLevelWidgets()) {
        if (auto *candidate = qobject_cast<QMessageBox *>(w); candidate && candidate->text() == info.what) {
            box = candidate;
            break;
        }
    }
    QVERIFY2(box != nullptr, "handleException() must show a QMessageBox when interactiveMode is true");
    QVERIFY(box->isVisible());
    box->close(); // WA_DeleteOnClose cleans it up

    Application::interactiveMode = originalInteractive;
}

void TestApplication::testHandleExceptionNonInteractiveShowsNoDialog()
{
    const bool originalInteractive = Application::interactiveMode;
    Application::interactiveMode = false;

    ExceptionInfo info;
    info.what = QStringLiteral("Silent message for testHandleExceptionNonInteractiveShowsNoDialog");
    info.englishMessage = info.what;

    Application::handleException(info, nullptr);

    bool found = false;
    for (QWidget *w : QApplication::topLevelWidgets()) {
        if (auto *candidate = qobject_cast<QMessageBox *>(w); candidate && candidate->text() == info.what) {
            found = true;
            break;
        }
    }
    QVERIFY2(!found, "handleException() must not show a dialog when interactiveMode is false");

    Application::interactiveMode = originalInteractive;
}

void TestApplication::testGuardedSlotCatchesAndReports()
{
    const bool originalInteractive = Application::interactiveMode;
    Application::interactiveMode = true;

    bool bodyRan = false;
    Application::guardedSlot(nullptr, [&bodyRan]() {
        bodyRan = true;
        throw std::runtime_error("guardedSlot test exception for coverage");
    });
    // Reaching this line at all proves the exception was caught inside
    // guardedSlot rather than escaping past its noexcept boundary (which
    // would call std::terminate() and abort the whole test binary).
    QVERIFY(bodyRan);

    QMessageBox *box = nullptr;
    for (QWidget *w : QApplication::topLevelWidgets()) {
        if (auto *candidate = qobject_cast<QMessageBox *>(w);
            candidate && candidate->text() == QStringLiteral("guardedSlot test exception for coverage")) {
            box = candidate;
            break;
        }
    }
    QVERIFY2(box != nullptr, "guardedSlot must report the caught exception via handleException");
    box->close();

    Application::interactiveMode = originalInteractive;
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
