// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief libFuzzer harness targeting ICRegistry blob management (2% coverage).
 *
 * ICRegistry manages the lifecycle of embedded IC blobs: storage, lookup,
 * cache invalidation, and IC initialisation.  With all existing harnesses,
 * ICRegistry.cpp sits at 2% line coverage because:
 *
 *   - Blob registration (setBlob, hasBlob, blob) is only called when a panda
 *     file contains an `embeddedICs` metadata entry — the structured/raw
 *     harnesses almost never synthesise this.
 *   - initEmbeddedIC is called from Scene::deserializeElement, which is only
 *     reached through a different Scene-level code path than Serialization::
 *     deserialize uses.
 *
 * This harness directly exercises all registry paths:
 *
 *   1. setBlob / hasBlob / blob — register fuzz-controlled blob bytes under
 *      a fuzz-controlled name, then read them back.
 *
 *   2. initEmbeddedIC — create an IC element, then call initEmbeddedIC which
 *      does hasBlob → IC::loadFromBlob.  This chains into the full IC inner
 *      deserialization (processLoadedItems, loadBoundaryElement, etc.).
 *
 *   3. WorkSpace::load with embedded IC blob — build a workspace panda stream
 *      whose metadata contains the fuzz blob in `embeddedICs`, so the normal
 *      load path exercises ICRegistry::setBlob in context.
 *
 * Input layout: a whole .panda file (WPCF magic, a legacy text header, or a leading 0 byte) takes path C, where the
 * input is an IC's definition (see exerciseDefinition()).  Any other input is split by FuzzedDataProvider into a
 * blob name and blob bytes, which go through path A, and also path B when the last byte is below 170.
 */

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <memory>

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QIODevice>
#include <QMap>
#include <QMetaObject>
#include <QScopeGuard>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QUndoStack>
#include <QVariant>
#include <QVersionNumber>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/IO/Serialization.h"
#include "App/Scene/ICRegistry.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "Tests/Fuzz/FuzzCircuitDir.h"
#include "Tests/Fuzz/FuzzSeed.h"
#include "Tests/Fuzz/PandaMutator.h"

