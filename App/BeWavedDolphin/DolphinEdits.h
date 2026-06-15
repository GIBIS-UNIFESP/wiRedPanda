// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief DolphinEdits: pure value-grid mutations for the beWavedDolphin waveform table.
 */

#pragma once

#include <functional>

#include <QModelIndexList>

class SignalModel;

/**
 * \namespace DolphinEdits
 * \brief Pure mutations of a SignalModel's value grid — the data side of the editor's
 * commands (set/invert/clear/clock/combinational/auto-crop/length).
 *
 * \details No view, selection model, dialog, simulation, or edited-flag bookkeeping — the
 * controller's slots own those and call these to change the cells. Functions touch only
 * input rows where that is the editor's contract (clear/combinational/grow); cell-list
 * variants act on whatever cells the caller passes (output rows are never selectable).
 */
namespace DolphinEdits {

/// Sets each cell in \a cells to `valueFn(currentValue)`.
void applyToCells(SignalModel &model, const QModelIndexList &cells, const std::function<int(int)> &valueFn);

/// Writes a 50%-duty clock into \a cells: LOW for the first half of each \a period, HIGH for
/// the second, with the phase anchored at column \a firstCol.
void clockWave(SignalModel &model, const QModelIndexList &cells, int firstCol, int period);

/// Fills the first \a inputPorts rows over \a columns with the Gray-code-style enumeration of
/// all input combinations (the combinational truth table).
void combinational(SignalModel &model, int inputPorts, int columns);

/// Sets every input cell (the first \a inputPorts rows, over the model's columns) to 0.
void clearInputs(SignalModel &model, int inputPorts);

/// Returns the index of the last column with any non-zero input value, or 0 if none.
int lastNonZeroColumn(const SignalModel &model, int inputPorts);

/// Zero-fills the newly added input cells in columns [\a oldLength, \a newLength) across the
/// first \a inputPorts rows (used after the model grows).
void growInputColumns(SignalModel &model, int inputPorts, int oldLength, int newLength);

} // namespace DolphinEdits
