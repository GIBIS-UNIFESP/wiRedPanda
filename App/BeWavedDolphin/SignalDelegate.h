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
 * with QPainter at the cell's real size.  Line-mode cells carry a WaveSegment in
 * SegmentRole and an input/output flag in InputRole (set by
 * BewavedDolphin::setCellValue).
 *
 * In Number mode (no SegmentRole stored) the delegate falls back to the default
 * QItemDelegate text rendering.
 */
class SignalDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    /// Item-data role holding the cell's WaveSegment (Line mode only).
    static constexpr int SegmentRole = Qt::UserRole + 1;
    /// Item-data role holding the cell's input flag (true → blue, false → green).
    static constexpr int InputRole = Qt::UserRole + 2;

    /// Constructs the delegate.
    explicit SignalDelegate(QObject *parent = nullptr);

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
};
