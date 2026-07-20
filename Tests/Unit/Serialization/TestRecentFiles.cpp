// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Serialization/TestRecentFiles.h"

#include <QFile>
#include <QSignalSpy>
#include <QTemporaryFile>

#include "App/Core/Settings.h"
#include "App/IO/RecentFiles.h"
#include "Tests/Common/TestUtils.h"

void TestRecentFilesUnit::testAddRecentFile()
{
    Settings::setRecentFiles({});
    RecentFiles recentFiles;

    QTemporaryFile tempFile;
    tempFile.setAutoRemove(false);
    QVERIFY(tempFile.open());
    QString filePath = tempFile.fileName();
    tempFile.close();

    recentFiles.addRecentFile(filePath);
    QVERIFY(recentFiles.recentFiles().contains(filePath));

    QFile::remove(filePath);
    Settings::setRecentFiles({});
}

void TestRecentFilesUnit::testDuplicateFileHandling()
{
    Settings::setRecentFiles({});
    RecentFiles recentFiles;

    QTemporaryFile tempFile;
    tempFile.setAutoRemove(false);
    QVERIFY(tempFile.open());
    QString filePath = tempFile.fileName();
    tempFile.close();

    recentFiles.addRecentFile(filePath);
    recentFiles.addRecentFile(filePath);

    int count = 0;
    for (const QString &f : recentFiles.recentFiles()) {
        if (f == filePath) {
            count++;
        }
    }
    QCOMPARE(count, 1);

    QFile::remove(filePath);
    Settings::setRecentFiles({});
}

void TestRecentFilesUnit::testMaxFilesLimit()
{
    Settings::setRecentFiles({});
    RecentFiles recentFiles;

    QVector<QString> paths;
    for (int i = 0; i < RecentFiles::maxFiles + 2; i++) {
        QTemporaryFile file;
        file.setAutoRemove(false);
        QVERIFY(file.open());
        paths.append(file.fileName());
        file.close();
        recentFiles.addRecentFile(paths[i]);
    }

    QVERIFY(recentFiles.recentFiles().size() <= RecentFiles::maxFiles);

    for (const QString &path : std::as_const(paths)) {
        QFile::remove(path);
    }
    Settings::setRecentFiles({});
}

void TestRecentFilesUnit::testFileWatcherRemovesDeletedFileFromList()
{
    // RecentFiles watches every recent file so the menu updates live if it's deleted/renamed
    // outside the application. The watcher's fileChanged signal is queued, so an event loop
    // (QSignalSpy::wait()) must actually run for the handler to fire.
    Settings::setRecentFiles({});
    RecentFiles recentFiles;

    QString filePath;
    {
        // QTemporaryFile::close() doesn't release its underlying OS handle (by design, so the
        // object can still manage/auto-remove the file later) -- only destroying the object
        // does. On Windows that leftover handle would make the QFile::remove() below fail with
        // a sharing violation, so scope tempFile out before removing the file ourselves.
        QTemporaryFile tempFile;
        tempFile.setAutoRemove(false);
        QVERIFY(tempFile.open());
        filePath = tempFile.fileName();
        tempFile.close();
    }

    recentFiles.addRecentFile(filePath);
    QVERIFY(recentFiles.recentFiles().contains(filePath));

    QSignalSpy updatedSpy(&recentFiles, &RecentFiles::recentFilesUpdated);
    QVERIFY(QFile::remove(filePath));

    QVERIFY2(updatedSpy.wait(), "recentFilesUpdated() must fire once the watcher notices the deletion");
    QVERIFY(!recentFiles.recentFiles().contains(filePath));

    Settings::setRecentFiles({});
}

void TestRecentFilesUnit::testRemoveOldestFile()
{
    Settings::setRecentFiles({});
    RecentFiles recentFiles;

    QVector<QString> paths;
    for (int i = 0; i < RecentFiles::maxFiles + 1; i++) {
        QTemporaryFile file;
        file.setAutoRemove(false);
        QVERIFY(file.open());
        paths.append(file.fileName());
        file.close();
        recentFiles.addRecentFile(paths[i]);
    }

    QVERIFY(!recentFiles.recentFiles().contains(paths[0]));

    for (const QString &path : std::as_const(paths)) {
        QFile::remove(path);
    }
    Settings::setRecentFiles({});
}
