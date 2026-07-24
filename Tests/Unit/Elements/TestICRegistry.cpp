// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestICRegistry.h"

#include <QDataStream>
#include <QFile>
#include <QRectF>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/IC.h"
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

// Builds an in-memory current-version blob: header + a metadata map (dolphinFileName/sceneRect
// plus whatever \a extraMetadata adds) + \a elementBytes appended raw after the metadata (empty
// by default, meaning a zero-element circuit). Reusable for any test that only needs
// makeBlobSelfContained()/registerBlob() to parse the metadata -- those never touch element data.
QByteArray buildBlobBytes(const QMap<QString, QVariant> &extraMetadata, const QByteArray &elementBytes = {})
{
    QMap<QString, QVariant> metadata;
    metadata["dolphinFileName"] = QString();
    metadata["sceneRect"] = QRectF();
    for (auto it = extraMetadata.constBegin(); it != extraMetadata.constEnd(); ++it) {
        metadata[it.key()] = it.value();
    }

    QByteArray payload;
    QDataStream payloadStream(&payload, QIODevice::WriteOnly);
    payloadStream.setVersion(QDataStream::Qt_5_12);
    payloadStream << metadata;
    payload += elementBytes;

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    Serialization::writePandaHeader(stream);
    Serialization::writePayload(stream, payload);
    return data;
}

// Builds a blob at QVersionNumber(4, 4) -- the last version before V_4_5 introduced metadata
// support -- so VersionInfo::hasMetadata() is false and makeBlobSelfContained()/
// renameBlobReference() take their early-return guard instead of parsing a (nonexistent)
// metadata map.
QByteArray buildOldFormatBlobPredatingMetadata()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_12);
    stream << Serialization::MAGIC_HEADER_CIRCUIT;
    stream << quint32(2) << 4 << 4; // QVersionNumber(4, 4)
    stream << QString(); // dolphin filename (hasDolphinFilename is true at 4.4)
    stream << QRectF();  // scene rect (hasSceneRect is true at 4.4)
    return data;
}

// Saves a single-element circuit to \a path via the real production save path, so it's a
// genuinely loadable IC file/blob (mirrors the established "build a minimal IC via
// WorkSpace::save()" technique used throughout this sweep).
void saveMinimalCircuit(const QString &path)
{
    WorkSpace ws;
    ws.scene()->addItem(new InputSwitch());
    QFile out(path);
    QVERIFY(out.open(QIODevice::WriteOnly));
    QDataStream stream(&out);
    Serialization::writePandaHeader(stream);
    ws.save(stream);
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

void TestICRegistry::testCachedFileBytesReturnsEmptyForMissingFile()
{
    WorkSpace workspace;
    ICRegistry registry(workspace.scene());

    // Do NOT insert an empty entry into the cache on failure -- a second lookup of the same
    // missing path must still report empty rather than a stale cached "empty" placeholder.
    QVERIFY(registry.cachedFileBytes("/nonexistent/path/does/not/exist.panda").isEmpty());
    QVERIFY(registry.cachedFileBytes("/nonexistent/path/does/not/exist.panda").isEmpty());
}

void TestICRegistry::testOnFileChangedRewatchesAndEmitsWhenNoTargets()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("unwatched.panda");
    writeChainedPandaFile(path, QString());

    WorkSpace workspace;
    ICRegistry *registry = workspace.scene()->icRegistry();

    // Never call watchFile() -- the path is exists-on-disk but not in the watcher's file list,
    // which is exactly the "some OS removed the watch after a change event" re-add condition.
    QSignalSpy definitionChangedSpy(registry, &ICRegistry::definitionChanged);
    QVERIFY(QMetaObject::invokeMethod(registry, "onFileChanged",
                                      Qt::DirectConnection, Q_ARG(QString, path)));

    // No IC on the scene references this file, so onFileChanged() must emit immediately
    // without pushing an undo command.
    QCOMPARE(definitionChangedSpy.count(), 1);
    QCOMPARE(definitionChangedSpy.constFirst().at(0).toString(), path);
    QCOMPARE(workspace.scene()->undoStack()->count(), 0);
}

