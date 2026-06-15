// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief SignalDelegate: paints digital waveform graphics into table cells.
 */

#pragma once

#include <QItemDelegate>

class QPainter;

/// Identifies which waveform segment a Line-mode cell draws.
enum class WaveSegment {
    Low,     ///< Logic-low plateau (signal stays at 0).
    High,    ///< Logic-high plateau (signal stays at 1).
    Rising,  ///< Low → high transition (high plateau + leading edge).
    Falling  ///< High → low transition (low plateau + leading edge).
};

/**
 * \class SignalDelegate
 * \brief Item delegate that draws digital waveform graphics inside table cells.
 *
 * \details Replaces the default text rendering with vector waveforms representing
 * logic-high, logic-low, rising-edge, and falling-edge states, painted directly
 * with QPainter at the cell's real size.  Everything is derived at paint time from
 * the SignalModel: the WaveSegment from the cell value and its left neighbour, and
 * the input/output colour from SignalModel::isInputRow().  Nothing is stored per cell.
 *
 * In Number mode (see setWaveformMode()) the delegate falls back to centered
 * QItemDelegate text rendering.
 */
class SignalDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    /// Constructs the delegate.
    explicit SignalDelegate(QObject *parent = nullptr);

    /// Selects waveform rendering (\c true) or numeric text rendering (\c false).
    void setWaveformMode(bool waveformMode);

    /// \reimp
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    /**
     * \brief Returns the waveform segment a cell should draw.
     * \param value      Current cell value (0 or 1).
     * \param hasPrev    true if a cell exists to the left of this one.
     * \param prevValue  Value of the previous cell (only meaningful when hasPrev is true).
     */
    static WaveSegment segmentFor(int value, bool hasPrev, int prevValue);

private:
    /// Draws \a seg into \a cell using the input/output color selected by \a isInput.
    void drawWaveform(QPainter *painter, const QRectF &cell, WaveSegment seg, bool isInput) const;

    bool m_waveformMode = true; ///< true → draw waveforms; false → numeric text.
};
