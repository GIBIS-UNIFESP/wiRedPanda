// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief DolphinFile: reads/writes a SignalModel to .dolphin/.csv files on disk.
 */

#pragma once

#include "App/BeWavedDolphin/Serializer.h"

class QString;
class QTextStream;
class SignalModel;

/**
 * \namespace DolphinFile
 * \brief Disk I/O for the beWavedDolphin formats: opens the file, dispatches by
 * extension (.dolphin = binary, .csv), and delegates the encoding to DolphinSerializer.
 *
 * \details Pure file ↔ model transfer — no dialogs, window state, or simulation.
 * Loading returns the parsed data for BewavedDolphin to apply (setLength + run).
 */
namespace DolphinFile {

/// Writes \a model to \a fileName atomically, choosing the format by extension.
void save(const SignalModel &model, const QString &fileName, int inputPorts);

/// Reads \a fileName (.dolphin or .csv) and returns its input rows, clamped to
/// \a maxInputPorts. Throws on a missing file or unsupported format.
DolphinSerializer::WaveformData load(const QString &fileName, int maxInputPorts);

/// Parses the terminal/automation stream protocol from \a in: a `"rows,cols"` header
/// line followed by one comma-separated row of 0/1 values per input. \a rows is clamped
/// to \a maxInputPorts; \a cols is validated against [1, SignalModel::kMaxColumns].
/// Returns the parsed input rows for the caller to apply. Throws on malformed input.
DolphinSerializer::WaveformData parseTerminal(QTextStream &in, int maxInputPorts);

} // namespace DolphinFile
