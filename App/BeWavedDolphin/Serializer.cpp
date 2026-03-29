// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/Serializer.h"

#include <QDataStream>
#include <QFile>
#include <QSaveFile>

#include "App/BeWavedDolphin/WaveformGrid.h"
#include "App/Core/Common.h"

namespace DolphinSerializer {

void saveBinary(QDataStream &stream, const WaveformGrid *grid, const int inputPorts)
{
    stream << static_cast<qint64>(inputPorts);
    stream << static_cast<qint64>(grid->length());

    for (int col = 0; col < grid->length(); ++col) {
        for (int row = 0; row < inputPorts; ++row) {
            stream << static_cast<qint64>(grid->cellValue(row, col));
        }
    }
}

WaveformData loadBinary(QDataStream &stream, const int maxInputPorts)
{
    qint64 rows; stream >> rows;
    qint64 cols; stream >> cols;

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

    for (int col = 0; col < data.columns; ++col) {
        for (int row = 0; row < data.inputPorts; ++row) {
            qint64 value; stream >> value;
            data.values[row * data.columns + col] = static_cast<int>(value);
        }
    }

    return data;
}

void saveCSV(QSaveFile &file, const WaveformGrid *grid)
{
    file.write(QString::number(grid->rowCount()).toUtf8());
    file.write(",");
    file.write(QString::number(grid->length()).toUtf8());
    file.write(",\n");

    for (int row = 0; row < grid->rowCount(); ++row) {
        for (int col = 0; col < grid->length(); ++col) {
            file.write(QString::number(grid->cellValue(row, col)).toUtf8());
            file.write(",");
        }

        file.write("\n");
    }
}

WaveformData loadCSV(QFile &file, const int maxInputPorts)
{
    const QByteArray content = file.readAll();
    const auto wordList = content.split(',');

    if (wordList.size() < 2) {
        throw PANDACEPTION_WITH_CONTEXT("DolphinSerializer", "Invalid CSV format: insufficient data.");
    }

    int       rows = wordList.at(0).toInt();
    const int cols = wordList.at(1).toInt();

    if (rows > maxInputPorts) {
        rows = maxInputPorts;
    }

    if ((cols < 2) || (cols > 2048)) {
        throw PANDACEPTION_WITH_CONTEXT("DolphinSerializer", "Invalid number of columns.");
    }

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

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            data.values[row * cols + col] = wordList.at(2 + col + row * cols).toInt();
        }
    }

    return data;
}

} // namespace DolphinSerializer