void TestICRegistry::testCreateEmbeddedICRegistersBlobAndAddsElement()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString leafPath = dir.filePath("leaf.panda");
    saveMinimalCircuit(leafPath);

    QFile leafFile(leafPath);
    QVERIFY(leafFile.open(QIODevice::ReadOnly));
    const QByteArray leafBytes = leafFile.readAll();

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    ICRegistry *registry = scene->icRegistry();

    IC *ic = registry->createEmbeddedIC("leaf_embedded", leafBytes, dir.path());

    QVERIFY(ic);
    QVERIFY(registry->hasBlob("leaf_embedded"));
    QCOMPARE(registry->blob("leaf_embedded"), leafBytes);
    QVERIFY(scene->elements().contains(ic));
    QVERIFY(scene->undoStack()->canUndo());

    // The whole operation (blob registration + element add) is one undo step.
    scene->undoStack()->undo();
    QVERIFY(!scene->elements().contains(ic));
    QVERIFY(!registry->hasBlob("leaf_embedded"));
}

void TestICRegistry::testEmbedICsByFileRollsBackOnLoadFailure()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("leaf.panda");
    saveMinimalCircuit(path);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    ICRegistry *registry = scene->icRegistry();

    auto *ic = new IC();
    ic->loadFile(path, dir.path());
    scene->addItem(ic);

    // Valid metadata (registerBlob()/makeBlobSelfContained() succeed) but the element section
    // is 2 stray bytes -- too short for deserialize()'s 4-byte element-type tag, so
    // ic->loadFromBlob() throws partway through the reload.
    const QByteArray corruptBytes = buildBlobBytes({}, QByteArray("\x01\x02"));

    bool threw = false;
    try {
        registry->embedICsByFile(path, corruptBytes, "myblob");
    } catch (...) {
        threw = true;
    }
    QVERIFY2(threw, "A corrupt element section must cause embedICsByFile() to throw");

    // The failed blob registration must be rolled back...
    QVERIFY(!registry->hasBlob("myblob"));
    // ...and the target IC must be left completely untouched: still file-backed, not
    // half-mutated into looking embedded (the bug this test also guards against).
    QCOMPARE(ic->file(), path);
    QVERIFY(ic->blobName().isEmpty());
}

void TestICRegistry::testExtractToFileThrowsWhenFileCannotBeOpened()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    WorkSpace workspace;
    ICRegistry *registry = workspace.scene()->icRegistry();
    registry->setBlob("some_blob", QByteArray("fake bytes"));

    // A path inside a nonexistent subdirectory: QSaveFile::open() rejects it deterministically,
    // unlike commit()-only failures which need a second OS user or root (see the tracking doc's
    // DolphinFile::save() exclusion for the same class of failure mode).
    const QString badPath = dir.filePath("no/such/dir/out.panda");

    bool threw = false;
    try {
        registry->extractToFile("some_blob", badPath);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(threw, "extractToFile() must throw when the destination file cannot be opened");
}

