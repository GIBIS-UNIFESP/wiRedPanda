// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestICRegistry.h"

#include <QDataStream>
#include <QFile>
#include <QRectF>
#include <QTemporaryDir>

#include "App/IO/Serialization.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "Tests/Common/TestUtils.h"

namespace {
// Writes a minimal .panda file: header + a metadata map referencing \a nextDep (if non-empty)
// as a file-backed IC dependency. Mirrors TestSerialization.cpp's writePandaWithFileBackedIC —
// just enough for makeBlobSelfContained()/copyPandaFile() to traverse.
void writeChainedPandaFile(const QString &path, const QString &nextDep)
{
    QFile out(path);
    QVERIFY(out.open(QIODevice::WriteOnly));
    QDataStream stream(&out);
    Serialization::writePandaHeader(stream);

    QMap<QString, QVariant> metadata;
    metadata["dolphinFileName"] = QString();
    metadata["sceneRect"] = QRectF();
    if (!nextDep.isEmpty()) {
        metadata["fileBackedICs"] = QStringList{nextDep};
    }

    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);
    payloadStream << metadata;
    Serialization::writePayload(stream, payload);
}
} // namespace

void TestICRegistry::testICRegistration()
{
    WorkSpace workspace;
    ICRegistry registry(workspace.scene());
    QVERIFY(registry.blobMap().isEmpty());
}

void TestICRegistry::testICFileWatcher()
{
    // Registering a file for watching must not crash, and the registry's file cache — the copy
    // the watcher invalidates and re-reads on external change — must return the on-disk content.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("watched.panda");
    writeChainedPandaFile(path, QString());

    WorkSpace workspace;
    ICRegistry *registry = workspace.scene()->icRegistry();
    registry->watchFile(path);

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QByteArray onDisk = file.readAll();
    QCOMPARE(registry->cachedFileBytes(path), onDisk);
}

void TestICRegistry::testRecursiveICLoading()
{
    // A short, legitimate dependency chain (root -> dep1 -> dep2, well under the
    // nesting cap) must still resolve successfully, not be rejected.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    writeChainedPandaFile(dir.filePath("dep2.panda"), QString());
    writeChainedPandaFile(dir.filePath("dep1.panda"), "dep2.panda");
    writeChainedPandaFile(dir.filePath("root.panda"), "dep1.panda");

    WorkSpace workspace;
    workspace.scene()->setContextDir(dir.path());
    ICRegistry *registry = workspace.scene()->icRegistry();

    QFile rootFile(dir.filePath("root.panda"));
    QVERIFY(rootFile.open(QIODevice::ReadOnly));
    const QByteArray rootBytes = rootFile.readAll();

    bool threw = false;
    try {
        registry->registerBlob("root", rootBytes);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(!threw, "A short, legitimate dependency chain must not be rejected");
    QVERIFY(registry->hasBlob("root"));
}

void TestICRegistry::testMakeBlobSelfContainedRejectsDeepDependencyChain()
{
    // A long but non-cyclic chain of distinct dependency files (each real, on disk,
    // never repeated) must be rejected once it exceeds the nesting depth cap, instead
    // of recursing until the call stack is exhausted.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    constexpr int chainLength = 20; // > ICRegistry::kMaxBlobNestingDepth (16)
    for (int i = 0; i < chainLength; ++i) {
        const QString nextDep = (i + 1 < chainLength) ? QString("dep%1.panda").arg(i + 1) : QString();
        writeChainedPandaFile(dir.filePath(QString("dep%1.panda").arg(i)), nextDep);
    }

    WorkSpace workspace;
    workspace.scene()->setContextDir(dir.path());
    ICRegistry *registry = workspace.scene()->icRegistry();

    QFile firstFile(dir.filePath("dep0.panda"));
    QVERIFY(firstFile.open(QIODevice::ReadOnly));
    const QByteArray firstBytes = firstFile.readAll();

    bool threw = false;
    try {
        registry->registerBlob("dep0", firstBytes);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(threw, "A dependency chain deeper than kMaxBlobNestingDepth must be rejected, not stack-overflow");
}

void TestICRegistry::testICValidation()
{
    WorkSpace workspace;
    ICRegistry registry(workspace.scene());
    QVERIFY(!registry.hasBlob("nonexistent"));
}
