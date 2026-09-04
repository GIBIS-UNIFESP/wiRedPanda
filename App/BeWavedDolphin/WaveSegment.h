// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief WaveSegment/PlotType: the beWavedDolphin waveform-cell rendering vocabulary, plus the
 * pure logic that derives a segment from a cell's value and its left neighbour.
 *
 * \details Split out of SignalDelegate.h so this vocabulary (and segmentFor()'s pure logic) is
 * reachable without pulling in SignalDelegate's own QItemDelegate base (Qt Widgets) -- letting
 * wiredpanda's QuickDolphinExporter/QuickDolphinController reuse it directly instead of
 * duplicating the enum values under a different type. SignalDelegate::segmentFor() (the
 * Widgets-side call site every existing caller already uses) now just forwards to segmentFor()
 * here, so no existing call site needed to change.
 */

#pragma once

/// Controls how signal cells are rendered in the waveform table.
enum class PlotType {
    Number, ///< Cells display the numeric value (0/1).
    Line    ///< Cells display a waveform-style rising/falling edge graphic.
};

/// Identifies which waveform segment a Line-mode cell draws.
enum class WaveSegment {
    Low,      ///< Logic-low plateau (signal stays at 0).
    High,     ///< Logic-high plateau (signal stays at 1).
    Rising,   ///< Low → high transition (high plateau + leading edge).
    Falling,  ///< High → low transition (low plateau + leading edge).
    Unknown,  ///< Undefined (Status::Unknown): drawn mid-level in the canvas's "unknown" grey.
    Error     ///< Conflicting drivers (Status::Error): drawn mid-level in the canvas's red.
};

/**
 * \brief Returns the waveform segment a cell should draw.
 * \param value      Current cell value (0 or 1, or a raw four-state Status).
 * \param hasPrev    true if a cell exists to the left of this one.
 * \param prevValue  Value of the previous cell (only meaningful when hasPrev is true).
 */
WaveSegment segmentFor(int value, bool hasPrev, int prevValue);
