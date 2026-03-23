// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "App/BeWavedDolphin/SignalModel.h"

SignalModel::SignalModel(const int inputs, const int rows, const int columns, QObject *parent)
    : QStandardItemModel(rows, columns, parent)
    , m_inputCount(inputs)
{
}

Qt::ItemFlags SignalModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index)
    // Cells are read-only in the model; editing is done programmatically via createElement()
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

