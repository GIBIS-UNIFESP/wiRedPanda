// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Common/TestCommon.h"

#include <cstdint>

#include <QCoreApplication>
#include <QTest>

#include "App/Core/Common.h"

// Exception Tests

void TestCommon::testPandaceptionMessage()
{
    // Test: Pandaception exception with translated message
    QString translatedMsg = "Translated error message";
    QString englishMsg = "English error message";

    Pandaception ex(translatedMsg, englishMsg);

    // Verify translated message is in what() (std::runtime_error)
    QCOMPARE(QString::fromStdString(ex.what()), translatedMsg);

    // Verify both messages are accessible
    QCOMPARE(ex.englishMessage(), englishMsg);
}

void TestCommon::testPandaceptionEnglishMessage()
{
    // Test: Pandaception keeps English message separate
    QString translatedMsg = "Mensaje de error traducido";  // Spanish
    QString englishMsg = "Error message";

    Pandaception ex(translatedMsg, englishMsg);

    // Translated message is in what()
    QCOMPARE(QString::fromStdString(ex.what()), translatedMsg);

    // English message is in englishMessage()
    QCOMPARE(ex.englishMessage(), englishMsg);

    // Both are different
    QVERIFY(QString::fromStdString(ex.what()) != ex.englishMessage());
}

void TestCommon::testSetVerbosityCategoryScale()
{
    // Regression test (F8): the old fall-through cascade appended "5 = false"
    // in every case (category five was unreachable at any verbosity) and
    // values above 5 fell into default:, silencing everything — backwards for
    // a "more verbose" request.
    const auto categoryEnabled = [](const char *name) {
        QLoggingCategory category(name);
        return category.isDebugEnabled();
    };

    Comment::setVerbosity(5);
    QVERIFY(categoryEnabled("0"));
    QVERIFY(categoryEnabled("4"));
    QVERIFY(categoryEnabled("5"));

    Comment::setVerbosity(7);
    QVERIFY(categoryEnabled("5"));

    Comment::setVerbosity(3);
    QVERIFY(categoryEnabled("2"));
    QVERIFY(!categoryEnabled("3"));
    QVERIFY(!categoryEnabled("5"));

    Comment::setVerbosity(0);
    QVERIFY(!categoryEnabled("0"));

    Comment::setVerbosity(-1);
    QVERIFY(!categoryEnabled("0"));

    // Restore the quiet test default.
    Comment::setVerbosity(-1);
}

void TestCommon::testLoggingCategoryOneRespondsToVerbosity()
{
    // Category "one" (declared alongside zero/two/three/four/five) has no
    // production qCDebug(one) call site anywhere in the codebase yet, so its
    // lazily-constructed QLoggingCategory singleton was never touched by any
    // other test. qCDebug(one) invokes the same accessor a real call site
    // would, checking (not necessarily printing) isDebugEnabled().
    Comment::setVerbosity(2);
    QVERIFY(one().isDebugEnabled());
    qCDebug(one) << "exercise the \"one\" logging category accessor";

    Comment::setVerbosity(-1);
    QVERIFY(!one().isDebugEnabled());
}

void TestCommon::testDeliberateCrashForCiDiagnosticsVerification()
{
    // TEMPORARY -- deliberately segfaults to exercise the CI crash-
    // diagnostics capture (core dump / lldb wrap / cdb wrap) end-to-end.
    // Must be removed before this branch is merged anywhere.
    //
    // A literal `int *p = nullptr; *p = 42;` is NOT reliable here: the
    // compiler is permitted to assume a provably-null dereference never
    // executes and can eliminate it entirely under -O3 (confirmed
    // empirically -- that version silently PASSED on ubuntu-24.04's
    // Release CI build while genuinely crashing in a local build using a
    // newer GCC). Deriving the address from a real runtime call the
    // optimizer cannot see through -- QCoreApplication::arguments(), whose
    // result size isn't knowable at compile time -- forces an actual,
    // unoptimizable write to a guaranteed-unmapped low address.
    const auto tinyRuntimeValue = static_cast<std::uintptr_t>(qApp->arguments().size());
    auto *p = reinterpret_cast<int *>(tinyRuntimeValue);
    *p = 42;
}
