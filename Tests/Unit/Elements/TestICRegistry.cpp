// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Elements/TestICRegistry.h"

#include <algorithm>

#include <QDataStream>
#include <QFile>
#include <QRectF>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "App/Element/GraphicElements/And.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/Element/GraphicElements/Led.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Wiring/Connection.h"
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
    file.close();
    QCOMPARE(registry->cachedFileBytes(path), onDisk);

    // Now actually rewrite the file and confirm onFileChanged() (QFileSystemWatcher's
    // fileChanged signal, wired via Qt::QueuedConnection) invalidates the cache and
    // re-reads the new content -- no IC element references this file, so onFileChanged()
    // takes the "no targets" path and emits definitionChanged() synchronously after
    // invalidating, which QSignalSpy::wait() below robustly waits for.
    QSignalSpy definitionChangedSpy(registry, &ICRegistry::definitionChanged);
    writeChainedPandaFile(path, QStringLiteral("second_dep"));
    QVERIFY(definitionChangedSpy.wait());

    QVERIFY(file.open(QIODevice::ReadOnly));
    const QByteArray onDiskAfterChange = file.readAll();
    file.close();
    QVERIFY2(onDiskAfterChange != onDisk, "Rewritten fixture must actually differ from the original");
    QCOMPARE(registry->cachedFileBytes(path), onDiskAfterChange);
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

void TestICRegistry::testCreateICFromSelectionAndFlattenRestoreWiring()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    auto *source = new InputSwitch();
    auto *gate = new And();
    auto *sink = new Led();
    source->setPos(QPointF(-240, 0));
    gate->setPos(QPointF(0, 0));
    sink->setPos(QPointF(240, 0));
    scene->addItem(source);
    scene->addItem(gate);
    scene->addItem(sink);

    auto *incoming = new Connection();
    incoming->setStartPort(source->outputPort(0));
    incoming->setEndPort(gate->inputPort(0));
    scene->addItem(incoming);
    auto *outgoing = new Connection();
    outgoing->setStartPort(gate->outputPort(0));
    outgoing->setEndPort(sink->inputPort(0));
    scene->addItem(outgoing);

    auto *registry = scene->icRegistry();
    IC *ic = registry->createICFromSelection({gate});
    QVERIFY(registry->hasBlob(ic->blobName()));
    QCOMPARE(ic->inputSize(), 1);
    QCOMPARE(ic->outputSize(), 1);
    QCOMPARE(ic->inputPort(0)->connections().size(), 1);
    QCOMPARE(ic->inputPort(0)->connections().constFirst()->startPort()->graphicElement(), source);
    QCOMPARE(ic->outputPort(0)->connections().size(), 1);
    QCOMPARE(ic->outputPort(0)->connections().constFirst()->endPort()->graphicElement(), sink);

    auto elements = scene->elements();
    QVERIFY(std::none_of(elements.cbegin(), elements.cend(),
                         [](GraphicElement *element) { return element->elementType() == ElementType::And; }));

    scene->undoStack()->undo();
    QVERIFY(!registry->hasBlob(QStringLiteral("IC")));
    elements = scene->elements();
    auto *restoredGate = qobject_cast<And *>(elements.constFirst());
    for (auto *element : elements) {
        if (element->elementType() == ElementType::And) restoredGate = qobject_cast<And *>(element);
    }
    QVERIFY(restoredGate);
    QCOMPARE(restoredGate->inputPort(0)->connections().size(), 1);
    QCOMPARE(restoredGate->outputPort(0)->connections().size(), 1);

    scene->undoStack()->redo();
    IC *restoredIc = nullptr;
    for (auto *element : scene->elements()) {
        if (auto *candidate = qobject_cast<IC *>(element)) restoredIc = candidate;
    }
    QVERIFY(restoredIc);
    registry->flattenIC(restoredIc);
    elements = scene->elements();
    And *flattenedGate = nullptr;
    for (auto *element : elements) {
        if (auto *candidate = qobject_cast<And *>(element)) flattenedGate = candidate;
    }
    QVERIFY(flattenedGate);
    QCOMPARE(flattenedGate->inputPort(0)->connections().size(), 1);
    QCOMPARE(flattenedGate->inputPort(0)->connections().constFirst()->startPort()->graphicElement(), source);
    const auto gateOutputs = flattenedGate->outputPort(0)->connections();
    QCOMPARE(gateOutputs.size(), 1);
    QVERIFY(std::any_of(gateOutputs.cbegin(), gateOutputs.cend(),
                        [sink](Connection *connection) { return connection->endPort() == sink->inputPort(0); }));
    QCOMPARE(sink->inputPort(0)->connections().size(), 1);
    QCOMPARE(std::count_if(elements.cbegin(), elements.cend(), [](GraphicElement *element) {
        return element->elementType() == ElementType::Led;
    }), 1);
    QVERIFY(!registry->hasBlob(QStringLiteral("IC")));

    scene->undoStack()->undo();
    elements = scene->elements();
    QVERIFY(std::any_of(elements.cbegin(), elements.cend(),
                        [](GraphicElement *element) { return element->elementType() == ElementType::IC; }));
}

