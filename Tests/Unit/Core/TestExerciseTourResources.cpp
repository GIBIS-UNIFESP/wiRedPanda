// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Core/TestExerciseTourResources.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include "App/Core/ExerciseTourResources.h"
#include "App/Core/Settings.h"

namespace {

void writeFile(const QString &path, const QString &contents)
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(contents.toUtf8());
}

} // namespace

void TestExerciseTourResources::testScanValidEntries()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    writeFile(dir.filePath("a.json"), R"({"id": "a", "title": "Title A", "description": "Desc A"})");
    writeFile(dir.filePath("b.json"), R"({"id": "b", "title": "Title B"})");

    const QVector<ExerciseTourResourceEntry> entries = ExerciseTourResources::scan(dir.path());

    QCOMPARE(entries.size(), 2);
    QCOMPARE(entries.at(0).id, QStringLiteral("a"));
    QCOMPARE(entries.at(0).title, QStringLiteral("Title A"));
    QCOMPARE(entries.at(0).description, QStringLiteral("Desc A"));
    QCOMPARE(entries.at(0).path, dir.path() + QStringLiteral("/a.json"));
    QCOMPARE(entries.at(1).id, QStringLiteral("b"));
}

void TestExerciseTourResources::testScanMissingDescriptionDefaultsToEmpty()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    writeFile(dir.filePath("no-description.json"), R"({"id": "x", "title": "Title X"})");

    const QVector<ExerciseTourResourceEntry> entries = ExerciseTourResources::scan(dir.path());

    QCOMPARE(entries.size(), 1);
    QVERIFY(entries.at(0).description.isEmpty());
}

void TestExerciseTourResources::testScanSkipsMalformedJson()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    writeFile(dir.filePath("broken.json"), QStringLiteral("{ this is not valid json"));
    writeFile(dir.filePath("good.json"), R"({"id": "good", "title": "Good"})");

    const QVector<ExerciseTourResourceEntry> entries = ExerciseTourResources::scan(dir.path());

    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.at(0).id, QStringLiteral("good"));
}

void TestExerciseTourResources::testScanSkipsMissingIdOrTitle()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    writeFile(dir.filePath("no-id.json"), R"({"title": "No Id"})");
    writeFile(dir.filePath("no-title.json"), R"({"id": "no-title"})");
    writeFile(dir.filePath("good.json"), R"({"id": "good", "title": "Good"})");

    const QVector<ExerciseTourResourceEntry> entries = ExerciseTourResources::scan(dir.path());

    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.at(0).id, QStringLiteral("good"));
}

void TestExerciseTourResources::testScanNonExistentDirectoryReturnsEmpty()
{
    const QVector<ExerciseTourResourceEntry> entries = ExerciseTourResources::scan(QStringLiteral("/no/such/directory/at/all"));

    QVERIFY(entries.isEmpty());
}

void TestExerciseTourResources::testTranslateFromCatalogFoundKey()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString catalogPath = dir.filePath("catalog.json");
    writeFile(catalogPath, R"({"basic-and-gate": {"title": "Construindo um Circuito com Porta AND"}})");

    const QString result = ExerciseTourResources::translateFromCatalog(catalogPath, QStringLiteral("basic-and-gate.title"),
                                                                        QStringLiteral("fallback"));

    QCOMPARE(result, QStringLiteral("Construindo um Circuito com Porta AND"));
}

void TestExerciseTourResources::testTranslateFromCatalogMissingKeyFallsBack()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString catalogPath = dir.filePath("catalog.json");
    writeFile(catalogPath, R"({"basic-and-gate": {"title": "Some Title"}})");

    const QString result = ExerciseTourResources::translateFromCatalog(catalogPath, QStringLiteral("basic-and-gate.description"),
                                                                        QStringLiteral("fallback"));

    QCOMPARE(result, QStringLiteral("fallback"));
}

void TestExerciseTourResources::testTranslateFromCatalogMalformedJsonFallsBack()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString catalogPath = dir.filePath("catalog.json");
    writeFile(catalogPath, QStringLiteral("not valid json at all"));

    const QString result = ExerciseTourResources::translateFromCatalog(catalogPath, QStringLiteral("basic-and-gate.title"),
                                                                        QStringLiteral("fallback"));

    QCOMPARE(result, QStringLiteral("fallback"));
}

