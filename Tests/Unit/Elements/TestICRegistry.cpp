// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestICRegistry.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include <QByteArray>
#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRectF>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QUndoCommand>
#include <QUndoStack>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/IC.h"
#include "App/Element/ICDefinition.h"
#include "App/IO/Serialization.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Port.h"
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
    ws.scene()->addItem(ElementFactory::buildElement(ElementType::InputSwitch));
    QFile out(path);
    QVERIFY(out.open(QIODevice::WriteOnly));
    QDataStream stream(&out);
    Serialization::writePandaHeader(stream);
    ws.save(stream);
}

// Saves a circuit of \a count input switches, each driving its own LED: an IC of \a count inputs and outputs.
void saveSwitchesToLeds(const QString &path, const int count)
{
    WorkSpace workspace;
    CircuitBuilder builder(workspace.scene());
    for (int i = 0; i < count; ++i) {
        auto *sw = ElementFactory::buildElement(ElementType::InputSwitch);
        auto *led = ElementFactory::buildElement(ElementType::Led);
        builder.add(sw, led);
        builder.connect(sw, 0, led, 0);
    }
    QCOMPARE(workspace.save(path), WorkSpace::SaveOutcome::Saved);
}

// Runs \a write, which rewrites \a path, then gives the file a modification time two seconds after its own before the
// rewrite. Qt's Windows watcher compares a file's owner, group, permissions and modification time, never its size, so a
// rewrite that keeps the time is no change there.
template <typename Write>
void rewriteLater(const QString &path, Write write)
{
    const QDateTime before = QFileInfo(path).lastModified();
    write();
    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadWrite));
    QVERIFY(file.setFileTime(before.addSecs(2), QFileDevice::FileModificationTime));
}

// Reports \a path to \a registry as the file watcher's fileChanged() does, on this thread: any reload has run when this
// returns, so a check reads the state the report left, with no wait.
void reportChanged(ICRegistry *registry, const QString &path)
{
    QVERIFY(QMetaObject::invokeMethod(registry, "onFileChanged", Qt::DirectConnection, Q_ARG(QString, path)));
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
    file.close();
    QCOMPARE(registry->cachedFileBytes(path), onDisk);

    // Now actually rewrite the file and confirm onFileChanged() (QFileSystemWatcher's
    // fileChanged signal, wired via Qt::QueuedConnection) invalidates the cache and
    // re-reads the new content -- no IC element references this file, so onFileChanged()
    // takes the "no targets" path and emits definitionChanged() synchronously after
    // invalidating, which QSignalSpy::wait() below robustly waits for.
    QSignalSpy definitionChangedSpy(registry, &ICRegistry::definitionChanged);
    rewriteLater(path, [&] { writeChainedPandaFile(path, QStringLiteral("second_dep")); });
    QVERIFY(definitionChangedSpy.wait());

    QVERIFY(file.open(QIODevice::ReadOnly));
    const QByteArray onDiskAfterChange = file.readAll();
    file.close();
    QVERIFY2(onDiskAfterChange != onDisk, "Rewritten fixture must actually differ from the original");
    QCOMPARE(registry->cachedFileBytes(path), onDiskAfterChange);
}

