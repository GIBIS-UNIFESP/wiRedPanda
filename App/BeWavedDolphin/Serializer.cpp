// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/Serializer.h"

#include <QDataStream>
#include <QFile>
#include <QSaveFile>
#include <QStandardItemModel>

#include "App/Core/Common.h"

namespace DolphinSerializer {

void saveBinary(QDataStream &stream, const QStandardItemModel *model, const int inputPorts)
{
    // .dolphin format: inputPortCount | columnCount | values (col-major, inputs only)
    // Output rows are not saved because they are deterministically computed by run()
    stream << static_cast<qint64>(inputPorts);
    stream << static_cast<qint64>(model->columnCount());

    for (int col = 0; col < model->columnCount(); ++col) {
        for (int row = 0; row < inputPorts; ++row) {
            stream << static_cast<qint64>(model->index(row, col).data().toInt());
        }
    }
}

WaveformData loadBinary(QDataStream &stream, const int maxInputPorts)
{
    qint64 rows; stream >> rows;
    qint64 cols; stream >> cols;

    if (rows < 0) {
        throw PANDACEPTION_WITH_CONTEXT("DolphinSerializer", "Invalid number of rows.");
    }
    if (rows > maxInputPorts) {
        rows = maxInputPorts;
    }

    if ((cols < 2) || (cols > 2048)) {
        throw PANDACEPTION_WITH_CONTEXT("DolphinSerializer", "Invalid number of columns.");
    }

    WaveformData data;
    data.inputPorts = static_cast<int>(rows);
    data.columns    = static_cast<int>(cols);
    data.values.resize(data.inputPorts * data.columns);

    // Binary format is col-major; normalize to row-major for the caller
    for (int col = 0; col < data.columns; ++col) {
        for (int row = 0; row < data.inputPorts; ++row) {
            qint64 value; stream >> value;
            data.values[row * data.columns + col] = static_cast<int>(value);
        }
    }

    return data;
}

void saveCSV(QSaveFile &file, const QStandardItemModel *model)
{
    // CSV format: first line is "rows,cols,"; subsequent lines are comma-separated row values.
    // Both input and output rows are written so the CSV is human-readable without re-running.
    file.write(QString::number(model->rowCount()).toUtf8());
    file.write(",");
    file.write(QString::number(model->columnCount()).toUtf8());
    file.write(",\n");

    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            file.write(model->index(row, col).data().toString().toUtf8());
            file.write(",");
        }

        file.write("\n");
    }
}

WaveformData loadCSV(QFile &file, const int maxInputPorts)
{
    // CSV is a flat comma-separated byte array: "rows,cols,v00,v01,...,"
    // (trailing comma on each row is tolerated; split(',') produces a trailing empty element)
    const QByteArray content = file.readAll();
    const auto wordList = content.split(',');

    if (wordList.size() < 2) {
        throw PANDACEPTION_WITH_CONTEXT("DolphinSerializer", "Invalid CSV format: insufficient data.");
    }

    int       rows = wordList.at(0).toInt();
    const int cols = wordList.at(1).toInt();

    if (rows < 0) {
        throw PANDACEPTION_WITH_CONTEXT("DolphinSerializer", "Invalid number of rows.");
    }
    if (rows > maxInputPorts) {
        rows = maxInputPorts;
    }

    if ((cols < 2) || (cols > 2048)) {
        throw PANDACEPTION_WITH_CONTEXT("DolphinSerializer", "Invalid number of columns.");
    }

    // Validate before indexing to avoid out-of-bounds access on corrupt files
    const int expectedSize = 2 + rows * cols;
    if (wordList.size() < expectedSize) {
        throw PANDACEPTION_WITH_CONTEXT("DolphinSerializer",
            "Invalid CSV format: expected %1 elements, got %2.",
            expectedSize, static_cast<int>(wordList.size()));
    }

    WaveformData data;
    data.inputPorts = rows;
    data.columns    = cols;
    data.values.resize(rows * cols);

    // CSV values are stored row-major: index = 2 + row*cols + col
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            data.values[row * cols + col] = wordList.at(2 + col + row * cols).toInt();
        }
    }

    return data;
}

} // namespace DolphinSerializer

