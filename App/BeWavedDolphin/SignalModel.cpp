// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/SignalModel.h"

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
    setData(index(row, col), value, Qt::DisplayRole);
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