void TestICRegistry::testCreateICFromSelectionIncludesSelectedInputAndOutput()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    auto *source = new InputSwitch();
    auto *gate = new And();
    auto *sink = new Led();
    auto *outsideSource = new InputSwitch();
    auto *outsideSink = new Led();
    source->setLabel(QStringLiteral("selected source"));
    sink->setLabel(QStringLiteral("selected sink"));
    source->setPos(QPointF(-240, 0));
    gate->setPos(QPointF(0, 0));
    sink->setPos(QPointF(240, 0));
    outsideSource->setPos(QPointF(-480, 120));
    outsideSink->setPos(QPointF(480, 120));
    scene->addItem(source);
    scene->addItem(gate);
    scene->addItem(sink);
    scene->addItem(outsideSource);
    scene->addItem(outsideSink);
    const QPointF originalSourcePos = source->pos();
    const QPointF originalGatePos = gate->pos();
    const QPointF originalSinkPos = sink->pos();
    const QPointF originalOutsideSourcePos = outsideSource->pos();
    const QPointF originalOutsideSinkPos = outsideSink->pos();

    auto *inputConnection = new Connection();
    inputConnection->setStartPort(source->outputPort(0));
    inputConnection->setEndPort(gate->inputPort(0));
    scene->addItem(inputConnection);
    auto *outputConnection = new Connection();
    outputConnection->setStartPort(gate->outputPort(0));
    outputConnection->setEndPort(sink->inputPort(0));
    scene->addItem(outputConnection);
    auto *crossingInput = new Connection();
    crossingInput->setStartPort(outsideSource->outputPort(0));
    crossingInput->setEndPort(gate->inputPort(1));
    scene->addItem(crossingInput);
    auto *crossingOutput = new Connection();
    crossingOutput->setStartPort(gate->outputPort(0));
    crossingOutput->setEndPort(outsideSink->inputPort(0));
    scene->addItem(crossingOutput);

    IC *ic = scene->icRegistry()->createICFromSelection({source, gate, sink});
    const QPointF oldICPos = ic->pos();
    ic->setPos(oldICPos + QPointF(160, 80));
    const QPointF translation = ic->pos() - oldICPos;
    QCOMPARE(ic->inputSize(), 2);
    QCOMPARE(ic->outputSize(), 2);
    QCOMPARE(ic->inputPort(0)->connections().size(), 1);
    QCOMPARE(ic->inputPort(0)->connections().constFirst()->startPort()->graphicElement(), outsideSource);
    QCOMPARE(ic->inputPort(1)->connections().size(), 0);
    QCOMPARE(ic->outputPort(0)->connections().size(), 0);
    QCOMPARE(ic->outputPort(1)->connections().size(), 1);
    QCOMPARE(ic->outputPort(1)->connections().constFirst()->endPort()->graphicElement(), outsideSink);
    QCOMPARE(scene->elements().size(), 3);
    QVERIFY(scene->elements().contains(ic));

    scene->icRegistry()->flattenIC(ic);
    const auto flattenedElements = scene->elements();
    And *flattenedGate = nullptr;
    GraphicElement *flattenedSource = nullptr;
    GraphicElement *flattenedSink = nullptr;
    bool selectedInputRestored = false;
    bool selectedOutputRestored = false;
    for (auto *element : flattenedElements) {
        if (auto *candidate = qobject_cast<And *>(element)) flattenedGate = candidate;
        if (element->label() == QStringLiteral("selected source")) flattenedSource = element;
        if (element->label() == QStringLiteral("selected sink")) flattenedSink = element;
        selectedInputRestored |= element == flattenedSource;
        selectedOutputRestored |= element == flattenedSink;
    }
    QVERIFY(flattenedGate);
    QVERIFY(flattenedSource);
    QVERIFY(flattenedSink);
    QVERIFY(selectedInputRestored);
    QVERIFY(selectedOutputRestored);
    QCOMPARE(flattenedSource->pos(), originalSourcePos + translation);
    QCOMPARE(flattenedGate->pos(), originalGatePos + translation);
    QCOMPARE(flattenedSink->pos(), originalSinkPos + translation);
    QCOMPARE(outsideSource->pos(), originalOutsideSourcePos);
    QCOMPARE(outsideSink->pos(), originalOutsideSinkPos);
    QCOMPARE(flattenedGate->inputPort(0)->connections().size(), 1);
    QCOMPARE(flattenedGate->inputPort(0)->connections().constFirst()->startPort()->graphicElement()->label(),
             QStringLiteral("selected source"));
    QCOMPARE(flattenedGate->inputPort(1)->connections().size(), 1);
    QCOMPARE(flattenedGate->inputPort(1)->connections().constFirst()->startPort()->graphicElement(), outsideSource);
    const auto flattenedOutputs = flattenedGate->outputPort(0)->connections();
    QVERIFY(std::any_of(flattenedOutputs.cbegin(), flattenedOutputs.cend(),
                        [](Connection *connection) {
                            return connection->endPort()->graphicElement()->label() == QStringLiteral("selected sink");
                        }));
    QVERIFY(std::any_of(flattenedOutputs.cbegin(), flattenedOutputs.cend(),
                        [outsideSink](Connection *connection) { return connection->endPort() == outsideSink->inputPort(0); }));
}

