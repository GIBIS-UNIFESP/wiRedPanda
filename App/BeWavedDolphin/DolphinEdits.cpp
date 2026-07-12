// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/DolphinEdits.h"

#include <algorithm>

#include "App/BeWavedDolphin/SignalModel.h"

namespace DolphinEdits {

void applyToCells(SignalModel &model, const QModelIndexList &cells, const std::function<int(int)> &valueFn)
{
    SignalModel::BulkEditGuard guard(model);
    for (const auto &cell : cells) {
        model.setValue(cell.row(), cell.column(), valueFn(model.value(cell.row(), cell.column())));
    }
}

void clockWave(SignalModel &model, const QModelIndexList &cells, const int firstCol, const int period)
{
    // First half of each period is LOW, second half is HIGH (50% duty cycle), with the
    // phase anchored at firstCol so the waveform starts at 0 regardless of the selection.
    const int halfPeriod = period / 2;

    SignalModel::BulkEditGuard guard(model);
    for (const auto &cell : cells) {
        const int value = ((cell.column() - firstCol) % period < halfPeriod ? 0 : 1);
        model.setValue(cell.row(), cell.column(), value);
    }
}

void combinational(SignalModel &model, const int inputPorts, const int columns)
{
    // Gray-code-like input patterns: row 0 toggles every 1 column (period=2), row 1 every
    // 2 columns (period=4), etc. Together they enumerate all input combinations.
    int halfClockPeriod = 1;
    int clockPeriod     = 2;

    SignalModel::BulkEditGuard guard(model);
    for (int row = 0; row < inputPorts; ++row) {
        for (int col = 0; col < columns; ++col) {
            model.setValue(row, col, (col % clockPeriod < halfClockPeriod ? 0 : 1));
        }

        // Double the period for each successive input bit; cap at max int-safe values
        halfClockPeriod = (std::min)(clockPeriod, 524288);
        clockPeriod     = (std::min)(2 * clockPeriod, 1048576);
    }
}

void clearInputs(SignalModel &model, const int inputPorts)
{
    SignalModel::BulkEditGuard guard(model);
    for (int row = 0; row < inputPorts; ++row) {
        for (int col = 0; col < model.columnCount(); ++col) {
            model.setValue(row, col, 0);
        }
    }
}

int lastNonZeroColumn(const SignalModel &model, const int inputPorts)
{
    for (int col = model.columnCount() - 1; col >= 0; --col) {
        for (int row = 0; row < inputPorts; ++row) {
            if (model.value(row, col) != 0) {
                return col;
            }
        }
    }

    return 0;
}

void growInputColumns(SignalModel &model, const int inputPorts, const int oldLength, const int newLength)
{
    // New input columns must be explicitly filled with zeros; output columns are populated
    // by the simulation run and don't need pre-filling.
    SignalModel::BulkEditGuard guard(model);
    for (int row = 0; row < inputPorts; ++row) {
        for (int col = oldLength; col < newLength; ++col) {
            model.setValue(row, col, 0);
        }
    }
}

} // namespace DolphinEdits
