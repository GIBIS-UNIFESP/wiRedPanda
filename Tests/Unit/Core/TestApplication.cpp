// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Core/TestApplication.h"

#include <QCoreApplication>
#include <QMessageBox>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "Tests/Common/TestUtils.h"

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

void TestApplication::testScrubbedMessageReducesPathsToBasenames()
{
    // The real throw messages that leak today, verbatim from their call sites.
    QCOMPARE(Application::scrubbedMessage("/home/torres/Documents/lab1.panda not found."),
             QString("lab1.panda not found."));
    QCOMPARE(Application::scrubbedMessage("This file does not exist: /home/torres/x.panda"),
             QString("This file does not exist: x.panda"));

    // Windows, both separators — this is the case that carries the user's real
    // name, so it matters most.
    QCOMPARE(Application::scrubbedMessage(R"(Could not open file for writing: C:\Users\Rodrigo\c.panda)"),
             QString("Could not open file for writing: c.panda"));
    QCOMPARE(Application::scrubbedMessage("Could not open file for writing: C:/Users/Rodrigo/c.panda"),
             QString("Could not open file for writing: c.panda"));

    // More than one path in a single message.
    QCOMPARE(Application::scrubbedMessage("Copy /home/a/one.panda to /home/b/two.panda"),
             QString("Copy one.panda to two.panda"));

    // A quoted path keeps its quotes; only the path inside is reduced.
    QCOMPARE(Application::scrubbedMessage(R"(File "/home/torres/w.dolphin" does not exist!)"),
             QString(R"(File "w.dolphin" does not exist!)"));
}

void TestApplication::testScrubbedMessageLeavesOrdinaryTextAlone()
{
    // Over-matching would mangle ordinary sentences, so the anchors matter as
    // much as the matching does.
    QCOMPARE(Application::scrubbedMessage(""), QString(""));
    QCOMPARE(Application::scrubbedMessage("Save file first."), QString("Save file first."));
    QCOMPARE(Application::scrubbedMessage("Use the input and/or the output"),
             QString("Use the input and/or the output"));
    QCOMPARE(Application::scrubbedMessage("Ratio is 1/2 of the total"),
             QString("Ratio is 1/2 of the total"));

    // A URL is not a filesystem path and must survive intact.
    QCOMPARE(Application::scrubbedMessage("See https://wiredpanda.org/docs for help"),
             QString("See https://wiredpanda.org/docs for help"));

    // A relative path has no home directory in it, so there is nothing to strip.
    QCOMPARE(Application::scrubbedMessage("Couldn't load pixmap: foo.svg"),
             QString("Couldn't load pixmap: foo.svg"));

    // Degenerate input must not be deleted and leave a mangled sentence.
    QCOMPARE(Application::scrubbedMessage("Root is / here"), QString("Root is / here"));
}

void TestApplication::testFingerprintForIsRepoRelative()
{
    QCOMPARE(Application::fingerprintFor("/home/torres/wiredpanda/App/Element/ICLoader.cpp", 95),
             QString("App/Element/ICLoader.cpp:95"));
    QCOMPARE(Application::fingerprintFor("/build/x/Tests/Unit/Core/TestApplication.cpp", 7),
             QString("Tests/Unit/Core/TestApplication.cpp:7"));
    QCOMPARE(Application::fingerprintFor("/src/MCP/Server/Core/MCPProcessor.cpp", 12),
             QString("MCP/Server/Core/MCPProcessor.cpp:12"));

    // An unrecognised layout still has to yield something machine-independent.
    QCOMPARE(Application::fingerprintFor("/opt/weird/Standalone.cpp", 3),
             QString("Standalone.cpp:3"));

    // The real value in this build is absolute, so the guard must actually fire here.
    const QString real = Application::fingerprintFor(__FILE__, 42);
    QCOMPARE(real, QString("Tests/Unit/Core/TestApplication.cpp:42"));
}

void TestApplication::testFingerprintForIsStableAcrossBuildMachines()
{
    // The property the fingerprint exists for. __FILE__ is absolute -- CMake passes
    // absolute source paths -- so the same throw site compiled on Linux CI, Windows CI
    // and a developer's laptop carries three different strings. If any of that leaked
    // into the key, one throw site would become one Sentry issue per platform, which is
    // precisely the fragmentation this is meant to remove.
    const QString linuxCi = Application::fingerprintFor(
        "/home/runner/work/wiRedPanda/wiRedPanda/App/Scene/Workspace.cpp", 481);
    const QString windowsCi = Application::fingerprintFor(
        R"(D:\a\wiRedPanda\wiRedPanda\App\Scene\Workspace.cpp)", 481);
    const QString devLaptop = Application::fingerprintFor(
        "/home/torres/wiredpanda-sentry/App/Scene/Workspace.cpp", 481);

    QCOMPARE(linuxCi, QString("App/Scene/Workspace.cpp:481"));
    QCOMPARE(windowsCi, linuxCi);
    QCOMPARE(devLaptop, linuxCi);

    // A different line in the same file is a different throw site, and must not merge.
    QVERIFY(Application::fingerprintFor("/x/App/Scene/Workspace.cpp", 482) != linuxCi);
}

void TestApplication::testFingerprintForIsEmptyWithoutAThrowSite()
{
    // A plain std::exception carries no file/line. The caller must then leave Sentry's
    // default grouping alone rather than fingerprinting everything onto ":0".
    QVERIFY(Application::fingerprintFor(QString(), 0).isEmpty());
    QVERIFY(Application::fingerprintFor("", 123).isEmpty());
}
