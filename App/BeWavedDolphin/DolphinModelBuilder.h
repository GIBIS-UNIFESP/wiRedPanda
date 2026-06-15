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

/// The input/output elements (label-sorted) and the row labels for a waveform table.
struct Signals {
    QVector<GraphicElementInput *> inputs;  ///< Input elements, sorted by label.
    QVector<GraphicElement *> outputs;      ///< Output elements, sorted by label.
    int inputPorts = 0;                     ///< Total input rows (sum of input output-port counts).
    QStringList inputLabels;                ///< Row label per input port (indexed for multi-port).
    QStringList outputLabels;               ///< Row label per output port (indexed for multi-port).
};

/// Gathers \a scene's input/output elements, stable-sorts them by label, validates the
/// circuit, and builds the per-port row labels. Throws (Pandaception) on an empty circuit
/// or one missing inputs or outputs.
Signals collect(Scene *scene);

} // namespace DolphinModelBuilder
