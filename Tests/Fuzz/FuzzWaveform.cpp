// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief libFuzzer harness for the beWavedDolphin waveform deserializer.
 *
 * The .panda harnesses never reach this code path: they speak the workspace
 * format, which only stores a dolphin *filename* in its metadata — it never
 * parses the waveform content.  The waveform body is only read when the user
 * opens a .dolphin file, which requires a separate File > Open call in
 * BewavedDolphin.
 *
 * This harness targets two distinct entry points:
 *
 *   1. Serialization::readDolphinHeader — detects the magic number / legacy
 *      app-name string and reads the version field.
 *   2. DolphinSerializer::loadBinary — reads the structured waveform body
 *      (row count, column count, cell values) that follows the header.
 *
 * Seed corpus: copy any .dolphin test files here; any raw binary files that
 * start with the WPCF magic (waveform variant) or the legacy "beWavedDolphin"
 * string also work.
 */

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QEvent>
#include <QFile>
#include <QSaveFile>
#include <QScopeGuard>
#include <QStandardItemModel>
#include <QString>
#include <QTemporaryFile>

#include "App/BeWavedDolphin/Serializer.h"
#include "App/Core/Application.h"
#include "App/Core/Common.h"
#include "App/IO/Serialization.h"

namespace {

QApplication *g_app  = nullptr;
int  g_argc = 0;
char **g_argv = nullptr;

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

    // Auto-detect format from the first 4 bytes:
    //   0x57505746 = "WPWF" (modern binary waveform magic)
    //   starts with 'b' + sufficient length → likely "beWavedDolphin" legacy
    //   otherwise → CSV path or structured binary generator
    const quint32 magic = (quint32(data[0]) << 24) | (quint32(data[1]) << 16)
                        | (quint32(data[2]) <<  8) |  quint32(data[3]);
    const bool looksLikeBinary = (magic == 0x57505746u)
                               || (data[0] == 'b' && size >= 14);

    const int maxInputPorts = std::max(1, static_cast<int>(data[size - 1]) % 64 + 1);

    if (!looksLikeBinary) {
        // Use the first byte to select between CSV path and structured binary generator.
        // The structured generator always produces a valid binary waveform stream,
        // ensuring the inner loops of loadBinary (col-major cell reads) are exercised
        // even when the raw bytes don't happen to start with WPWF.
        if (data[0] & 1) {
            // --- Structured binary path: FuzzedDataProvider → valid waveform ---
            FuzzedDataProvider fdp(data + 1, size - 1);
            const int rows = fdp.ConsumeIntegralInRange<int>(0, 8);
            const int cols = fdp.ConsumeIntegralInRange<int>(2, 32);

            QByteArray buf;
            QDataStream s(&buf, QIODevice::WriteOnly);
            s.setVersion(QDataStream::Qt_5_12);
            Serialization::writeDolphinHeader(s);
            s << static_cast<qint64>(rows) << static_cast<qint64>(cols);
            for (int c = 0; c < cols; ++c) {
                for (int r = 0; r < rows; ++r) {
                    s << static_cast<qint64>(fdp.ConsumeBool() ? 1 : 0);
                }
            }

            QDataStream stream2(buf);
            stream2.setVersion(QDataStream::Qt_5_12);
            try { Serialization::readDolphinHeader(stream2); } catch (...) { return 0; }
            DolphinSerializer::WaveformData loaded2;
            try { loaded2 = DolphinSerializer::loadBinary(stream2, maxInputPorts); } catch (...) {}

            // saveBinary + saveCSV round-trip on the structured path.
            // rows is guaranteed >= 1 here (ConsumeIntegralInRange(0,8) gives 0-8 but
            // we skip the round-trip when rows == 0 since the model would be empty).
            if (loaded2.inputPorts > 0 && loaded2.columns > 0) {
                QStandardItemModel model2(loaded2.inputPorts + 1, loaded2.columns);
                for (int r = 0; r < loaded2.inputPorts; ++r) {
                    for (int c = 0; c < loaded2.columns; ++c) {
                        const int v = loaded2.values.value(r * loaded2.columns + c, 0);
                        model2.setItem(r, c, new QStandardItem(QString::number(v)));
                    }
                }
                // saveBinary round-trip
                QByteArray sb;
                QDataStream ss(&sb, QIODevice::WriteOnly);
                ss.setVersion(QDataStream::Qt_5_12);
                Serialization::writeDolphinHeader(ss);
                try { DolphinSerializer::saveBinary(ss, &model2, loaded2.inputPorts); } catch (...) {}

                // saveCSV round-trip — always exercise both save formats in the structured path
                QTemporaryFile csvFile2;
                csvFile2.setAutoRemove(true);
                if (csvFile2.open()) {
                    const QString csvPath2 = csvFile2.fileName();
                    csvFile2.close();
                    QSaveFile sf2(csvPath2);
                    if (sf2.open(QIODevice::WriteOnly)) {
                        try { DolphinSerializer::saveCSV(sf2, &model2); sf2.commit(); } catch (...) {}
                    }
                    QFile readBack2(csvPath2);
                    if (readBack2.open(QIODevice::ReadOnly)) {
                        try { DolphinSerializer::loadCSV(readBack2, loaded2.inputPorts); } catch (...) {}
                    }
                }
            }
        } else {
            // --- CSV path: DolphinSerializer::loadCSV ---
            QTemporaryFile tmp;
            if (tmp.open()) {
                tmp.write(reinterpret_cast<const char *>(data), static_cast<qint64>(size));
                tmp.seek(0);
                try { DolphinSerializer::loadCSV(tmp, maxInputPorts); } catch (...) {}
            }
        }
        return 0;
    }

