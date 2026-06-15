// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/DolphinFile.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QFile>
#include <QSaveFile>
#include <QTextStream>

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

DolphinSerializer::WaveformData parseTerminal(QTextStream &in, const int maxInputPorts)
{
    // Protocol: first line is "rows,cols"; subsequent lines contain comma-separated 0/1
    // values per row. This allows driving the simulator from scripts without a GUI dialog.
    QString str = in.readLine();
    const auto wordList(str.split(','));

    if (wordList.size() < 2) {
        throw PANDACEPTION_WITH_CONTEXT("BewavedDolphin", "Invalid header: expected 'rows,cols' on the first line.");
    }

    int rows = wordList.at(0).toInt();
    const int cols = wordList.at(1).toInt();

    // Clamp rows to the number of actual input ports to avoid out-of-bounds writes
    if (rows > maxInputPorts) {
        rows = maxInputPorts;
    }

    if ((cols < 1) || (cols > SignalModel::kMaxColumns)) {
        throw PANDACEPTION_WITH_CONTEXT("BewavedDolphin", "Invalid column count %1: must be between 1 and %2.", QString::number(cols), QString::number(SignalModel::kMaxColumns));
    }

    DolphinSerializer::WaveformData data;
    data.inputPorts = rows;
    data.columns = cols;
    data.values.resize(rows * cols);

    for (int row = 0; row < rows; ++row) {
        str = in.readLine();
        const auto wordList2(str.split(','));

        if (wordList2.size() < cols) {
            throw PANDACEPTION_WITH_CONTEXT("BewavedDolphin", "Row %1 has %2 value(s) but %3 are required.", QString::number(row), QString::number(wordList2.size()), QString::number(cols));
        }

        for (int col = 0; col < cols; ++col) {
            data.values[row * cols + col] = wordList2.at(col).toInt();
        }
    }

    return data;
}

} // namespace DolphinFile
