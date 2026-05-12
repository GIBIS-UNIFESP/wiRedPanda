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
 * Input layout (first byte selects which path to exercise):
 *   0–84  → registry manipulation + initEmbeddedIC
 *   85–169 → WorkSpace::load with embedded blob in metadata
 *   170+  → both paths sequentially
 */

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QEvent>
#include <QIODevice>
#include <QMap>
#include <QScopeGuard>
#include <QString>
#include <QVersionNumber>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/Element/IC.h"
#include "App/Element/ICRegistry.h"
#include "App/IO/Serialization.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"

namespace {

QApplication *g_app  = nullptr;
int  g_argc = 0;
char **g_argv = nullptr;

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

} // namespace

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_IM_MODULES", "none");

    Comment::setVerbosity(-1);
    Application::interactiveMode = false;
    Application::migrationEnabled = false;

    g_argc = *argc;
    g_argv = *argv;
    g_app  = new QApplication(g_argc, g_argv);

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

        g_validBlob.clear();
        QDataStream s(&g_validBlob, QIODevice::WriteOnly);
        Serialization::writePandaHeader(s);
        QMap<QString, QVariant> meta;
        meta.insert("dolphinFileName", QVariant(QString()));
        s << meta;
        Serialization::serialize(items, s);
        qDeleteAll(items);
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