namespace {

QApplication *g_app  = nullptr;
int  g_argc = 0;
char **g_argv = nullptr;

// Where path C writes the definition's file and extracts it to; the scene's context directory. It holds the
// repository's circuits (Tests/Fuzz/FuzzCircuitDir.h), so the IC files the definition names load.
// Held until exit, which removes it
std::unique_ptr<QTemporaryDir> g_dir;

// Path C loads its input as a circuit about 25 times, in time linear in the elements it loads through its nested ICs,
// about 1.2 ms each under ASan, not in its size: a 1.7 KB file naming an ALU loads 1,781, a 33 KB register file 2,001.
// A definition loading more than this stops after its first instance, so a unit stays under about 1.5 s and reading a
// round's corpus takes a few minutes of the round, not all of it. The registry's paths do not depend on the circuit's
// size, and one load of a large circuit is fuzz_deserialize's target.
constexpr qsizetype kMaxDefinitionElements = 50;

// Valid IC blob (InputSwitch + Led) built in LLVMFuzzerInitialize.
// Used to exercise the successful path through ICRegistry::initEmbeddedIC
// → IC::loadFromBlob → IC::processLoadedItems → loadBoundaryElement.
QByteArray g_validBlob;

// Build a minimal panda stream that has one embedded IC blob in metadata,
// but no IC elements in the scene.  Exercises ICRegistry::setBlob.
QByteArray buildBlobMetadataPanda(const QString &blobName, const QByteArray &blobData)
{
    // Serialise the blob registry: QMap<QString,QByteArray> with one entry.
    QByteArray regBytes;
    {
        QDataStream rs(&regBytes, QIODevice::WriteOnly);
        rs.setVersion(QDataStream::Qt_5_12);
        QMap<QString, QByteArray> reg;
        reg.insert(blobName, blobData);
        rs << reg;
    }

    QByteArray buf;
    QDataStream s(&buf, QIODevice::WriteOnly);
    s.setVersion(QDataStream::Qt_5_12);

    // WPCF header, version 5.1
    s.writeRawData("WPCF", 4);
    s << QList<int>{5, 1};

    // Metadata map: dolphinFileName + embeddedICs (the blob registry)
    QMap<QString, QVariant> meta;
    meta.insert("dolphinFileName", QVariant(QString()));
    meta.insert("embeddedICs", QVariant(regBytes));
    s << meta;

    // No items — Serialization::deserialize uses while(!stream.atEnd()),
    // so an empty item stream is represented by writing nothing here.

    return buf;
}

// Path A: direct ICRegistry API + initEmbeddedIC
void exerciseRegistryDirect(const QString &blobName, const QByteArray &blobBytes,
                             WorkSpace &ws)
{
    ICRegistry *reg = ws.scene()->icRegistry();

    // Register fuzz bytes as the blob (exercises error paths in loadFromBlob).
    // Also register the valid blob under a different name so initEmbeddedIC
    // can succeed and exercise the full IC::processLoadedItems path.
    reg->setBlob(blobName, blobBytes);
    const QString validName = blobName + "_valid";
    reg->setBlob(validName, g_validBlob);

    if (reg->hasBlob(blobName)) {
        const QByteArray retrieved = reg->blob(blobName);
        (void)retrieved;
    }

    // initEmbeddedIC with fuzz blob (usually throws on invalid panda).
    auto *ic = static_cast<IC *>(ElementFactory::buildElement(ElementType::IC));
    try { reg->initEmbeddedIC(ic, blobName); } catch (...) {}
    delete ic;

    // initEmbeddedIC with valid blob — exercises the success path:
    // processLoadedItems → loadBoundaryElement for InputSwitch and Led.
    auto *ic2 = static_cast<IC *>(ElementFactory::buildElement(ElementType::IC));
    try { reg->initEmbeddedIC(ic2, validName); } catch (...) {}
    delete ic2;

    // uniqueBlobName: exercises the name-collision resolution loop
    const QString unique = reg->uniqueBlobName(blobName);
    (void)unique;

    // renameBlob — internally calls renameBlobReference which parses panda bytes
    if (reg->hasBlob(blobName)) {
        try { reg->renameBlob(blobName, blobName + "_renamed"); } catch (...) {}
    }

    // findICsByBlobName — searches scene elements by blob name
    const auto found = reg->findICsByBlobName(blobName + "_renamed");
    (void)found;

    // registerBlob — wraps blob in a self-contained panda (walks nested blobs)
    try { reg->registerBlob(blobName + "_reg", blobBytes); } catch (...) {}

    // clearBlobs — removes all blobs
    reg->clearBlobs();

    // removeBlob (no-op after clearBlobs, but exercises the path)
    try { reg->removeBlob(blobName + "_renamed"); } catch (...) {}
}

// Path B: WorkSpace::load with embedded blob in metadata
void exerciseViaLoad(const QString &blobName, const QByteArray &blobBytes,
                     WorkSpace &ws)
{
    const QByteArray panda = buildBlobMetadataPanda(blobName, blobBytes);
    QDataStream stream(panda);
    QVersionNumber version;
    try {
        version = Serialization::readPandaHeader(stream);
    } catch (...) {
        return;
    }
    try {
        ws.load(stream, version, /*contextDir=*/QString());
    } catch (...) {}
}

void writeFile(const QString &path, const QByteArray &bytes)
{
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(bytes);
    }
}