void TestICRegistry::testFlattenICWithoutBoundaryElements()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    auto *gate = new And();
    scene->addItem(gate);

    IC *ic = scene->icRegistry()->createICFromSelection({gate});
    QCOMPARE(ic->inputSize(), 0);
    QCOMPARE(ic->outputSize(), 0);

    scene->icRegistry()->flattenIC(ic);
    const auto elements = scene->elements();
    QCOMPARE(elements.size(), 1);
    QVERIFY(qobject_cast<And *>(elements.constFirst()));
}

void TestICRegistry::testCreateICFromSelectionSpacesBoundaryElements()
{
    WorkSpace workspace;
    Scene *scene = workspace.scene();
    auto *gate = new And();
    gate->setInputSize(3);
    scene->addItem(gate);

    for (int i = 0; i < gate->inputSize(); ++i) {
        auto *source = new InputSwitch();
        source->setPos(QPointF(-320, static_cast<qreal>(i) * 96.0));
        scene->addItem(source);

        auto *connection = new Connection();
        connection->setStartPort(source->outputPort(0));
        connection->setEndPort(gate->inputPort(i));
        scene->addItem(connection);
    }

    auto *secondGate = new And();
    secondGate->setPos(QPointF(160, 0));
    scene->addItem(secondGate);

    auto *sink = new Led();
    sink->setPos(QPointF(320, 0));
    scene->addItem(sink);

    auto *outgoing = new Connection();
    outgoing->setStartPort(gate->outputPort(0));
    outgoing->setEndPort(sink->inputPort(0));
    scene->addItem(outgoing);

    auto *secondSink = new Led();
    secondSink->setPos(QPointF(480, 0));
    scene->addItem(secondSink);

    auto *secondOutgoing = new Connection();
    secondOutgoing->setStartPort(secondGate->outputPort(0));
    secondOutgoing->setEndPort(secondSink->inputPort(0));
    scene->addItem(secondOutgoing);

    IC *ic = scene->icRegistry()->createICFromSelection({gate, secondGate});
    QCOMPARE(ic->inputSize(), 3);
    QCOMPARE(ic->outputSize(), 2);

    const auto &internalElements = ic->internalElements();
    for (qsizetype i = 0; i < internalElements.size(); ++i) {
        const QRectF first = internalElements[i]->boundingRect().translated(internalElements[i]->pos());
        for (qsizetype j = i + 1; j < internalElements.size(); ++j) {
            const QRectF second = internalElements[j]->boundingRect().translated(internalElements[j]->pos());
            QVERIFY2(!first.intersects(second), "Generated IC elements overlap.");
        }
    }
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
