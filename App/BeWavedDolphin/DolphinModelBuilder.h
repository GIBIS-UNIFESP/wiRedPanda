// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief DolphinModelBuilder: maps a circuit scene's I/O elements to waveform signal rows.
 */

#pragma once

#include <QStringList>
#include <QVector>

class GraphicElement;
class GraphicElementInput;
class Scene;

/**
 * \namespace DolphinModelBuilder
 * \brief Collects and orders the input/output elements of a circuit scene and derives the
 * waveform table's row labels.
 *
 * \details Pure domain logic — no view, model, or simulation. The controller takes the
 * result, snapshots the live input states, builds the SignalModel, and wires the view.
 */
namespace DolphinModelBuilder {

/// Which side of the table a row belongs to.
enum class RowKind { Input, Output };

/**
 * \brief One waveform table row: exactly one PORT of one element.
 *
 * \details The table is laid out per port -- a 2-bit rotary input occupies two rows, a
 * Display7 occupies eight -- so an element index is never a row index. This descriptor is the
 * single source of truth for that layout: the label builder, the sweep and the snapshot API
 * all consume it rather than deriving it independently, which is what keeps them from
 * disagreeing about which row belongs to which port.
 */
struct Row {
    GraphicElement *element = nullptr;  ///< The element this row reads or drives.
    int port = 0;                       ///< Port index on \a element (output port for an
                                        ///< input row, input port for an output row).
    RowKind kind = RowKind::Input;
    QString label;                      ///< Per-PORT label ("Bus[0]"), never the bare element
                                        ///< label -- eight Display7 rows must not share one.
};

/// The input/output elements (label-sorted) and the row layout for a waveform table.
struct Signals {
    QVector<GraphicElementInput *> inputs;  ///< Input elements, sorted by label.
    QVector<GraphicElement *> outputs;      ///< Output elements, sorted by label.
    int inputPorts = 0;                     ///< Total input rows; derived from \a rows.
    QStringList inputLabels;                ///< Row label per input port (indexed for multi-port).
    QStringList outputLabels;               ///< Row label per output port (indexed for multi-port).
    /// Every table row in display order: all input rows, then all output rows. Derived once,
    /// here; inputPorts/inputLabels/outputLabels are projections of it, kept for the callers
    /// that only need the counts or the header strings.
    QVector<Row> rows;
};

/// Gathers \a scene's input/output elements, stable-sorts them by label, validates the
/// circuit, and builds the per-port row labels. Throws (Pandaception) on an empty circuit
/// or one missing inputs or outputs.
Signals collect(Scene *scene);

} // namespace DolphinModelBuilder
