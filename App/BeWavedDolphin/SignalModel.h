// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief SignalModel and PlotType for the beWavedDolphin waveform table.
 */

#pragma once

#include <QStandardItemModel>

/// Controls how signal cells are rendered in the waveform table.
enum class PlotType {
    Number, ///< Cells display the numeric value (0/1).
    Line    ///< Cells display a waveform-style rising/falling edge graphic.
};

/**
 * \class SignalModel
 * \brief QStandardItemModel subclass that makes all cells non-editable.
 *
 * \details All editing is done programmatically via BewavedDolphin::createElement().
 * The model enforces read-only at the Qt item level so the table view never opens
 * an inline editor.
 */
class SignalModel : public QStandardItemModel
{
    Q_OBJECT

public:
    /**
     * \brief Constructs the model.
     * \param inputs  Number of input signal rows (stored for reference).
     * \param rows    Total number of signal rows.
     * \param columns Number of time-step columns.
     * \param parent  Optional parent object.
     */
    SignalModel(int inputs, int rows, int columns, QObject *parent = nullptr);

    /// \reimp Returns Qt::ItemIsEnabled only (no editing).
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    // --- Members ---

    const int m_inputCount; ///< Number of input rows; stored for potential range checks.
};
