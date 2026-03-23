// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief DolphinSerializer: encoding and decoding of .dolphin and .csv waveform files.
 */

#pragma once

#include <QVector>

class QDataStream;
class QFile;
class QSaveFile;
class QStandardItemModel;

/**
 * \namespace DolphinSerializer
 * \brief Pure I/O functions for the beWavedDolphin waveform file formats.
 *
 * \details Handles only the encoding and decoding of data.  BewavedDolphin
 * retains responsibility for model updates (setLength, setCellValue, run).
 */
namespace DolphinSerializer {

/**
 * \brief Raw waveform data returned by the load functions.
 *
 * \details Contains only the input rows — output rows are always recomputed
 * by run() and are not persisted in the binary format.  CSV files do include
 * output rows but they are ignored on load for the same reason.
 */
struct WaveformData {
    int inputPorts = 0; ///< Number of input rows stored in \a values.
    int columns    = 0; ///< Number of time-step columns.
    /// Cell values in row-major order (inputs only): index = row * columns + col.
    QVector<int> values;
};

/**
 * \brief Serializes input rows to the binary .dolphin stream.
 * \param stream     Destination data stream (header already written by caller).
 * \param model      Source model.
 * \param inputPorts Number of input rows to serialize (output rows are skipped).
 */
void saveBinary(QDataStream &stream, const QStandardItemModel *model, int inputPorts);

/**
 * \brief Deserializes input rows from a binary .dolphin stream.
 * \param stream        Source data stream (header already consumed by caller).
 * \param maxInputPorts Clamp row count to this value (current circuit's input count).
 * \return Parsed waveform data ready for BewavedDolphin to apply.
 * \throws Pandaception on invalid column count.
 */
WaveformData loadBinary(QDataStream &stream, int maxInputPorts);

/**
 * \brief Serializes all rows (inputs + outputs) to a .csv file.
 * \param file  Destination save file (already opened for writing).
 * \param model Source model.
 */
void saveCSV(QSaveFile &file, const QStandardItemModel *model);

/**
 * \brief Deserializes input rows from a .csv file.
 * \param file          Source file (already opened for reading).
 * \param maxInputPorts Clamp row count to this value.
 * \return Parsed waveform data ready for BewavedDolphin to apply.
 * \throws Pandaception on invalid or insufficient data.
 */
WaveformData loadCSV(QFile &file, int maxInputPorts);

} // namespace DolphinSerializer

