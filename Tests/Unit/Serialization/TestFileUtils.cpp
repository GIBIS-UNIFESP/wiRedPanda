// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Serialization/TestFileUtils.h"

#include <QFile>
#include <QTemporaryDir>

#include "App/IO/FileUtils.h"
#include "Tests/Common/TestUtils.h"

void TestFileUtils::testCopyToDirEmptyPath()
{
    // copyToDir with empty source is a no-op (void return)
    QTemporaryDir tempDir;
    FileUtils::copyToDir("", tempDir.path());
    QVERIFY(true); // no crash
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
    // copyToDir with resource path (":/" prefix) is a no-op
    QTemporaryDir tempDir;
    FileUtils::copyToDir(":/nonexistent", tempDir.path());
    QVERIFY(true); // no crash
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
