// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Core/TestInstallRelativePaths.h"

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
