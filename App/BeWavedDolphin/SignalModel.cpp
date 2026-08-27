// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/SignalModel.h"

#include "App/Core/Common.h"

SignalModel::SignalModel(const int rows, const int columns, QObject *parent)
    : QStandardItemModel(rows, columns, parent)
{
}

Qt::ItemFlags SignalModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    // Cells are read-only in the model; editing is done programmatically via setCellValue()
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

void SignalModel::setValue(const int row, const int col, const int value)
{
    // Input rows are two-state by definition: WaveformSimulator::sweep() reads them as
    // `value(row, col) != 0`, so anything that is not 0 or 1 drives the input HIGH. Output rows
    // legitimately hold four-state Status (Unknown and Error have their own waveform segments
    // and export characters), and copying one of those cells into an input row is a
    // plain copy/paste away -- DolphinClipboard::paste() bounds the row and column but not the
    // value, and documents the clipboard as untrusted. Clamp here rather than in paste() so all
    // three writers (paste, file load, setCellValue) inherit it, and warn the way
    // DolphinSerializer already does when it clamps a non-binary cell on load.
    int stored = value;
    if (isInputRow(row) && value != 0 && value != 1) {
        qCWarning(zero) << "SignalModel: non-binary value" << value << "written to input row"
                        << row << "column" << col << "- clamped";
        stored = (value == 1) ? 1 : 0;
    }
    setData(index(row, col), stored, Qt::DisplayRole);
}

int SignalModel::value(const int row, const int col) const
{
    return index(row, col).data(Qt::DisplayRole).toInt();
}

void SignalModel::setInputRows(const int inputRows)
{
    m_inputRows = inputRows;
}

void SignalModel::notifyBulkChanged()
{
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

SignalModel::BulkEditGuard::BulkEditGuard(SignalModel &model)
    : m_model(model)
{
    m_model.blockSignals(true);
}

SignalModel::BulkEditGuard::~BulkEditGuard()
{
    m_model.blockSignals(false);
    m_model.notifyBulkChanged();
}
