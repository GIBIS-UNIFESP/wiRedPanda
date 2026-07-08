// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTest>

class TestExternalFilePath : public QObject
{
    Q_OBJECT

private slots:
    // resolve()
    void testResolveEmptyStoredPathReturnsUnchanged();
    void testResolveEmptyContextDirReturnsUnchanged();
    void testResolveBareFilenameAgainstContextDir();
    void testResolveRelativeSubfolderPath();
    void testResolveSameOsAbsolutePath();
    void testResolveForeignAbsolutePathForwardSlashFallsBackToFilename();
    void testResolveForeignAbsolutePathBackslashFallsBackToFilename();
    void testResolveForeignAbsolutePathMixedSlashesFallsBackToFilename();
    void testResolveNonExistentFileReturnsConstructedContextDirPath();

    // forWriting()
    void testForWritingResourcePathPortableFileUnchanged();
    void testForWritingResourcePathInMemorySnapshotUnchanged();
    void testForWritingNonResourcePathPortableFileStripsToBareFilename();
    void testForWritingNonResourcePathInMemorySnapshotUnchanged();

    // forReading()
    void testForReadingInMemorySnapshotResourceValueVerbatim();
    void testForReadingInMemorySnapshotNonResourceValueVerbatim();
    void testForReadingPortableFileResourceValueReturnsNullopt();
    void testForReadingPortableFileNonResourceValueResolves();
};
