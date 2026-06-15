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
 * \brief The beWavedDolphin clipboard layer: cell ↔ stream mapping plus the system
 * clipboard transport (MIME wrapping and the versioned data header).
 *
 * \details copy()/paste() are the pure data mapping between a SignalModel and a
 * QDataStream (no simulation — the controller's slots own that). copyToClipboard()/
 * pasteFromClipboard() add the transport: the application MIME type, the legacy MIME
 * type accepted on paste, and the versioned Serialization header. Cell values are
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

/// Serializes the cells in \a ranges (with the versioned header) and places them on the
/// system clipboard under the beWavedDolphin MIME type. \a ranges must be non-empty.
void copyToClipboard(const SignalModel &model, const QItemSelection &ranges);

/// Reads beWavedDolphin waveform data from the system clipboard (current or legacy MIME
/// type) and pastes it into \a model anchored at the top-left of \a ranges. Returns
/// \c true iff the clipboard held waveform data that was applied.
bool pasteFromClipboard(SignalModel &model, const QItemSelection &ranges);

} // namespace DolphinClipboard