    // --- Raw binary path: readDolphinHeader + loadBinary ---
    const QByteArray buffer = QByteArray::fromRawData(reinterpret_cast<const char *>(data),
                                                      static_cast<int>(size));
    QDataStream stream(buffer);
    stream.setVersion(QDataStream::Qt_5_12);

    try { Serialization::readDolphinHeader(stream); } catch (...) { return 0; }

    DolphinSerializer::WaveformData loaded;
    try { loaded = DolphinSerializer::loadBinary(stream, maxInputPorts); } catch (...) {}

    // --- saveBinary round-trip: load → save → reload ---
    // saveBinary and saveCSV are completely dark without a QStandardItemModel.
    // Build a minimal model from the loaded data and exercise the save path.
    if (loaded.inputPorts > 0 && loaded.columns > 0) {
        QStandardItemModel model(loaded.inputPorts + 1, loaded.columns);
        for (int row = 0; row < loaded.inputPorts; ++row) {
            for (int col = 0; col < loaded.columns; ++col) {
                const int val = loaded.values.value(row * loaded.columns + col, 0);
                model.setItem(row, col, new QStandardItem(QString::number(val)));
            }
        }

        QByteArray savedBuf;
        {
            QDataStream saveStream(&savedBuf, QIODevice::WriteOnly);
            saveStream.setVersion(QDataStream::Qt_5_12);
            Serialization::writeDolphinHeader(saveStream);
            try { DolphinSerializer::saveBinary(saveStream, &model, loaded.inputPorts); } catch (...) {}
        }

        if (!savedBuf.isEmpty()) {
            QDataStream reloadStream(savedBuf);
            reloadStream.setVersion(QDataStream::Qt_5_12);
            try { Serialization::readDolphinHeader(reloadStream); } catch (...) {}
            try { DolphinSerializer::loadBinary(reloadStream, loaded.inputPorts); } catch (...) {}
        }

        // saveCSV round-trip: write model as CSV, reload from temp file
        {
            QTemporaryFile csvFile;
            csvFile.setAutoRemove(true);
            if (csvFile.open()) {
                const QString csvPath = csvFile.fileName();
                csvFile.close();
                QSaveFile sf(csvPath);
                if (sf.open(QIODevice::WriteOnly)) {
                    try { DolphinSerializer::saveCSV(sf, &model); sf.commit(); } catch (...) {}
                }
                QFile readBack(csvPath);
                if (readBack.open(QIODevice::ReadOnly)) {
                    try { DolphinSerializer::loadCSV(readBack, loaded.inputPorts); } catch (...) {}
                }
            }
        }
    }

    return 0;
}
