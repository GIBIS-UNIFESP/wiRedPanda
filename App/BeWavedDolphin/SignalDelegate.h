// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief SignalDelegate: waveform cell-segment classification shared by the exporters.
 */

#pragma once

/// Controls how signal cells are rendered in the waveform table.
enum class PlotType {
    Number, ///< Cells display the numeric value (0/1).
    Line    ///< Cells display a waveform-style rising/falling edge graphic.
};

/// Identifies which waveform segment a Line-mode cell draws.
enum class WaveSegment {
    Low,     ///< Logic-low plateau (signal stays at 0).
    High,    ///< Logic-high plateau (signal stays at 1).
    Rising,  ///< Low → high transition (high plateau + leading edge).
    Falling  ///< High → low transition (low plateau + leading edge).
};

/**
 * \namespace SignalDelegate
 * \brief Waveform cell-segment classification, shared by every renderer of a SignalModel.
 *
 * \details Originally a QItemDelegate subclass that painted waveform cells directly into a
 * QTableView (App/BeWavedDolphin/BeWavedDolphin.cpp, the Widgets waveform editor). That
 * painting code is gone along with the rest of the Widgets app; segmentFor() is the one
 * genuinely shared piece -- QuickDolphinExporter.cpp's drawWaveformCell() and
 * SignalTable.qml's cell delegate both reproduce the same band/plateau/edge-bar geometry
 * independently (QML can't call into this), keyed off this same classification.
 */
namespace SignalDelegate {

/**
 * \brief Returns the waveform segment a cell should draw.
 * \param value      Current cell value (0 or 1).
 * \param hasPrev    true if a cell exists to the left of this one.
 * \param prevValue  Value of the previous cell (only meaningful when hasPrev is true).
 */
WaveSegment segmentFor(int value, bool hasPrev, int prevValue);

} // namespace SignalDelegate
