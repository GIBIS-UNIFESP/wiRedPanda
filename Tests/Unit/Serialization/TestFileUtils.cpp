// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Serialization/TestFileUtils.h"

#include <QDataStream>
#include <QFile>
#include <QRectF>
#include <QTemporaryDir>

#include "App/IO/FileUtils.h"
#include "App/IO/Serialization.h"
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

void TestFileUtils::testCopyPandaDepsBasic()
{
    // copyPandaDeps takes 3 args: pandaPath, srcDir, destDir
    QTemporaryDir sourceDir;
    QTemporaryDir destDir;

    // Write a minimal file that won't parse as a real .panda — just verifies no crash
    QString sourcePanda = sourceDir.path() + "/circuit.panda";
    QFile pandaFile(sourcePanda);
    QVERIFY(pandaFile.open(QIODevice::WriteOnly));
    pandaFile.write("not a real panda file");
    pandaFile.close();

    FileUtils::copyPandaDeps(sourcePanda, sourceDir.path(), destDir.path());
    QVERIFY(true); // no crash
}

void TestFileUtils::testCopyPandaDepsRecursive()
{
    QTemporaryDir sourceDir;
    QTemporaryDir destDir;

    QString sourcePanda = sourceDir.path() + "/parent.panda";
    QFile pandaFile(sourcePanda);
    QVERIFY(pandaFile.open(QIODevice::WriteOnly));
    pandaFile.write("not a real panda file");
    pandaFile.close();

    FileUtils::copyPandaDeps(sourcePanda, sourceDir.path(), destDir.path());
    QVERIFY(true); // no crash
}

void TestFileUtils::testCopyPandaDepsNoDependencies()
{
    QTemporaryDir sourceDir;
    QTemporaryDir destDir;

    QString sourcePanda = sourceDir.path() + "/simple.panda";
    QFile pandaFile(sourcePanda);
    QVERIFY(pandaFile.open(QIODevice::WriteOnly));
    pandaFile.write("not a real panda file");
    pandaFile.close();

    FileUtils::copyPandaDeps(sourcePanda, sourceDir.path(), destDir.path());
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
    QVERIFY_EXCEPTION_THROWN(FileUtils::copyToDir(sourceFile, badDest), std::exception);
}

namespace {
// Writes a minimal .panda file with only the header + a metadata map containing
// a single fileBackedICs entry. Just enough for copyPandaDeps to traverse.
void writePandaWithFileBackedIC(const QString &path, const QString &referencedIC)
{
    QFile out(path);
    QVERIFY(out.open(QIODevice::WriteOnly));
    QDataStream stream(&out);
    Serialization::writePandaHeader(stream);

    QMap<QString, QVariant> metadata;
    metadata["dolphinFileName"] = QString();
    metadata["sceneRect"] = QRectF();
    metadata["fileBackedICs"] = QStringList{referencedIC};
    stream << metadata;
}
} // namespace

void TestFileUtils::testCopyPandaDepsTerminatesOnCircularMetadata()
{
    // Hand-craft two .panda files that reference each other in their
    // fileBackedICs metadata. Pre-fix copyPandaDeps would infinite-recurse
    // until stack overflow; the visited-set guard short-circuits the cycle.
    QTemporaryDir sourceDir;
    QTemporaryDir destDir;
    QVERIFY(sourceDir.isValid() && destDir.isValid());

    const QString aPath = sourceDir.path() + "/a.panda";
    const QString bPath = sourceDir.path() + "/b.panda";
    writePandaWithFileBackedIC(aPath, "b.panda");
    writePandaWithFileBackedIC(bPath, "a.panda");

    // Should return promptly — if it doesn't terminate the test runner kills us.
    FileUtils::copyPandaDeps(aPath, sourceDir.path(), destDir.path());
    QVERIFY(true);
}