void TestICRegistry::testMakeBlobSelfContainedSkipsBlobPredatingMetadata()
{
    WorkSpace workspace;
    ICRegistry *registry = workspace.scene()->icRegistry();

    const QByteArray oldBytes = buildOldFormatBlobPredatingMetadata();

    bool threw = false;
    try {
        registry->registerBlob("old", oldBytes);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(!threw, "A blob predating metadata support must be accepted, not rejected");
    QVERIFY(registry->hasBlob("old"));
    // Nothing to inline (the format has no metadata to hold dependencies), so the bytes must
    // pass through completely unchanged.
    QCOMPARE(registry->blob("old"), oldBytes);
}

void TestICRegistry::testMakeBlobSelfContainedRecursesIntoAlreadyEmbeddedBlob()
{
    // A nested blob that's already embedded in the parent's metadata (as opposed to a
    // file-backed dependency) must still be walked recursively to ensure it's self-contained.
    const QByteArray nestedBytes = buildBlobBytes({});

    QMap<QString, QByteArray> nestedRegistry;
    nestedRegistry["nested_leaf"] = nestedBytes;
    QByteArray regBytes;
    QDataStream regStream(&regBytes, QIODevice::WriteOnly);
    regStream.setVersion(QDataStream::Qt_5_12);
    regStream << nestedRegistry;

    const QByteArray parentBytes = buildBlobBytes({{"embeddedICs", regBytes}});

    WorkSpace workspace;
    ICRegistry *registry = workspace.scene()->icRegistry();

    bool threw = false;
    try {
        registry->registerBlob("parent", parentBytes);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(!threw, "An already-self-contained nested blob must not be rejected");
    QVERIFY(registry->hasBlob("parent"));
    // The nested blob was already self-contained, so nothing needed re-serializing.
    QCOMPARE(registry->blob("parent"), parentBytes);
}

void TestICRegistry::testMakeBlobSelfContainedSkipsDuplicateFileBackedDependency()
{
    // "dep" is already present as an embedded IC in the parent's own metadata; a fileBackedICs
    // entry for the SAME baseName must be skipped rather than re-resolved from disk.
    const QByteArray nestedBytes = buildBlobBytes({});
    QMap<QString, QByteArray> nestedRegistry;
    nestedRegistry["dep"] = nestedBytes;
    QByteArray regBytes;
    QDataStream regStream(&regBytes, QIODevice::WriteOnly);
    regStream.setVersion(QDataStream::Qt_5_12);
    regStream << nestedRegistry;

    // "dep.panda" deliberately does not exist on disk -- the duplicate check must short-circuit
    // before the file-existence check ever runs.
    const QByteArray parentBytes = buildBlobBytes({{"embeddedICs", regBytes},
                                                    {"fileBackedICs", QStringList{"dep.panda"}}});

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    WorkSpace workspace;
    workspace.scene()->setContextDir(dir.path());
    ICRegistry *registry = workspace.scene()->icRegistry();

    bool threw = false;
    try {
        registry->registerBlob("parent", parentBytes);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(!threw, "A fileBackedICs entry duplicating an already-embedded dependency must be skipped, not fail");
    QVERIFY(registry->hasBlob("parent"));
}

void TestICRegistry::testMakeBlobSelfContainedWarnsWhenDependencyFileMissing()
{
    const QByteArray parentBytes = buildBlobBytes({{"fileBackedICs", QStringList{"missing.panda"}}});

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    WorkSpace workspace;
    workspace.scene()->setContextDir(dir.path());
    ICRegistry *registry = workspace.scene()->icRegistry();

    bool threw = false;
    try {
        registry->registerBlob("parent", parentBytes);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(!threw, "A missing dependency file must be skipped with a warning, not fail the whole blob");
    QVERIFY(registry->hasBlob("parent"));
}

void TestICRegistry::testMakeBlobSelfContainedWarnsWhenDependencyFileUnreadable()
{
#ifdef Q_OS_WIN
    QSKIP("QFile::setPermissions cannot simulate an unreadable file on Windows (uses ACLs, not Unix permission bits)");
#else
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString depPath = dir.filePath("unreadable.panda");
    writeChainedPandaFile(depPath, QString());
    QVERIFY(QFile::setPermissions(depPath, QFileDevice::Permissions()));

    const QByteArray parentBytes = buildBlobBytes({{"fileBackedICs", QStringList{"unreadable.panda"}}});

    WorkSpace workspace;
    workspace.scene()->setContextDir(dir.path());
    ICRegistry *registry = workspace.scene()->icRegistry();

    bool threw = false;
    try {
        registry->registerBlob("parent", parentBytes);
    } catch (...) {
        threw = true;
    }
    QVERIFY2(!threw, "An unreadable dependency file must be skipped with a warning, not fail the whole blob");
    QVERIFY(registry->hasBlob("parent"));

    // Restore permissions so QTemporaryDir can clean up.
    QFile::setPermissions(depPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
#endif
}

void TestICRegistry::testRenameBlobSkipsSiblingBlobPredatingMetadata()
{
    WorkSpace workspace;
    ICRegistry *registry = workspace.scene()->icRegistry();

    const QByteArray targetBytes = buildOldFormatBlobPredatingMetadata();
    const QByteArray siblingBytes = buildOldFormatBlobPredatingMetadata();
    registry->setBlob("target", targetBytes);
    registry->setBlob("sibling", siblingBytes);

    registry->renameBlob("target", "target2");

    QVERIFY(registry->hasBlob("target2"));
    QVERIFY(!registry->hasBlob("target"));
    QCOMPARE(registry->blob("target2"), targetBytes);
    // The sibling predates metadata support -- renameBlobReference() must bail out via its own
    // hasMetadata() guard rather than attempting to reinterpret bytes it can't safely parse.
    QCOMPARE(registry->blob("sibling"), siblingBytes);
}