void TestExerciseTourResources::testTranslateFromCatalogMissingFileFallsBack()
{
    const QString result = ExerciseTourResources::translateFromCatalog(QStringLiteral("/no/such/catalog.json"),
                                                                        QStringLiteral("basic-and-gate.title"),
                                                                        QStringLiteral("fallback"));

    QCOMPARE(result, QStringLiteral("fallback"));
}

void TestExerciseTourResources::testTranslateEnglishNeverTouchesDisk()
{
    const QString originalLang = Settings::language();
    Settings::setLanguage("en");

    const QString result = ExerciseTourResources::translate(QStringLiteral("basic-and-gate.title"), QStringLiteral("fallback"));

    Settings::setLanguage(originalLang);

    QCOMPARE(result, QStringLiteral("fallback"));
}

void TestExerciseTourResources::testTranslateEmptyLanguageNeverTouchesDisk()
{
    const QString originalLang = Settings::language();
    Settings::setLanguage("");

    const QString result = ExerciseTourResources::translate(QStringLiteral("basic-and-gate.title"), QStringLiteral("fallback"));

    Settings::setLanguage(originalLang);

    QCOMPARE(result, QStringLiteral("fallback"));
}

void TestExerciseTourResources::testMergeUniqueMergesNonCollidingEntries()
{
    QVector<ExerciseTourResourceEntry> base = {
        {"/a.json", "a", "Title A", ""},
    };
    const QVector<ExerciseTourResourceEntry> additional = {
        {"/b.json", "b", "Title B", ""},
    };

    const QVector<ExerciseTourResourceEntry> merged = ExerciseTourResources::mergeUnique(base, additional);

    QCOMPARE(merged.size(), 2);
    QCOMPARE(merged.at(0).id, QStringLiteral("a"));
    QCOMPARE(merged.at(1).id, QStringLiteral("b"));
}

void TestExerciseTourResources::testMergeUniqueSkipsCollidingId()
{
    QVector<ExerciseTourResourceEntry> base = {
        {"/builtin/a.json", "a", "Built-in A", ""},
    };
    const QVector<ExerciseTourResourceEntry> additional = {
        {"/user/a.json", "a", "User A", ""},
        {"/user/b.json", "b", "User B", ""},
    };

    const QVector<ExerciseTourResourceEntry> merged = ExerciseTourResources::mergeUnique(base, additional);

    QCOMPARE(merged.size(), 2);
    QCOMPARE(merged.at(0).id, QStringLiteral("a"));
    QCOMPARE(merged.at(0).path, QStringLiteral("/builtin/a.json")); // base entry wins, not overwritten
    QCOMPARE(merged.at(1).id, QStringLiteral("b"));
}

void TestExerciseTourResources::testManagedContentDirCreatesAndReturnsExistingPath()
{
    const QString dir = ExerciseTourResources::managedContentDir(QStringLiteral("ExerciseTourResourcesTestManagedCategory"));

    QVERIFY(!dir.isEmpty());
    QVERIFY(QDir(dir).exists());
}

void TestExerciseTourResources::testPreferredContentDirReturnsWritablePathOutsideManagedDir()
{
    const QString category = QStringLiteral("ExerciseTourResourcesTestPreferredCategory");

    const QString preferred = ExerciseTourResources::preferredContentDir(category);
    const QString managed = ExerciseTourResources::managedContentDir(category);

    QVERIFY(!preferred.isEmpty());
    QVERIFY(QDir(preferred).exists());
    QVERIFY(QFileInfo(preferred).isWritable());
    QVERIFY(!preferred.startsWith(managed)); // never resolves into the teacher/IT-managed folder
}

void TestExerciseTourResources::testResolveWritableDirFallsBackWhenCandidatesUnwritable()
{
    QTemporaryDir blockerDir;
    QVERIFY(blockerDir.isValid());
    writeFile(blockerDir.filePath("blocker.txt"), QStringLiteral("not a directory"));

    // A path component that's a plain file, not a directory, reliably fails QDir::mkpath()
    // on every platform - simulates "this candidate can't be created/written to" without
    // needing real OS permission manipulation.
    const QString unwritableCandidate = blockerDir.filePath("blocker.txt/subdir");

    QTemporaryDir fallbackParent;
    QVERIFY(fallbackParent.isValid());
    const QString fallback = fallbackParent.filePath("fallback-subdir");

    const QString result = ExerciseTourResources::resolveWritableDir({unwritableCandidate}, fallback);

    QCOMPARE(result, fallback);
    QVERIFY(QDir(fallback).exists());
    QVERIFY(QFileInfo(fallback).isWritable());
}
