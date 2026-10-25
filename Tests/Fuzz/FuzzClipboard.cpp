// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief libFuzzer harness for the clipboard paste/copy deserializer.
 *
 * Two kinds of input:
 *
 *   1. A whole .panda file (it starts with "WPCF", or with the zero byte of a
 *      legacy header's length): loaded into a workspace,
 *      every element selected and copied with ClipboardManager::copy(), the
 *      clipboard pasted into a second workspace, the paste undone and redone,
 *      the circuit's clone-drag image built, the selection duplicated, and
 *      everything cut.  Exercises the clipboard's writer, IC blob collection
 *      included, on circuits from the seed corpus.
 *   2. Anything else is a clipboard, put on the system clipboard as paste()
 *      finds one and pasted into an empty workspace, the paste then undone and
 *      redone:
 *        byte 0      bit 0 selects the versioned blob registry (BlobRegistryV2)
 *                    over the legacy one, bit 1 the legacy item format
 *                    (ClipboardLegacy) over the current one, bit 2 leaves the
 *                    registry out
 *        bytes 1-2   the registry's length, little-endian, clamped to the input
 *        then        the registry, then the items: a .panda header, the
 *                    selection's centre and the serialized items, uncompressed
 *
 * With FUZZ_CLIPBOARD_SEED_DIR set, every clipboard path 1 copies is written
 * to that directory in path 2's layout, so a run over the seed corpus with
 * -runs=0 gives path 2 real clipboards to mutate.
 */

#include <cstddef>
#include <cstdint>

#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDataStream>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QMimeData>
#include <QRectF>
#include <QScopeGuard>
#include <QString>
#include <QTransform>
#include <QUndoStack>
#include <QVersionNumber>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/Core/MimeTypes.h"
#include "App/IO/Serialization.h"
#include "App/Scene/ClipboardManager.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"

namespace {

QApplication *g_app  = nullptr;
int  g_argc = 0;
char **g_argv = nullptr;

constexpr uint8_t kRegistryV2 = 1;
constexpr uint8_t kItemsLegacy = 2;
constexpr uint8_t kNoRegistry = 4;

// Writes the clipboard copy() just set to FUZZ_CLIPBOARD_SEED_DIR, in path 2's layout, named by its hash
void exportClipboardSeed()
{
    const QString dir = qEnvironmentVariable("FUZZ_CLIPBOARD_SEED_DIR");
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    if (dir.isEmpty() || !mimeData || !mimeData->hasFormat(MimeType::Clipboard)) {
        return;
    }

    const QByteArray registry = mimeData->data(MimeType::BlobRegistryV2);
    if (registry.size() > 0xFFFF) {
        return;
    }
    QByteArray seed;
    seed.append(static_cast<char>(registry.isEmpty() ? kNoRegistry : kRegistryV2));
    seed.append(static_cast<char>(registry.size() & 0xFF));
    seed.append(static_cast<char>((registry.size() >> 8) & 0xFF));
    seed.append(registry);
    seed.append(mimeData->data(MimeType::Clipboard));

    QFile file(QDir(dir).filePath(QCryptographicHash::hash(seed, QCryptographicHash::Sha1).toHex().left(16)));
    if (file.open(QIODevice::WriteOnly)) {
        file.write(seed);
    }
}

void pasteUndoRedo(WorkSpace &workspace)
{
    ClipboardManager clipboard(workspace.scene());
    try { clipboard.paste(); } catch (...) {}
    auto *undoStack = workspace.scene()->undoStack();
    try { undoStack->undo(); } catch (...) {}
    try { undoStack->redo(); } catch (...) {}
}

void copyPasteCut(const QByteArray &panda)
{
    QDataStream stream(panda);
    QVersionNumber version;
    WorkSpace source;
    try {
        version = Serialization::readPandaHeader(stream);
        source.load(stream, version, /*contextDir=*/QString());
    } catch (...) {
        return;
    }

    source.scene()->selectAll();
    ClipboardManager clipboard(source.scene());
    try { clipboard.copy(); } catch (...) {}
    exportClipboardSeed();

    WorkSpace target;
    pasteUndoRedo(target);

    // The clone-drag ghost of the whole circuit: its size is capped however far apart the elements are
    const QRectF bounds = source.scene()->itemsBoundingRect();
    try { ClipboardManager::buildDragImage(source.scene(), QTransform(), bounds); } catch (...) {}

    // duplicate() copies the selection in place, through the scene's own blob registry, then the copies are cut:
    // cut() serializes the selection and then deletes it, unlike copy()
    try { clipboard.duplicate(); } catch (...) {}
    source.scene()->selectAll();
    try { clipboard.cut(); } catch (...) {}
}

void pasteClipboard(const uint8_t *data, size_t size)
{
    const uint8_t flags = data[0];
    const size_t registrySize = std::min<size_t>(size - 3, data[1] | (static_cast<size_t>(data[2]) << 8));
    const QByteArray registry(reinterpret_cast<const char *>(data + 3), static_cast<int>(registrySize));
    const QByteArray items(reinterpret_cast<const char *>(data + 3 + registrySize),
                           static_cast<int>(size - 3 - registrySize));

    auto *mimeData = new QMimeData;
    if (!(flags & kNoRegistry)) {
        mimeData->setData((flags & kRegistryV2) ? MimeType::BlobRegistryV2 : MimeType::BlobRegistry, registry);
    }
    mimeData->setData((flags & kItemsLegacy) ? MimeType::ClipboardLegacy : MimeType::Clipboard, items);
    QApplication::clipboard()->setMimeData(mimeData);

    WorkSpace target;
    pasteUndoRedo(target);
}

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
    g_app  = new QApplication(g_argc, g_argv);

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

    // A legacy .panda file starts with its header string's big-endian length, so with a zero byte; a clipboard's
    // flags byte is never zero in the seeds path 1 writes
    const QByteArray input = QByteArray::fromRawData(reinterpret_cast<const char *>(data), static_cast<int>(size));
    if (input.startsWith("WPCF") || (data[0] == 0)) {
        copyPasteCut(input);
    } else {
        pasteClipboard(data, size);
    }

    QApplication::clipboard()->clear();
    return 0;
}