// Path C: a whole .panda input is an IC's definition, put through what ICController does with one: placed from its
// file, reloaded when the file changes, embedded, renamed, extracted to a file again and embedded anew, then every
// step undone and redone.  A blob that nests it and names its file as a file-backed dependency makes
// makeBlobSelfContained() and renameBlobReference() walk it; the definition cut in half is the file edited into one
// that no longer loads.
void exerciseDefinition(const QByteArray &definition)
{
    const QString dir = g_dir->path();
    const QString filePath = g_dir->filePath("def.panda");
    const QString extractedPath = g_dir->filePath("extracted.panda");
    const QString unusedPath = g_dir->filePath("unused.panda");
    const QByteArray broken = definition.left(definition.size() / 2);
    QFile::remove(extractedPath);
    writeFile(filePath, definition);
    writeFile(unusedPath, definition);

    WorkSpace ws;
    Scene *scene = ws.scene();
    scene->setContextDir(dir);
    ICRegistry *reg = scene->icRegistry();

    try { reg->registerBlob("raw", definition); } catch (...) {}
    {
        // "inner.panda" is skipped, its base name already embedded; "unreadable.panda" is a directory.
        QMap<QString, QVariant> meta;
        meta.insert("dolphinFileName", QVariant(QString()));
        meta.insert("fileBackedICs", QStringList{"def.panda", "missing.panda", "inner.panda", "unreadable.panda"});
        Serialization::serializeBlobRegistry({{"inner", definition}}, meta);
        try { reg->registerBlob("wrapper", FuzzSeed::pandaFile(meta, QList<QGraphicsItem *>())); } catch (...) {}
    }
    {
        // The definition nested 0 to 19 levels deep, past the limit of 16 at the deepest.
        QByteArray nested = definition;
        for (qsizetype depth = definition.size() % 20; depth > 0; --depth) {
            QMap<QString, QVariant> meta;
            meta.insert("dolphinFileName", QVariant(QString()));
            Serialization::serializeBlobRegistry({{QString("level%1").arg(depth), nested}}, meta);
            nested = FuzzSeed::pandaFile(meta, QList<QGraphicsItem *>());
        }
        try { reg->registerBlob("nested", nested); } catch (...) {}
    }

    auto *unplaced = new IC();
    (void)reg->initEmbeddedIC(unplaced, "missing");
    delete unplaced;

    // Two instances placed from the file, as from the palette; a definition that does not load stops here, and so
    // does one too large to load about 25 times in a unit.
    for (int i = 0; i < 2; ++i) {
        auto *ic = new IC();
        try {
            ic->loadFile(filePath, dir);
        } catch (...) {
            delete ic;
            return;
        }
        scene->addItem(ic);
        if (FuzzCircuitDir::loadedElementCount(ic->internalElements()) > kMaxDefinitionElements) {
            return;
        }
    }

    const auto fileChanged = [reg](const QString &path) {
        QMetaObject::invokeMethod(reg, "onFileChanged", Qt::DirectConnection, Q_ARG(QString, path));
    };
    (void)reg->cachedFileBytes(filePath);
    (void)reg->cachedFileBytes(g_dir->filePath("missing.panda"));
    fileChanged(filePath);
    reg->watchFile(filePath);
    fileChanged(unusedPath);
    writeFile(filePath, broken);
    fileChanged(filePath);
    writeFile(filePath, definition);
    fileChanged(filePath);
    reg->invalidate(filePath);

    const QString blobName = reg->uniqueBlobName("def");
    try { reg->embedICsByFile(unusedPath, definition, reg->uniqueBlobName("unused")); } catch (...) {}
    try { reg->embedICsByFile(filePath, broken, reg->uniqueBlobName("broken")); } catch (...) {}
    try { reg->embedICsByFile(filePath, definition, blobName); } catch (...) {}
    try { reg->renameBlob(blobName, "raw"); } catch (...) {}
    try { reg->renameBlob(blobName, "renamed"); } catch (...) {}
    (void)ICRegistry::captureSnapshot(reg->findICsByBlobName("renamed"));
    try { reg->extractToFile("renamed", g_dir->filePath("no-such-dir/extracted.panda")); } catch (...) {}
    try { reg->extractToFile("raw", g_dir->filePath("raw.panda")); } catch (...) {}
    try { reg->extractToFile("renamed", extractedPath); } catch (...) {}
    try { reg->createEmbeddedIC(reg->uniqueBlobName("created"), definition, dir); } catch (...) {}

    QUndoStack *undoStack = scene->undoStack();
    try {
        while (undoStack->canUndo()) {
            undoStack->undo();
        }
        while (undoStack->canRedo()) {
            undoStack->redo();
        }
    } catch (...) {}
}

} // namespace

