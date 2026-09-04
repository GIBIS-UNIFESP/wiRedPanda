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
class Simulation;

/**
 * \class WaveformSimulator
 * \brief Drives the circuit simulation across the waveform's time columns.
 *
 * \details Owns the circuit-coupled sweep logic — resetting sequential state, applying
 * input values per time step, advancing the simulation, and reading back output statuses —
 * while leaving the waveform model and its presentation to the caller via read/write
 * callbacks.
 *
 * Takes the element list as a parameter (see sweep()'s \a allElements) rather than storing
 * a Scene*: the list must be fetched fresh on every sweep() call, never cached, since the
 * live element set can change between two sweeps (e.g. the user editing the main canvas
 * while this window is open) and a stale cache would silently skip resetting newly added
 * elements' sequential state. Both Scene::elements() and CanvasItem::elements() produce the
 * same QVector<GraphicElement*>, so this class works unmodified for both apps.
 */
class WaveformSimulator
{
public:
    /// Constructs the driver for \a simulation.
    explicit WaveformSimulator(Simulation *simulation);

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
     * \param allElements Every element in the circuit (Scene::elements()/CanvasItem::elements()),
     *                    fetched fresh by the caller immediately before this call -- reset to
     *                    power-on sequential state before the sweep.
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
    void sweep(const QVector<DolphinModelBuilder::Row> &rows,
               const QVector<GraphicElement *> &allElements, int columns,
               const std::function<bool(int row, int col)> &readInput,
               const std::function<void(int row, int col, int value)> &writeOutput) const;

private:
    Simulation *m_simulation = nullptr;  ///< The simulation engine used for the sweep.
};
