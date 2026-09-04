// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/QuickShell/TestInstallRelativePaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QTemporaryDir>

#include "App/Core/InstallRelativePaths.h"

void TestInstallRelativePaths::testCandidatesFirstEntryIsAppDirCategory()
{
    const QString category = QStringLiteral("InstallRelativePathsTestCategoryA");
    const QString expectedFirst = QCoreApplication::applicationDirPath() + QStringLiteral("/") + category;

    QCOMPARE(InstallRelativePaths::candidates(category).first(), expectedFirst);
}

void TestInstallRelativePaths::testCandidatesLastEntryIsBareCategoryFallback()
{
    const QString category = QStringLiteral("InstallRelativePathsTestCategoryB");
    const QStringList result = InstallRelativePaths::candidates(category);

    QVERIFY(!result.isEmpty());
    QCOMPARE(result.last(), category);
}

void TestInstallRelativePaths::testCandidatesLinuxIncludesNativeFhsEntryInOrder()
{
#ifndef Q_OS_LINUX
    QSKIP("Linux-specific candidate list (AppImage / native FHS install)");
#else
    const QString category = QStringLiteral("InstallRelativePathsTestCategoryC");
    const QString appDir = QCoreApplication::applicationDirPath();

    const QStringList expected = {
        appDir + QStringLiteral("/") + category,
        qEnvironmentVariable("APPDIR") + QStringLiteral("/usr/share/wiredpanda/") + category,
        appDir + QStringLiteral("/../share/wiredpanda/") + category,
        category,
    };

    QCOMPARE(InstallRelativePaths::candidates(category), expected);
#endif
}

void TestInstallRelativePaths::testResolveReturnsEmptyWhenNoCandidateExists()
{
    const QString category = QStringLiteral("InstallRelativePathsTestNoSuchCategory12345");

    QCOMPARE(InstallRelativePaths::resolve(category), QString());
}

void TestInstallRelativePaths::testResolveFindsCwdFallbackCandidate()
{
    QTemporaryDir cwdDir;
    QVERIFY(cwdDir.isValid());

    const QString category = QStringLiteral("InstallRelativePathsTestCwdCategory");
    QVERIFY(QDir(cwdDir.path()).mkpath(category));

    const QString previousCwd = QDir::currentPath();
    QVERIFY(QDir::setCurrent(cwdDir.path()));

    const QString resolved = InstallRelativePaths::resolve(category);

    QVERIFY(QDir::setCurrent(previousCwd));

    QCOMPARE(resolved, category);
}

void TestInstallRelativePaths::testIsCandidateMatchesCwdFallbackByCanonicalPath()
{
    QTemporaryDir cwdDir;
    QVERIFY(cwdDir.isValid());

    const QString category = QStringLiteral("InstallRelativePathsTestIsCandidate");
    QVERIFY(QDir(cwdDir.path()).mkpath(category));

    const QString previousCwd = QDir::currentPath();
    QVERIFY(QDir::setCurrent(cwdDir.path()));

    // The bare-category CWD fallback is a *relative* candidate; isCandidate() is given the
    // absolute path a caller would actually hold, so this only passes via canonicalisation.
    const bool matched = InstallRelativePaths::isCandidate(category, QDir(cwdDir.path()).absoluteFilePath(category));

    QVERIFY(QDir::setCurrent(previousCwd));

    QVERIFY(matched);
}

void TestInstallRelativePaths::testIsCandidateMatchesEquivalentPathSpellings()
{
    QTemporaryDir cwdDir;
    QVERIFY(cwdDir.isValid());

    const QString category = QStringLiteral("InstallRelativePathsTestSpellings");
    QVERIFY(QDir(cwdDir.path()).mkpath(category));

    const QString previousCwd = QDir::currentPath();
    QVERIFY(QDir::setCurrent(cwdDir.path()));

    const QString base = QDir(cwdDir.path()).absoluteFilePath(category);
    const bool trailingDot = InstallRelativePaths::isCandidate(category, base + QStringLiteral("/."));
    const bool parentRoundTrip = InstallRelativePaths::isCandidate(category, base + QStringLiteral("/../") + category);

    QVERIFY(QDir::setCurrent(previousCwd));

    QVERIFY(trailingDot);
    QVERIFY(parentRoundTrip);
}

void TestInstallRelativePaths::testIsCandidateRejectsUnrelatedMissingAndEmptyDirs()
{
    QTemporaryDir cwdDir;
    QVERIFY(cwdDir.isValid());

    const QString category = QStringLiteral("InstallRelativePathsTestReject");
    QVERIFY(QDir(cwdDir.path()).mkpath(category));
    const QString unrelated = QStringLiteral("SomewhereElse");
    QVERIFY(QDir(cwdDir.path()).mkpath(unrelated));

    const QString previousCwd = QDir::currentPath();
    QVERIFY(QDir::setCurrent(cwdDir.path()));

    const bool unrelatedDir = InstallRelativePaths::isCandidate(category, QDir(cwdDir.path()).absoluteFilePath(unrelated));
    const bool missingDir = InstallRelativePaths::isCandidate(category, QDir(cwdDir.path()).absoluteFilePath(QStringLiteral("NoSuchDir")));
    const bool emptyPath = InstallRelativePaths::isCandidate(category, QString());

    QVERIFY(QDir::setCurrent(previousCwd));

    QVERIFY(!unrelatedDir);
    QVERIFY(!missingDir);
    QVERIFY(!emptyPath);
}
