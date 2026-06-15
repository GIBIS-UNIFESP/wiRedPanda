// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief SignalModel for the beWavedDolphin waveform table.
 */

#pragma once

#include <QStandardItemModel>

/**
 * \class SignalModel
 * \brief QStandardItemModel subclass that makes all cells non-editable.
 *
 * \details All editing is done programmatically via BewavedDolphin::setCellValue().
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
    SignalModel(int rows, int columns, QObject *parent = nullptr);

    /// Maximum number of time-step columns a waveform may have. Single source of truth for
    /// the upper bound, shared by the file loaders, the runtime length, and the MCP cap.
    /// The *minimum* is intentionally context-specific (a saved file needs >= 2 columns; a
    /// runtime length allows >= 1) and is not centralized here.
    static constexpr int kMaxColumns = 2048;

    /// \reimp Returns Qt::ItemIsEnabled only (no editing).
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // --- Waveform data access ---

    /// Sets the logic value (0/1) of the cell at (\a row, \a col).
    void setValue(int row, int col, int value);

    /// Returns the logic value (0/1) of the cell at (\a row, \a col).
    int value(int row, int col) const;

    /// Records how many leading rows are inputs (the rest are outputs).
    void setInputRows(int inputRows);

    /// Returns the number of input rows (the output rows follow them).
    int inputRows() const { return m_inputRows; }

    /// Returns \c true if \a row is an input row (\a row < inputRows()).
    bool isInputRow(int row) const { return row < m_inputRows; }

private:
    int m_inputRows = 0; ///< Number of leading input rows; the rest are outputs.
};
