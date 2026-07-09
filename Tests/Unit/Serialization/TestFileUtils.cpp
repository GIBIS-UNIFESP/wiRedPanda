// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Serialization/TestFileUtils.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include "App/IO/FileUtils.h"
#include "Tests/Common/TestUtils.h"

void TestFileUtils::testCopyToDirEmptyPath()
{
    // An empty source path is a no-op: nothing is written into the destination directory.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    FileUtils::copyToDir("", tempDir.path());
    QVERIFY(QDir(tempDir.path()).entryList(QDir::Files | QDir::NoDotAndDotDot).isEmpty());
}

void TestFileUtils::testCopyToDirValidFile()
{
    QTemporaryDir sourceDir;
    QTemporaryDir destDir;

    QString sourceFile = sourceDir.path() + "/test.txt";
    QFile file(sourceFile);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("test content");
    file.close();

    FileUtils::copyToDir(sourceFile, destDir.path());
    QVERIFY(QFile::exists(destDir.path() + "/test.txt"));
}

void TestFileUtils::testCopyToDirFileAlreadyExists()
{
    QTemporaryDir sourceDir;
    QTemporaryDir destDir;

    QString sourceFile = sourceDir.path() + "/source.txt";
    QString destFile = destDir.path() + "/source.txt";

    QFile source(sourceFile);
    QVERIFY(source.open(QIODevice::WriteOnly));
    source.write("source");
    source.close();

    QFile dest(destFile);
    QVERIFY(dest.open(QIODevice::WriteOnly));
    dest.write("dest");
    dest.close();

    // copyToDir skips when file already exists in dest
    FileUtils::copyToDir(sourceFile, destDir.path());
    QVERIFY(QFile::exists(destFile));
}

void TestFileUtils::testCopyToDirResourcePath()
{
    // A ":/"-prefixed resource path is a no-op: resources are already embedded, so nothing
    // is copied into the destination directory.
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    FileUtils::copyToDir(":/nonexistent", tempDir.path());
    QVERIFY(QDir(tempDir.path()).entryList(QDir::Files | QDir::NoDotAndDotDot).isEmpty());
}

void TestFileUtils::testCopyToDirThrowsOnFailure()
{
    // Pre-fix QFile::copy's bool return was discarded; a failed copy left the
    // .panda referencing missing external files with no signal to the user.
    QTemporaryDir sourceDir;
    QVERIFY(sourceDir.isValid());

    const QString sourceFile = sourceDir.path() + "/payload.txt";
    QFile f(sourceFile);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("data");
    f.close();

    // Destination directory does not exist — QFile::copy returns false.
    const QString badDest = sourceDir.path() + "/no/such/subdirectory";
    QVERIFY_THROWS(std::exception, FileUtils::copyToDir(sourceFile, badDest));
}

void TestFileUtils::testFilesHaveSameContent()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const auto write = [&](const QString &path, const QByteArray &bytes) {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write(bytes);
        file.close();
    };

    const QString original          = dir.path() + "/a.panda";
    const QString identicalCopy     = dir.path() + "/copy.panda";
    const QString differentSameSize = dir.path() + "/b.panda";
    const QString differentSize     = dir.path() + "/c.panda";
    write(original, "hello world");
    write(identicalCopy, "hello world");
    write(differentSameSize, "HELLO WORLD"); // same length, different bytes
    write(differentSize, "hi");

    // The very same file on disk.
    QVERIFY(FileUtils::filesHaveSameContent(QFileInfo(original), QFileInfo(original)));
    // A different path with identical bytes — a harmless re-add, not a collision.
    QVERIFY(FileUtils::filesHaveSameContent(QFileInfo(original), QFileInfo(identicalCopy)));
    // Same size but different content — the silent-mis-bind case that must be flagged.
    QVERIFY(!FileUtils::filesHaveSameContent(QFileInfo(original), QFileInfo(differentSameSize)));
    // Different size.
    QVERIFY(!FileUtils::filesHaveSameContent(QFileInfo(original), QFileInfo(differentSize)));
}
