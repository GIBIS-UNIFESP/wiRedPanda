// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Serialization/TestFileUtils.h"

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QRectF>
#include <QTemporaryDir>

#include "App/IO/FileUtils.h"
#include "App/IO/Serialization.h"
#include "Tests/Common/TestUtils.h"

namespace {

/// Files (not . / ..) currently present in \a dirPath
QStringList dirFiles(const QString &dirPath)
{
    return QDir(dirPath).entryList(QDir::Files, QDir::Name);
}

// Writes a minimal .panda file with only the header + a metadata map containing
// the given fileBackedICs entries. Just enough for copyPandaDeps to traverse.
void writePandaWithFileBackedICs(const QString &path, const QStringList &referencedICs)
{
    QFile out(path);
    QVERIFY(out.open(QIODevice::WriteOnly));
    QDataStream stream(&out);
    Serialization::writePandaHeader(stream);

    QMap<QString, QVariant> metadata;
    metadata["dolphinFileName"] = QString();
    metadata["sceneRect"] = QRectF();
    metadata["fileBackedICs"] = referencedICs;
    stream << metadata;
}

} // namespace

void TestFileUtils::testCopyToDirEmptyPath()
{
    // copyToDir with empty source is a no-op
    QTemporaryDir tempDir;
    FileUtils::copyToDir("", tempDir.path());
    QVERIFY(dirFiles(tempDir.path()).isEmpty());
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

    // copyToDir skips when the file already exists in dest — it must not
    // overwrite the existing content
    FileUtils::copyToDir(sourceFile, destDir.path());
    QFile check(destFile);
    QVERIFY(check.open(QIODevice::ReadOnly));
    QCOMPARE(check.readAll(), QByteArray("dest"));
}

void TestFileUtils::testCopyToDirResourcePath()
{
    // copyToDir with resource path (":/" prefix) is a no-op
    QTemporaryDir tempDir;
    FileUtils::copyToDir(":/nonexistent", tempDir.path());
    QVERIFY(dirFiles(tempDir.path()).isEmpty());
}

void TestFileUtils::testCopyPandaDepsBasic()
{
    QTemporaryDir sourceDir;
    QTemporaryDir destDir;

    // A file that won't parse as a real .panda has nothing to recurse into —
    // copyPandaDeps must return without copying anything
    QString sourcePanda = sourceDir.path() + "/circuit.panda";
    QFile pandaFile(sourcePanda);
    QVERIFY(pandaFile.open(QIODevice::WriteOnly));
    pandaFile.write("not a real panda file");
    pandaFile.close();

    FileUtils::copyPandaDeps(sourcePanda, sourceDir.path(), destDir.path());
    QVERIFY(dirFiles(destDir.path()).isEmpty());
}

void TestFileUtils::testCopyPandaDepsRecursive()
{
    QTemporaryDir sourceDir;
    QTemporaryDir destDir;

    // parent → child → grandchild: both dependencies must land in destDir
    writePandaWithFileBackedICs(sourceDir.path() + "/parent.panda", {"child.panda"});
    writePandaWithFileBackedICs(sourceDir.path() + "/child.panda", {"grandchild.panda"});
    writePandaWithFileBackedICs(sourceDir.path() + "/grandchild.panda", {});

    FileUtils::copyPandaDeps(sourceDir.path() + "/parent.panda", sourceDir.path(), destDir.path());
    QCOMPARE(dirFiles(destDir.path()), QStringList({"child.panda", "grandchild.panda"}));
}

void TestFileUtils::testCopyPandaDepsNoDependencies()
{
    QTemporaryDir sourceDir;
    QTemporaryDir destDir;

    // A valid .panda with an empty fileBackedICs list copies nothing
    writePandaWithFileBackedICs(sourceDir.path() + "/simple.panda", {});

    FileUtils::copyPandaDeps(sourceDir.path() + "/simple.panda", sourceDir.path(), destDir.path());
    QVERIFY(dirFiles(destDir.path()).isEmpty());
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
    writePandaWithFileBackedICs(aPath, {"b.panda"});
    writePandaWithFileBackedICs(bPath, {"a.panda"});

    // Must return promptly (the visited set breaks the cycle) and copy the
    // one dependency reachable from a.panda
    FileUtils::copyPandaDeps(aPath, sourceDir.path(), destDir.path());
    QCOMPARE(dirFiles(destDir.path()), QStringList({"a.panda", "b.panda"}));
}
