// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief libFuzzer harness for Serialization::copyPandaFile(), readPreamble() and File > Save As.
 *
 * All other harnesses exercise the WorkSpace::load() path, which skips the preamble-only reader (readPreamble) and
 * the file copy helper (copyPandaFile). Both run when a circuit is saved to another folder, copying each file-backed
 * IC's file and the files it names in turn, and when an IC's file is added to a project:
 *
 *   - readPreamble(): reads the panda preamble (header, then the metadata) WITHOUT deserialising scene elements,
 *     after copyPandaFile()'s peek-based plausibility check.
 *
 *   - copyPandaFile(): copies a .panda file into another folder, then the files its "fileBackedICs" metadata names,
 *     each once (a file named twice is copied once) and to a depth of 16.
 *
 * The input is written as fuzz_input.panda beside the repository's circuits (Tests/Fuzz/FuzzCircuitDir.h), so the
 * files it names are copied with it, and copied into a new folder, read-only for half the inputs. Then a circuit
 * naming the input both directly and through a second file that names it too, or naming it at the end of a chain
 * of files as long as the input's flags pick, is copied into another folder. Last, the input is loaded and saved
 * into the first folder, as File > Save As does.
 */

#include <cstddef>
#include <cstdint>
#include <memory>

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileDevice>
#include <QFileInfo>
#include <QIODevice>
#include <QMap>
#include <QScopeGuard>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QVariant>

#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/IO/Serialization.h"
#include "App/Presentation/ElementPresenter.h"
#include "App/Scene/Workspace.h"
#include "Tests/Fuzz/FuzzCircuitDir.h"
#include "Tests/Fuzz/FuzzSeed.h"
#include "Tests/Fuzz/PandaMutator.h"

namespace {

QApplication *g_app  = nullptr;
int  g_argc = 0;
char **g_argv = nullptr;

// The source folder: the repository's circuits, the files below naming the input, and the input
// Held until exit, which removes it
std::unique_ptr<QTemporaryDir> g_src;

// The longest chain of files the input is named at the end of: it puts the input at copyPandaFile()'s depth limit of
// 16, so the input is refused, and the shorter ones reach the limit inside the files the input names
constexpr int kMaxChain = 15;

// Writes a .panda file into the source folder, read-only, holding no items and naming \a fileBackedICs
void writeNamingFile(const QString &name, const QStringList &fileBackedICs)
{
    QMap<QString, QVariant> meta;
    meta.insert("fileBackedICs", fileBackedICs);
    const QString path = QDir(g_src->path()).filePath(name);
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(FuzzSeed::pandaFile(meta, [](QDataStream &) {}));
    }
    file.close();
    QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::ReadUser);
}

QString chainName(const int length)
{
    return QString("fuzz_chain_%1.panda").arg(length);
}

} // namespace

// The input is a whole .panda file: the shared structure-aware mutator works on a current-format file's decompressed
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
    installPresenters();

    g_src = std::make_unique<QTemporaryDir>();
    FuzzCircuitDir::fill(g_src->path(), "fuzz_copy_panda");

    // A file naming the input, so a circuit naming both names the input twice; and chains of files ending in it,
    // fuzz_chain_1 naming the input and each longer one the one before
    writeNamingFile("fuzz_shared.panda", {"fuzz_input.panda"});
    writeNamingFile(chainName(1), {"fuzz_input.panda"});
    for (int length = 2; length <= kMaxChain; ++length) {
        writeNamingFile(chainName(length), {chainName(length - 1)});
    }
    writeNamingFile("fuzz_outer_0.panda", {"fuzz_input.panda", "fuzz_shared.panda"});
    for (int length = 1; length <= kMaxChain; ++length) {
        writeNamingFile(QString("fuzz_outer_%1.panda").arg(length), {chainName(length), "fuzz_shared.panda"});
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

    if (size == 0) return 0;
    const uint8_t flags = data[size - 1];

    const QString inputPath = QDir(g_src->path()).filePath("fuzz_input.panda");
    {
        QFile file(inputPath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return 0;
        file.write(reinterpret_cast<const char *>(data), static_cast<qint64>(size));
    }

    // The input copied into a new folder, as when an IC's file is added to a project; half the folders are
    // read-only, so the copy fails
    QTemporaryDir dest;
    if (!dest.isValid()) return 0;
    const auto destPermissions = QFile::permissions(dest.path());
    if ((flags & 1) != 0) {
        QFile::setPermissions(dest.path(), QFileDevice::ReadOwner | QFileDevice::ExeOwner);
    }
    auto restoreDest = qScopeGuard([&] { QFile::setPermissions(dest.path(), destPermissions); });
    const QString destPath = QDir(dest.path()).filePath("fuzz_input.panda");
    try {
        Serialization::copyPandaFile(QFileInfo(inputPath), QFileInfo(destPath));
    } catch (...) {}

    // A circuit naming the input twice, directly or through a chain, then through fuzz_shared, copied into another
    // folder
    {
        QTemporaryDir outerDest;
        const int chain = (flags >> 1) % (kMaxChain + 1);
        const QString outer = QString("fuzz_outer_%1.panda").arg(chain);
        if (outerDest.isValid()) {
            try {
                Serialization::copyPandaFile(QFileInfo(QDir(g_src->path()).filePath(outer)),
                                             QFileInfo(QDir(outerDest.path()).filePath(outer)));
            } catch (...) {}
        }
    }

    // File > Save As: the input loaded, then saved into the first folder, which copies the files its ICs name
    WorkSpace ws;
    try {
        ws.load(inputPath);
    } catch (...) {
        return 0;
    }
    try {
        ws.save(destPath);
    } catch (...) {}

    return 0;
}