void TestICRegistry::testReloadUndoRestoresTheInputAndItsWire()
{
    // An IC from a file, wired to its second input. The file changes so the input goes, and the reload cuts the wire.
    // The undo brings both back in the session (checked here). Saved and reopened, the document keeps the restored
    // definition, so the file's new one does not come back (the reopen check below).
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("two.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);

    auto *ic = new IC();
    ic->loadFile(path, dir.path());
    builder.addElement(ic);
    QCOMPARE(ic->inputSize(), 2);

    auto *source = ElementFactory::buildElement(ElementType::InputSwitch);
    builder.add(source);
    builder.connect(source, 0, ic, 1);
    QCOMPARE(source->outputPort(0)->connections().size(), 1);

    // Rewrite the file with one input and report the change, as the watcher would: the reload has run when this returns.
    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    saveSwitchesToLeds(path, 1);
    reportChanged(scene->icRegistry(), path);
    QCOMPARE(definitionChangedSpy.count(), 1);

    QCOMPARE(ic->inputSize(), 1);
    QVERIFY2(source->outputPort(0)->connections().isEmpty(), "the reload must cut the wire to the removed input");

    scene->undoStack()->undo();

    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(source->outputPort(0)->connections().size(), 1);

    // The undone document saved and reopened: the IC is file-backed, so the file's new definition (one input) comes
    // back with no undo step, unless the document keeps the definition it restored. It does, as a copy with the file's hash.
    const QString savedPath = dir.filePath("undone.panda");
    QCOMPARE(workspace.save(savedPath), WorkSpace::SaveOutcome::Saved);
    WorkSpace reopened;
    reopened.load(savedPath);
    IC *reopenedIC = nullptr;
    for (auto *elm : reopened.scene()->elements()) {
        if (elm->elementType() == ElementType::IC) {
            reopenedIC = static_cast<IC *>(elm);
        }
    }
    QVERIFY(reopenedIC);
    QCOMPARE(reopenedIC->inputSize(), 2);
}

void TestICRegistry::testReloadUndoRedoRoundTrip()
{
    // The undo, then the redo of a reload, leave the IC and its cut wire as each state was.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("two.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    auto *ic = new IC();
    ic->loadFile(path, dir.path());
    builder.addElement(ic);
    auto *source = ElementFactory::buildElement(ElementType::InputSwitch);
    builder.add(source);
    builder.connect(source, 0, ic, 1);

    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    saveSwitchesToLeds(path, 1);
    reportChanged(scene->icRegistry(), path);
    QCOMPARE(definitionChangedSpy.count(), 1);
    QCOMPARE(ic->inputSize(), 1);
    QVERIFY(source->outputPort(0)->connections().isEmpty());

    scene->undoStack()->undo();
    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(source->outputPort(0)->connections().size(), 1);

    scene->undoStack()->redo();
    QCOMPARE(ic->inputSize(), 1);
    QVERIFY(source->outputPort(0)->connections().isEmpty());
}

void TestICRegistry::testReloadThatAddsAnInputUndoesCleanly()
{
    // A reload that adds an input cuts nothing; its undo removes the input and keeps the wire.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("grow.panda");
    saveSwitchesToLeds(path, 1);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    auto *ic = new IC();
    ic->loadFile(path, dir.path());
    builder.addElement(ic);
    auto *source = ElementFactory::buildElement(ElementType::InputSwitch);
    builder.add(source);
    builder.connect(source, 0, ic, 0);

    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    saveSwitchesToLeds(path, 2);
    reportChanged(scene->icRegistry(), path);
    QCOMPARE(definitionChangedSpy.count(), 1);
    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(source->outputPort(0)->connections().size(), 1);

    // The undo restores the definition the reload replaced, not the file's current one.
    scene->undoStack()->undo();
    QCOMPARE(ic->inputSize(), 1);
    QCOMPARE(source->outputPort(0)->connections().size(), 1);
}

void TestICRegistry::testTwoReloadsUndoInOrder()
{
    // Two reloads of one file, shrinking then growing, each undone in turn.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("twice.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    auto *ic = new IC();
    ic->loadFile(path, dir.path());
    builder.addElement(ic);
    auto *source = ElementFactory::buildElement(ElementType::InputSwitch);
    builder.add(source);
    builder.connect(source, 0, ic, 1);

    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    saveSwitchesToLeds(path, 1);
    reportChanged(scene->icRegistry(), path);
    QCOMPARE(definitionChangedSpy.count(), 1);
    saveSwitchesToLeds(path, 3);
    reportChanged(scene->icRegistry(), path);
    QCOMPARE(definitionChangedSpy.count(), 2);
    QCOMPARE(ic->inputSize(), 3);

    scene->undoStack()->undo();
    QCOMPARE(ic->inputSize(), 1);
    scene->undoStack()->undo();
    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(source->outputPort(0)->connections().size(), 1);
}

void TestICRegistry::testTwoInstancesOfOneFileUndoRestoreBoth()
{
    // Two instances of one file, each wired to the input the reload removes; one undo restores both.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("pair.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    auto *first = new IC();
    first->loadFile(path, dir.path());
    auto *second = new IC();
    second->loadFile(path, dir.path());
    builder.add(first, second);
    auto *firstSource = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *secondSource = ElementFactory::buildElement(ElementType::InputSwitch);
    builder.add(firstSource, secondSource);
    builder.connect(firstSource, 0, first, 1);
    builder.connect(secondSource, 0, second, 1);

    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    saveSwitchesToLeds(path, 1);
    reportChanged(scene->icRegistry(), path);
    QCOMPARE(definitionChangedSpy.count(), 1);
    QCOMPARE(first->inputSize(), 1);
    QCOMPARE(second->inputSize(), 1);
    QVERIFY(firstSource->outputPort(0)->connections().isEmpty());
    QVERIFY(secondSource->outputPort(0)->connections().isEmpty());

    scene->undoStack()->undo();
    QCOMPARE(first->inputSize(), 2);
    QCOMPARE(second->inputSize(), 2);
    QCOMPARE(firstSource->outputPort(0)->connections().size(), 1);
    QCOMPARE(secondSource->outputPort(0)->connections().size(), 1);
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
    reportChanged(registry, path);

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

void TestICRegistry::testCreateEmbeddedICLeavesNothingWhenItsBlobFailsToLoad()
{
    // The MCP server's instantiate_ic embeds whatever file it is given: bytes that do not load throw, and must leave
    // no blob registered, no step on the undo stack, and the stack able to take the next command as a step of its own.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    ICRegistry *registry = scene->icRegistry();
    QUndoStack *undoStack = scene->undoStack();

    const QByteArray broken = QByteArrayLiteral("WPCF not a circuit");
    QVERIFY_THROWS_EXCEPTION(Pandaception, registry->createEmbeddedIC("broken", broken, dir.path()));

    QVERIFY(!registry->hasBlob("broken"));
    QVERIFY(scene->elements().isEmpty());
    QCOMPARE(undoStack->count(), 0);

    const QString leafPath = dir.filePath("leaf.panda");
    saveMinimalCircuit(leafPath);
    QFile leafFile(leafPath);
    QVERIFY(leafFile.open(QIODevice::ReadOnly));
    IC *ic = registry->createEmbeddedIC("leaf", leafFile.readAll(), dir.path());
    QVERIFY(ic);
    QCOMPARE(undoStack->count(), 1);
    QCOMPARE(undoStack->index(), 1);
    undoStack->undo();
    QVERIFY(!scene->elements().contains(ic));
    QVERIFY(!registry->hasBlob("leaf"));
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

void TestICRegistry::testExtractToFileThrowsWhenCommitFails()
{
#ifdef Q_OS_WIN
    QSKIP("RLIMIT_FSIZE (used to force a deferred QSaveFile write failure) has no Windows equivalent");
#else
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    WorkSpace workspace;
    ICRegistry *registry = workspace.scene()->icRegistry();
    // Bigger than ScopedTinyFsizeLimit's cap, so the write is guaranteed to force
    // QSaveFile::commit() to fail rather than open() (already covered above).
    registry->setBlob("some_blob", QByteArray(64, 'x'));

    const QString path = dir.filePath("out.panda");

    bool threw = false;
    {
        TestUtils::ScopedTinyFsizeLimit tinyLimit;
        try {
            registry->extractToFile("some_blob", path);
        } catch (...) {
            threw = true;
        }
    }
    QVERIFY2(threw, "extractToFile() must throw when commit() fails");
    QVERIFY2(!QFile::exists(path), "A failed commit() must not leave a partial file behind");
#endif
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

namespace {
// Builds a file-backed IC of \a path in \a scene and returns it, added to the scene.
IC *addFileIC(CircuitBuilder &builder, const QString &path, const QString &contextDir)
{
    auto *ic = new IC();
    ic->loadFile(path, contextDir);
    builder.addElement(ic);
    return ic;
}
} // namespace

void TestICRegistry::testIdenticalRewriteAddsNoUndoStep()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("same.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());

    saveSwitchesToLeds(path, 2); // the same bytes again
    reportChanged(scene->icRegistry(), path);
    QCOMPARE(scene->undoStack()->count(), 0);
    QCOMPARE(ic->inputSize(), 2);
}

void TestICRegistry::testTouchOnlyAddsNoUndoStep()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("touched.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadWrite));
    QVERIFY(file.setFileTime(QFileInfo(path).lastModified().addSecs(2), QFileDevice::FileModificationTime));
    file.close();
    reportChanged(scene->icRegistry(), path);
    QCOMPARE(scene->undoStack()->count(), 0);
    QCOMPARE(ic->inputSize(), 2);
}

void TestICRegistry::testRenameOverReloadsInstance()
{
    // An editor's save: the new bytes go to a temporary file, which is renamed over the original.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("renamed.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());

    // The saved file carries a time two seconds after the original's, as a later save does: the watcher compares times.
    const QString temporary = dir.filePath("renamed-saving.panda");
    saveSwitchesToLeds(temporary, 1);
    QFile saved(temporary);
    QVERIFY(saved.open(QIODevice::ReadWrite));
    QVERIFY(saved.setFileTime(QFileInfo(path).lastModified().addSecs(2), QFileDevice::FileModificationTime));
    saved.close();
    QVERIFY(QFile::remove(path));
    QVERIFY(QFile::rename(temporary, path));
    QVERIFY2(TestUtils::waitFor([&] { return ic->inputSize() == 1; }),
             "the rename over the file must reach the instance");
    QCOMPARE(scene->undoStack()->count(), 1);
}

void TestICRegistry::testDeletedFileKeepsInstanceAndStack()
{
    // The file is deleted, then made again with one input: the instance keeps its definition meanwhile, and the new
    // file reaches it.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("deleted.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());

    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    QVERIFY(QFile::remove(path));
    QVERIFY(definitionChangedSpy.wait());
    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(scene->undoStack()->count(), 0);

    saveSwitchesToLeds(path, 1);
    const bool reloaded = TestUtils::waitFor([&] { return ic->inputSize() == 1; });
    QCOMPARE(reloaded, true);
    QCOMPARE(scene->undoStack()->count(), 1);
}

// --- Edge cases of a reload and its undo ---

namespace {

// Writes \a bytes to \a path, replacing what it holds.
void writeBytes(const QString &path, const QByteArray &bytes)
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    QCOMPARE(file.write(bytes), bytes.size());
}

} // namespace

void TestICRegistry::testCorruptFileKeepsInstanceAndStack()
{
    // A file that does not load: the instance keeps its definition, no undo step is pushed, and a good file after it
    // reloads.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("corrupt.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());
    auto *source = ElementFactory::buildElement(ElementType::InputSwitch);
    builder.add(source);
    builder.connect(source, 0, ic, 1);

    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    writeBytes(path, QByteArray("not a circuit at all"));
    reportChanged(scene->icRegistry(), path);
    QCOMPARE(definitionChangedSpy.count(), 1);
    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(source->outputPort(0)->connections().size(), 1);
    QCOMPARE(scene->undoStack()->count(), 0);

    saveSwitchesToLeds(path, 1);
    reportChanged(scene->icRegistry(), path);
    QCOMPARE(definitionChangedSpy.count(), 2);
    QCOMPARE(ic->inputSize(), 1);
}

void TestICRegistry::testCorruptElementsKeepInstanceAndDropItsDefinition()
{
    // A file whose header reads but whose elements do not: the reload throws after its definition is parsed. The instance
    // keeps its inputs and its preview, and holds no definition of the bytes it failed on. The budget is 0 for the check,
    // so a definition nothing holds is parsed again, and one the instance held would not be.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("bad-elements.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());
    QVERIFY(!ic->preview().isNull());

    const QByteArray badBytes = buildBlobBytes({}, QByteArray("\x01\x02"));
    writeBytes(path, badBytes);
    const qsizetype budget = ICDefinition::budget();
    ICDefinition::setBudget(0);
    reportChanged(scene->icRegistry(), path);

    QCOMPARE(ic->inputSize(), 2);
    QVERIFY(!ic->preview().isNull());
    const int parses = ICDefinition::parseCount();
    QVERIFY(ICDefinition::acquire(path, badBytes) != nullptr);
    QCOMPARE(ICDefinition::parseCount() - parses, 1);
    ICDefinition::setBudget(budget);
}

void TestICRegistry::testEmptiedFileKeepsInstanceAndStack()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("emptied.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());

    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    writeBytes(path, QByteArray());
    reportChanged(scene->icRegistry(), path);
    QCOMPARE(definitionChangedSpy.count(), 1);
    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(scene->undoStack()->count(), 0);
}

void TestICRegistry::testLockedFileKeepsInstanceAndStack()
{
    // A file another program holds open without sharing it: the reload cannot read it, so the instance keeps its
    // definition and no undo step is pushed.
#ifdef Q_OS_WIN
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("locked.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());

    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    saveSwitchesToLeds(path, 1);
    const HANDLE lock = CreateFileW(reinterpret_cast<const wchar_t *>(path.utf16()), GENERIC_READ,
                                    0 /* no sharing */, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    QVERIFY(lock != INVALID_HANDLE_VALUE);
    reportChanged(scene->icRegistry(), path);
    QCOMPARE(definitionChangedSpy.count(), 1);
    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(scene->undoStack()->count(), 0);
    CloseHandle(lock);
#else
    QSKIP("An exclusive lock on a file is a Windows case; the other platforms' unreadable file is not simulated");
#endif
}

void TestICRegistry::testTwoWritesInARowSettleToTheLast()
{
    // Two saves in quick succession: whatever the watcher reports, the instance ends on the last file, and undoing
    // every step returns to the first state, with its wire.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("burst.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());
    auto *source = ElementFactory::buildElement(ElementType::InputSwitch);
    builder.add(source);
    builder.connect(source, 0, ic, 1);

    rewriteLater(path, [&] { saveSwitchesToLeds(path, 1); });
    rewriteLater(path, [&] { saveSwitchesToLeds(path, 3); });
    QVERIFY(TestUtils::waitFor([&] { return ic->inputSize() == 3; }));
    QCOMPARE(ic->inputSize(), 3);

    while (scene->undoStack()->canUndo()) {
        scene->undoStack()->undo();
    }
    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(source->outputPort(0)->connections().size(), 1);
}

void TestICRegistry::testChangedBackToItsOriginalUndoesToIt()
{
    // The file goes from two inputs to one and back to two: undoing both reloads returns to the first state.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("back.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());
    auto *source = ElementFactory::buildElement(ElementType::InputSwitch);
    builder.add(source);
    builder.connect(source, 0, ic, 1);

    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    saveSwitchesToLeds(path, 1);
    reportChanged(scene->icRegistry(), path);
    saveSwitchesToLeds(path, 2);
    reportChanged(scene->icRegistry(), path);
    QCOMPARE(definitionChangedSpy.count(), 2);
    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(scene->undoStack()->count(), 2);

    scene->undoStack()->undo();
    scene->undoStack()->undo();
    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(source->outputPort(0)->connections().size(), 1);
}

void TestICRegistry::testChangeAfterUndoDiscardsTheRedo()
{
    // Undo a reload, then the file changes again: the undone reload is gone, and undoing the new one returns to the
    // state the undo left.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("after.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());
    auto *source = ElementFactory::buildElement(ElementType::InputSwitch);
    builder.add(source);
    builder.connect(source, 0, ic, 1);

    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    saveSwitchesToLeds(path, 1);
    reportChanged(scene->icRegistry(), path);
    scene->undoStack()->undo();
    QCOMPARE(ic->inputSize(), 2);

    saveSwitchesToLeds(path, 3);
    reportChanged(scene->icRegistry(), path);
    QCOMPARE(definitionChangedSpy.count(), 2);
    QCOMPARE(ic->inputSize(), 3);
    QCOMPARE(scene->undoStack()->count(), 1);

    scene->undoStack()->undo();
    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(source->outputPort(0)->connections().size(), 1);
}

void TestICRegistry::testOutputWireCutAndRestoredByUndo()
{
    // The reload removes an output as well as an input: the wire from the removed output is cut, and its undo
    // restores it.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("outs.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());
    auto *led = ElementFactory::buildElement(ElementType::Led);
    builder.add(led);
    builder.connect(ic, 1, led, 0);
    QCOMPARE(led->inputPort(0)->connections().size(), 1);

    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    saveSwitchesToLeds(path, 1);
    reportChanged(scene->icRegistry(), path);
    QCOMPARE(ic->outputSize(), 1);
    QVERIFY(led->inputPort(0)->connections().isEmpty());

    scene->undoStack()->undo();
    QCOMPARE(ic->outputSize(), 2);
    QCOMPARE(led->inputPort(0)->connections().size(), 1);
}

void TestICRegistry::testUndoBeyondTheStackIsNoOp()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("empty.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());

    scene->undoStack()->undo();
    scene->undoStack()->undo();
    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(scene->undoStack()->count(), 0);
}

void TestICRegistry::testUndoOfAnUnrelatedEditLeavesTheReload()
{
    // A command pushed after the reload is undone first; the reload's undo is still the next step, and it works.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("between.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());
    auto *source = ElementFactory::buildElement(ElementType::InputSwitch);
    builder.add(source);
    builder.connect(source, 0, ic, 1);

    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    saveSwitchesToLeds(path, 1);
    reportChanged(scene->icRegistry(), path);
    scene->undoStack()->push(new QUndoCommand(QStringLiteral("unrelated")));
    QCOMPARE(scene->undoStack()->count(), 2);

    scene->undoStack()->undo();
    QCOMPARE(ic->inputSize(), 1);
    scene->undoStack()->undo();
    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(source->outputPort(0)->connections().size(), 1);
}

void TestICRegistry::testDeletedInstanceChangeLeavesNoUndoStep()
{
    // The file of an instance the scene no longer holds changes: no undo step, no crash.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("gone.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());
    scene->removeItem(ic);
    delete ic;

    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    saveSwitchesToLeds(path, 1);
    reportChanged(scene->icRegistry(), path);
    QCOMPARE(definitionChangedSpy.count(), 1);
    QCOMPARE(scene->undoStack()->count(), 0);
}

void TestICRegistry::testTwoFilesChangeAtOnce()
{
    // Two IC files change together: each instance reloads, and each reload is its own undo step.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString firstPath = dir.filePath("first.panda");
    const QString secondPath = dir.filePath("second.panda");
    saveSwitchesToLeds(firstPath, 2);
    saveSwitchesToLeds(secondPath, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *first = addFileIC(builder, firstPath, dir.path());
    IC *second = addFileIC(builder, secondPath, dir.path());
    auto *firstSource = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *secondSource = ElementFactory::buildElement(ElementType::InputSwitch);
    builder.add(firstSource, secondSource);
    builder.connect(firstSource, 0, first, 1);
    builder.connect(secondSource, 0, second, 1);

    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    saveSwitchesToLeds(firstPath, 1);
    reportChanged(scene->icRegistry(), firstPath);
    saveSwitchesToLeds(secondPath, 1);
    reportChanged(scene->icRegistry(), secondPath);
    QCOMPARE(definitionChangedSpy.count(), 2);
    QCOMPARE(first->inputSize(), 1);
    QCOMPARE(second->inputSize(), 1);
    QCOMPARE(scene->undoStack()->count(), 2);

    scene->undoStack()->undo();
    scene->undoStack()->undo();
    QCOMPARE(first->inputSize(), 2);
    QCOMPARE(second->inputSize(), 2);
    QCOMPARE(firstSource->outputPort(0)->connections().size(), 1);
    QCOMPARE(secondSource->outputPort(0)->connections().size(), 1);
}

void TestICRegistry::testEmbedThenUndoRestoresTheFileBackedIC()
{
    // Embedding a file-backed IC is an undoable change of its definition: the undo makes it file-backed again.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("embed.panda");
    saveSwitchesToLeds(path, 2);
    const QString oneInput = dir.filePath("one.panda");
    saveSwitchesToLeds(oneInput, 1);
    QFile oneFile(oneInput);
    QVERIFY(oneFile.open(QIODevice::ReadOnly));
    const QByteArray oneBytes = oneFile.readAll();

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());
    auto *source = ElementFactory::buildElement(ElementType::InputSwitch);
    builder.add(source);
    builder.connect(source, 0, ic, 1);

    QCOMPARE(scene->icRegistry()->embedICsByFile(path, oneBytes, "embedded_one"), 1);
    QVERIFY(ic->isEmbedded());
    QCOMPARE(ic->inputSize(), 1);

    scene->undoStack()->undo();
    QVERIFY(!ic->isEmbedded());
    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(source->outputPort(0)->connections().size(), 1);
}

void TestICRegistry::testExtractThenUndoRestoresTheEmbeddedIC()
{
    // Extracting an embedded IC writes its file and makes it file-backed; the undo makes it embedded again.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString leafPath = dir.filePath("leaf_source.panda");
    saveSwitchesToLeds(leafPath, 2);
    QFile leafFile(leafPath);
    QVERIFY(leafFile.open(QIODevice::ReadOnly));
    const QByteArray leafBytes = leafFile.readAll();

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = scene->icRegistry()->createEmbeddedIC("leaf", leafBytes, dir.path());
    QVERIFY(ic);
    auto *source = ElementFactory::buildElement(ElementType::InputSwitch);
    builder.add(source);
    builder.connect(source, 0, ic, 1);
    QVERIFY(ic->isEmbedded());

    const QString extracted = dir.filePath("extracted.panda");
    QCOMPARE(scene->icRegistry()->extractToFile("leaf", extracted), 1);
    QVERIFY(QFileInfo::exists(extracted));
    QVERIFY(!ic->isEmbedded());

    scene->undoStack()->undo();
    QVERIFY(ic->isEmbedded());
    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(source->outputPort(0)->connections().size(), 1);
}

void TestICRegistry::testReopenAfterGrowingUndoKeepsTheRestoredDefinition()
{
    // A growing reload, undone, then saved and reopened: the document keeps the restored definition (one input), so
    // the file's new one does not come back.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("grow_reopen.panda");
    saveSwitchesToLeds(path, 1);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());

    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    saveSwitchesToLeds(path, 2);
    reportChanged(scene->icRegistry(), path);
    scene->undoStack()->undo();

    const QString savedPath = dir.filePath("grow_reopened.panda");
    QCOMPARE(workspace.save(savedPath), WorkSpace::SaveOutcome::Saved);
    WorkSpace reopened;
    reopened.load(savedPath);
    IC *reopenedIC = nullptr;
    for (auto *elm : reopened.scene()->elements()) {
        if (elm->elementType() == ElementType::IC) {
            reopenedIC = static_cast<IC *>(elm);
        }
    }
    QVERIFY(reopenedIC);
    QCOMPARE(reopenedIC->inputSize(), 1);
    Q_UNUSED(ic)
}

void TestICRegistry::testReopenChangedFileReloadsWithAnUndo()
{
    // A document saved with a file-backed IC, and the file changed on disk before the document is opened: the file's new
    // definition comes in as a reload, and the undo of that reload returns to the definition the document kept.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("open_changed.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    addFileIC(builder, path, dir.path());

    const QString savedPath = dir.filePath("open_changed_doc.panda");
    QCOMPARE(workspace.save(savedPath), WorkSpace::SaveOutcome::Saved);
    rewriteLater(path, [&] { saveSwitchesToLeds(path, 1); });

    WorkSpace reopened;
    reopened.load(savedPath);
    IC *reopenedIC = nullptr;
    for (auto *elm : reopened.scene()->elements()) {
        if (elm->elementType() == ElementType::IC) {
            reopenedIC = static_cast<IC *>(elm);
        }
    }
    QVERIFY(reopenedIC);
    QCOMPARE(reopenedIC->inputSize(), 1);
    QVERIFY(reopened.scene()->undoStack()->canUndo());

    reopened.scene()->undoStack()->undo();
    QCOMPARE(reopenedIC->inputSize(), 2);

    reopened.scene()->undoStack()->redo();
    QCOMPARE(reopenedIC->inputSize(), 1);
}

void TestICRegistry::testReopenMissingFileKeepsTheCopy()
{
    // A document saved with a file-backed IC, and the file deleted before the document is opened: the document's copy
    // is the definition, and the IC keeps its link to the file.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("open_missing.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    addFileIC(builder, path, dir.path());

    const QString savedPath = dir.filePath("open_missing_doc.panda");
    QCOMPARE(workspace.save(savedPath), WorkSpace::SaveOutcome::Saved);
    QVERIFY(QFile::remove(path));

    WorkSpace reopened;
    reopened.load(savedPath);
    IC *reopenedIC = nullptr;
    for (auto *elm : reopened.scene()->elements()) {
        if (elm->elementType() == ElementType::IC) {
            reopenedIC = static_cast<IC *>(elm);
        }
    }
    QVERIFY(reopenedIC);
    QCOMPARE(reopenedIC->inputSize(), 2);
    QCOMPARE(reopenedIC->file(), QFileInfo(path).absoluteFilePath());
    QVERIFY(!reopened.scene()->undoStack()->canUndo());
}

void TestICRegistry::testReopenedDocumentSavesAgainByteForByte()
{
    // A document holding a file-backed IC's copy reads back and saves again with the same bytes.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("round_trip.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    addFileIC(builder, path, dir.path());

    const QString firstPath = dir.filePath("round_trip_first.panda");
    QCOMPARE(workspace.save(firstPath), WorkSpace::SaveOutcome::Saved);

    WorkSpace reopened;
    reopened.load(firstPath);
    const QString secondPath = dir.filePath("round_trip_second.panda");
    QCOMPARE(reopened.save(secondPath), WorkSpace::SaveOutcome::Saved);

    QFile first(firstPath);
    QFile second(secondPath);
    QVERIFY(first.open(QIODevice::ReadOnly));
    QVERIFY(second.open(QIODevice::ReadOnly));
    QCOMPARE(second.readAll(), first.readAll());
}

void TestICRegistry::testOpenedFileCopyStaysOutOfTheRegistry()
{
    // The copy is the IC's definition only: the registry holds no blob for it, so an embedded blob cannot take its name.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("copy_registry.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    addFileIC(builder, path, dir.path());

    const QString savedPath = dir.filePath("copy_registry_doc.panda");
    QCOMPARE(workspace.save(savedPath), WorkSpace::SaveOutcome::Saved);

    WorkSpace reopened;
    reopened.load(savedPath);
    QVERIFY(reopened.scene()->icRegistry()->blobMap().isEmpty());
}

void TestICRegistry::testFileCopyIsSavedInItsOwnEntry()
{
    // The copy goes under a metadata key of its own, by the file's name as the document references it, and the blob
    // registry holds no embedded blob for it
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("own_entry.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    addFileIC(builder, path, dir.path());

    const QString savedPath = dir.filePath("own_entry_doc.panda");
    QCOMPARE(workspace.save(savedPath), WorkSpace::SaveOutcome::Saved);

    QFile saved(savedPath);
    QVERIFY(saved.open(QIODevice::ReadOnly));
    QByteArray savedBytes = saved.readAll();
    QDataStream savedStream(&savedBytes, QIODevice::ReadOnly);
    const QMap<QString, QVariant> metadata = Serialization::readPreamble(savedStream).metadata;
    QVERIFY(!metadata.contains("embeddedICs"));
    QCOMPARE(Serialization::deserializeFileCopies(metadata).keys(), QList<QString>{"own_entry.panda"});
}

void TestICRegistry::testCopyHashIsTheFileAsTheRegistryRead()
{
    // The copy's hash is the file as the registry read it. A file changed on disk since, unseen by the session, is changed
    // when the document opens: the copy is loaded over as a reload, whose undo returns to the copy
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("read_hash.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());
    QCOMPARE(ic->inputSize(), 2);

    saveSwitchesToLeds(path, 1);
    const QString savedPath = dir.filePath("read_hash_doc.panda");
    QCOMPARE(workspace.save(savedPath), WorkSpace::SaveOutcome::Saved);

    WorkSpace reopened;
    reopened.load(savedPath);
    IC *reopenedIC = nullptr;
    for (auto *elm : reopened.scene()->elements()) {
        if (elm->elementType() == ElementType::IC) {
            reopenedIC = static_cast<IC *>(elm);
        }
    }
    QVERIFY(reopenedIC);
    QCOMPARE(reopenedIC->inputSize(), 1);
    QVERIFY(reopened.scene()->undoStack()->canUndo());
}

void TestICRegistry::testSaveWithoutUndoReopensTheNewDefinition()
{
    // The control: a reload not undone is saved as the file's definition, and reopens as the file has it.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("control.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    addFileIC(builder, path, dir.path());

    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    saveSwitchesToLeds(path, 1);
    reportChanged(scene->icRegistry(), path);

    const QString savedPath = dir.filePath("control_saved.panda");
    QCOMPARE(workspace.save(savedPath), WorkSpace::SaveOutcome::Saved);
    WorkSpace reopened;
    reopened.load(savedPath);
    IC *reopenedIC = nullptr;
    for (auto *elm : reopened.scene()->elements()) {
        if (elm->elementType() == ElementType::IC) {
            reopenedIC = static_cast<IC *>(elm);
        }
    }
    QVERIFY(reopenedIC);
    QCOMPARE(reopenedIC->inputSize(), 1);
}

void TestICRegistry::testReloadAndUndoWhileTheSimulationRuns()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("running.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());
    auto *source = ElementFactory::buildElement(ElementType::InputSwitch);
    builder.add(source);
    builder.connect(source, 0, ic, 1);
    Simulation *simulation = builder.initSimulation();
    simulation->start();

    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    saveSwitchesToLeds(path, 1);
    reportChanged(scene->icRegistry(), path);
    QCOMPARE(ic->inputSize(), 1);

    scene->undoStack()->undo();
    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(source->outputPort(0)->connections().size(), 1);

    // The simulation still runs after the undo: the restored wire drives the restored input, so a switch set on reaches
    // it through the running simulation.
    source->setOn(true);
    QVERIFY(TestUtils::waitFor([&] { return TestUtils::inputStatus(ic, 1) == TestUtils::level(true); }));
    simulation->stop();
}

void TestICRegistry::testFileInAnotherDirectoryReloads()
{
    // The IC's file is in a subdirectory; the scene's context directory is its parent. The reload reads the file where
    // it is.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QDir(dir.path()).mkpath("parts"));
    const QString path = dir.filePath("parts/sub.panda");
    saveSwitchesToLeds(path, 2);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());

    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    rewriteLater(path, [&] { saveSwitchesToLeds(path, 1); });
    QVERIFY(definitionChangedSpy.wait());
    QCOMPARE(ic->inputSize(), 1);
    scene->undoStack()->undo();
    QCOMPARE(ic->inputSize(), 2);
}

void TestICRegistry::testNestedFileChangeReachesTheOuterInstance()
{
    // An IC file that nests another IC file: a change to the nested file reaches the outer instance, whose own file
    // did not change.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString leafPath = dir.filePath("leaf.panda");
    saveSwitchesToLeds(leafPath, 2);

    WorkSpace nester;
    nester.scene()->setContextDir(dir.path());
    CircuitBuilder nesterBuilder(nester.scene());
    IC *nested = addFileIC(nesterBuilder, leafPath, dir.path());
    auto *outerSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *outerLed = ElementFactory::buildElement(ElementType::Led);
    nesterBuilder.add(outerSwitch, outerLed);
    nesterBuilder.connect(outerSwitch, 0, nested, 1);
    nesterBuilder.connect(nested, 1, outerLed, 0);
    const QString outerPath = dir.filePath("outer.panda");
    QCOMPARE(nester.save(outerPath), WorkSpace::SaveOutcome::Saved);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *outer = addFileIC(builder, outerPath, dir.path());
    QCOMPARE(outer->inputSize(), 1);
    QCOMPARE(outer->outputSize(), 1);

    // The leaf now has one input. The outer instance's inner IC is the leaf: its wire from the outer switch to the
    // second input is cut when the inner IC reloads. Shown through the outer IC's own pins once it reloads.
    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    rewriteLater(leafPath, [&] { saveSwitchesToLeds(leafPath, 1); });
    QVERIFY(definitionChangedSpy.wait());
    QCOMPARE(definitionChangedSpy.count(), 1);
    QVERIFY(scene->undoStack()->canUndo());
}

// ─── Undo probes: the redo of an embed ───

void TestICRegistry::testUndoOfANestedFilesReloadRestoresIt()
{
    // The reload of a nested file reaches the outer instance, and its undo returns the inner IC to the definition it had:
    // the outer instance's own file did not change, so its snapshot must not skip the carried files it reads
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString leafPath = dir.filePath("leaf_undo.panda");
    saveSwitchesToLeds(leafPath, 2);

    WorkSpace nester;
    nester.scene()->setContextDir(dir.path());
    CircuitBuilder nesterBuilder(nester.scene());
    IC *nested = addFileIC(nesterBuilder, leafPath, dir.path());
    auto *outerSwitch = ElementFactory::buildElement(ElementType::InputSwitch);
    auto *outerLed = ElementFactory::buildElement(ElementType::Led);
    nesterBuilder.add(outerSwitch, outerLed);
    nesterBuilder.connect(outerSwitch, 0, nested, 1);
    nesterBuilder.connect(nested, 1, outerLed, 0);
    const QString outerPath = dir.filePath("outer_undo.panda");
    QCOMPARE(nester.save(outerPath), WorkSpace::SaveOutcome::Saved);

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *outer = addFileIC(builder, outerPath, dir.path());
    const auto innerOf = [](const IC *ic) {
        for (auto *elm : ic->internalElements()) {
            if (elm->elementType() == ElementType::IC) {
                return static_cast<IC *>(elm);
            }
        }
        return static_cast<IC *>(nullptr);
    };
    QVERIFY(innerOf(outer));
    QCOMPARE(innerOf(outer)->inputSize(), 2);

    QSignalSpy definitionChangedSpy(scene->icRegistry(), &ICRegistry::definitionChanged);
    rewriteLater(leafPath, [&] { saveSwitchesToLeds(leafPath, 1); });
    QVERIFY(definitionChangedSpy.wait());
    QCOMPARE(innerOf(outer)->inputSize(), 1);

    scene->undoStack()->undo();
    QVERIFY(innerOf(outer));
    QCOMPARE(innerOf(outer)->inputSize(), 2);
}

void TestICRegistry::testEmbeddedICRedoRestoresTheBlobAndTheInstance()
{
    // An embed that succeeded is one step: its undo takes the blob and the instance away, and its redo puts both back.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString leafPath = dir.filePath("leaf_redo.panda");
    saveMinimalCircuit(leafPath);
    QFile leafFile(leafPath);
    QVERIFY(leafFile.open(QIODevice::ReadOnly));
    const QByteArray leafBytes = leafFile.readAll();

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    ICRegistry *registry = scene->icRegistry();
    QUndoStack *undoStack = scene->undoStack();

    IC *ic = registry->createEmbeddedIC("leaf_redo", leafBytes, dir.path());
    QVERIFY(ic);
    const int id = ic->id();
    undoStack->undo();
    QVERIFY(!registry->hasBlob("leaf_redo"));
    QVERIFY(scene->elements().isEmpty());

    undoStack->redo();
    QVERIFY(registry->hasBlob("leaf_redo"));
    QCOMPARE(scene->elements().size(), 1);
    auto *restored = dynamic_cast<IC *>(scene->itemById(id));
    QVERIFY(restored != nullptr);
    QVERIFY(restored->isEmbedded());
    QCOMPARE(restored->blobName(), QStringLiteral("leaf_redo"));
    QCOMPARE(undoStack->count(), 1);
    QCOMPARE(undoStack->index(), 1);
}

void TestICRegistry::testEmbedThenUndoThenRedoRestoresTheEmbeddedIC()
{
    // An embed is one step; after its undo the redo embeds the IC again, with its input count and with the wire to the
    // input the embed removed cut once more.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("embed_redo.panda");
    saveSwitchesToLeds(path, 2);
    const QString oneInput = dir.filePath("one_redo.panda");
    saveSwitchesToLeds(oneInput, 1);
    QFile oneFile(oneInput);
    QVERIFY(oneFile.open(QIODevice::ReadOnly));
    const QByteArray oneBytes = oneFile.readAll();

    WorkSpace workspace;
    Scene *scene = workspace.scene();
    scene->setContextDir(dir.path());
    CircuitBuilder builder(scene);
    IC *ic = addFileIC(builder, path, dir.path());
    auto *source = ElementFactory::buildElement(ElementType::InputSwitch);
    builder.add(source);
    builder.connect(source, 0, ic, 1);

    QCOMPARE(scene->icRegistry()->embedICsByFile(path, oneBytes, "embedded_redo"), 1);
    QVERIFY(ic->isEmbedded());
    QCOMPARE(ic->inputSize(), 1);
    QVERIFY(source->outputPort(0)->connections().isEmpty());

    scene->undoStack()->undo();
    QVERIFY(!ic->isEmbedded());
    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(source->outputPort(0)->connections().size(), 1);

    scene->undoStack()->redo();
    QVERIFY(ic->isEmbedded());
    QCOMPARE(ic->blobName(), QStringLiteral("embedded_redo"));
    QCOMPARE(ic->inputSize(), 1);
    QVERIFY(source->outputPort(0)->connections().isEmpty());
    QVERIFY(scene->icRegistry()->hasBlob("embedded_redo"));
}
