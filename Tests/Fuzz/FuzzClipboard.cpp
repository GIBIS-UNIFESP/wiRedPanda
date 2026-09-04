// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief libFuzzer harness for the clipboard paste/copy deserializer.
 *
 * Three phases:
 *
 *   1. BlobRegistry / BlobRegistryV2 MIME slot deserialization.
 *   2. QuickWorkSpace::load() from the clipboard panda stream.
 *   3. Clipboard round-trip: load a workspace from fuzz bytes, select all
 *      elements, call CanvasItem::copyAction() (serializes to clipboard),
 *      then pasteAction() (deserializes back) — exercises the full clipboard
 *      serialize/deserialize path including IC blob collection.
 */

#include <cstddef>
#include <cstdint>

#include <QGuiApplication>
#include <QByteArray>
#include <QClipboard>
#include <QCoreApplication>
#include <QDataStream>
#include <QEvent>
#include <QMimeData>
#include <QScopeGuard>
#include <QString>
#include <QVersionNumber>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/MimeTypes.h"
#include "App/Element/ElementFactory.h"
#include "App/Element/GraphicElement.h"
#include "App/IO/Serialization.h"
#include "App/QuickShell/Canvas/CanvasItem.h"
#include "App/QuickShell/Chrome/QuickWorkSpace.h"

namespace {

QGuiApplication *g_app  = nullptr;
int  g_argc = 0;
char **g_argv = nullptr;

// Pre-built valid blob registry bytes (Qt_5_12 versioned) containing one entry.
// Used to exercise the readBoundedBlobMap loop body which requires count >= 1,
// a condition that raw fuzz bytes rarely satisfy in a small corpus.
QByteArray g_validBlobRegistry;

} // namespace

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
    g_app  = new QGuiApplication(g_argc, g_argv);

    // Build a minimal IC blob (InputSwitch + Led) for the registry entry.
    {
        auto *sw  = ElementFactory::buildElement(ElementType::InputSwitch);
        auto *led = ElementFactory::buildElement(ElementType::Led);
        sw->setId(1);  led->setId(2);
        QList<ItemWithId *> items;
        items.append(sw);  items.append(led);

        QByteArray icBlob;
        QDataStream s(&icBlob, QIODevice::WriteOnly);
        Serialization::writePandaHeader(s);
        QMap<QString, QVariant> meta;
        meta.insert("dolphinFileName", QVariant(QString()));
        s << meta;
        Serialization::serialize(items, s, {.purpose = SerializationPurpose::PortableFile});
        qDeleteAll(items);

        // Serialise as a blob registry: QMap<QString,QByteArray> in Qt_5_12 format.
        // readBoundedBlobMap reads: quint32 count + count × (QString key + QByteArray val).
        // This matches QMap<QString,QByteArray>::operator<<(QDataStream&) wire format.
        QDataStream rs(&g_validBlobRegistry, QIODevice::WriteOnly);
        rs.setVersion(QDataStream::Qt_5_12);
        QMap<QString, QByteArray> blobMap;
        blobMap.insert("fuzz_ic.panda", icBlob);
        rs << blobMap;
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

    if (size < 2) return 0;

    const bool useV2     = data[0] != 0;
    const size_t split   = std::min<size_t>(256, size / 2);
    const QByteArray regBytes(reinterpret_cast<const char *>(data + 1),
                               static_cast<int>(split));
    const QByteArray pandaBytes(reinterpret_cast<const char *>(data + 1 + split),
                                 static_cast<int>(size - 1 - split));

    // --- 1. BlobRegistry deserialization ---
    // Exercises Serialization::readBoundedBlobMap which protects both the
    // versioned (BlobRegistryV2 / Qt_5_12) and legacy (unversioned) clipboard
    // blob registry formats against fuzz-controlled OOM.
    {
        QByteArray reg(regBytes);
        QDataStream regStream(&reg, QIODevice::ReadOnly);
        if (useV2) regStream.setVersion(QDataStream::Qt_5_12);
        try {
            Serialization::readBoundedBlobMap(regStream);
        } catch (...) {}
    }

    // 1b. Valid blob registry path — exercises the readBoundedBlobMap loop body
    // (lines 296-302 in Serialization.cpp) which requires count >= 1 and a valid
    // entry.  Random fuzz bytes rarely produce count > 0 at the right stream offset
    // so this path was dark.  Mixing the valid registry ensures the loop runs.
    {
        QDataStream validStream(g_validBlobRegistry);
        validStream.setVersion(QDataStream::Qt_5_12);
        try {
            Serialization::readBoundedBlobMap(validStream);
        } catch (...) {}
    }

    // --- 2. Clipboard panda stream deserialization ---
    // Same path as WorkSpace::load but entered via the clipboard paste route.
    // Exercises readPandaHeader + deserializeAndAdd (which calls
    // Serialization::deserialize into a scene).
    {
        QDataStream stream(pandaBytes);
        QVersionNumber version;
        try {
            version = Serialization::readPandaHeader(stream);
        } catch (...) {
            return 0;
        }

        QuickWorkSpace workspace;
        try {
            workspace.load(stream, version, /*contextDir=*/QString());
        } catch (...) {}
    }

    // --- 3. Clipboard round-trip: load → selectAll → copy → paste ---
    // Exercises ClipboardManager::copy() (serialize to clipboard with blob
    // collection) and the paste() path reading those serialized bytes back.
    // ClipboardManager::copy calls Serialization::serialize + IC blob registry
    // collection, which is dark in all other harnesses.
    {
        QDataStream stream2(pandaBytes);
        QVersionNumber ver2;
        try {
            ver2 = Serialization::readPandaHeader(stream2);
        } catch (...) {
            return 0;
        }

        QuickWorkSpace ws2;
        try {
            ws2.load(stream2, ver2, /*contextDir=*/QString());
        } catch (...) {
            return 0;
        }

        // Select all loaded elements so copyAction() has something to serialize.
        ws2.canvas()->selectAll();

        // copyAction() → serialize selected items + collect IC blobs to clipboard.
        // pasteAction() → deserialize the clipboard panda stream back into the canvas.
        // Both live directly on CanvasItem (no separate ClipboardManager on this side).
        try { ws2.canvas()->copyAction(); } catch (...) {}
        try { ws2.canvas()->pasteAction(); } catch (...) {}

        // cutAction(): serializes + DELETES selected items — distinct from copyAction()
        // because it also removes the elements from the canvas. Re-select all first
        // (pasteAction() may have changed selection state).
        ws2.canvas()->selectAll();
        try { ws2.canvas()->cutAction(); } catch (...) {}
    }

    return 0;
}
