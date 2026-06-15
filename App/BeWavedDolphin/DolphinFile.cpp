// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/DolphinFile.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QFile>
#include <QSaveFile>

#include "App/BeWavedDolphin/SignalModel.h"
#include "App/Core/Common.h"
#include "App/IO/Serialization.h"

namespace DolphinFile {

void save(const SignalModel &model, const QString &fileName, const int inputPorts)
{
    // QSaveFile writes to a temp file and atomically renames on commit,
    // preventing data loss if the process is interrupted during a write
    QSaveFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)) {
        throw PANDACEPTION_WITH_CONTEXT("BewavedDolphin", "Error opening file: %1", file.errorString());
    }

    if (fileName.endsWith(".dolphin")) {
        qCDebug(zero) << "Saving dolphin file.";
        QDataStream stream(&file);
        Serialization::writeDolphinHeader(stream);
        DolphinSerializer::saveBinary(stream, &model, inputPorts);
    } else {
        qCDebug(zero) << "Saving CSV file.";
        DolphinSerializer::saveCSV(file, &model);
    }

    if (!file.commit()) {
        throw PANDACEPTION_WITH_CONTEXT("BewavedDolphin", "Error saving file: %1", file.errorString());
    }
}

DolphinSerializer::WaveformData load(const QString &fileName, const int maxInputPorts)
{
    QFile file(fileName);

    if (!file.exists()) {
        throw PANDACEPTION_WITH_CONTEXT("BewavedDolphin", "File \"%1\" does not exist!", fileName);
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(zero) << "Could not open file in ReadOnly mode: " << file.errorString();
        throw PANDACEPTION_WITH_CONTEXT("BewavedDolphin", "Could not open file for reading: %1", file.errorString());
    }

    DolphinSerializer::WaveformData data;

    if (fileName.endsWith(".dolphin")) {
        qCDebug(zero) << "Dolphin file opened.";
        QDataStream stream(&file);
        Serialization::readDolphinHeader(stream);
        data = DolphinSerializer::loadBinary(stream, maxInputPorts);
    } else if (fileName.endsWith(".csv")) {
        qCDebug(zero) << "CSV file opened.";
        data = DolphinSerializer::loadCSV(file, maxInputPorts);
    } else {
        qCDebug(zero) << "Format not supported. Could not open file: " << fileName;
        throw PANDACEPTION_WITH_CONTEXT("BewavedDolphin", "Format not supported. Could not open file: %1", fileName);
    }

    file.close();
    return data;
}

} // namespace DolphinFile
