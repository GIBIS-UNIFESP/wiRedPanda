// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Core/TestExerciseTourResources.h"

#include <algorithm>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
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

void TestExerciseTourResources::testResolveWritableDirEmptyCandidateSkipped()
{
    // An empty candidate (e.g. an unset environment variable feeding one of
    // InstallRelativePaths::candidates()'s entries) must be skipped outright,
    // not passed to QDir/QFileInfo.
    QTemporaryDir fallbackParent;
    QVERIFY(fallbackParent.isValid());
    const QString fallback = fallbackParent.filePath("fallback-subdir");

    const QString result = ExerciseTourResources::resolveWritableDir({QString(), QString()}, fallback);

    QCOMPARE(result, fallback);
}

void TestExerciseTourResources::testResolveWritableDirReturnsEmptyWhenNothingWorks()
{
    // Both the candidates AND the documents fallback are unwritable (a plain
    // file masquerading as a directory component reliably fails QDir::mkpath()
    // on every platform) — resolveWritableDir() must give up and return "".
    QTemporaryDir blockerDir;
    QVERIFY(blockerDir.isValid());
    writeFile(blockerDir.filePath("blocker.txt"), QStringLiteral("not a directory"));

    const QString unwritableCandidate = blockerDir.filePath("blocker.txt/subdir");
    const QString unwritableFallback = blockerDir.filePath("blocker.txt/fallback-subdir");

    const QString result = ExerciseTourResources::resolveWritableDir({unwritableCandidate}, unwritableFallback);

    QVERIFY(result.isEmpty());
}

void TestExerciseTourResources::testScanUnreadableFileSkipped()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot simulate an unreadable file on Windows (uses ACLs, not Unix permission bits)");
#else
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString unreadablePath = dir.filePath("unreadable.json");
    writeFile(unreadablePath, R"({"id": "unreadable", "title": "Unreadable"})");
    QVERIFY(QFile::setPermissions(unreadablePath, QFileDevice::WriteOwner));

    writeFile(dir.filePath("good.json"), R"({"id": "good", "title": "Good"})");

    const QVector<ExerciseTourResourceEntry> entries = ExerciseTourResources::scan(dir.path());

    // Restore permissions so QTemporaryDir can clean itself up on scope exit.
    QFile::setPermissions(unreadablePath, QFileDevice::ReadOwner | QFileDevice::WriteOwner);

    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.at(0).id, QStringLiteral("good"));
#endif
}

void TestExerciseTourResources::testManagedContentDirCreatesFreshDirectory()
{
    // Force the "doesn't exist yet, must mkpath()" branch regardless of what
    // an earlier test run on this machine left behind under QStandardPaths'
    // (persistent, not per-run) test-mode redirection.
    const QString category = QStringLiteral("ExerciseTourResourcesTestFreshManagedCategory");
    const QString expectedPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
        + QLatin1Char('/') + category;
    QDir(expectedPath).removeRecursively();
    QVERIFY(!QDir(expectedPath).exists());

    const QString dir = ExerciseTourResources::managedContentDir(category);

    QCOMPARE(dir, expectedPath);
    QVERIFY(QDir(dir).exists());

    QDir(dir).removeRecursively();
}

void TestExerciseTourResources::testDiscoverMergesInstallRelativeDir()
{
    // InstallRelativePaths::candidates()'s last entry is a bare `category`
    // path, resolved relative to the process's current working directory —
    // the "CWD dev fallback". Creating that directory for real exercises
    // discover()'s InstallRelativePaths::resolve() merge branch.
    const QString category = QStringLiteral("TestExerciseTourResourcesCwdCategory");
    QDir cwd = QDir::current();
    QDir(cwd.filePath(category)).removeRecursively();
    QVERIFY(cwd.mkpath(category));
    writeFile(cwd.filePath(category + "/entry.json"), R"({"id": "cwd-entry", "title": "CWD Entry"})");

    const QVector<ExerciseTourResourceEntry> entries = ExerciseTourResources::discover(category);

    QDir(cwd.filePath(category)).removeRecursively();

    QVERIFY(std::any_of(entries.cbegin(), entries.cend(),
                         [](const ExerciseTourResourceEntry &e) { return e.id == QStringLiteral("cwd-entry"); }));
}

void TestExerciseTourResources::testDiscoverMergesDocumentsFallbackDir()
{
    // A user who hand-drops a custom exercise/tour into
    // Documents/wiRedPanda/<category> (documented as the Documents fallback
    // when the install-relative folder isn't writable) must have it merged in.
    const QString category = QStringLiteral("TestExerciseTourResourcesDocsCategory");
    const QString docsDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
        + QStringLiteral("/wiRedPanda/") + category;
    QDir().mkpath(docsDir);
    writeFile(docsDir + "/entry.json", R"({"id": "docs-entry", "title": "Docs Entry"})");

    const QVector<ExerciseTourResourceEntry> entries = ExerciseTourResources::discover(category);

    QDir(docsDir).removeRecursively();

    QVERIFY(std::any_of(entries.cbegin(), entries.cend(),
                         [](const ExerciseTourResourceEntry &e) { return e.id == QStringLiteral("docs-entry"); }));
}

void TestExerciseTourResources::testTranslateFromCatalogNonObjectMidPathFallsBack()
{
    // "basic-and-gate.title" resolves to a plain string; walking a further
    // key segment past it ("...title.extra") must fail the mid-loop
    // isObject() check, not just the final "is this a string" check that
    // testTranslateFromCatalogMissingKeyFallsBack already covers.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString catalogPath = dir.filePath("catalog.json");
    writeFile(catalogPath, R"({"basic-and-gate": {"title": "Some Title"}})");

    const QString result = ExerciseTourResources::translateFromCatalog(
        catalogPath, QStringLiteral("basic-and-gate.title.extra"), QStringLiteral("fallback"));

    QCOMPARE(result, QStringLiteral("fallback"));
}

void TestExerciseTourResources::testTranslateNonEnglishReadsCatalog()
{
    // translate()'s actual "delegate to the embedded per-language catalog"
    // path is only reached for a real, non-empty, non-"en" language — the
    // existing translate() tests only cover the "en"/empty short-circuits.
    // pt_BR's real embedded catalog won't have this made-up key, so this
    // still exercises the real :/i18n/ExerciseTour/pt_BR.json lookup path
    // end-to-end and falls back for a key that doesn't exist in it.
    const QString originalLang = Settings::language();
    Settings::setLanguage("pt_BR");

    const QString result = ExerciseTourResources::translate(
        QStringLiteral("nonexistent-key-for-coverage.title"), QStringLiteral("fallback"));

    Settings::setLanguage(originalLang);

    QCOMPARE(result, QStringLiteral("fallback"));
}
