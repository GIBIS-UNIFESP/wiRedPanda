// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief SignalDelegate: paints digital waveform graphics into table cells.
 */

#pragma once

#include <QItemDelegate>
#include <QPixmap>

/**
 * \class SignalDelegate
 * \brief Item delegate that draws digital waveform graphics inside table cells.
 *
 * \details Replaces the default text rendering with pixmaps representing logic-high,
 * logic-low, rising-edge, and falling-edge states.  The pixmaps are loaded once at
 * construction time and owned by the delegate.
 *
 * In Number mode (no pixmap stored in DecorationRole) the delegate falls back to the
 * default QItemDelegate text rendering.
 */
class SignalDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    /// Constructs the delegate and pre-loads all waveform pixmaps.
    explicit SignalDelegate(QObject *parent = nullptr);

    /// \reimp
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    /**
     * \brief Returns the correct waveform pixmap for a cell.
     * \param value      Current cell value (0 or 1).
     * \param isInput    true → blue (input) pixmaps; false → green (output) pixmaps.
     * \param hasPrev    true if a cell exists to the left of this one.
     * \param prevValue  Value of the previous cell (only meaningful when hasPrev is true).
     */
    QPixmap pixmapFor(int value, bool isInput, bool hasPrev, int prevValue) const;

private:
    // --- Helpers ---

    /// Loads all 8 waveform pixmaps from Qt resources into the member variables.
    void loadPixmaps();

    // --- Members ---

    QPixmap m_fallingBlue;  ///< Blue falling-edge waveform (input signal).
    QPixmap m_fallingGreen; ///< Green falling-edge waveform (output signal).
    QPixmap m_highBlue;     ///< Blue logic-high plateau (input signal).
    QPixmap m_highGreen;    ///< Green logic-high plateau (output signal).
    QPixmap m_lowBlue;      ///< Blue logic-low plateau (input signal).
    QPixmap m_lowGreen;     ///< Green logic-low plateau (output signal).
    QPixmap m_risingBlue;   ///< Blue rising-edge waveform (input signal).
    QPixmap m_risingGreen;  ///< Green rising-edge waveform (output signal).
};