// A whole .panda input takes path C: the shared structure-aware mutator works on a current-format file's decompressed
// body (Tests/Fuzz/PandaMutator.h)
extern "C" size_t LLVMFuzzerCustomMutator(uint8_t *Data, size_t Size, size_t MaxSize, unsigned int Seed)
{
    return PandaMutator::mutate(Data, Size, MaxSize, Seed);
}

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_IM_MODULES", "none");

    Comment::setVerbosity(-1);
    Application::interactiveMode = false;
    Application::renderingEnabled = false;
    Application::migrationEnabled = false;

    g_argc = *argc;
    g_argv = *argv;
    g_app  = new QApplication(g_argc, g_argv);
    g_dir  = std::make_unique<QTemporaryDir>();
    QDir(g_dir->path()).mkdir("unreadable.panda");
    FuzzCircuitDir::fill(g_dir->path(), "fuzz_ic_registry");

    // Build a valid blob so initEmbeddedIC can succeed and exercise the full
    // IC::processLoadedItems → loadBoundaryElement path.
    {
        auto *sw  = ElementFactory::buildElement(ElementType::InputSwitch);
        auto *led = ElementFactory::buildElement(ElementType::Led);
        sw->setId(1);
        led->setId(2);
        QList<QGraphicsItem *> items;
        items.append(sw);
        items.append(led);

        QMap<QString, QVariant> meta;
        meta.insert("dolphinFileName", QVariant(QString()));
        g_validBlob = FuzzSeed::pandaFile(meta, items);
        qDeleteAll(items);
        FuzzSeed::requireAccepted(g_validBlob, "fuzz_ic_registry");
    }

    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    // Drain Qt's deferred-delete queue on every exit path.  Audio elements
    // (Buzzer, AudioBox) queue DeferredDelete events for the audio backend's
    // internal QObject helpers when destroyed; without spinning the event
    // loop, those helpers stay alive and hold PipeWire stream FDs open
    // across iterations, exhausting the audio subsystem after ~1000 inputs.
    auto drainQtEvents = qScopeGuard([] {
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();
    });

    if (size < 4) return 0;

    if (PandaMutator::isPandaFile(data, size)) {
        exerciseDefinition(QByteArray(reinterpret_cast<const char *>(data), static_cast<qsizetype>(size)));
        return 0;
    }

    FuzzedDataProvider fdp(data, size);

    const uint8_t path = fdp.ConsumeIntegral<uint8_t>();

    // Blob name: short printable string from fuzz bytes
    const size_t nameLen = fdp.ConsumeIntegralInRange<size_t>(1, 16);
    const QString blobName = QString::fromLatin1(
        fdp.ConsumeBytesAsString(nameLen).c_str());

    // Blob bytes: remainder up to 4096 bytes (IC blobs can be large)
    const size_t blobSize = std::min<size_t>(fdp.remaining_bytes(), 4096);
    const QByteArray blobBytes(reinterpret_cast<const char *>(data + size - blobSize),
                                static_cast<int>(blobSize));

    // Always exercise both paths so the seed corpus (which all start with 'W'
    // = 0x57 = 87, falling only in the via-load branch) also covers the direct
    // registry API.  Use `path` to vary which sub-operations are exercised within
    // exerciseRegistryDirect (e.g. whether to also call exerciseViaLoad after).
    WorkSpace ws;
    exerciseRegistryDirect(blobName, blobBytes, ws);

    if (path < 170) {
        // Also exercise the load-path blob registration
        WorkSpace ws2;
        exerciseViaLoad(blobName, blobBytes, ws2);
    }

    return 0;
}
