// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Common/TestRecentFiles.h"

#include <QFile>
#include <QTemporaryFile>
#include <QTest>

#include "App/Core/Settings.h"
#include "App/IO/RecentFiles.h"
#include "Tests/Common/TestUtils.h"

void TestRecentFiles::initTestCase()
{
    // Setup before all tests
    // Clear recent files from settings to start fresh
    Settings::setRecentFiles({});
}

void TestRecentFiles::cleanup()
{
    // Clean up after each test
    Settings::setRecentFiles({});
}

// ============================================================================
// RecentFiles Tests (5 tests)
// ============================================================================

void TestRecentFiles::testAddFile()
{
    // Test adding a single file to the recent files list
    RecentFiles recentFiles;

    // Create temporary file
    QTemporaryFile tempFile;
    tempFile.setAutoRemove(false);
    QVERIFY2(tempFile.open(),
             qPrintable(QString("Failed to open temporary file: %1")
                       .arg(tempFile.errorString())));
    QString filePath = tempFile.fileName();
    tempFile.close();

    // Add file to recent files
    recentFiles.addRecentFile(filePath);

    // Verify file is in the list
    QStringList files = recentFiles.recentFiles();
    QCOMPARE(files.size(), 1);
    QCOMPARE(files[0], filePath);

    // Cleanup
    QFile::remove(filePath);
}

void TestRecentFiles::testDuplicateHandling()
{
    // Test that adding a duplicate file moves it to the top (MRU order)
    RecentFiles recentFiles;

    // Create temporary files
    QTemporaryFile file1, file2;
    file1.setAutoRemove(false);
    file2.setAutoRemove(false);
    QVERIFY(file1.open());
    QVERIFY(file2.open());
    QString path1 = file1.fileName();
    QString path2 = file2.fileName();
    file1.close();
    file2.close();

    // Add files in order
    recentFiles.addRecentFile(path1);
    recentFiles.addRecentFile(path2);

    // Verify order
    QStringList files = recentFiles.recentFiles();
    QCOMPARE(files.size(), 2);
    QCOMPARE(files[0], path2);  // Most recent first
    QCOMPARE(files[1], path1);

    // Add path1 again - should move to top
    recentFiles.addRecentFile(path1);

    files = recentFiles.recentFiles();
    QCOMPARE(files.size(), 2);
    QCOMPARE(files[0], path1);  // Now at top (most recent)
    QCOMPARE(files[1], path2);

    // Cleanup
    QFile::remove(path1);
    QFile::remove(path2);
}

void TestRecentFiles::testSizeLimit()
{
    // Test that the list is limited to maximum recent files (10)
    RecentFiles recentFiles;

    // Create and add 11 files (max is 10)
    QVector<QString> paths;
    for (int i = 0; i < 11; ++i) {
        QTemporaryFile file;
        file.setAutoRemove(false);
        QVERIFY2(file.open(),
                 qPrintable(QString("Failed to open temporary file: %1")
                           .arg(file.errorString())));
        paths.append(file.fileName());
        file.close();

        // Add each file
        recentFiles.addRecentFile(paths[i]);
    }

    // Should only have 10 files (max)
    QStringList files = recentFiles.recentFiles();
    QCOMPARE(files.size(), 10);

    // The first file added (oldest) should be removed
    QVERIFY(!files.contains(paths[0]));

    // The most recent 10 files should remain
    for (int i = 1; i < 11; ++i) {
        QVERIFY(files.contains(paths[i]));
    }

    // Cleanup
    for (const QString &path : std::as_const(paths)) {
        QFile::remove(path);
    }
}

void TestRecentFiles::testNonExistentFile()
{
    // Test that adding a non-existent file is silently ignored
    RecentFiles recentFiles;

    // Create a valid temp file
    QTemporaryFile validFile;
    validFile.setAutoRemove(false);
    QVERIFY2(validFile.open(),
             qPrintable(QString("Failed to open temporary file: %1")
                       .arg(validFile.errorString())));
    QString validPath = validFile.fileName();
    validFile.close();

    // Add valid file
    recentFiles.addRecentFile(validPath);
    QCOMPARE(recentFiles.recentFiles().size(), 1);

    // Try to add non-existent file
    QString nonExistentPath = validPath + "_nonexistent";
    recentFiles.addRecentFile(nonExistentPath);

    // List should still have only 1 file (non-existent was ignored)
    QStringList files = recentFiles.recentFiles();
    QCOMPARE(files.size(), 1);
    QCOMPARE(files[0], validPath);

    // Cleanup
    QFile::remove(validPath);
}

void TestRecentFiles::testPersistence()
{
    // Test that recent files are persisted to Settings and restored
    QString path1, path2;

    // Scope 1: Create files and add them
    {
        RecentFiles recentFiles;

        // Create temporary files
        QTemporaryFile file1, file2;
        file1.setAutoRemove(false);
        file2.setAutoRemove(false);
        QVERIFY(file1.open());
        QVERIFY(file2.open());
        path1 = file1.fileName();
        path2 = file2.fileName();
        file1.close();
        file2.close();

        // Add files
        recentFiles.addRecentFile(path1);
        recentFiles.addRecentFile(path2);

        // Verify files are in list
        QStringList files = recentFiles.recentFiles();
        QCOMPARE(files.size(), 2);
    }

    // Scope 2: Create new RecentFiles object and verify persistence
    {
        RecentFiles recentFiles;

        // Get recent files - should be loaded from Settings
        QStringList files = recentFiles.recentFiles();
        QCOMPARE(files.size(), 2);

        // Verify both files are still there (ignoring exact order)
        QVERIFY(files.contains(path1));
        QVERIFY(files.contains(path2));
    }

    // Cleanup
    QFile::remove(path1);
    QFile::remove(path2);
}

