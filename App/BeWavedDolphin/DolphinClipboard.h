// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief DolphinClipboard: copy/paste of waveform cell rectangles to/from a data stream.
 */

#pragma once

class QDataStream;
class QItemSelection;
class SignalModel;

/**
 * \namespace DolphinClipboard
 * \brief Serializes a selected rectangle of waveform cells to/from a QDataStream.
 *
 * \details Pure data mapping between a SignalModel and a stream: no clipboard/MIME
 * handling and no simulation (the controller's slots own those). Cell values are
 * stored relative to the selection's top-left so they can be re-anchored on paste.
 */
namespace DolphinClipboard {

/// Returns the leftmost column index in \a ranges (clamped to the model's columns).
int firstColumn(const SignalModel &model, const QItemSelection &ranges);

/// Returns the topmost row index in \a ranges (clamped to the model's rows).
int firstRow(const SignalModel &model, const QItemSelection &ranges);

/// Serializes the values of the cells in \a ranges to \a stream (offsets relative to
/// the selection's top-left).
void copy(const SignalModel &model, const QItemSelection &ranges, QDataStream &stream);

/// Deserializes cells from \a stream into \a model, anchored at the top-left of \a ranges.
/// Cells landing outside the input rows or past the last column are silently dropped.
void paste(SignalModel &model, const QItemSelection &ranges, QDataStream &stream);

} // namespace DolphinClipboard
