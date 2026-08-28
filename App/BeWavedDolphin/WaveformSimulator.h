// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

/** \file
 * \brief Drives the wiRedPanda circuit simulation column-by-column for the waveform table.
 */

#pragma once

#include <functional>

#include <QVector>

#include "App/BeWavedDolphin/DolphinModelBuilder.h"
#include "App/Core/Enums.h"

class GraphicElement;
class GraphicElementInput;
class Scene;
class Simulation;

/**
 * \class WaveformSimulator
 * \brief Drives the circuit simulation across the waveform's time columns.
 *
 * \details Extracted from BewavedDolphin: this owns the circuit-coupled sweep logic —
 * resetting sequential state, applying input values per time step, advancing the
 * simulation, and reading back output statuses — while leaving the waveform model and
 * its presentation to the caller via read/write callbacks.
 */
class WaveformSimulator
{
public:
    /// Constructs the driver for \a externalScene and its \a simulation.
    WaveformSimulator(Scene *externalScene, Simulation *simulation);

    /**
     * \brief Snapshots the live output-port state of every input element.
     * \param inputs     Input elements to snapshot.
     * \param inputPorts Total number of output ports across \a inputs (snapshot size).
     * \return Port-indexed states, to be restored after a sweep via restoreInputs().
     */
    static QVector<Status> captureInputs(const QVector<GraphicElementInput *> &inputs, int inputPorts);

    /// Restores input-port states previously captured by captureInputs().
    static void restoreInputs(const QVector<GraphicElementInput *> &inputs, const QVector<Status> &saved);

    /**
     * \brief Runs the simulation across \a columns time steps.
     * \param rows        The table's row layout, in display order (DolphinModelBuilder::Row).
     * \param columns     Number of time-step columns to sweep.
     * \param readInput   Returns the input bit at (row, col) from the waveform model.
     * \param writeOutput Stores the computed output bit at (row, col) into the model.
     *
     * \details Takes the row descriptors rather than the element vectors plus a row offset:
     * a row's index, its element and its port all come from one place, so the sweep cannot
     * drift out of step with the snapshot API (see DolphinModelBuilder::Row).
     * Resets every element's sequential state before sweeping. Does not restore inputs — the
     * caller pairs this with restoreInputs().
     */
    void sweep(const QVector<DolphinModelBuilder::Row> &rows, int columns,
               const std::function<bool(int row, int col)> &readInput,
               const std::function<void(int row, int col, int value)> &writeOutput) const;

private:
    Scene *m_externalScene = nullptr;    ///< The circuit scene being simulated.
    Simulation *m_simulation = nullptr;  ///< The simulation engine used for the sweep.
};
