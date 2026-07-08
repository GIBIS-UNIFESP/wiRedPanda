// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Core/TestExternalFilePath.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include "App/IO/ExternalFilePath.h"

// ============================================================================
// resolve()
// ============================================================================

void TestExternalFilePath::testResolveEmptyStoredPathReturnsUnchanged()
{
    QCOMPARE(ExternalFilePath::resolve("", "/some/dir"), QString(""));
}

void TestExternalFilePath::testResolveEmptyContextDirReturnsUnchanged()
{
    QCOMPARE(ExternalFilePath::resolve("foo.png", ""), QString("foo.png"));
}

void TestExternalFilePath::testResolveBareFilenameAgainstContextDir()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QFile::copy(":/Components/Input/switchOff.svg", tempDir.filePath("custom.svg")));

    QCOMPARE(ExternalFilePath::resolve("custom.svg", tempDir.path()), tempDir.filePath("custom.svg"));
}

void TestExternalFilePath::testResolveRelativeSubfolderPath()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QDir(tempDir.path()).mkpath("Examples");
    QVERIFY(QFile::copy(":/Components/Input/switchOff.svg", tempDir.filePath("Examples/input.panda")));

    QCOMPARE(ExternalFilePath::resolve("Examples/input.panda", tempDir.path()), tempDir.filePath("Examples/input.panda"));
}

void TestExternalFilePath::testResolveSameOsAbsolutePath()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QFile::copy(":/Components/Input/switchOff.svg", tempDir.filePath("custom.svg")));

    // An already-absolute path resolves to itself regardless of contextDir
    QCOMPARE(ExternalFilePath::resolve(tempDir.filePath("custom.svg"), "/some/unrelated/directory"), tempDir.filePath("custom.svg"));
}

void TestExternalFilePath::testResolveForeignAbsolutePathForwardSlashFallsBackToFilename()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QFile::copy(":/Components/Input/switchOff.svg", tempDir.filePath("custom.svg")));

    QCOMPARE(ExternalFilePath::resolve("C:/Users/alice/project/custom.svg", tempDir.path()), tempDir.filePath("custom.svg"));
}

void TestExternalFilePath::testResolveForeignAbsolutePathBackslashFallsBackToFilename()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QFile::copy(":/Components/Input/switchOff.svg", tempDir.filePath("custom.svg")));

    QCOMPARE(ExternalFilePath::resolve("C:\\Users\\alice\\project\\custom.svg", tempDir.path()), tempDir.filePath("custom.svg"));
}

void TestExternalFilePath::testResolveForeignAbsolutePathMixedSlashesFallsBackToFilename()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QFile::copy(":/Components/Input/switchOff.svg", tempDir.filePath("custom.svg")));

    QCOMPARE(ExternalFilePath::resolve("C:\\Users/alice\\project/custom.svg", tempDir.path()), tempDir.filePath("custom.svg"));
}

void TestExternalFilePath::testResolveNonExistentFileReturnsConstructedContextDirPath()
{
    // Neither the path as given nor the bare-filename fallback exists on disk. resolve()
    // doesn't validate existence itself (that's the caller's job) -- it always returns a
    // candidate path: contextDir joined with the fallback's bare filename.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    QCOMPARE(ExternalFilePath::resolve("C:\\Users\\alice\\project\\nonexistent.svg", tempDir.path()),
             tempDir.filePath("nonexistent.svg"));
}

// ============================================================================
// forWriting()
// ============================================================================

void TestExternalFilePath::testForWritingResourcePathPortableFileUnchanged()
{
    QCOMPARE(ExternalFilePath::forWriting(":/Components/Input/switchOff.svg", SerializationPurpose::PortableFile),
             QString(":/Components/Input/switchOff.svg"));
}

void TestExternalFilePath::testForWritingResourcePathInMemorySnapshotUnchanged()
{
    QCOMPARE(ExternalFilePath::forWriting(":/Components/Input/switchOff.svg", SerializationPurpose::InMemorySnapshot),
             QString(":/Components/Input/switchOff.svg"));
}

void TestExternalFilePath::testForWritingNonResourcePathPortableFileStripsToBareFilename()
{
    QCOMPARE(ExternalFilePath::forWriting("/home/alice/project/custom.svg", SerializationPurpose::PortableFile),
             QString("custom.svg"));
}

void TestExternalFilePath::testForWritingNonResourcePathInMemorySnapshotUnchanged()
{
    QCOMPARE(ExternalFilePath::forWriting("/home/alice/project/custom.svg", SerializationPurpose::InMemorySnapshot),
             QString("/home/alice/project/custom.svg"));
}

// ============================================================================
// forReading()
// ============================================================================

void TestExternalFilePath::testForReadingInMemorySnapshotResourceValueVerbatim()
{
    const auto resolved = ExternalFilePath::forReading(":/Components/Input/switchOff.svg", "/some/dir", SerializationPurpose::InMemorySnapshot);
    QVERIFY(resolved.has_value());
    QCOMPARE(*resolved, QString(":/Components/Input/switchOff.svg"));
}

void TestExternalFilePath::testForReadingInMemorySnapshotNonResourceValueVerbatim()
{
    const auto resolved = ExternalFilePath::forReading("/home/alice/project/custom.svg", "/some/dir", SerializationPurpose::InMemorySnapshot);
    QVERIFY(resolved.has_value());
    QCOMPARE(*resolved, QString("/home/alice/project/custom.svg"));
}

void TestExternalFilePath::testForReadingPortableFileResourceValueReturnsNullopt()
{
    // A stale saved resource identifier from an older app version must not clobber the
    // element's current compiled-in default.
    const auto resolved = ExternalFilePath::forReading(":/Components/Input/switchOff.svg", "/some/dir", SerializationPurpose::PortableFile);
    QVERIFY(!resolved.has_value());
}

void TestExternalFilePath::testForReadingPortableFileNonResourceValueResolves()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    QVERIFY(QFile::copy(":/Components/Input/switchOff.svg", tempDir.filePath("custom.svg")));

    const auto resolved = ExternalFilePath::forReading("custom.svg", tempDir.path(), SerializationPurpose::PortableFile);
    QVERIFY(resolved.has_value());
    QCOMPARE(*resolved, tempDir.filePath("custom.svg"));
}
